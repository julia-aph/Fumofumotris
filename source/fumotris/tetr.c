#include "tetr.h"


bool CreateTetrMap(struct TetrMap *map, usize wid, usize hgt)
{
    u8 *blks = calloc(wid * hgt, sizeof(u8));
    
    if (blks == nullptr)
        return false;

    *map = (struct TetrMap) {
        .wid = wid,
        .hgt = hgt,

        .x = 0,
        .y = 0,

        .rot = 0,

        .blks = blks
    };

    return true;
}

void FreeTetrMap(struct TetrMap *map)
{
    free(map->blks);
}

void TetrMapDraw(struct TetrMap *map, struct Terminal *term)
{
    static const u8f blk_colors[8] = { 8, 14, 11, 13, 10, 9, 12, 3 };

    for (usize y = 0; y < map->hgt; y++) {
    for (usize x = 0; x < map->wid; x++) {
        usize map_i = y * map->wid + x;
        usize term_i = (y + map->y) * term->wid + (x + map->x) * 2;

        struct Char4 *block = term->buf + term_i;

        if (map->blks[map_i] == 0) {
            block[0].ch = '(';
            block[1].ch = ')';
        } else {
            block[0].ch = '[';
            block[1].ch = ']';
        }

        u8 fg = blk_colors[map->blks[map_i]];
        block[0].color.fg = fg;
        block[1].color.fg = fg;
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