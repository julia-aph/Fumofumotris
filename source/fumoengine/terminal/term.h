#pragma once
#include <iso646.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "fumocommon.h"


struct Char4 {
    char ch;
    u8 bg : 4;
    u8 fg : 4;
};

struct Terminal {
    size_t wid;
    size_t hgt;
    size_t area;
    
    struct Char4 *chs;
};

bool CreateTerminal(struct Terminal *term, size_t wid, size_t hgt);

void FreeTerminal(struct Terminal *term);

size_t TerminalPrint(char *dest, size_t n, struct Terminal *term);