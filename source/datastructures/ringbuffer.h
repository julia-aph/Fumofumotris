#pragma once
#include <iso646.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "fumotris.h"

typedef const struct RingBufferT {
    size_t LEN;
    size_t SIZE;
} *const RingBufferT;

struct RingBufferHead {
    u8f len;
    u8f start;
};

#define RINGBUF_HEAD_INIT ((struct RingBufferHead) { 0, 0 })

size_t RingBufferEmpty(RingBufferT T, struct RingBufferHead *head);

void *RingBufferGet(
    RingBufferT T,
    struct RingBufferHead *head,
    size_t i
);

void *RingBufferNext(RingBufferT T, struct RingBufferHead *head);

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