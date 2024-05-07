#include "terminal.h"

#define RESET_STR_LEN 7
#define MAX_CH4_LEN 11


usize TerminalStringSize(usize wid,  usize hgt)
{
    return RESET_STR_LEN
        + MAX_CH4_LEN * wid * hgt
        + hgt
        + 1;
}

bool CreateTerminal(struct Terminal *term, usize wid, usize hgt)
{
    struct Char4 *ch4s = calloc(wid * hgt, sizeof(struct Char4));
    
    if (ch4s == nullptr)
        return false;

    *term = (struct Terminal) {
        .wid = wid,
        .hgt = hgt,

        .ch4s = ch4s
    };

    return true;
}

void FreeTerminal(struct Terminal *term)
{
    free(term->ch4s);
}

usize u8_to_buf(char *buf, u8f x)
{
    usize len = 1;

    if (x > 9) {
        u8f ones, tens;

        ones = x % 10;
        tens = x / 10;

        if (x > 99) {
            u8f hnds;

            tens %= 10;
            hnds = tens / 10;
            len = 3;

            buf[0] = hnds + 48;
            buf[1] = tens + 48;
            buf[2] = ones + 48;
        } else {
            len = 2;

            buf[0] = tens + 48;
            buf[1] = ones + 48;
        }
    } else {
        buf[0] = x + 48;
    }

    return len;
}

u8f ansi_bg(u8f bg)
{
    return bg + (bg < 8 ? 40 : 92);
}

u8f ansi_fg(u8f fg)
{
    return fg + (fg < 8 ? 30 : 82);
}

usize ch4_dif_to_buf(char *buf, struct Color4 *dif, struct Char4 *ch4)
{
    usize len = 0;

    if (dif->bg != ch4->color.bg) {
        buf[len++] = '\x1b';
        buf[len++] = '[';
        len += u8_to_buf(buf + len, ansi_bg(ch4->color.bg));

        if (dif->fg != ch4->color.fg) {
            buf[len++] = ';';
            len += u8_to_buf(buf + len, ansi_fg(ch4->color.fg));
        }

        buf[len++] = 'm';

    } else if (dif->fg != ch4->color.fg) {
        buf[len++] = '\x1b';
        buf[len++] = '[';
        len += u8_to_buf(buf + len, ansi_fg(ch4->color.fg));
        buf[len++] = 'm';
    }

    buf[len++] = ch4->ch;
    *dif = ch4->color;
    
    return len;
}

usize TerminalPrint(char *buf, usize n, struct Terminal *term)
{
    struct Char4 dif;

    usize len = 7;
    memcpy(buf, "\x1b[H\x1b[0m", 7);

    for (usize y = 0; y < term->hgt; y++) {
    for (usize x = 0; x < term->wid; x++) {
        usize i = y * term->wid + x;
        struct Char4 *ch4 = &term->ch4s[i];

        // DEBUG
        if (ch4->ch == 0)
            ch4->ch = '#';
        // DEBUG

        len += ch4_dif_to_buf(buf + len, &dif, ch4);
    }
        buf[len++] = '\n';
    }

    buf[len] = 0;
    return len;
}