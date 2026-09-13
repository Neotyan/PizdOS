MBOOT_PAGE_ALIGN   equ 1<<0 
MBOOT_MEM_INFO     equ 1<<1
MBOOT_HEADER_MAGIC equ 0x1BADB002
MBOOT_HEADER_FLAGS equ MBOOT_PAGE_ALIGN | MBOOT_MEM_INFO
MBOOT_CHECKSUM     equ -(MBOOT_HEADER_MAGIC + MBOOT_HEADER_FLAGS)

[BITS 32]

SECTION .mboot
[GLOBAL mboot]

mboot:
    dd MBOOT_HEADER_MAGIC
    dd MBOOT_HEADER_FLAGS
    dd MBOOT_CHECKSUM

SECTION .bss
align 16
stack_bottom:
    resb 16384 
stack_top:

SECTION .text
[GLOBAL _start]
[EXTERN main]

_start:
    mov esp, stack_top  
    push ebx            
    cli                
    call main
    jmp $               