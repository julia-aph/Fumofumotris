#include <stdio.h>
#include <stdlib.h>
#include "fumocommon.h"
#include "terminal.h"


struct TetraTemplate {
    u8 *blks;

    usize wid;
    usize hgt;
};

struct Tetra {
    u8 *blks;
    
    u16 wid;
    u16 hgt;

    i16 x;
    i16 y;
    u8 rot;
};

bool CreateTetra(struct Tetra *tetra, u16 wid, u16 hgt);

void FreeTetra(struct Tetra *tetra);

void TetraSet(struct Tetra *tetra, struct TetraTemplate *template);

bool TetraIsCollision(struct Tetra *t, struct Tetra *board);

bool TetraMove(struct Tetra *t, struct Tetra *board, i16 dx, i16 dy);

bool TetraRotate(struct Tetra *t, struct Tetra *board, i8 dr);

void TetraOverlay(struct Tetra *t, struct Tetra *board);

void TetraTerminalClear(struct Tetra *board, struct Terminal *term, u16 OFS_X, u16 OFS_Y);

void TetraTerminalDraw(struct Tetra *piece, struct Terminal *term, u16 OFS_X, u16 OFS_Y);

void TetraTerminalDrawGhost(struct Tetra *t, struct Tetra *board, struct Terminal *term, u16 OFS_X, u16 OFS_Y);
