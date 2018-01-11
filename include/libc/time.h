#ifndef __LIBC_TIME_H__
#define __LIBC_TIME_H__

#include <types.h>

typedef struct timeval {
    uint32_t tv_sec;
    uint32_t tv_usec;
} timeval_t;

struct itimerval {
    timeval_t it_interval;  // Timer interval
    timeval_t it_value;     // Current value
};

struct tm {
    int tm_sec;     // Seconds after the minute [0, 59]
    int tm_min;     // Minutes after the hour [0, 59]
    int tm_hour;    // Hours since midnight [0, 23]
    int tm_mday;    // Day of the month [1, 31]
    int tm_mon;     // Months since January [0, 11]
    int tm_year;    // Years since 1900
    int tm_wday;    // Days since Sunday [0, 6]
    int tm_yday;    // Days since January 1 [0, 365]
    int tm_isdst;   // Daylight Saving Time flag
    int tm_gmtoff;  // Seconds east of UTC
    char *tm_zone;  // Timezone abbreviation
};

struct tm *gmtime_r(const time_t *timer, struct tm *tmbuf);
struct tm *localtime_r(const time_t *timer, struct tm *tmbuf);
#endif
