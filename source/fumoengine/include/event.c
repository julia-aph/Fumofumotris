#include "event.h"


bool CreateEvent(struct Event *event)
{
    struct Method *methods = malloc(16 * sizeof(struct Method));
    
    if (methods == nullptr)
        return false;

    *event = (struct Event) {
        .methods = methods,
        .len = 0,
        .capacity = 16
    };

    return true;
}

void FreeEvent(struct Event *event)
{
    free(event->methods);
}

bool EventRegister(struct Event *event, callback func, void *instance)
{
    if (event->len == event->capacity) {
        usize new_size = event->capacity * 2 * sizeof(struct Method);
        struct Method *new_methods = realloc(event->methods, new_size);

        if (new_methods == nullptr)
            return false;
        
        event->methods = new_methods;
        event->capacity = new_size;
    }

    event->methods[event->len++] = (struct Method) {
        .func = func,
        .instance = instance
    };
    
    return true;
}

void EventInvoke(struct Event *event, void *state)
{
    for (usize i = 0; i < event->len; i++) {
        struct Method *method = event->methods + i;
        method->func(state, method->instance);
    }
}