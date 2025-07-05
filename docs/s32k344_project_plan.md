# Project Plan: QEMU `s32k344-evb` Model and Test Strategy

## 0. Reference Materials to Acquire
- **NXP S32K3xx Reference Manual & Datasheet**  
- **CMSIS Device Header `S32K344.h`**  
- **Renode S32K388 Models & Scripts**  
- **Zephyr MR-CANHUBK3 Board Support**  
- **Ceedling + Unity + CMock**  
- **QEMU Upstream Source (≥ v9.0)**  
- **Windows Build Instructions (MSYS2)**  
- **Docker + QEMU Static User Instructions**  
- *(Optional)* Renode Robot Tests for CAN/Ethernet

## 1. Repository Layout & Branching
```
qemu-s32k344/
 ├─ README-s32k344.md
 ├─ scripts/
 ├─ hw/arm/s32k344_evb.c
 ├─ hw/char/nxp_lpuart.c
 ├─ hw/gpio/s32k_siul2.c
 ├─ hw/timer/s32k_stm.c
 ├─ hw/can/s32k_flexcan.c
 ├─ include/hw/... 
 └─ tests/qtest/...
```
- Create branch `first`.

## 2. Build Environment on Windows 11

### 2.1 Native MSYS2
1. Install MSYS2; update packages.
2. Install mingw-w64 toolchain and QEMU deps.
3. Build QEMU:
   ```bash
   meson setup build-mingw64 --cross-file=scripts/cross-win64-arm.cmake -Dtarget_list=arm-softmmu
   ninja -C build-mingw64
   ```
4. Add `build-mingw64/qemu-system-arm.exe` to PATH.

### 2.2 Dockerized Workflow
- **Base Image**: `ubuntu:24.04` + build deps.
- **Dockerfile**: clone repo, install deps, configure `--target-list=arm-softmmu`.
- Use `qemu-user-static` for ARM binaries.
- **Trade-offs**:
  | Aspect | Native MSYS2 | Docker |
  |---|---|---|
  | Setup time | ~15 min | ~1 GiB download |
  | Reproducibility | Manual | Consistent CI image |
  | Debugging | Full IDE support | Remote container |
  | Performance | Native speed | Slight overhead |
  | Host device access | Direct COM/VCP | `--device` mapping |
- **Recommendation**: MSYS2 for dev; Docker for CI.

## 3. Implementation Work-Packages

| WP | Description | Acceptance Criteria |
|---|---|---|
| **WP-0** | Skeleton `s32k344_evb.c` machine | Boot semihosting "Hello World" |
| **WP-1** | LPUART device (`nxp_lpuart.c`) | `puts()` echoes to QEMU stdout |
| **WP-2** | System Timer Module + NVIC | 1 ms tick qtest passes |
| **WP-3** | GPIO/SIUL2 model | Pin toggle qtest passes |
| **WP-4** | FlexCAN loopback | Zephyr CAN loopback sample PASS |
| **WP-5** | Peripheral stubs (GMAC, extra CAN) | No crash if accessed |
| **WP-6** | Clock tree MC_ME/MC_CGM stubs | Zephyr boots without assert |
| **WP-7** | Dual-core lockstep placeholder | Single core mirrored |
| **WP-8** | Documentation & Upstream PR | `docs/system/arm/s32k344.rst` added |

*Iterate WP-1 → WP-4 in small PRs with green tests.*

## 4. Test Plan

### 4.1 Device-Level QTests
| Device | Test | Criterion |
|---|---|---|
| LPUART | TXDATA write & RXDATA read | Flags correct |
| STM | Compare match IRQ | Single IRQ |
| GPIO | DIR & ODR write/read | State matches |
| FlexCAN | Send & receive frame | No errors |

### 4.2 Firmware-Level Tests (Zephyr)
| Sample | Expected Output |
|---|---|
| blinky | GPIO toggle logs |
| can/loopback | PASS |
| uart_async_api | PASS |
| kernel common | All sub-tests PASS |

### 4.3 Host-Side Logic Tests
- Use Ceedling for HAL-free unit tests.
- `ceedling test:all` in CI.

### 4.4 Renode Parity (Optional)
- Compare Zephyr CAN Robot logs on Renode vs QEMU.

## 5. CI/CD Pipeline Outline
```yaml
jobs:
  build-test:
    runs-on: windows-latest
    steps:
      - uses: actions/checkout@v4
      - name: Build Docker
        run: docker build -t qemu-s32k344 .
      - name: Run QTests
        run: docker run qemu-s32k344 ninja -C build test
      - name: Run Zephyr Tests
        run: docker run qemu-s32k344 scripts/run-zephyr.sh
      - name: Run Ceedling
        run: docker run ceedling:latest ceedling test:all
```

## 6. Milestone Schedule

| Week | Deliverable |
|---|---|
| 1 | WP-0 + build env |
| 2 | WP-1 + UART |
| 3 | WP-2 & WP-3 + blinky |
| 4 | WP-4 + CAN |
| 5 | WP-6 + docs |
| 6 | Buffer & upstream |

## 7. Risks & Mitigations
- **Spec errors**: Validate with Renode & RM errata.
- **Timer skew**: Use cycle-based qtests.
- **CAN complexity**: Begin with loopback.
- **CI flakiness**: Full emulation, no KVM.

---

*End of project plan.*
