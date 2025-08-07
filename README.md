# UART CRC32 Applications for TMS570LC4357

This repository contains a series of UART-based CRC32 applications for the **TI LAUNCHXL2-570LC43** development board (TMS570LC4357BZWT).  
The goal is to reliably receive high-speed UART data, detect idle timeouts, and compute a **CRC32 checksum using the Ethernet (IEEE 802.3) polynomial**.

---

## Hardware & Tools

- **Board:** TI LAUNCHXL2-570LC43
- **MCU:** TMS570LC4357BZWT
- **Compiler:** TI ARM Compiler v20.2.7.LTS
- **UART Ports:** SCI1 (USB UART), SCI3 (external USB-TTL via CH340E)
- **Baud Rate:** 937500 (SCI1, interrupt/polling), 26042 (DMA, SCI3)
- **UART Config:** 2 Stop Bits, No Parity
- **Max Data Size:** 4096 bytes (except large file variant)
- **Idle Timeout:** 5000 ms
- **CRC Algorithm:** Ethernet CRC32 (IEEE 802.3 Polynomial)

---

## Project Evolution & Key Learnings

### 📁 uart-crc32

- **Blocking UART reception** on SCI1 (USB UART).
- CRC calculated after 5000ms idle.
- **Limitation:** At 937500 baud, Tera Term must be set to **1ms character delay** to avoid data loss.
- **Lesson:** High baud rates cause character loss in blocking mode.

---

### 📁 uart-crc32-interrupt

- **Interrupt-based UART reception** with RTI-based idle timeout.
- **HALCoGen config:**
  - Enable SCI1, set baudrate 937500, 2 stop bits, no parity.
  - Enable SCI RX interrupt (VIM Channel 13).
  - Enable RTI (default config, VIM Channel 2).
- **Buffer size:** 6144 bytes.
- **No character loss at high baud rates** (937500) even with bulk data.
- **Notes:**
  - Use `sciReceive()` for bulk interrupt-driven reception.
  - No PINMUX needed for SCI1.
  - Remove or modify default handlers in `HL_notification.c`.
  - RTI Compare 0 Period can be tuned in HALCoGen for 1ms tick.
- **Lesson:** Interrupt-based UART is robust for high-speed, bulk data.

---

### 📁 uart-dma

- **DMA-based UART RX echo** on SCI3 (external USB-TTL via CH340E).
- **HALCoGen does not support DMA for SCI1** on TMS570LC43x, so SCI3 is used.
- **Manual DMA configuration** required.
- **Critical finding:**
  - **DMA will not work unless "Enable Cache" is unchecked** in the R5-MPU-PMU tab in HALCoGen.
  - After disabling cache, DMA works on both new and old boards.
- **Limitation:**
  - **DMA RX echo is only reliable at lower baud rates (e.g., 26042).**
  - At higher baud rates, character loss occurs.
- **Lesson:** DMA and CPU cache can cause data coherency issues; always disable cache for DMA on this device.

---

### 📁 uart-crc32-dma

- **Current project:**
  - DMA-based UART RX on SCI3, with CRC32 calculation after 5 seconds of idle.
  - Baud rate set to 26042 for reliable operation.
  - Uses RTI for 1ms tick and idle timeout detection.
  - Welcome message at startup.
  - **DMA is re-armed for every byte, so reception is continuous.**
- **Lesson:** For continuous, reliable DMA UART RX and CRC32, use lower baud rates and disable cache.

---

## Problems Faced & Solutions

- **Character loss at high baud rates** in blocking mode:
  - Solution: Use interrupt-based UART or DMA (with cache disabled).
- **DMA not working:**
  - Solution: Disable cache in HALCoGen (R5-MPU-PMU tab).
- **DMA not supported for SCI1:**
  - Solution: Use SCI3 with external USB-TTL adapter.
- **DMA RX echo only reliable at low baud rates:**
  - Solution: Set baud rate to 26042 for DMA-based projects.

---

## Reference

- [TI E2E Forum - DMA not working on TMS570LC4357](https://e2e.ti.com/support/microcontrollers/arm-based-microcontrollers-group/arm-based-microcontrollers/f/arm-based-microcontrollers-forum/1542573/tms570lc4357-dma-not-working-on-tms570lc4357-no-transfer-no-errors-all-software-steps-correct)
- [TI E2E: DMA can't read some data correctly (cache issue)](https://e2e.ti.com/support/microcontrollers/arm-based-microcontrollers-group/arm-based-microcontrollers/f/arm-based-microcontrollers-forum/1200732/tmdx570lc43hdk-dma-can-t-read-some-data-correctly-trying-to-sci-tx)

---

## 📌 Summary Table

| Project                                                              | Method                            | Reliable @ 937500 baud | Notes                                                                            |
| -------------------------------------------------------------------- | --------------------------------- | ---------------------- | -------------------------------------------------------------------------------- |
| [uart-crc32](./uart-crc32)                                           | Blocking                          | ❌ (char loss)         | Needs 1ms char delay in Tera Term                                                |
| [uart-crc32-interrupt](./uart-crc32-interrupt)                       | Interrupt + RTI                   | ✅                     | Reliable; idle-based framing                                                     |
| [uart-crc32-interrupt-largefiles](./uart-crc32-interrupt-largefiles) | Interrupt + RTI + Incremental CRC | ✅                     | Supports large files with incremental CRC over 5s timeout                        |
| [uart-dma](./uart-dma)                                               | DMA RX (cache disabled)           | ⚠️ (Low baud only)     | **Cache must be disabled for DMA to work. Reliable at ≤26042 baud.**             |
| [uart-crc32-dma](./uart-crc32-dma)                                   | DMA RX + CRC32 (cache disabled)   | ⚠️ (Low baud only)     | Continuous stream, 5s idle CRC, cache must be disabled, reliable at ≤26042 baud. |

---

## License

This repository is provided for educational and reference purposes. Not intended for direct production use. Please refer to each file header for any specific restrictions.

---

**If you use or adapt this code, please cite the TI E2E forum threads and this repository.**
