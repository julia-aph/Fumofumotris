#pragma once
#include <iso646.h>
#include <stdalign.h>
#include <stdbool.h>
#include <stdint.h>

#include "fumocommon.h"


typedef const struct RingBufferT {
    size_t OFFSET;
    size_t SIZE;
    size_t LEN;
} *const RingBufferT;

struct RingBufferHead {
    size_t len;
    size_t start;
};

#define RINGBUF_T_INIT(RBUF_STRUCT, RBUF_ITEM, RBUF_LEN) \
    (&(struct RingBufferT) { \
        .OFFSET = offsetof(struct { \
            struct RingBufferHead head; \
            RBUF_ITEM item; \
        }, item), \
        .SIZE = sizeof(RBUF_ITEM), \
        .LEN = RBUF_LEN \
    }) \

#define RINGBUF_HEAD_INIT ((struct RingBufferHead) { 0, 0 })

size_t RingBufferEmpty(RingBufferT T, struct RingBufferHead *head);

void *RingBufferGet(
    RingBufferT T,
    struct RingBufferHead *head,
    size_t i
);

void *RingBufferNext(RingBufferT T, struct RingBufferHead *head);

void RingBufferAdd(RingBufferT T, struct RingBufferHead *dest, void *item);

void RingBufferTransfer(
    RingBufferT T,
    struct RingBufferHead *dest,
    struct RingBufferHead *tmp
);

size_t RingBufferOut(
    RingBufferT T,
    size_t n,
    void *dest,
    struct RingBufferHead *src
);