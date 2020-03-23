//
// Created by shangqi on 24/1/20.
//

#ifndef MEASUREMENT_CMSKETCH_H
#define MEASUREMENT_CMSKETCH_H

#include <algorithm>
#include <sstream>

#include "../SpookyHash/SpookyV2.h"

using namespace std;

#define SKETCH_HASH 3

template <int key_len, int d>
class CMSketch {
private:
    int memory_in_bytes = 0;

    int w = 0;
    uint32_t *counters[d] = {nullptr};

public:
    CMSketch() = default;

    explicit CMSketch(int bytes) {
        initial(bytes);
    }

    ~CMSketch() {
        clear();
    }

    void initial(int bytes) {
        this->memory_in_bytes = bytes;
        w = memory_in_bytes / d / sizeof(uint32_t);

        for(int i = 0; i < d; i++) {
            counters[i] = new uint32_t[w];
            memset(counters[i], 0, w * sizeof(uint32_t));
        }
    }

    void clear() {
        for(int i = 0; i < d; i++) {
            delete[] counters[i];
        }
    }

    void reset() {
        for(int i = 0; i < d; i++) {
            memset(counters[i], 0, w);
        }
    }

    void print_basic_info()
    {
        printf("CM sketch\n");
        printf("\tCounters: %d\n", w);
        printf("\tMemory: %.6lfMB\n", w * sizeof(uint32_t) / 1024 / 1024.0);
    }

    void insert(uint8_t * key, uint32_t count = 1)
    {
        for (int i = 0; i < d; i++) {
            uint32_t index = (SpookyHash::Hash32(key, key_len, i)) % w;
            counters[i][index] += count;
        }
    }

    uint32_t query(uint8_t * key)
    {
        uint32_t ret = 0xFFFFFFFF;
        for (int i = 0; i < d; i++) {
            uint32_t index = (SpookyHash::Hash32(key, key_len, i)) % w;
            uint32_t tmp = counters[i][index];
            ret = min(ret, tmp);
        }
        return ret;
    }
};

#endif //MEASUREMENT_CMSKETCH_H
