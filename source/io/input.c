#include <iso646.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "ctrl.h"
#include "fumotris.h"
#include "gametime.h"

#ifdef _WIN32
#include "win.h"
#endif

bool dispatch(Ctrl *ctrl, struct InputRecord *record)
{
    switch (record->type) {
    case KEY:
        return CtrlUpdateKey(ctrl, record);
    case AXIS:
        return CtrlUpdateAxis(ctrl, record);
    case JOYSTICK:
        return CtrlUpdateJoystick(ctrl, record);
    case WINDOW:
        return CtrlUpdateWindow(ctrl, record);
    case ESCAPE:
    default:
        return false;
    }
}

bool write_result(Ctrl *ctrl, struct InputResult *result)
{
    double now = GetTime();
    pthread_mutex_lock(&ctrl->mutex);

    for (size_t i = 0; i < result->count; i++) {
        if (result->buf[i].type == ESCAPE)
            return false;
        result->buf[i].timestamp = now;

        dispatch(ctrl, &result->buf[i]);
    }

    pthread_mutex_unlock(&ctrl->mutex);
    return true;
}

void *block_input(void *args)
{
    Ctrl *ctrl = args;
    struct InputResult result;

input_loop:
    bool success;
    #ifdef _WIN32
    success = WindowsBlockInput(&result);
    #endif

    if (!success) {
        exit(1);
    }

    if (!write_result(ctrl, &result)) {
        return nullptr;
    }

    goto input_loop;

    return nullptr;
}

void StartInput(Ctrl *ctrl)
{
    pthread_t input_thread;
    pthread_create(&input_thread, nullptr, block_input, ctrl);
    //pthread_join(input_thread, nullptr);
}