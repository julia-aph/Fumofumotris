#pragma once
#include <iso646.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fumotris.h"

struct CharBlk4 {
    char ch;
    u8 bg : 4;
    u8 fg : 4;
};

struct TermBuf {
    size_t wid;
    size_t hgt;
    size_t area;
    struct CharBlk4 *blks;
};

struct TermBuf NewTermBuf(size_t wid, size_t hgt);

size_t TermMaxChars(struct TermBuf *term);

size_t TermBufToChars(struct TermBuf *term, char *buf, size_t max_chars);