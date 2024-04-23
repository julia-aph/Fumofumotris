#include "win.h"

#include <windows.h>

#include "gametime.h"
#include "input.h"

struct windows {
    HANDLE input_hand;
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

    return true;
}

bool init_console()
{
    DWORD mode =  ENABLE_EXTENDED_FLAGS
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

inline size_t ucs2_to_utf8(u16f ucs2, size_t n, char *buf) {
    return WideCharToMultiByte(CP_UTF8, 0, ucs2, 1, buf, n, NULL, NULL);
}

void read_key(
    struct InputBuffer *in,
    struct InputString *str,
    KEY_EVENT_RECORD *key
) {
    struct InputRecord *rec = &in->buf[in->len++];

    rec->id.type = BUTTON;
    rec->id.bind = key->wVirtualKeyCode;

    rec->is_down = key->bKeyDown;
    rec->is_up = !key->bKeyDown;

    ucs2_to_utf8(key->uChar.UnicodeChar, IO_BUF_SIZE - str->len, str->buf);
}

bool is_mouse_wheel(DWORD event_flags) {
    return (event_flags & (MOUSE_WHEELED | MOUSE_HWHEELED)) != 0;
}

void read_mouse(struct InputBuffer *in, MOUSE_EVENT_RECORD *mouse)
{
    if (mouse->dwEventFlags == MOUSE_MOVED) {
        struct InputRecord *rec = &in->buf[in->len++];
    
        rec->id.type = JOYSTICK;
        rec->id.bind = 0;
        rec->js.x = mouse->dwMousePosition.X;
        rec->js.y = mouse->dwMousePosition.Y;
    } else if (is_mouse_wheel(mouse->dwEventFlags)) {
        struct InputRecord *rec = &in->buf[in->len++];

        rec->id.type = AXIS;
        rec->id.bind = (mouse->dwEventFlags == MOUSE_WHEELED);
        rec->axis.value = mouse->dwButtonState;
    }
}

bool read_rec(
    struct InputBuffer *in,
    struct InputString *str,
    INPUT_RECORD *rec
) {
    switch (rec->EventType) {
    case KEY_EVENT:
        read_key(in, str, &rec->Event.KeyEvent);
        return true;

    case MOUSE_EVENT:
        read_mouse(in, &rec->Event.MouseEvent);
        return true;

    case WINDOW_BUFFER_SIZE_EVENT:
        return true;
        // TODO: Handle window resizing
    }

    return false;
}

bool PlatformReadInput(struct InputBuffer *in, struct InputString *str)
{
    DWORD max_records = IO_BUF_SIZE - in->len;
    INPUT_RECORD win_buf[max_records];
    DWORD filled;

    if (!ReadConsoleInputW(win.input_hand, win_buf, max_records, &filled))
        return false;

    for (size_t i = 0; i < filled; i++) {
        if (!read_rec(in, str, win_buf + i))
            return false;
    }

    return true;
}

bool PlatformStopInput()
{
    return CancelSynchronousIo(win.input_hand);
}

bool PlatformWait(Time duration)
{
    LARGE_INTEGER nsec_div_100;
    nsec_div_100.QuadPart = -duration / 100;

    if (!SetWaitableTimer(win.timer, &nsec_div_100, 0, NULL, NULL, FALSE))
        return false;

    DWORD result = WaitForSingleObject(win.timer, INFINITE);
    if (result != WAIT_OBJECT_0)
        return false;
    
    return true;
}