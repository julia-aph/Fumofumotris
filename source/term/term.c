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

    struct TChar4 *bufs[2];
    u8f switch_read;
    u8f switch_write;

    pthread_mutex_t mutex;
    pthread_cond_t update;

    struct {
        u8f is_writing : 1;
        u8f resize : 1;
    } flags;
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

        .bufs = nullptr,
        .switch_read = 0,
        .switch_write = 0,
        
        .mutex = PTHREAD_MUTEX_INITIALIZER,
        .update = PTHREAD_COND_INITIALIZER,
        
        .flags.is_writing = true,
        .flags.resize = false
    };
}

void TermSetBufs(struct Terminal *term, struct TChar4 *buf0, struct TChar4 *buf1)
{
    term->buf_size = buf_size(term->area, term->hgt);
    term->bufs[0] = buf0;
    term->bufs[1] = buf1;
}

void TermResize(struct Terminal *term, size_t wid, size_t hgt)
{
    size_t area = wid * hgt;
    size_t buf_size = term_buf_size(area, hgt);
    
    pthread_mutex_lock(&term->mutex);
    {
        term->wid = wid;
        term->hgt = hgt;

        term->area = area;
        term->buf_size = buf_size;

        term->flags.resize = true;
        
        pthread_cond_signal(&term->update);
    }
    pthread_mutex_unlock(&term->mutex);
}

void UpdateTerm(struct Terminal *term)
{
    pthread_mutex_lock(&term->mutex);
    {
        term->switch_read = term->switch_write;
        term->switch_write = (term->switch_write + 1) % 2;

        term->flags.is_writing = false;

        pthread_cond_signal(&term->update);
    }
    pthread_mutex_unlock(&term->mutex);
}

bool TermWaitUpdate(struct Terminal *term)
{
    while (term->flags.is_writing) {
        pthread_cond_wait(&term->update, &term->mutex);

        if (term->flags.resize) {
            term->flags.resize = false;
            return false;
        }
    }
    return true;
}

void TermSignalSafe(struct Terminal *term)
{
    pthread_cond_signal(&term->update);
}

void WaitSafeTerm(struct Terminal *term)
{
    pthread_mutex_lock(&term->mutex);
    while (term->bufs == nullptr) {
        pthread_cond_wait(&term->update, &term->mutex);
    }
    
    pthread_mutex_unlock(&term->mutex);
}

void TermLock(struct Terminal *term)
{
    pthread_mutex_lock(&term->mutex);
}

void TermUnlock(struct Terminal *term)
{
    pthread_mutex_unlock(&term->mutex);
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

size_t TermOut(struct Terminal *term, char *buf)
{
    u8f last_bg = 0;
    u8f last_fg = 0;

    size_t filled = snprintf(buf, term->buf_size, "\x1b[H\x1b[0m");

    for(size_t y = 0; y < term->hgt; y++) {
    for(size_t x = 0; x < term->wid; x++) {
        size_t i = y * term->wid + x;
        struct TChar4 *blk = &term->bufs[i];
        
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