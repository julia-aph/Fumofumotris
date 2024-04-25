#pragma once
#include <iso646.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "fumotris.h"

typedef void *RingBuffer;

typedef const struct RingBufferT {
    size_t LEN;
    size_t SIZE;
} *const RingBufferT;

struct RingBufferHead {
    u8f len;
    u8f start;
};

void RingBufferTransfer(RingBufferT T, RingBuffer *dest, RingBuffer *tmp);

#define INIT_RING_BUF_T(len, type) &(struct RingBufferT) { len, sizeof(type) }