#include "ctrl.h"

#include <pthread.h>

#define INIT_SIZE 16

bool CreateCtrl(struct Controller *ctrl)
{
    struct ctrl_bkt *code_bkts = calloc(INIT_SIZE, sizeof(struct ctrl_bkt));
    struct ctrl_bkt *bind_bkts = calloc(INIT_SIZE, sizeof(struct ctrl_bkt));
    struct InputAxis *axes = calloc(INIT_SIZE, sizeof(struct InputAxis));

    if (code_bkts == nullptr or bind_bkts == nullptr or axes == nullptr)
        return false;

    *ctrl = (struct Controller) {
        .buf.len = 0,
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

void FreeCtrl(struct Controller *ctrl)
{
    free(ctrl->codes.bkts);
    free(ctrl->binds.bkts);
    free(ctrl->axis_vec.axes);
}

struct ctrl_bkt *get_bkt(struct ctrl_dict *dict, size_t i) {
    return &dict->bkts[i];
}

size_t wrap_index(size_t i, size_t max) {
    return i % (SIZE_MAX - max + 1);
}

struct ctrl_bkt *find_or_set(struct ctrl_dict *dict, union InputID id)
{
    size_t i = id.hash % dict->capacity;

    size_t last = wrap_index(i - 1, dict->capacity);
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
    size_t i = id.hash % dict->capacity;

    size_t last = wrap_index(i - 1, dict->capacity);
    while (i != last) {
        struct ctrl_bkt *bkt = get_bkt(dict, i);

        if (bkt->id.hash == id.hash)
            return bkt;

        i = (i + 1) % dict->capacity;
    };

    return nullptr;
}

struct InputAxis *find_axis(struct ctrl_dict *dict, union InputID id)
{
    struct ctrl_bkt *bkt = find(dict, id);
    if (bkt == nullptr)
        return nullptr;

    return bkt->axis;
}

union InputID to_id(u16f value, u16f type) {
    return (union InputID) { .value = value, .type = type };
}

bool CtrlMap(struct Controller *ctrl, u16f code, u16f type, u16f bind)
{
    struct ctrl_bkt *code_bkt = find_or_set(&ctrl->codes, to_id(code, type));
    struct ctrl_bkt *bind_bkt = find_or_set(&ctrl->binds, to_id(bind, type));
    
    if (code_bkt->axis == nullptr)
        code_bkt->axis = &ctrl->axis_vec.axes[ctrl->axis_vec.len++];
    else if (code_bkt->axis == bind_bkt->axis)
        return false;
    
    bind_bkt->axis = code_bkt->axis;
    code_bkt->axis->id.type = type;
    return true;
}

struct InputAxis *CtrlGet(struct Controller *ctrl, u16f code, u16f type)
{
    struct ctrl_bkt *code_bkt = find(&ctrl->codes, to_id(code, type));
    if (code_bkt == nullptr)
        return nullptr;

    return code_bkt->axis;
}

void dispatch_update(struct InputAxis *axis, struct InputRecord *rec)
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

bool is_alphanumeric(char c) {
    return c >=32 and c <= 126;
}

bool read_input_buf(struct Controller *ctrl)
{
    for (size_t i = 0; i < ctrl->pending_buf.len; i++) {
        struct InputAxis *axis = ctrl->pending_buf.axes[i];

        axis->is_up = false;
        axis->is_down = false;
    }
    ctrl->pending_buf.len = 0;

    for (size_t i = 0; i < ctrl->buf.len; i++) {
        struct InputRecord *rec = &ctrl->buf.recs[i];

        union InputID rec_id = to_id(rec->bind, rec->type);
        struct InputAxis *axis = find_axis(&ctrl->binds, rec_id);
        
        if (axis == nullptr)
            continue;

        dispatch_update(axis, rec);
        ctrl->pending_buf.axes[ctrl->pending_buf.len++] = axis;
    }

    ctrl->buf.len = 0;
    return true;
}

bool CtrlPoll(struct Controller *ctrl, struct InputThreadHandle *hand)
{
    if (pthread_mutex_lock(&hand->mutex) != 0)
        return false;   
    
    read_input_buf(ctrl);

    if (pthread_cond_signal(&hand->buf_read) != 0)
        return false;

    if (pthread_mutex_unlock(&hand->mutex) != 0)
        return false;
    
    return true;
}

bool CtrlInputString(struct Controller *ctrl, size_t n, char *buf)
{
    
}

/*int main() 
{
    struct Controller ctrl;
    if (!CreateCtrl(&ctrl))
        return 1;

    CtrlMap(&ctrl, 123, BUTTON, 111);
    CtrlMap(&ctrl, 0, BUTTON, 8);
    
    ctrl.buf.recs[ctrl.buf.len++] = (struct InputRecord) {
        .bind = 111,
        .type = BUTTON,
        .is_down = true,
        .but.value = 69
    };
    ctrl.buf.recs[ctrl.buf.len++] = (struct InputRecord) {
        .bind = 8,
        .type = BUTTON,
        .is_down = true,
        .but.value = 1000
    };

    CtrlPoll(&ctrl);

    struct InputAxis *a = CtrlGet(&ctrl, 123, BUTTON);
    printf("%u\n", a->but.value);
    
    struct InputAxis *b = CtrlGet(&ctrl, 0, BUTTON);
    printf("%u\n", b->but.value);

    printf("success");
    return 0;
}*/