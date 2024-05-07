#pragma once
#include <stdio.h>
#include <stdlib.h>

#include "fumocommon.h"


typedef void (*callback)(void *state, void *instance);

struct Method {
    callback func;
    void *instance;
};

struct Event {
    struct Method *methods;
    usize len;
    usize capacity;
};


bool CreateEvent(struct Event *event, void *state);

bool EventRegister(struct Event *event, callback func, void *instance);

void EventInvoke(struct Event *event);