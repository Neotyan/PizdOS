#ifndef SHELL_H
#define SHELL_H

void shell_print_prompt(void);
void shell_execute(char *line);
void shell_write(const char *text);
#endif