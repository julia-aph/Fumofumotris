#include "fumocommon.h"

#define VECTOR_T(VEC_ITEM_T) \
    (&(struct VectorT) { \
        .SIZE = sizeof(VEC_ITEM_T) \
    }) \


typedef const struct VectorT {
    usize SIZE;
} *const VectorT;

struct Vector {
    usize len;
    usize capacity;
    void *array;
};


bool CreateVector(VectorT T, struct Vector *vec);

void FreeVector(struct Vector *vec);

void *VectorGet(VectorT T, struct Vector *vec, usize i);

bool VectorAdd(VectorT T, struct Vector *vec, void *item);