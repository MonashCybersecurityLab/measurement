#include <fstream>
#include <iostream>
#include <pthread.h>
#include <vector>
#include <arpa/inet.h>

#include "sgx_urts.h"
#include "Enclave_u.h"
#include "../../Common/Queue.h"

#define ENCLAVE_FILE "Enclave.signed.so"

using namespace std;

#define START_FILE_NO 1
#define END_FILE_NO 2

typedef vector<FIVE_TUPLE> TRACE;
TRACE traces[END_FILE_NO - START_FILE_NO + 1];

Queue global_pool;
Queue mbox[2];

void ocall_print_string(const char *str) {
    printf("%s\n", str);
}

void ocall_alloc_message(void *ptr, size_t size) {
    Message *msg = (Message*) ptr;
    msg->payload = (uint8_t*) malloc(size);
}

void ocall_free_message(void *ptr) {
    Message *msg = (Message*) ptr;
    if(msg->payload != nullptr) {
        free(msg->payload);
    }
    memset(msg, 0, sizeof(Message));

}

void ReadInTraces(const char *trace_prefix) {
    for(int datafileCnt = START_FILE_NO; datafileCnt <= END_FILE_NO; ++datafileCnt)
    {
        char datafileName[100];
        sprintf(datafileName, "%s%d", trace_prefix, datafileCnt);
        FILE *fin = fopen(datafileName, "rb");

        FIVE_TUPLE tmp_five_tuple{};
        traces[datafileCnt - 1].clear();
        while(fread(&tmp_five_tuple, 1, 13, fin) == 13)
        {
            int value;
            fread(&value, sizeof(uint32_t), 1, fin);
            traces[datafileCnt - 1].push_back(tmp_five_tuple);
        }
        fclose(fin);

        printf("Successfully read in %s, %ld flows\n", datafileName, traces[datafileCnt - 1].size());
    }
    printf("\n");
}
void *e_thread(void *eid) {
    ecall_run(*((sgx_enclave_id_t*) eid));
}

void parse_flow_id(uint32_t key) {
    printf("Flow ID: %s\n", inet_ntoa(*((struct in_addr*) &(key))));
}

void add_test_queries(struct ctx_gcm_s *ctx) {
    // Add test queries
    for(int i = FLOW_SIZE; i <= ENTROPY; i++) {
        Message *query_message = pop_front(&global_pool);
        switch ((message_type) i) {
            case FLOW_SIZE:
                pack_message(query_message, (message_type) i, ctx, (uint8_t*) traces[0].data(), FLOW_KEY_SIZE, 0);
                break;
            case HEAVY_HITTER:
            {
                int k = HEAVY_HITTER_SIZE; // top-20 flows as the heavy hitters
                pack_message(query_message, (message_type) i, ctx, (uint8_t*) &k, sizeof(int), 0);
            }
                break;
            case HEAVY_CHANGE:
            {
                float T = HEAVY_CHANGE_THRESHOLD;
                pack_message(query_message, (message_type) i, ctx, (uint8_t*) &T, sizeof(float), 0);
            }
                break;
            default:
                pack_message(query_message, (message_type) i, ctx, nullptr, 0, 0);
                break;
        }
        // add into the buffer
        push_back(&mbox[0], query_message);
    }

    // Add end message
    Message *end_message = pop_front(&global_pool);
    pack_message(end_message, STOP, ctx, nullptr, 0, 0);

    push_back(&mbox[0], end_message);
}

void process_result(struct ctx_gcm_s *ctx) {
    while (!is_empty_queue(&mbox[1])) {
        Message *res_message = pop_front(&mbox[1]);
        uint8_t valid_payload[res_message->header.payload_size - GCM_IV_SIZE];
        unpack_message(res_message, ctx, valid_payload);

        switch (res_message->header.type) {
            case FLOW_SIZE:
            {
                int *flow_size = (int *) (valid_payload + FLOW_KEY_SIZE);
                parse_flow_id(*((uint32_t*)valid_payload));
                printf("size: %d\n", *flow_size);
            }
                break;
            case HEAVY_HITTER:
            {
                printf("Heavy hitter list:\n");
                for(int i = 0; i < HEAVY_HITTER_SIZE; i++) {
                    parse_flow_id(*((uint32_t*)(valid_payload + i * FLOW_KEY_SIZE)));
                }
            }
                break;
            case HEAVY_CHANGE:
            {
                printf("Heavy change list:\n");
                int size = (res_message->header.payload_size - GCM_IV_SIZE) / FLOW_KEY_SIZE;
                for(int i = 0; i < size; i++) {
                    parse_flow_id(*((uint32_t*)(valid_payload + i * FLOW_KEY_SIZE)));
                }
            }
                break;
            case CARDINALITY:
            {
                int *card = (int*) valid_payload;
                printf("Cardinality of Flows: %d\n", *card);
            }
                break;
            case DIST:
            {
                uint32_t *dist = (uint32_t*) valid_payload;
                int size = (res_message->header.payload_size - GCM_IV_SIZE) / sizeof(uint32_t);
                printf("Flow Size Distribution <Flow Size, Count>:\n");
                for(int i = 0, j = 0; i < size; i++) {
                    if(dist[i] != 0) {
                        printf("<%d, %d>", i, dist[i]);
                        if(++j % 10 == 0) {
                            printf("\n");
                        } else {
                            printf("\t");
                        }
                    }
                }
                printf("\n");
            }
                break;
            case ENTROPY:
            {
                float *entropy = (float*) valid_payload;
                printf("Entropy of Flows: %.3f\n", *entropy);
            }
                break;
            default:
                break;
        }

        // free incoming message from the enclave and return the container to the global pool
        free_message(res_message);
        push_back(&global_pool, res_message);
    }
}

int main(int argc, char **argv) {
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

    queue_init(&global_pool);

    queue_init(&mbox[0]);
    queue_init(&mbox[1]);

    for(int i = 0; i < MAX_POOL_SIZE; i++) {
        Message *tmp = (Message *) malloc(sizeof(Message));
        memset(tmp, 0, sizeof(Message));
        push_back(&global_pool, tmp);
    }

    // initialise the enclave with message queues
    ecall_init(eid, &global_pool, &mbox[0], &mbox[1], ctx.key, GCM_KEY_SIZE);

    // use another thread to process requests
    pthread_t pid;
    pthread_create(&pid, NULL, e_thread, (void*) &eid);

    // read offline data
    ReadInTraces(argv[1]);

    // submit trace to the enclave
    for(int datafileCnt = START_FILE_NO; datafileCnt <= END_FILE_NO; ++datafileCnt) {
        char datafileName[100];
        sprintf(datafileName, "%s%d", argv[1], datafileCnt);

        // pack and offline data
        Message *message = pop_front(&global_pool);
        pack_message_with_file(message, STAT, &ctx, datafileName);

        // send to the enclave
        push_back(&mbox[0], message);

    }

    add_test_queries(&ctx);

    void *status;
    pthread_join(pid, &status);

    // get query results
    process_result(&ctx);

    // destroy the enclave
    sgx_destroy_enclave(eid);
}

