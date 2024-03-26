#pragma once
#include <iso646.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "ctrl.h"
#include "fumotris.h"

struct InputBuffer {
    struct InputRecord records[IO_BUF_SIZE];
    size_t count;
    pthread_mutex_t mutex;
};

void StartInput(Ctrl *ctrl);