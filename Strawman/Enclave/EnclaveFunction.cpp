//
// Created by shangqi on 20/2/20.
//

#include "EnclaveFunction.h"

void add_trace(CMSketch<FLOW_KEY_SIZE, SKETCH_HASH> *sketch, unordered_map<string, float> &statistics, uint8_t *trace, int size) {
    // remove the last statistics
    sketch->reset();
    statistics.clear();

    // add new info
    for(size_t i = 0; i < size; i += FLOW_ID_SIZE) {
        sketch->insert((uint8_t*)(trace + i));
        // insert into the statistics table
        string str((const char*)(trace + i), FLOW_KEY_SIZE);
        statistics[str] = 0;
    }

    // compute occurrences
    for(auto & it : statistics) {
        it.second = (float) sketch->query((uint8_t*) it.first.c_str()) * 100 / (size / FLOW_ID_SIZE);
    }
}

vector<pair<string, float>> query_heavy_hitter(unordered_map<string, float> const &statistics, int k) {
    vector<pair<string, float>> top_k(k);

    std::partial_sort_copy(statistics.begin(), statistics.end(),
            top_k.begin(), top_k.end(),
            [](pair<const string, float> const& l, pair<const string, float> const& r)
            {
                return l.second > r.second;
            });

    return top_k;
}

vector<string> query_heavy_change(unordered_map<string, float> &prev_statistics, unordered_map<string, float> &cur_statistics, float T) {
    vector<string> detected_flow;
    // scan the cur statistics
    for(auto & it : cur_statistics) {
        if(fabs(prev_statistics[it.first] - it.second) >= T) {
            detected_flow.push_back(it.first);
        }
    }
    return detected_flow;
}

vector<uint32_t> query_dist(CMSketch<FLOW_KEY_SIZE, SKETCH_HASH> *sketch, unordered_map<string, float> const &statistics) {
    vector<uint32_t> dist;
    // loop in the statistics to get the distribution info
    for(auto & it : statistics) {
        uint32_t val = sketch->query((uint8_t*) it.first.c_str());
        if(dist.size() < val + 1) {
            dist.resize(val + 1);
        }
        dist[val]++;
    }
    return dist;
}

float query_entropy(unordered_map<string, float> const &statistics) {
    float entropy = 0.0f;
    for(auto & it : statistics) {
        if(it.second != 0) {
            entropy += (it.second / 100) * log2(it.second / 100);
        } else {
            entropy += 0;
        }
    }
    return -entropy;
}