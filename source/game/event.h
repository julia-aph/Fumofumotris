#pragma once
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

struct Delegate NewDelegate(size_t capacity);

void Subscribe(struct Delegate *d, void (*event)(void *args));

void Invoke(struct Delegate *d, void *args);