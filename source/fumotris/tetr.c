#include <iso646.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fumocommon.h"
#include "term.h"


struct TetrMap {
    size_t wid;
    size_t hgt;
    size_t area;

    int x;
    int y;
    u8 rot;

    u8 *blks;
};

struct TetrMap NewTetrMap(u8 *blks, size_t wid, size_t hgt)
{
    size_t area = wid * hgt;
    memset(blks, 0, area);

    return (struct TetrMap) {
        wid, hgt, area,
        0, 0, 0,
        blks
    };
}

void TetrMapToTermBuf(struct TetrMap *map, struct Terminal *term)
{
    static const u8f blk_colors[8] = { 8, 14, 11, 13, 10, 9, 12, 3 };

    for (size_t y = 0; y < map->hgt; y++) {
    for (size_t x = 0; x < map->wid; x++) {
        size_t map_i = y * map->wid + x;
        size_t buf_i = (y + map->y) * term->wid + (x + map->x) * 2;

        struct Char4 *a = &term->chs[buf_i];
        struct Char4 *b = &term->chs[buf_i + 1];

        if (map->blks[map_i] == 0) {
            a->ch = '(';
            b->ch = ')';
        } else {
            a->ch = '[';
            b->ch = ']';
        }

        u8 fg = blk_colors[map->blks[map_i]];
        a->fg = fg;
        b->fg = fg;
    }
    }
}

bool TetrCollisionCheck(struct TetrMap *board, struct TetrMap *piece, int dx, int dy)
{
    size_t i = 0;
    for (size_t y = piece->y + dy; y < piece->y + piece->hgt + dy; y++) {
    for (size_t x = piece->x + dx; x < piece->x + piece->wid + dx; x++) {
        if(piece->blks[i] == 0)
            goto next;

        if(y >= board->hgt or x >= board->wid)
            return false;

        size_t board_i = y * board->wid + x;
        if(board->blks[board_i] != 0)
            return false;

next:
        i++;
    }
    }
    return true;
}