#include "fumotris.h"


struct Fumotris {
    struct ControlMapping mappings[BINDS_N];
    struct ControlAxis *input[BINDS_N];

    struct TetrMap board;
    struct TetrMap piece;
};


void FumotrisStart(void *inst_arg, void *game_arg)
{
    struct FumoInstance *inst = inst_arg;
    struct Fumotris *game = game_arg;

    ControllerMapMulti(&inst->ctrl, BINDS_N, game->mappings, game->input);

    CreateTetrMap(&game->board, 10, 10);
    CreateTetrMap(&game->piece, 3, 3);
    game->piece.blks = T;
}

void FumotrisUpdate(void *inst_arg, void *game_arg)
{
    struct FumoInstance *inst = inst_arg;
    struct Fumotris *game = game_arg;

    if (game->input[LEFT]->is_down)
        game->piece.x -= 1;
    if (game->input[RIGHT]->is_down)
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