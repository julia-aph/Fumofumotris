#include "gametime.h"
#include <time.h>

#define ONE_E_9 1000000000

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