#ifndef MEASUREMENT_PATHORAM_H
#define MEASUREMENT_PATHORAM_H

#include <algorithm>
#include <cmath>
#include <unordered_map>
#include <vector>

#include "../../../Common/CommonUtil.h"

//#define ORAM_DEBUG_PRINT

using namespace std;

struct Block {
    uint32_t id;
    uint8_t *block;
};

class PathORAM {
private:
    // basic settings
    uint32_t depth;         // the depth of the tree
    uint8_t Z;              // # of blocks per bucket
    uint32_t B;             // block size
    uint32_t N;             // # of buckets

    Block **store;         // a bucket list
    unordered_map<uint32_t, uint32_t> position;     // position map
    unordered_map<uint32_t, uint8_t*> stash;

    // operators on the tree
    uint32_t random_path();
    void fetch_path(uint32_t x);
    void write_path(uint32_t x);
    uint32_t get_bucket_on_path(uint32_t leaf, int p_depth);

    // operators on stash
    vector<uint32_t> get_intersect_blocks(uint32_t x, int p_depth);   // retrieve the blocks in the given path and depths
    void read_stash(int bid, uint8_t *b);
    void write_stash(int bid, uint8_t *b);

    enum Op {
        READ,
        WRITE
    };

    void access(Op op, int bid, uint8_t *&b);

public:
    PathORAM(int depth, uint8_t bucket_size, uint32_t block_size);
    ~PathORAM();

    void clear();

    uint32_t get_block_count();

    uint8_t* read(int bid);
    void write(int bid, uint8_t *b);
};


#endif //MEASUREMENT_PATHORAM_H
