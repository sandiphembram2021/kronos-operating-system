# Kronos OS - Automated Setup and Run Script
# This script will help you choose the best option to run Kronos OS

param(
    [string]$Option = ""
)

Write-Host @"
╔══════════════════════════════════════════════════════════════╗
║                        KRONOS OS v1.0                       ║
║                   64-bit Operating System                   ║
║                                                              ║
║                    Choose Setup Option:                     ║
╚══════════════════════════════════════════════════════════════╝
"@ -ForegroundColor Cyan

Write-Host ""
Write-Host "Available Options:" -ForegroundColor Yellow
Write-Host "1. WSL2 Setup (Recommended for Windows)" -ForegroundColor Green
Write-Host "2. Docker Setup (If Docker is available)" -ForegroundColor Green  
Write-Host "3. Virtual Machine Guide" -ForegroundColor Green
Write-Host "4. Online Development (GitHub Codespaces)" -ForegroundColor Green
Write-Host "5. Check Current System" -ForegroundColor Green
Write-Host "6. Exit" -ForegroundColor Red
Write-Host ""

if ($Option -eq "") {
    $Option = Read-Host "Enter your choice (1-6)"
}

switch ($Option) {
    "1" {
        Write-Host "Setting up WSL2..." -ForegroundColor Yellow
        
        # Check if WSL is available
        $wslStatus = wsl --status 2>$null
        if ($LASTEXITCODE -eq 0) {
            Write-Host "WSL is already installed!" -ForegroundColor Green
            $distros = wsl --list --quiet
            if ($distros -match "Ubuntu") {
                Write-Host "Ubuntu is installed. Opening WSL..." -ForegroundColor Green
                Write-Host "Run these commands in WSL:" -ForegroundColor Yellow
                Write-Host "cd /mnt/c/Users/sandi/OneDrive/Desktop/os" -ForegroundColor White
                Write-Host "chmod +x setup-ubuntu.sh && ./setup-ubuntu.sh" -ForegroundColor White
                wsl
            } else {
                Write-Host "Installing Ubuntu..." -ForegroundColor Yellow
                wsl --install -d Ubuntu
            }
        } else {
            Write-Host "WSL not available. Running installation script..." -ForegroundColor Yellow
            if (Test-Path "install-wsl.ps1") {
                & .\install-wsl.ps1
            } else {
                Write-Host "install-wsl.ps1 not found. Please run as Administrator:" -ForegroundColor Red
                Write-Host "dism.exe /online /enable-feature /featurename:Microsoft-Windows-Subsystem-Linux /all /norestart" -ForegroundColor White
                Write-Host "dism.exe /online /enable-feature /featurename:VirtualMachinePlatform /all /norestart" -ForegroundColor White
                Write-Host "Then restart and run: wsl --install -d Ubuntu" -ForegroundColor White
            }
        }
    }
    
    "2" {
        Write-Host "Checking Docker..." -ForegroundColor Yellow
        
        try {
            $dockerVersion = docker --version 2>$null
            if ($LASTEXITCODE -eq 0) {
                Write-Host "Docker is available!" -ForegroundColor Green
                Write-Host "Creating Dockerfile..." -ForegroundColor Yellow
                
                $dockerfile = @"
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

WORKDIR /kronos
COPY . .

RUN chmod +x setup-ubuntu.sh || true

CMD ["/bin/bash"]
"@
                $dockerfile | Out-File -FilePath "Dockerfile" -Encoding UTF8
                
                Write-Host "Building Docker image..." -ForegroundColor Yellow
                docker build -t kronos-os .
                
                Write-Host "Running Docker container..." -ForegroundColor Yellow
                docker run -it kronos-os
            } else {
                Write-Host "Docker not found. Please install Docker Desktop:" -ForegroundColor Red
                Write-Host "https://www.docker.com/products/docker-desktop" -ForegroundColor Blue
            }
        } catch {
            Write-Host "Docker not available. Install from: https://www.docker.com/products/docker-desktop" -ForegroundColor Red
        }
    }
    
    "3" {
        Write-Host "Opening Virtual Machine setup guide..." -ForegroundColor Yellow
        if (Test-Path "vm-setup.md") {
            notepad "vm-setup.md"
        } else {
            Write-Host "vm-setup.md not found!" -ForegroundColor Red
        }
        Write-Host "Key steps:" -ForegroundColor Yellow
        Write-Host "1. Install VirtualBox or VMware" -ForegroundColor White
        Write-Host "2. Create Ubuntu 22.04 VM (4GB RAM, 20GB disk)" -ForegroundColor White
        Write-Host "3. Install development tools in VM" -ForegroundColor White
        Write-Host "4. Build cross-compiler" -ForegroundColor White
        Write-Host "5. Copy Kronos OS files to VM" -ForegroundColor White
        Write-Host "6. Build and run: make && make iso && make run" -ForegroundColor White
    }
    
    "4" {
        Write-Host "Online Development Options:" -ForegroundColor Yellow
        Write-Host ""
        Write-Host "GitHub Codespaces (Recommended):" -ForegroundColor Green
        Write-Host "1. Create GitHub repository" -ForegroundColor White
        Write-Host "2. Upload Kronos OS files" -ForegroundColor White
        Write-Host "3. Open in Codespaces" -ForegroundColor White
        Write-Host "4. Run setup commands" -ForegroundColor White
        Write-Host ""
        Write-Host "Other options:" -ForegroundColor Green
        Write-Host "- Replit: https://replit.com" -ForegroundColor Blue
        Write-Host "- Gitpod: https://gitpod.io" -ForegroundColor Blue
        Write-Host "- Google Cloud Shell: https://console.cloud.google.com" -ForegroundColor Blue
        Write-Host ""
        if (Test-Path "run-online.md") {
            $openOnline = Read-Host "Open detailed online guide? (y/n)"
            if ($openOnline -eq "y") {
                notepad "run-online.md"
            }
        }
    }
    
    "5" {
        Write-Host "Checking current system capabilities..." -ForegroundColor Yellow
        Write-Host ""
        
        # Check GCC
        try {
            $gccVersion = gcc --version 2>$null
            if ($LASTEXITCODE -eq 0) {
                Write-Host "✓ GCC found: $($gccVersion[0])" -ForegroundColor Green
            } else {
                Write-Host "✗ GCC not found" -ForegroundColor Red
            }
        } catch {
            Write-Host "✗ GCC not found" -ForegroundColor Red
        }
        
        # Check NASM
        try {
            $nasmVersion = nasm --version 2>$null
            if ($LASTEXITCODE -eq 0) {
                Write-Host "✓ NASM found: $nasmVersion" -ForegroundColor Green
            } else {
                Write-Host "✗ NASM not found" -ForegroundColor Red
            }
        } catch {
            Write-Host "✗ NASM not found" -ForegroundColor Red
        }
        
        # Check QEMU
        try {
            $qemuVersion = qemu-system-x86_64 --version 2>$null
            if ($LASTEXITCODE -eq 0) {
                Write-Host "✓ QEMU found: $($qemuVersion[0])" -ForegroundColor Green
            } else {
                Write-Host "✗ QEMU not found" -ForegroundColor Red
            }
        } catch {
            Write-Host "✗ QEMU not found" -ForegroundColor Red
        }
        
        # Check WSL
        try {
            $wslStatus = wsl --status 2>$null
            if ($LASTEXITCODE -eq 0) {
                Write-Host "✓ WSL available" -ForegroundColor Green
            } else {
                Write-Host "✗ WSL not available" -ForegroundColor Red
            }
        } catch {
            Write-Host "✗ WSL not available" -ForegroundColor Red
        }
        
        # Check Docker
        try {
            $dockerVersion = docker --version 2>$null
            if ($LASTEXITCODE -eq 0) {
                Write-Host "✓ Docker found: $dockerVersion" -ForegroundColor Green
            } else {
                Write-Host "✗ Docker not found" -ForegroundColor Red
            }
        } catch {
            Write-Host "✗ Docker not found" -ForegroundColor Red
        }
        
        Write-Host ""
        Write-Host "Recommendation:" -ForegroundColor Yellow
        Write-Host "Install WSL2 with Ubuntu for the best development experience." -ForegroundColor Green
    }
    
    "6" {
        Write-Host "Goodbye! Happy OS development!" -ForegroundColor Green
        exit
    }
    
    default {
        Write-Host "Invalid option. Please choose 1-6." -ForegroundColor Red
        & $MyInvocation.MyCommand.Path
    }
}

Write-Host ""
Write-Host "For more information, check these files:" -ForegroundColor Yellow
Write-Host "- README.md: Project overview" -ForegroundColor White
Write-Host "- setup.md: Detailed setup instructions" -ForegroundColor White
Write-Host "- vm-setup.md: Virtual machine guide" -ForegroundColor White
Write-Host "- run-online.md: Online development options" -ForegroundColor White
