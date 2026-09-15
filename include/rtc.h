#ifndef RTC_H
#define RTC_H

typedef struct Time {
    unsigned char second;
    unsigned char minute;
    unsigned char hour;
    unsigned char day;
    unsigned char month;
    unsigned char year;
} rtc_time_t;

rtc_time_t get_time(void);
void format_time(rtc_time_t time, char *buffer);
void see_time();

#endif