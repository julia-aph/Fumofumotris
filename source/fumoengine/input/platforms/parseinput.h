#include <iso646.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "fumocommon.h"
#include "input.h"


void ReadButton(struct InputRecord *rec, u16f bind, bool is_down);

void ReadAxis(struct InputRecord *rec, u16f bind, u64 value);

void ReadJoystick(struct InputRecord *rec, u16f bind, i32 x, i32 y);

size_t UCS2ToUTF8(char *buf, u16f ucs2);