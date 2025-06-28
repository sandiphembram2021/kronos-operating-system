# Kronos OS - A Modern 64-bit Operating System

Kronos OS is a lightweight, modular, and high-performance 64-bit operating system built from scratch in C and x86_64 Assembly.

## Features

### 🖥️ **Core Operating System**
- **64-bit Architecture**: Full x86_64 long mode support
- **Custom Bootloader**: GRUB2 multiboot2 support
- **Memory Management**: Physical memory manager with paging
- **Interrupt Handling**: IDT with timer and keyboard IRQ support
- **VGA Text Mode**: Console output with color support
- **PS/2 Keyboard**: Full keyboard input handling
- **CLI Shell (ksh)**: Interactive command-line interface

### 🎨 **Graphical User Interface (NEW!)**
- **Desktop Environment**: Ubuntu-like desktop with wallpaper and icons
- **Window Manager**: Full windowing system with title bars, controls, and focus management
- **Taskbar**: Bottom panel with start menu, running applications, and system tray
- **Start Menu**: Application launcher with categorized programs
- **Desktop Icons**: Clickable shortcuts for quick application access
- **Mouse Support**: Full mouse interaction with windows and desktop

### 📱 **Built-in Applications**
- **Terminal Emulator**: Full-featured terminal with command execution
- **File Manager**: Browse directories, view files, and navigate filesystem
- **Text Editor**: Multi-document editor with syntax highlighting and file operations
- **Calculator**: Mathematical calculations and operations
- **System Settings**: Configure system preferences and hardware
- **Web Browser**: Basic web browsing capabilities (planned)

### 🛠️ **Advanced Features**
- **Multi-Window Support**: Run multiple applications simultaneously
- **Window Management**: Minimize, maximize, close, move, and resize windows
- **Application Framework**: Extensible system for adding new applications
- **Event System**: Mouse and keyboard event handling
- **Graphics Framework**: 2D graphics with shapes, text, and image support
- **Theme System**: Ubuntu-inspired color scheme and styling

## Project Structure

```
kronos/
├── Makefile                    # Build system
├── grub.cfg                   # GRUB2 configuration
├── kronos-gui-demo.html       # Interactive GUI demonstration
├── src/
│   ├── boot/                  # Boot code and multiboot header
│   │   ├── boot.asm          # 64-bit transition and setup
│   │   └── multiboot2.asm    # Multiboot2 header
│   ├── kernel/                # Core kernel code
│   │   ├── main.c            # Kernel entry point
│   │   ├── gdt.c             # Global Descriptor Table
│   │   ├── idt.c             # Interrupt Descriptor Table
│   │   ├── interrupt.asm     # Interrupt handlers
│   │   ├── gdt_flush.asm     # GDT loading
│   │   └── string.c          # String utilities
│   ├── drivers/               # Device drivers
│   │   ├── vga.c             # VGA text mode driver
│   │   ├── keyboard.c        # PS/2 keyboard driver
│   │   └── framebuffer.c     # Graphics framebuffer driver
│   ├── gui/                   # Graphical user interface
│   │   ├── gui_main.c        # GUI system initialization
│   │   ├── window_manager.c  # Window management system
│   │   └── desktop.c         # Desktop environment
│   ├── apps/                  # GUI applications
│   │   ├── terminal_app.c    # Terminal emulator
│   │   ├── file_manager.c    # File browser application
│   │   └── text_editor.c     # Text editing application
│   ├── mm/                    # Memory management
│   │   └── memory.c          # Physical memory allocator
│   ├── shell/                 # Command shell
│   │   └── shell.c           # Kronos shell (ksh)
│   ├── include/               # Header files
│   │   ├── kronos.h          # Main system header
│   │   └── multiboot2.h      # Multiboot2 definitions
│   └── linker.ld             # Linker script
```

## Prerequisites

### For Building (Cross-Compilation Recommended)

