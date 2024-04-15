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

struct Input {
    struct Controller ctrl;

    pthread_t thread;
    pthread_mutex_t access_mutex;
};

void *block_input(void *args_ptr)
{
    struct Input *in = args_ptr;
    struct InputBuffer tmp_buf = { .len = 0 };

    while (true) {
        if (!PlatformBlockInput(&tmp_buf))
            return false;

        pthread_mutex_lock(&in->access_mutex);
        {
            
        }
        pthread_mutex_unlock(&in->access_mutex);
    }

    return nullptr;
}

bool StartInput(struct Input *in)
{
    return pthread_create(&in->thread, nullptr, block_input, in) == 0;
}

bool JoinInput(struct Input *in)
{
    return pthread_join(in->thread, nullptr) == 0;
}