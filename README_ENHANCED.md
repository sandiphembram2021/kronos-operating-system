# 🚀 Kronos Operating System

<div align="center">

![Kronos OS Logo](https://img.shields.io/badge/Kronos%20OS-v2.1.0-orange?style=for-the-badge&logo=linux)
![Build Status](https://img.shields.io/badge/Build-Passing-brightgreen?style=for-the-badge)
![License](https://img.shields.io/badge/License-MIT-blue?style=for-the-badge)
![Platform](https://img.shields.io/badge/Platform-x86__64-red?style=for-the-badge)

**A Modern 64-bit Operating System Built from Scratch**

*Educational • Real-Time • Arduino Compatible • Professional*

[🎮 Live Demo](./kronos-fixed-demo.html) • [📖 Documentation](./KRONOS_OS_COMPLETE_SUMMARY.md) • [🛠️ Setup Guide](./setup.md) • [🎯 Features](#features)

</div>

---

## 🌟 **What is Kronos OS?**

Kronos OS is a **comprehensive, educational operating system** built from scratch in C and x86_64 Assembly. Designed for learning, teaching, and real-world applications, it features a complete GUI environment, real-time capabilities, and Arduino/IoT integration.

### 🎯 **Perfect For:**
- 🎓 **Computer Science Education** - Teaching OS concepts
- 🔧 **Arduino Development** - Embedded systems learning
- 🏫 **STEM Education** - Hands-on programming experience
- 🔬 **Research Projects** - OS development and RTOS studies
- 💻 **Professional Development** - Understanding system internals

---

## ✨ **Key Features**

### 🖥️ **Core Operating System**
- ✅ **64-bit Kernel** - Modern x86_64 architecture
- ✅ **Memory Management** - Advanced virtual memory system
- ✅ **Process Scheduling** - CFS (Completely Fair Scheduler)
- ✅ **Interrupt Handling** - Hardware and software interrupts
- ✅ **File System** - Virtual file system with multiple backends
- ✅ **Device Drivers** - VGA, keyboard, timer, and more

### 🎨 **Graphical User Interface**
- ✅ **Modern Desktop Environment** - Professional GUI with taskbar
- ✅ **Window Management** - Minimize, maximize, restore with animations
- ✅ **Application Suite** - 8+ built-in applications
- ✅ **App Store** - Application marketplace with 15+ apps
- ✅ **Settings System** - Comprehensive system configuration
- ✅ **File Manager** - Advanced file operations and navigation

### ⚡ **Real-Time Operating System (RTOS)**
- ✅ **Priority-based Scheduling** - 100 priority levels (0-99)
- ✅ **Real-time IPC** - Semaphores, mutexes, message queues
- ✅ **Deadline Monitoring** - Task deadline violation detection
- ✅ **Performance Monitoring** - CPU utilization and timing stats
- ✅ **Interrupt Latency** - <10μs maximum interrupt response
- ✅ **Deterministic Behavior** - Guaranteed response times

### 🤖 **Arduino & IoT Integration**
- ✅ **Hardware Interfaces** - GPIO, I2C, SPI, UART, PWM, ADC
- ✅ **Sensor Support** - Temperature, humidity, pressure, light
- ✅ **Real-time Monitoring** - Live sensor data visualization
- ✅ **Development Tools** - Arduino IDE integration
- ✅ **Educational Commands** - Hardware learning interface
- ✅ **IoT Dashboard** - Multi-device monitoring system

---

## 🚀 **Quick Start**

### 🎮 **Try the Live Demo**
Experience Kronos OS instantly in your browser:
```bash
# Open the demo file in your browser
open kronos-fixed-demo.html
```

### 🛠️ **Build from Source**

#### Prerequisites
```bash
# Ubuntu/Debian
sudo apt update
sudo apt install build-essential nasm qemu-system-x86 grub2-common xorriso

# Install cross-compiler
sudo apt install gcc-multilib
```

#### Build & Run
```bash
# Clone the repository
git clone https://github.com/sandiphembram2021/kronos-operating-system.git
cd kronos-operating-system

# Build the OS
make clean
make all

# Run in QEMU
make run

# Or use the automated setup
chmod +x auto-setup.sh
./auto-setup.sh
```

### 🪟 **Windows Setup**
```powershell
# Run the Windows setup script
.\install-wsl.ps1

# Or use the PowerShell runner
.\run-kronos.ps1
```

---

## 📱 **Applications & Features**

### 🏪 **Built-in Applications**
| Application | Description | Features |
|-------------|-------------|----------|
| 🖥️ **Terminal** | Advanced command-line interface | Real-time OS data, Arduino commands, 25+ built-in commands |
| 📁 **File Manager** | Professional file operations | Multiple view modes, navigation, file operations |
| 🧮 **Calculator** | Scientific calculator | Basic and advanced mathematical operations |
| 🎮 **Games** | Entertainment center | 6 classic games with modern enhancements |
| ⚙️ **Settings** | System configuration | 12 categories, network settings, display options |
| 📊 **Monitor** | System performance | Real-time metrics, hardware monitoring |
| 🎵 **Music Player** | Audio playback | Media controls, playlist support |
| 🎬 **Video Player** | Video playback | Media library, playback controls |

### 🏬 **App Store (15+ Applications)**
- 💻 **Kronos Code Editor** - Professional IDE for Arduino & C++
- 🔌 **Circuit Simulator** - Electronic circuit design and testing
- 📊 **IoT Dashboard** - Real-time sensor monitoring
- 📝 **Advanced Text Editor** - Syntax highlighting and plugins
- 🔧 **Arduino IDE Pro** - Enhanced development environment
- 🧮 **Mathematical Solver** - Equation solving and graphing
- 🎨 **Pixel Art Studio** - Digital art creation
- 🌐 **Network Analyzer** - Traffic monitoring and analysis

---

## 🎓 **Educational Features**

### 📚 **Learning Modules**
- **Operating System Concepts** - Process management, memory, I/O
- **Real-Time Systems** - RTOS principles and implementation
- **Hardware Interfaces** - GPIO, communication protocols
- **System Programming** - Kernel development, device drivers
- **Embedded Systems** - Arduino integration and IoT development

### 🔧 **Arduino/IoT Commands**
```bash
# Hardware interface commands
gpio          # GPIO pin control and status
i2c           # I2C bus communication
spi           # SPI protocol operations
uart          # Serial communication
pwm           # Pulse width modulation
adc           # Analog-to-digital conversion
sensors       # Environmental sensor readings
temp          # Temperature monitoring
voltage       # Voltage level monitoring
```

### 📊 **Real-Time Monitoring**
- **Live System Metrics** - CPU, memory, storage usage
- **Hardware Status** - GPIO pins, communication buses
- **Environmental Data** - Temperature, humidity, pressure
- **Performance Analytics** - Timing, latency, throughput

---

## 🏗️ **Architecture**

### 🧠 **Kernel Architecture**
```
┌─────────────────────────────────────┐
│           User Applications         │
├─────────────────────────────────────┤
│              GUI Layer              │
├─────────────────────────────────────┤
│            System Calls             │
├─────────────────────────────────────┤
│         Kernel Services             │
│  ┌─────────┬─────────┬─────────┐    │
│  │   VFS   │   IPC   │  RTOS   │    │
│  └─────────┴─────────┴─────────┘    │
├─────────────────────────────────────┤
│          Device Drivers             │
├─────────────────────────────────────┤
│         Hardware Layer              │
└─────────────────────────────────────┘
```

### 📁 **Project Structure**
```
kronos-operating-system/
├── src/                    # Source code
│   ├── kernel/            # Kernel implementation
│   ├── drivers/           # Device drivers
│   ├── mm/                # Memory management
│   ├── gui/               # GUI system
│   ├── apps/              # Built-in applications
│   └── boot/              # Boot loader
├── include/               # Header files
├── demo files/            # Browser-based demos
├── docs/                  # Documentation
└── tools/                 # Build tools and scripts
```

---

## 📊 **Technical Specifications**

### ⚡ **Performance Metrics**
- **Boot Time:** < 5 seconds in QEMU
- **Memory Usage:** 64MB minimum, 512MB recommended
- **Interrupt Latency:** < 10 microseconds
- **Context Switch:** < 5 microseconds
- **File System:** Virtual FS with multiple backends
- **Network Stack:** TCP/IP implementation

### 🔧 **Hardware Requirements**
- **Architecture:** x86_64 (64-bit)
- **RAM:** 64MB minimum, 512MB recommended
- **Storage:** 100MB minimum
- **Graphics:** VGA compatible
- **Network:** Ethernet adapter (optional)

### 🛠️ **Development Tools**
- **Compiler:** GCC cross-compiler (x86_64-elf-gcc)
- **Assembler:** NASM
- **Bootloader:** GRUB2
- **Emulator:** QEMU
- **Debugger:** GDB with QEMU integration

---

## 🤝 **Contributing**

We welcome contributions from the community! Here's how you can help:

### 🐛 **Bug Reports**
- Use GitHub Issues to report bugs
- Include system information and reproduction steps
- Provide logs and error messages

### 💡 **Feature Requests**
- Suggest new features or improvements
- Discuss implementation approaches
- Help with testing and validation

### 🔧 **Development**
```bash
# Fork the repository
git fork https://github.com/sandiphembram2021/kronos-operating-system.git

# Create a feature branch
git checkout -b feature/amazing-feature

# Make your changes and commit
git commit -m "Add amazing feature"

# Push to your fork and create a pull request
git push origin feature/amazing-feature
```

---

## 📄 **License**

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

---

## 👨‍💻 **Author**

**Sandip Hembram**
- 🌐 GitHub: [@sandiphembram2021](https://github.com/sandiphembram2021)
- 📧 Email: [Contact](mailto:sandiphembram2021@gmail.com)

---

<div align="center">

### 🌟 **Star this repository if you find it helpful!** ⭐

**Built with ❤️ for education, innovation, and the future of computing**

*Kronos OS - Where Learning Meets Innovation* 🚀

</div>
