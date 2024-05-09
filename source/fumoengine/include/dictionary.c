#include "dictionary.h"
#include <stdlib.h>
#include <string.h>


bool CreateDictionary(DictT T, struct Dictionary *dict)
{
    void *bkts = calloc(16, T->BKT_SIZE);

    if (bkts == nullptr)
        return false;

    *dict = (struct Dictionary) {
        .filled = 0,
        .capacity = 16,
        .bkts = bkts
    };

    return true;
}

void FreeDictionary(struct Dictionary *dict)
{
    free(dict->bkts);
}

void *index_bkt(DictT T, struct Dictionary *dict, usize i)
{
    return (u8 *)dict->bkts + i * T->BKT_SIZE;
}

u32 *get_key(DictT T, void *bkt)
{
    return (u32 *)bkt;
}

void *get_val(DictT T, void *bkt)
{
    return (u8 *)bkt + T->VAL_OFS;
}

void set_bkt(DictT T, void *bkt, u32 key, void *val)
{
    *get_key(T, bkt) = key;
    memcpy(get_val(T, bkt), val, T->VAL_SIZE);
}

void *probe_bkt(DictT T, struct Dictionary *dict, usize index, u32 key)
{
    for (usize i = 0; i < dict->capacity; i++) {
        void *bkt = index_bkt(T, dict, (index + i) % dict->capacity);

        if (*get_key(T, bkt) == key)
            return bkt;
    }

    return nullptr;
}

void *probe_empty_bkt(DictT T, struct Dictionary *dict, usize index, u32 key)
{
    for (usize i = 0; i < dict->capacity; i++) {
        void *bkt = index_bkt(T, dict, (index + i) % dict->capacity);

        u32 k = *get_key(T, bkt);
        if (k == 0 or k == key)
            return bkt;
    }

    return nullptr;
}

void *DictionaryFind(DictT T, struct Dictionary *dict, u32 key)
{
    usize index = key % dict->capacity;

    return probe_bkt(T, dict, index, key);
}

void *DictionarySet(DictT T, struct Dictionary *dict, u32 key, void *val)
{
    usize index = key % dict->capacity;

    void *bkt = probe_empty_bkt(T, dict, index, key);
    if (*get_key(T, bkt) == 0)
        set_bkt(T, bkt, key, val);

    return bkt;
}