#pragma once
#include <iso646.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "fumotris.h"
#include "term.h"

typedef struct Windows *platform;

bool WindowsInit(platform win, struct Terminal *term);

bool WinBlockInput(platform win, struct InputBuffer *buf);

bool WinWait(platform win, struct timespec relative);