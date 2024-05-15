#include "ctrl.h"
#include "ringbuffer.h"

#define INIT_SIZE 16


bool CreateController(struct Controller *ctrl)
{
    struct InputAxis *axes = calloc(16, sizeof(struct InputAxis));
    if (axes == nullptr)
        return false;

    if (!CreateDictionary(&ctrl->binds, sizeof(struct InputAxis*)))
        return false;

    ctrl->pending_len = 0;
    ctrl->axes = axes;
    ctrl->axes_len = 0;

    return true;
}

void FreeController(struct Controller *ctrl)
{
    free(ctrl->axes);
    FreeDictionary(&ctrl->binds);
}

u32 hash_bind(u16f code, u16f type)
{
    return code | (type << 16);
}

bool ControllerBind(struct Controller *ctrl, u16 control, u16 code, u16 type)
{
    u32 hash = hash_bind(code, type);

    struct InputAxis *axis = &ctrl->axes[control];
    struct InputAxis **bind = DictionarySet(&ctrl->binds, hash, &axis);
    
    if (bind == nullptr)
        return false;

    return true;
}

bool ControllerBindMulti(
    struct Controller *ctrl,
    usize n,
    u16 *controls,
    u16 *codes,
    u16 *types
) {
    for (usize i = 0; i < n; i++) {
        if (!ControllerBind(ctrl, controls[i], codes[i], types[i]))
            return false;
    }

    return true;
}

void dispatch_update(struct InputAxis *axis, struct InputRecord *rec)
{
    if (rec->is_down and !axis->is_held) {
        axis->is_down = true;
        axis->is_held = true;
        axis->last_pressed = rec->time;
    } else {
        axis->is_up = true;
        axis->is_held = false;
        axis->last_released = rec->time;
    }

    axis->data = rec->data;
}

void ControllerPoll(struct Controller *ctrl, struct RecordBuffer *recs)
{
    for (usize i = 0; i < ctrl->pending_len; i++) {
        struct InputAxis *axis = ctrl->pending[i];

        axis->is_up = false;
        axis->is_down = false;
    }
    
    ctrl->pending_len = 0;
    
    for (usize i = 0; i < recs->head.len; i++) {
        struct InputRecord *rec = recs->buf + i;
        
        u32 hash = hash_bind(rec->code, rec->type);
        struct InputAxis **axis = DictionaryFind(&ctrl->binds, hash);

        if (axis == nullptr)
            continue;

        dispatch_update(*axis, rec);
        ctrl->pending[ctrl->pending_len++] = *axis;
    }

    recs->head.len = 0;
}