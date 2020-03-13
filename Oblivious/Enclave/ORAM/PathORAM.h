#ifndef MEASUREMENT_PATHORAM_H
#define MEASUREMENT_PATHORAM_H

#include <algorithm>
#include <cmath>
#include <unordered_map>
#include <vector>

#include "../../../Common/CommonUtil.h"

//#define ORAM_DEBUG_PRINT

using namespace std;

template <uint32_t B>
class PathORAM {
private:
    struct __attribute__((packed)) Block {
        uint32_t id;
        uint8_t block[B];
    };

    // basic settings
    uint32_t depth;         // the depth of the tree
    uint8_t Z;              // # of blocks per bucket
    uint32_t N;             // # of buckets

    Block *store;         // a bucket list
    uint32_t *position;     // position map
    unordered_map<uint32_t, uint8_t*> stash;

    // operators on the tree
    uint32_t random_path() {
        // assign a random path to the leaf
        // note that the leaf id is between [0, N/2]
        uint32_t rand;
        RAND_bytes((uint8_t*) &rand, sizeof(uint32_t));
        return rand % (N / 2);
    }

    // fetch all the blocks on a path and put them into the stash
    void fetch_path(uint32_t x) {
        for(int d = 0; d <= depth; d++) {
            // fetch the block in the path
            Block *bucket = store + get_bucket_on_path(x, d) * Z;
            // put the block into the stash
            for(int z = 0; z < Z; z++) {
                if(bucket[z].id != 1 << 31) {
                    // insert into the stash if the block is not a dummy block
                    stash[bucket[z].id] = (uint8_t*) malloc(B);
                    memcpy(stash[bucket[z].id], bucket[z].block, B);
                }
            }
        }
    }

    void write_path(uint32_t x) {
        for(int d = depth; d >= 0; d--) {
            // find all blocks in the stash that can be write into the path
            auto valid_blocks = get_intersect_blocks(x, d);
            // get the bucket and write the block as mush as it can
            Block *bucket = store + get_bucket_on_path(x, d) * Z;
            for(int z = 0; z < min((int) valid_blocks.size(), (int) Z); z++) {
                bucket[z].id = valid_blocks[z];
                memcpy(bucket[z].block, stash[bucket[z].id], B);
                // the block is no longer needs to be stored in the stash
                free(stash[bucket[z].id]);
                stash.erase(bucket[z].id);
            }
            // fill the bucket with dummy blocks
            for(int z = valid_blocks.size(); z < Z; z++) {
                bucket[z].id = 1 << 31;
                memset(bucket[z].block, 0, B);
            }
        }
    }

    uint32_t get_bucket_on_path(uint32_t leaf, int p_depth) {
        leaf += N / 2;  // add the offset to the leaf id in order to convert it to the real bucket id
        for(int d = depth; d >= p_depth + 1; d--) {     // find the parent bucket iteratively
            leaf = (leaf + 1) / 2 - 1;
        }
        return leaf;
    }

    // operators on stash
    vector<uint32_t> get_intersect_blocks(uint32_t x, int p_depth) {    // retrieve the blocks in the given path and depths
        vector<uint32_t> valid_blocks;
        // find the bucket id
        uint32_t bucket_id = get_bucket_on_path(x, p_depth);
        // scan the stash and retrieve all the blocks that can be put into the bucket
        for(auto b : stash) {
            uint32_t bid = b.first;
            if(get_bucket_on_path(position[bid], p_depth) == bucket_id) {
                valid_blocks.push_back(bid);
            }
        }
        return valid_blocks;
    }


    void read_stash(int bid, uint8_t *b) {
        if(stash.find(bid) == stash.end()) {
            memset(b, 0, B);
        } else {
            memcpy(b, stash[bid], B);
        }
    }

    void write_stash(int bid, uint8_t *b) {
        stash[bid] = b;
    }

    enum Op {
        READ,
        WRITE
    };

    void access(Op op, int bid, uint8_t *&b) {
        uint32_t x = position[bid];
        // randomise the path again
        position[bid] = random_path();
        // fetch the whole path into the stash
        fetch_path(x);
        // process the operation
        if(op == READ) {
            // read the block
            read_stash(bid, b);
        } else {
            // write to the stash
            write_stash(bid, b);
        }
        // write to the tree
        write_path(x);
    }

public:
    PathORAM(int depth, uint8_t bucket_size) {
        // assign basic parameters
        this->depth = depth;
        N = pow(2, depth + 1) - 1;
        Z = bucket_size;
        // initialise the position map
        position = new uint32_t[get_block_count()];
        // initialise the data store
        store = new Block[get_block_count()];
        // fill the blocks with dummy data
        for(int i = 0; i < N; i++) {
            for(int z = 0; z < Z; z++) {
                store[i * Z + z].id = 0xFFFFFFFF;
                memset(store[i * Z + z].block, 0, B);
            }
        }
        // randomise the path
        for(int i = 0; i < get_block_count(); i++) {
            position[i] = random_path();
#ifdef ORAM_DEBUG_PRINT
            printf("New Random Position: %d\n", position[i]);
#endif
        }
        // clear the stash
        stash.clear();
    }

    ~PathORAM() {
        // clear the position map and the stash
        clear();

        // remove data
        delete position;
        delete store;
    }

    void clear() {
        stash.clear();
        // refill positions
        for(int i = 0; i < get_block_count(); i++) {
            position[i] = random_path();
        }
    }

    uint32_t get_block_count() {
        return N * Z;
    }

    uint8_t* read(int bid) {
        auto *b = (uint8_t*) malloc(B);
        access(READ, bid, b);
        return b;
    }

    void write(int bid, uint8_t *b) {
        access(WRITE, bid, b);
    }
};


#endif //MEASUREMENT_PATHORAM_H
