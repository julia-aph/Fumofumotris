#include <iso646.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "fumotris.h"
#include "input.h"

void ParseButton(struct InputRecord *rec, u16f bind, bool is_down);

void ParseAxis(struct InputRecord *rec, u16f bind, u64 value);

void ParseJoystick(struct InputRecord *rec, u16f bind, i32 x, i32 y);