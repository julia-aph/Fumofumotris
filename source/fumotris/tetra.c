#include "tetra.h"


bool CreateTetra(struct Tetra *tetra, u16 wid, u16 hgt)
{
    u8 *blks = calloc(wid * hgt, sizeof(u8));
    
    if (blks == nullptr)
        return false;

    *tetra = (struct Tetra) {
        .blks = blks,

        .wid = wid,
        .hgt = hgt,

        .x = 0,
        .y = 0,

        .rot = 0
    };

    return true;
}

void FreeTetra(struct Tetra *tetra)
{
    free(tetra->blks);
}

void TetraSet(struct Tetra *tetra, struct TetraTemplate *template)
{
    tetra->blks = template->blks;
    tetra->wid = template->wid;
    tetra->hgt = template->hgt;

    tetra->x = 0;
    tetra->y = 0;
    tetra->rot = 0;
}

usize rotate_index(usize i, usize wid, u8 rot)
{
    if(rot == 0)
        return i;

    usize row = i / wid;
    usize col = i % wid;
    
    switch (rot) {
    case 1:
        return (wid - col - 1) * wid + row;
    case 2:
        return (wid - row - 1) * wid + (wid - col - 1);
    case 3:
        return col * wid + (wid - row - 1);
    }
    
    return 0;
}

bool TetraIsCollision(struct Tetra *t, struct Tetra *board)
{
    usize i = 0;
    for (i16 y = t->y; y < t->y + t->hgt; y++) {
    for (i16 x = t->x; x < t->x + t->wid; x++, i++) {
        usize rot_i = rotate_index(i, t->wid, t->rot);

        if(t->blks[rot_i] == 0)
            continue;

        if(x < 0 or x >= board->wid or y < 0 or y >= board->hgt)
            return true;

        if(board->blks[board->wid * y + x] != 0)
            return true;
    }
    }

    return false;
}

bool TetraMove(struct Tetra *t, struct Tetra *board, i16 dx, i16 dy)
{
    t->x += dx;
    t->y += dy;

    if (TetraIsCollision(t, board)) {
        t->x -= dx;
        t->y -= dy;

        return false;
    }

    return true;
}

bool TetraRotate(struct Tetra *t, struct Tetra *board, i8 dr)
{
    u8 rot = t->rot;
    t->rot = (t->rot + 4 + dr) % 4;

    if (TetraIsCollision(t, board)) {
        t->rot = rot;

        return false;
    }

    return true;
}

void TetraOverlay(struct Tetra *t, struct Tetra *board)
{
    usize i = 0;
    for (i16 y = t->y; y < t->y + t->hgt; y++) {
    for (i16 x = t->x; x < t->x + t->wid; x++, i++) {
        usize rot_i = rotate_index(i, t->wid, t->rot);

        if(t->blks[rot_i] == 0)
            continue;

        if(x < 0 or x >= board->wid or y < 0 or y >= board->hgt)
            continue;

        board->blks[board->wid * y + x] = t->blks[rot_i];
    }
    }
}

void TetraTerminalClear(struct Tetra *board, struct Terminal *term)
{
    for (usize i = 0; i < board->wid * board->hgt; i++) {
        struct Char4 *block = term->buf + i * 2;

        block[0] = (struct Char4) { .ch = '.', .color.fg = 8 };
        block[1] = (struct Char4) { .ch = ' ', .color.fg = 8 };
    }
}

void TetraTerminalDraw(struct Tetra *t, struct Terminal *term)
{
    static const u8f blk_colors[8] = { 8, 14, 11, 13, 10, 9, 12, 3 };

    usize i = 0;
    for (usize y = 0; y < t->hgt; y++) {
    for (usize x = 0; x < t->wid; x++, i++) {
        usize rot_i = rotate_index(i, t->wid, t->rot);

        if (t->blks[rot_i] == 0)
            continue;

        usize term_i = (y + t->y) * term->wid + (x + t->x) * 2;
        struct Char4 *block = term->buf + term_i;
        
        u8 fg = blk_colors[t->blks[rot_i]];
        block[0] = (struct Char4) { .ch = '[', .color.fg = fg };
        block[1] = (struct Char4) { .ch = ']', .color.fg = fg };
    }
    }
}