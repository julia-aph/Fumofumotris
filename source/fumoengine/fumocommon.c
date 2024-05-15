#include "fumocommon.h"
#include <time.h>


usize min(usize a, usize b)
{
    return a < b ? a : b;
}

usize max(usize a, usize b)
{
    return a > b ? a : b;
}

nsec TimeNow()
{
    struct timespec ts;
    timespec_get(&ts, TIME_UTC);
    
    return ts.tv_nsec + ts.tv_sec * ONE_E_9;
}

double TimeNowD()
{
    struct timespec ts;
    timespec_get(&ts, TIME_UTC);

    return ts.tv_sec + (double)ts.tv_nsec / 1000000000.0;
}

u32 Hash(void *item, usize size)
{
    u8 *data = (u8 *)item;

    u32 h = 98317;
    for (usize i = 0; i < size; i++) {
        h ^= data[i];
        h *= 0x5bd1e995;
        h ^= h >> 15;
    }

    return h;
}