#include "input.h"

#include "platform.h"

struct InputRecord *buf_get(struct InputBuffer *buf, size_t i) {
    return buf->recs + (buf->start + i) % IO_BUF_SIZE;
}

size_t min_size(size_t a, size_t b) {
    return a < b ? a : b;
}

void InputBufferTransfer(struct InputBuffer *dest, struct InputBuffer *src)
{
    size_t copy_amt = min_size(IO_BUF_SIZE - dest->len, src->len);

    for (size_t i = 0; i < copy_amt; i++)
        *buf_get(dest, dest->len + i) = *buf_get(src, i);

    if (copy_amt < src->len)
        src->start += copy_amt;

    src->len -= copy_amt;
    dest->len += copy_amt;
}

void InputBufferAdd(struct InputBuffer *buf, struct InputRecord *rec)
{
    *buf_get(buf, buf->len++) = *rec;
}

void *input_thread_loop(void *arg)
{
    struct InputThreadHandle *hand = arg;
    struct InputBuffer tmp_buf = { .len = 0, .start = 0 };

    while (!hand->is_terminating) {
        if (!PlatformReadInput(&tmp_buf)) {
            hand->err = true;
            return nullptr;
        }

        hand->err = pthread_mutex_lock(&hand->mutex);
        if (hand->err)
            return nullptr;

        while (tmp_buf.len == IO_BUF_SIZE) {
            hand->err = pthread_cond_wait(&hand->buf_read, &hand->mutex);
            if (hand->err)
                return nullptr;
        }

        InputBufferTransfer(hand->buf, &tmp_buf);

        hand->err = pthread_mutex_unlock(&hand->mutex);
        if (hand->err)
            return nullptr;
    }

    return nullptr;
}

bool BeginInputThread(struct InputThreadHandle *hand, struct InputBuffer *buf)
{
    hand->buf = buf;
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