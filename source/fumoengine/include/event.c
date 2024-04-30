#include "event.h"

#define INIT_CAPACITY 16

size_t clbks_size(size_t capacity) {
    return sizeof(union func) * capacity;
}

bool CreateEvent(struct Event *event)
{
    union func *callbacks = malloc(clbks_size(INIT_CAPACITY));
    
    if (callbacks == nullptr)
        return false;

    *event = (struct Event) {
        .clbks = callbacks,
        .len = 0,
        .capacity = INIT_CAPACITY,
    };
    return true;
}

void FreeEvent(struct Event *event)
{
    free(event->clbks);
}

bool EventSubscribe(struct Event *event, union func callback)
{
    if (event->len == event->capacity) {
        size_t new_cap = event->capacity * 2;
        union func *new_clbks = realloc(event->clbks, clbks_size(new_cap));

        if (new_clbks == nullptr)
            return false;
        
        event->clbks = new_clbks;
        event->capacity = new_cap;
    }

    event->clbks[event->len++] = callback;
    return true;
}

void EventInvoke(struct Event *event, void *arg)
{
    for (size_t i = 0; i < event->len; i++) {
        event->clbks[i].generic(arg);
    }
}

void EventInvokeUpdate(struct Event *event, u64 dt)
{
    for (size_t i = 0; i < event->len; i++) {
        event->clbks[i].update(dt);
    }
}