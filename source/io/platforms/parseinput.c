#include "parseinput.h"

void ParseButton(struct InputRecord *rec, u16f bind, bool is_down)
{
    rec->id = (union InputID) { .bind = bind, .type = BUTTON };
    rec->is_down = is_down;
    rec->is_up = !is_down;
}

void ParseAxis(struct InputRecord *rec, u16f bind, u64 value)
{
    rec->id = (union InputID) { .bind = bind, .type = AXIS };
    rec->axis.value = value;
}

void ParseJoystick(struct InputRecord *rec, u16f bind, i32 x, i32 y)
{
    rec->id = (union InputID) { .bind = bind, .type = JOYSTICK };
    rec->js.x = x;
    rec->js.y = y;
}