1. **x86_64-elf-gcc** - Cross-compiler for x86_64
2. **NASM** - Netwide Assembler
3. **QEMU** - For testing and emulation
4. **GRUB2** - For creating bootable ISO

### Installation on Ubuntu/Debian

```bash
sudo apt update
sudo apt install build-essential nasm qemu-system-x86 grub-pc-bin grub-common xorriso

# For cross-compiler (recommended):
# Follow instructions at: https://wiki.osdev.org/GCC_Cross-Compiler
```

### Installation on Windows

1. Install MSYS2 or MinGW-w64
2. Install NASM from https://www.nasm.us/
3. Install QEMU from https://www.qemu.org/
4. Set up cross-compiler toolchain

## Building

```bash
# Build the kernel
make

# Create bootable ISO
make iso

# Run in QEMU
make run

# Run with debugging
make debug

# Show interactive GUI demo
make demo

# Clean build files
make clean
```

## Running

Once built, you can run Kronos OS in several ways:

### QEMU (Recommended for Testing)
```bash
make run
```

### Real Hardware
1. Create bootable USB/CD with the generated ISO
2. Boot from the device
3. Requires x86_64 CPU with at least 512MB RAM

## Shell Commands

Once booted, you'll see the Kronos shell prompt. Available commands:

### **System Commands**
- `help` - Show available commands
- `clear` - Clear the screen
- `meminfo` - Display memory usage statistics
- `uptime` - Show system uptime
- `echo <text>` - Echo text to console
- `reboot` - Restart the system
- `halt` - Halt the system

### **GUI Commands (NEW!)**
- `gui` - Start the graphical user interface
- `desktop` - Launch desktop environment with applications
- `demo` - Show interactive GUI demonstration

### **GUI Features**
- **Desktop Icons**: Click to launch applications
- **Start Menu**: Click "Kronos" button for application menu
- **Window Controls**: Minimize (−), Maximize (□), Close (×)
- **Taskbar**: Shows running applications and system tray
- **Applications**: Terminal, File Manager, Text Editor, Calculator, Settings

## Development Status

### Completed Features ✅
- [x] Project setup and build system
- [x] GRUB2 multiboot2 configuration
- [x] 64-bit boot stub and long mode transition
- [x] Kernel entry point and initialization
- [x] Global Descriptor Table (GDT) setup
- [x] Interrupt Descriptor Table (IDT) and handlers
- [x] VGA text mode driver
- [x] PS/2 keyboard driver
- [x] Basic memory management
- [x] Interactive CLI shell (ksh)

### Planned Features 🚧
- [ ] Timer-based scheduler
- [ ] Process management
- [ ] File system support (FAT12/EXT2)
- [ ] User mode applications
- [ ] System calls interface
- [ ] Network stack
- [ ] UEFI boot support

## Technical Details

### Memory Layout
- Kernel loads at 1MB (0x100000)
- Heap starts at 2MB (0x200000)
- 8MB heap size
- Identity mapping for first 2MB

### Interrupt Handling
- PIC remapped to IRQ 32-47
- Timer interrupt (IRQ0) at 32
- Keyboard interrupt (IRQ1) at 33
- Exception handlers for CPU faults

### Architecture
- Monolithic kernel design
- 64-bit long mode operation
- No user/kernel space separation (v1.0)
- Direct hardware access

## Contributing

This is an educational project demonstrating OS development concepts. Feel free to:

1. Fork the repository
2. Create feature branches
3. Submit pull requests
4. Report issues

## License

This project is released under the MIT License. See LICENSE file for details.

## Resources

- [OSDev Wiki](https://wiki.osdev.org/)
- [Intel 64 and IA-32 Architectures Software Developer's Manual](https://software.intel.com/content/www/us/en/develop/articles/intel-sdm.html)
- [Multiboot2 Specification](https://www.gnu.org/software/grub/manual/multiboot2/multiboot.html)

## Author

**Sandip Hembram**  
Date: 2025-06-28  
Version: 1.0

---

*Kronos OS - Building the future, one instruction at a time.*
