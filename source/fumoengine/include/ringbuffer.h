#pragma once
#include <stdalign.h>

#include "fumocommon.h"

#define RINGBUF_T(RBUF_ITEM_T, RBUF_LEN) \
    (&(struct RingBufferT) { \
        .OFFSET = offsetof(struct { \
            struct RingBufferHead head; \
            RBUF_ITEM_T item; \
        }, item), \
        .SIZE = sizeof(RBUF_ITEM_T), \
        .LEN = RBUF_LEN \
    }) \

#define RINGBUF_HEAD_INIT ((struct RingBufferHead) { 0, 0 })


typedef const struct RingBufferT {
    usize OFFSET;
    usize SIZE;
    usize LEN;
} *const RingBufferT;

struct RingBufferHead {
    usize len;
    usize start;
};


usize RingBufferEmpty(RingBufferT T, struct RingBufferHead *head);

void *RingBufferGet(
    RingBufferT T,
    struct RingBufferHead *head,
    usize i
);

void *RingBufferNext(RingBufferT T, struct RingBufferHead *head);

void RingBufferAdd(RingBufferT T, struct RingBufferHead *dest, void *item);

void RingBufferTransfer(
    RingBufferT T,
    struct RingBufferHead *dest,
    struct RingBufferHead *tmp
);

usize RingBufferOut(
    RingBufferT T,
    usize n,
    void *dest,
    struct RingBufferHead *src
);