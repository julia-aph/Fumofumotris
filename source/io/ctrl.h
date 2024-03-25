#pragma once
#include <iso646.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fumotris.h"

#define IO_BUF_SIZE 8

enum InputType {
    KEY,
    AXIS,
    JOYSTICK,
    WINDOW,
    ESCAPE
};

struct InputRecord {
    enum InputType type;
    u16 id;

    union {
        struct {
            u32 value;
            bool is_down;
        } axis;
        struct {
            u32 x;
            u32 y;
        } joystick;
    } data;

    double timestamp;
};

struct InputResult {
    size_t count;
    struct InputRecord buf[IO_BUF_SIZE];
};

struct Axis {
    union {
        struct {
            u32 value;
            bool is_down;
        } axis;
        struct {
            u32 x;
            u32 y;
        } joystick;
    } data;

    double last_pressed;
    double last_released;
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

struct ctrl_bkt {
    hashtype bind_hash;
    size_t index;
    hashtype code_hash;

    struct Axis axis;
};

struct Ctrl {
    size_t capacity;
    size_t filled;
    struct ctrl_bkt *bkts;

    pthread_mutex_t mutex;
};
typedef struct Ctrl Ctrl;

Ctrl NewCtrl(struct ctrl_bkt *bkts_prealloc, size_t capacity);

bool CtrlMap(Ctrl *ctrl, u16f bind, u16f code, enum InputType type);

struct Axis *CtrlGet(Ctrl *ctrl, u16f code, enum InputType type);

bool CtrlUpdateKey(Ctrl *ctrl, struct InputRecord *record);

bool CtrlUpdateAxis(Ctrl *ctrl, struct InputRecord *record);

bool CtrlUpdateJoystick(Ctrl *ctrl, struct InputRecord *record);

bool CtrlUpdateWindow(Ctrl *ctrl, struct InputRecord *record);

