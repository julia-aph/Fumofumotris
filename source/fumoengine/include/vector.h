#include "fumocommon.h"


struct Vector {
    usize value_size;

    usize len;
    usize capacity;
    void *array;
};


bool CreateVector(struct Vector *vec, usize value_size);

void FreeVector(struct Vector *vec);

void *VectorGet(struct Vector *vec, usize i);

bool VectorAdd(struct Vector *vec, void *item);