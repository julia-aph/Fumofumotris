#include <stdio.h>
#include <stdlib.h>
#include "fumocommon.h"
#include "terminal.h"


struct Tetra {
    u8 *blks;

    u16 wid;
    u16 hgt;

    i16 x;
    i16 y;

    u8f rot;
};

bool CreateTetra(struct Tetra *map, u16 wid, u16 hgt);

void FreeTetra(struct Tetra *map);

void SetTetra(struct Tetra *map, u8 *blks, u16 wid, u16 hgt, i16 x, i16 y);

void TetraTerminalClear(struct Tetra *board, struct Terminal *term);

void TetraTerminalDraw(struct Tetra *map, struct Terminal *term);

bool TetraMove(struct Tetra *piece, struct Tetra *board, i16 dx, i16 dy);

bool TetraIsCollision(struct Tetra *piece, struct Tetra *board, i16 dx, i16 dy);

void TetraOverlay(struct Tetra *piece, struct Tetra *board);