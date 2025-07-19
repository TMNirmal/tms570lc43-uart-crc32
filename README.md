# UART CRC32 Applications for TMS570LC4357

This repository contains a series of UART-based CRC32 applications developed for the **TI LAUNCHXL2-570LC43** development board featuring the **TMS570LC4357BZWT** microcontroller. The objective is to reliably receive high-speed UART data, detect idle timeouts, and compute a **CRC32 checksum using the Ethernet (IEEE 802.3) polynomial**.

## Hardware & Tools

- **Board**: TI LAUNCHXL2-570LC43
- **MCU**: TMS570LC4357BZWT
- **Compiler**: TI ARM Compiler v20.2.7.LTS
- **UART Port**: SCI1 (USB-to-PC), SCI3 (external USB-TTL via CH340E)
- **Baud Rate**: 937500
- **UART Config**: 2 Stop Bits, No Parity
- **Max Data Size**: 4096 bytes
- **Idle Timeout**: 5000 ms
- **CRC Algorithm**: Ethernet CRC32 (IEEE 802.3 Polynomial)

---

## 🗂 Project Structure

### 📁 uart-crc32

**Objective**: Basic implementation using UART SCI1 (USB) with CRC32 calculation after idle timeout.

**Features**:

- Blocking UART reception.
- Uses HALCoGen configuration.
- CRC calculated after 5000ms idle time.
- Simple implementation, but has **character loss at high baud rates**.

**Limitations**:

- At 937500 baud, Tera Term must be set to **1ms character delay** to avoid data loss.

---

### 📁 uart-crc32-interrupt

**Objective**: Improve reliability with interrupt-based UART reception and RTI-based idle timeout detection.

**Key HALCoGen Configurations**:

- **SCI1**: Enabled RX interrupt (VIM Channel 13).
- **RTI**: Enabled with default config for 1ms tick using Compare 0 (VIM Channel 2).
- **Receive Buffer Size**: 6144 bytes.
- `sciReceive()` used to initiate buffered interrupt-driven reception.

**Advantages**:

- No data loss at 937500 baud, even without Tera Term character delay.
- Efficient for moderate use cases.

**Notes**:

- `HL_notification.c` default handlers removed or redirected to user `main.c`.
- RTI compare period tuned to detect 5000ms idle.

---

### 📁 uart-crc32-dma (WIP)

**Objective**: Implement UART reception using DMA for robust, non-blocking, high-speed transfer.

**Challenges**:

- **HALCoGen lacks DMA support** for TMS570LC43x SCI modules.
- SCI1 **cannot be used with DMA**, requiring external UART via SCI3/SCI4.
- Used external USB-TTL module (CH340E) for SCI3.
- DMA setup performed manually via `HL_sys_dma.c`.

**Current Status**:

- Interrupt and polling for SCI3 working fine.
- DMA initialization appears correct but **no data is transferred**.
- Suspected hardware or undocumented limitations.
- TI community post for support:  
  [TI E2E Forum - DMA not working on TMS570LC4357](https://e2e.ti.com/support/microcontrollers/arm-based-microcontrollers-group/arm-based-microcontrollers/f/arm-based-microcontrollers-forum/1542573/tms570lc4357-dma-not-working-on-tms570lc4357-no-transfer-no-errors-all-software-steps-correct)

---

## 📌 Summary

| Project              | Method          | Reliable @ 937500 baud  | Notes                                                 |
| -------------------- | --------------- | ----------------------- | ----------------------------------------------------- |
| uart-crc32           | Blocking        | ❌ (char loss)          | Needs 1ms char delay in Tera Term                     |
| uart-crc32-interrupt | Interrupt + RTI | ✅                      | Works without delay; best for low-moderate throughput |
| uart-crc32-dma       | DMA (WIP)       | ⚠️ (Not yet functional) | Pending TI response, hardware issue suspected         |

---

## 🔧 Future Work

- Resolve DMA issues and finalize `uart-crc32-dma`.
- Benchmark CPU load: interrupt vs. DMA.
- Add CRC32 verification for received binary files.
- Optionally extend support for ASCII/binary data mode.

---

## 📜 License

This repository is provided for educational and reference purposes. Not intended for direct production use. Please refer to each file header for any specific restrictions.
