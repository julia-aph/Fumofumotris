#pragma once
#include <iso646.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "fumotris.h"

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

void InputBufferTransfer(struct InputBuffer *tmp, struct InputBuffer *dest);

void InputBufferCopy(struct InputBuffer *buf, struct InputRecord *src);

bool InputStart(struct InputBuffer *buf, pthread_mutex_t *mutex);