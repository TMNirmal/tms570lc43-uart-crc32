# DMA-UART Example for TMS570LC43x

## Overview

This project demonstrates **UART echo using DMA RX** on the TMS570LC43x (e.g., LAUNCHXL2-570LC43) Hercules microcontroller.  
It is designed for robust, high-speed UART communication using DMA for receive and polling for transmit.

- **DMA RX:** Each received byte is transferred from SCI3 RX to RAM using DMA.
- **Polling TX:** Each received byte is echoed back using polling (sciSendByte).
- **Welcome message** is sent at startup.
- **Baud rate:** 26042 (set in both HALCoGen and your terminal).
- **No interrupts required for DMA operation.**

## Key Configuration Notes

- **DMA Parameter RAM** does not need to be explicitly mapped in the linker file for most projects, but best practice is to reserve `.dmaRAM` at `0xFFF80000`.
- **SCI3** is routed to the FTDI USB-to-UART interface on the LaunchPad, so you can use the virtual COM port on your PC.
- **Cache must be disabled** in the R5-MPU-PMU tab in HALCoGen for DMA to work reliably.
  - Go to **R5-MPU-PMU** tab and **uncheck "Enable Cache"**.
  - If cache is enabled, DMA and CPU may see different data, causing DMA to silently fail or not update RAM as expected.
- **MPU (Memory Protection Unit)** can remain enabled.

## How to Use

1. **Open the project in Code Composer Studio.**
2. **Set the baud rate in HALCoGen and your terminal to 26042.**
3. **Ensure "Enable Cache" is unchecked in the R5-MPU-PMU tab in HALCoGen.**
4. **Build and flash the project to your board.**
5. **Open a terminal (Tera Term, PuTTY, etc.) and connect to the FTDI virtual COM port.**
6. **Type any character**—it will be echoed back immediately.

## File Structure

- `uart_dma_main.c` — Main application code (DMA RX, polling TX, welcome message)
- `HL_sys_dma.c/h` — DMA driver (HALCoGen generated)
- `HL_sci.c/h` — SCI driver (HALCoGen generated)
- `HL_system.c/h` — System initialization (HALCoGen generated)
- `HL_sys_link.cmd` — Linker file (ensure `.dmaRAM` is reserved if needed)

## Troubleshooting

- If DMA does not work, **double-check that cache is disabled** in the R5-MPU-PMU tab.
- Make sure `systemInit()` is called at the start of `main()`.
- Ensure `.dmaRAM` is not used for any variables.
- If using a custom linker file, reserve `.dmaRAM` at `0xFFF80000`.

## Credits

- Author: Nirmal Thyvalappil Muraleedharan
- Based on TI Hercules HALCoGen examples and community support.

## Reference

For detailed discussion, troubleshooting steps, and TI community support, see:  
[TI E2E Forum: FAQ - TMS570LC4357: Examples and Demos available for Hercules Controllers](https://e2e.ti.com/support/microcontrollers/arm-based-microcontrollers-group/arm-based-microcontrollers/f/arm-based-microcontrollers-forum/1275733/faq-tms570lc4357-examples-and-demos-available-for-hercules-controllers-e-g-tms570x-rm57x-and-rm46x-etc)

This project and its DMA configuration were validated with help from the TI E2E community and official example projects.
