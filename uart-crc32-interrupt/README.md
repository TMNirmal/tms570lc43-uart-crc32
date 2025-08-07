# UART CRC32 (Interrupt) — TMS570LC43x

This project demonstrates a **UART-based CRC32 calculation using interrupt-driven reception** on the TI LAUNCHXL2-570LC43 board (TMS570LC4357BZWT), using **SCI1 (USB UART)**.

---

## Features

- **UART RX:** Receives data via SCI1 (USB UART) at 937500 baud using interrupt-driven reception.
- **Idle Timeout:** Waits for 5000ms of inactivity (using RTI) to determine end of frame.
- **CRC32 Calculation:** Uses Ethernet (IEEE 802.3) polynomial (0x04C11DB7).
- **Result Reporting:** Prints the CRC32 result back over UART after each 5-second window.
- **Max Data Size:** 6144 bytes per frame.
- **Welcome message** is sent at startup.

---

## Configuration

- **Board:** TI LAUNCHXL2-570LC43
- **MCU:** TMS570LC4357BZWT
- **UART Port:** SCI1 (USB UART, FTDI virtual COM port)
- **Baud Rate:** 937500
- **Data Bits:** 8
- **Parity:** None
- **Stop Bits:** 2
- **Flow Control:** None
- **Terminal:** Tera Term or similar

**HALCoGen Setup:**

- Enable SCI1 driver.
- Set baud rate to 937500, 8N2.
- Enable SCI RX interrupt (VIM Channel 13, LIN1 High).
- Enable RTI with Compare 0 for 1ms tick (VIM Channel 2).
- Set RTI Compare 0 Period to 9375 for 1ms at 9.375 MHz.
- No PINMUX needed for SCI1.
- Remove or modify default handlers in `HL_notification.c` (move to your main file).

---

## How It Works

- Receives bytes via UART SCI1 using interrupt-driven reception (`sciReceive()`).
- Waits for 5000ms of inactivity (idle timeout) to assume end of transmission.
- Calculates CRC32 over the received buffer using the Ethernet polynomial.
- Sends the calculated CRC32 value back to the host terminal.
- If no data is received in a window, the last CRC is resent.

---

## Advantages

- **No character loss at high baud rates** (937500) even with bulk data.
- No need for Tera Term character delay.
- Efficient and robust for high-speed UART.

---

## Usage

1. Open the project in Code Composer Studio.
2. Set up HALCoGen as described above.
3. Build and flash the project to your board.
4. Open Tera Term (or similar) and connect to the FTDI virtual COM port.
5. Set baud rate to 937500, 8N2, no flow control.
6. Send data; after 5 seconds idle, the CRC32 will be calculated and sent back.

---

## Author

Nirmal Thyvalappil Muraleedharan
