#include "EnclaveFunction.h"

using namespace std;

struct ctx_gcm_s ctx;

CMSketch<FLOW_KEY_SIZE, SKETCH_HASH> *sketch = nullptr;

Queue *message_pool = nullptr;
Queue *input_queue = nullptr;
Queue *output_queue = nullptr;

unordered_map<string, float> prep_statistics;
unordered_map<string, float> cur_statistics;

void ecall_init(void *pool, void *queue_in, void *queue_out, unsigned char *ovs_key, size_t key_size) {
    memcpy(ctx.key, ovs_key, key_size);

    message_pool = (Queue*) pool;
    input_queue = (Queue*) queue_in;
    output_queue = (Queue*) queue_out;

    sketch = new CMSketch<FLOW_KEY_SIZE, SKETCH_HASH>(TOTAL_MEM);
    sketch->print_basic_info();
}

void ecall_run() {
    while (1) {
        // get one message from buffer
        if(!is_empty_queue(input_queue)){
            Message *in_message = pop_front(input_queue);

            switch (in_message->header.type) {
                case STAT:
                    // decrypt the payload if it is available
                    if(in_message->header.payload_size > 0) {
                        // allocate space for the message
                        uint8_t valid_payload[in_message->header.payload_size - GCM_IV_SIZE];
                        int payload_size = unpack_message(in_message, &ctx, valid_payload);
                        printf("%d packets received\n", payload_size / FLOW_ID_SIZE);
                        // clear the present statistics
                        prep_statistics.clear();
                        prep_statistics = cur_statistics;
                        // insert into the sketch and list
                        add_trace(sketch, cur_statistics, valid_payload, payload_size);
                    }
                    break;
                case FLOW_SIZE:
                    // query the flow size of a given flow ID
                    if(in_message->header.payload_size > 0) {
                        // allocate space for the message
                        uint8_t five_tuple[FLOW_KEY_SIZE + sizeof(int)]; // FLOW_ID + result = 13 + 4 = 17
                        unpack_message(in_message, &ctx, five_tuple);
                        uint32_t flow_size = sketch->query(five_tuple);
                        memcpy(five_tuple + FLOW_KEY_SIZE, &flow_size, sizeof(int));
                        // add a flow size response
                        Message *out_message = pop_front(message_pool);
                        pack_message(out_message, FLOW_SIZE, &ctx, five_tuple, FLOW_KEY_SIZE + sizeof(int), 1);
                        push_back(output_queue, out_message);
                    }
                    break;
                case HEAVY_HITTER:
                    // query the heavy hitter regarding a pre-defined K
                    if(in_message->header.payload_size > 0) {
                        // query the statistics module
                        uint8_t k[sizeof(int)];
                        unpack_message(in_message, &ctx, k);
                        vector<pair<string, float>> res_vector = query_heavy_hitter(cur_statistics, *((int*) k));
                        // convert vector to uint8_t
                        uint8_t heavy_hitter_buffer[res_vector.size() * FLOW_ID_SIZE];
                        for(int i = 0; i < res_vector.size(); i++) {
                            memcpy(heavy_hitter_buffer + i * FLOW_KEY_SIZE, res_vector[i].first.c_str(), FLOW_KEY_SIZE);
                        }
                        // add a Heavy Hitters response
                        Message *out_message = pop_front(message_pool);
                        pack_message(out_message, HEAVY_HITTER, &ctx, heavy_hitter_buffer, res_vector.size() * FLOW_KEY_SIZE, 1);
                        push_back(output_queue, out_message);
                    }
                    break;
                case HEAVY_CHANGE:
                {
                    // query the heavy change regarding a pre-defined T
                    if(in_message->header.payload_size > 0) {
                        // query the statistics module
                        uint8_t T[sizeof(float)];
                        unpack_message(in_message, &ctx, T);
                        vector<string> res_vector = query_heavy_change(prep_statistics, cur_statistics, *((float*) T));
                        // convert vector to uint8_t
                        uint8_t heavy_change_buffer[res_vector.size() * FLOW_KEY_SIZE];
                        for(int i = 0; i < res_vector.size(); i++) {
                            memcpy(heavy_change_buffer + i * FLOW_KEY_SIZE, res_vector[i].c_str(), FLOW_KEY_SIZE);
                        }
                        // add a Heavy Changes response
                        Message *out_message = pop_front(message_pool);
                        pack_message(out_message, HEAVY_CHANGE, &ctx, heavy_change_buffer, res_vector.size() * FLOW_KEY_SIZE, 1);
                        push_back(output_queue, out_message);
                    }
                }
                    break;
                case DIST:
                {
                    vector<uint32_t> dist = query_dist(sketch, cur_statistics);
                    // add a dist response
                    Message *out_message = pop_front(message_pool);
                    pack_message(out_message, DIST, &ctx, (uint8_t*) dist.data(), dist.size() * sizeof(uint32_t), 1);
                    push_back(output_queue, out_message);
                }
                    break;
                case CARDINALITY:
                {
                    int stat_size = cur_statistics.size();
                    // add a cardinality response
                    Message *out_message = pop_front(message_pool);
                    pack_message(out_message, CARDINALITY, &ctx, (uint8_t*) &stat_size, sizeof(int), 1);
                    push_back(output_queue, out_message);
                }
                    break;
                case ENTROPY:
                {
                    float entropy = query_entropy(cur_statistics);
                    // add an entropy response
                    Message *out_message = pop_front(message_pool);
                    pack_message(out_message, ENTROPY, &ctx, (uint8_t*) &entropy, sizeof(float), 1);
                    push_back(output_queue, out_message);
                }
                    break;
                case STOP:
                    return;
                default:
                    break;
            }
            // free incoming message from the simulator and return the container to the global pool
            ocall_free_message(in_message);
            push_back(message_pool, in_message);
        }
    }
}