#include "ctrl.h"
#include "ringbuffer.h"

#define INIT_SIZE 16


DictT CODES_T = DICT_T(struct ControlAxis);
DictT BINDS_T = DICT_T(struct ControlAxis *);


bool CreateController(struct Controller *ctrl)
{
    if (!CreateDictionary(CODES_T, &ctrl->codes))
        return false;

    if (!CreateDictionary(BINDS_T, &ctrl->binds))
        return false;

    *ctrl = (struct Controller) { .pending.len = 0 };

    return true;
}

void FreeController(struct Controller *ctrl)
{
    FreeDictionary(&ctrl->codes);
    FreeDictionary(&ctrl->binds);
}

struct ControlAxis *find_axis(struct ctrl_dict *dict, union InputID id)
{
    struct ctrl_bkt *bkt = find(dict, id);
    if (bkt == nullptr)
        return nullptr;

    return bkt->axis;
}

struct ControlAxis *ControllerMap(
    struct Controller *ctrl,
    struct ControlMapping *map
) {
    struct ControlAxis *axis = DictionarySet(CODES_T, &ctrl->codes, map->code);
    struct ctrl_bkt *bind_bkt = set(&ctrl->binds, map->bind, map->type);
    
    if (code_bkt->axis == nullptr)
        code_bkt->axis = &ctrl->axis_vec.axes[ctrl->axis_vec.len++];
    else if (code_bkt->axis == bind_bkt->axis)
        return nullptr;
    
    bind_bkt->axis = code_bkt->axis;
    code_bkt->axis->id.type = map->type;

    return code_bkt->axis;
}

bool ControllerMapMulti(
    struct Controller *ctrl,
    usize n,
    struct ControlMapping *maps,
    struct ControlAxis **axis_ptrs
) {
    for (usize i = 0; i < n; i++) {
        struct ControlAxis *axis = ControllerMap(ctrl, maps + i);

        if (axis == nullptr)
            return false;

        axis_ptrs[i] = axis;
    }

    return true;
}

struct ControlAxis *ControllerGet(struct Controller *ctrl, u16f code, u16f type)
{
    struct ctrl_bkt *code_bkt = find(&ctrl->codes, as_id(code, type));
    if (code_bkt == nullptr)
        return nullptr;

    return code_bkt->axis;
}

void dispatch_update(struct ControlAxis *axis, struct InputRecord *rec)
{
    if (rec->is_down and !axis->is_held) {
        axis->is_down = true;
        axis->is_held = true;
        axis->last_pressed = rec->time;
    } else if (rec->is_up) {
        axis->is_up = true;
        axis->is_held = false;
        axis->last_released = rec->time;
    }

    axis->data = rec->data;
}

void ControllerPoll(struct Controller *ctrl, struct RecordBuffer *recs)
{
    for (usize i = 0; i < ctrl->pending.len; i++) {
        struct ControlAxis *axis = ctrl->pending.buf[i];

        axis->is_up = false;
        axis->is_down = false;
    }
    
    ctrl->pending.len = 0;
    
    for (usize i = 0; i < recs->head.len; i++) {
        struct InputRecord *rec = &recs->buf[i];

        struct ControlAxis *axis = find_axis(&ctrl->binds, rec->id);
        if (axis == nullptr)
            continue;

        dispatch_update(axis, rec);
        ctrl->pending.buf[ctrl->pending.len++] = axis;
    }

    recs->head.len = 0;
}

/*int main() 
{
    struct Controller ctrl;
    if (!CreateController(&ctrl))
        return 1;

    ControllerMap(&ctrl, 123, 111, BUTTON);
    ControllerMap(&ctrl, 0, 8, BUTTON);
    
    struct RecordBuffer recs = { .head.len = 0, .head.start = 0 };

    recs.buf[recs.head.len++] = (struct InputRecord) {
        .but.value = 69,

        .is_down = true,

        .id.bind = 111,
        .id.type = BUTTON
    };
    recs.buf[recs.head.len++] = (struct InputRecord) {
        .but.value = 1000,
        
        .is_down = true,

        .id.bind = 8,
        .id.type = BUTTON
    };

    ControllerPoll(&ctrl, &recs);

    struct InputAxis *a = ControllerGet(&ctrl, 123, BUTTON);
    printf("%u\n", a->but.value);
    
    struct InputAxis *b = ControllerGet(&ctrl, 0, BUTTON);
    printf("%u\n", b->but.value);

    printf("success");
    return 0;
}*/