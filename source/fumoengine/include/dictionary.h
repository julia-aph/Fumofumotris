#pragma once
#include "fumocommon.h"


struct Dictionary {
    usize value_size;
    usize value_offset;
    usize bkt_size;

    usize filled;
    usize capacity;
    void *bkts;
};

bool CreateDictionary(struct Dictionary *dict, usize value_size);

void FreeDictionary(struct Dictionary *dict);

void *DictionaryFind(struct Dictionary *dict, u32 key);

void *DictionarySet(struct Dictionary *dict, u32 key, void *value_ptr);