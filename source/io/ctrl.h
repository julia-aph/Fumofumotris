#pragma once
#include <iso646.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "fumotris.h"

#define IO_BUF_SIZE 16

enum InputType {
    KEY,
    AXIS,
    JOYSTICK,
    WINDOW,
    ESCAPE
};

struct InputRecord {
    u16 id;
    u8 type;

    union {
        struct {
            bool is_down;
        } key;
        struct {
            u64 value;
        } axis;
        struct {
            u32 x;
            u32 y;
        } joystick;
    } data;

    struct timespec timestamp;
};

struct InputBuffer {
    struct InputRecord records[IO_BUF_SIZE];
    size_t count;
    pthread_mutex_t mutex;
};

struct InputBuffer NewInputBuffer();

struct Axis {
    union {
        struct {
            u32 value;
            u32 is_down : 1;
            u32 is_up : 1;
        } key;
        struct {
            u64 value;
        } axis;
        struct {
            u32 x;
            u32 y;
        } joystick;
    } data;

    struct timespec last_pressed;
    struct timespec last_released;
};

enum KeyCode {
    LEFT,
    RIGHT,
    SOFT_DROP,
    HARD_DROP,
    ROTATE_CCW,
    ROTATE_CW,
    ROTATE_180,
    SWAP,
    ESC
};

enum AxisCode {
    VSCROLL,
    HSCROLL
};

enum JoystickCode {
    MOUSE
};

typedef u32 hashtype;

struct bkt {
    hashtype hash;
    u16 value;
    u8 type;
    struct Axis *axis;
};

struct dict {
    size_t capacity;
    size_t filled;
    struct bkt *bkts;
};

struct Ctrl {
    struct dict codes;
    struct dict binds;

    pthread_t thread;
    pthread_mutex_t mutex;
};
typedef struct Ctrl Ctrl;

#define NEW_CTRL(CTRL, CODES, BINDS)    \
    struct bkt NEW_CTRL_CODES[CODES];   \
    struct bkt NEW_CTRL_BINDS[BINDS];   \
    struct Axis NEW_CTRL_AXES[CODES];   \
    CTRL = NewCtrl(                     \
        NEW_CTRL_CODES,                 \
        NEW_CTRL_AXES, CODES,           \
        NEW_CTRL_BINDS, BINDS           \
    );                                  \

Ctrl NewCtrl(struct bkt *codes, struct Axis *axes, size_t c, struct bkt *binds, size_t b);

bool CtrlMap(Ctrl *ctrl, u16f code, u16f bind, u8f type);

struct Axis *CtrlGet(Ctrl *ctrl, u16f code, u8f type);

bool CtrlPoll(Ctrl *ctrl, struct InputBuffer *buf);