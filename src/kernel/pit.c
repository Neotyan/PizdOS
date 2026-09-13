#include "io.h"
#include "shell.h"

volatile unsigned int timer_ticks = 0;
volatile unsigned long needed_time = 0;

void pit_init(int frequency) {
	int divisor = 1193182 / frequency;

	if (divisor > 65535) divisor = 0;
	if (divisor < 1) divisor = 1;

	outb(0x43, 0x36);

	outb(0x40, (divisor & 0xFF));
	outb(0x40, (divisor >> 8) & 0xFF);

}
void timer_handler(void) {
	timer_ticks++;
	outb(0x20, 0x20);
}

void sleep(unsigned long time) {
	needed_time = timer_ticks + time;
	while (needed_time > timer_ticks) {
		__asm__ __volatile__ ("hlt");
	}
}