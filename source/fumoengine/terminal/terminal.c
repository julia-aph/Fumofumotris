#include "terminal.h"
#include <string.h>

#define RESET_STR_LEN 7
#define MAX_CH4_LEN 11


usize TerminalMaxOut(usize wid, usize hgt)
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

    char *str = malloc(TerminalMaxOut(wid, hgt));
    if (str == nullptr)
        return false;

    *term = (struct Terminal) {
        .buf = ch4s,
        .str = str,

        .wid = wid,
        .hgt = hgt       
    };

    return true;
}

void FreeTerminal(struct Terminal *term)
{
    free(term->buf);
}

usize u8_to_str(char *out, u8f x)
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

            out[0] = hnds + 48;
            out[1] = tens + 48;
            out[2] = ones + 48;
        } else {
            len = 2;

            out[0] = tens + 48;
            out[1] = ones + 48;
        }
    } else {
        out[0] = x + 48;
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

usize ch4_dif_to_str(char *out, struct Color4 *dif, struct Char4 *ch4)
{
    usize len = 0;

    if (dif->bg != ch4->color.bg) {
        out[len++] = '\x1b';
        out[len++] = '[';
        len += u8_to_str(out + len, ansi_bg(ch4->color.bg));

        if (dif->fg != ch4->color.fg) {
            out[len++] = ';';
            len += u8_to_str(out + len, ansi_fg(ch4->color.fg));
        }

        out[len++] = 'm';

    } else if (dif->fg != ch4->color.fg) {
        out[len++] = '\x1b';
        out[len++] = '[';
        len += u8_to_str(out + len, ansi_fg(ch4->color.fg));
        out[len++] = 'm';
    }

    out[len++] = ch4->ch;
    *dif = ch4->color;
    
    return len;
}

usize TerminalPrint(struct Terminal *term)
{
    struct Color4 dif = { 0, 0 };

    usize len = 7;
    memcpy(term->str, "\x1b[H\x1b[0m", 7);

    usize i = 0;
    for (usize y = 0; y < term->hgt; y++) {
    for (usize x = 0; x < term->wid; x++, i++) {
        struct Char4 *ch4 = &term->buf[i];

        // DEBUG
        if (ch4->ch == 0)
            ch4->ch = '#';
        // DEBUG

        len += ch4_dif_to_str(term->str + len, &dif, ch4);
    }
        term->str[len++] = '\n';
    }

    term->str[len] = 0;
    return len;
}