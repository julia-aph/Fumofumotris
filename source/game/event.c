#include <iso646.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "fumotris.h"

struct Delegate {
    size_t len;
    size_t capacity;
    void (**events)(void *args);
};

inline size_t delegate_size(size_t capacity)
{
    return sizeof(void(*)(void *)) * capacity;
}

struct Delegate NewDelegate(size_t capacity)
{
    return (struct Delegate) {
        .len = 0,
        .capacity = capacity,
        .events = malloc(delegate_size(capacity))
    };
}

void Subscribe(struct Delegate *d, void (*event)(void *args))
{
    if (d->len == d->capacity) {
        d->capacity *= 2;
        d->events = realloc(d->events, delegate_size(d->capacity));
    }

    d->events[d->len] = event;
    d->len += 1;
}

void Invoke(struct Delegate *d, void *args)
{
    for (size_t i = 0; i < d->len; i++) {
        d->events[i](args);
    }
}