#include <stdio.h>
#include <stdlib.h>

#include "fumocommon.h"
#include "terminal.h"


struct TetrMap {
    usize wid;
    usize hgt;

    int x;
    int y;

    u8f rot;

    u8 *blks;
};

bool CreateTetrMap(struct TetrMap *map, usize wid, usize hgt);

void FreeTetrMap(struct TetrMap *map);

void TetrMapDraw(struct TetrMap *map, struct Terminal *term);

bool TetrCollisionCheck(struct TetrMap *board, struct TetrMap *piece, int dx, int dy);