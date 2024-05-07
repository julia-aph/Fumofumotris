#pragma once
#include <iso646.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "fumocommon.h"


typedef void (*void_func)(void *arg);

typedef void (*invoker_func)(void_func func, void *arg);


struct Event {
    void_func *callbacks;
    usize len;
    usize capacity;
};


bool CreateEvent(struct Event *event);

bool EventRegister(struct Event *event, void_func callback);

void EventInvoke(struct Event *event, void *args);