#pragma once
#include <iso646.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "input.h"

bool WindowsInit();

bool WindowsBlockInput(struct InputResult *result);

bool WindowsWait(double seconds);