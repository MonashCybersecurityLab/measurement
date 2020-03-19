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

    unordered_map<uint32_t, uint32_t> serialised_bucket = bucket->get_count_map();

    std::partial_sort_copy(serialised_bucket.begin(), serialised_bucket.end(),
            top_k.begin(), top_k.end(),
            [](pair<const uint32_t, uint32_t> const& l, pair<const uint32_t, uint32_t> const& r)
            {
                return l.second > r.second;
            });

    return top_k;
}

vector<uint32_t> query_heavy_change(ObliviousBucket<BUCKET_NUM> *prev_bucket, int prev_total, ObliviousBucket<BUCKET_NUM> *cur_bucket, int cur_total, float T) {
    vector<uint32_t> detected_flow;
    // scan the cur statistics
    unordered_map<uint32_t, float> prev_statistics = prev_bucket->get_stat_map(prev_total);
    unordered_map<uint32_t, float> cur_statistics = cur_bucket->get_stat_map(cur_total);

    for(auto & it : cur_statistics) {
        if(fabs(prev_statistics[it.first] - it.second) >= T) {
            detected_flow.push_back(it.first);
        }
    }
    return detected_flow;
}

vector<uint32_t> query_dist(ObliviousBucket<BUCKET_NUM> *bucket, CMSketch<FLOW_KEY_SIZE, SKETCH_HASH> *sketch) {
    // reset the dist array
    uint32 small_dist[256];
    memset(small_dist, 0, 256 * sizeof(uint32_t));
    // loop in the statistics to get the distribution info
    sketch->get_dist(small_dist);
    // update the distribution based on bucket
    vector<uint32_t> dist(small_dist, small_dist + 256);
    bucket->get_distribution(dist, sketch);
    return dist;

}

float query_entropy(ObliviousBucket<BUCKET_NUM> *bucket, CMSketch<FLOW_KEY_SIZE, SKETCH_HASH> *sketch) {
    // get distribution
    vector<uint32_t> dist = query_dist(bucket, sketch);
    // compute the sum
    int sum = 0;
    for(int i = 0; i < dist.size(); i++) {
        sum += dist[i];
    }
    // compute entropy based on the distribution and sum
    float entropy = 0.0f;
    for(int i = 0; i < dist.size(); i++) {
        if(dist[i] != 0) {
            entropy += ((float) dist[i] / sum) *log2((float) dist[i]/sum);
        } else {
            entropy += 0;
        }
        if(isnan(entropy)) {
            printf("%d\n",i);
        }
    }
    return -entropy;
}