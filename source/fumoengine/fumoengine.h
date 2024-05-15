#pragma once
#include "ctrl.h"
#include "event.h"
#include "fumocommon.h"
#include "input.h"
#include "vector.h"


typedef nsec (*coroutine_handler)(void *state, void *instance);


struct Coroutine {
    coroutine_handler callback;
    void *state;
    nsec next_scheduled;
};

struct Instance {
    struct Controller ctrl;
    struct InputHandle input_hand;

    struct Event on_start;
    struct Event on_update;
    struct Event on_draw;

    struct Vector coroutines;

    nsec time;
    nsec frametime;
};


void Panic(char *message);

bool CoroutineAdd(struct Instance *inst, void *state, coroutine_handler callback);

void CoroutineTryInvoke(struct Instance *inst, struct Coroutine *co);

bool CreateFumoInstance(struct Instance *game);

bool FumoInstanceRun(struct Instance *game);