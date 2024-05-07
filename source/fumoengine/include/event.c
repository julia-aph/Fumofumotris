#include "event.h"

#define INIT_CAPACITY 16


bool CreateEvent(struct Event *event)
{
    void_func *callbacks = malloc(INIT_CAPACITY * sizeof(void_func));
    
    if (callbacks == nullptr)
        return false;

    *event = (struct Event) {
        .callbacks = callbacks,
        .len = 0,
        .capacity = INIT_CAPACITY
    };

    return true;
}

void FreeEvent(struct Event *event)
{
    free(event->callbacks);
}

bool EventRegister(struct Event *event, void_func callback)
{
    if (event->len == event->capacity) {
        usize new_size = event->capacity * 2 * sizeof(void_func);
        void_func *new_callbacks = realloc(event->callbacks, new_size);

        if (new_callbacks == nullptr)
            return false;
        
        event->callbacks = new_callbacks;
        event->capacity = new_size;
    }

    event->callbacks[event->len++] = callback;
    
    return true;
}

void EventInvoke(struct Event *event, void *args)
{
    for (usize i = 0; i < event->len; i++) {
        event->callbacks[i](args);
    }
}