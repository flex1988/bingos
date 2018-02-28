#include <time.h>
#include <types.h>

#include "kernel/timer.h"

#define from_bcd(val) ((val / 16) * 10 + (val & 0xf))

#define CMOS_ADDRESS 0x70
#define CMOS_DATA 0x71

enum { CMOS_SECOND = 0, CMOS_MINUTE = 2, CMOS_HOUR = 4, CMOS_DAY = 7, CMOS_MONTH = 8, CMOS_YEAR = 9 };

uint32_t boot_time = 0;

int is_update_in_progress(void) {
    outb(CMOS_ADDRESS, 0x0a);
    return inb(CMOS_DATA) & 0x80;
}

void cmos_dump(uint16_t* values) {
    uint16_t index;
    for (index = 0; index < 128; index++) {
        outb(CMOS_ADDRESS, index);
        values[index] = inb(CMOS_DATA);
    }
}

void get_date(uint16_t* month, uint16_t* day) {
    uint16_t values[128];
    cmos_dump(values);

    *month = from_bcd(values[CMOS_MONTH]);
    *day = from_bcd(values[CMOS_DAY]);
}

uint32_t secs_of_years(int years) {
    uint32_t days = 0;
    years += 2000;

    while (years > 1969) {
        days += 365;
        if (years % 4 == 0) {
            if (years % 100 == 0) {
                if (years % 400 == 0) {
                    days++;
                }
            } else {
                days++;
            }
        }
        years--;
    }
    return days * 86400;
}

uint32_t secs_of_month(int months, int year) {
    year += 2000;

    uint32_t days = 0;
    switch (months) {
        case 11:
            days += 30;
        case 10:
            days += 31;
        case 9:
            days += 30;
        case 8:
            days += 31;
        case 7:
            days += 31;
        case 6:
            days += 30;
        case 5:
            days += 31;
        case 4:
            days += 30;
        case 3:
            days += 31;
        case 2:
            days += 28;
            if ((year % 4 == 0) && ((year % 100 != 0) || (year % 400 == 0))) {
                days++;
            }
        case 1:
            days += 31;
        default:
            break;
    }

    return days * 86400;
}

void get_time(uint16_t* hours, uint16_t* minutes, uint16_t* seconds) {
    uint16_t values[128];
    cmos_dump(values);

    *hours = from_bcd(values[CMOS_HOUR]);
    *minutes = from_bcd(values[CMOS_MINUTE]);
    *seconds = from_bcd(values[CMOS_SECOND]);
}

uint32_t read_cmos(void) {
    uint16_t values[128];
    uint16_t old_values[128];

    while (is_update_in_progress())
        ;

    do {
        memcpy(old_values, values, 128);
        while (is_update_in_progress())
            ;

        cmos_dump(values);
    } while ((old_values[CMOS_SECOND] != values[CMOS_SECOND]) || (old_values[CMOS_MINUTE] != values[CMOS_MINUTE]) ||
             (old_values[CMOS_HOUR] != values[CMOS_HOUR]) || (old_values[CMOS_DAY] != values[CMOS_DAY]) ||
             (old_values[CMOS_MONTH] != values[CMOS_MONTH]) || (old_values[CMOS_YEAR] != values[CMOS_YEAR]));

    uint32_t time = secs_of_years(from_bcd(values[CMOS_YEAR]) - 1) +
                    secs_of_month(from_bcd(values[CMOS_MONTH]) - 1, from_bcd(values[CMOS_YEAR])) +
                    (from_bcd(values[CMOS_DAY]) - 1) * 86400 + (from_bcd(values[CMOS_HOUR])) * 3600 +
                    (from_bcd(values[CMOS_MINUTE])) * 60 + from_bcd(values[CMOS_SECOND]) + 0;

    return time;
}

int sys_gettimeofday(timeval_t* t, void* z) {
    t->tv_sec = boot_time + timer_ticks + timer_drift;
    t->tv_usec = timer_subticks * 1000;
    return 0;
}

uint32_t now(void) { return boot_time + timer_ticks + timer_drift; }
