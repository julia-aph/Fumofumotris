#include <iso646.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "input.h"
#include "winhandler.h"

bool WindowsInit()
{
    if (!WinInitInputHandle())
        return false;

    if (!WinInitTimer())
        return false;

    if(!WinInitConsole())
        return false;

    return true;
}

bool WindowsBlockInput(struct InputResult *result)
{
    return WinBlockInput(result);
}

bool WindowsWait(double seconds)
{
    return WinWait(seconds);
}