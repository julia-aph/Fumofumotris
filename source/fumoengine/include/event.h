#pragma once
#include <iso646.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "fumocommon.h"


union func {
    void (*generic)(void *arg);
    void (*update)(Time dt);
};

struct Event {
    union func *clbks;
    size_t len;
    size_t capacity;
};

bool CreateEvent(struct Event *event);

bool EventSubscribe(struct Event *event, union func callback);

void EventInvoke(struct Event *event, void *arg);

void EventInvokeUpdate(struct Event *event, Time dt);