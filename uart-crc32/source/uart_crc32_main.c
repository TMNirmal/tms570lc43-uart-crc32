/**
 * @example uart_crc_main.c
 *
 * This example demonstrates a UART-based CRC32 calculation on the TMS570LS12x device.
 * The application receives data via SCI2 (USB UART) at 937500 baud, waits for a 500ms
 * idle timeout to determine end of frame, calculates CRC32 (Ethernet polynomial 0x04C11DB7)
 * over the received data, and prints the CRC result back over UART.
 *
 * Baud Rate     : 937500
 * Data Bits     : 8
 * Parity        : None
 * Stop Bits     : 1
 * Flow Control  : None
 *
 * Hardware Setup:
 * - Development board: TMS570LS12x HDK (e.g., TMS570LS1227)
 * - UART Port: SCI2 (USB UART)
 * - Connected to PC via XDS100v2 emulator's virtual COM port.
 * - Terminal Software (e.g., Tera Term) used to send and receive UART data.
 *
 * Code Generation Setup:
 *
 * @b Step @b 1:
 * Create a new HALCoGen project for TMS570LS12x.
 *
 * @b Step @b 2:
 * Enable SCI2 driver only.
 *
 * @b Step @b 3:
 * Configure SCI2:
 *   - Baud Rate: 937500
 *   - Data Format: 8N1 (8 data bits, No parity, 1 stop bit)
 *
 * @b Step @b 4:
 * Generate driver code.
 *
 * @b Step @b 5:
 * Replace the content of `sys_main.c` with this example source file.
 *
 * Functional Overview:
 * - Receives bytes via UART SCI2.
 * - Waits 500ms of inactivity (idle timeout) to assume end of transmission.
 * - Calculates CRC32 over the received buffer using Ethernet polynomial.
 * - Sends calculated CRC32 value back to host terminal.
 *
 * Limitations:
 * - No framing or escape sequence for binary or non-ASCII data.
 * - No integration with external protocol layers.
 *
 * @note: This example is meant for local development and testing, and is not intended for production use.
 *
 * Author: Nirmal T M
 * Date  : July 2025
 */


/* USER CODE BEGIN (0) */
/* USER CODE END */

/* Include Files */

#include "HL_sys_common.h"
#include "HL_system.h"
#include "string.h"
#include <stdio.h>

/* USER CODE BEGIN (1) */
#include "HL_sci.h"

/* Maximum data size of single crc Calculation */
#define MAX_DATA_LEN 4096
#define POLYNOMIAL 0x04C11DB7
#define INIT_CRC 0xFFFFFFFF
#define FINAL_XOR 0xFFFFFFFF

uint8_t rx_data[MAX_DATA_LEN];
uint32_t rx_count = 0;


void sciDisplayText(sciBASE_t *sci, unsigned char *text);
void delay_ms(uint32_t ms);
uint8_t reflect8(uint8_t data);
uint32_t reflect32(uint32_t data);
uint32_t calc_crc32(uint8_t *data, uint32_t len);
void debug_receivedData(uint8_t *data, uint32_t len);
void debug_byteCount(uint32_t len);

#define USB_UART sciREG1
/* USER CODE END */

/** @fn void main(void)
*   @brief Application main function
*
*/

/* USER CODE BEGIN (2) */
/* USER CODE END */


