#include "ringbuffer.h"
#include <string.h>

struct ring_buf {
    struct RingBufferHead head;
    u8 bytes[];
};

void *get_ptr(RingBufferT T, struct RingBufferHead *head, size_t i)
{
    struct ring_buf *ring = (struct ring_buf *)head;
    return ring->bytes + T->SIZE * i;
}

size_t RingBufferEmpty(RingBufferT T, struct RingBufferHead *head)
{
    return T->LEN - head->len;
}

void *RingBufferGet(RingBufferT T, struct RingBufferHead *head, size_t i)
{
    size_t wrap_i = (head->start + i) % T->LEN;
    return get_ptr(T, head, wrap_i);
}

void *RingBufferNext(RingBufferT T, struct RingBufferHead *head)
{
    size_t wrap_i = (head->start + head->len) % T->LEN;
    return get_ptr(T, head, wrap_i);
}

void RingBufferAdd(RingBufferT T, struct RingBufferHead *dest, void *item)
{
    memcpy(RingBufferNext(T, dest), item, T->SIZE);
}

void RingBufferTransfer(
    RingBufferT T,
    struct RingBufferHead *dest,
    struct RingBufferHead *src
) {
    size_t copy_max = min_size(T->LEN - dest->len, src->len);

    for (size_t i = 0; i < copy_max; i++) {
        void *to = RingBufferGet(T, dest_head, dest->len + i);
        void *from = RingBufferGet(T, src_head, i);
        memcpy(to, from, T->SIZE);
    }

    dest->len += copy_max;
    src->len -= copy_max;
}

size_t RingBufferOut(RingBufferT T, size_t n, void *dest, void *src_head)
{
    struct ring_buf *src = src_head;

    size_t copy_max = min_size(n, src->head.len);

    for (size_t i = 0; i < copy_max; i++) {
        void *to = (char *)dest + i * T->SIZE;
        void *from = RingBufferGet(T, src, i);
        memcpy(to, from, T->SIZE);
    }

    return copy_max;
}