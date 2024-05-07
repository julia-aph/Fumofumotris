#pragma once
#include <iso646.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "fumocommon.h"


struct Color4 {
    u8 bg : 4;
    u8 fg : 4;
};

struct Char4 {
    struct Color4 color;
    char ch;
};

struct Terminal {
    usize wid;
    usize hgt;
    
    struct Char4 *ch4s;
};

bool CreateTerminal(struct Terminal *term, usize wid, usize hgt);

void FreeTerminal(struct Terminal *term);

usize TerminalPrint(char *buf, usize n, struct Terminal *term);