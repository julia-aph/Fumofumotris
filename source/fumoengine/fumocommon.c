#include "fumocommon.h"
#include <time.h>


size_t MinSize(size_t a, size_t b)
{
    return a < b ? a : b;
}

Time TimeNow()
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