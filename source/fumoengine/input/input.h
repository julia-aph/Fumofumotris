#pragma once
#include <iso646.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "fumocommon.h"
#include "ringbuffer.h"

#define IO_BUF_SIZE 16
#define STR_BUF_SIZE (IO_BUF_SIZE * 4)


extern const struct RingBufferT IO_BUF_T;
extern const struct RingBufferT STR_BUF_T;


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

struct InputBuffer {
    struct {
        struct RingBufferHead head;
        struct InputRecord buf[IO_BUF_SIZE];
    } recs;

    struct {
        struct RingBufferHead head;
        char buf[STR_BUF_SIZE];
    } str;
};

struct InputHandle {
    struct InputBuffer in;

    pthread_t thread;
    pthread_mutex_t mutex;
    pthread_cond_t is_consumed;

    int err;
    bool is_terminating;
};

bool BeginInputThread(
    struct InputHandle *hand,
    struct InputBuffer *in,
);

bool EndInputThread(struct InputHandle *hand);

bool InputAquire(struct InputHandle *hand);

bool InputRelease(struct InputHandle *hand);

size_t InputString(struct Controller *ctrl, size_t n, char *buf);