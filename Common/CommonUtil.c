//
// Created by shangqi on 8/2/20.
//

#include "CommonUtil.h"

void incr_ctr(char *ctr, int size) {
    int i;
    for(i = 0; i < size; i++) {
        if ( ++ctr[i] != 0)
            return;
    }
}

int gcm_encrypt(unsigned char *plaintext, int plaintext_len,
                unsigned char *key, unsigned char *iv,
                unsigned char *ciphertext,
                unsigned char *mac)
{
    EVP_CIPHER_CTX *ctx;

    int len = 0;

    int ciphertext_len = 0;

    /* Create and initialise the context */
    ctx = EVP_CIPHER_CTX_new();

    /* Initialise the encryption operation. */
    EVP_EncryptInit_ex(ctx, EVP_aes_128_gcm(), NULL, NULL, NULL);

    /*
     * Set IV length if default 12 bytes (96 bits) is not appropriate
     */
    //EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, iv_len, NULL);

    /* Initialise key and IV */
    EVP_EncryptInit_ex(ctx, NULL, NULL, key, iv);

    /*
     * Provide any AAD data. This can be called zero or more times as
     * required
     */

    // EVP_EncryptUpdate(ctx, NULL, &len, aad, aad_len);

    /*
     * Provide the message to be encrypted, and obtain the encrypted output.
     * EVP_EncryptUpdate can be called multiple times if necessary
     */
    EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, plaintext_len);
    ciphertext_len = len;

    /*
     * Finalise the encryption. Normally ciphertext bytes may be written at
     * this stage, but this does not occur in GCM mode
     */
    EVP_EncryptFinal_ex(ctx, ciphertext + len, &len);
    ciphertext_len += len;

    /* Get the tag */
    EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, 16, mac);

    /* Clean up */
    EVP_CIPHER_CTX_free(ctx);

    return ciphertext_len;
}

int gcm_decrypt(unsigned char *ciphertext, int ciphertext_len,
                unsigned char *mac,
                unsigned char *key, unsigned char *iv,
                unsigned char *plaintext)
{
    EVP_CIPHER_CTX *ctx;
    int len = 0;
    int plaintext_len = 0;
    int ret;

    /* Create and initialise the context */
    ctx = EVP_CIPHER_CTX_new();

    /* Initialise the decryption operation. */
    EVP_DecryptInit_ex(ctx, EVP_aes_128_gcm(), NULL, NULL, NULL);

    /* Set IV length. Not necessary if this is 12 bytes (96 bits) */
    //EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, iv_len, NULL);

    /* Initialise key and IV */
    EVP_DecryptInit_ex(ctx, NULL, NULL, key, iv);

    /*
     * Provide any AAD data. This can be called zero or more times as
     * required
     */
    //EVP_DecryptUpdate(ctx, NULL, &len, aad, aad_len);

    /*
     * Provide the message to be decrypted, and obtain the plaintext output.
     * EVP_DecryptUpdate can be called multiple times if necessary
     */
    EVP_DecryptUpdate(ctx, plaintext, &len, ciphertext, ciphertext_len);
    plaintext_len = len;

    /* Set expected tag value. Works in OpenSSL 1.0.1d and later */
    EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, 16, mac);

    /*
     * Finalise the decryption. A positive return value indicates success,
     * anything else is a failure - the plaintext is not trustworthy.
     */
    ret = EVP_DecryptFinal_ex(ctx, plaintext + len, &len);

    /* Clean up */
    EVP_CIPHER_CTX_free(ctx);

    if(ret > 0) {
        /* Success */
        plaintext_len += len;
        return plaintext_len;
    } else {
        /* Verify failed */
        return 0;
    }
}

void alloc_gcm(struct ctx_gcm_s *ctx) {
    memset(ctx, 0, sizeof(struct ctx_gcm_s));

    RAND_bytes(ctx->key, GCM_KEY_SIZE);
    RAND_bytes(ctx->IV, GCM_IV_SIZE);
}