#pragma once
#include <stdbool.h>

struct Time {
    u32 sec;
    u32 nsec;
};

struct Time TimeNow();

double TimeNowDouble();
