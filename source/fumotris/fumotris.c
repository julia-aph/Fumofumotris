#include "fumotris.h"


struct Fumotris {
    struct TetrMap board;
    struct TetrMap piece;
};


void FumotrisStart(void *engine, void *app)
{
    struct FumoInstance *inst = engine;
    struct Fumotris *game = app;

    ControllerMapMulti(&inst->ctrl, CODE_COUNT, mappings_global);

    CreateTetrMap(&game->board, 10, 10);
    CreateTetrMap(&game->piece, 3, 3);
    game->piece.blks = T;
}

void FumotrisUpdate(void *engine, void *app)
{
    struct FumoInstance *inst = engine;
    struct Fumotris *game = app;

    if (mappings_global[LEFT].axis->is_down)
        game->piece.x -= 1;
    if (mappings_global[RIGHT].axis->is_down)
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