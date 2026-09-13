[BITS 32]

SECTION .text

[GLOBAL irq0_stub]
[EXTERN timer_handler]

[GLOBAL irq1_stub]
[EXTERN keyboard_handler]

irq0_stub:
	pushad
	call timer_handler
	popad
	iretd

irq1_stub:
	pushad
	call keyboard_handler
	popad
	iretd