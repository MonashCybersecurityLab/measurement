#include <iostream>
#include <vector>

#include "sgx_urts.h"
#include "../Enclave_u.h"

#define ENCLAVE_FILE "Enclave.signed.so"

using namespace std;

#define START_FILE_NO 1
#define END_FILE_NO 2

typedef vector<FIVE_TUPLE> TRACE;
TRACE traces[END_FILE_NO - START_FILE_NO + 1];

void ReadInTraces(const char *trace_prefix)
{
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


void ocall_print_string(const char *str) {
    printf("%s\n", str);
}

int main() {
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

    // initialise the enclave with message queues
    ecall_init(eid);

    // ecall in another thread

    // read offline data
    ReadInTraces("data/");

    // add the test data into the queue

    // add the data into sketch
    for(int datafileCnt = START_FILE_NO; datafileCnt <= END_FILE_NO; ++datafileCnt) {
        ecall_add_trace(eid, traces[datafileCnt - 1].data(), traces[datafileCnt - 1].size());
    }

    // add query into the queue

    // destroy the enclave
    sgx_destroy_enclave(eid);
}

