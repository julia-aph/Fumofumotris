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
    struct RecordBuffer *rec_buf = args_ptr;

    while (true) {
        bool success;

        #ifdef _WIN32
        success = WindowsBlockInput(rec_buf);
        #endif
        
        if (!success) {
            printf("winfail");
            exit(1);
        }
    }

    return nullptr;
}

void StartInput(struct Controller *ctrl)
{
    pthread_create(&ctrl->thread, nullptr, block_input, &ctrl->buf);
}

void JoinInput(struct Controller *ctrl)
{
    pthread_join(ctrl->thread, nullptr);
}