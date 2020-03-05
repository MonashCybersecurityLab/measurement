//
// Created by shangqi on 8/2/20.
//

#ifndef MEASUREMENT_MESSAGE_H
#define MEASUREMENT_MESSAGE_H

#include <stdint.h>
#include "CommonUtil.h"

#ifdef __cplusplus
extern "C" {
#endif

enum message_type {
    STAT = 0,       // new stat info
    FLOW_SIZE,      // query flow size
    HEAVY_HITTER,   // query the heavy hitter
    HEAVY_CHANGE,   // query the heavy change
    CARDINALITY,    // query the cardinality
    STOP            // terminate the system
};

struct __attribute__((aligned(64))) header_s {
    struct message_s *next;			///< pointer to the next
    struct message_s *prev;			///< pointer to the prev
    enum message_type type;              /// message type
    int payload_size;                    /// payload size
    unsigned char mac[GCM_MAC_SIZE];	 /// <each message has MAC even it does not use it. Because of because.
};

#define HEADER_SIZE	(sizeof(struct header_s))
#define PL_SIZE 65536

struct __attribute__((aligned(64))) message_s {
    struct header_s header;
    uint8_t *payload;
};

typedef struct message_s Message;

void pack_message_with_file(Message *message, enum message_type type, struct ctx_gcm_s *ctx, char *fileName);
void pack_message(Message *message, enum message_type type, struct ctx_gcm_s *ctx, uint8_t *payload, int size, int is_enclave);

int unpack_message(Message *message, struct ctx_gcm_s *ctx, uint8_t *res);

void free_message(Message *message);

#ifdef __cplusplus
}
#endif


#endif //MEASUREMENT_MESSAGE_H
