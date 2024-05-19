#include "fumotris.h"
#include <stdlib.h>
#include <string.h>


struct Fumotris {
    struct Terminal term;

    struct TetraTemplate *bag[7];
    usize bag_i;

    struct Tetra board;
    struct Tetra piece;

    bool is_ground;

    nsec last_moved;
    nsec last_dropped;
};

void shuffle(struct Fumotris *fumo)
{
    for (usize i = 6; i > 0; i--) {
        usize swap = rand() % i;
        
        struct TetraTemplate *tmp = fumo->bag[swap];
        fumo->bag[swap] = fumo->bag[i];
        fumo->bag[i] = tmp;
    }
}

bool check_clear(struct Tetra *board, usize y)
{
    for (usize x = 0; x < board->wid; x++) {
        if (board->blks[board->wid * y + x] == 0)
            return false;
    }

    return true;
}

void shift_down(struct Tetra *board, usize y)
{
    memmove(board->blks + board->wid, board->blks, board->wid * y);
}

void place(struct Fumotris *fumo)
{
    TetraOverlay(&fumo->piece, &fumo->board);

    usize lines_cleared = 0;
    for (usize y = fumo->piece.y; y < fumo->piece.y + fumo->piece.hgt; y++) {
        if (check_clear(&fumo->board, y)) {
            lines_cleared += 1;
            shift_down(&fumo->board, y);
        }
    }

    TetraSet(&fumo->piece, fumo->bag[fumo->bag_i++]);

    if (fumo->bag_i == 7) {
        shuffle(fumo);
        fumo->bag_i = 0;
    }

}

i16 get_horizontal(struct Controller *ctrl)
{
    return (-(i16)ctrl->axes[LEFT].is_held) + ctrl->axes[RIGHT].is_held;
}

void FumotrisStart(struct Instance *inst, struct Fumotris *fumo)
{
    ControllerBindMulti(&inst->ctrl, BINDS_N, CONTROLS, CODES, TYPES);

    CreateTerminal(&fumo->term, 64, 26);
    FILE *file;
    file = fopen("fumo.txt", "r");
    if (file) {
        signed char ch;
        usize x = 0;
        usize y = 0;

        while ((ch = getc(file)) != EOF) {
            if (ch == '\n') {
                x = 0;
                y += 1;
                continue;
            }

            fumo->term.buf[fumo->term.wid * y + x++] = (struct Char4) { .ch = ch, .color.fg = 15 };
        }
        fclose(file);
    }

    for (usize i = 0; i < 7; i++) 
        fumo->bag[i] = templates[i];
    fumo->bag_i = 1;

    CreateTetra(&fumo->board, 10, 20);
    TetraSet(&fumo->piece, fumo->bag[0]);

    fumo->is_ground = false;
    fumo->last_moved = 0;
    fumo->last_dropped = 0;
}

void FumotrisUpdate(struct Instance *inst, struct Fumotris *fumo)
{
    i16 horizontal = get_horizontal(&inst->ctrl);

    if (horizontal != 0 and fumo->last_moved < inst->time) {
        fumo->last_moved = inst->time + 1e8;
        TetraMove(&fumo->piece, &fumo->board, horizontal, 0);
    }

    if (inst->ctrl.axes[ROTATE_CCW].is_down)
        TetraRotate(&fumo->piece, &fumo->board, 1);

    if (inst->ctrl.axes[ROTATE_CW].is_down)
        TetraRotate(&fumo->piece, &fumo->board, -1);

    if (inst->ctrl.axes[SOFT_DROP].is_held and fumo->last_dropped < inst->time) {
        fumo->last_dropped = inst->time + 1e8;
        TetraMove(&fumo->piece, &fumo->board, 0, 1);
    }
        
    if (inst->ctrl.axes[HARD_DROP].is_down) {
        while (TetraMove(&fumo->piece, &fumo->board, 0, 1));
        place(fumo);
    }
}

nsec FumotrisFall(struct Instance *inst, struct Fumotris *fumo)
{
    if (!TetraMove(&fumo->piece, &fumo->board, 0, 1)) {
        if (!fumo->is_ground)
            fumo->is_ground = true;
        else
            place(fumo);
    }

    return 5e8;
}

void FumotrisDraw(struct Instance *inst, struct Fumotris *fumo)
{
    TetraTerminalClear(&fumo->board, &fumo->term, 3, 5);
    TetraTerminalDraw(&fumo->board, &fumo->term, 3, 5);

    TetraTerminalDrawGhost(&fumo->piece, &fumo->board, &fumo->term, 3, 5);
    TetraTerminalDraw(&fumo->piece, &fumo->term, 3, 5);

    TerminalPrint(&fumo->term);
    puts(fumo->term.str);
}

int main()
{
    system("color");

    struct Instance inst;
    CreateFumoInstance(&inst);
    
    struct Fumotris game;
    
    EventAdd(&inst.on_start, &game, FumotrisStart);
    EventAdd(&inst.on_update, &game, FumotrisUpdate);
    EventAdd(&inst.on_draw, &game, FumotrisDraw);

    CoroutineAdd(&inst, &game, FumotrisFall);

    FumoInstanceRun(&inst);

    return 0;
}