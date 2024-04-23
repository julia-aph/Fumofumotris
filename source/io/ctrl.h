#pragma once
#include <iso646.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "fumotris.h"
#include "gametime.h"
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
    u8 is_down;
    u8 is_held;
    u8 is_up;
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
    struct InputBuffer buf;
    
    struct {
        struct InputAxis *axes[IO_BUF_SIZE];
        u8f len;
    } pending_buf;

    struct ctrl_axis_vec axis_vec;
    struct ctrl_dict codes;
    struct ctrl_dict binds;
};

bool CreateCtrl(struct Controller *ctrl);

void FreeCtrl(struct Controller *ctrl);

bool CtrlMap(struct Controller *ctrl, u16f code, u16f type, u16f bind);

struct InputAxis *CtrlGet(struct Controller *ctrl, u16f code, u16f type);

bool CtrlPoll(struct Controller *ctrl, struct InputThreadHandle *hand);

enum ControlCode {
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

struct ControlBind {
    enum ControlCode code;
    u16 bind;
    u8 type;
};

#define CODE_COUNT 12
/*const struct ControlBind ctrl_binds[12] = {
    { LEFT, 0x25, BUTTON },
    { RIGHT, 0x27, BUTTON },
    { SOFT_DROP, 0x28, BUTTON },
    { HARD_DROP, 0x20, BUTTON },
    { ROTATE_CCW, 'Z', BUTTON },
    { ROTATE_CW, 'X', BUTTON },
    { ROTATE_180, 'A', BUTTON },
    { SWAP, 'C', BUTTON },
    { ESC, 0x1B, BUTTON },
    { VSCROLL, 0, AXIS },
    { HSCROLL, 1, AXIS },
    { MOUSE, 0, JOYSTICK }
};*/