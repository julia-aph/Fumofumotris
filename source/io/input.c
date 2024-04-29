#include "input.h"
#include <string.h>

#include "platform.h"

const struct RingBufferT IO_BUF_T = {
    .LEN = IO_BUF_SIZE,
    .SIZE = sizeof(struct InputRecord)
};

const struct RingBufferT STR_BUF_T = {
    .LEN = STR_BUF_SIZE,
    .SIZE = sizeof(char)
};

void *input_worker(void *arg)
{
    struct InputHandle *hand = arg;

    struct InputRecordBuf tmp_recs = { .head.len = 0, .head.start = 0 };
    struct InputStringBuf tmp_str = { .head.len = 0, .head.start = 0 };

    while (!hand->is_terminating) {
        if (!PlatformReadInput(&tmp_recs, &tmp_str)) {
            hand->err = true;
            return nullptr;
        }
        printf("input read, len:%u\n", hand->recs->head.len);

        for (int i = 0; i < hand->recs->head.len; i++) {
            struct InputRecord *rec = RingBufferGet(&IO_BUF_T, &hand->recs->head, i);
            printf("\ti:%u, type:%u, but:%u\n", i, rec->id.type, rec->id.bind);
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

        RingBufferTransfer(&IO_BUF_T, &hand->recs->head, &tmp_recs.head);
        RingBufferTransfer(&STR_BUF_T, &hand->str->head, &tmp_str.head);

        if (pthread_mutex_unlock(&hand->mutex) != 0) {
            hand->err = true;
            return nullptr;
        }
    }

    return nullptr;
}

bool BeginInputThread(
    struct InputHandle *hand,
    struct InputRecordBuf *in,
    struct InputStringBuf *str
) {
    *hand = (struct InputHandle) {
        .recs = in,
        .str = str,
    
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