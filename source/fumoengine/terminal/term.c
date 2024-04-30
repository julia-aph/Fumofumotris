#include <iso646.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fumocommon.h"


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

size_t term_buf_size(size_t area,  size_t hgt)
{
    static const size_t reset_str_len = 7;
    static const size_t max_color_str_len = 10;

    return reset_str_len + (max_color_str_len + 1) * area + hgt + 1;
}

struct TChar4 *alloc_blks(size_t area)
{
    return calloc(area, sizeof(struct TChar4));
}

char *alloc_buf(size_t buf_size)
{
    return malloc(buf_size);
}

bool NewTerm(struct Terminal *term, size_t wid, size_t hgt)
{
    size_t area = wid * hgt;
    size_t buf_size = term_buf_size(area, hgt);

    struct TChar4 *blks = alloc_blks(area);
    char *buf = alloc_buf(buf_size);
    
    if (blks == nullptr or buf == nullptr)
        return false;

    *term = (struct Terminal) {
        .wid = wid,
        .hgt = hgt,
        .area = area,
        .blks = blks,

        .buf_size = buf_size,
        .buf = buf,

        .refresh_rate = 60,
    };
    return true;
}

bool ResizeTerm(struct Terminal *term, size_t wid, size_t hgt)
{
    size_t area = wid * hgt;
    size_t buf_size = term_buf_size(area, hgt);
    
    struct TChar4 *tchs = realloc(term->blks, area * sizeof(struct TChar4));
    char *buf = realloc(term->buf, buf_size);
    
    if (tchs == nullptr or buf == nullptr)
        return false;

    term->blks = tchs;
    term->buf = buf;
    return true;
}

void FreeTerm(struct Terminal *term)
{
    free(term->blks);
    free(term->buf);
}

size_t u8_to_buf(char *buf, u8f x)
{
    size_t len = 1;

    if (x > 9) {
        u8f o, t;

        o = x % 10;
        t = x / 10;

        if (x > 99) {
            u8f h;

            t %= 10;
            h = t / 10;
            len = 3;

            buf[0] = h + 48;
            buf[1] = t + 48;
            buf[2] = o + 48;
        } else {
            len = 2;

            buf[0] = t + 48;
            buf[1] = o + 48;
        }
    } else {
        buf[0] = x + 48;
    }

    return len;
}

size_t tch4_dif_to_buf(char *buf, struct TChar4 *dif, struct TChar4 *blk)
{
    size_t len = 0;

    if (dif->bg != blk->bg) {
        dif->bg = blk->bg;
        u8f bg = blk->bg + (blk->bg < 8 ? 40 : 92);

        buf[len++] = '\x1b';
        buf[len++] = '[';
        len += u8_to_buf(buf + len, bg);

        if (dif->fg != blk->fg) {
            dif->fg = blk->fg;
            u8f fg = blk->fg + (blk->fg < 8 ? 30 : 82);

            buf[len++] = ';';
            len += u8_to_buf(buf + len, fg);
        }

        buf[len++] = 'm';
    } else if (dif->fg != blk->fg) {
        dif->fg = blk->fg;
        u8f fg = blk->fg + (blk->fg < 8 ? 30 : 82);

        buf[len++] = '\x1b';
        buf[len++] = '[';
        len += u8_to_buf(buf + len, fg);
        buf[len++] = 'm';
    }
    buf[len++] = blk->ch;
    
    return len;
}

size_t TermOut(struct Terminal *term)
{
    struct TChar4 dif;

    size_t len = 7;
    memcpy(term->buf, "\x1b[H\x1b[0m", 7);

    for (size_t y = 0; y < term->hgt; y++) {
    for (size_t x = 0; x < term->wid; x++) {
        size_t i = y * term->wid + x;
        struct TChar4 *blk = &term->blks[i];

        // DEBUG
        if (blk->ch == 0)
            blk->ch = '#';
        // DEBUG

        len += tch4_dif_to_buf(term->buf + len, &dif, blk);
    }
        term->buf[len++] = '\n';
    }

    term->buf[len] = 0;
    return len;
}

/*int main()
{
    struct Terminal term;
    NewTerm(&term, 20, 20);

    for (int i = 0; i < 20 * 20; i++) {
        term.blks[i].ch = 'l';
        term.blks[i].bg = 0;
        term.blks[i].fg = 8;
    }

    size_t len = TermOut(&term);
    puts(term.buf);

    printf("hi");
    return 0;
}*/