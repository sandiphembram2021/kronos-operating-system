#!/bin/bash
# Kronos OS - Automatic Setup Script for Ubuntu/WSL

set -e

echo "╔══════════════════════════════════════════════════════════════╗"
echo "║                    KRONOS OS AUTO SETUP                     ║"
echo "║                  64-bit Operating System                    ║"
echo "╚══════════════════════════════════════════════════════════════╝"
echo ""

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

print_status() {
    echo -e "${CYAN}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Check if running in WSL
if grep -q Microsoft /proc/version; then
    print_status "Running in WSL environment"
    WSL_ENV=true
else
    print_status "Running in native Linux environment"
    WSL_ENV=false
fi

# Update system
print_status "Updating package lists..."
sudo apt update

# Install basic development tools
print_status "Installing development tools..."
sudo apt install -y \
    build-essential \
    nasm \
    qemu-system-x86 \
    grub-pc-bin \
    grub-common \
    xorriso \
    wget \
    curl \
    git \
    vim \
    tree

print_success "Development tools installed"

# Create cross-compiler directory
print_status "Setting up cross-compiler..."
mkdir -p ~/cross-compiler
cd ~/cross-compiler

# Check if cross-compiler already exists
if [ -f "$HOME/cross-compiler/x86_64-elf/bin/x86_64-elf-gcc" ]; then
    print_success "Cross-compiler already installed"
else
    print_status "Building x86_64-elf cross-compiler (this may take 20-30 minutes)..."
    
    # Download sources
    if [ ! -f "binutils-2.39.tar.gz" ]; then
        print_status "Downloading binutils..."
        wget -q https://ftp.gnu.org/gnu/binutils/binutils-2.39.tar.gz
    fi
    
    if [ ! -f "gcc-12.2.0.tar.gz" ]; then
        print_status "Downloading GCC..."
        wget -q https://ftp.gnu.org/gnu/gcc/gcc-12.2.0/gcc-12.2.0.tar.gz
    fi
    
    # Extract if not already done
    if [ ! -d "binutils-2.39" ]; then
        print_status "Extracting binutils..."
        tar -xf binutils-2.39.tar.gz
    fi
    
    if [ ! -d "gcc-12.2.0" ]; then
        print_status "Extracting GCC..."
        tar -xf gcc-12.2.0.tar.gz
    fi
    
    # Set environment
    export PREFIX="$HOME/cross-compiler/x86_64-elf"
    export TARGET=x86_64-elf
    export PATH="$PREFIX/bin:$PATH"
    
    # Build binutils
    if [ ! -d "build-binutils" ]; then
        print_status "Building binutils..."
        mkdir build-binutils
        cd build-binutils
        ../binutils-2.39/configure --target=$TARGET --prefix="$PREFIX" --with-sysroot --disable-nls --disable-werror
        make -j$(nproc)
        make install
        cd ..
        print_success "Binutils built and installed"
    fi
    
    # Build GCC
    if [ ! -d "build-gcc" ]; then
        print_status "Building GCC..."
        mkdir build-gcc
        cd build-gcc
        ../gcc-12.2.0/configure --target=$TARGET --prefix="$PREFIX" --disable-nls --enable-languages=c,c++ --without-headers
        make all-gcc -j$(nproc)
        make all-target-libgcc -j$(nproc)
        make install-gcc
        make install-target-libgcc
        cd ..
        print_success "GCC built and installed"
    fi
fi

# Add to PATH permanently
if ! grep -q "cross-compiler/x86_64-elf/bin" ~/.bashrc; then
    echo 'export PATH="$HOME/cross-compiler/x86_64-elf/bin:$PATH"' >> ~/.bashrc
    print_success "Added cross-compiler to PATH"
fi

# Source the updated PATH
export PATH="$HOME/cross-compiler/x86_64-elf/bin:$PATH"

# Verify cross-compiler
if command -v x86_64-elf-gcc &> /dev/null; then
    print_success "Cross-compiler verification: $(x86_64-elf-gcc --version | head -n1)"
else
    print_error "Cross-compiler not found in PATH"
    exit 1
fi

# Navigate to Kronos OS directory
if [ "$WSL_ENV" = true ]; then
    KRONOS_DIR="/mnt/c/Users/sandi/OneDrive/Desktop/os"
else
    KRONOS_DIR="$(pwd)"
fi

print_status "Navigating to Kronos OS directory: $KRONOS_DIR"
cd "$KRONOS_DIR"

# Verify Kronos OS files
if [ ! -f "Makefile" ] || [ ! -d "src" ]; then
    print_error "Kronos OS files not found in $KRONOS_DIR"
    print_error "Please ensure you're in the correct directory"
    exit 1
fi

print_success "Kronos OS source files found"

# Build Kronos OS
print_status "Building Kronos OS kernel..."
make clean 2>/dev/null || true

if make; then
    print_success "Kernel built successfully"
else
    print_error "Kernel build failed"
    exit 1
fi

# Create ISO
print_status "Creating bootable ISO..."
if make iso; then
    print_success "Bootable ISO created: kronos.iso"
else
    print_error "ISO creation failed"
    exit 1
fi

# Check if we can run QEMU
print_status "Checking QEMU availability..."
if command -v qemu-system-x86_64 &> /dev/null; then
    print_success "QEMU found: $(qemu-system-x86_64 --version | head -n1)"
    
    # Check if we're in WSL (GUI might not work)
    if [ "$WSL_ENV" = true ]; then
        print_warning "Running in WSL - GUI display might not work"
        print_status "You can run Kronos OS with: make run"
        print_status "Or copy kronos.iso to Windows and run in a VM"
    else
        print_status "Starting Kronos OS in QEMU..."
        print_success "Kronos OS is booting! Press Ctrl+Alt+G to release mouse, Ctrl+Alt+Q to quit"
        make run
    fi
else
    print_error "QEMU not found"
fi

print_success "Kronos OS setup complete!"
print_status "Available commands:"
echo "  make          - Build kernel"
echo "  make iso      - Create bootable ISO"
echo "  make run      - Run in QEMU"
echo "  make clean    - Clean build files"
echo ""
print_status "Kronos OS shell commands (when running):"
echo "  help          - Show available commands"
echo "  clear         - Clear screen"
echo "  meminfo       - Show memory information"
echo "  uptime        - Show system uptime"
echo "  echo <text>   - Echo text"
echo "  reboot        - Restart system"
echo "  halt          - Halt system"
