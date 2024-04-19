#include "input.h"
#include <pthread.h>

#include "platform.h"

struct InputRecord *buf_get(struct InputBuffer *buf, size_t i) {
    return buf->recs + (buf->start + i) % IO_BUF_SIZE;
}

size_t max_size(size_t a, size_t b) {
    return a > b ? a : b;
}

void InputBufferTransfer(struct InputBuffer *dest, struct InputBuffer *src)
{
    size_t copy_amt = IO_BUF_SIZE - max_size(dest->len, src->len);

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

struct input_args {
    struct InputBuffer *buf;
    pthread_mutex_t *mutex;
};

void *input_thread_loop(struct input_args *args)
{
    struct InputBuffer *buf = args->buf;
    pthread_mutex_t *mutex = args->mutex;
    free(args);

    struct InputBuffer tmp_buf = { .len = 0, .start = 0 };

    while (true) {
        if (!PlatformReadInput(&tmp_buf))
            return false;

        pthread_mutex_lock(mutex);
        {
            InputBufferTransfer(&tmp_buf, buf);
        }
        pthread_mutex_unlock(mutex);
    }

    return nullptr;
}

bool CreateInputThread(struct InputBuffer *buf, pthread_mutex_t *mutex)
{
    struct input_args *args = malloc(sizeof(struct input_args));
    *args = (struct input_args) {
        .buf = buf,
        .mutex = mutex,
    };

    pthread_t thread;
    return pthread_create(&thread, nullptr, input_thread_loop, args) == 0;
}