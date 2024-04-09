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
#include "event.h"

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

struct Instance {
    struct Ctrl ctrl;

    struct Delegate on_start;
    struct Delegate on_update;
    struct Delegate on_draw;

    struct Terminal term;
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

void *Update(void *args)
{
    struct Instance *game = args;

    struct TChar4 blks[game->term.area];
    game->term.blks = blks;

    while (true) {
        // Input
        CtrlPoll(&game->ctrl);

        // Game logic
        Invoke(&game->on_update, game);

        // Draw
        TermOut(&game->term);
        puts(game->term.buf);
    }
}

void Loop(struct Instance *game)
{
    pthread_t update_thread;
    pthread_create(&update_thread, nullptr, Update, (void *)game);
}

bool Start(struct Instance *game)
{
    if (!NewCtrl(&game->ctrl, code_count, code_count))
        return false;

    for (size_t i = 0; i < code_count; i++) {
        const struct CtrlBind *bind = &ctrl_binds[i];
        CtrlMap(&game->ctrl, bind->code, bind->bind, bind->type);
    }

    if (!NewTerm(&game->term, 20, 20))
        return false;
    
    return true;
}

int main()
{
    struct Instance game;
    Start(&game);

    #ifdef _WIN32
    if(!WindowsInit(&game.term)) {
        printf("FUCK");
        exit(1);
    }
    #endif
    printf("does it work");

    StartInput(&game.ctrl);
    Loop(&game);

    JoinInput(&game.ctrl);

    return 0;
}