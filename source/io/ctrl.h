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

enum CtrlType {
    KEY,
    AXIS,
    JOYSTICK,
    ESCAPE
};

struct Record {
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

struct RecordBuffer {
    struct Record records[IO_BUF_SIZE];
    size_t count;
    pthread_mutex_t mutex;
};

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

struct ctrl_bkt {
    hashtype hash;
    u16 value;
    u8 type;
    struct Axis *axis;
};

struct ctrl_dict {
    size_t capacity;
    size_t filled;
    struct ctrl_bkt *bkts;
};

struct Ctrl {
    struct ctrl_dict codes;
    struct ctrl_dict binds;

    pthread_t thread;
    pthread_mutex_t mutex;
};

struct Ctrl NewCtrl();

bool CtrlMap(struct Ctrl *ctrl, u16f code, u16f bind, u8f type);

struct Axis *CtrlGet(struct Ctrl *ctrl, u16f code, u8f type);

bool CtrlPoll(struct Ctrl *ctrl, struct RecordBuffer *buf);