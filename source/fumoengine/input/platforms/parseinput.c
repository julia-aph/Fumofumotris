#include "parseinput.h"


void ReadButton(struct InputRecord *rec, u16f bind, bool is_down)
{
    rec->id = (union InputID) { .bind = bind, .type = BUTTON };
    rec->is_down = is_down;
    rec->is_up = !is_down;
}

void ReadAxis(struct InputRecord *rec, u16f bind, u64 value)
{
    rec->id = (union InputID) { .bind = bind, .type = AXIS };
    rec->axis.value = value;
}

void ReadJoystick(struct InputRecord *rec, u16f bind, i32 x, i32 y)
{
    rec->id = (union InputID) { .bind = bind, .type = JOYSTICK };
    rec->js.x = x;
    rec->js.y = y;
}

size_t UCS2ToUTF8(char *buf, u16f ucs2)
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