#include "fumotris.h"


void Update(struct FumoInstance *instance, void *args)
{
    struct Fumotris *game = args;

    TetrMapDraw(&game->board, &instance->term);
}

bool CreateFumotris(struct Fumotris *game)
{
    if (!CreateTetrMap(&game->board, 10, 10))
        return false;

    return true;
}

int main()
{
    struct FumoInstance instance;
    CreateFumoInstance(&instance);
    
    struct Fumotris game;
    CreateFumotris(&game);

    ControllerMapMulti(&instance.ctrl, CODE_COUNT, MAPPINGS);

    FumoInstanceHook(&instance.on_update, Update, &game);


    FumoInstanceRun(&instance);

    return 0;
}