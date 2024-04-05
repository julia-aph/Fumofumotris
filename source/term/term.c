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
    u16f refresh_rate;

    size_t area;
    size_t buf_size;

    struct TChar4 *buf;
};

size_t buf_size(size_t area,  size_t hgt)
{
    static const size_t max_color_str_len = 10;
    static const size_t reset_str_len = 7;

    return reset_str_len
        + (max_color_str_len + 1) * area
        + (hgt - 1)
        + 1;
}

struct Terminal NewTerm(size_t wid, size_t hgt)
{
    size_t area = wid * hgt;

    return (struct Terminal) {
        .wid = wid,
        .hgt = hgt,
        .refresh_rate = 60,

        .area = area,
        .buf_size = 0,
    };
}

void TermSetBufs(struct Terminal *term, struct TChar4 *buf)
{
    term->buf_size = buf_size(term->area, term->hgt);
    term->buf = buf;
}

void TermResize(struct Terminal *term, size_t wid, size_t hgt)
{
    size_t area = wid * hgt;
    size_t buf_size = term_buf_size(area, hgt);
    
    term->wid = wid;
    term->hgt = hgt;

    term->area = area;
    term->buf_size = buf_size;
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

size_t u8_to_buf(char *buf, u8f x)
{
    
}

size_t print4(char *buf, struct TChar4 *blk)
{
    u8f bg = blk->bg + (blk->bg < 8 ? 40 : 92);
    u8f fg = blk->fg + (blk->fg < 8 ? 30 : 82);

    memcpy(buf, "\x1b[", 2);
    
}

size_t TermOut_(struct Terminal *term, char *buf)
{
    struct TChar4 last;

    size_t filled = 0;
    memcpy(buf, "\x1b[H\x1b[0m", 7);

}

size_t TermOut(struct Terminal *term, char *buf)
{
    u8f last_bg = 0;
    u8f last_fg = 0;

    size_t filled = snprintf(buf, term->buf_size, "\x1b[H\x1b[0m");

    for(size_t y = 0; y < term->hgt; y++) {
    for(size_t x = 0; x < term->wid; x++) {
        size_t i = y * term->wid + x;
        struct TChar4 *blk = &term->buf[i];
        
        // DEBUG
        if (blk->ch == 0)
            blk->ch = '#';
        // DEBUG
        
        if (blk->bg != 0 and blk->bg != last_bg) {
            last_bg = blk->bg;
            if (blk->fg != 0 and blk->fg != last_fg) {
                filled += printblk4(buf, filled, term->buf_size, blk);
                last_fg = blk->fg;
            } else {
                filled += printcol4(buf, filled, term->buf_size, blk->bg, blk->ch);
            }
        } else if (blk->fg != 0 and blk->fg != last_fg) {
            filled += printcol4(buf, filled, term->buf_size, blk->fg, blk->ch);
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