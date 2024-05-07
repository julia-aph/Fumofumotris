#pragma once
#include <iso646.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "fumocommon.h"

#ifdef _WIN32
    #include "win.h"
#endif


bool PlatformInit();

bool PlatformGetRefreshRate(u16f *out);

bool PlatformReadInput(struct RecordBuffer *recs, struct StringBuffer *str);

bool PlatformStopInput();

bool PlatformWait(nsec duration);