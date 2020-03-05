//
// Created by shangqi on 29/2/20.
//

#include "Queue.h"

void queue_init(Queue *q) {
    q->top = NULL;
    q->bottom = NULL;
    q->lock = 0;
}

int is_empty_queue(Queue *q) {
    return (q->top == NULL);
}

void push_back(Queue *q, Message *new_msg) {
    new_msg->header.prev = NULL;
    new_msg->header.next = NULL;

    spin_lock(&q->lock);

    if(q->bottom == NULL) {
        q->bottom = new_msg;
        q->top = new_msg;
    } else {
        q->bottom->header.prev = new_msg;
        new_msg->header.next = q->bottom;
        q->bottom = new_msg;
    }

    spin_unlock(&q->lock);
}

void push_front(Queue *q, Message *new_msg) {
    new_msg->header.prev = NULL;
    new_msg->header.next = NULL;

    spin_lock(&q->lock);

    if(q->top == NULL) {
        q->top = new_msg;
        q->bottom = new_msg;
    } else {
        q->top->header.next = new_msg;
        new_msg->header.prev = q->top;
        q->top = new_msg;
    }

    spin_unlock(&q->lock);
}

Message *pop_front(Queue *q) {

    Message *msg;

    spin_lock(&q->lock);

    if(q->top == NULL) {
        spin_unlock(&q->lock);
        return NULL;
    }

    msg = q->top;
    if(msg->header.prev)
        msg->header.prev->header.next = NULL;

    q->top = msg->header.prev;

    if(q->top == NULL)
        q->bottom = NULL;

    spin_unlock(&q->lock);

    msg->header.prev = NULL;
    msg->header.next = NULL;

    return msg;
}

Message *pop_back(Queue *q) {
    if(q->bottom == NULL)
        return NULL;

    Message *msg;

    spin_lock(&q->lock);

    msg = q->bottom;
    if(msg->header.next)
        msg->header.next->header.prev = NULL;
    q->bottom = msg->header.next;

    if(q->bottom == NULL)
        q->top = NULL;

    spin_unlock(&q->lock);

    msg->header.prev = NULL;
    msg->header.next = NULL;

    return msg;
}