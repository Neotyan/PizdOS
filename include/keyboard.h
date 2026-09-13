#ifndef KEYBOARD_H
#define KEYBOARD_H

typedef struct {
    unsigned char scancode;
    unsigned int pressed;
    unsigned int extended;
} key_event_t;

void keyboard_handler(void);
void keyboard_procces_queue(void);
void terminal_set_line_start(void);

#endif