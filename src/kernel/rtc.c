#include "io.h"
#include "rtc.h"
#include "libc.h"
#include "shell.h"

extern volatile rtc_time_t os_time;


unsigned char read_rtc(unsigned char reg) {
	outb(0x70, reg | 0x80);
	return inb(0x71);
}

int is_updateing(void) {
	outb(0x70, 0x0A | 0x80);
	return (inb(0x71) & 0x80);
}

static inline unsigned char bcd_decoder(unsigned char bcd) {
	return ((bcd & 0xF0) >> 4) * 10 + (bcd & 0x0F);
}

rtc_time_t get_time(void) {
	rtc_time_t time, last_time;

	do {
		while(is_updateing());

		time.second = read_rtc(0x00);  
		time.minute = read_rtc(0x02); 
		time.hour = read_rtc(0x04);
		time.day = read_rtc(0x07);
		time.month = read_rtc(0x08);
		time.year = read_rtc(0x09);

		while(is_updateing());
		
		last_time.second = read_rtc(0x00); 
		last_time.minute = read_rtc(0x02);
		last_time.hour = read_rtc(0x04);
		last_time.day = read_rtc(0x07);
		last_time.month = read_rtc(0x08);
		last_time.year = read_rtc(0x09);	

	} while(time.second != last_time.second || 
            time.minute != last_time.minute || 
            time.hour   != last_time.hour   || 
            time.day    != last_time.day    || 
            time.month  != last_time.month  || 
            time.year   != last_time.year);

	time.second = bcd_decoder(time.second);   
	time.minute = bcd_decoder(time.minute);
	time.hour = bcd_decoder(time.hour);
	time.day = bcd_decoder(time.day);
	time.month = bcd_decoder(time.month);
	time.year = bcd_decoder(time.year);

	return time;
}

void see_time(void) {
    char time_buffer[64];
    int offset = 0;

    offset += sprint(os_time.hour, time_buffer + offset);
    time_buffer[offset++] = ':';
    
    offset += sprint(os_time.minute, time_buffer + offset);
    time_buffer[offset++] = ':';
    
    offset += sprint(os_time.second, time_buffer + offset);
    time_buffer[offset] = '\0';

    shell_write(time_buffer);
}
