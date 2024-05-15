#include "tetra.h"


bool CreateTetra(struct Tetra *map, u16 wid, u16 hgt)
{
    u8 *blks = calloc(wid * hgt, sizeof(u8));
    
    if (blks == nullptr)
        return false;

    map->blks = blks;
    map->wid = wid;
    map->hgt = hgt;

    map->x = 0;
    map->y = 0;
    map->rot = 0;

    return true;
}

void FreeTetra(struct Tetra *tetra)
{
    free(tetra->blks);
}

void SetTetra(struct Tetra *map, struct TetraTemplate *t, i16 x, i16 y)
{
    map->blks = t->blks;
    map->wid = t->wid;
    map->hgt = t->hgt;
    
    map->x = x;
    map->y = y;
    map->rot = 0;
}

bool TetraMove(struct Tetra *piece, struct Tetra *board, i16 dx, i16 dy)
{
    if (TetraIsCollision(piece, board, dx, dy))
        return false;

    piece->x += dx;
    piece->y += dy;

    return true;
}

bool TetraIsCollision(struct Tetra *piece, struct Tetra *board, i16 dx, i16 dy)
{
    i16 x_start = piece->x + dx;
    i16 y_start = piece->y + dy;

    usize i = 0;
    for (i16 y = y_start; y < y_start + piece->hgt; y++) {
    for (i16 x = x_start; x < x_start + piece->wid; x++, i++) {
        if(piece->blks[i] == 0)
            continue;

        if(x < 0 or x >= board->wid or y < 0 or y >= board->hgt)
            return true;

        if(board->blks[board->wid * y + x] != 0)
            return true;
    }
    }

    return false;
}

void TetraTerminalClear(struct Tetra *board, struct Terminal *term)
{
    for (usize i = 0; i < board->wid * board->hgt; i++) {
        struct Char4 *block = term->buf + i * 2;

        block[0] = (struct Char4) { .ch = '(', .color.fg = 8 };
        block[1] = (struct Char4) { .ch = ')', .color.fg = 8 };
    }
}

void TetraTerminalDraw(struct Tetra *tetra, struct Terminal *term)
{
    static const u8f blk_colors[8] = { 8, 14, 11, 13, 10, 9, 12, 3 };

    usize i = 0;
    for (usize y = 0; y < tetra->hgt; y++) {
    for (usize x = 0; x < tetra->wid; x++, i++) {
        if (tetra->blks[i] == 0)
            continue;

        usize term_i = (y + tetra->y) * term->wid + (x + tetra->x) * 2;
        struct Char4 *block = term->buf + term_i;
        
        u8 fg = blk_colors[tetra->blks[i]];
        block[0] = (struct Char4) { .ch = '[', .color.fg = fg };
        block[1] = (struct Char4) { .ch = ']', .color.fg = fg };
    }
    }
}

void TetraOverlay(struct Tetra *piece, struct Tetra *board)
{
    usize i = 0;
    for (usize y = piece->y; y < piece->y + piece->hgt; y++) {
    for (usize x = piece->x; x < piece->x + piece->wid; x++, i++) {
        if(piece->blks[i] == 0)
            continue;

        if(x < 0 or x >= board->wid or y < 0 or y >= board->hgt)
            continue;

        board->blks[board->wid * y + x] = piece->blks[i];
    }
    }
}