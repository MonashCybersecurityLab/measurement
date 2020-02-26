#include "EnclaveFunction.h"
#include "../Common/Message.h"
#include "../Common/Ringbuffer.h"

using namespace std;

struct ctx_gcm_s ctx;

CMSketch<4, 3> *sketch = nullptr;

Ringbuffer<Message, 1024> *input_buf = nullptr;
Ringbuffer<Message, 1024> *output_buf = nullptr;

unordered_map<string, float> prep_statistics;
unordered_map<string, float> cur_statistics;

void ecall_init(void *rb_in, void *rb_out, unsigned char *ovs_key, size_t key_size) {
    memcpy(ctx.key, ovs_key, key_size);

    input_buf = (Ringbuffer<Message, 1024>*) rb_in;
    output_buf = (Ringbuffer<Message, 1024>*) rb_out;

    sketch = new CMSketch<4, 3>(600 * 1024);
    sketch->print_basic_info();
}

void ecall_run() {
    while (1) {
        // get one message from buffer
        Message in_message;
        input_buf->pop(in_message);

        switch (in_message.header.type) {
            case STAT:
                // decrypt the payload if it is available
                if(in_message.header.payload_size > 0) {
                    // allocate space for the message
                    uint8_t valid_payload[in_message.header.payload_size - GCM_IV_SIZE];
                    int payload_size = unpack_message(&in_message, &ctx, valid_payload);
                    printf("%d packets received\n", payload_size / 13);
                    // clear the present statistics
                    prep_statistics.clear();
                    prep_statistics = cur_statistics;
                    // insert into the sketch and list
                    add_trace(sketch, cur_statistics, valid_payload, payload_size);
                }
                break;
            case FLOW_SIZE:
                // query the flow size of a given flow ID
                if(in_message.header.payload_size > 0) {
                    // allocate space for the message
                    uint8_t five_tuple[13];
                    int payload_size = unpack_message(&in_message, &ctx, five_tuple);
                    int flow_size = sketch->query(five_tuple);
                }
                break;
            case HEAVY_HITTER:
                break;
            case CARDINALITY:
            {
                int stat_size = cur_statistics.size();
                Message out_message;
                pack_message(&out_message, CARDINALITY, &ctx, (uint8_t*) &stat_size, sizeof(int));
                output_buf->push(out_message);
            }
                break;
            case STOP:
                return;
            default:
                break;
        }
    }

}

void ecall_add_trace(struct FIVE_TUPLE *trace, size_t trace_size) {
    // clear the present statistics
    prep_statistics.clear();
    prep_statistics = cur_statistics;

    printf("%d packets received\n", trace_size);
    // remove the last statistics
    sketch->reset();
    cur_statistics.clear();

    // add new info
    for(size_t i = 0; i < trace_size; i++) {
        sketch->insert((uint8_t*)(trace[i].key));

        string str((const char*)(trace[i].key), 4);
        cur_statistics[str]++;
    }

    // compute occurrences
    for(auto & it : cur_statistics) {
        it.second = it.second * 100 / trace_size;
    }
}