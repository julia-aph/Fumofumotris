#include "input.h"
#include <string.h>

#include "platform.h"

void InputBufferTransfer(struct RecordBuffer *dest, struct RecordBuffer *src)
{
    size_t copy_max = min_size(IO_BUF_SIZE - dest->len, src->len);

    for (size_t i = 0; i < copy_max; i++) {
        size_t dest_i = (dest->start + dest->len + i) % IO_BUF_SIZE;
        size_t src_i = (src->start + i) % IO_BUF_SIZE;

        dest->buf[dest_i] = src->buf[src_i];
    }

    dest->start += copy_max;
    if (copy_max < src->len)
        src->len -= copy_max;
}

void StringBufferTransfer(struct StringBuffer *dest, struct StringBuffer *src)
{
    size_t copy_max = min_size(STR_BUF_SIZE - dest->len, src->len);

    for (size_t i = 0; i < copy_max; i++) {
        size_t dest_i = (dest->start + dest->len + i) % STR_BUF_SIZE;
        size_t src_i = (src->start + i) % STR_BUF_SIZE;

        dest->buf[dest_i] = src->buf[src_i];
    }

    dest->start += copy_max;
    if (copy_max < src->len)
        src->len -= copy_max;
}

void *input_thread_loop(void *arg)
{
    struct InputThreadHandle *hand = arg;

    struct RecordBuffer tmp_in = { .len = 0, .start = 0 };
    struct StringBuffer tmp_str = { .len = 0, .start = 0 };

    while (!hand->is_terminating) {
        if (!PlatformReadInput(&tmp_in, &tmp_str))
            return nullptr;

        if (hand->err = pthread_mutex_lock(&hand->mutex))
            return nullptr;

        while (tmp_in.len == IO_BUF_SIZE) {
            if (hand->err = pthread_cond_wait(&hand->consume, &hand->mutex))
                return nullptr;
        }

        InputBufferTransfer(hand->in, &tmp_in);
        StringBufferTransfer(hand->str, &tmp_str);

        if (hand->err = pthread_mutex_unlock(&hand->mutex))
            return nullptr;
    }

    return nullptr;
}

bool BeginInputThread(
    struct InputThreadHandle *hand,
    struct RecordBuffer *in,
    struct StringBuffer *str
) {
    *hand = (struct InputThreadHandle) {
        .in = in,
        .str = str,
        
        .mutex = PTHREAD_MUTEX_INITIALIZER,
        .consume = PTHREAD_COND_INITIALIZER,

        .err = 0,
        .is_terminating = false,
    };

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