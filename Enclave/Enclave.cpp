#include "EnclaveUtil.h"
#include "CMSketch/CMSketch.h"

using namespace std;

CMSketch<4, 3> *sketch = nullptr;

unordered_map<string, float> statistics;

void ecall_init() {
    sketch = new CMSketch<4, 3>(600 * 1024);
    sketch->print_basic_info();
}

void ecall_add_trace(struct FIVE_TUPLE *trace, size_t trace_size) {
    printf("%d packets received\n", trace_size);
    for(auto i = 0; i < trace_size; i++) {
        sketch->insert((uint8_t*)(trace[i].key));

        string str((const char*)(trace[i].key), sizeof(struct FIVE_TUPLE));
        statistics[str] = 0;

    }
}