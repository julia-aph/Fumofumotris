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
#include "tetr.h"
#include "event.h"
#include "platform.h"

void ErrorExit(char *message)
{
    printf(message);
    exit(1);
}

int main()
{
    if (!PlatformInit())
        ErrorExit("Platform failed to initialize");

    struct Controller ctrl;
    if (!CreateCtrl(&ctrl))
        ErrorExit("Out of memory");

    struct InputThreadHandle input;
    if (!BeginInputThread(&input, &ctrl.buf))
        ErrorExit("Input handle failed to initialize");

    CtrlMap(&ctrl, 0, BUTTON, 'A');

    while (true) {
        bool poll = CtrlPoll(&ctrl);
        printf("poll: %u\n", poll);

        struct InputAxis *a = CtrlGet(&ctrl, 0, BUTTON);
        printf("get: %llu\n", a);

        printf("val: %u\n", a->but.value);
    }

    return 0;
}