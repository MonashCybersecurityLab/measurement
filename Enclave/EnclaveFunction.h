//
// Created by shangqi on 20/2/20.
//

#ifndef MEASUREMENT_ENCLAVEFUNCTION_H
#define MEASUREMENT_ENCLAVEFUNCTION_H

#include <algorithm>
#include <math.h>
#include <unordered_map>
#include <vector>

#include "EnclaveUtil.h"
#include "Sketch/CMSketch.h"

using namespace std;

void add_trace(CMSketch<4, 3> *sketch, unordered_map<string, float> &statistics, uint8_t *trace, int size);

vector<pair<string, float>> query_heavy_hitter(unordered_map<string, float> const &statistics, int k);

float query_entropy(unordered_map<string, float> const &statistics);


#endif //MEASUREMENT_ENCLAVEFUNCTION_H
