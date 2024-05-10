#include "fumotris.h"


struct Fumotris {
    struct TetrMap board;
    struct TetrMap piece;
};


void FumotrisStart(void *inst_arg, void *game_arg)
{
    struct FumoInstance *inst = inst_arg;
    struct Fumotris *game = game_arg;

    ControllerBindMulti(&inst->ctrl, BINDS_N, controls_g, codes_g, types_g);

    CreateTetrMap(&game->board, 10, 10);
    CreateTetrMap(&game->piece, 3, 3);
    game->piece.blks = T;
}

void FumotrisUpdate(void *inst_arg, void *game_arg)
{
    struct FumoInstance *inst = inst_arg;
    struct Fumotris *game = game_arg;

    if (inst->ctrl.axes[LEFT].is_down)
        game->piece.x -= 1;
        
    if (inst->ctrl.axes[RIGHT].is_down)
        game->piece.x += 1;

    TetrMapDraw(&game->board, &inst->term);
    TetrMapDraw(&game->piece, &inst->term);
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