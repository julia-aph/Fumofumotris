#include "vector.h"
#include <stdlib.h>
#include <string.h>


bool CreateVector(struct Vector *vec, usize value_size)
{
    void *array = malloc(16 * value_size);

    if (array == nullptr)
        return false;

    vec->value_size = value_size;

    vec->len = 0;
    vec->capacity = 16;
    vec->array = array;

    return true;
}

void FreeVector(struct Vector *vec)
{
    free(vec->array);
}

void *VectorGet(struct Vector *vec, usize i)
{
    return (u8 *)vec->array + i * vec->value_size;
}

bool VectorAdd(struct Vector *vec, void *item)
{
    if (vec->len == vec->capacity) {
        usize new_capacity = vec->capacity * 2;
        void *new_array = realloc(vec->array, new_capacity * vec->value_size);

        if (new_array == nullptr)
            return false;

        vec->capacity = new_capacity;
    }

    memcpy(VectorGet(vec, vec->len++), item, vec->value_size);

    return true;
}