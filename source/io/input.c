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
    struct InputBuffer *buf;
    pthread_mutex_t mutex;
};

void *block_input(void *args_ptr)
{
    struct input_args *args = args_ptr;
    Ctrl *ctrl = args->ctrl;
    struct InputBuffer *buf = args->buf;
    printf("RECIEVED BUF: %u, %u\n", args->ctrl, args->buf);

    while (true) {
        printf("\tIN LOOP\n");
        bool success;

        #ifdef _WIN32
        success = WindowsBlockInput(buf);
        #endif

        printf("%llu\n", buf->records[0].timestamp.tv_sec);

        if (!success) {
            printf("winfail");
            exit(1);
        }
    }

    return nullptr;
}

void StartInput(Ctrl *ctrl, struct InputBuffer *buf)
{
    printf("START INPUT:%u\n", buf);
    struct input_args args = {
        .ctrl = ctrl,
        .buf = buf,
        .mutex = PTHREAD_MUTEX_INITIALIZER
    };
    pthread_mutex_lock(&args.mutex);
    pthread_create(&ctrl->thread, nullptr, block_input, &args);
    pthread_mutex_destroy();
}

void JoinInput(Ctrl *ctrl)
{
    pthread_join(ctrl->thread, nullptr);
}