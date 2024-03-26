#pragma once
#include <iso646.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "fumotris.h"

bool WinInitInputHandle();

bool WinInitTimer();

bool WinInitConsole();

bool WinBlockInput(struct InputBuffer *buf);

bool WinWait(double seconds);