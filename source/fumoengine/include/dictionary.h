#pragma once
#include "fumocommon.h"

#define DICT_T(DICT_VAL_T) \
    (&(struct DictT) { \
        .VAL_SIZE = sizeof(DICT_VAL_T), \
        .VAL_OFS = offsetof(struct { u32 k; DICT_VAL_T v; }, v), \
        .BKT_SIZE = sizeof(struct { u32 k; DICT_VAL_T v; }) \
    }) \


typedef const struct DictT {
    usize VAL_SIZE;
    usize VAL_OFS;
    usize BKT_SIZE;
} *const DictT;

struct Dictionary {
    usize filled;
    usize capacity;
    void *bkts;
};


bool CreateDictionary(DictT T, struct Dictionary *dict);

void FreeDictionary(struct Dictionary *dict);

void *DictionaryFind(DictT T, struct Dictionary *dict, u32 key);

void *DictionarySet(DictT T, struct Dictionary *dict, u32 key, void *val);