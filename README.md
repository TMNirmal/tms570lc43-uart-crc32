# UART CRC32 Applications for TMS570LC4357

This repository contains a series of UART-based CRC32 applications developed for the **TI LAUNCHXL2-570LC43** development board featuring the **TMS570LC4357BZWT** microcontroller.  
The goal is to reliably receive high-speed UART data, detect idle timeouts, and compute a **CRC32 checksum using the Ethernet (IEEE 802.3) polynomial**.

---

## ⚠️ Critical Notes for DMA Operation

- **Cache must be disabled for DMA to work reliably.**

  - In HALCoGen, go to the R5-MPU-PMU tab and **uncheck "Enable Cache"**.
  - If cache is enabled, DMA transfers may silently fail or not update RAM as expected.
  - This applies to both new and old boards.

- **DMA UART RX echo is only reliable at lower baud rates (e.g., 26042).**

  - At higher baud rates, character loss or data corruption may occur due to CPU/DMA limitations and cache effects.
  - For high-speed UART (e.g., 937500 baud), use interrupt-based or polling-based UART reception.

- **Interrupt and polling-based UART reception are reliable at high baud rates.**
  - Use these methods for applications requiring maximum UART throughput.

---

## Hardware & Tools

- **Board:** TI LAUNCHXL2-570LC43
- **MCU:** TMS570LC4357BZWT
- **Compiler:** TI ARM Compiler v20.2.7.LTS
- **UART Ports:** SCI1 (USB-to-PC), SCI3 (external USB-TTL via CH340E)
- **Baud Rate:** 937500 (for interrupt/polling), 26042 (for DMA)
- **UART Config:** 2 Stop Bits, No Parity
- **Max Data Size:** 4096 bytes (except large file variant)
- **Idle Timeout:** 5000 ms
- **CRC Algorithm:** Ethernet CRC32 (IEEE 802.3 Polynomial)

---

## 🗂 Project Structure

### 📁 uart-crc32

**Objective:** Basic implementation using UART SCI1 (USB) with CRC32 calculation after idle timeout.

- Blocking UART reception.
- Uses HALCoGen configuration.
- CRC calculated after 5000ms idle time.
- Simple implementation, but has **character loss at high baud rates**.

**Limitations:**  
At 937500 baud, Tera Term must be set to **1ms character delay** to avoid data loss.

---

### 📁 uart-crc32-interrupt

**Objective:** Improve reliability with interrupt-based UART reception and RTI-based idle timeout detection.

- **SCI1:** Enabled RX interrupt (VIM Channel 13).
- **RTI:** Enabled with default config for 1ms tick using Compare 0 (VIM Channel 2).
- **Receive Buffer Size:** 6144 bytes.
- `sciReceive()` used to initiate buffered interrupt-driven reception.

**Advantages:**  
No data loss at 937500 baud, even without Tera Term character delay.

**Notes:**

- `HL_notification.c` default handlers removed or redirected to user `main.c`.
- RTI compare period tuned to detect 5000ms idle.

---

### 📁 uart-crc32-interrupt-largefiles

**Objective:** Enhance interrupt-based implementation to support **incremental CRC32 calculation** for large files.

- Based on `uart-crc32-interrupt`.
- Supports CRC32 computation over continuous large data streams using an **incremental algorithm**.
- Resets and finalizes CRC after **5 seconds of idle timeout**.
- Ideal for real-world scenarios where data exceeds buffer limits.

---

### 📁 uart-crc32-dma

**Objective:** Implement UART reception using DMA for robust, non-blocking, high-speed transfer.

- **DMA RX echo is now working** after disabling cache in the R5-MPU-PMU tab in HALCoGen.
- SCI3 (external USB-TTL via CH340E) is used for DMA, as SCI1 does not support DMA.
- DMA setup is performed manually via `HL_sys_dma.c`.
- **DMA Parameter RAM** does not need to be explicitly mapped in the linker file for most projects, but best practice is to reserve `.dmaRAM` at `0xFFF80000`.

**Critical Note:**

> **DMA will not work reliably if cache is enabled.**  
> You must uncheck "Enable Cache" in the R5-MPU-PMU tab in HALCoGen.  
> This resolves the "DMA not working" issue on both new and old boards.

**Baud Rate Note:**

> **DMA UART RX echo is only reliable at lower baud rates (e.g., 26042).**  
> For high-speed UART, use interrupt or polling-based reception.

**Reference:**

- [TI E2E Forum - DMA not working on TMS570LC4357](https://e2e.ti.com/support/microcontrollers/arm-based-microcontrollers-group/arm-based-microcontrollers/f/arm-based-microcontrollers-forum/1542573/tms570lc4357-dma-not-working-on-tms570lc4357-no-transfer-no-errors-all-software-steps-correct)
- [TI E2E: DMA can't read some data correctly (cache issue)](https://e2e.ti.com/support/microcontrollers/arm-based-microcontrollers-group/arm-based-microcontrollers/f/arm-based-microcontrollers-forum/1200732/tmdx570lc43hdk-dma-can-t-read-some-data-correctly-trying-to-sci-tx)

---

## 📌 Summary

| Project                         | Method                            | Reliable @ 937500 baud | Notes                                                     |
| ------------------------------- | --------------------------------- | ---------------------- | --------------------------------------------------------- |
| uart-crc32                      | Blocking                          | ❌ (char loss)         | Needs 1ms char delay in Tera Term                         |
| uart-crc32-interrupt            | Interrupt + RTI                   | ✅                     | Reliable; idle-based framing                              |
| uart-crc32-interrupt-largefiles | Interrupt + RTI + Incremental CRC | ✅                     | Supports large files with incremental CRC over 5s timeout |
| uart-crc32-dma                  | DMA RX (cache disabled)           | ⚠️ (Low baud only)     | **Cache must be disabled for DMA to work**                |

---

## 🔧 Future Work

- Benchmark CPU load: interrupt vs. DMA.
- Add CRC32 verification for received binary files.
- Optionally extend support for ASCII/binary data mode.
- Add UART transmit path for CRC acknowledgment.

---

## 📜 License

This repository is provided for educational and reference purposes. Not intended for direct production use. Please refer to each file header for any specific restrictions.

---

## Reference

For detailed discussion, troubleshooting steps, and TI community support, see:

- [TI E2E Forum: FAQ - TMS570LC4357: Examples and Demos available for Hercules Controllers](https://e2e.ti.com/support/microcontrollers/arm-based-microcontrollers-group/arm-based-microcontrollers/f/arm-based-microcontrollers-forum/1275733/faq-tms570lc4357-examples-and-demos-available-for-hercules-controllers-e-g-tms570x-rm57x-and-rm46x-etc)
- [TI E2E: DMA can't read some data correctly (cache issue)](https://e2e.ti.com/support/microcontrollers/arm-based-microcontrollers-group/arm-based-microcontrollers/f/arm-based-microcontrollers-forum/1200732/tmdx570lc43hdk-dma-can-t-read-some-data-correctly-trying-to-sci-tx)

---

**If you use or adapt this code, please cite the TI E2E forum threads and this repository.**
