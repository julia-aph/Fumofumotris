#include "fumotris.h"
#include <stdlib.h>


struct Fumotris {
    struct Tetra board;
    struct Tetra piece;

    bool is_ground;
};


struct TetraTemplate *bag[7] = { &I, &O, &T, &S, &Z, &J, &L };
usize bag_i = 0;


void shuffle()
{
    for (usize i = 6; i >= 0; i--) {
        usize swap = rand() % i;
        
        struct TetraTemplate *tmp = bag[swap];
        bag[swap] = bag[i];
        bag[i] = tmp;
    }
}

void place_piece(struct Fumotris *fumo)
{
    TetraOverlay(&fumo->piece, &fumo->board);
    SetTetra(&fumo->piece, bag[bag_i++], 0, 0);

    if (bag_i == 7) {
        shuffle();
        bag_i = 0;
    }
}

i16 get_horizontal(struct Controller *ctrl)
{
    return (-(i16)ctrl->axes[LEFT].is_down) + ctrl->axes[RIGHT].is_down;
}

void FumotrisStart(struct FumoInstance *inst, struct Fumotris *fumo)
{
    ControllerBindMulti(&inst->ctrl, BINDS_N, controls_g, codes_g, types_g);
    CreateTetra(&fumo->board, 10, 10);



    fumo->is_ground = false;

    SetTetra(&fumo->piece, bag[bag_i++], 0, 0);
}

void FumotrisUpdate(struct FumoInstance *inst, struct Fumotris *fumo)
{
    TetraMove(&fumo->piece, &fumo->board, get_horizontal(&inst->ctrl), 0);

    if (inst->ctrl.axes[SOFT_DROP].is_down) {
        TetraMove(&fumo->piece, &fumo->board, 0, 1);
    }
        
    if (inst->ctrl.axes[HARD_DROP].is_down) {
        while (TetraMove(&fumo->piece, &fumo->board, 0, 1));
        place_piece(fumo);
    }
}

void FumotrisOnFall(struct FumoInstance *inst, struct Fumotris *fumo)
{
    if (!TetraMove(&fumo->piece, &fumo->board, 0, 1)) {
        if (!fumo->is_ground)
            fumo->is_ground = true;
        else
            place_piece(fumo);
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
    EventAdd(&inst.on_draw, FumotrisDraw, &game);

    VectorAdd(FUMOCO_T, &inst.coroutines, );

    FumoInstanceRun(&inst);

    return 0;
}