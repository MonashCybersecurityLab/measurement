#ifndef MEASUREMENT_PATHORAM_H
#define MEASUREMENT_PATHORAM_H

#include <algorithm>
#include <cmath>
#include <unordered_map>
#include <vector>

#include "Stash.h"

//#define ORAM_DEBUG_PRINT

using namespace std;

class PathORAM {
private:
    // basic settings
    uint32_t depth;         // the depth of the tree
    uint8_t Z;              // # of blocks per bucket
    uint32_t N;             // # of buckets
    uint32_t B;             // size of data;

    Block *store;           // a bucket list
    uint32_t *position;     // position map
    Stash *stash;            // stash

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
                if(bucket[z].id != 0xFFFFFFFF) {
                    // insert into the stash if the block is not a dummy block
                    stash->insert(bucket[z]);
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
                bucket[z].id = valid_blocks[z]->b.id;
                memcpy(bucket[z].block, valid_blocks[z]->b.block, B);
                // the block is no longer needs to be stored in the stash
                stash->erase(valid_blocks[z]);
            }
            // fill the bucket with dummy blocks
            for(int z = valid_blocks.size(); z < Z; z++) {
                bucket[z].id = 0xFFFFFFFF;
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
    vector<Node*> get_intersect_blocks(uint32_t x, int p_depth) {    // retrieve the blocks in the given path and depths
        vector<Node*> valid_blocks;
        // find the bucket id
        uint32_t bucket_id = get_bucket_on_path(x, p_depth);
        // scan the stash and retrieve all the blocks that can be put into the bucket
        Node* iter = stash->get_start();
        while (iter != nullptr) {
            uint32_t bid = iter->b.id;
            if(get_bucket_on_path(position[bid], p_depth) == bucket_id) {
                valid_blocks.push_back(iter);
            }
            iter = iter->prev;
        }
        return valid_blocks;
    }


    void read_stash(int bid, uint8_t *b) {
        Node *retrieve_block = stash->search(bid);
        if(retrieve_block == nullptr) {
            memset(b, 0, B);
        } else {
            memcpy(b, retrieve_block->b.block, B);
        }
    }

    void write_stash(int bid, uint8_t *b) {
        Block new_block;
        new_block.id = bid;
        memcpy(new_block.block, b, B);
        stash->insert(new_block);
    }

    enum Op {
        READ,
        WRITE
    };

    void access(Op op, int bid, uint8_t *b) {
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
    PathORAM(int depth, uint8_t bucket_size, uint32_t data_size) {
        // assign basic parameters
        this->depth = depth;
        N = pow(2, depth + 1) - 1;
        Z = bucket_size;
        B = data_size;
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
        stash = new Stash(ORAM_STASH_SIZE);
    }

    ~PathORAM() {
        // clear the position map and the stash
        clear();
        // remove data
        delete position;
        delete store;
    }

    void clear() {
        stash->clear();
        // refill positions
        for(int i = 0; i < get_block_count(); i++) {
            position[i] = random_path();
        }
        // reset the store
        for(int i = 0; i < N; i++) {
            for(int z = 0; z < Z; z++) {
                store[i * Z + z].id = 0xFFFFFFFF;
                memset(store[i * Z + z].block, 0, B);
            }
        }
    }

    uint32_t get_block_count() {
        return N * Z;
    }

    void read(int bid, uint8_t *b) {
        access(READ, bid, b);
    }

    void write(int bid, uint8_t *b) {
        access(WRITE, bid, b);
    }
};


#endif //MEASUREMENT_PATHORAM_H
