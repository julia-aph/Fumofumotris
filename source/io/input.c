#include "input.h"
#include <string.h>

#include "platform.h"

void *input_thread_loop(void *arg)
{
    struct InputThreadHandle *hand = arg;

    struct InputRecordBuf tmp_in = { .head.len = 0, .head.start = 0 };
    struct InputStringBuf tmp_str = { .head.len = 0, .head.start = 0 };

    while (!hand->is_terminating) {
        if (!PlatformReadInput(&tmp_in, &tmp_str))
            return nullptr;

        if (hand->err = pthread_mutex_lock(&hand->mutex))
            return nullptr;

        while (tmp_in.head.len == IO_BUF_SIZE) {
            if (hand->err = pthread_cond_wait(&hand->consume, &hand->mutex))
                return nullptr;
        }

        RingBufferTransfer(IO_BUF_T, hand->in, &tmp_in);
        RingBufferTransfer(STR_BUF_T, hand->str, &tmp_str);

        if (hand->err = pthread_mutex_unlock(&hand->mutex))
            return nullptr;
    }

    return nullptr;
}

bool BeginInputThread(
    struct InputThreadHandle *hand,
    struct InputRecordBuf *in,
    struct InputStringBuf *str
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