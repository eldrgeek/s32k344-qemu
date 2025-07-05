---
name: "S32K344 QEMU Development"
description: "Core development rules for S32K344 QEMU project"
applies_to:
  - "src/**/*"
  - "qemu-upstream/hw/arm/s32k344/**/*"
  - "firmware/**/*"
  - "tests/**/*"
contexts:
  - "development"
  - "coding"
  - "implementation"
priority: 100
---

# S32K344 QEMU Development Rules

## Core Development Principles

### Implementation Flow
1. **Write** → **Test** → **Commit** → **Document**
2. Make atomic commits with clear, descriptive messages
3. Use feature branches for experimental work
4. Keep main branch always buildable
5. Test in Docker environment before committing

### Code Quality Standards
- **ARM Cortex-M7 Focus**: All implementations must be compatible with ARM Cortex-M7 architecture
- **QEMU Standards**: Follow QEMU coding conventions and object model patterns
- **Memory Safety**: Use proper bounds checking and validation
- **Register Mapping**: Ensure all peripheral registers match S32K344 documentation

### Commit Message Format
```
<type>(<scope>): <description>

feat(lpuart): implement transmit data register
fix(siul2): correct GPIO interrupt handling  
test(qtest): add S32K344 machine boot validation
docs(readme): update build instructions
```

### Safe Command Patterns
- Git operations: `add`, `commit`, `status`, `log`, `diff`
- Docker commands within project containers
- Build operations: `make`, `ninja`, `meson`
- File operations within project directory
- Read-only system queries

### Integration Requirements
- All peripherals must integrate with QEMU object model
- Memory-mapped I/O must use proper QEMU memory regions
- Interrupts must route through ARM NVIC
- Clock and reset handling must follow QEMU patterns

### Testing Strategy
- Unit tests for each peripheral component
- Integration tests for complete machine
- Firmware validation with real embedded code
- Cross-compilation verification with ARM toolchain 