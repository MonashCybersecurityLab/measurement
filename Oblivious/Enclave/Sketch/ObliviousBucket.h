//
// Created by shangqi on 15/3/20.
//

#ifndef MEASUREMENT_OBLIVIOUSBUCKET_H
#define MEASUREMENT_OBLIVIOUSBUCKET_H

#include "../SpookyHash/SpookyV2.h"
#include "../../../Common/CommonUtil.h"

struct __attribute__((packed)) Bucket
{
    uint32_t key[COUNTER_PER_BUCKET];
    uint32_t val[COUNTER_PER_BUCKET];
};

template <int bucket_num>
class ObliviousBucket {

private:
    Bucket buckets[bucket_num];

    // oblivious insert into a bucket
    bool oinsert(uint8_t *key, uint8_t *swap_key, uint32_t &swap_value, uint32 count, int position) {
        // flag for success update
        bool success = false;
        // assign swap key
        *((uint32_t*) swap_key) = *((uint32_t*) key);
        swap_value = count;
        // retrieve the first value to scan for minimum
        uint32_t min_val = get_val(buckets[position].val[0]);
        for(int i = 0; i < COUNTER_PER_BUCKET - 1; i++) {
            // test whether the current bucket is empty
            bool empty = (buckets[position].key[i] == 0);
            // add the key into bucket if the bucket is empty and the flow id has not been inserted
            buckets[position].key[i] |= *((uint32_t*) key) & bool_extend(empty && !success);
            // test whether the current bucket has the corresponding key
            bool found = (buckets[position].key[i] == *((uint32_t*) key));
            // if the current bucket has the insert key, update the bucket val
            buckets[position].val[i] += bool_extend(found) & count;
            // update min_val
            min_val = get_min(min_val, get_val(buckets[position].val[i]));
            // the success flag will be flipped if the bucket id is correct and either the empty bucket is founded or
            // the id is founded
            success |= (empty || found);
        }
        // accumulate negative count if failed to accumulate counts
        buckets[position].val[COUNTER_PER_BUCKET - 1] += count & bool_extend(!success);
        // test whether the bucket will be evicted
        bool swap = !success & swap_threshold(buckets[position].val[COUNTER_PER_BUCKET - 1], min_val);
        // copy the swap key/value to the buffer
        bool swap_success = false;
        for(int i = 0; i < COUNTER_PER_BUCKET - 1; i++) {
            bool target = (min_val == get_val(buckets[position].val[i]));
            // update swap value
            *((uint32_t*) swap_key) = selector(buckets[position].key[i], *((uint32_t*) swap_key), (!swap_success && swap && target));
            swap_value = selector(min_val, swap_value, (!swap_success && swap && target));
            // update the bucket
            buckets[position].key[i] = selector(*((uint32_t*) key), buckets[position].key[i], (!swap_success && swap && target));
            buckets[position].val[i] = selector((0x80000000 ^ count), buckets[position].val[i], (!swap_success && swap && target));
            swap_success |= (swap && target);
        }
        // reset negative count after swap success
        buckets[position].val[COUNTER_PER_BUCKET - 1] = selector(0, buckets[position].val[COUNTER_PER_BUCKET - 1], swap_success);
        return success;
    }

    uint32_t oscan(uint8_t *key, int position) {
        uint32_t result = 0;
        for(int i = 0; i < COUNTER_PER_BUCKET - 1; i++) {
            result = selector(buckets[position].val[i], result, (buckets[position].key[i] == *((uint32_t*) key)));
        }
        return result;
    }

public:
    ObliviousBucket() {
        clear();
    }

    void clear()
    {
        memset(buckets, 0, sizeof(Bucket) * bucket_num);
    }

    bool insert(uint8_t *key, uint8_t *swap_key, uint32_t &swap_value, uint32_t count = 1) {
        int position = SpookyHash::Hash32(key, FLOW_KEY_SIZE, 0) % bucket_num;
        // insert the flow obliviously
        return oinsert(key, swap_key, swap_value, count, position);
    }

    uint32_t query(uint8_t *key) {
        int position = SpookyHash::Hash32(key, FLOW_KEY_SIZE, 0) % bucket_num;
        // scan the bucket
        return oscan(key, position);
    }

    void get_distribution(vector<uint32_t> dist) {

    }

    int get_cardinality() {
        int card = 0;
        for(int i = 0; i < BUCKET_NUM; i++) {
            for(int j = 0; j < COUNTER_PER_BUCKET - 1; j++) {
                card += selector(1, 0, (buckets[i].key[j] != 0 && !get_flag(buckets[i].val[j])));
            }
        }
        return card;
    }

    unordered_map<uint32_t, uint32_t> get_count_map() {
        unordered_map<uint32_t, uint32_t> result_map;

        for(int i = 0; i < BUCKET_NUM; i++) {
            for(int j = 0; j < COUNTER_PER_BUCKET - 1; j++) {
                result_map[buckets[i].key[j]] = get_val(buckets[i].val[j]);
            }
        }
        return result_map;
    }

    unordered_map<uint32_t, float> get_stat_map(int total) {
        unordered_map<uint32_t, float> result_map;

        for(int i = 0; i < BUCKET_NUM; i++) {
            for(int j = 0; j < COUNTER_PER_BUCKET - 1; j++) {
                result_map[buckets[i].key[j]] = (float) get_val(buckets[i].val[j]) * 100 / total;
            }
        }
        return result_map;
    }
};

#endif //MEASUREMENT_OBLIVIOUSBUCKET_H
