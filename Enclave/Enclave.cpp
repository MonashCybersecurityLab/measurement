#include "sgx_trts.h"
#include "Enclave_t.h"

#include "EnclaveUtil.h"
#include "CMSketch/CMSketch.h"

using namespace std;

CMSketch<4, 3> *sketch = nullptr;

void ecall_init() {
    sketch = new CMSketch<4, 3>(600 * 1024);
    sketch->print_basic_info();
}