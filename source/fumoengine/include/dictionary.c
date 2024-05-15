#include "dictionary.h"
#include <stdalign.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


bool CreateDictionary(struct Dictionary *dict, usize value_size)
{
    void *bkts = calloc(16, value_size);

    if (bkts == nullptr)
        return false;

    dict->value_size = value_size;
    dict->value_offset = max(alignof(u32), alignof(u8[dict->value_size]));
    dict->bkt_size = dict->value_offset + value_size;

    dict->filled = 0;
    dict->capacity = 16;
    dict->bkts = bkts;

    return true;
}

void FreeDictionary(struct Dictionary *dict)
{
    free(dict->bkts);
}

void *index_bkt(struct Dictionary *dict, usize i)
{
    return (u8 *)dict->bkts + i * dict->bkt_size;
}

u32 *get_key(struct Dictionary *dict, void *bkt)
{
    return (u32 *)bkt;
}

void *get_value_ptr(struct Dictionary *dict, void *bkt)
{
    return (u8 *)bkt + dict->value_offset;
}

void set_bkt(struct Dictionary *dict, void *bkt, u32 key, void *value_ptr)
{
    *get_key(dict, bkt) = key;
    memcpy(get_value_ptr(dict, bkt), value_ptr, dict->value_size);
}

void *probe_bkt(struct Dictionary *dict, usize index, u32 key)
{
    for (usize i = 0; i < dict->capacity; i++) {
        void *bkt = index_bkt(dict, (index + i) % dict->capacity);

        if (*get_key(dict, bkt) == key)
            return bkt;
    }

    return nullptr;
}

void *probe_empty_bkt(struct Dictionary *dict, usize index, u32 key)
{
    for (usize i = 0; i < dict->capacity; i++) {
        void *bkt = index_bkt(dict, (index + i) % dict->capacity);

        u32 k = *get_key(dict, bkt);
        if (k == 0 or k == key)
            return bkt;
    }

    return nullptr;
}

void *DictionaryFind(struct Dictionary *dict, u32 key)
{
    usize index = key % dict->capacity;

    void *bkt = probe_bkt(dict, index, key);
    if (bkt == nullptr)
        return false;

    return get_value_ptr(dict, bkt);
}

void *DictionarySet(struct Dictionary *dict, u32 key, void *value_ptr)
{
    usize index = key % dict->capacity;

    void *bkt = probe_empty_bkt(dict, index, key);

    if (*get_key(dict, bkt) == 0) 
        set_bkt(dict, bkt, key, value_ptr);

    return bkt;
}