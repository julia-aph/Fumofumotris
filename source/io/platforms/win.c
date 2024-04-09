#include <iso646.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#include "input.h"
#include "winhandler.h"
#include "term.h"

bool WindowsInit(struct Terminal *term)
{
    if (!WinInitHandles())
        return false;

    if (!WinInitConsole())
        return false;

    if (!WinGetRefreshRate(&term->refresh_rate))
        return false;

    return true;
}

bool WindowsBlockInput(struct RecordBuffer *buf)
{
    return WinBlockInput(buf);
}

bool WindowsWait(double seconds)
{
    struct timespec duration = {
        .tv_sec = seconds,
        .tv_nsec = fmod(seconds, 1) * 1e9
    };

    return WinWait(duration);
}