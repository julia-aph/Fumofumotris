#include <iso646.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "fumotris.h"

#define IO_BUF_SIZE 16

enum InputType {
    KEY,
    AXIS,
    JOYSTICK,
    ESCAPE
};

struct Button {
    u32 value;
    bool is_down;
    bool is_held;
    bool is_up;
};

struct Axis {
    i64 value;
};

struct Joystick {
    i32 x;
    i32 y;
};

struct InputRecord {
    u16 bind;
    u8 type;

    union {
        struct Button but;
        struct Axis axis;
        struct Joystick js;
    };

    struct timespec timestamp;
};

struct InputAxis {
    u8 type;
    
    union {
        struct Button but;
        struct Axis axis;
        struct Joystick js;
    };

    struct timespec last_pressed;
    struct timespec last_released;
};

typedef u32 hashtype;
hashtype Hash(void *item, size_t size)
{
    u8 *data = (u8 *)item;

    u32 h = 98317;
    for (size_t i = 0; i < size; i++) {
        h ^= data[i];
        h *= 0x5bd1e995;
        h ^= h >> 15;
    }

    return h;
}

hashtype hash_id(u16f value, u8f type)
{
    struct { u16 id; u8 type; } id = { value, type };
    return Hash(&id, sizeof(id));
}

struct ctrl_dict {
    size_t capacity;
    size_t filled;

    struct ctrl_bkt {
        hashtype hash;
        u16 value;
        u8 type;

        struct InputAxis *axis;
    } *bkts;
};

struct Controller {
    struct ctrl_dict codes;
    struct ctrl_dict binds;
    struct InputAxis *axes;

    struct {
        struct InputRecord records[IO_BUF_SIZE];
        size_t len;
    } input_buf;

    struct {
        size_t indexes[IO_BUF_SIZE];
        size_t len;
    } pending_buf;
};

bool NewCtrl(struct Controller *ctrl, size_t code_cap, size_t bind_cap)
{
    struct ctrl_bkt *code_bkts = calloc(code_cap, sizeof(struct ctrl_bkt));
    struct ctrl_bkt *bind_bkts = calloc(bind_cap, sizeof(struct ctrl_bkt));
    struct InputAxis *axes = calloc(code_cap, sizeof(struct InputAxis));

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
        
        .input_buf = {
            .len = 0,
        },
        .pending_buf = {
            .len = 0,
        },
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

struct InputAxis *find_axis(struct ctrl_dict *dict, u16f value, u8f type)
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

struct InputAxis *CtrlGet(struct Controller *ctrl, u16f code, u8f type)
{
    struct ctrl_bkt *code_bkt = find(&ctrl->codes, code, type);
    if (code_bkt == nullptr)
        return nullptr;

    return code_bkt->axis;
}

void update_key(struct InputAxis *axis, struct InputRecord *rec)
{
    if (rec->but.is_down) {
        axis->last_pressed = rec->timestamp;
        axis->but.is_held = true;
    } else {
        axis->last_released = rec->timestamp;
        axis->but.is_held = false;
    }
    
    axis->but.is_down |= rec->but.is_down;
    axis->but.is_up |= rec->but.is_up;
}

void update_axis(struct InputAxis *axis, struct InputRecord *rec)
{
    axis->axis.value = rec->axis.value;

    axis->last_pressed = rec->timestamp;
}

void update_joystick(struct InputAxis *axis, struct InputRecord *rec)
{
    axis->js.x = rec->js.x;
    axis->js.y = rec->js.y;

    axis->last_pressed = rec->timestamp;
}

bool dispatch_update(struct InputAxis *axis, struct InputRecord *rec)
{
    switch (rec->type) {
    case KEY:
        update_key(axis, rec);
        printf("axis:%hu\n", axis->but.is_down);
        return true;

    case AXIS:
        update_axis(axis, rec);
        return true;
        
    case JOYSTICK:
        update_joystick(axis, rec);
        return true;
    }

    return false;
}

bool CtrlPoll(struct Controller *ctrl)
{
    for (size_t i = 0; i < ctrl->pending_buf.len; i++) {

    }

    for (size_t i = 0; i < ctrl->input_buf.len; i++) {
        struct InputRecord *rec = &ctrl->input_buf.records[i];
        printf("i:%hu\n", rec->bind);

        struct InputAxis *axis = find_axis(&ctrl->binds, rec->bind, rec->type);
        if (axis == nullptr)
            continue;

        if (!dispatch_update(axis, rec))
            return false;
    }

    ctrl->input_buf.len = 0;
    return true;
}