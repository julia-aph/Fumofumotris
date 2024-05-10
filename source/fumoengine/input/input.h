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
    struct Button but;
    struct Axis axis;
    struct Joystick js;
};

struct InputRecord {
    nsec time;

    union InputData data;

    u16 code;
    u16 type;
    
    bool is_down;
};

struct InputAxis {
    nsec last_pressed;
    nsec last_released;

    union InputData data;

    u16 type;

    bool is_down;
    bool is_held;
    bool is_up;
};

struct RecordBuffer {
    struct RingBufferHead head;
    struct InputRecord buf[IO_BUF_SIZE];
};

struct StringBuffer {
    struct RingBufferHead head;
    char buf[STR_BUF_SIZE];
};

struct InputHandle {
    struct RecordBuffer recs;
    struct StringBuffer str;

    pthread_t thread;
    pthread_mutex_t mutex;
    pthread_cond_t is_consumed;

    int err;
    bool is_terminating;
};


extern RingBufferT IO_BUF_T;
extern RingBufferT STR_BUF_T;


bool CreateInputThread(struct InputHandle *hand);

bool JoinInputThread(struct InputHandle *hand);

bool InputAquire(struct InputHandle *hand);

bool InputRelease(struct InputHandle *hand);

size_t InputString(struct InputHandle *hand, size_t n, char *buf);