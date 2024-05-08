#pragma once
#include <stdio.h>
#include <stdlib.h>

#include "fumocommon.h"


typedef void (*handler)(void *state, void *instance);

struct Method {
    handler callback;
    void *instance;
};

struct Event {
    struct Method *methods;
    usize len;
    usize capacity;
};


bool CreateEvent(struct Event *event);

bool EventAdd(struct Event *event, handler callback, void *instance);

void EventInvoke(struct Event *event, void *state);