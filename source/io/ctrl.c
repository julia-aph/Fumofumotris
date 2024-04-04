#include <assert.h>
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

enum CtrlType {
    KEY,
    AXIS,
    JOYSTICK,
    ESCAPE
};

struct Record {
    u16 id;
    u8 type;

    union {
        struct {
            bool is_down;
        } key;
        struct {
            u64 value;
        } axis;
        struct {
            u32 x;
            u32 y;
        } joystick;
    } data;

    struct timespec timestamp;
};

struct RecordBuffer {
    struct Record records[IO_BUF_SIZE];
    size_t count;
    pthread_mutex_t mutex;
};

struct Axis {
    union {
        struct {
            u32 value;
            u32 is_down : 1;
            u32 is_up : 1;
        } key;
        struct {
            u64 value;
        } axis;
        struct {
            u32 x;
            u32 y;
        } joystick;
    } data;

    struct timespec last_pressed;
    struct timespec last_released;
};

typedef u32 hashtype;
hashtype Hash(void *item, size_t size)
{
    u8* data = (u8*)item;

    u32 h = 98317;
    for (size_t i = 0; i < size; i++) {
        h ^= data[i];
        h *= 0x5bd1e995;
        h ^= h >> 15;
    }

    return h;
}

hashtype hash_ident(u16f value, u8f type)
{
    struct id {
        u16 id;
        u8 type;
    };

    struct id id = { value, type };
    return Hash(&id, sizeof(struct id));
}

struct ctrl_bkt {
    hashtype hash;
    u16 value;
    u8 type;
    struct Axis *axis;
};

struct ctrl_dict {
    size_t capacity;
    size_t filled;
    struct ctrl_bkt *bkts;
};

struct Ctrl {
    struct ctrl_dict codes;
    struct ctrl_dict binds;

    pthread_t thread;
    pthread_mutex_t mutex;
};

struct Ctrl NewCtrl()
{
    struct Ctrl ctrl = (struct Ctrl) {
        .codes = (struct ctrl_dict) {
            .capacity = 0,
            .filled = 0,
            .bkts = nullptr
        },
        .binds = (struct ctrl_dict) {
            .capacity = 0,
            .filled = 0,
            .bkts = nullptr
        },

        .mutex = PTHREAD_MUTEX_INITIALIZER
    };
    return ctrl;
}

void CtrlSet(
    struct Ctrl *ctrl,
    struct ctrl_bkt *code_bkts,
    struct ctrl_bkt *bind_bkts,
    struct Axis *axes,
    size_t c_len,
    size_t b_len
) {
    ctrl->codes.capacity = c_len;
    ctrl->codes.bkts = code_bkts;
    memset(code_bkts, 0, sizeof(struct ctrl_bkt) * c_len);

    ctrl->binds.capacity = b_len;
    ctrl->binds.bkts = bind_bkts;
    memset(bind_bkts, 0, sizeof(struct ctrl_bkt) * b_len);
    
    
}

struct ctrl_bkt *get_bkt(struct ctrl_dict *dict, size_t i)
{
    assert(i < dict->capacity);
    return &dict->bkts[i];
}

void set_bkt(struct ctrl_bkt *bkt, hashtype hash, u16f value, u8f type)
{
    bkt->hash = hash;
    bkt->value = value;
    bkt->type = type;
}

size_t wrap(size_t x, size_t wrap)
{
    return x % (SIZE_MAX - wrap + 1);
}

bool find_or_set(struct ctrl_dict *dict, struct ctrl_bkt **out, u16f value, u8f type)
{
    hashtype hash = hash_ident(value, type);
    const size_t index = hash % dict->capacity;

    size_t i = index;
    while (i != wrap(index - 1, dict->capacity)) {
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
    hashtype hash = hash_ident(value, type);
    const size_t index = hash % dict->capacity;

    size_t i = index;
    while (i != wrap(index - 1, dict->capacity)) {
        struct ctrl_bkt *bkt = get_bkt(dict, i);
        if (bkt->hash == 0)
            goto next;

        if (bkt->value == value and bkt->type == type) {
            return bkt;
        }
next:
        i = (i + 1) % dict->capacity;
    };

    return nullptr;
}

struct Axis *find_axis(struct ctrl_dict *dict, u16f value, u8f type)
{
    struct ctrl_bkt *bkt = find(dict, value, type);
    if (bkt == nullptr)
        return nullptr;

    return bkt->axis;
}

bool CtrlMap(struct Ctrl *ctrl, u16f code, u16f bind, u8f type)
{
    assert(ctrl->codes.filled < ctrl->codes.capacity);
    assert(ctrl->binds.filled < ctrl->binds.capacity);

    struct ctrl_bkt *code_bkt;
    find_or_set(&ctrl->codes, &code_bkt, code, type);
    assert(code_bkt != nullptr);

    struct ctrl_bkt *bind_bkt;
    bool bind_existed = find_or_set(&ctrl->binds, &bind_bkt, bind, type);
    assert(bind_bkt != nullptr);
    
    if(bind_existed and bind_bkt->axis == code_bkt->axis)
        return false;
    
    bind_bkt->axis = code_bkt->axis;
    return true;
}

struct Axis *CtrlGet(struct Ctrl *ctrl, u16f code, u8f type)
{
    struct ctrl_bkt *code_bkt = find(&ctrl->codes, code, type);
    if (code_bkt == nullptr)
        return nullptr;

    return code_bkt->axis;
}

void update_key(struct Axis *axis, struct Record *record)
{
    if (record->data.key.is_down)
        axis->last_pressed = record->timestamp;
    else
        axis->last_released = record->timestamp;

    axis->data.key.is_down = record->data.key.is_down;
}

void update_axis(struct Axis *axis, struct Record *record)
{
    axis->data.axis.value = record->data.axis.value;
    axis->last_pressed = record->timestamp;
}

void update_joystick(struct Axis *axis, struct Record *record)
{
    axis->data.joystick.x = record->data.joystick.x;
    axis->data.joystick.y = record->data.joystick.y;
    axis->last_pressed = record->timestamp;
}

bool CtrlPoll(struct Ctrl *ctrl, struct RecordBuffer *rec_buf)
{
    for (size_t i = 0; i < rec_buf->count; i++) {
        struct Record *rec = &rec_buf->records[i];

        struct Axis *axis = find_axis(&ctrl->binds, rec->id, rec->type);
        if (axis == nullptr)
            continue;

        switch (rec->type) {
        case KEY:
            update_key(axis, rec);
            break;
        case AXIS:
            update_axis(axis, rec);
            break;
        case JOYSTICK:
            update_joystick(axis, rec);
            break;
        default:
            return false;
        }
    }

    rec_buf->count = 0;
    return true;
}