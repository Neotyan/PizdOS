#include "shell.h"
#include "vga.h"
#include "keyboard.h"

extern void sprint(unsigned int num, char *buffer);
extern volatile unsigned int timer_ticks;

static int str_equal(const char *a, const char *b) {
    while (*a != '\0' && *b != '\0') {
        if (*a != *b) {
            return 0;
        }

        a++;
        b++;
    }

    return *a == '\0' && *b == '\0';
}

static int is_space(char c) {
    return c == ' ' || c == '\t';
}

static void shell_write(const char *text) {
    while (*text != '\0') {
        vga_putchar(*text);
        text++;
    }
}

void shell_print_prompt(void) {
    shell_write("kernel:~$ ");
    terminal_set_line_start();
}

void shell_execute(char *line) {
    char *command;
    char *argument;

    while (is_space(*line)) {
        line++;
    }

    if (*line == '\0') {
        shell_print_prompt();
        return;
    }

    command = line;

    while (*line != '\0' && !is_space(*line)) {
        line++;
    }

    if (*line == '\0') {
        argument = 0;
    } else {
        *line = '\0';
        line++;

        while (is_space(*line)) {
            line++;
        }

        if (*line == '\0') {
            argument = 0;
        } else {
            argument = line;
        }
    }

    if (str_equal(command, "help")) {
        shell_write("Commands:\n");
        shell_write("    help\n");
        shell_write("    echo\n");
        shell_write("    clear\n");
        shell_write("    ticks\n");
    } else if (str_equal(command, "echo")) {
        if (argument != 0) {
            shell_write(argument);
            vga_putchar('\n');
        }
    } else if(str_equal(command, "clear")) {
    	clear_screen(0x0A);
    } else if (str_equal(command, "ticks")) {
        char buffer[32];
        sprint(timer_ticks, buffer);
        shell_write("ticks: ");
        shell_write(buffer);
        shell_write("\n"); 
    }

    else {
        shell_write("unknown command\n");
    } 

    shell_print_prompt();
}