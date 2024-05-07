#include "fumocommon.h"
#include "fumoengine.h"


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

#define CODE_COUNT 12

const struct ControlBind ctrl_binds[12] = {
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

void Loop(struct FumoGame *game)
{
    while (true) {
        game->time = TimeNow();

        if (!InputAquire(&game->input_hand))
            Panic("Aquire failed");
            
        ControllerPoll(&game->ctrl, &game->input_hand.recs);

        if (!InputRelease(&game->input_hand))
            Panic("Release failed");

        

        EventInvoke(&game->update, 0);

        _sleep(100);
    }
}

int main()
{
    struct FumoGame game;
    FumoInit(&game);
    
    for (size_t i = 0; i < CODE_COUNT; i++) {
        const struct ControlBind *bind = &ctrl_binds[i];
        ControllerMap(&game.ctrl, bind->code, bind->bind, bind->type);
    }

    Loop(&game);

    return 0;
}


/*const u8 I[16] = {
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
};*/