#include "ctrl.h"
#include "ringbuffer.h"

#define INIT_SIZE 16


bool CreateController(struct Controller *ctrl)
{
    struct ctrl_bkt *code_bkts = calloc(INIT_SIZE, sizeof(struct ctrl_bkt));
    struct ctrl_bkt *bind_bkts = calloc(INIT_SIZE, sizeof(struct ctrl_bkt));
    struct ControlAxis *axes = calloc(INIT_SIZE, sizeof(struct ControlAxis));

    if (code_bkts == nullptr or bind_bkts == nullptr or axes == nullptr)
        return false;

    *ctrl = (struct Controller) {
        .pending_buf.len = 0,

        .axis_vec = (struct ctrl_axis_vec) {
            .axes = axes,
            .size = INIT_SIZE,
            .len = 0,
        },
        .codes = (struct ctrl_dict) {
            .bkts = code_bkts,
            .capacity = INIT_SIZE,
            .filled = 0,
        },
        .binds = (struct ctrl_dict) {
            .bkts = bind_bkts,
            .capacity = INIT_SIZE,
            .filled = 0,
        },
    };
    return true;
}

void FreeController(struct Controller *ctrl)
{
    free(ctrl->codes.bkts);
    free(ctrl->binds.bkts);
    free(ctrl->axis_vec.axes);
}

struct ctrl_bkt *get_bkt(struct ctrl_dict *dict, usize i) {
    return &dict->bkts[i];
}

usize wrap_index(usize i, usize max) {
    return i % (SIZE_MAX - max + 1);
}

struct ctrl_bkt *find_set(struct ctrl_dict *dict, union InputID id)
{
    usize i = id.hash % dict->capacity;

    usize last = wrap_index(i - 1, dict->capacity);
    while (i != last) {
        struct ctrl_bkt *bkt = get_bkt(dict, i);

        if (bkt->axis == nullptr) {
            bkt->id.hash = id.hash;
            dict->filled += 1;

            return bkt;
        }

        if (bkt->id.hash == id.hash)
            return bkt;
        
        i = (i + 1) % dict->capacity;
    }

    return nullptr;
}

struct ctrl_bkt *find(struct ctrl_dict *dict, union InputID id)
{
    usize i = id.hash % dict->capacity;

    usize last = wrap_index(i - 1, dict->capacity);
    while (i != last) {
        struct ctrl_bkt *bkt = get_bkt(dict, i);

        if (bkt->id.hash == id.hash)
            return bkt;

        i = (i + 1) % dict->capacity;
    };

    return nullptr;
}

struct ControlAxis *find_axis(struct ctrl_dict *dict, union InputID id)
{
    struct ctrl_bkt *bkt = find(dict, id);
    if (bkt == nullptr)
        return nullptr;

    return bkt->axis;
}

union InputID as_id(u16f value, u16f type) {
    return (union InputID) { .value = value, .type = type };
}

bool ControllerMap(struct Controller *ctrl, struct ControlMapping *mapping)
{
    struct ctrl_bkt *code_bkt = find_set(&ctrl->codes, as_id(mapping->code, mapping->type));
    struct ctrl_bkt *bind_bkt = find_set(&ctrl->binds, as_id(mapping->bind, mapping->type));
    
    if (code_bkt->axis == nullptr)
        code_bkt->axis = &ctrl->axis_vec.axes[ctrl->axis_vec.len++];
    else if (code_bkt->axis == bind_bkt->axis)
        return false;
    
    bind_bkt->axis = code_bkt->axis;
    code_bkt->axis->id.type = mapping->type;

    mapping->axis = code_bkt->axis;
    return true;
}

bool ControllerMapMulti(
    struct Controller *ctrl,
    usize n,
    struct ControlMapping *mappings
) {
    for (usize i = 0; i < n; i++) {
        struct ControlMapping *mapping = mappings + i;

        if (!ControllerMap(ctrl, mapping))
            return false;
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
    for (usize i = 0; i < ctrl->pending_buf.len; i++) {
        struct ControlAxis *axis = ctrl->pending_buf.axes[i];

        axis->is_up = false;
        axis->is_down = false;
    }
    
    ctrl->pending_buf.len = 0;
    
    for (usize i = 0; i < recs->head.len; i++) {
        struct InputRecord *rec = &recs->buf[i];

        struct ControlAxis *axis = find_axis(&ctrl->binds, rec->id);
        if (axis == nullptr)
            continue;

        dispatch_update(axis, rec);
        ctrl->pending_buf.axes[ctrl->pending_buf.len++] = axis;
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