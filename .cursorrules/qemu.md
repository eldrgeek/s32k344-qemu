---
name: "QEMU Development"
description: "QEMU-specific development rules and patterns"
applies_to:
  - "qemu-upstream/**/*"
  - "src/s32k344/**/*"
  - "tests/qtest/**/*"
contexts:
  - "qemu"
  - "emulation"
  - "arm-cortex-m7"
priority: 95
---

# QEMU Development Rules

## S32K344 Architecture Requirements

### ARM Cortex-M7 Compliance
- Use ARMv7E-M instruction set architecture
- Implement proper memory protection unit (MPU) support
- Support single-precision floating-point operations
- Implement cache and branch prediction features

### Memory Layout (Fixed)
```c
#define S32K344_FLASH_BASE    0x00400000  // 4MB Flash
#define S32K344_FLASH_SIZE    0x00400000
#define S32K344_SRAM_BASE     0x20000000  // 512KB SRAM
#define S32K344_SRAM_SIZE     0x00080000
#define S32K344_PERIPH_BASE   0x40000000  // Peripheral base
```

### Peripheral Integration Patterns

#### QEMU Object Model
```c
// Always use QEMU object model
#define TYPE_S32K344_DEVICE "s32k344-device"
OBJECT_DECLARE_SIMPLE_TYPE(S32K344DeviceState, S32K344_DEVICE)

// State structure pattern
struct S32K344DeviceState {
    SysBusDevice parent_obj;
    MemoryRegion iomem;
    qemu_irq irq;
    // Device-specific state
};
```

#### Register Implementation
```c
// Use proper register read/write handlers
static uint64_t s32k344_device_read(void *opaque, hwaddr addr, unsigned size)
{
    S32K344DeviceState *s = S32K344_DEVICE(opaque);
    uint32_t value = 0;
    
    switch (addr) {
    case DEVICE_REG_OFFSET:
        value = s->register_value;
        break;
    default:
        qemu_log_mask(LOG_GUEST_ERROR, 
                     "s32k344_device_read: Bad offset 0x%" HWADDR_PRIx "\n", addr);
        break;
    }
    
    return value;
}
```

## Build System Integration

### Meson Build Files
Always create `meson.build` for new peripherals:
```meson
s32k344_ss = ss.source_set()
s32k344_ss.add(files(
  'device_name.c',
))
hw_arch += {'s32k344': s32k344_ss}
```

### Kconfig Integration
Add configuration options in `hw/arm/Kconfig`:
```
config S32K344_DEVICE
    bool
    depends on ARM
    select ARM_V7M
    select SERIAL
```

### Default Configuration
Enable in `configs/devices/arm-softmmu/default.mak`:
```
CONFIG_S32K344_DEVICE=y
```

## Testing Requirements

### QTest Integration
All peripherals must have QTest coverage:
```c
static void test_s32k344_device_registers(void)
{
    QTestState *qts = qtest_init("-M s32k344");
    
    // Test register read/write
    qtest_writel(qts, DEVICE_BASE + REG_OFFSET, 0x12345678);
    uint32_t val = qtest_readl(qts, DEVICE_BASE + REG_OFFSET);
    g_assert_cmpuint(val, ==, 0x12345678);
    
    qtest_quit(qts);
}
```

### Firmware Validation
Test with real ARM firmware:
```c
// firmware/hello-world/main.c pattern
void device_init(void) {
    // Configure device registers
    *((volatile uint32_t*)(DEVICE_BASE + CONFIG_REG)) = CONFIG_VALUE;
    
    // Verify configuration
    uint32_t status = *((volatile uint32_t*)(DEVICE_BASE + STATUS_REG));
    assert(status & EXPECTED_STATUS_BIT);
}
```

## Safe QEMU Operations

### Build Operations (Always Safe)
```bash
# Configure build
cd build && ../qemu-upstream/configure --target-list=arm-softmmu --enable-debug

# Build with ninja
ninja

# Run tests
make check-qtest-arm
```

### Execution and Testing
```bash
# Run S32K344 machine
./qemu-system-arm -M s32k344 -nographic

# Run with firmware
./qemu-system-arm -M s32k344 -kernel ../firmware/hello-world/hello-world.elf -nographic

# Debug mode
./qemu-system-arm -M s32k344 -s -S -kernel firmware.elf -nographic
```

### QEMU Monitor Commands
```bash
# In QEMU monitor (Ctrl+A, C)
(qemu) info registers
(qemu) info mtree
(qemu) info qtree
(qemu) x/10i $pc
```

## Code Quality Standards

### Error Handling
- Always use `qemu_log_mask(LOG_GUEST_ERROR, ...)` for invalid accesses
- Use `assert()` for programming errors
- Provide meaningful error messages

### Memory Management
- Use `memory_region_init_io()` for device registers
- Proper cleanup in device finalization
- Use `g_malloc0()` for zero-initialized allocations

### Interrupt Handling
- Connect interrupts properly to ARM NVIC
- Use `qemu_irq_raise()` and `qemu_irq_lower()` correctly
- Document interrupt numbers and priorities 