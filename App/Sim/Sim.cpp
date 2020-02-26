#include <fstream>
#include <iostream>
#include <pthread.h>
#include <vector>

#include "sgx_urts.h"
#include "../Enclave_u.h"
#include "../../Common/Message.h"
#include "../../Common/Ringbuffer.h"

#define ENCLAVE_FILE "Enclave.signed.so"

using namespace std;

#define START_FILE_NO 1
#define END_FILE_NO 2

typedef vector<FIVE_TUPLE> TRACE;
TRACE traces[END_FILE_NO - START_FILE_NO + 1];

void ocall_print_string(const char *str) {
    printf("%s\n", str);
}

void ReadInTraces(const char *trace_prefix) {
    for(int datafileCnt = START_FILE_NO; datafileCnt <= END_FILE_NO; ++datafileCnt)
    {
        char datafileName[100];
        sprintf(datafileName, "%s%d.dat", trace_prefix, datafileCnt - 1);
        FILE *fin = fopen(datafileName, "rb");

        FIVE_TUPLE tmp_five_tuple{};
        traces[datafileCnt - 1].clear();
        while(fread(&tmp_five_tuple, 1, 13, fin) == 13)
        {
            traces[datafileCnt - 1].push_back(tmp_five_tuple);
        }
        fclose(fin);

        printf("Successfully read in %s, %ld packets\n", datafileName, traces[datafileCnt - 1].size());
    }
    printf("\n");
}

void *e_thread(void *eid) {
    ecall_run(*((sgx_enclave_id_t*) eid));
}

void add_test_queries(Ringbuffer<Message, 1024> *query_buf, struct ctx_gcm_s *ctx) {
    // Add test queries
    for(int i = FLOW_SIZE; i <= CARDINALITY; i++) {
        Message query_message;
        pack_message(&query_message, (message_type) i, ctx, nullptr, 0);

        query_buf->push(query_message);
    }

    // Add end message
    Message end_message;
    pack_message(&end_message, STOP, ctx, nullptr, 0);

    query_buf->push(end_message);
}

void process_result(Ringbuffer<Message, 1024> *res_buf, struct ctx_gcm_s *ctx) {
    Message res_message;
    while (!res_buf->isEmpty()) {
        res_buf->pop(res_message);
        uint8_t valid_payload[res_message.header.payload_size - GCM_IV_SIZE];
        unpack_message(&res_message, ctx, valid_payload);

        switch (res_message.header.type) {
            case CARDINALITY:
                int *card = (int*) valid_payload;
                printf("Cardinality of Flows: %d\n", *card);
                break;
        }
    }
}

int main() {
    // setup OVS parameters
    struct ctx_gcm_s ctx;

    // init ctx block
    alloc_gcm(&ctx);

    // setup enclave
    sgx_enclave_id_t eid;
    sgx_status_t ret;
    sgx_launch_token_t token = { 0 };
    int token_updated = 0;

    ret = sgx_create_enclave(ENCLAVE_FILE, SGX_DEBUG_FLAG, &token, &token_updated, &eid, NULL);
    if (ret != SGX_SUCCESS)
    {
        cout <<"sgx_create_enclave failed: 0x" << std::hex << ret << endl;
        return 1;
    }

    Ringbuffer<Message, 1024> *rb_in = new Ringbuffer<Message, 1024>();
    Ringbuffer<Message, 1024> *rb_out = new Ringbuffer<Message, 1024>();

    // initialise the enclave with message queues
    ecall_init(eid, rb_in, rb_out, ctx.key, GCM_KEY_SIZE);

    // use another thread to process requests
    pthread_t pid;
    pthread_create(&pid, NULL, e_thread, (void*) &eid);

    // read offline data
    //ReadInTraces("data/");
    for(int datafileCnt = START_FILE_NO; datafileCnt <= END_FILE_NO; ++datafileCnt) {
        char datafileName[100];
        sprintf(datafileName, "%s%d.dat", "data/", datafileCnt - 1);

        // pack and offline data
        Message message;
        pack_message_with_file(&message, STAT, &ctx, datafileName);

        // send to the enclave
        rb_in->push(message);

    }

    add_test_queries(rb_in, &ctx);

    void *status;
    pthread_join(pid, &status);

    // get query results
    process_result(rb_out, &ctx);

    // clean up

    // destroy the enclave
    sgx_destroy_enclave(eid);
}

