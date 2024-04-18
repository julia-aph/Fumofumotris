#pragma once
#include <iso646.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "fumotris.h"
#include "gametime.h"
#include "input.h"

struct InputAxis {
    struct Time last_pressed;
    struct Time last_released;

    union {
        struct Button but;
        struct Axis axis;
        struct Joystick js;
        union InputData data;
    };

    union InputID id;
    u8 is_down;
    u8 is_held;
    u8 is_up;
};

static struct ctrl_bkt {
    union InputID id;
    struct InputAxis *axis;
};

static struct ctrl_dict {
    struct ctrl_bkt *bkts;

    u16f capacity;
    u16f filled;
};

static struct axis_vec {
    struct InputAxis *axes;

    u16f size;
    u16f len;
};

struct Controller {
    struct InputBuffer buf;
    struct {
        struct InputAxis *axes[IO_BUF_SIZE];
        u16f len;
    } pending_buf;

    struct axis_vec axis_vec;
    struct ctrl_dict codes;
    struct ctrl_dict binds;
};

bool NewCtrl(struct Controller *ctrl, size_t init_axes);

void FreeCtrl(struct Controller *ctrl);

bool CtrlMap(struct Controller *ctrl, u16f code, u16f type, u16f bind);

struct InputAxis *CtrlGet(struct Controller *ctrl, u16f code, u16f type);

bool CtrlPoll(struct Controller *ctrl);