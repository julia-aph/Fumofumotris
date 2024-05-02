#include "input.h"
#include <string.h>

#include "platform.h"


RingBufferT IO_BUF_T = RINGBUF_T_INIT(
    struct RecordBuffer,
    struct InputRecord,
    IO_BUF_SIZE
);

RingBufferT STR_BUF_T = RINGBUF_T_INIT(
    struct StringBuffer,
    char,
    STR_BUF_SIZE
);


void *input_worker(void *arg)
{
    struct InputHandle *hand = arg;

    struct RecordBuffer tmp_recs = { .head.len = 0, .head.start = 0 };
    struct StringBuffer tmp_str = { .head.len = 0, .head.start = 0 };

    while (!hand->is_terminating) {
        if (!PlatformReadInput(&tmp_recs, &tmp_str)) {
            hand->err = true;
            return nullptr;
        }
        
        if (pthread_mutex_lock(&hand->mutex) != 0) {
            hand->err = true;
            return nullptr;
        }

        while (tmp_recs.head.len == IO_BUF_SIZE) {
            if (pthread_cond_wait(&hand->is_consumed, &hand->mutex) != 0) {
                hand->err = true;
                return nullptr;
            }
        }

        RingBufferTransfer(IO_BUF_T, &hand->recs.head, &tmp_recs.head);
        RingBufferTransfer(STR_BUF_T, &hand->str.head, &tmp_str.head);

        if (pthread_mutex_unlock(&hand->mutex) != 0) {
            hand->err = true;
            return nullptr;
        }
    }

    return nullptr;
}

bool BeginInputThread(struct InputHandle *hand)
{
    *hand = (struct InputHandle) {
        .recs.head = RINGBUF_HEAD_INIT,
        .str.head = RINGBUF_HEAD_INIT,
    
        .err = 0,
        .is_terminating = false,
    };

    if (pthread_mutex_init(&hand->mutex, nullptr) != 0)
        return false;

    if (pthread_cond_init(&hand->is_consumed, nullptr) != 0)
        return false;

    if (pthread_create(&hand->thread, nullptr, input_worker, hand) != 0)
        return false;

    return true;
}

bool EndInputThread(struct InputHandle *hand)
{
    hand->is_terminating = true;

    if (!PlatformStopInput())
        return false;

    if (pthread_mutex_destroy(&hand->mutex) != 0)
        return false;

    if (pthread_join(hand->thread, nullptr) != 0)
        return false;

    return true;
}

bool InputAquire(struct InputHandle *hand)
{
    if (pthread_mutex_lock(&hand->mutex) != 0)
        return false;

    return true;
}

bool InputRelease(struct InputHandle *hand)
{
    if (pthread_cond_signal(&hand->is_consumed) != 0)
        return false;

    if (pthread_mutex_unlock(&hand->mutex) != 0)
        return false;

    return true;
}

size_t InputString(struct InputHandle *hand, size_t n, char *buf)
{
    return RingBufferOut(STR_BUF_T, n, buf, &hand->str.head);
}