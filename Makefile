CC = gcc
AS = nasm
LD = ld

SRC_DIR = src
BUILD_DIR = build
ISO_DIR = iso

CFLAGS = -m32 -ffreestanding -fno-stack-protector -fno-builtin \
         -nostdlib -nostdinc -Wall -Wextra -MMD -MP \
         -Iinclude

ASFLAGS = -f elf32
LDFLAGS = -m elf_i386 -T linker.ld

C_SOURCES = \
	src/kernel/kernel.c \
	src/kernel/gdt.c \
	src/kernel/idt.c \
	src/kernel/pic.c \
	src/kernel/pit.c \
	src/kernel/rtc.c \
	src/drivers/keyboard.c \
	src/drivers/vga.c \
	src/shell/shell.c \
	src/libc/sprint.c
	
ASM_SOURCES = \
	src/boot/boot.S \
	src/boot/gdt.S \
	src/boot/idt.S \
	src/boot/irq.S

C_OBJECTS = $(patsubst src/%.c,$(BUILD_DIR)/%.o,$(C_SOURCES))
ASM_OBJECTS = $(patsubst src/%.S,$(BUILD_DIR)/%.o,$(ASM_SOURCES))

OBJECTS = $(ASM_OBJECTS) $(C_OBJECTS)
DEP_FILES = $(C_OBJECTS:.o=.d)

KERNEL = $(BUILD_DIR)/kernel.bin
ISO_IMAGE = pizdos.iso
ISO_KERNEL = $(ISO_DIR)/boot/kernel.bin
ISO_CONFIG = $(ISO_DIR)/boot/grub/grub.cfg

all: $(KERNEL)

$(BUILD_DIR)/%.o: src/%.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: src/%.S
	mkdir -p $(dir $@)
	$(AS) $(ASFLAGS) $< -o $@

$(KERNEL): $(OBJECTS)
	mkdir -p $(BUILD_DIR)
	$(LD) $(LDFLAGS) $(OBJECTS) -o $@

$(ISO_CONFIG): grub.cfg
	mkdir -p $(ISO_DIR)/boot/grub
	cp grub.cfg $(ISO_CONFIG)

$(ISO_KERNEL): $(KERNEL)
	mkdir -p $(ISO_DIR)/boot
	cp $(KERNEL) $(ISO_KERNEL)

$(ISO_IMAGE): $(ISO_KERNEL) $(ISO_CONFIG)
	grub-mkrescue -o $@ $(ISO_DIR)

run: $(KERNEL)
	qemu-system-i386 -kernel $(KERNEL)

iso: $(ISO_IMAGE)

run-iso: $(ISO_IMAGE)
	qemu-system-i386 -cdrom $(ISO_IMAGE)

clean:
	rm -rf $(BUILD_DIR)
	rm -rf $(ISO_DIR)
	rm -f $(ISO_IMAGE)

.PHONY: all run iso run-iso clean

-include $(DEP_FILES)