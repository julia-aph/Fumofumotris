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

struct Terminal NewTerm(struct TChar4 *blks, size_t wid, size_t hgt)
{
    size_t area = wid * hgt;
    memset(blks, 0, sizeof(struct TChar4) * area);

    return (struct Terminal) {
        .wid = wid,
        .hgt = hgt,
        .area = area,
        .refresh_rate = 60,
        .blks = blks,
        
        .mutex = PTHREAD_MUTEX_INITIALIZER,
        .is_initialized = PTHREAD_COND_INITIALIZER,
        .draw_ready = PTHREAD_COND_INITIALIZER
    };
}

size_t TermBufSize(struct Terminal *term)
{
    static const size_t max_color_str_len = 10;
    static const size_t reset_str_len = 7;

    return reset_str_len
        + (max_color_str_len + 1) * term->area
        + (term->hgt - 1)
        + 1;
}

size_t printcol4(char *buf, size_t at, size_t max, u8f col, char ch)
{
    if (col < 8)
        col += 30;
    else
        col += 82;

    return snprintf(buf + at, max - at, "\x1b[%um%c", col, ch);
}

size_t printblk4(char *buf, size_t at, size_t max, struct TChar4 *blk)
{
    u8f bg;
    if (blk->bg < 8)
        bg = blk->bg + 40;
    else
        bg = blk->bg + 92;
    
    u8f fg;
    if (blk->fg < 8)
        fg = blk->fg + 30;
    else
        fg = blk->fg + 82;

    return snprintf(buf + at, max - at, "\x1b[%u;%um%c", bg, fg, blk->ch);
}

size_t TermOut(struct Terminal *term, char *buf, size_t n)
{
    u8f last_bg = 0;
    u8f last_fg = 0;

    size_t filled = snprintf(buf, n, "\x1b[H\x1b[0m");

    for(size_t y = 0; y < term->hgt; y++) {
    for(size_t x = 0; x < term->wid; x++) {
        size_t i = y * term->wid + x;
        struct TChar4 *blk = &term->blks[i];
        
        // DEBUG
        if (blk->ch == 0)
            blk->ch = '#';
        // DEBUG
        
        if (blk->bg != 0 and blk->bg != last_bg) {
            last_bg = blk->bg;
            if (blk->fg != 0 and blk->fg != last_fg) {
                filled += printblk4(buf, filled, n, blk);
                last_fg = blk->fg;
            } else {
                filled += printcol4(buf, filled, n, blk->bg, blk->ch);
            }
        } else if (blk->fg != 0 and blk->fg != last_fg) {
            filled += printcol4(buf, filled, n, blk->fg, blk->ch);
            last_fg = blk->fg;
        } else {
            buf[filled] = blk->ch;
            filled += 1;
        }
    }
        buf[filled] = '\n';
        filled += 1;
    }
    buf[filled] = 0;
    return filled;
}