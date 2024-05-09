#pragma once
#include "dictionary.h"
#include "fumocommon.h"
#include "input.h"


struct ControlMapping {
    u16 code;
    u16 bind;
    u16 type;
};

struct InputAxis {
    nsec last_pressed;
    nsec last_released;

    union {
        struct Button but;
        struct Axis axis;
        struct Joystick js;
        union InputData data;
    };

    u16 type;

    bool is_down;
    bool is_held;
    bool is_up;
};

struct Controller {
    struct InputAxis *pending[IO_BUF_SIZE];
    usize pending_len;

    struct InputAxis *axes;
    usize axes_len;

    struct Dictionary binds;
};


bool CreateController(struct Controller *ctrl);

void FreeController(struct Controller *ctrl);

struct InputAxis *ControllerMap(
    struct Controller *ctrl,
    struct ControlMapping *map
);

bool ControllerMapMulti(
    struct Controller *ctrl,
    usize n,
    struct ControlMapping *maps,
    struct InputAxis **axis_ptrs
);

void ControllerPoll(struct Controller *ctrl, struct RecordBuffer *recs);

