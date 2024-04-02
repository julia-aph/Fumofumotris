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

struct Windows {
    HANDLE input_handle;
    HANDLE timer;
};
static struct Windows windows;

bool WinInitInputHandle()
{
    windows.input_handle = GetStdHandle(STD_INPUT_HANDLE);
    if (windows.input_handle == INVALID_HANDLE_VALUE)
        return false;
    return true;
}

bool WinInitTimer()
{
    windows.timer = CreateWaitableTimer(NULL, TRUE, NULL);
    if (!windows.timer)
        return false;
    return true;
}

bool WinInitConsole()
{
    DWORD mode = ENABLE_EXTENDED_FLAGS
        | ENABLE_PROCESSED_INPUT
        | ENABLE_MOUSE_INPUT
        | ENABLE_WINDOW_INPUT 
        | ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    return SetConsoleMode(windows.input_handle, mode);
}

void set_key_record(struct CtrlRecord *record, KEY_EVENT_RECORD win_key)
{
    record->type = KEY;
    record->id = win_key.wVirtualKeyCode;
    record->data.key.is_down = win_key.bKeyDown;

    if (win_key.wVirtualKeyCode == VK_ESCAPE)
        record->type = ESCAPE;
}

bool set_mouse_record(struct CtrlRecord *record, MOUSE_EVENT_RECORD win_mouse)
{
    switch (win_mouse.dwEventFlags) {
    case MOUSE_WHEELED:
        record->type = AXIS;
        record->id = 0;
        record->data.axis.value = win_mouse.dwButtonState;
        break;
    case MOUSE_HWHEELED:
        record->type = AXIS;
        record->id = 1;
        record->data.axis.value = win_mouse.dwButtonState;
        break;
    case MOUSE_MOVED:
        record->type = JOYSTICK;
        record->id = 0;
        record->data.joystick.x = win_mouse.dwMousePosition.X;
        record->data.joystick.y = win_mouse.dwMousePosition.Y;
        break;
    default:
        return false;
    }
    return true;
}

void set_window_record(struct CtrlRecord *record, WINDOW_BUFFER_SIZE_RECORD win_resize)
{
    record->type = WINDOW;
    record->data.joystick.x = win_resize.dwSize.X;
    record->data.joystick.y = win_resize.dwSize.Y;
}

bool dispatch_record(struct CtrlRecord *record, INPUT_RECORD win_record)
{
    switch (win_record.EventType) {
    case KEY_EVENT:
        set_key_record(record, win_record.Event.KeyEvent);
        return true;
    case MOUSE_EVENT:
        return set_mouse_record(record, win_record.Event.MouseEvent);
    case WINDOW_BUFFER_SIZE_EVENT:
        set_window_record(record, win_record.Event.WindowBufferSizeEvent);
        return true;
    default:
        record->type = ESCAPE;
    }
    return true;
}

bool WinBlockInput(struct RecordBuffer *buf)
{
    size_t win_size = IO_BUF_SIZE - buf->count;
    INPUT_RECORD win_buf[win_size];
    DWORD count;

    if (!ReadConsoleInput(windows.input_handle, win_buf, win_size, &count))
        return false;
    
    struct timespec now;
    timespec_get(&now, TIME_UTC);

    pthread_mutex_lock(&buf->mutex);

    for (size_t i = 0; i < count; i++) {
        struct CtrlRecord record;
        record.timestamp = now;

        bool include = dispatch_record(&record, win_buf[i]);
        if (!include)
            continue;

        buf->records[buf->count] = record;
        buf->count += 1;
    }

    pthread_mutex_unlock(&buf->mutex);
    return true;
}

bool WinWait(double seconds)
{
    LARGE_INTEGER duration;
    duration.QuadPart = (u64)(-10000000.0 * seconds);
    if (!SetWaitableTimer(windows.timer, &duration, 0, NULL, NULL, FALSE))
        return false;

    DWORD result = WaitForSingleObject(windows.timer, INFINITE);
    if (result != WAIT_OBJECT_0)
        return false;
    
    return true;
}