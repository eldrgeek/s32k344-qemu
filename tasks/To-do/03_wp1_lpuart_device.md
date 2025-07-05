# WP-1: LPUART Device Implementation

## Test-Driven Development Setup
- [ ] **Done** - [ ] **Reviewed** - Create unit tests for LPUART register operations
- [ ] **Done** - [ ] **Reviewed** - Write qtest for LPUART TXDATA/RXDATA operations
- [ ] **Done** - [ ] **Reviewed** - Set up CMock for LPUART peripheral mocking
- [ ] **Done** - [ ] **Reviewed** - Create test cases for UART flag handling

## LPUART Device Structure
- [ ] **Done** - [ ] **Reviewed** - Create `hw/char/nxp_lpuart.c` device file
- [ ] **Done** - [ ] **Reviewed** - Define LPUART register map according to S32K344 spec
- [ ] **Done** - [ ] **Reviewed** - Set up LPUART device state structure
- [ ] **Done** - [ ] **Reviewed** - Create LPUART device class and instance initialization

## Register Implementation
- [ ] **Done** - [ ] **Reviewed** - Implement VERID (Version ID) register (read-only)
- [ ] **Done** - [ ] **Reviewed** - Implement PARAM (Parameter) register (read-only)
- [ ] **Done** - [ ] **Reviewed** - Implement GLOBAL (Global) register
- [ ] **Done** - [ ] **Reviewed** - Implement PINCFG (Pin Configuration) register
- [ ] **Done** - [ ] **Reviewed** - Implement BAUD (Baud Rate) register
- [ ] **Done** - [ ] **Reviewed** - Implement STAT (Status) register with flags
- [ ] **Done** - [ ] **Reviewed** - Implement CTRL (Control) register
- [ ] **Done** - [ ] **Reviewed** - Implement DATA (Data) register for TX/RX
- [ ] **Done** - [ ] **Reviewed** - Implement MATCH (Match Address) register
- [ ] **Done** - [ ] **Reviewed** - Implement MODIR (Modem IrDA) register
- [ ] **Done** - [ ] **Reviewed** - Implement FIFO (FIFO) register
- [ ] **Done** - [ ] **Reviewed** - Implement WATER (Watermark) register

## FIFO Implementation
- [ ] **Done** - [ ] **Reviewed** - Implement TX FIFO with configurable depth
- [ ] **Done** - [ ] **Reviewed** - Implement RX FIFO with configurable depth
- [ ] **Done** - [ ] **Reviewed** - Set up FIFO watermark handling
- [ ] **Done** - [ ] **Reviewed** - Implement FIFO overflow/underflow detection
- [ ] **Done** - [ ] **Reviewed** - Configure FIFO interrupt generation

## Interrupt Handling
- [ ] **Done** - [ ] **Reviewed** - Implement TX interrupt generation
- [ ] **Done** - [ ] **Reviewed** - Implement RX interrupt generation
- [ ] **Done** - [ ] **Reviewed** - Set up error interrupt handling (parity, framing, noise)
- [ ] **Done** - [ ] **Reviewed** - Implement idle line detection interrupt
- [ ] **Done** - [ ] **Reviewed** - Configure interrupt mask handling

## Character Backend Integration
- [ ] **Done** - [ ] **Reviewed** - Connect LPUART to QEMU character backend
- [ ] **Done** - [ ] **Reviewed** - Implement character transmission to host
- [ ] **Done** - [ ] **Reviewed** - Set up character reception from host
- [ ] **Done** - [ ] **Reviewed** - Handle character backend events (connect/disconnect)

## Baud Rate and Timing
- [ ] **Done** - [ ] **Reviewed** - Implement baud rate calculation from BAUD register
- [ ] **Done** - [ ] **Reviewed** - Set up character timing simulation
- [ ] **Done** - [ ] **Reviewed** - Configure oversampling ratio handling
- [ ] **Done** - [ ] **Reviewed** - Implement stop bit configuration

## Error Handling
- [ ] **Done** - [ ] **Reviewed** - Implement parity error detection
- [ ] **Done** - [ ] **Reviewed** - Set up framing error handling
- [ ] **Done** - [ ] **Reviewed** - Implement noise error detection
- [ ] **Done** - [ ] **Reviewed** - Configure overrun error handling

## Power Management
- [ ] **Done** - [ ] **Reviewed** - Implement doze mode support
- [ ] **Done** - [ ] **Reviewed** - Set up stop mode handling
- [ ] **Done** - [ ] **Reviewed** - Configure wake-up from stop mode

## Memory Map Integration
- [ ] **Done** - [ ] **Reviewed** - Map LPUART0 to 0x40328000
- [ ] **Done** - [ ] **Reviewed** - Map LPUART1 to 0x40329000
- [ ] **Done** - [ ] **Reviewed** - Map LPUART2 to 0x4032A000
- [ ] **Done** - [ ] **Reviewed** - Set up LPUART instance differentiation

## Acceptance Testing
- [ ] **Done** - [ ] **Reviewed** - Test `puts()` echoes to QEMU stdout
- [ ] **Done** - [ ] **Reviewed** - Verify TXDATA write operations
- [ ] **Done** - [ ] **Reviewed** - Test RXDATA read operations
- [ ] **Done** - [ ] **Reviewed** - Validate interrupt flag handling
- [ ] **Done** - [ ] **Reviewed** - Test FIFO operations
- [ ] **Done** - [ ] **Reviewed** - Run comprehensive qtest suite for LPUART 