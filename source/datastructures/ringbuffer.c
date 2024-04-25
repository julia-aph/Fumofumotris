#pragma once
#include "ringbuffer.h"
#include <string.h>

struct rbuf_generic {
    struct RingBufferHead head;
    u8 arr[];
};

inline void *rbuf_start(RingBufferT T, struct rbuf_generic *rbuf, size_t i)
{
    size_t wrap_i = (rbuf->head.start + i) % T->LEN;
    return rbuf->arr + wrap_i * T->SIZE;
}

inline void *rbuf_tail(RingBufferT T, struct rbuf_generic *rbuf, size_t i)
{
    size_t wrap_i = (rbuf->head.start + rbuf->head.len + i) % T->LEN;
    return rbuf->arr + i * T->SIZE;
}

void RingBufferTransfer(RingBufferT T, RingBuffer *dest, RingBuffer *tmp)
{ 
    struct rbuf_generic *write = dest;
    struct rbuf_generic *read = tmp;

    size_t copy_max = min_size(T->LEN - write->head.len, read->head.len);

    for (size_t i = 0; i < copy_max; i++) {
        memcpy(rbuf_tail(T, write, i), rbuf_start(T, read, i), T->LEN);
    }

    write->head.start += copy_max;
    read->head.len -= copy_max;
}