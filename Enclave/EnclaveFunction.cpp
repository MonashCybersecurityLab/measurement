//
// Created by shangqi on 20/2/20.
//

#include "EnclaveFunction.h"

void add_trace(CMSketch<4, 3> *sketch, unordered_map<string, float> &statistics, uint8_t *trace, int size) {
    // remove the last statistics
    sketch->reset();
    statistics.clear();

    // add new info
    for(size_t i = 0; i < size; i += FLOW_ID_SIZE) {
        sketch->insert((uint8_t*)(trace + i));
        // insert into the statistics table
        string str((const char*)(trace + i), FLOW_ID_SIZE);
        statistics[str]++;
    }

    // compute occurrences
    for(auto & it : statistics) {
        it.second = it.second * 100 / (size / FLOW_ID_SIZE);
    }
}

vector<pair<string, float>> query_heavy_hitter(unordered_map<string, float> &statistics, int k) {
    vector<pair<string, float>> top_k(k);

    std::partial_sort_copy(statistics.begin(), statistics.end(),
            top_k.begin(), top_k.end(),
            [](pair<const string, float> const& l, pair<const string, float> const& r)
            {
                return l.second > r.second;
            });

    return top_k;
}