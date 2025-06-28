# Kronos OS Development Setup

This guide will help you set up the development environment for building Kronos OS.

## Required Tools

1. **x86_64-elf-gcc** - Cross-compiler for 64-bit x86 targets
2. **NASM** - Netwide Assembler for assembly code
3. **QEMU** - System emulator for testing
4. **GRUB2 tools** - For creating bootable images

## Setup Instructions

### Option 1: Linux (Ubuntu/Debian) - Recommended

```bash
# Update package list
sudo apt update

# Install basic development tools
sudo apt install build-essential nasm qemu-system-x86 grub-pc-bin grub-common xorriso

# Install cross-compiler dependencies
sudo apt install gcc-multilib

# Build cross-compiler (this takes time)
mkdir ~/cross-compiler
cd ~/cross-compiler

# Download binutils and GCC
wget https://ftp.gnu.org/gnu/binutils/binutils-2.39.tar.gz
wget https://ftp.gnu.org/gnu/gcc/gcc-12.2.0/gcc-12.2.0.tar.gz

# Extract
tar -xf binutils-2.39.tar.gz
tar -xf gcc-12.2.0.tar.gz

# Set environment
export PREFIX="$HOME/cross-compiler/x86_64-elf"
export TARGET=x86_64-elf
export PATH="$PREFIX/bin:$PATH"

# Build binutils
mkdir build-binutils
cd build-binutils
../binutils-2.39/configure --target=$TARGET --prefix="$PREFIX" --with-sysroot --disable-nls --disable-werror
make
make install
cd ..

# Build GCC
mkdir build-gcc
cd build-gcc
../gcc-12.2.0/configure --target=$TARGET --prefix="$PREFIX" --disable-nls --enable-languages=c,c++ --without-headers
make all-gcc
make all-target-libgcc
make install-gcc
make install-target-libgcc
```

### Option 2: Windows with WSL2

```bash
# Install WSL2 with Ubuntu
wsl --install -d Ubuntu

# Follow Linux instructions above in WSL2
```

### Option 3: Windows with MSYS2

```bash
# Install MSYS2 from https://www.msys2.org/

# In MSYS2 terminal:
pacman -S mingw-w64-x86_64-gcc
pacman -S mingw-w64-x86_64-nasm
pacman -S mingw-w64-x86_64-qemu

# Note: You'll need to build the cross-compiler manually
```

### Option 4: Docker (Cross-platform)

Create a Dockerfile:

```dockerfile
FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    build-essential \
    nasm \
    qemu-system-x86 \
    grub-pc-bin \
    grub-common \
    xorriso \
    wget \
    && rm -rf /var/lib/apt/lists/*

# Build cross-compiler
WORKDIR /cross-compiler
RUN wget https://ftp.gnu.org/gnu/binutils/binutils-2.39.tar.gz && \
    wget https://ftp.gnu.org/gnu/gcc/gcc-12.2.0/gcc-12.2.0.tar.gz && \
    tar -xf binutils-2.39.tar.gz && \
    tar -xf gcc-12.2.0.tar.gz

ENV PREFIX="/cross-compiler/x86_64-elf"
ENV TARGET="x86_64-elf"
ENV PATH="$PREFIX/bin:$PATH"

RUN mkdir build-binutils && cd build-binutils && \
    ../binutils-2.39/configure --target=$TARGET --prefix="$PREFIX" --with-sysroot --disable-nls --disable-werror && \
    make && make install

RUN mkdir build-gcc && cd build-gcc && \
    ../gcc-12.2.0/configure --target=$TARGET --prefix="$PREFIX" --disable-nls --enable-languages=c,c++ --without-headers && \
    make all-gcc && make all-target-libgcc && \
    make install-gcc && make install-target-libgcc

WORKDIR /kronos
```

Build and run:
```bash
docker build -t kronos-dev .
docker run -it -v $(pwd):/kronos kronos-dev
```

## Verification

After setup, verify your tools:

```bash
# Check cross-compiler
x86_64-elf-gcc --version

# Check assembler
nasm --version

# Check QEMU
qemu-system-x86_64 --version

# Check GRUB tools
grub-mkrescue --version
```

## Building Kronos OS

Once tools are installed:

```bash
# Clone/navigate to Kronos OS directory
cd kronos-os

# Build kernel
make

# Create ISO
make iso

# Run in QEMU
make run
```

## Troubleshooting

### Cross-compiler not found
- Ensure `$PREFIX/bin` is in your PATH
- Verify the cross-compiler was built successfully
- Check that `x86_64-elf-gcc` exists in the bin directory

### NASM not found
- Install NASM using your package manager
- On Windows, download from https://www.nasm.us/

### QEMU not found
- Install QEMU using your package manager
- On Windows, download from https://www.qemu.org/

### GRUB tools not found
- Install grub-pc-bin and grub-common packages
- Ensure xorriso is installed for ISO creation

### Build errors
- Check that all source files are present
- Verify include paths in Makefile
- Ensure proper file permissions

## Alternative: Pre-built Toolchain

For faster setup, you can download pre-built cross-compilers:

- **Linux**: Available in some distributions' repositories
- **Windows**: MinGW-w64 with cross-compilation support
- **macOS**: Homebrew cross-compilation tools

## Next Steps

After successful setup:

1. Build the kernel: `make`
2. Test in QEMU: `make run`
3. Explore the source code
4. Try modifying features
5. Add new functionality

## Support

If you encounter issues:

1. Check the troubleshooting section
2. Verify all prerequisites are installed
3. Ensure proper versions of tools
4. Check file permissions and paths

For development questions, refer to:
- [OSDev Wiki](https://wiki.osdev.org/)
- [GCC Cross-Compiler Guide](https://wiki.osdev.org/GCC_Cross-Compiler)
- [NASM Documentation](https://www.nasm.us/docs.php)
