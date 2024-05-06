#include "term.h"


size_t term_buf_size(size_t area,  size_t hgt)
{
    static const size_t reset_str_len = 7;
    static const size_t max_color_str_len = 10;

    return reset_str_len + (max_color_str_len + 1) * area + hgt + 1;
}

bool CreateTerminal(struct Terminal *term, size_t wid, size_t hgt)
{
    size_t area = wid * hgt;
    size_t buf_size = term_buf_size(area, hgt);

    struct Char4 *chs = calloc(area, sizeof(struct Char4));
    
    if (chs == nullptr)
        return false;

    *term = (struct Terminal) {
        .wid = wid,
        .hgt = hgt,
        .area = area,

        .chs = chs,
    };

    return true;
}

void FreeTerminal(struct Terminal *term)
{
    free(term->chs);
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

size_t ch4_dif_to_buf(char *buf, struct Char4 *dif, struct Char4 *blk)
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

size_t TerminalPrint(char *dest, size_t n, struct Terminal *term)
{
    struct Char4 dif;

    size_t len = 7;
    memcpy(term->buf, "\x1b[H\x1b[0m", 7);

    for (size_t y = 0; y < term->hgt; y++) {
    for (size_t x = 0; x < term->wid; x++) {
        size_t i = y * term->wid + x;
        struct Char4 *blk = &term->chs[i];

        // DEBUG
        if (blk->ch == 0)
            blk->ch = '#';
        // DEBUG

        len += ch4_dif_to_buf(term->buf + len, &dif, blk);
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