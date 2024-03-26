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

const enum KeyCode key_codes[] = {
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
const u16f key_binds[] = {
    0x25,
    0x27,
    0x28,
    0x20,
    'Z',
    'X',
    'A',
    'C',
    0x1B
};

const enum AxisCode axis_codes[] = {
    VSCROLL,
    HSCROLL
};
const u16f axis_binds[] = { 0, 1 };

u8 I[16] = {
    0, 0, 0, 0,
    0, 0, 0, 0,
    1, 1, 1, 1,
    0, 0, 0, 0
};

void Loop(Ctrl *ctrl)
{
    struct TermBuf term = NewTermBuf(20, 20);
    struct CharBlk4 term_blks[term.area];
    memset(term_blks, 0, sizeof(struct CharBlk4) * term.area);
    term.blks = term_blks;

    size_t out_max = TermMaxChars(&term);
    char out[out_max];
    memset(out, 0, out_max);

    struct TetrMap board = NewTetrMap(10, 20);
    u8 board_blks[board.area];
    memset(board_blks, 0, board.area);
    board.blks = board_blks;

    struct TetrMap falling = NewTetrMap(4, 4);
    u8 falling_blks[falling.area];
    memcpy(falling_blks, I, falling.area);
    falling.blks = falling_blks;

    for (int i = 0; i < 7779997; i++) {
        

        TetrMapToTermBuf(&board, &term);
        TetrMapToTermBuf(&falling, &term);

        size_t size = TermBufToChars(&term, out, out_max);
        puts(out);

        WindowsWait(0.1);
    }
}

int main()
{
    WindowsInit();

    struct ctrl_bkt bkts[13];
    Ctrl ctrl = NewCtrl(bkts, 13);

    for (size_t i = 0; i < 9; i++) {
        CtrlMap(&ctrl, key_binds[i], key_codes[i], KEY);
    }
    for (size_t i = 0; i < 2; i++) {
        CtrlMap(&ctrl, axis_codes[i], axis_binds[i], AXIS);
    }
    CtrlMap(&ctrl, 0, MOUSE, JOYSTICK);
    CtrlMap(&ctrl, 0, 0, WINDOW);

    printf("set controls\n");

    StartInput(&ctrl);
    Loop(&ctrl);

    return 0;
}