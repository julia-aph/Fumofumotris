#include "fumotris.h"


struct Fumotris {
    struct Tetra board;
    struct Tetra piece;
    nsec timer;
    bool is_ground;
};


void FumotrisStart(struct FumoInstance *inst, struct Fumotris *fumo)
{
    ControllerBindMulti(&inst->ctrl, BINDS_N, controls_g, codes_g, types_g);

    CreateTetra(&fumo->board, 10, 10);
    SetTetra(&fumo->piece, T, 3, 3, 0, 0);
    fumo->timer = 0;
    fumo->is_ground = false;
}

void FumotrisUpdate(struct FumoInstance *inst, struct Fumotris *fumo)
{
    i16 horizontal = 0;
    if (inst->ctrl.axes[LEFT].is_down)
        horizontal -= 1;
    if (inst->ctrl.axes[RIGHT].is_down)
        horizontal += 1;
    TetraMove(&fumo->piece, &fumo->board, horizontal, 0);


    if (inst->ctrl.axes[SOFT_DROP].is_down)
        TetraMove(&fumo->piece, &fumo->board, 0, 1);
        
        
    if (inst->ctrl.axes[HARD_DROP].is_down) {
        while (TetraMove(&fumo->piece, &fumo->board, 0, 1));
        
        fumo->timer = 0;
        TetraOverlay(&fumo->piece, &fumo->board);
        SetTetra(&fumo->piece, I, 4, 4, 0, 0);

        return;
    }

    fumo->timer += inst->frametime;
    while (fumo->timer > 5e8) {
        fumo->timer -= 5e8;

        if (!TetraMove(&fumo->piece, &fumo->board, 0, 1)) {
            if (!fumo->is_ground) {
                fumo->is_ground = true;
            } else {
                TetraOverlay(&fumo->piece, &fumo->board);
                SetTetra(&fumo->piece, I, 4, 4, 0, 0);

                fumo->is_ground = false;
            }
        }
    }
}

void FumotrisDraw(struct FumoInstance *inst, struct Fumotris *fumo)
{
    TetraTerminalClear(&fumo->board, &inst->term);
    TetraTerminalDraw(&fumo->board, &inst->term);
    TetraTerminalDraw(&fumo->piece, &inst->term);
}

int main()
{
    struct FumoInstance inst;
    CreateFumoInstance(&inst);
    
    struct Fumotris game;
    EventAdd(&inst.on_start, FumotrisStart, &game);
    EventAdd(&inst.on_update, FumotrisUpdate, &game);

    FumoInstanceRun(&inst);

    return 0;
}