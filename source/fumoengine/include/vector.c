#include "vector.h"
#include <stdlib.h>
#include <string.h>


bool CreateVector(VectorT T, struct Vector *vec)
{
    void *array = malloc(16 * T->SIZE);

    if (array == nullptr)
        return false;

    vec->len = 0;
    vec->capacity = 16;
    vec->array = array;

    return true;
}

void FreeVector(struct Vector *vec)
{
    free(vec->array);
}

void *VectorGet(VectorT T, struct Vector *vec, usize i)
{
    return (u8 *)vec->array + i * T->SIZE;
}

bool VectorAdd(VectorT T, struct Vector *vec, void *item)
{
    if (vec->len == vec->capacity) {
        usize new_capacity = vec->capacity * 2;
        void *new_array = realloc(vec->array, new_capacity * T->SIZE);

        if (new_array == nullptr)
            return false;

        vec->capacity = new_capacity;
    }

    memcpy(VectorGet(T, vec, vec->len++), item, T->SIZE);

    return true;
}