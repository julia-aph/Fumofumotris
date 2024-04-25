#pragma once
#include <iso646.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "fumotris.h"
#include "gametime.h"
#include "ringbuffer.h"

#define IO_BUF_SIZE 16
#define STR_BUF_SIZE (IO_BUF_SIZE * 4)

enum InputType {
    BUTTON,
    AXIS,
    JOYSTICK,
    ESCAPE
};

union InputID {
    struct {
        union {
            u16 code;
            u16 bind;
            u16 value;
        };
        u16 type;
    };
    u32 hash;
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
    Time time;

    union {
        struct Button but;
        struct Axis axis;
        struct Joystick js;
        union InputData data;
    };

    union InputID id;
    
    struct {
        u8f is_down : 1;
        u8f is_held : 1;
        u8f is_up : 1;
    };
};

RingBufferT IO_BUF_T = INIT_RING_BUF_T(IO_BUF_SIZE, struct InputRecord);
RingBufferT STR_BUF_T = INIT_RING_BUF_T(STR_BUF_SIZE, char);

struct InputRecordBuf {
    struct RingBufferHead head;
    struct InputRecord buf[IO_BUF_SIZE];
};

struct InputStringBuf {
    struct RingBufferHead head;
    char buf[STR_BUF_SIZE];
};

struct InputThreadHandle {
    struct InputRecordBuf *in;
    struct InputStringBuf *str;

    pthread_t thread;
    pthread_mutex_t mutex;
    pthread_cond_t consume;

    int err;
    bool is_terminating;
};

bool BeginInputThread(
    struct InputThreadHandle *hand,
    struct InputRecordBuf *in,
    struct InputStringBuf *str
);

bool EndInputThread(struct InputThreadHandle *hand);