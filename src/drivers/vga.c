#include "io.h"
#include "vga.h"

static unsigned short* const VGA_BUFFER = (unsigned short*)0xB8000;
static int cursor_x = 0;
static int cursor_y = 0;
int vga_get_cursor_x(void);
int vga_get_cursor_y(void);

void update_cursor(int x, int y) {
	unsigned short pos = y * 80 + x;
	
	outb(0x3D4, 0x0F);
	outb(0x3D5, (unsigned char)(pos & 0xFF));

	outb(0x3D4, 0x0E);
	outb(0x3D5, (unsigned char)(pos >> 8) & 0xFF);
}

void vga_set_cursor(int x, int y) {
	cursor_x = x;
	cursor_y = y;
	update_cursor(cursor_x, cursor_y);
}

int vga_get_cursor_x(void) {
    return cursor_x;
}

int vga_get_cursor_y(void) {
    return cursor_y;
}

void clear_screen(unsigned char color) {
	volatile unsigned short *vga = (volatile unsigned short*)0xB8000;
	const unsigned short blank = (color << 8) | ' ';

	for (int i = 0; i < 80 * 25; i++) {
		vga[i] = blank;
	}
	cursor_x = 0;
	cursor_y = 0;
	update_cursor(cursor_x, cursor_y);
}

void vga_putchar(char c) {
	if (c == '\n') {
		cursor_x = 0;
		cursor_y++;
	} else if (c == '\b') {
		if (cursor_x > 0) {
			cursor_x--;
		} else if (cursor_y > 0) {
			cursor_y--;
			cursor_x = 79;
		}

		int index = cursor_y * 80 + cursor_x;
		VGA_BUFFER[index] = (0x0A << 8) | ' ';

	} else if (c == '\t') {
		cursor_x = (cursor_x + 8) & ~7;

		if (cursor_x >= 80) {
			cursor_x = 0;
			cursor_y++;
		}
		update_cursor(cursor_x, cursor_y);
	}

	else {
		int index = cursor_y * 80 + cursor_x;
		VGA_BUFFER[index] = (0x0A << 8) | c;

		cursor_x++;

		if (cursor_x >= 80) {
			cursor_x = 0;
			cursor_y++;
		}
	}

	if (cursor_y >= 25) {
		for (int i = 0; i < 24 * 80; i++) {
			VGA_BUFFER[i] = VGA_BUFFER[i + 80];
		}
		for (int i = 24 * 80; i < 25 * 80; i++) {
			VGA_BUFFER[i] = (0x0A << 8) | ' ';
		}
		cursor_y = 24;
	}
	update_cursor(cursor_x, cursor_y);
}