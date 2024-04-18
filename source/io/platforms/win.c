#include <iso646.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

#include "fumotris.h"
#include "ctrl.h"
#include "gametime.h"

static struct windows {
    HANDLE timer;
    HANDLE input_handle;
} win;

bool init_handles()
{
    win.input_handle = GetStdHandle(STD_INPUT_HANDLE);
    if (win.input_handle == INVALID_HANDLE_VALUE)
        return false;

    win.timer = CreateWaitableTimerW(NULL, TRUE, NULL);
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

    if(!EnumDisplaySettingsA(NULL, ENUM_CURRENT_SETTINGS, &mode))
        return false;

    *out = mode.dmDisplayFrequency;
    return true;
}

void copy_key_event(struct InputRecord *rec, KEY_EVENT_RECORD *win_key)
{
    rec->type = BUTTON;
    rec->bind = win_key->wVirtualKeyCode;

    rec->is_down = win_key->bKeyDown;
    rec->is_up = !win_key->bKeyDown;
}

bool copy_mouse_event(struct InputRecord *rec, MOUSE_EVENT_RECORD *win_mouse)
{
    if (win_mouse->dwEventFlags == MOUSE_MOVED) {
        rec->type = JOYSTICK;
        rec->bind = 0;
        rec->js.x = win_mouse->dwMousePosition.X;
        rec->js.y = win_mouse->dwMousePosition.Y;
        
        return true;
    }

    if (win_mouse->dwEventFlags & (MOUSE_WHEELED | MOUSE_HWHEELED) != 0) {
        rec->type = AXIS;
        rec->bind = win_mouse->dwEventFlags == MOUSE_WHEELED;
        rec->axis.value = win_mouse->dwButtonState;

        return true;
    }

    return false;
}

bool copy_rec(struct InputRecord *rec, INPUT_RECORD *win_rec)
{
    switch (win_rec->EventType) {
    case KEY_EVENT:
        copy_key_event(rec, &win_rec->Event.KeyEvent);
        return true;

    case MOUSE_EVENT:
        return copy_mouse_event(rec, &win_rec->Event.MouseEvent);

    case WINDOW_BUFFER_SIZE_EVENT:
        return false;
        // TODO: Handle window resizing
    }

    rec->type = ESCAPE;
    return false;
}

bool PlatformReadInput(struct InputBuffer *buf)
{
    DWORD max_records = IO_BUF_SIZE - buf->len;
    INPUT_RECORD win_buf[max_records];
    
    DWORD filled;
    if (!ReadConsoleInputW(win.input_handle, win_buf, max_records, &filled))
        return false;

    struct InputRecord rec = { .timestamp = TimeNow() };

    for (size_t i = 0; i < filled; i++) {
        if (!copy_rec(&rec, win_buf + i))
            continue;

        InputBufferCopy(buf, &rec);
    }

    return true;
}

bool PlatformWait(struct timespec relative)
{
    LARGE_INTEGER duration;
    duration.QuadPart = -10000000 * relative.tv_sec - relative.tv_nsec / 100;

    if (!SetWaitableTimer(win.timer, &duration, 0, NULL,NULL, FALSE))
        return false;

    DWORD result = WaitForSingleObject(win.timer, INFINITE);
    if (result != WAIT_OBJECT_0)
        return false;
    
    return true;
}