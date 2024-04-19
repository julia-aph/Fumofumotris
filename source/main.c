#include <iso646.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ctrl.h"
#include "input.h"
#include "fumotris.h"
#include "term.h"
#include "tetr.h"
#include "event.h"
#include "platform.h"

int main()
{
    struct Instance game;
    if (!Start(&game))
        exit(1);

    if(!PlatformInit())
        exit(1);

    CreateInputThread(&game.ctrl.buf, );
    Loop(&game);

    return 0;
}