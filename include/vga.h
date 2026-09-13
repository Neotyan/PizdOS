#ifndef VGA_H
#define VGA_H

void vga_putchar(char c);
void clear_screen(unsigned char color);

int vga_get_cursor_x(void);
int vga_get_cursor_y(void);
void vga_set_cursor(int x, int y);

#endif