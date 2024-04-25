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

struct RecordBuffer {
    struct InputRecord buf[IO_BUF_SIZE];
    u8f len;
    u8f start;
};

struct StringBuffer {
    char buf[STR_BUF_SIZE];
    u8f len;
    u8f start;
};

struct InputThreadHandle {
    struct RecordBuffer *in;
    struct StringBuffer *str;

    pthread_t thread;
    pthread_mutex_t mutex;
    pthread_cond_t consume;

    int err;
    bool is_terminating;
};

void InputBufferTransfer(struct RecordBuffer *tmp, struct RecordBuffer *dest);

bool BeginInputThread(
    struct InputThreadHandle *hand,
    struct RecordBuffer *in,
    struct StringBuffer *str
);

bool EndInputThread(struct InputThreadHandle *hand);