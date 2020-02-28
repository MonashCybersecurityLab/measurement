//
// Created by shangqi on 8/2/20.
//

#ifndef MEASUREMENT_COMMONUTIL_H
#define MEASUREMENT_COMMONUTIL_H

#include <stdlib.h>
#include <string.h>
#include <openssl/evp.h>
#include <openssl/rand.h>

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

#define FLOW_ID_SIZE sizeof(struct FIVE_TUPLE)

#define HEAVY_HITTER_SIZE 20

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
