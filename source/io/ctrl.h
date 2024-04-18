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
#include "hash.h"
#include "input.h"

struct CtrlAxis {
    struct timespec last_pressed;
    struct timespec last_released;

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

struct ctrl_dict {
    size_t capacity;
    size_t filled;

    struct ctrl_bkt {
        hashtype hash;
        u16 value;
        u8 type;

        struct CtrlAxis *axis;
    } *bkts;
};

struct Controller {
    struct ctrl_dict codes;
    struct ctrl_dict binds;
    struct CtrlAxis *axes;

    struct InputBuffer input_buf;

    struct {
        size_t len;
        struct CtrlAxis *axes[IO_BUF_SIZE];
    } pending_buf;
};

bool NewCtrl(struct Controller *ctrl, size_t code_cap, size_t bind_cap);

void FreeCtrl(struct Controller *ctrl);

bool CtrlMap(struct Controller *ctrl, u16f code, u16f bind, u8f type);

struct CtrlAxis *CtrlGet(struct Controller *ctrl, u16f code, u8f type);

bool CtrlPoll(struct Controller *ctrl);