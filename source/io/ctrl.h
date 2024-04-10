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
    ESCAPE
};

struct Button {
    u32 value;
    bool is_down;
    bool is_held;
    bool is_up;
};

struct Axis {
    i64 value;
};

struct Joystick {
    i32 x;
    i32 y;
};

struct InputRecord {
    u16 bind;
    u8 type;

    union {
        struct Button but;
        struct Axis axis;
        struct Joystick js;
    };

    struct timespec timestamp;
};

struct InputAxis {
    union {
        struct Button but;
        struct Axis axis;
        struct Joystick js;
    };

    struct timespec last_pressed;
    struct timespec last_released;
};

enum CtrlCode {
    LEFT,
    RIGHT,
    SOFT_DROP,
    HARD_DROP,
    ROTATE_CCW,
    ROTATE_CW,
    ROTATE_180,
    SWAP,
    ESC,
    VSCROLL,
    HSCROLL,
    MOUSE
};

typedef u32 hashtype;

struct ctrl_dict {
    size_t capacity;
    size_t filled;

    struct ctrl_bkt {
        hashtype hash;
        u16 value;
        u8 type;

        struct InputAxis *axis;
    } *bkts;
};

struct Controller {
    struct ctrl_dict codes;
    struct ctrl_dict binds;
    struct InputAxis *axes;

    struct {
        struct InputRecord records[IO_BUF_SIZE];
        size_t len;
    } input_buf;

    struct {
        size_t indexes[IO_BUF_SIZE];
        size_t len;
    } pending_buf;
};

bool NewCtrl(struct Controller *ctrl, size_t code_cap, size_t bind_cap);

void FreeCtrl(struct Controller *ctrl);

bool CtrlMap(struct Controller *ctrl, u16f code, u16f bind, u8f type);

struct InputAxis *CtrlGet(struct Controller *ctrl, u16f code, u8f type);

bool CtrlPoll(struct Controller *ctrl);