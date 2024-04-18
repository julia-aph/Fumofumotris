#include "input.h"
#include <pthread.h>

#include "platform.h"

struct InputRecord *in_buf_get(struct InputBuffer *buf, size_t i)
{
    return buf->recs + (buf->start + 1) % IO_BUF_SIZE;
}

void InputBufferTransfer(struct InputBuffer *tmp, struct InputBuffer *dest)
{
    size_t n = IO_BUF_SIZE - (tmp->len > dest->len ? tmp->len : dest->len);

    for (size_t i = 0; i < n; i++) {
        *in_buf_get(dest, dest->len + i) = *in_buf_get(tmp, i);
    }

    if (n < tmp->len)
        tmp->start += n;

    tmp->len -= n;
    dest->len += n;
}

void InputBufferCopy(struct InputBuffer *buf, struct InputRecord *src)
{
    buf->recs[(buf->start + buf->len) % IO_BUF_SIZE] = *src;
    buf->len += 1;
}

struct input_args {
    struct InputBuffer *buf;
    pthread_mutex_t *mutex;
};

void *block_input(struct input_args *args)
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

bool InputStart(struct InputBuffer *buf, pthread_mutex_t *mutex)
{
    struct input_args *args = malloc(sizeof(struct input_args));
    *args = (struct input_args) {
        .buf = buf,
        .mutex = mutex,
    };

    pthread_t thread;
    return pthread_create(&thread, nullptr, block_input, args) == 0;
}