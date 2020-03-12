#include "PathORAM.h"
#include "../../../Common/CommonUtil.h"

PathORAM::PathORAM(int depth, uint8_t bucket_size, uint32_t block_size) {
    // assign basic parameters
    this->depth = depth;
    N = pow(2, depth + 1) - 1;
    Z = bucket_size;
    B = block_size;
    // initialise the position map
    position.reserve(get_block_count());
    // initialise the data store
    store = new Block*[N];
    // fill the blocks with dummy data
    for(int i = 0; i < N; i++) {
        store[i] = new Block[Z];
        for(int z = 0; z < Z; z++) {
            store[i][z].id = 1 << 31;
            store[i][z].block = new uint8_t[B];
            memset(store[i][z].block, 0, B);
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
    printf("ORAM Size: %f MB\n", N * Z * B / 1024.0 / 1024.0);
}

PathORAM::~PathORAM() {
    // clear the position map and the stash
    position.clear();
    stash.clear();
    // clear the tree
    clear();
    delete store;
}

void PathORAM::clear() {
    for(int i = 0; i < N; i++) {
        delete[] store[i];
    }
}

uint32_t PathORAM::get_block_count() {
    return N * Z;
}

uint32_t PathORAM::random_path() {
    uint32_t rand;
    RAND_bytes((uint8_t*) &rand, sizeof(uint32_t));
    return rand % (N / 2);
}

// fetch all the blocks on a path and put them into the stash
void PathORAM::fetch_path(uint32_t x) {
    for(int d = 0; d <= depth; d++) {
        // fetch the block in the path
        Block *bucket = store[get_bucket_on_path(x, d)];
        // put the block into the stash
        for(int z = 0; z < Z; z++) {
            if(bucket[z].id != 1 << 31) {
                // insert into the stash if the block is not a dummy block
                stash.insert({bucket[z].id, bucket[z].block});
            }
        }
    }
}

void PathORAM::write_path(uint32_t x) {
    for(int d = depth; d >= 0; d--) {
        // find all blocks in the stash that can be write into the path
        auto valid_blocks = get_intersect_blocks(x, d);
        // get the bucket and write the block as mush as it can
        Block *bucket = store[get_bucket_on_path(x, d)];
        for(int z = 0; z < min((int) valid_blocks.size(), (int) Z); z++) {
            bucket[z].id = valid_blocks[z];
            memcpy(bucket[z].block, stash[bucket[z].id], B);
            // the block is no longer needs to be stored in the stash
            stash.erase(bucket[z].id);
        }
        // fill the bucket with dummy blocks
        for(int z = valid_blocks.size(); z < Z; z++) {
            bucket[z].id = 1 << 31;
            memset(bucket[z].block, 0, B);
        }
    }
}

uint32_t PathORAM::get_bucket_on_path(uint32_t leaf, int p_depth) {
    leaf += N / 2;
    for(int d = depth; d >= p_depth + 1; d--) {
        leaf = (leaf + 1) / 2 - 1;
    }
    return leaf;
}

vector<uint32_t> PathORAM::get_intersect_blocks(uint32_t x, int p_depth) {
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

uint8_t* PathORAM::read_stash(int bid) {
    return stash[bid];
}

void PathORAM::write_stash(int bid, uint8_t *b) {
    stash[bid] = b;
}

void PathORAM::access(Op op, int bid, uint8_t *&b) {
    uint32_t x = position[bid];
    // randomise the path again
    position[bid] = random_path();
    // fetch the whole path into the stash
    fetch_path(x);
    // process the operation
    if(op == READ) {
        // read the block
        b = read_stash(bid);
    } else {
        // write to the stash
        write_stash(bid, b);
    }
    // write to the tree
    write_path(x);
}

uint8_t* PathORAM::read(int bid) {
    uint8_t *b;
    access(READ, bid, b);
    return b;
}

void PathORAM::write(int bid, uint8_t *b) {
    access(WRITE, bid, b);
}