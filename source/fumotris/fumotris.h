#include "fumocommon.h"
#include "fumoengine.h"
#include "tetr.h"

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


u16 controls_g[BINDS_N] = {
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

u16 codes_g[BINDS_N] = {
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

u16 types_g[BINDS_N] = {
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

u8 I[16] = {
    0, 0, 0, 0,
    0, 0, 0, 0,
    1, 1, 1, 1,
    0, 0, 0, 0
};

u8 O[4] = {
    1, 1,
    1, 1
};

u8 T[9] = {
    0, 1, 0,
    1, 1, 1,
    0, 0, 0
};

u8 S[9] = {
    0, 1, 1,
    1, 1, 0,
    0, 0, 0
};

u8 Z[9] = {
    1, 1, 0,
    0, 1, 1,
    0, 0, 0
};

u8 J[9] = {
    1, 0, 0,
    1, 1, 1,
    0, 0, 0
};

u8 L[9] = {
    0, 0, 1,
    1, 1, 1,
    0, 0, 0
};