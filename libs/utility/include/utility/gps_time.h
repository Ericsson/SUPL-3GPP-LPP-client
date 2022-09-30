#pragma once
#include <sys/time.h>

#include "utility/types.h"

struct GPS_Time {
    time_t time; /* time (s) expressed by standard time_t */
    double sec;  /* fraction of second under 1 s */
};