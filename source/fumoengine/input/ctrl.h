#pragma once
#include <iso646.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "fumocommon.h"
#include "input.h"


struct InputAxis {
    Time last_pressed;
    Time last_released;

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

struct ctrl_bkt {
    struct InputAxis *axis;
    union InputID id;
};

struct ctrl_dict {
    struct ctrl_bkt *bkts;
    u16f capacity;
    u16f filled;
};

struct ctrl_axis_vec {
    struct InputAxis *axes;
    u16f size;
    u16f len;
};

struct Controller {
    struct {
        struct InputAxis *axes[IO_BUF_SIZE];
        u8f len;
    } pending_buf;

    struct ctrl_axis_vec axis_vec;
    struct ctrl_dict codes;
    struct ctrl_dict binds;
};

struct ControlBind {
    int code;
    u16 bind;
    u16 type;
};


bool CreateController(struct Controller *ctrl);
void FreeController(struct Controller *ctrl);

bool ControllerMap(struct Controller *ctrl, u16f code, u16f bind, u16f type);

struct InputAxis *ControllerGet(struct Controller *ctrl, u16f code, u16f type);

void ControllerPoll(struct Controller *ctrl, struct RecordBuffer *recs);
