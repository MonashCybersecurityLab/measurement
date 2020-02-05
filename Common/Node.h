//
// Implemented by EActor, GPL 3.0 license applied
//

#ifndef MEASUREMENT_NODE_H
#define MEASUREMENT_NODE_H

#include <stdio.h>
#include <sgx_tseal.h>

#ifdef __cplusplus
extern "C" {
#endif

struct __attribute__((aligned(64))) header_s {
    struct node_s *next;			///< pointer to the next
    struct node_s *prev;			///< pointer to the prev
    struct queue_s *pool;				///< pool where return
    uint32_t payload_size;         /// payload size
};

#define HEADER_SIZE	(sizeof(struct header_s))

struct __attribute__((aligned(64))) node_s {
    struct header_s header;
    uint8_t *payload;
};

typedef struct node_s Node;

struct __attribute__((aligned(64))) queue_s {
    Node	*top;
    Node	*bottom;
    unsigned char lock;				///< HLE lock
};


typedef struct queue_s queue;

void queue_init(queue *q);
int how_long(queue *q);
int is_empty(queue *q);
void push_back(queue *q, Node *new_node);
void push_front(queue *q, Node *new_node);
Node *pop_back(queue *q);
Node *pop_front(queue *q);

Node *nalloc(queue *q);
void nfree(Node *n);

#ifdef __cplusplus
}
#endif

#endif //MEASUREMENT_NODE_H
