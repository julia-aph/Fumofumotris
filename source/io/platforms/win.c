#include "win.h"

#include <windows.h>

#include "gametime.h"
#include "input.h"
#include "parseinput.h"
#include "ringbuffer.h"

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

struct win_key {
    BOOL is_down;
    WORD repeat;
    WORD vk_code;
    WORD vs_code;
    WCHAR ucs2_char;
    DWORD state;
};

struct win_mouse {
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
        struct win_key key;
        struct win_mouse mouse;
        struct win_coord window;
    };
};

union record {
    struct win_rec native;
    INPUT_RECORD win;
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

bool dispatch_rec(struct InputRecord *out, struct win_rec *rec) {
    u8f type = rec->type
        | (rec->is_mouse & rec->mouse.flags)
        | (rec->is_key & rec->key.is_down);

    switch (type) {
        case KEY_EVENT: {
            ReadButton(out, rec->key.vk_code, rec->key.is_down);

            return true;
        }
        case MOUSE_MOVE: {
            ReadJoystick(out, 0, rec->mouse.pos.x, rec->mouse.pos.y);

            return true;
        }
        case MOUSE_VWHEEL: {
            ReadAxis(out, 0, rec->mouse.but);

            return true;
        }
        case MOUSE_HWHEEL: {
            ReadAxis(out, 1, rec->mouse.but);

            return true;
        }
        case WINDOW_BUFFER_SIZE_EVENT: {
            // TODO: Handle window resizing
            
            return false;
        }
    }

    return false;
}

bool PlatformReadInput(struct InputRecordBuf *recs)
{
    DWORD read_max = RingBufferEmpty(&IO_BUF_T, &recs->head);
    union record win_buf[read_max];
    DWORD filled;

    if (!ReadConsoleInputW(win.input_hand, &win_buf->win, read_max, &filled))
        return false;

    Time now = TimeNow();

    for (size_t i = 0; i < filled; i++) {
        struct InputRecord *rec = RingBufferNext(&IO_BUF_T, &recs->head);
        
        if (dispatch_rec(rec, &win_buf->native + i)) {
            rec->time = now;
            recs->head.len += 1;
        }
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