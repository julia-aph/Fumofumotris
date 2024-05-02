#include "fumoengine.h"
#include "platform.h"


void ErrorExit(char *message)
{
    printf(message);
    exit(1);
}

bool FumoInit(struct FumoGame *game)
{
    if (!PlatformInit())
        ErrorExit("Platform failed to initialize");

    if (!CreateController(&game->ctrl))
        ErrorExit("Out of memory");

    if (!CreateEvent(&game->update))
        ErrorExit("Out of memory");

    if (!BeginInputThread(&game->input_hand))
        ErrorExit("Input handle failed to initialize");

    return 0;
}