//
// Implemented by EActor, GPL 3.0 license applied
//

#ifndef MEASUREMENT_PACK_MEMCPY_H
#define MEASUREMENT_PACK_MEMCPY_H

#include <string.h>
#include <stdlib.h>

#include "Node.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ctx_memcpy_s {
    uint8_t key[16]; /// <for compatibility
};


void pack_memcpy(uint8_t *dst, uint8_t *src, int);
void unpack_memcpy(uint8_t *dst, uint8_t *src, int);
void alloc_memcpy(struct ctx_memcpy_s **ctx);
int pack_get_size_memcpy();

#ifdef __cplusplus
}
#endif


#endif //MEASUREMENT_PACK_MEMCPY_H
