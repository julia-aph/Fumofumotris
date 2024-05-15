#include "ringbuffer.h"

#include <string.h>


void *get_ptr(RingBufferT T, struct RingBufferHead *head, usize i)
{
    u8 *bytes = (u8 *)head;
    return bytes + T->OFFSET + T->SIZE * i;
}

usize RingBufferEmpty(RingBufferT T, struct RingBufferHead *head)
{
    return T->LEN - head->len;
}

void *RingBufferGet(RingBufferT T, struct RingBufferHead *head, usize i)
{
    usize wrap_i = (head->start + i) % T->LEN;
    return get_ptr(T, head, wrap_i);
}

void *RingBufferNext(RingBufferT T, struct RingBufferHead *head)
{
    usize wrap_i = (head->start + head->len) % T->LEN;
    return get_ptr(T, head, wrap_i);
}

void RingBufferAdd(RingBufferT T, struct RingBufferHead *dest, void *item)
{
    memcpy(RingBufferNext(T, dest), item, T->SIZE);
    dest->len += 1;
}

void RingBufferTransfer(
    RingBufferT T,
    struct RingBufferHead *dest,
    struct RingBufferHead *tmp
) {
    usize copy_max = min(T->LEN - dest->len, tmp->len);

    for (usize i = 0; i < copy_max; i++) {
        void *to = RingBufferGet(T, dest, dest->len + i);
        void *from = RingBufferGet(T, tmp, i);
        memcpy(to, from, T->SIZE);
    }

    dest->len += copy_max;
    tmp->len -= copy_max;
}

usize RingBufferOut(
    RingBufferT T,
    usize n,
    void *dest,
    struct RingBufferHead *src
) {
    usize copy_max = min(n, src->len);

    for (usize i = 0; i < copy_max; i++) {
        void *to = (u8 *)dest + i * T->SIZE;
        void *from = RingBufferGet(T, src, i);
        memcpy(to, from, T->SIZE);
    }

    src->len -= copy_max;

    return copy_max;
}