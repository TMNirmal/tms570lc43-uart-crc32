# tms570lc43-uart-crc32
UART-based CRC32 calculator for the TI LAUNCHXL2-570LC43 board. Receives bytes over UART, waits for 500ms idle timeout, then computes and transmits CRC32 using Ethernet polynomial (0x04C11DB7). Ported from TMS570LS12x implementation. Tested with USB-UART (e.g., Tera Term).
