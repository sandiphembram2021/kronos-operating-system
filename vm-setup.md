# Kronos OS - Virtual Machine Setup Guide

If you prefer using a virtual machine instead of WSL2, follow this guide.

## Option A: VirtualBox Setup

### 1. Download and Install VirtualBox
- Download from: https://www.virtualbox.org/
- Install with default settings

### 2. Download Ubuntu ISO
- Download Ubuntu 22.04 LTS from: https://ubuntu.com/download/desktop
- Choose the 64-bit version

### 3. Create Virtual Machine
1. Open VirtualBox
2. Click "New"
3. Name: "Kronos-Dev"
4. Type: Linux
5. Version: Ubuntu (64-bit)
6. Memory: 4096 MB (4GB)
7. Create virtual hard disk: 20GB VDI
8. Settings → System → Enable VT-x/AMD-V
9. Settings → System → Enable nested paging

### 4. Install Ubuntu
1. Start the VM
2. Select Ubuntu ISO file
3. Follow Ubuntu installation
4. Username: developer
5. Install updates and third-party software

### 5. Install Development Tools
```bash
# Update system
sudo apt update && sudo apt upgrade -y

# Install development tools
sudo apt install -y build-essential nasm qemu-system-x86 grub-pc-bin grub-common xorriso wget curl git vim

# Install VirtualBox Guest Additions (for better integration)
sudo apt install -y virtualbox-guest-additions-iso
```

### 6. Build Cross-Compiler
```bash
# Create directory
mkdir ~/cross-compiler && cd ~/cross-compiler

# Download sources
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
mkdir build-binutils && cd build-binutils
../binutils-2.39/configure --target=$TARGET --prefix="$PREFIX" --with-sysroot --disable-nls --disable-werror
make -j$(nproc) && make install && cd ..

# Build GCC
mkdir build-gcc && cd build-gcc
../gcc-12.2.0/configure --target=$TARGET --prefix="$PREFIX" --disable-nls --enable-languages=c,c++ --without-headers
make all-gcc -j$(nproc)
make all-target-libgcc -j$(nproc)
make install-gcc
make install-target-libgcc

# Add to PATH
echo 'export PATH="$HOME/cross-compiler/x86_64-elf/bin:$PATH"' >> ~/.bashrc
source ~/.bashrc
```

### 7. Transfer Kronos OS Files
1. Create shared folder in VirtualBox
2. Or use git to clone the repository
3. Or copy files via USB/network

## Option B: VMware Setup

### 1. Download VMware
- VMware Workstation Pro (paid)
- VMware Player (free for personal use)

### 2. Similar steps as VirtualBox
- Create new VM with Ubuntu
- Allocate 4GB RAM, 20GB disk
- Enable virtualization features

## Option C: Cloud Development

### GitHub Codespaces
1. Push Kronos OS to GitHub repository
2. Open in GitHub Codespaces
3. Install tools in the cloud environment

### Replit
1. Create new Repl with C template
2. Upload Kronos OS files
3. Install cross-compiler tools

## Running Kronos OS

Once your development environment is ready:

```bash
# Navigate to Kronos OS directory
cd kronos-os

# Build the kernel
make

# Create bootable ISO
make iso

# Run in QEMU
make run
```

## Troubleshooting

### VirtualBox Issues
- Enable VT-x/AMD-V in BIOS
- Increase VM memory if build fails
- Install Guest Additions for better performance

### QEMU Display Issues
```bash
# If QEMU doesn't show display, try:
make run QEMU_OPTS="-display gtk"

# Or use VNC:
make run QEMU_OPTS="-vnc :1"
# Then connect with VNC viewer to localhost:5901
```

### Cross-Compiler Build Fails
```bash
# Install additional dependencies
sudo apt install -y gcc-multilib g++-multilib

# If still fails, try pre-built toolchain:
sudo apt install gcc-x86-64-linux-gnu
# Then modify Makefile to use x86_64-linux-gnu-gcc
```

## Performance Tips

1. **Allocate enough resources**:
   - 4GB+ RAM for VM
   - Enable all CPU cores
   - Use SSD if available

2. **Enable hardware acceleration**:
   - VT-x/AMD-V in BIOS
   - Nested virtualization in VM settings

3. **Use shared folders**:
   - Edit code on host OS
   - Build in VM for better performance

## Next Steps

After successful setup:
1. Verify tools: `x86_64-elf-gcc --version`
2. Build Kronos: `make && make iso`
3. Test in QEMU: `make run`
4. Explore and modify the code
5. Add new features to the OS

## Support

If you encounter issues:
- Check VM logs
- Verify virtualization is enabled
- Ensure sufficient resources
- Try different VM software if needed
