# S32K344 QEMU Project - Fast Track Demo

## Project Summary
This project demonstrates a complete LLM-driven development workflow by implementing a QEMU machine model for the NXP S32K344 ARM Cortex-M7 microcontroller. **Primary goal: Fastest path to working demo on macOS/Docker, with Windows conversion as Phase 2.**

## Fast Track Strategy (Phase 1)

### Core Demo Components
1. **Docker Environment** - Complete development environment in containers
2. **Basic Machine (WP-0)** - Minimal S32K344-EVB with semihosting
3. **LPUART (WP-1)** - Console output for "Hello World"
4. **GPIO (WP-3)** - Pin control for "blinky" LED demo
5. **Essential Testing** - Prove the workflow works end-to-end

### Deferred to Phase 2 (Windows Conversion)
- MSYS2/Windows native builds
- Complex timer implementation (WP-2)
- FlexCAN implementation (WP-4)
- Comprehensive peripheral stubs (WP-5/6/7)
- Full documentation suite (WP-8)
- Extensive firmware testing (WP-9)

## Revised Task Priority

### Week 1: Foundation (Critical Path)
- [ ] **Done** - [ ] **Reviewed** - Docker environment setup (`01_setup_and_automation.md`)
- [ ] **Done** - [ ] **Reviewed** - Basic machine skeleton (`02_wp0_skeleton_machine.md`)

### Week 2: Core Functionality
- [ ] **Done** - [ ] **Reviewed** - LPUART for output (`03_wp1_lpuart_device.md`)
- [ ] **Done** - [ ] **Reviewed** - GPIO for blinky (`05_wp3_gpio_siul2.md`)

### Week 3: Demo Validation
- [ ] **Done** - [ ] **Reviewed** - Zephyr "Hello World" working
- [ ] **Done** - [ ] **Reviewed** - Zephyr "blinky" working
- [ ] **Done** - [ ] **Reviewed** - Demo recording and documentation

## Fast Track Acceptance Criteria

### Minimum Viable Demo
- [ ] **Done** - [ ] **Reviewed** - Boot semihosting "Hello World" (WP-0)
- [ ] **Done** - [ ] **Reviewed** - Console output via LPUART (WP-1)
- [ ] **Done** - [ ] **Reviewed** - GPIO LED blinky working (WP-3)
- [ ] **Done** - [ ] **Reviewed** - All development in Docker containers
- [ ] **Done** - [ ] **Reviewed** - Automated build and test pipeline
- [ ] **Done** - [ ] **Reviewed** - LLM-driven workflow demonstrated

### Success Metrics
- [ ] **Done** - [ ] **Reviewed** - Zephyr "Hello World" sample runs
- [ ] **Done** - [ ] **Reviewed** - Zephyr "blinky" sample shows GPIO toggles
- [ ] **Done** - [ ] **Reviewed** - Complete workflow from blank repo to working demo
- [ ] **Done** - [ ] **Reviewed** - Repeatable in clean Docker environment

## Development Environment

### macOS Host + Docker Containers
- **Host**: macOS development tools, Playwright MCP for web automation
- **Docker**: Ubuntu 24.04 + ARM cross-compilation + QEMU build environment
- **Integration**: Volume mounts for source code, automated build/test

### LLM-Driven Workflow
- **Code Generation**: LLM implements all C code for peripherals
- **Web Automation**: Playwright MCP handles reference material downloads
- **Test Automation**: LLM creates and runs all validation tests
- **Documentation**: Automated generation of technical documentation

## Fast Track Task Files

### Phase 1 (Demo) - Priority Order
1. **`01_setup_and_automation.md`** - Docker environment (CRITICAL)
2. **`02_wp0_skeleton_machine.md`** - Basic machine (CRITICAL)
3. **`03_wp1_lpuart_device.md`** - Console output (CRITICAL)
4. **`05_wp3_gpio_siul2.md`** - GPIO for blinky (CRITICAL)
5. **`09_firmware_testing.md`** - Essential Zephyr tests (CRITICAL)

### Phase 2 (Windows + Full Features) - Deferred
- `04_wp2_system_timer_nvic.md` - Complex timer implementation
- `06_wp4_flexcan.md` - CAN bus implementation  
- `07_wp5_wp6_wp7_peripheral_stubs.md` - Additional peripherals
- `08_wp8_documentation_upstream.md` - Full documentation

## Getting Started (Fast Track)
1. **START HERE**: `01_setup_and_automation.md` - Docker setup
2. **Follow Critical Path**: Focus only on Phase 1 tasks
3. **Test Early**: Validate each component immediately
4. **Demo First**: Get working example before adding complexity
5. **Document Success**: Capture the workflow for Phase 2 conversion

## Success Definition
**Demo Success**: Show a complete LLM-driven development workflow that takes a blank repository and produces a working QEMU machine model running real firmware (Zephyr) in Docker containers on macOS.

---
*Fast Track: ~50 critical tasks | Full Project: 400+ tasks*
*Target: 3 weeks to working demo | Phase 2: Windows conversion + full features* 