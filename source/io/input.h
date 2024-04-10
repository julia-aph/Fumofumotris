#pragma once
#include <iso646.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "ctrl.h"
#include "fumotris.h"

struct Input {
    struct Controller ctrl;

    pthread_t thread;
    pthread_mutex_t access_mutex;
};

bool StartInput(struct Input *in);

bool JoinInput(struct Input *in);