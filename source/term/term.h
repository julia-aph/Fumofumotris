#pragma once
#include <iso646.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "fumotris.h"

struct TChar4 {
    char ch;
    u8 bg : 4;
    u8 fg : 4;
};

struct Terminal {
    size_t wid;
    size_t hgt;
    size_t area;
    struct TChar4 *blks;
    
    size_t buf_size;
    char *buf;

    u16f refresh_rate;
};

bool NewTerm(struct Terminal *term, size_t wid, size_t hgt);

bool ResizeTerm(struct Terminal *term, size_t wid, size_t hgt);

void FreeTerm(struct Terminal *term);

size_t TermOut(struct Terminal *term);