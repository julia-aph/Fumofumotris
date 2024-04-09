#pragma once
#include <pthread.h>
#include <iso646.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "input.h"
#include "term.h"

bool WindowsInit(struct Terminal *term);

bool WindowsBlockInput(struct RecordBuffer *buf);

bool WindowsWait(double seconds);