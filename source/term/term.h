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
    u16f refresh_rate;
    struct TChar4 *blks;

    pthread_mutex_t mutex;
    pthread_cond_t is_initialized;
    pthread_cond_t draw_ready;
};

struct Terminal NewTerm(struct TChar4 *blks, size_t wid, size_t hgt);

size_t TermBufSize(struct Terminal *term);

size_t TermOut(struct Terminal *term, char *buf, size_t max_chars);