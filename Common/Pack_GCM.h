//
// Implemented by EActor, GPL 3.0 license applied
//

#ifndef MEASUREMENT_PACK_GCM_H
#define MEASUREMENT_PACK_GCM_H

#include <string.h>
#include <stdlib.h>
#include "Node.h"

#ifdef __cplusplus
extern "C" {
#endif

#define GCM_KEY_SIZE	16

/**
\struct encryption context for AES GCM

\note I encrypt these variables together, do not move
*/
struct ctx_gcm_s {
    uint8_t key[GCM_KEY_SIZE];	///< encryption key
    uint8_t IV[SGX_AESGCM_IV_SIZE];	///< should be incremeted at both sides
};

void pack_gcm(uint8_t *dst, uint8_t *src, int size, struct ctx_gcm_s *ctx, uint8_t *mac);
void unpack_gcm(uint8_t *dst, uint8_t *src, int size, struct ctx_gcm_s *ctx, uint8_t *mac);
void alloc_gcm(struct ctx_gcm_s **ctx);
void free_gcm(struct ctx_gcm_s **ctx);
int pack_get_size_gcm();


#ifdef __cplusplus
}
#endif

#endif //MEASUREMENT_PACK_GCM_H
