#include <iso646.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "ctrl.h"
#include "fumotris.h"

#ifdef _WIN32
#include "win.h"
#endif

void *block_input(void *args_ptr)
{
    struct RecordBuffer *buf = args_ptr;

    while (true) {
        bool success;

        #ifdef _WIN32
        success = WindowsBlockInput(buf);
        #endif
        
        if (!success) {
            printf("winfail");
            exit(1);
        }
    }

    return nullptr;
}

void StartInput(struct Ctrl *ctrl, struct RecordBuffer *buf)
{
    pthread_create(&ctrl->thread, nullptr, block_input, buf);
}

void JoinInput(struct Ctrl *ctrl)
{
    pthread_join(ctrl->thread, nullptr);
}