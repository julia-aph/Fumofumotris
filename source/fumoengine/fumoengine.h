#pragma once
#include "ctrl.h"
#include "event.h"
#include "fumocommon.h"
#include "input.h"
#include "terminal.h"
#include "vector.h"


struct FumoInstance {
    struct Controller ctrl;
    struct InputHandle input_hand;
    struct Terminal term;

    struct Event on_start;
    struct Event on_update;

    nsec time;
    nsec frametime;
};


void Panic(char *message);

bool CreateFumoInstance(struct FumoInstance *game);

bool FumoInstanceRun(struct FumoInstance *game);