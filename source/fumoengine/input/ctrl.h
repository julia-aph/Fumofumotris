#pragma once
#include "dictionary.h"
#include "fumocommon.h"
#include "input.h"


struct ControlMapping {
    u16 code;
    u16 bind;
    u16 type;
};

struct ControlAxis {
    nsec last_pressed;
    nsec last_released;

    union {
        struct Button but;
        struct Axis axis;
        struct Joystick js;
        union InputData data;
    };

    union InputID id;

    struct {
        u8f is_down : 1;
        u8f is_held : 1;
        u8f is_up : 1;
    };
};

struct Controller {
    struct {
        struct ControlAxis *buf[IO_BUF_SIZE];
        u8f len;
    } pending;

    struct Dictionary codes;
    struct Dictionary binds;
};


bool CreateController(struct Controller *ctrl);

void FreeController(struct Controller *ctrl);

struct ControlAxis *ControllerMap(
    struct Controller *ctrl,
    struct ControlMapping *map
);

bool ControllerMapMulti(
    struct Controller *ctrl,
    usize n,
    struct ControlMapping *maps,
    struct ControlAxis **axis_ptrs
);

struct ControlAxis *ControllerGet(struct Controller *ctrl, u16f code, u16f type);

void ControllerPoll(struct Controller *ctrl, struct RecordBuffer *recs);

