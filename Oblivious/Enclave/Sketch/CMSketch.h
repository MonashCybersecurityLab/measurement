//
// Created by shangqi on 24/1/20.
//

#ifndef MEASUREMENT_CMSKETCH_H
#define MEASUREMENT_CMSKETCH_H

#include "../ORAM/PathORAM.h"
#include "../SpookyHash/SpookyV2.h"

using namespace std;

template <int key_len, int d>
class CMSketch {
private:
    int memory_in_bytes = 0;

    int w = 0;
    PathORAM *counters;

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
        // initialise the oram
        int num_of_bucket = memory_in_bytes / ORAM_DATA_SIZE / ORAM_BUCKET_SIZE;
        int oram_depth = ceil(log2(num_of_bucket));
        counters = new PathORAM(oram_depth, ORAM_BUCKET_SIZE, ORAM_DATA_SIZE);

        w = memory_in_bytes / d;
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
        printf("\tMemory: %.6lfMB\n", w / 1024 / 1024.0);
    }

    void insert(uint8_t * key, uint8_t count = 1)
    {
        for (int i = 0; i < d; i++) {
            uint32_t index = (SpookyHash::Hash32(key, key_len, i)) % w;
            uint8_t *value = counters->read(index);
            (*value) += count;
            counters->write(index, value);
            //counters[i][index] += count;
        }
    }

    uint8_t query(uint8_t * key)
    {
        uint8_t ret = 255;
        for (int i = 0; i < d; i++) {
            uint32_t index = (SpookyHash::Hash32(key, key_len, i)) % w;
            uint8_t tmp = *(counters->read(index));
            ret = min(ret, tmp);
        }
        return ret;
    }
};

#endif //MEASUREMENT_CMSKETCH_H
