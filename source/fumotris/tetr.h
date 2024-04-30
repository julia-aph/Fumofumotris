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

struct TetrMap NewTetrMap(u8 *blks, size_t wid, size_t hgt);

void TetrMapToTermBuf(struct TetrMap *map, struct Terminal *term);

bool TetrCollisionCheck(struct TetrMap *board, struct TetrMap *piece, int dx, int dy);