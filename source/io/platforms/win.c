#include "win.h"

#include <windows.h>

#include "gametime.h"
#include "input.h"

struct windows {
    union {
        HANDLE input_hands[2];
        struct {
            HANDLE input_hand;
            HANDLE early_exit_hand;
        };
    };
    HANDLE timer;
} win;

bool init_handles()
{
    win.input_hand = GetStdHandle(STD_INPUT_HANDLE);
    if (win.input_hand == INVALID_HANDLE_VALUE)
        return false;

    win.timer = CreateWaitableTimerW(NULL, TRUE, NULL);
    if (win.timer == NULL)
        return false;

    win.early_exit_hand = CreateEventW(NULL, FALSE, FALSE, NULL);
    if (win.early_exit_hand == NULL)
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

    return SetConsoleMode(win.input_hand, mode) != 0;
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
    DEVMODEW mode;
    mode.dmSize = sizeof(DEVMODEW);
    mode.dmDriverExtra = 0;

    if(!EnumDisplaySettingsW(NULL, ENUM_CURRENT_SETTINGS, &mode))
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

    if ((win_mouse->dwEventFlags & (MOUSE_WHEELED | MOUSE_HWHEELED)) != 0) {
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
    DWORD wait_status = WaitForMultipleObjects(2, win.input_hands, FALSE, 0);
    
    if (wait_status != 0)
        return wait_status == 1;

    DWORD max_records = IO_BUF_SIZE - buf->len;
    INPUT_RECORD win_buf[max_records];
    DWORD filled;
    
    if (!ReadConsoleInputW(win.input_hand, win_buf, max_records, &filled))
        return false;

    struct InputRecord rec = { .time = TimeNow() };

    for (size_t i = 0; i < filled; i++) {
        if (!copy_rec(&rec, win_buf + i))
            continue;

        InputBufferAdd(buf, &rec);
    }

    return true;
}

bool PlatformStopInput()
{
    return SetEvent(win.early_exit_hand) == 0;
}

bool PlatformWait(struct Time relative)
{
    LARGE_INTEGER duration;
    duration.QuadPart = -10000000 * relative.sec - relative.nsec / 100;

    if (!SetWaitableTimer(win.timer, &duration, 0, NULL, NULL, FALSE))
        return false;

    DWORD result = WaitForSingleObject(win.timer, INFINITE);
    if (result != WAIT_OBJECT_0)
        return false;
    
    return true;
}