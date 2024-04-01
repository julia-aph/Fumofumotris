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

struct TermBuf NewTermBuf(size_t wid, size_t hgt)
{
    return (struct TermBuf) {
        wid, hgt, wid * hgt
    };
}

size_t TermMaxChars(struct TermBuf *term)
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

size_t printblk4(char *buf, size_t at, size_t max, struct CharBlk4 *blk)
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

size_t TermBufToChars(struct TermBuf *term, char *buf, size_t max_chars)
{
    u8f last_bg = 0;
    u8f last_fg = 0;

    //size_t filled = snprintf(buf, max_chars, "\x1b[H\x1b[0m");
    size_t filled = snprintf(buf, max_chars, "\x1b[0m");

    for(size_t y = 0; y < term->hgt; y++) {
    for(size_t x = 0; x < term->wid; x++) {
        size_t i = y * term->wid + x;
        struct CharBlk4 *blk = &term->blks[i];
        
        // DEBUG
        if (blk->ch == 0)
            blk->ch = '#';
        // DEBUG
        
        if (blk->bg != 0 and blk->bg != last_bg) {
            last_bg = blk->bg;
            if (blk->fg != 0 and blk->fg != last_fg) {
                filled += printblk4(buf, filled, max_chars, blk);
                last_fg = blk->fg;
            } else {
                filled += printcol4(buf, filled, max_chars, blk->bg, blk->ch);
            }
        } else if (blk->fg != 0 and blk->fg != last_fg) {
            filled += printcol4(buf, filled, max_chars, blk->fg, blk->ch);
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

/*int main()
{
    struct TermBuf term;
    term.wid = 20;
    term.hgt = 10;
    term.area = 20 * 10;
    struct CharBlk4 blks[term.area];
    memset(&blks, 0, sizeof(struct CharBlk4) * term.area);
    term.blks = blks;

    size_t out_max = TermMaxChars(&term);
    char out[out_max];
    memset(out, 0, out_max);

    TermBufToChars(&term, out, out_max);

    puts(out);

    return 0;
}*/