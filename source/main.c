#include <iso646.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "control.h"
#include "fumotris.h"
#include "term.h"
#include "tetr.h"

#ifdef _WIN32
#include "win.h"
#endif

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

struct CtrlBind {
    enum CtrlCode code;
    u16 bind;
    u8 type;
};

const size_t code_count = 12;
const struct CtrlBind ctrl_binds[12] = {
    { LEFT, 0x25, KEY },
    { RIGHT, 0x27, KEY },
    { SOFT_DROP, 0x28, KEY },
    { HARD_DROP, 0x20, KEY },
    { ROTATE_CCW, 'Z', KEY },
    { ROTATE_CW, 'X', KEY },
    { ROTATE_180, 'A', KEY },
    { SWAP, 'C', KEY },
    { ESC, 0x1B, KEY },
    { VSCROLL, 0, AXIS },
    { HSCROLL, 1, AXIS },
    { MOUSE, 0, JOYSTICK }
};

void Update(Ctrl *ctrl)
{

}

void Loop(Ctrl *ctrl, struct RecordBuffer *in_buf)
{
    struct CharBlk4 term_blks[20 * 20];
    struct TermBuf term = NewTermBuf(term_blks, 20, 20);

    size_t out_max = TermMaxChars(&term);
    char out[out_max];

    u8 board_blks[10 * 20];
    struct TetrMap board = NewTetrMap(board_blks, 10, 20);

    u8 falling_blks[4 * 4];
    struct TetrMap falling = NewTetrMap(falling_blks, 4, 4);
    memcpy(falling_blks, I, falling.area);

    for (int i = 0; i < 7779997; i++) {
        CtrlPoll(ctrl, in_buf);

        TetrMapToTermBuf(&board, &term);
        TetrMapToTermBuf(&falling, &term);

        TermBufToChars(&term, out, out_max);
        puts(out);

        WindowsWait(1.0/30);
    }
}

int main()
{
    #ifdef _WIN32
    if(!WindowsInit())
        exit(1);
    #endif

    struct bkt code_bkts[code_count];
    struct dict codes = {
        .capacity = code_count,
        .filled = 0,
        .bkts = code_bkts
    };
    struct bkt bind_bkts[code_count];
    struct dict binds = {
        .capacity = code_count,
        .filled = 0,
        .bkts = bind_bkts
    };
    struct Axis axes[code_count];
    Ctrl ctrl = NewCtrl(&codes, &binds, axes);

    for (size_t i = 0; i < code_count; i++) {
        const struct CtrlBind *bind = &ctrl_binds[i];
        CtrlMap(&ctrl, bind->code, bind->bind, bind->type);
    }

    struct RecordBuffer in_buf = {
        .count = 0,
        .mutex = PTHREAD_MUTEX_INITIALIZER
    };

    StartInput(&ctrl, &in_buf);
    Loop(&ctrl, &in_buf);
    JoinInput(&ctrl);

    return 0;
}