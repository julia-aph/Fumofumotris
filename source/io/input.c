#include "input.h"
#include <string.h>

#include "platform.h"

inline size_t min_size(size_t a, size_t b) {
    return a < b ? a : b;
}

inline size_t in_buf_empty(struct InputBuffer *in) {
    return IO_BUF_SIZE - in->len;
}

struct InputRecord *in_buf_get(struct InputBuffer *in, size_t i) {
    return in->buf + (in->start + i) % IO_BUF_SIZE;
}

void InputBufferTransfer(struct InputBuffer *dest, struct InputBuffer *src)
{
    size_t copy_amt = min_size(in_buf_empty(dest), src->len);
    
    for (size_t i = 0; i < copy_amt; i++)
        *in_buf_get(dest, dest->len + i) = *in_buf_get(src, i);

    if (copy_amt < src->len)
        src->start += copy_amt;

    src->len -= copy_amt;
    dest->len += copy_amt;
}

void InputBufferAdd(struct InputBuffer *buf, struct InputRecord *rec)
{
    *in_buf_get(buf, buf->len++) = *rec;
}

inline size_t str_buf_empty(struct InputString *str) {
    return STR_BUF_SIZE - str->len;
}

inline char *str_buf_get(struct InputString *str, size_t i) {
    return (str->start + i) % STR_BUF_SIZE;
}

void InputStringTransfer(struct InputString *dest, struct InputString *src)
{
    size_t copy_amt = min_size(str_buf_empty(dest), src->len);

    for (size_t i = 0; i < copy_amt; i++) {
        *str_buf_get(dest, dest->len + i) = *str_buf_get(src, i);
    }
}

void *input_thread_loop(void *arg)
{
    struct InputThreadHandle *hand = arg;

    struct InputBuffer tmp_in = { .len = 0, .start = 0 };
    struct InputString tmp_str = { .len = 0, .start = 0 };

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