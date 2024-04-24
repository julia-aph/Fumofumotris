#include "input.h"
#include <string.h>

#include "platform.h"

inline size_t min_size(size_t a, size_t b) {
    return a < b ? a : b;
}

void InputBufferTransfer(struct InputBuffer *dest, struct InputBuffer *src)
{
    size_t copy_amt = min_size(IO_BUF_SIZE - dest->len, src->len);

    for (size_t i = 0; i < dest->start; i++) {
        size_t dest_i = (dest->start + dest->len + i) % IO_BUF_SIZE;
        size_t src_i = (src->start + i) % IO_BUF_SIZE;

        dest->buf[dest_i] = src->buf[src_i];
    }

    dest->start += copy_amt;
    if (copy_amt < src->len)
        src->len -= copy_amt;
}

void InputStringTransfer(struct InputString *dest, struct InputString *src)
{
    size_t copy_amt = min_size(STR_BUF_SIZE - dest->len, src->len);

    for (size_t i = 0; i < copy_amt; i++) {
        size_t dest_i = (dest->start + dest->len + i) % STR_BUF_SIZE;
        size_t src_i = (src->start + i) % STR_BUF_SIZE;

        dest->buf[dest_i] = src->buf[src_i];
    }

    dest->start += copy_amt;
    if (copy_amt < src->len)
        src->len -= copy_amt;
}

void *input_thread_loop(void *arg)
{
    struct InputThreadHandle *hand = arg;

    struct InputBuffer tmp_in = { .len = 0 };
    struct InputString tmp_str = { .len = 0 };

    while (!hand->is_terminating) {
        if (!PlatformReadInput(&tmp_in, &tmp_str)) {
            hand->err = true;
            return nullptr;
        }

        hand->err = pthread_mutex_lock(&hand->mutex);
        if (hand->err)
            return nullptr;

        while (tmp_in.len == IO_BUF_SIZE) {
            hand->err = pthread_cond_wait(&hand->buf_read, &hand->mutex);
            if (hand->err)
                return nullptr;
        }

        InputBufferTransfer(hand->in, &tmp_in);
        InputStringTransfer(hand->str, &tmp_str);

        hand->err = pthread_mutex_unlock(&hand->mutex);
        if (hand->err)
            return nullptr;
    }

    return nullptr;
}

bool BeginInputThread(struct InputThreadHandle *hand, struct InputBuffer *buf)
{
    hand->in = buf;
    hand->mutex = PTHREAD_MUTEX_INITIALIZER;
    hand->buf_read = PTHREAD_COND_INITIALIZER;
    hand->err = 0;

    return pthread_create(&hand->thread, nullptr, input_thread_loop, hand) == 0;
}

bool EndInputThread(struct InputThreadHandle *hand)
{
    hand->is_terminating = true;

    if (!PlatformStopInput())
        return false;

    if (!pthread_mutex_destroy(&hand->mutex))
        return false;

    if (!pthread_join(hand->thread, nullptr))
        return false;

    return true;
}