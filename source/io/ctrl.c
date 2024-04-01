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

enum InputType {
    KEY,
    AXIS,
    JOYSTICK,
    WINDOW,
    ESCAPE
};

struct InputRecord {
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

struct InputBuffer {
    struct InputRecord records[IO_BUF_SIZE];
    size_t count;
    pthread_mutex_t mutex;
};

struct InputBuffer NewInputBuffer()
{
    struct InputBuffer buf;
    buf.count = 0;
    buf.mutex = PTHREAD_MUTEX_INITIALIZER;
    return buf;
}

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

enum KeyCode {
    LEFT,
    RIGHT,
    SOFT_DROP,
    HARD_DROP,
    ROTATE_CCW,
    ROTATE_CW,
    ROTATE_180,
    SWAP,
    ESC
};

enum AxisCode {
    VSCROLL,
    HSCROLL
};

enum JoystickCode {
    MOUSE
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

struct ident {
    u16 id;
    u8 type;
};

hashtype hash_ident(u16f value, u8f type)
{
    struct ident id = { value, type };
    return Hash(&id, sizeof(struct ident));
}

struct bkt {
    hashtype hash;
    u16 value;
    u8 type;
    struct Axis *axis;
};

struct dict {
    size_t capacity;
    size_t filled;
    struct bkt *bkts;
};

struct Ctrl {
    struct dict codes;
    struct dict binds;

    pthread_t thread;
    pthread_mutex_t mutex;
};
typedef struct Ctrl Ctrl;

Ctrl NewCtrl(struct bkt *codes, struct Axis *axes, size_t c, struct bkt *binds, size_t b)
{
    memset(codes, 0, sizeof(struct bkt) * c);
    memset(axes, 0, sizeof(struct Axis) * c);
    memset(binds, 0, sizeof(struct bkt) * b);

    for (size_t i = 0; i < c; i++) {
        codes[i].axis = &axes[i];
    }

    Ctrl ctrl;

    ctrl.codes.capacity = c;
    ctrl.codes.filled = 0;
    ctrl.codes.bkts = codes;

    ctrl.binds.capacity = b;
    ctrl.binds.filled = 0;
    ctrl.binds.bkts = binds;

    ctrl.mutex = PTHREAD_MUTEX_INITIALIZER;

    return ctrl;
}

struct bkt *get_bkt(struct dict *dict, size_t i)
{
    assert(i < dict->capacity);
    return &dict->bkts[i];
}

void set_bkt(struct bkt *bkt, hashtype hash, u16f value, u8f type)
{
    bkt->hash = hash;
    bkt->value = value;
    bkt->type = type;
}

size_t wrap(size_t x, size_t wrap)
{
    return x % (SIZE_MAX - wrap + 1);
}

bool find_or_set(struct dict *dict, struct bkt **out, u16f value, u8f type)
{
    hashtype hash = hash_ident(value, type);
    const size_t index = hash % dict->capacity;

    size_t i = index;
    while (i != wrap(index - 1, dict->capacity)) {
        struct bkt *bkt = get_bkt(dict, i);

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

struct bkt *find(struct dict *dict, u16f value, u8f type)
{
    hashtype hash = hash_ident(value, type);
    const size_t index = hash % dict->capacity;

    size_t i = index;
    while (i != wrap(index - 1, dict->capacity)) {
        struct bkt *bkt = get_bkt(dict, i);
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

struct Axis *find_axis(struct dict *dict, u16f value, u8f type)
{
    struct bkt *bkt = find(dict, value, type);
    if (bkt == nullptr)
        return nullptr;

    return bkt->axis;
}

bool CtrlMap(Ctrl *ctrl, u16f code, u16f bind, u8f type)
{
    assert(ctrl->codes.filled < ctrl->codes.capacity);
    assert(ctrl->binds.filled < ctrl->binds.capacity);

    struct bkt *code_bkt;
    find_or_set(&ctrl->codes, &code_bkt, code, type);
    assert(code_bkt != nullptr);

    struct bkt *bind_bkt;
    bool bind_existed = find_or_set(&ctrl->binds, &bind_bkt, bind, type);
    assert(bind_bkt != nullptr);
    
    if(bind_existed and bind_bkt->axis == code_bkt->axis)
        return false;
    
    bind_bkt->axis = code_bkt->axis;
    return true;
}

struct Axis *CtrlGet(Ctrl *ctrl, u16f code, u8f type)
{
    struct bkt *code_bkt = find(&ctrl->codes, code, type);
    if (code_bkt == nullptr)
        return nullptr;

    return code_bkt->axis;
}

void update_key(struct Axis *axis, struct InputRecord *record)
{
    if (record->data.key.is_down)
        axis->last_pressed = record->timestamp;
    else
        axis->last_released = record->timestamp;

    axis->data.key.is_down = record->data.key.is_down;
}

void update_axis(struct Axis *axis, struct InputRecord *record)
{
    axis->data.axis.value = record->data.axis.value;
    axis->last_pressed = record->timestamp;
}

void update_joystick(struct Axis *axis, struct InputRecord *record)
{
    axis->data.joystick.x = record->data.joystick.x;
    axis->data.joystick.y = record->data.joystick.y;
    axis->last_pressed = record->timestamp;
}

bool CtrlPoll(Ctrl *ctrl, struct InputBuffer *buf)
{
    pthread_mutex_lock(&buf->mutex);
    pthread_mutex_lock(&ctrl->mutex);

    for (size_t i = 0; i < buf->count; i++) {
        struct InputRecord *rec = &buf->records[i];

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
        case WINDOW:
            update_joystick(axis, rec);
            break;
        default:
            return false;
        }
    }

    buf->count = 0;
    pthread_mutex_unlock(&buf->mutex);
    pthread_mutex_unlock(&ctrl->mutex);
    return true;
}