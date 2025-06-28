# Kronos OS - WSL2 Installation Script
# Run this script as Administrator in PowerShell

Write-Host "Kronos OS - Setting up WSL2 Development Environment" -ForegroundColor Green
Write-Host "=================================================" -ForegroundColor Green

# Check if running as administrator
if (-NOT ([Security.Principal.WindowsPrincipal] [Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole] "Administrator")) {
    Write-Host "ERROR: This script must be run as Administrator!" -ForegroundColor Red
    Write-Host "Right-click PowerShell and select 'Run as Administrator'" -ForegroundColor Yellow
    exit 1
}

Write-Host "Installing WSL2..." -ForegroundColor Yellow

# Enable WSL feature
dism.exe /online /enable-feature /featurename:Microsoft-Windows-Subsystem-Linux /all /norestart

# Enable Virtual Machine Platform
dism.exe /online /enable-feature /featurename:VirtualMachinePlatform /all /norestart

Write-Host "WSL features enabled. Please restart your computer." -ForegroundColor Green
Write-Host "After restart, run: wsl --install -d Ubuntu" -ForegroundColor Yellow
Write-Host "Then follow the Ubuntu setup instructions in setup-ubuntu.sh" -ForegroundColor Yellow

# Create Ubuntu setup script
$ubuntuScript = @"
#!/bin/bash
# Kronos OS - Ubuntu Development Environment Setup

echo "Kronos OS - Setting up Ubuntu Development Environment"
echo "=================================================="

# Update package list
sudo apt update

# Install development tools
echo "Installing development tools..."
sudo apt install -y build-essential nasm qemu-system-x86 grub-pc-bin grub-common xorriso wget curl git

# Create cross-compiler directory
mkdir -p ~/cross-compiler
cd ~/cross-compiler

# Download and build cross-compiler
echo "Building x86_64-elf cross-compiler (this will take 20-30 minutes)..."

# Download sources
wget -q https://ftp.gnu.org/gnu/binutils/binutils-2.39.tar.gz
wget -q https://ftp.gnu.org/gnu/gcc/gcc-12.2.0/gcc-12.2.0.tar.gz

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
make -j$(nproc)
make install
cd ..

# Build GCC
mkdir build-gcc
cd build-gcc
../gcc-12.2.0/configure --target=$TARGET --prefix="$PREFIX" --disable-nls --enable-languages=c,c++ --without-headers
make all-gcc -j$(nproc)
make all-target-libgcc -j$(nproc)
make install-gcc
make install-target-libgcc

# Add to PATH permanently
echo 'export PATH="$HOME/cross-compiler/x86_64-elf/bin:$PATH"' >> ~/.bashrc

echo "Cross-compiler installation complete!"
echo "Please run: source ~/.bashrc"
echo "Then verify with: x86_64-elf-gcc --version"

cd /mnt/c/Users/sandi/OneDrive/Desktop/os
echo "Ready to build Kronos OS!"
echo "Run: make && make iso && make run"
"@

$ubuntuScript | Out-File -FilePath "setup-ubuntu.sh" -Encoding UTF8

Write-Host "Created setup-ubuntu.sh for Ubuntu configuration" -ForegroundColor Green
Write-Host ""
Write-Host "Next steps:" -ForegroundColor Yellow
Write-Host "1. Restart your computer" -ForegroundColor White
Write-Host "2. Open PowerShell and run: wsl --install -d Ubuntu" -ForegroundColor White
Write-Host "3. Set up Ubuntu username/password" -ForegroundColor White
Write-Host "4. In Ubuntu terminal, run: chmod +x setup-ubuntu.sh && ./setup-ubuntu.sh" -ForegroundColor White
Write-Host "5. Build Kronos OS with: make && make iso && make run" -ForegroundColor White
