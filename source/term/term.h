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

    struct TChar4 *bufs[2];
    size_t buf_size;
    u8f switch_read;
    u8f switch_write;

    pthread_mutex_t mutex;
    pthread_cond_t update;

    struct {
        u8f is_writing : 1;
        u8f resize : 1;
    } flags;
};

struct Terminal NewTerm(size_t wid, size_t hgt);

void TermSetBufs(struct Terminal *term, struct TChar4 *buf0, struct TChar4 *buf1);

void TermResize(struct Terminal *term, size_t wid, size_t hgt);

void UpdateTerm(struct Terminal *term);
bool TermWaitUpdate(struct Terminal *term);

void TermSignalSafe(struct Terminal *term);
void WaitSafeTerm(struct Terminal *term);

void TermLock(struct Terminal *term);
void TermUnlock(struct Terminal *term);

size_t TermOut(struct Terminal *term, char *buf);