//
// Created by shangqi on 20/2/20.
//

#ifndef MEASUREMENT_ENCLAVEFUNCTION_H
#define MEASUREMENT_ENCLAVEFUNCTION_H

#include "EnclaveUtil.h"
#include "Sketch/CMSketch.h"

void add_trace(CMSketch<4, 3> *sketch, unordered_map<string, float> &statistics, uint8_t *trace, int size);


#endif //MEASUREMENT_ENCLAVEFUNCTION_H
