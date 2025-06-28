# Kronos OS Makefile
# 64-bit Operating System Build System

# Cross-compiler configuration (fallback to regular tools if not available)
CC = gcc
AS = as
LD = ld
OBJCOPY = objcopy

# Directories
SRCDIR = src
BUILDDIR = build
ISODIR = iso
BOOTDIR = $(ISODIR)/boot
GRUBDIR = $(BOOTDIR)/grub

# Compiler flags
CFLAGS = -std=gnu99 -ffreestanding -O2 -Wall -Wextra -m64 -mno-red-zone -mno-mmx -mno-sse -mno-sse2
CPPFLAGS = -I$(SRCDIR)/include
LDFLAGS = -nostdlib
ASFLAGS = --64

# Source files
ASM_SOURCES = $(wildcard $(SRCDIR)/boot/*.asm) $(wildcard $(SRCDIR)/kernel/*.asm)
C_SOURCES = $(wildcard $(SRCDIR)/kernel/*.c) $(wildcard $(SRCDIR)/drivers/*.c) $(wildcard $(SRCDIR)/mm/*.c) $(wildcard $(SRCDIR)/shell/*.c) $(wildcard $(SRCDIR)/gui/*.c) $(wildcard $(SRCDIR)/apps/*.c)

# Object files
ASM_OBJECTS = $(ASM_SOURCES:$(SRCDIR)/%.asm=$(BUILDDIR)/%.o)
C_OBJECTS = $(C_SOURCES:$(SRCDIR)/%.c=$(BUILDDIR)/%.o)
OBJECTS = $(ASM_OBJECTS) $(C_OBJECTS)

# Target kernel
KERNEL = $(BUILDDIR)/kernel.elf

# Default target
all: $(KERNEL)

# Create build directories
$(BUILDDIR):
	mkdir -p $(BUILDDIR)/boot $(BUILDDIR)/kernel $(BUILDDIR)/drivers $(BUILDDIR)/mm $(BUILDDIR)/shell $(BUILDDIR)/gui $(BUILDDIR)/apps

# Compile assembly files
$(BUILDDIR)/%.o: $(SRCDIR)/%.asm | $(BUILDDIR)
	$(AS) $(ASFLAGS) $< -o $@

# Compile C files
$(BUILDDIR)/%.o: $(SRCDIR)/%.c | $(BUILDDIR)
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

# Link kernel
$(KERNEL): $(OBJECTS) $(SRCDIR)/linker.ld
	$(LD) -T $(SRCDIR)/linker.ld -o $@ $(OBJECTS) $(LDFLAGS)

# Create ISO image
iso: $(KERNEL)
	mkdir -p $(GRUBDIR)
	cp $(KERNEL) $(BOOTDIR)/
	cp grub.cfg $(GRUBDIR)/
	grub-mkrescue -o kronos.iso $(ISODIR)

# Run in QEMU
run: iso
	qemu-system-x86_64 -cdrom kronos.iso -m 512M

# Run in QEMU with debugging
debug: iso
	qemu-system-x86_64 -cdrom kronos.iso -m 512M -s -S

# Show GUI demo
demo:
	@echo "Opening Kronos OS GUI Demo..."
	@start kronos-gui-demo.html || open kronos-gui-demo.html || xdg-open kronos-gui-demo.html

# Clean build files
clean:
	rm -rf $(BUILDDIR) $(ISODIR) kronos.iso

# Install cross-compiler (Ubuntu/Debian)
install-deps:
	sudo apt update
	sudo apt install build-essential nasm qemu-system-x86 grub-pc-bin grub-common xorriso

.PHONY: all iso run debug clean install-deps
