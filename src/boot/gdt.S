[BITS 32]
 
SECTION .text
[GLOBAL gdt_flush]
[EXTERN gdtp]

gdt_flush:
    lgdt [gdtp]
    mov ax, 0x10
    mov ds, ax
    mov gs, ax
    mov ss, ax
    mov es, ax
    mov fs, ax

    jmp 0x08:.reload

.reload:
    ret