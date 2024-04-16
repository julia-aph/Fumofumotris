#include <iso646.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "fumotris.h"
#include "hash.h"
#include "input.h"

struct CtrlAxis {
    struct timespec last_pressed;
    struct timespec last_released;

    u8 type;
    u8 is_down;
    u8 is_held;
    u8 is_up;

    union {
        struct Button but;
        struct Axis axis;
        struct Joystick js;
        union InputData data;
    };
};

struct ctrl_dict {
    size_t capacity;
    size_t filled;

    struct ctrl_bkt {
        hashtype hash;
        u16 value;
        u8 type;

        struct CtrlAxis *axis;
    } *bkts;
};

struct Controller {
    struct ctrl_dict codes;
    struct ctrl_dict binds;
    struct CtrlAxis *axes;

    struct InputBuffer input_buf;

    struct {
        size_t len;
        struct CtrlAxis *axes[IO_BUF_SIZE];
    } pending_buf;
};

bool NewCtrl(struct Controller *ctrl, size_t code_cap, size_t bind_cap)
{
    struct ctrl_bkt *code_bkts = calloc(code_cap, sizeof(struct ctrl_bkt));
    struct ctrl_bkt *bind_bkts = calloc(bind_cap, sizeof(struct ctrl_bkt));
    struct CtrlAxis *axes = calloc(code_cap, sizeof(struct CtrlAxis));

    if (code_bkts == nullptr or bind_bkts == nullptr or axes == nullptr)
        return false;

    for (size_t i = 0; i < code_cap; i++) {
        code_bkts[i].axis = axes + i;
    }

    *ctrl = (struct Controller) {
        .codes = (struct ctrl_dict) {
            .capacity = code_cap,
            .filled = 0,
            .bkts = code_bkts,
        },
        .binds = (struct ctrl_dict) {
            .capacity = bind_cap,
            .filled = 0,
            .bkts = bind_bkts,
        },
        .axes = axes,
        
        .input_buf = (struct InputBuffer) {
            .len = 0,
        },
        .pending_buf.len = 0,
    };
    return true;
}

void FreeCtrl(struct Controller *ctrl)
{
    free(ctrl->codes.bkts);
    free(ctrl->binds.bkts);
    free(ctrl->axes);
}

struct ctrl_bkt *get_bkt(struct ctrl_dict *dict, size_t i)
{
    return &dict->bkts[i];
}

void set_bkt(struct ctrl_bkt *bkt, hashtype hash, u16f value, u8f type)
{
    bkt->hash = hash;
    bkt->value = value;
    bkt->type = type;
}

size_t wrap_index(size_t i, size_t max)
{
    return i % (SIZE_MAX - max + 1);
}

hashtype hash_id(u16f value, u8f type)
{
    struct { u16 id; u8 type; } id = { value, type };
    return Hash(&id, sizeof(id));
}

bool find_or_set(struct ctrl_dict *dict, struct ctrl_bkt **out, u16f value, u8f type)
{
    hashtype hash = hash_id(value, type);
    const size_t index = hash % dict->capacity;

    size_t i = index;
    while (i != wrap_index(index - 1, dict->capacity)) {
        struct ctrl_bkt *bkt = get_bkt(dict, i);

        if (bkt->hash == 0) {
            set_bkt(bkt, hash, value, type);
            dict->filled += 1;
            *out = bkt;
            return false;
        }

        if (bkt->value == value and bkt->type == type) {
            *out = bkt;
            return true;
        }
        
        i = (i + 1) % dict->capacity;
    }

    *out = nullptr;
    return false;
}

struct ctrl_bkt *find(struct ctrl_dict *dict, u16f value, u8f type)
{
    hashtype hash = hash_id(value, type);
    const size_t index = hash % dict->capacity;

    size_t i = index;
    while (i != wrap_index(index - 1, dict->capacity)) {
        struct ctrl_bkt *bkt = get_bkt(dict, i);
        if (bkt->hash == 0)
            goto next;

        if (bkt->value == value and bkt->type == type)
            return bkt;

next:
        i = (i + 1) % dict->capacity;
    };

    return nullptr;
}

struct CtrlAxis *find_axis(struct ctrl_dict *dict, u16f value, u8f type)
{
    struct ctrl_bkt *bkt = find(dict, value, type);
    if (bkt == nullptr)
        return nullptr;

    return bkt->axis;
}

bool CtrlMap(struct Controller *ctrl, u16f code, u16f bind, u8f type)
{
    if (ctrl->codes.filled >= ctrl->codes.capacity or ctrl->binds.filled >= ctrl->binds.capacity) {
        printf("fatal error");
        exit(1);
    }

    struct ctrl_bkt *code_bkt;
    find_or_set(&ctrl->codes, &code_bkt, code, type);

    struct ctrl_bkt *bind_bkt;
    bool bind_existed = find_or_set(&ctrl->binds, &bind_bkt, bind, type);
    
    if(bind_existed and bind_bkt->axis == code_bkt->axis)
        return false;
    
    bind_bkt->axis = code_bkt->axis;
    code_bkt->axis->type = type;
    return true;
}

struct CtrlAxis *CtrlGet(struct Controller *ctrl, u16f code, u8f type)
{
    struct ctrl_bkt *code_bkt = find(&ctrl->codes, code, type);
    if (code_bkt == nullptr)
        return nullptr;

    return code_bkt->axis;
}

void dispatch_update(struct CtrlAxis *axis, struct InputRecord *rec)
{
    if (rec->is_down and !axis->is_held) {
        axis->is_down = true;
        axis->is_held = true;
        axis->last_pressed = rec->timestamp;
    } else if (rec->is_up) {
        axis->is_up = true;
        axis->is_held = false;
        axis->last_released = rec->timestamp;
    }

    axis->data = rec->data;
}

bool CtrlPoll(struct Controller *ctrl)
{
    for (size_t i = 0; i < ctrl->pending_buf.len; i++) {
        struct CtrlAxis *axis = ctrl->pending_buf.axes[i];

        axis->is_up = false;
        axis->is_down = false;
    }
    ctrl->pending_buf.len = ctrl->input_buf.len;

    for (size_t i = 0; i < ctrl->input_buf.len; i++) {
        struct InputRecord *rec = &ctrl->input_buf.records[i];
        struct CtrlAxis *axis = find_axis(&ctrl->binds, rec->bind, rec->type);

        if (axis == nullptr)
            continue;

        dispatch_update(axis, rec);

        ctrl->pending_buf.axes[i] = axis;
    }
    ctrl->input_buf.len = 0;

    return true;
}

int main() 
{
    struct Controller ctrl;
    if (!NewCtrl(&ctrl, 3, 3))
        return 1;

    CtrlMap(&ctrl, 123, 111, BUTTON);
    
    ctrl.input_buf.records[ctrl.input_buf.len++] = (struct InputRecord) {
        .bind = 111,
        .type = BUTTON,
        .is_down = true,
        .but.value = 69
    };

    CtrlPoll(&ctrl);

    struct CtrlAxis *a = CtrlGet(&ctrl, 123, BUTTON);
    printf("%u\n", a->but.value);

    printf("success");
    return 0;
}