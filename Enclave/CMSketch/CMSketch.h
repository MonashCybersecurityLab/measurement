//
// Created by shangqi on 24/1/20.
//

#ifndef MEASUREMENT_CMSKETCH_H
#define MEASUREMENT_CMSKETCH_H

#include <algorithm>
#include <sstream>
#include <cstring>

#include "../SpookyHash/SpookyV2.h"

using namespace std;

template <int key_len, int d>
class CMSketch {
private:
    int memory_in_bytes = 0;

    int w = 0;
    int *counters[d] = {nullptr};

public:
    CMSketch() {}

    CMSketch(int bytes) {
        initial(bytes);
    }

    ~CMSketch() {

    }

    void initial(int bytes) {
        this->memory_in_bytes = bytes;
        w = memory_in_bytes / 4 / d;

        for(int i = 0; i < d; i++) {
            counters[i] = new int[w];
            memset(counters[i], 0, 4 * w);
        }
    }

    void clear() {
        for(int i = 0; i < d; i++) {
            delete[] counters[i];
        }
    }

    void print_basic_info()
    {
        printf("CM sketch\n");
        printf("\tCounters: %d\n", w);
        printf("\tMemory: %.6lfMB\n", w * 4.0 / 1024 / 1024);
    }

    void insert(uint8_t * key, int count = 1)
    {
        for (int i = 0; i < d; i++) {
            int index = (SpookyHash::Hash32(key, key_len, i)) % w;
            counters[i][index] += count;
        }
    }

    int query(uint8_t * key)
    {
        int ret = 1 << 30;
        for (int i = 0; i < d; i++) {
            int index = (SpookyHash::Hash32(key, key_len, i)) % w;
            int tmp = counters[i][index];
            ret = min(ret, tmp);
        }
        return ret;
    }

};

#endif //MEASUREMENT_CMSKETCH_H
