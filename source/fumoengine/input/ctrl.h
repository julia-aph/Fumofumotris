#pragma once
#include "dictionary.h"
#include "fumocommon.h"
#include "input.h"


struct Controller {
    struct InputAxis *pending[IO_BUF_SIZE];
    usize pending_len;

    struct InputAxis *axes;
    usize axes_len;

    struct Dictionary binds;
};


bool CreateController(struct Controller *ctrl);

void FreeController(struct Controller *ctrl);

bool ControllerBind(struct Controller *ctrl, u16 control, u16 code, u16 type);

bool ControllerBindMulti(
    struct Controller *ctrl,
    usize n,
    u16 *controls,
    u16 *codes,
    u16 *types
);

void ControllerPoll(struct Controller *ctrl, struct RecordBuffer *recs);

