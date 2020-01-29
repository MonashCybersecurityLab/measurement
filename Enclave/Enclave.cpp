#include "EnclaveUtil.h"
#include "Sketch/CMSketch.h"

using namespace std;

CMSketch<4, 3> *sketch = nullptr;

unordered_map<string, float> prep_statistics;
unordered_map<string, float> cur_statistics;


void ecall_init() {
    sketch = new CMSketch<4, 3>(600 * 1024);
    sketch->print_basic_info();
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