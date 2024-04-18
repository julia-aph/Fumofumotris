#include <time.h>
#include <stdbool.h>

#ifdef _WIN32
#include "win.h"
#endif

struct Time {
    u32 sec;
    u32 nsec;
};

struct Time TimeNow()
{
    struct timespec ts;
    // Need to check for failiure
    timespec_get(&ts, TIME_UTC);
    return (struct Time) { ts.tv_sec, ts.tv_nsec };
}

double TimeNowDouble()
{
    struct timespec ts;
    timespec_get(&ts, TIME_UTC);

    return ts.tv_sec + (double)ts.tv_nsec / 1000000000.0;
}