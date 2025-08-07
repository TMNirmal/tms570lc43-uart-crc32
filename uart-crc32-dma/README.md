# UART CRC32 with DMA RX (SCI3) — TMS570LC43x

This project demonstrates **UART CRC32 calculation using DMA RX** on the TI LAUNCHXL2-570LC43 board (TMS570LC4357BZWT), with a 5-second idle timeout and CRC32 reporting.

---

## Features

- **DMA RX:** Receives UART data on SCI3 using DMA (1 byte at a time, re-armed after each byte).
- **Incremental CRC32:** CRC is updated with each received byte.
- **Idle Timeout:** After 5 seconds of no data, CRC32 is sent via UART TX (polling).
- **Welcome message** is sent at startup.
- **Baud rate:** 26042 (set in both HALCoGen and your terminal).
- **No interrupts required for DMA operation.**
- **Tested and working on both new and old boards after disabling cache.**

---

## Critical Notes

- **Cache must be disabled for DMA to work reliably.**
  - In HALCoGen, go to the R5-MPU-PMU tab and **uncheck "Enable Cache"**.
  - If cache is enabled, DMA transfers may silently fail or not update RAM as expected.
- **DMA RX echo is only reliable at lower baud rates (e.g., 26042).**
  - For high-speed UART, use interrupt or polling-based reception.

---

## RTI Interrupt Configuration

- **RTI (Real Time Interrupt) is used to generate a 1ms system tick** for idle timeout detection.
- In HALCoGen:
  - Go to the **RTI** tab and enable **Compare 0** for a 1ms period.
  - In the **VIM Channel 0-31** tab, enable **RTI Compare 0** interrupt (usually Channel 2).
- The RTI interrupt increments a `systemTick` variable, which is used to detect 5 seconds of UART inactivity.

---

## How to Use

1. **Open the project in Code Composer Studio.**
2. **Set the baud rate in HALCoGen and your terminal to 26042.**
3. **Disable "Enable Cache"** in the R5-MPU-PMU tab in HALCoGen.
4. **Enable RTI Compare 0 interrupt** in the VIM tab for 1ms tick.
5. **Build and flash the project to your board.**
6. **Open a terminal (Tera Term, PuTTY, etc.) and connect to the FTDI virtual COM port.**
7. **Send any data**—the CRC32 will be calculated and sent back after 5 seconds of idle.

---

## Reference

- [TI E2E Forum - DMA not working on TMS570LC4357](https://e2e.ti.com/support/microcontrollers/arm-based-microcontrollers-group/arm-based-microcontrollers/f/arm-based-microcontrollers-forum/1542573/tms570lc4357-dma-not-working-on-tms570lc4357-no-transfer-no-errors-all-software-steps-correct)
- [TI E2E: DMA can't read some data correctly (cache issue)](https://e2e.ti.com/support/microcontrollers/arm-based-microcontrollers-group/arm-based-microcontrollers/f/arm-based-microcontrollers-forum/1200732/tmdx570lc43hdk-dma-can-t-read-some-data-correctly-trying-to-sci-tx)

---

**Author:**  
Nirmal Thyvalappil Muraleedharan
