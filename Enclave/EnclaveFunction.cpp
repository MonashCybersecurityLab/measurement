//
// Created by shangqi on 20/2/20.
//

#include "EnclaveFunction.h"

void add_trace(CMSketch<4, 3> *sketch, unordered_map<string, float> &statistics, uint8_t *trace, int size) {
    // remove the last statistics
    sketch->reset();
    statistics.clear();

    // add new info
    for(size_t i = 0; i < size; i += 13) {
        sketch->insert((uint8_t*)(trace + i));
        // insert into the statistics table
        string str((const char*)(trace + i), 4);
        statistics[str]++;
    }

    // compute occurrences
    for(auto & it : statistics) {
        it.second = it.second * 100 / (size / 13);
    }
}