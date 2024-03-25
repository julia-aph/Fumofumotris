#include <iso646.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fumotris.h"

#define IO_BUF_SIZE 8

enum InputType {
    KEY,
    AXIS,
    JOYSTICK,
    WINDOW,
    ESCAPE
};

struct InputRecord {
    enum InputType type;
    u16 id;

    union {
        struct {
            u32 value;
            bool is_down;
        } axis;
        struct {
            u32 x;
            u32 y;
        } joystick;
    } data;

    double timestamp;
};

struct InputResult {
    size_t count;
    struct InputRecord buf[IO_BUF_SIZE];
};

struct Axis {
    union {
        struct {
            u32 value;
            bool is_down;
        } axis;
        struct {
            u32 x;
            u32 y;
        } joystick;
    } data;

    double last_pressed;
    double last_released;
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
    enum InputType type;
};

hashtype hash_ident(u16f id, enum InputType type)
{
    struct ident obj = { id, type };
    return Hash(&obj, sizeof(struct ident));
}

struct ctrl_bkt {
    hashtype bind_hash;
    size_t index;
    hashtype code_hash;

    struct Axis axis;
};

struct Ctrl {
    size_t capacity;
    size_t filled;
    struct ctrl_bkt *bkts;

    pthread_mutex_t mutex;
};
typedef struct Ctrl Ctrl;

Ctrl NewCtrl(struct ctrl_bkt *bkts_prealloc, size_t capacity)
{
    Ctrl ctrl;
    ctrl.capacity = capacity;
    ctrl.filled = 0;
    memset(bkts_prealloc, 0, sizeof(struct ctrl_bkt) * capacity);
    ctrl.bkts = bkts_prealloc;
    ctrl.mutex = PTHREAD_MUTEX_INITIALIZER;
    return ctrl;
}

struct ctrl_bkt *get_bkt(Ctrl *ctrl, size_t i)
{
    return &ctrl->bkts[i];
}

struct ctrl_bkt *find_bind(Ctrl *ctrl, hashtype bind_hash, hashtype search)
{
    size_t i = bind_hash % ctrl->capacity;

    struct ctrl_bkt *next;
    for (size_t offset = 0; offset < ctrl->capacity; offset++) {
        i = (i + 1) % ctrl->capacity;

        next = get_bkt(ctrl, i);
        if (next->bind_hash != search)
            continue;
        
        return next;
    }
    return nullptr;
}

struct ctrl_bkt *find_code(Ctrl *ctrl, size_t *i, hashtype search)
{
    struct ctrl_bkt *next;
    for (size_t offset = 0; offset < ctrl->capacity; offset++) {
        *i = (*i + 1) % ctrl->capacity;

        next = get_bkt(ctrl, *i);
        if (next->code_hash != search)
            continue;

        return next;
    }
    return nullptr;
}

bool CtrlMap(Ctrl *ctrl, u16f bind, u16f code, enum InputType type)
{
    if (ctrl->filled == ctrl->capacity)
        return false;

    hashtype bind_hash = hash_ident(bind, type);
    struct ctrl_bkt *bind_bkt = find_bind(ctrl, bind_hash, 0);
    bind_bkt->bind_hash = bind_hash;

    hashtype code_hash = hash_ident(code, type);
    size_t code_i = code_hash % ctrl->capacity;
    struct ctrl_bkt *code_bkt = find_code(ctrl, &code_i, 0);
    code_bkt->code_hash = code_hash;
    
    bind_bkt->index = code_i;
    ctrl->filled += 1;

    return true;
}

struct Axis *find_bind_axis(Ctrl *ctrl, u16f bind, enum InputType type)
{
    hashtype bind_hash = hash_ident(bind, KEY);
    struct ctrl_bkt *bind_bkt = find_bind(ctrl, bind_hash, bind_hash);
    if (bind_bkt == nullptr)
        return nullptr;

    return &get_bkt(ctrl, bind_bkt->index)->axis;
}

struct Axis *CtrlGet(Ctrl *ctrl, u16f code, enum InputType type)
{
    hashtype code_hash = hash_ident(code, type);
    size_t code_i = code_hash % ctrl->capacity;
    struct ctrl_bkt *code_bkt = find_code(ctrl, &code_i, code_hash);
    if (code_bkt == nullptr)
        return nullptr;

    return &code_bkt->axis;
}

bool CtrlUpdateKey(Ctrl *ctrl, struct InputRecord *record)
{
    struct Axis *axis = find_bind_axis(ctrl, record->id, KEY);
    if (axis == nullptr)
        return false;

    if (record->data.axis.is_down) {
        axis->last_pressed = record->timestamp;
    } else {
        axis->last_released = record->timestamp;
    }
    axis->data.axis.is_down = record->data.axis.is_down;

    return true;
}

bool CtrlUpdateAxis(Ctrl *ctrl, struct InputRecord *record)
{
    struct Axis *axis = find_bind_axis(ctrl, record->id, AXIS);
    if (axis == nullptr)
        return false;

    axis->data.axis.value = record->data.axis.value;
    axis->last_pressed = record->timestamp;

    return true;
}

bool CtrlUpdateJoystick(Ctrl *ctrl, struct InputRecord *record)
{
    struct Axis *axis = find_bind_axis(ctrl, record->id, JOYSTICK);
    if (axis == nullptr)
        return false;

    axis->data.joystick.x = record->data.joystick.x;
    axis->data.joystick.y = record->data.joystick.y;
    axis->last_pressed = record->timestamp;

    return true;
}

bool CtrlUpdateWindow(Ctrl *ctrl, struct InputRecord *record)
{
    struct Axis *axis = find_bind_axis(ctrl, record->id, WINDOW);
    if (axis == nullptr)
        return false;

    axis->data.joystick.x = record->data.joystick.x;
    axis->data.joystick.y = record->data.joystick.y;
    axis->last_pressed = record->timestamp;

    return true;
}