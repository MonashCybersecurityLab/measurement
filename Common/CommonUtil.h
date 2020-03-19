//
// Created by shangqi on 8/2/20.
//

#ifndef MEASUREMENT_COMMONUTIL_H
#define MEASUREMENT_COMMONUTIL_H

#include <stdlib.h>
#include <string.h>
#include <openssl/evp.h>
#include <openssl/rand.h>

#define MAX_POOL_SIZE 16384

#define ORAM_DATA_SIZE 1
#define ORAM_BUCKET_SIZE 5
#define ORAM_STASH_SIZE 105  // fp rate 2^(-128) accroding to the PathORAM paper

#define GCM_KEY_SIZE	16
#define GCM_IV_SIZE     12
#define GCM_MAC_SIZE    16

struct FIVE_TUPLE {
    char key[13];
};

struct FLOW_KEY {   // 13 bytes
    // 8 (4*2) bytes
    uint32_t src_ip;  // source IP address
    uint32_t dst_ip;
    // 4 (2*2) bytes
    uint16_t src_port;
    uint16_t dst_port;
    // 1 bytes
    uint8_t proto;
};

#define COUNTER_PER_BUCKET 8

#define bool_extend(val) (-(val) >> 32)
#define get_min(x, y) ((uint32_t) y & bool_extend(x > y)) | ((uint32_t) x & bool_extend(x <= y))
#define selector(x, y, bit) ((uint32_t) x & bool_extend(bit)) | ((uint32_t) y & bool_extend(!bit))
#define swap_threshold(negative_val, val) (negative_val > (val << 3))
#define get_flag(val) (((uint32_t)(val) & 0x80000000) == 0x80000000)
#define get_val(val) ((uint32_t)((val) & 0x7FFFFFFF))

// sketch definitions
#define TOTAL_MEM 600 * 1024    // 600 KB
#define BUCKET_MEM (150 * 1024)
#define BUCKET_NUM (BUCKET_MEM / 64)
#define FLOW_KEY_SIZE 4
#define SKETCH_HASH 1

#define FLOW_ID_SIZE sizeof(struct FIVE_TUPLE)
#define HEAVY_HITTER_SIZE 20
#define HEAVY_CHANGE_THRESHOLD 0.05

#ifdef __cplusplus
extern "C" {
#endif

struct ctx_gcm_s {
    unsigned char key[GCM_KEY_SIZE];	///< encryption key
    unsigned char IV[GCM_IV_SIZE];	///< should be incremeted at both sides
};

void incr_ctr(char *ctr, int size);

int gcm_encrypt(unsigned char *plaintext, int plaintext_len,
                unsigned char *key, unsigned char *iv,
                unsigned char *ciphertext,
                unsigned char *mac);

int gcm_decrypt(unsigned char *ciphertext, int ciphertext_len,
                unsigned char *mac,
                unsigned char *key, unsigned char *iv,
                unsigned char *plaintext);

void alloc_gcm(struct ctx_gcm_s *ctx);

#ifdef __cplusplus
}
#endif

#endif //MEASUREMENT_COMMONUTIL_H
