#include "io.h"
#include "shell.h"
#include "rtc.h"

volatile rtc_time_t os_time;
volatile unsigned int ms_counter = 0; 
volatile unsigned int timer_ticks = 0;
volatile unsigned long needed_time = 0;
static const unsigned char days_in_months[13] = {
    0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};

void pit_init(int frequency) {
	int divisor = 1193182 / frequency;

	if (divisor > 65535) divisor = 0;
	if (divisor < 1) divisor = 1;

	outb(0x43, 0x36);

	outb(0x40, (divisor & 0xFF));
	outb(0x40, (divisor >> 8) & 0xFF);

}

int is_leap_year(unsigned int year) {
    unsigned int full_year = 2000 + year;

    if ((full_year % 4 == 0 && full_year % 100 != 0) || (full_year % 400 == 0)) {
        return 1;
    }
    return 0;
}

void timer_handler(void) {
    timer_ticks++;
    ms_counter++;

    if (ms_counter >= 1000) {
        ms_counter = 0;
        os_time.second++;
        
        if (os_time.second >= 60) {
            os_time.second = 0;
            os_time.minute++;
            
            if (os_time.minute >= 60) {
                os_time.minute = 0;
                os_time.hour++;
                
                if (os_time.hour >= 24) {
                    os_time.hour = 0;
                    os_time.day++;

                    unsigned char max_days = days_in_months[os_time.month];

                    if (os_time.month == 2 && is_leap_year(os_time.year)) {
                        max_days = 29;
                    }

                    if (os_time.day > max_days) {
                        os_time.day = 1;
                        os_time.month++;

                        if (os_time.month > 12) {
                            os_time.month = 1;
                            os_time.year++;
                            
                            if (os_time.year >= 100) {
                                os_time.year = 0;
                            }
                        }
                    }
                } 
            } 
        } 
    } 

    outb(0x20, 0x20);
}

void sleep(unsigned long time) {
	needed_time = timer_ticks + time;
	while (needed_time > timer_ticks) {
		__asm__ __volatile__ ("hlt");
	}
}