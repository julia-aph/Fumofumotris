#include "fumoengine.h"
#include "platform.h"


void Panic(char *message)
{
    printf(message);
    exit(1);
}

void InvokeUpdate(void_func func, void *arg)
{
    struct { nsec frametime; } *args = arg;

    ((update_func)func)(args->frametime);
}

bool FumoInit(struct FumoGame *game)
{
    if (!PlatformInit())
        Panic("Platform failed to initialize");

    if (!CreateController(&game->ctrl))
        Panic("Out of memory");

    if (!CreateEvent(&game->update))
        Panic("Out of memory");

    if (!BeginInputThread(&game->input_hand))
        Panic("Input handle failed to initialize");

    return 0;
}