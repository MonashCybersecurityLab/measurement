#include <iostream>

#include "sgx_urts.h"
#include "Enclave_u.h"

#define ENCLAVE_FILE "Enclave.signed.so"

using namespace std;

void ocall_print_string(const char *str) {
    printf("%s\n", str);
}

int main() {
    /* Setup enclave */
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

    ecall_test(eid);
}