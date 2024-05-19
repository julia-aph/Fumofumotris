#pragma once
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
    struct Char4 *buf;
    char *str;

    usize wid;
    usize hgt;
    
};

usize TerminalMaxOut(usize wid, usize hgt);

bool CreateTerminal(struct Terminal *term, usize wid, usize hgt);

void FreeTerminal(struct Terminal *term);

usize TerminalPrint(struct Terminal *term);

struct Char4 *TerminalGet(struct Terminal *term, usize x, usize y);