#include <iso646.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <time.h>

#include "fumotris.h"
#include "gametime.h"
#include "input.h"
#include "term.h"

struct Windows {
    HANDLE input_handle;
    HANDLE timer;
};
static struct Windows win;

bool WinInitHandles()
{
    win.input_handle = GetStdHandle(STD_INPUT_HANDLE);
    if (win.input_handle == INVALID_HANDLE_VALUE)
        return false;

    win.timer = CreateWaitableTimer(
        NULL,                       // Timer attributes
        TRUE,                       // Manual reset
        NULL                        // Name
    );
    if (!win.timer)
        return false;

    return true;
}

bool WinInitConsole()
{
    DWORD mode = ENABLE_EXTENDED_FLAGS
        | ENABLE_PROCESSED_INPUT
        | ENABLE_PROCESSED_OUTPUT
        | ENABLE_MOUSE_INPUT
        | ENABLE_WINDOW_INPUT;
    return SetConsoleMode(win.input_handle, mode) != 0;
}

bool WinGetRefreshRate(u16f *out)
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

void set_key_record(struct InputRecord *rec, KEY_EVENT_RECORD win_key)
{
    rec->type = KEY;
    rec->bind = win_key.wVirtualKeyCode;

    rec->button.is_down = win_key.bKeyDown;
    rec->button.is_up = !win_key.bKeyDown;

    if (win_key.wVirtualKeyCode == VK_ESCAPE)
        rec->type = ESCAPE;
}

bool set_mouse_record(struct InputRecord *rec, MOUSE_EVENT_RECORD win_mouse)
{
    switch (win_mouse.dwEventFlags) {
    case MOUSE_WHEELED:
        rec->type = AXIS;
        rec->bind = 0;
        rec->axis.value = win_mouse.dwButtonState;
        break;
    case MOUSE_HWHEELED:
        rec->type = AXIS;
        rec->bind = 1;
        rec->axis.value = win_mouse.dwButtonState;
        break;
    case MOUSE_MOVED:
        rec->type = JOYSTICK;
        rec->bind = 0;
        rec->joystick.x = win_mouse.dwMousePosition.X;
        rec->joystick.y = win_mouse.dwMousePosition.Y;
        break;
    default:
        return false;
    }
    return true;
}

bool dispatch_record(struct InputRecord *rec, INPUT_RECORD win_rec)
{
    switch (win_rec.EventType) {
    case KEY_EVENT:
        set_key_record(rec, win_rec.Event.KeyEvent);
        break;
    case MOUSE_EVENT:
        return set_mouse_record(rec, win_rec.Event.MouseEvent);
    case WINDOW_BUFFER_SIZE_EVENT:
        // TODO: Handle window resizing
        return false;
    default:
        rec->type = ESCAPE;
    }
    return true;
}

bool WinBlockInput(struct RecordBuffer *buf)
{
    size_t win_size = IO_BUF_SIZE - buf->count;
    INPUT_RECORD win_buf[win_size];
    DWORD count;

    if (!ReadConsoleInput(
        win.input_handle,           // Input handle
        win_buf,                    // Record buffer
        win_size,                   // Record buffer length
        &count                      // Out number of records
    ))
        return false;
    
    struct timespec now;
    timespec_get(&now, TIME_UTC);

    pthread_mutex_lock(&buf->mutex);

    for (size_t i = 0; i < count; i++) {
        struct InputRecord rec;
        rec.timestamp = now;

        bool include = dispatch_record(&rec, win_buf[i]);
        if (!include)
            continue;

        buf->records[buf->count++] = rec;
    }

    pthread_mutex_unlock(&buf->mutex);
    return true;
}

bool WinWait(struct timespec relative)
{
    LARGE_INTEGER duration;
    duration.QuadPart = -10000000 * relative.tv_sec - relative.tv_nsec / 100;

    if (!SetWaitableTimer(
        win.timer,                  // Timer
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