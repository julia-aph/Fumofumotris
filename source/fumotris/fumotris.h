#include "fumocommon.h"
#include "fumoengine.h"
#include "tetr.h"

#define BINDS_N 12


enum FumotrisCode {
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


struct ControlMapping mappings_global[BINDS_N] = {
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
};

const u8 I[16] = {
    0, 0, 0, 0,
    0, 0, 0, 0,
    1, 1, 1, 1,
    0, 0, 0, 0
};

const u8 O[4] = {
    1, 1,
    1, 1
};

const u8 T[9] = {
    0, 1, 0,
    1, 1, 1,
    0, 0, 0
};

const u8 S[9] = {
    0, 1, 1,
    1, 1, 0,
    0, 0, 0
};

const u8 Z[9] = {
    1, 1, 0,
    0, 1, 1,
    0, 0, 0
};

const u8 J[9] = {
    1, 0, 0,
    1, 1, 1,
    0, 0, 0
};

const u8 L[9] = {
    0, 0, 1,
    1, 1, 1,
    0, 0, 0
};