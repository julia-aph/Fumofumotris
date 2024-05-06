#pragma once
#include "ctrl.h"
#include "event.h"
#include "fumocommon.h"
#include "input.h"


struct FumoGame {
    struct Controller ctrl;
    struct InputHandle input_hand;

    struct Event start;
    struct Event draw;
    struct Event update;

    Time time;
};


void Panic(char *message);

bool FumoInit(struct FumoGame *game);

