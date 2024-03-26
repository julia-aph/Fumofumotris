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

#define IO_BUF_SIZE 8

struct input_args {
    Ctrl *ctrl;
    struct InputBuffer *in_buf;
};

void *block_input(void *args_ptr)
{
    struct input_args *args = args_ptr;
    Ctrl *ctrl = args->ctrl;
    struct InputBuffer *in_buf = args->in_buf;

input_loop:
    bool success;

    #ifdef _WIN32
    success = WindowsBlockInput(&in_buf);
    #endif

    if (!success)
        exit(1);

    goto input_loop;
    return nullptr;
}

void StartInput(Ctrl *ctrl)
{
    pthread_t input_thread;
    pthread_create(&input_thread, nullptr, block_input, ctrl);
    //pthread_join(input_thread, nullptr);
}