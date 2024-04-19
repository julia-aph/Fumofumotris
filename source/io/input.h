#pragma once
#include <iso646.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "fumotris.h"
#include "gametime.h"

#define IO_BUF_SIZE 16

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
    struct Time time;

    union {
        struct Button but;
        struct Axis axis;
        struct Joystick js;
        union InputData data;
    };

    u16f bind;

    u8 type;
    u8 is_down;
    u8 is_held;
    u8 is_up;
};

struct InputBuffer {
    struct InputRecord recs[IO_BUF_SIZE];
    u8f len;
    u8f start;
};

struct InputThreadHandle {
    struct InputBuffer *buf;

    pthread_t thread;
    pthread_mutex_t mutex;

    int err;
    bool is_terminating;
};

void InputBufferTransfer(struct InputBuffer *tmp, struct InputBuffer *dest);

void InputBufferAdd(struct InputBuffer *buf, struct InputRecord *src);

bool BeginInputThread(struct InputThreadHandle *hand, struct InputBuffer *buf);

bool EndInputThread(struct InputThreadHandle *hand);