void main(void)
{
/* USER CODE BEGIN (3) */

    uint32_t crc_value = 0xFFFFFFFF;
    char crc_formatBuffer[9];  // 8 hex digits + null terminator
    uint32_t idle_counter = 0;

    /* initialize sci/sci-lin */
    sciInit();
    /* Optional: Welcome message */
    sciDisplayText(USB_UART, (uint8_t *)"Sequential CRC Calculator Started\r\n");

    while (1)
    {
        /* Clear buffer count */
        rx_count = 0;
        idle_counter = 0;

        /* Optional: send message */
        sciDisplayText(USB_UART, (uint8_t *)"UART Ready. Send data...\r\n");

        // Wait for data (timeout after idle)
        while (idle_counter < 500 && rx_count < MAX_DATA_LEN) // 500ms idle
        {
            if (sciIsRxReady(USB_UART))
            {
                rx_data[rx_count++] = sciReceiveByte(USB_UART);
                idle_counter = 0;  // Reset idle timer on new byte
            }
            else
            {
                delay_ms(1);
                idle_counter++;
            }
        }

        /* Only calculate if new data received */
        if(0 < rx_count)
        {
            /* Compute CRC */
            crc_value = calc_crc32(rx_data, rx_count);

            /* Formatting CRC and sending via UART */
            sciDisplayText(USB_UART, (uint8_t *)"Updated CRC in Hex is : 0x");
            sprintf(crc_formatBuffer, "%08X", crc_value);  /* Format CRC as 8-digit upper case hex */
            sciDisplayText(USB_UART, (unsigned char *)crc_formatBuffer);
            sciDisplayText(USB_UART, (uint8_t *)"\r\n");
        }
        else
        {
            /* Formatting CRC and sending via UART */
            sciDisplayText(USB_UART, (uint8_t *)"No Data Received, Last Calculated CRC in Hex is : 0x");
            sciDisplayText(USB_UART, (unsigned char *)crc_formatBuffer);
            sciDisplayText(USB_UART, (uint8_t *)"\r\n");
        }
    }

/* USER CODE END */
}

/* USER CODE BEGIN (4) */
void sciDisplayText(sciBASE_t *sci, unsigned char *text)
{
    uint32 length = strlen((const char *)text);

    while(length--)
    {
        while ((sci->FLR & 0x4) == 4); /* wait until busy */
        sciSendByte(sci,*text++);      /* send out text   */
    };
}
/*  Delay Function
 *  Considering 160Mhz system clock
 *  delay in ms
 */

void delay_ms(uint32_t ms)
{
    uint32_t i;
    while (ms--)
    {
        // Inner loop for 1 millisecond delay at 90 MHz
        for (i = 0; i < 90000; i++)
        {
            __asm(" nop");
        }
    }
}

/* Supporting Function for calculating CRC */
uint8_t reflect8(uint8_t data)
{
    uint8_t res = 0;
    int i;
    for (i = 0; i < 8; i++)
        res |= ((data >> i) & 1) << (7 - i);
    return res;
}

/* Supporting Function for calculating CRC */
uint32_t reflect32(uint32_t data)
{
    uint32_t res = 0;
    int i;
    for (i = 0; i < 32; i++)
        res |= ((data >> i) & 1) << (31 - i);
    return res;
}

/* Calculating CRC */
uint32_t calc_crc32(uint8_t *data, uint32_t len)
{
    uint32_t i, crc = INIT_CRC;
    int j;
    for (i = 0; i < len; i++) {
        crc ^= ((uint32_t)reflect8(data[i])) << 24;
        for (j = 0; j < 8; j++)
            crc = (crc & 0x80000000) ? (crc << 1) ^ POLYNOMIAL : (crc << 1);
    }
    return reflect32(crc) ^ FINAL_XOR;
}

/*
 * Debugging purpose only
 *
 */
void debug_receivedData(uint8_t *data, uint32_t len)
{
    char hexByte[6];
    uint32_t i;
    sciDisplayText(USB_UART, (uint8_t *)"\r\nData Received is : ");
    for (i = 0; i < len; i++) {
        sprintf(hexByte, " %02X", data[i]);
        sciDisplayText(USB_UART, (uint8_t *)hexByte);
    }
    sciDisplayText(USB_UART, (uint8_t *)"\r\n");
}

/*
 * Debugging purpose only
 *
 */
void debug_byteCount(uint32_t len)
{
    char msg[64];
    sprintf(msg, "\r\nBytes received: %lu\r\n", len);
    sciDisplayText(USB_UART, (uint8_t *)msg);
}

/* USER CODE END */

