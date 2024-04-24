#include "win.h"

#include <windows.h>

#include "gametime.h"
#include "input.h"
#include "parseinput.h"

#define MOUSE_MOVE (MOUSE_EVENT | MOUSE_MOVED)
#define MOUSE_VWHEEL (MOUSE_EVENT | MOUSE_WHEELED)
#define MOUSE_HWHEEL (MOUSE_EVENT | MOUSE_HWHEELED)

struct windows {
    HANDLE input_hand;
    HANDLE timer;
} win;

struct win_coord {
    SHORT x;
    SHORT y;
};

struct win_rec {
    union {
        WORD type;

        struct {
            WORD is_key : 1;
            WORD is_mouse : 1;
            WORD is_window : 1;
            WORD is_menu : 1;
            WORD is_focus : 1;
        };
    };

    union {
        struct {
            BOOL is_down;
            WORD repeat;
            WORD vk_code;
            WORD vs_code;
            WCHAR ucs2_char;
            DWORD state;
        } key;

        struct {
            struct win_coord pos;
            DWORD but;
            DWORD state;

            union {
                DWORD flags;

                struct {
                    DWORD is_moved : 1;
                    DWORD is_dbl_clk : 1;
                    DWORD is_vwheel : 1;
                    DWORD is_hwheel : 1;
                };

                struct {
                    DWORD : 2;
                    DWORD wheel : 2;
                };
            };
        } mouse;

        struct win_coord window;
    };
};

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

size_t ucs2_to_utf8(char *buf, u16f ucs2)
{
    if (ucs2 < 0xFF) {
        buf[0] = ucs2;
        return 1;
    }

    if (ucs2 < 0x7FF) {
        buf[0] = 0xC0 + (ucs2 >> 6);
        buf[1] = 0x80 + (ucs2 & 0x3F);
        return 2;
    }

    buf[0] = 0xE0 + (ucs2 >> 12);
    buf[1] = 0x80 + ((ucs2 >> 6) & 0x3F);
    buf[2] = 0x80 + (ucs2 & 0x3F);
    return 3;
}

inline u8f win_rec_type(struct win_rec *rec) {
    return rec->type | (rec->is_mouse & rec->mouse.flags);
}

bool read_rec(
    struct InputRecord *out,
    struct InputString *str,
    struct win_rec *in
) {
    switch (win_rec_type(in)) {
    case KEY_EVENT:
        ParseButton(out, in->key.vk_code, in->key.is_down);
        str->len += ucs2_to_utf8(&str->buf[str->len], in->key.ucs2_char);
        return true;

    case MOUSE_MOVE:
        ParseJoystick(out, 0, in->mouse.pos.x, in->mouse.pos.y);
        return true;

    case MOUSE_VWHEEL:
    case MOUSE_HWHEEL:
        ParseAxis(out, in->mouse.is_hwheel, in->mouse.but);
        return true;
        
    case WINDOW_BUFFER_SIZE_EVENT:
        return false;
        // TODO: Handle window resizing
    }

    return false;
}

bool PlatformReadInput(struct InputBuffer *in, struct InputString *str)
{
    DWORD max_records = IO_BUF_SIZE - in->len;
    struct win_rec win_buf[max_records];
    DWORD filled;

    if (!ReadConsoleInputW(win.input_hand, win_buf, max_records, &filled))
        return false;

    for (size_t i = 0; i < filled; i++) {
        if (read_rec(in, str, win_buf + i))
            in->len += 1;
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