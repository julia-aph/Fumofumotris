#include "fumocommon.h"
#include "fumoengine.h"
#include "terminal.h"
#include "tetra.h"

#define BINDS_N 12


enum FumotrisControls {
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


u16 CONTROLS[BINDS_N] = {
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

    MOUSE,
};

u16 CODES[BINDS_N] = {
    0x25,
    0x27,
    0x28,
    0x20,
    'Z',
    'X',
    'A',
    'C',
    0x1B,

    0,
    1,

    0
};

u16 TYPES[BINDS_N] = {
    BUTTON,
    BUTTON,
    BUTTON,
    BUTTON,
    BUTTON,
    BUTTON,
    BUTTON,
    BUTTON,
    BUTTON,

    AXIS,
    AXIS,

    JOYSTICK
};

struct TetraTemplate I = {
    .blks = (u8[16]) {
        0, 0, 0, 0,
        0, 0, 0, 0,
        1, 1, 1, 1,
        0, 0, 0, 0
    },
    .wid = 4,
    .hgt = 4
};

struct TetraTemplate O = {
    .blks = (u8[4]) {
        2, 2,
        2, 2
    },
    .wid = 2,
    .hgt = 2
};

struct TetraTemplate T = {
    .blks = (u8[9]) {
        0, 3, 0,
        3, 3, 3,
        0, 0, 0
    },
    .wid = 3,
    .hgt = 3
};

struct TetraTemplate S = {
    .blks = (u8[9]) {
        0, 4, 4,
        4, 4, 0,
        0, 0, 0
    },
    .wid = 3,
    .hgt = 3
};

struct TetraTemplate Z = {
    .blks = (u8[9]) {
        5, 5, 0,
        0, 5, 5,
        0, 0, 0
    },
    .wid = 3,
    .hgt = 3
};

struct TetraTemplate J = {
    .blks = (u8[9]) {
        6, 0, 0,
        6, 6, 6,
        0, 0, 0
    },
    .wid = 3,
    .hgt = 3
};

struct TetraTemplate L = {
    .blks = (u8[9]) {
        0, 0, 7,
        7, 7, 7,
        0, 0, 0
    },
    .wid = 3,
    .hgt = 3
};

struct TetraTemplate *templates[7] = { &I, &O, &T, &S, &Z, &J, &L };