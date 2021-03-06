//
// Created by shangqi on 20/2/20.
//

#ifndef MEASUREMENT_ENCLAVEFUNCTION_H
#define MEASUREMENT_ENCLAVEFUNCTION_H

#include <algorithm>
#include <math.h>
#include <numeric>
#include <unordered_map>
#include <vector>

#include "EnclaveUtil.h"
#include "Sketch/CMSketch.h"
#include "Sketch/ObliviousBucket.h"

using namespace std;

void add_trace(ObliviousBucket<BUCKET_NUM> *bucket, CMSketch<FLOW_KEY_SIZE, SKETCH_HASH> *sketch, uint8_t *trace, int size);

vector<pair<uint32_t, uint32_t>> query_heavy_hitter(ObliviousBucket<BUCKET_NUM> *bucket, int k);

vector<uint32_t> query_heavy_change(ObliviousBucket<BUCKET_NUM> *prev_bucket, int prev_total, ObliviousBucket<BUCKET_NUM> *cur_bucket, int cur_total, float T);

vector<uint32_t> query_dist(ObliviousBucket<BUCKET_NUM> *bucket, CMSketch<FLOW_KEY_SIZE, SKETCH_HASH> *sketch);

float query_entropy(ObliviousBucket<BUCKET_NUM> *bucket, CMSketch<FLOW_KEY_SIZE, SKETCH_HASH> *sketch);


#endif //MEASUREMENT_ENCLAVEFUNCTION_H
