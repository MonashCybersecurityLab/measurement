#include "Pack_memcpy.h"
/***** MEMCPY *****/

/**
\brief a memcpy-based pack function
\param[out] *dst where 'encrypted' data will be stored
\param[in] *src data to encrypt
\param[in] size a size of a data to encrypt
\param[in] *ctx an encryption context (unused)
\param[in] *mac a mac (unused)
*/
void pack_memcpy(uint8_t *dst, uint8_t *src, int size) {
    memcpy(dst, src, size);
}

/**
\brief a memcpy-based unpack function
\param[out] *dst where 'decrypted' data will be stored
\param[in] *src data to decrypt
\param[in] size a size of a data to decrypt
\param[in] *ctx an decryption context (unsided)
\param[in] *mac a mac (unsided)
*/
void unpack_memcpy(uint8_t *dst, uint8_t *src, int size) {
    memcpy(dst, src, size);
}

/**
\brief memory allocation for the context
\param[in] **ctx where to allocate
*/
void alloc_memcpy(struct ctx_memcpy_s **ctx) {
    *ctx = (struct ctx_memcpy_s *) malloc(sizeof(struct ctx_memcpy_s));
    if(*ctx == NULL) {
       printf("[%d] malloc failed, die\n",__LINE__);while(1);
    }

    memset(*ctx, 0, sizeof(struct ctx_memcpy_s));
}

/**
\brief get size of the encryption context
\return size of the unused key
*/
int pack_get_size_memcpy() {
    return 16;
}
