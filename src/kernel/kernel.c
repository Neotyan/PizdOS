#include "io.h"
#include "keyboard.h"
#include "shell.h"
#include "pit.h"

extern void idt_load(void);
extern void idt_init(void);
extern void idt_set_gate(unsigned char num, unsigned int base, unsigned short sel, unsigned char flags);

extern void gdt_init(void);

extern void pic_remap(void);
extern void pit_init(int frequency);

extern void irq0_stub(void);
extern void irq1_stub(void);

extern void clear_screen(unsigned char color);

void main(void) {
	gdt_init();
	idt_init();
	idt_set_gate(32, (unsigned int)irq0_stub, 0x08, 0x8E);
	idt_set_gate(33, (unsigned int)irq1_stub, 0x08, 0x8E);
	idt_load();
	pic_remap();
	pit_init(1000);
	clear_screen(0x0A);
	shell_print_prompt();

	__asm__ __volatile__("sti");

	while(1) {
		keyboard_procces_queue();
		__asm__ __volatile__ ("hlt");
	}
}