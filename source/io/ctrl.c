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

    double timestamp;
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
    u8 type;

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

struct code_bkt {
    hashtype hash;
    u16 code;
    struct Axis axis;
};

struct bind_bkt {
    hashtype hash;
    u16 bind;
    struct Axis *axis;
};

struct Ctrl {
    struct {
        size_t capacity;
        size_t filled;
        struct code_bkt *bkts;
    } codes;

    struct {
        size_t capacity;
        size_t filled;
        struct bind_bkt *bkts;
    } binds;

    pthread_mutex_t mutex;
};
typedef struct Ctrl Ctrl;

Ctrl NewCtrl(struct code_bkt *codes, size_t c, struct bind_bkt *binds, size_t b)
{
    memset(codes, 0, sizeof(struct code_bkt) * c);
    memset(binds, 0, sizeof(struct bind_bkt) * b);

    Ctrl ctrl;
    ctrl.codes.bkts = codes;
    ctrl.binds.bkts = binds;
    ctrl.mutex = PTHREAD_MUTEX_INITIALIZER;

    return ctrl;
}

void CtrlPoll(struct InputBuffer *buf)
{
    for (size_t i = 0; i < buf->count; i++) {
        struct InputRecord *rec = &buf->records[i];

        switch (rec->type) {
        case KEY:
            key_update();
            break;
        case AXIS:
            axis_update();
            break;
        case JOYSTICK:
            joystick_update();
            break;
        }
    }
}

struct ctrl_bkt *get_code_bkt(Ctrl *ctrl, size_t i)
{
    assert(i < ctrl->codes.capacity);
    return &ctrl->codes.bkts[i];
}

struct ctrl_bkt *get_bind_bkt(Ctrl *ctrl, size_t i)
{
    assert(i < ctrl->binds.capacity);
    return &ctrl->binds.bkts[i];
}

struct code_bkt *find_code(Ctrl *ctrl, hashtype hash)
{
    const size_t index = hash % ctrl->codes.capacity;
    size_t i = index;

    while (i != index - 1) {
        struct code_bkt *bkt = get_code_bkt(ctrl, i);
        if (bkt->hash == 0)
            return bkt;
        
        i = (i + 1) % ctrl->codes.capacity;
    }
    return nullptr;
}

struct ctrl_bkt *find_bind(Ctrl *ctrl, hashtype hash)
{
    const size_t index = hash % ctrl->binds.capacity;
    size_t i = index;

    while (i != index - 1) {
        struct bind_bkt *bkt = get_bind_bkt(ctrl, i);
        if (bkt->hash == 0)
            return bkt;
        
        i = (i + 1) % ctrl->binds.capacity;
    }
    return nullptr;
}

bool CtrlMap(Ctrl *ctrl, u16f bind, u16f code, enum InputType type)
{
    if (ctrl->binds.filled == ctrl->binds.capacity)
        return false;

    hashtype code_hash = hash_ident(code, type);
    struct code_bkt *code_bkt = find_code(ctrl, code_hash);
    code_bkt->hash = code_hash;
    code_bkt->code = code;

    hashtype bind_hash = hash_ident(bind, type);
    struct bind_bkt *bind_bkt = find_bind(ctrl, bind_hash);
    bind_bkt->hash = bind_hash;
    bind_bkt->bind = bind;
    bind_bkt->axis = &code_bkt->axis;

    ctrl->binds.filled += 1;

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

struct Axis *CtrlGet(Ctrl *ctrl, u16f code, u8f type)
{
=    struct ctrl_bkt *code_bkt = find_code(ctrl, code, type);
    if (code_bkt == nullptr)
        return nullptr;

    return &code_bkt->axis;
}

bool CtrlUpdateKey(Ctrl *ctrl, struct InputRecord *record)
{
    struct Axis *axis = find_bind_axis(ctrl, record->id, KEY);
    if (axis == nullptr)
        return false;

    if (record->data.key.is_down) {
        axis->last_pressed = record->timestamp;
    } else {
        axis->last_released = record->timestamp;
    }
    axis->data.axis.value = record->data.key.is_down;

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