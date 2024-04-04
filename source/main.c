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

struct Instance {
    struct Terminal term;
    struct Ctrl ctrl;
    struct RecordBuffer rec_buf;

    struct Delegate on_start;
    struct Delegate on_draw;
    struct Delegate on_update;
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

bool DrawUpdate(struct Terminal *term, struct Instance *game, char *buf)
{
    bool is_update;

    TermLock(term);
    {
        is_update = TermWaitUpdate(term);
        if(is_update) {
            Invoke(&game->on_draw, game);
            TermOut(term, buf);
        }
    }
    TermUnlock(term);

    return is_update;
}

void Draw(struct Terminal *term, struct Instance *game)
{
    struct TChar4 buf0[term->area];
    struct TChar4 buf1[term->area];
    
    TermLock(term);
    {
        TermSetBufs(term, buf0, buf1);
    }
    TermUnlock(term);
    TermSignalSafe(term);

    char buf[term->buf_size];
    
    while (true) {
        if (!DrawUpdate(term, game, buf))
            break;
        
        puts(buf);
    }

    
}

void Update(struct Instance *game)
{
    WaitSafeTerm(&game->term);

    while (true) {
        // Input
        CtrlPoll(&game->ctrl, &game->rec_buf);

        // Game logic
        Invoke(&game->on_update, game);

        // Draw
        UpdateTerm(&game->term);
    }
}

void Loop(struct Ctrl *ctrl, struct RecordBuffer *rec_buf)
{
    pthread_t draw_thread;
    pthread_create(&draw_thread, nullptr, Draw, &term);

    pthread_t update_thread;
    pthread_create(&update_thread, nullptr, Update, &term);
}

int main()
{
    #ifdef _WIN32
    if(!WindowsInit())
        exit(1);
    #endif

    struct ctrl_bkt code_bkts[code_count];
    struct ctrl_dict codes = {
        .capacity = code_count,
        .filled = 0,
        .bkts = code_bkts
    };
    struct ctrl_bkt bind_bkts[code_count];
    struct ctrl_dict binds = {
        .capacity = code_count,
        .filled = 0,
        .bkts = bind_bkts
    };
    struct Axis axes[code_count];
    struct Ctrl ctrl = NewCtrl(&codes, &binds, axes);

    for (size_t i = 0; i < code_count; i++) {
        const struct CtrlBind *bind = &ctrl_binds[i];
        CtrlMap(&ctrl, bind->code, bind->bind, bind->type);
    }

    struct RecordBuffer rec_buf = {
        .count = 0,
        .mutex = PTHREAD_MUTEX_INITIALIZER
    };

    struct Instance game = {
        .term = NewTerm(20, 20),
        .ctrl = NewCtrl(),

        .on_start = NewDelegate(16),
        .on_draw = NewDelegate(16),
        .on_update = NewDelegate(16)
    };

    StartInput(&ctrl, &rec_buf);
    Loop(&ctrl, &rec_buf);

    JoinInput(&ctrl);

    return 0;
}