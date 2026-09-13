[BITS 32]

SECTION .text
[GLOBAL idt_load]
[EXTERN idtp]

idt_load:
	lidt [idtp]
	ret