#include <time.h>
#include <stdbool.h>

#ifdef _WIN32
#include "win.h"
#endif

struct timespec TimeNow()
{
    struct timespec ts;
    timespec_get(&ts, TIME_UTC);
    return ts;
}

double TimeNowDouble()
{
    struct timespec ts;
    timespec_get(&ts, TIME_UTC);

    return ts.tv_sec + (double)ts.tv_nsec / 1000000000.0;
}