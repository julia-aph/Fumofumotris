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

struct thread_init {
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    bool done;

    void *args;
};

struct input_args {
    struct InputBuffer *buf;
    pthread_mutex_t *mutex;
};

bool ThreadSignalInit()
{
    
}

void *block_input(void *args)
{
    struct input_args *args = args + 1;
    pthread_mutex_lock(&args->init.mutex);
    args->init.is_init = true;
    pthread_cond_signal(&args->init.cond);
    pthread_mutex_unlock(&args->init.mutex);

    struct InputBuffer tmp_buf = { .len = 0, .start = 0 };

    while (true) {
        if (!PlatformReadInput(&tmp_buf))
            return false;

        pthread_mutex_lock(&in->mutex);
        {
            InputBufferTransfer(&tmp_buf, buf);
        }
        pthread_mutex_unlock(&in->mutex);
    }

    return nullptr;
}

void *thread_wrap(void *init_ptr)
{
    struct thread_init *init = init_ptr;
    
    block_input();
}

bool ThreadWaitInit(void *(*func), size_t args_len, void *args)
{
    struct thread_init init = {
        .mutex = PTHREAD_MUTEX_INITIALIZER,
        .cond = PTHREAD_COND_INITIALIZER,
        .done = false,
        .args = &args,
    };

    pthread_t thread;
    if (pthread_create(&thread, nullptr, thread_wrap, &init) != 0)
        return false;

    pthread_mutex_lock(&init.mutex);
    {
        while (!init.done)
            pthread_cond_wait(&init.cond, &init.mutex);
    }
    pthread_mutex_unlock(&init.mutex);

    pthread_mutex_destroy(&init.mutex);
    pthread_cond_destroy(&init.cond);
}

bool InputStart(struct InputBuffer *buf, pthread_mutex_t *mutex)
{
    struct input_args args = {
        .buf = buf,
        .mutex = mutex,
    };

    return ThreadWaitInit(block_input, &args);
}