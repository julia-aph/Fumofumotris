#include <iso646.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ctrl.h"
#include "input.h"
#include "fumotris.h"
#include "term.h"
#include "event.h"
#include "platform.h"

void ErrorExit(char *message)
{
    printf(message);
    exit(1);
}

struct Game {
    struct Event Start;
    struct Event Draw;
    struct Event Update;

    Time time;
};

int main()
{
    if (!PlatformInit())
        ErrorExit("Platform failed to initialize");

    struct Controller ctrl;
    if (!CreateCtrl(&ctrl))
        ErrorExit("Out of memory");

    CtrlMap(&ctrl, 0, BUTTON, 'A');

    struct Game game;
    CreateEvent(&game.Update);

    struct InputHandle input_hand;
    if (!BeginInputThread(&input_hand, &ctrl.recs, &ctrl.str))
        ErrorExit("Input handle failed to initialize");

    _sleep(1000000);
    while (true) {
        if (!InputAquire(&input_hand))
            ErrorExit("Aquire failed");
            
        //printf("%u\n", ctrl.recs.head.len);
        //CtrlPoll(&ctrl);

        if (!InputRelease(&input_hand))
            ErrorExit("Release failed");

        EventInvokeUpdate(&game.Update, 0);
    }

    return 0;
}