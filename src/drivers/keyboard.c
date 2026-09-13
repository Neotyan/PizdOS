#include "vga.h"
#include "io.h"
#include "shell.h"
#include "keyboard.h"

static const char kbd_layout[128] = {
    0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0',
    '-', '=', '\b', '\t',
    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p',
    '[', ']', '\n', 0,
    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l',
    ';', '\'', '`', 0,
    '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm',
    ',', '.', '/', 0, '*', 0, ' '
};

static const char kbd_shift_layout[128] = {
    0, 27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')',
    '_', '+', '\b', '\t',
    'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P',
    '{', '}', '\n', 0,
    'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L',
    ':', '"', '~', 0,
    '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M',
    '<', '>', '?', 0, '*', 0, ' '
};

static volatile key_event_t keyboard_queue[128];
static volatile unsigned int write_q = 0;
static volatile unsigned int read_q = 0;

static int extended_scancode = 0;
static int left_shift_pressed = 0;
static int right_shift_pressed = 0;
static int capslock_active = 0;

static char input_line[128];
static unsigned int input_length = 0;
static unsigned int input_cursor = 0;

static char previous_command[128];
static int has_previous_command = 0;

static int line_start_x = 0;
static int line_start_y = 0;

static void keyboard_queue_push(unsigned char scancode, unsigned int pressed, unsigned int extended) {
    unsigned int next;

    next = (write_q + 1) % 128;

    if (next == read_q) {
        return;
    }

    keyboard_queue[write_q].scancode = scancode;
    keyboard_queue[write_q].pressed = pressed;
    keyboard_queue[write_q].extended = extended;
    write_q = next;
}

static int keyboard_queue_pop(key_event_t *event) {
    if (read_q == write_q) {
        return 0;
    }

    *event = keyboard_queue[read_q];
    read_q = (read_q + 1) % 128;
    return 1;
}

void terminal_set_line_start(void) {
    line_start_x = vga_get_cursor_x();
    line_start_y = vga_get_cursor_y();
}

static void redraw_input_line(void) {
    unsigned int i;

    vga_set_cursor(line_start_x, line_start_y);

    for (i = 0; i < input_length; i++) {
        vga_putchar(input_line[i]);
    }

    vga_putchar(' ');
    vga_set_cursor(line_start_x + input_cursor, line_start_y);
}

static void clear_input_line(void) {
    unsigned int i;

    vga_set_cursor(line_start_x, line_start_y);

    for (i = 0; i < input_length; i++) {
        vga_putchar(' ');
    }

    input_length = 0;
    input_cursor = 0;
    input_line[0] = '\0';
    vga_set_cursor(line_start_x, line_start_y);
}

static void save_previous_command(void) {
    unsigned int i;

    for (i = 0; i < input_length; i++) {
        previous_command[i] = input_line[i];
    }

    previous_command[input_length] = '\0';
    has_previous_command = 1;
}

static void load_previous_command(void) {
    unsigned int i;

    if (!has_previous_command) {
        return;
    }

    clear_input_line();

    for (i = 0; previous_command[i] != '\0'; i++) {
        input_line[i] = previous_command[i];
    }

    input_length = i;
    input_cursor = i;
    input_line[input_length] = '\0';
    redraw_input_line();
}

static void insert_char(char ch) {
    unsigned int i;

    if (input_length >= 128 - 1) {
        return;
    }

    for (i = input_length; i > input_cursor; i--) {
        input_line[i] = input_line[i - 1];
    }

    input_line[input_cursor] = ch;
    input_length++;
    input_cursor++;
    input_line[input_length] = '\0';
    redraw_input_line();
}

static void delete_before_cursor(void) {
    unsigned int i;

    if (input_cursor == 0) {
        return;
    }

    for (i = input_cursor - 1; i < input_length - 1; i++) {
        input_line[i] = input_line[i + 1];
    }

    input_length--;
    input_cursor--;
    input_line[input_length] = '\0';
    redraw_input_line();
}

static int is_letter(char ch) {
    return (ch >= 'a' && ch <= 'z') ||
           (ch >= 'A' && ch <= 'Z');
}

static char key_to_char(unsigned char scancode) {
    char normal;
    char shifted;
    int shift_active;
    int uppercase;

    normal = kbd_layout[scancode];
    shifted = kbd_shift_layout[scancode];
    shift_active = left_shift_pressed || right_shift_pressed;

    if (is_letter(normal)) {
        uppercase = shift_active ^ capslock_active;
        return uppercase ? shifted : normal;
    }

    return shift_active ? shifted : normal;
}

static void terminal_input(char ch) {
    if (ch == '\b') {
        delete_before_cursor();
        return;
    }

    if (ch == '\n') {
        save_previous_command();
        vga_set_cursor(line_start_x + input_length, line_start_y);
        vga_putchar('\n');
        shell_execute(input_line);
        input_length = 0;
        input_cursor = 0;
        input_line[0] = '\0';
        return;
    }

    if (ch == '\t') {
        unsigned int column;
        unsigned int spaces;

        column = line_start_x + input_cursor;
        spaces = 8 - (column % 8);

        while (spaces > 0) {
            insert_char(' ');
            spaces--;
        }
        return;
    }

    if (ch >= 32 && ch <= 126) {
        insert_char(ch);
    }
}

static void process_key_event(key_event_t event) {
    char ch;

    if (event.extended) {
        if (!event.pressed) {
            return;
        }

        if (event.scancode == 0x4B) {
            if (input_cursor > 0) {
                input_cursor--;
                vga_set_cursor(line_start_x + input_cursor, line_start_y);
            }
            return;
        }

        if (event.scancode == 0x4D) {
            if (input_cursor < input_length) {
                input_cursor++;
                vga_set_cursor(line_start_x + input_cursor, line_start_y);
            }
            return;
        }

        if (event.scancode == 0x48) {
            load_previous_command();
            return;
        }

        return;
    }

    if (event.scancode == 0x2A) {
        left_shift_pressed = event.pressed;
        return;
    }

    if (event.scancode == 0x36) {
        right_shift_pressed = event.pressed;
        return;
    }

    if (event.scancode == 0x3A) {
        if (event.pressed) {
            capslock_active = !capslock_active;
        }
        return;
    }

    if (!event.pressed || event.scancode >= 128) {
        return;
    }

    ch = key_to_char(event.scancode);

    if (ch != 0) {
        terminal_input(ch);
    }
}

void keyboard_procces_queue(void) {
    key_event_t event;

    while (keyboard_queue_pop(&event)) {
        process_key_event(event);
    }
}

void keyboard_handler(void) {
    unsigned char raw_scancode;
    unsigned char scancode;
    unsigned int pressed;
    unsigned int extended;

    raw_scancode = inb(0x60);

    if (raw_scancode == 0xE0) {
        extended_scancode = 1;
        outb(0x20, 0x20);
        return;
    }

    scancode = raw_scancode & 0x7F;
    pressed = !(raw_scancode & 0x80);
    extended = extended_scancode;
    extended_scancode = 0;

    keyboard_queue_push(scancode, pressed, extended);
    outb(0x20, 0x20);
}