#include <iso646.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "input.h"
#include "instance.h"
#include "winhandler.h"

bool WindowsInit(struct Window *window)
{
    if (!WinInitInputHandle())
        return false;

    if (!WinInitTimer())
        return false;

    if (!WinInitConsole())
        return false;

    

    return true;
}

bool WindowsBlockInput(struct RecordBuffer *buf)
{
    return WinBlockInput(buf);
}

bool WindowsWait(double seconds)
{
    return WinWait(seconds);
}