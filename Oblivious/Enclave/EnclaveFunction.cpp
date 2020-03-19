//
// Created by shangqi on 20/2/20.
//

#include "EnclaveFunction.h"

void add_trace(ObliviousBucket<BUCKET_NUM> *bucket, CMSketch<FLOW_KEY_SIZE, SKETCH_HASH> *sketch, unordered_map<string, float> &statistics, uint8_t *trace, int size) {
    // remove the last statistics
    sketch->reset();
    statistics.clear();
    // add new info
    for(size_t i = 0; i < size; i += FLOW_ID_SIZE) {
        // dummy data block
        uint8_t swap_key[FLOW_KEY_SIZE];
        uint32_t swap_value = 0;
        memcpy(swap_key, (uint8_t*)(trace + i), FLOW_KEY_SIZE);
        // insert into the bucket; insert into sketch if failed in bucket
        bucket->insert((uint8_t*)(trace + i), swap_key, swap_value);
        sketch->insert(swap_key, swap_value);
        // insert into the statistics table
        string str((const char*)(trace + i), FLOW_ID_SIZE);
        statistics[str]++;
    }
    // compute occurrences
    for(auto & it : statistics) {
        it.second = it.second * 100 / (size / FLOW_ID_SIZE);
    }
}

vector<pair<uint32_t, uint32_t>> query_heavy_hitter(ObliviousBucket<BUCKET_NUM> *bucket, int k) {
    vector<pair<uint32_t, uint32_t>> top_k(k);

    unordered_map<uint32_t, uint32_t> serialised_bucket = bucket->get_map();

    std::partial_sort_copy(serialised_bucket.begin(), serialised_bucket.end(),
            top_k.begin(), top_k.end(),
            [](pair<const uint32_t, uint32_t> const& l, pair<const uint32_t, uint32_t> const& r)
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

void query_dist(CMSketch<FLOW_KEY_SIZE, SKETCH_HASH> *sketch, uint32_t *dist) {
    // reset the dist array
    memset(dist, 0, 256 * sizeof(uint32_t));
    // loop in the statistics to get the distribution info
    sketch->dist(dist);
}

float query_entropy(unordered_map<string, float> const &statistics) {
    float entropy = 0.0f;
    for(auto & it : statistics) {
        entropy += it.second * log2(it.second);
    }
    return -entropy;
}