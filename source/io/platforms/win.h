#pragma once
#include <iso646.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "fumotris.h"

bool PlatformInit();

bool PlatformGetRefreshRate(u16f *out);

bool PlatformReadInput(struct InputBuffer *buf);

bool PlatformWait(struct timespec relative);