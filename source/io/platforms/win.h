#pragma once
#include <pthread.h>
#include <iso646.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "input.h"

bool WindowsInit();

bool WindowsBlockInput(struct InputBuffer *buf);

bool WindowsWait(double seconds);