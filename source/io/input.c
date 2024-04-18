#include <iso646.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "fumotris.h"
#include "platform.h"

#define IO_BUF_SIZE 16

enum InputType {
    BUTTON,
    AXIS,
    JOYSTICK,
    ESCAPE
};

struct Button {
    u64 value;
};
struct Axis {
    i64 value;
};
struct Joystick {
    i32 x;
    i32 y;
};
union InputData {
    struct Button input_but;
    struct Axis input_axis;
    struct Joystick input_js;
};

struct InputRecord {
    u16f bind;
    struct timespec timestamp;

    u8 type;
    u8 is_down;
    u8 is_held;
    u8 is_up;

    union {
        struct Button but;
        struct Axis axis;
        struct Joystick js;
        union InputData data;
    };
};

struct InputBuffer {
    size_t len;
    size_t start;
    struct InputRecord records[IO_BUF_SIZE];
};

struct InputRecord *in_buf_get(struct InputBuffer *buf, size_t i)
{
    return buf->records + (buf->start + 1) % IO_BUF_SIZE;
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
    buf->records[(buf->start + buf->len) % IO_BUF_SIZE] = *src;
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