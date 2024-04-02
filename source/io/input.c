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

struct input_args {
    Ctrl *ctrl;
    struct RecordBuffer *buf;
};

void *block_input(void *args_ptr)
{
    struct input_args *args = args_ptr;
    Ctrl *ctrl = args->ctrl;
    struct RecordBuffer *buf = args->buf;
    free(args_ptr);

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

void StartInput(Ctrl *ctrl, struct RecordBuffer *buf)
{
    struct input_args *args = malloc(sizeof(struct input_args));
    args->ctrl = ctrl;
    args->buf = buf;

    pthread_create(&ctrl->thread, nullptr, block_input, args);
}

void JoinInput(Ctrl *ctrl)
{
    pthread_join(ctrl->thread, nullptr);
}