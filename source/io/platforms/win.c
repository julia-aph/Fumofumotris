#include <iso646.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

#include "fumotris.h"
#include "ctrl.h"
#include "gametime.h"

static struct Windows {
    HANDLE timer;
    HANDLE input_handle;
    DWORD input_n;
    INPUT_RECORD input_buf[IO_BUF_SIZE];
} win;

bool init_handles()
{
    win.input_handle = GetStdHandle(STD_INPUT_HANDLE);
    if (win.input_handle == INVALID_HANDLE_VALUE)
        return false;

    win.timer = CreateWaitableTimer(
        NULL,                       // Timer attributes
        TRUE,                       // Manual reset
        NULL                        // Name
    );
    if (win.timer == NULL)
        return false;

    return true;
}

bool init_console()
{
    DWORD mode = ENABLE_EXTENDED_FLAGS
        | ENABLE_PROCESSED_INPUT
        | ENABLE_PROCESSED_OUTPUT
        | ENABLE_MOUSE_INPUT
        | ENABLE_WINDOW_INPUT;
    return SetConsoleMode(win.input_handle, mode) != 0;
}

bool PlatformInit()
{
    if (!init_handles())
        return false;

    if (!init_console())
        return false;

    return true;
}

bool PlatformGetRefreshRate(u16f *out)
{
    DEVMODE mode;
    mode.dmSize = sizeof(DEVMODE);
    mode.dmDriverExtra = 0;

    if(!EnumDisplaySettingsA(
        NULL,                       // Device name (null for current)
        ENUM_CURRENT_SETTINGS,      // Mode
        &mode                       // Out
    ))
        return false;

    *out = mode.dmDisplayFrequency;
    return true;
}

void read_key_rec(struct InputRecord *rec, KEY_EVENT_RECORD win_key)
{
    rec->type = BUTTON;
    rec->bind = win_key.wVirtualKeyCode;

    rec->is_down = win_key.bKeyDown;
    rec->is_up = !win_key.bKeyDown;
}

bool read_mouse_rec(struct InputRecord *rec, MOUSE_EVENT_RECORD win_mouse)
{
    if (win_mouse.dwEventFlags == MOUSE_MOVED) {
        rec->type = JOYSTICK;
        rec->bind = 0;
        rec->js.x = win_mouse.dwMousePosition.X;
        rec->js.y = win_mouse.dwMousePosition.Y;
        
        return true;
    }

    if (win_mouse.dwEventFlags == MOUSE_WHEELED) {
        rec->bind = 0;
    } else if (win_mouse.dwEventFlags == MOUSE_HWHEELED) {
        rec->bind = 1;
    } else {
        return false;
    }

    rec->type = AXIS;
    rec->axis.value = win_mouse.dwButtonState;

    return true;
}

bool read_rec(struct InputRecord *rec, INPUT_RECORD win_rec)
{
    switch (win_rec.EventType) {
    case KEY_EVENT:
        read_key_rec(rec, win_rec.Event.KeyEvent);
        return true;

    case MOUSE_EVENT:
        return read_mouse_rec(rec, win_rec.Event.MouseEvent);

    case WINDOW_BUFFER_SIZE_EVENT:
        return false;
        // TODO: Handle window resizing
    }

    rec->type = ESCAPE;
    return false;
}

bool PlatformReadInput(struct InputBuffer *tmp)
{
    if (!ReadConsoleInput(
        win.input_handle,           // Input handle
        &win.input_buf[tmp->len],   // Record buffer
        IO_BUF_SIZE - tmp->len,     // Record buffer length
        &win.input_n                // Out number of records
    ))
        return false;

    struct InputRecord rec = { .timestamp = TimeNow() };
    for (size_t i = 0; i < win.input_n; i++) {

        if (!read_rec(&rec, win.input_buf[i]))
            continue;

        InputBufferCopy(tmp, &rec);
    }

    return true;
}

bool PlatformWait(struct timespec relative)
{
    LARGE_INTEGER duration;
    duration.QuadPart = -10000000 * relative.tv_sec - relative.tv_nsec / 100;

    if (!SetWaitableTimer(
        win.timer,                 // Timer
        &duration,                  // Duration
        0,                          // Period
        NULL,                       // Completion coroutine
        NULL,                       // Completion coroutine arg
        FALSE                       // Resume
    ))
        return false;

    DWORD result = WaitForSingleObject(win.timer, INFINITE);
    if (result != WAIT_OBJECT_0)
        return false;
    
    return true;
}