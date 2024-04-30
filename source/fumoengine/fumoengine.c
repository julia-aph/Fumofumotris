#include "ctrl.h"

#include "event.h"
#include "fumocommon.h"


void ErrorExit(char *message)
{
    printf(message);
    exit(1);
}

struct FumoGame {
    struct Controller ctrl;
    struct InputHandle input_hand;

    struct Event start;
    struct Event draw;
    struct Event update;

    Time time;
};

bool FumoEngineInit(struct FumoGame *game)
{
    if (!PlatformInit())
        ErrorExit("Platform failed to initialize");

    if (!CreateCtrl(&game->ctrl))
        ErrorExit("Out of memory");

    //CtrlMap(&ctrl, 0, BUTTON, 'A');

    CreateEvent(&game->update);

    if (!BeginInputThread(&game->input_hand, &game->input_hand.recs, &ctrl.str))
        ErrorExit("Input handle failed to initialize");

    while (true) {
        if (!InputAquire(&input_hand))
            ErrorExit("Aquire failed");
            
        CtrlPoll(&ctrl);

        if (!InputRelease(&input_hand))
            ErrorExit("Release failed");

        EventInvokeUpdate(&game.update, 0);
    }

    return 0;
}