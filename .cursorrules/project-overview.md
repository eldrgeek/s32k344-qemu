---
name: "S32K344 QEMU Project Overview"
description: "High-level project structure and development flow"
applies_to:
  - "**/*"
contexts:
  - "project"
  - "overview"
  - "getting-started"
priority: 80
---

# S32K344 QEMU Project Overview

## Project Goal
Create a complete QEMU emulation environment for the NXP S32K344 automotive microcontroller, enabling firmware development and testing without physical hardware.

## Architecture Overview

### Target Hardware: NXP S32K344
- **CPU**: ARM Cortex-M7 @ 160MHz
- **Flash**: 4MB at 0x00400000
- **SRAM**: 512KB at 0x20000000
- **Peripherals**: LPUART, FlexCAN, ADC, GPIO (SIUL2), Timers
- **Package**: 144-pin LQFP
- **Use Case**: Automotive applications (body control, gateway, etc.)

### QEMU Implementation Structure
```
qemu-upstream/hw/arm/s32k344/
├── s32k344_machine.c     # Main machine implementation
├── s32k344_siul2.c       # GPIO peripheral (144 pins)
├── s32k344_lpuart.c      # UART peripheral
└── s32k344.h             # Common definitions
```

## Development Environment

### Docker-Based Development
- **Container**: `s32k344-dev` with Ubuntu 24.04
- **Tools**: ARM GCC toolchain, QEMU dependencies, development tools
- **Volumes**: Project mounted at `/workspace/project`
- **Scripts**: `./scripts/docker-dev.sh` for container management

### Build System
- **QEMU**: Integrated via meson build system
- **Firmware**: ARM cross-compilation with custom linker scripts
- **Tests**: QTest framework for hardware validation

## Current Implementation Status

### ✅ Completed Components
1. **Development Environment**
   - Docker container with ARM toolchain
   - QEMU build system integration
   - Cross-compilation environment

2. **S32K344 Machine**
   - ARM Cortex-M7 CPU configuration
   - Memory layout (4MB Flash, 512KB SRAM)
   - NVIC interrupt controller
   - Basic peripheral integration

3. **SIUL2 GPIO Peripheral**
   - 144-pin GPIO controller
   - Register read/write operations
   - Pin configuration and data registers
   - Interrupt support framework

4. **LPUART Peripheral**
   - Full register set implementation
   - Transmit/receive data handling
   - Baud rate configuration
   - Status and control registers

5. **Demo Firmware**
   - GPIO LED blinking example
   - UART output functionality
   - ARM Cortex-M7 startup code
   - Cross-compilation build system

6. **Test Infrastructure**
   - QTest framework integration
   - Hardware validation tests
   - Build automation scripts

### 🔄 In Progress
- QEMU build system integration (Kconfig)
- Complete peripheral testing
- Firmware validation

### 📋 Planned Components
- FlexCAN controller
- ADC modules
- Timer peripherals
- Clock and reset management
- Power management
- Advanced interrupt handling

## Development Workflow

### 1. Feature Development
```bash
# Start development environment
./scripts/docker-dev.sh start

# Get shell in container
./scripts/docker-dev.sh shell

# Build QEMU
cd build && ninja

# Test new features
make check-qtest-arm
```

### 2. Firmware Testing
```bash
# Build firmware
cd firmware/hello-world && make

# Run in QEMU
cd build && ./qemu-system-arm -M s32k344 -kernel ../firmware/hello-world/hello-world.elf -nographic
```

### 3. Commit Pattern
```bash
# Stage changes
git add .

# Commit with descriptive message
git commit -m "feat(lpuart): implement LPUART peripheral

- Add complete register set implementation
- Support transmit/receive operations
- Integrate with S32K344 machine
- Add QTest validation
"

# Push to feature branch
git push origin feature/s32k344
```

## Key Files and Directories

### Source Code
- `src/s32k344/` - S32K344 implementation (to be moved to qemu-upstream)
- `qemu-upstream/hw/arm/s32k344/` - Integrated QEMU source
- `firmware/` - Test firmware and examples
- `tests/qtest/` - Hardware validation tests

### Configuration
- `Dockerfile` - Development environment setup
- `docker-compose.yml` - Container orchestration
- `scripts/` - Build and development scripts
- `.cursorrules/` - Development rules and guidelines

### Documentation
- `README.md` - Project overview and setup
- `docs/` - Technical documentation
- `tasks/` - Development task tracking

## Quick Start Commands

### First Time Setup
```bash
# Build development environment
./scripts/docker-dev.sh build

# Start container
./scripts/docker-dev.sh start

# Get shell
./scripts/docker-dev.sh shell
```

### Daily Development
```bash
# Build QEMU
cd build && ninja

# Run tests
make check-qtest-arm

# Test firmware
./qemu-system-arm -M s32k344 -kernel ../firmware/hello-world/hello-world.elf -nographic
```

### Debug Mode
```bash
# Start QEMU with GDB server
./qemu-system-arm -M s32k344 -kernel firmware.elf -s -S -nographic

# In another terminal, connect GDB
arm-none-eabi-gdb firmware.elf
(gdb) target remote localhost:1234
(gdb) continue
```

## Success Metrics

### Technical Goals
- ✅ ARM Cortex-M7 emulation working
- ✅ Basic peripherals functional
- ✅ Firmware can run successfully
- ✅ Development environment accessible
- 🔄 Full peripheral set implemented
- 📋 Performance optimization
- 📋 Advanced debugging support

### Development Goals
- ✅ Docker-based development environment
- ✅ Automated build system
- ✅ Test framework integration
- ✅ Documentation and examples
- 🔄 CI/CD pipeline
- �� Release automation 