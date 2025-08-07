# UART CRC32 (Blocking) — TMS570LC43x

This project demonstrates a **UART-based CRC32 calculation** on the TI LAUNCHXL2-570LC43 board (TMS570LC4357BZWT), using **SCI1 (USB UART)** in **blocking mode**.

---

## Features

- **UART RX:** Receives data via SCI1 (USB UART) at 937500 baud.
- **Idle Timeout:** Waits for 500ms of inactivity to determine end of frame.
- **CRC32 Calculation:** Uses Ethernet (IEEE 802.3) polynomial (0x04C11DB7).
- **Result Reporting:** Prints the CRC32 result back over UART.
- **Max Data Size:** 4096 bytes per frame.
- **Welcome message** is sent at startup.

---

## Configuration

- **Board:** TI LAUNCHXL2-570LC43
- **MCU:** TMS570LC4357BZWT
- **UART Port:** SCI1 (USB UART, FTDI virtual COM port)
- **Baud Rate:** 937500
- **Data Bits:** 8
- **Parity:** None
- **Stop Bits:** 1
- **Flow Control:** None
- **Terminal:** Tera Term or similar

**HALCoGen Setup:**

- Enable SCI1 driver.
- Set baud rate to 937500, 8N1.
- No interrupts or DMA required.

---

## How It Works

- Receives bytes via UART SCI1.
- Waits for 500ms of inactivity (idle timeout) to assume end of transmission.
- Calculates CRC32 over the received buffer using the Ethernet polynomial.
- Sends the calculated CRC32 value back to the host terminal.

---

## Limitations

- **Character loss at high baud rates:**  
  At 937500 baud, Tera Term must be set to **1ms character delay** to avoid data loss.
- No framing or escape sequence for binary or non-ASCII data.
- Not intended for production use.

---

## Usage

1. Open the project in Code Composer Studio.
2. Set up HALCoGen as described above.
3. Build and flash the project to your board.
4. Open Tera Term (or similar) and connect to the FTDI virtual COM port.
5. Set baud rate to 937500, 8N1, no flow control.
6. Send data; after 500ms idle, the CRC32 will be calculated and sent back.

---

## Author

Nirmal Thyvalappil Muraleedharan
