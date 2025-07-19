/**
 ******************************************************************************
 * @file    uart_crc32_interrupt_main.c
 * @brief   UART CRC32 Calculation with 5-Second Window (SCI Interrupt Driven)
 *
 * @details
 *   - Receives UART data at 937,500 baud, 2 stop bits, no parity.
 *   - Calculates CRC32 (Ethernet/IEEE 802.3 polynomial) of all data received
 *     during each 5-second window.
 *   - If no data is received in a window, the last calculated CRC is resent.
 *   - Uses interrupt-driven reception with sciReceive() (1 byte at a time)
 *     for robust, variable-length data handling.
 *   - CRC is sent via UART at the end of each 5-second interval.
 *
 * @hardware
 *   - MCU: TMS570LS12x HDK (e.g., TMS570LS1227)
 *   - UART Port: SCI2 (USB UART)
 *   - Connection: PC via XDS100v2 emulator's virtual COM port
 *   - Terminal: Tera Term or similar for UART communication
 *
 * @usage
 *   - Configure HALCoGen:
 *       * Enable SCI2 RX interrupt (VIM Channel for SCI2/LIN2)
 *       * Set SCI2 baud rate to 937500, 2 stop bits, no parity
 *       * Enable RTI Compare 0 interrupt for 5s interval
 *       * Set RTI Compare 0 Period to 5000 ms
 *   - Compile and flash to board.
 *   - Send data from PC terminal; CRC will be calculated and sent back every 5s.
 *
 * @note
 *   - Maximum data length per window: 6144 bytes
 *   - CRC32 uses Ethernet polynomial (0x04C11DB7), initial value 0xFFFFFFFF,
 *     final XOR 0xFFFFFFFF, input/output reflected.
 *   - sciNotification() and rtiNotification() are implemented in this file
 *     (remove from HL_notification.c).
 *
 * @author  Nirmal Thyvalappil Muraleedharan
 * @date    18.07.2025
 ******************************************************************************
 */

/* USER CODE BEGIN (0) */

/* Include Files */
#include "HL_sys_common.h"
#include "HL_sci.h"
#include "HL_rti.h"
#include "HL_system.h"
#include "string.h"
#include <stdio.h>

/* USER CODE END */

/* USER CODE BEGIN (1) */
#define MAX_DATA_LEN 6144U
#define POLYNOMIAL 0x04C11DB7
#define INIT_CRC 0xFFFFFFFF
#define FINAL_XOR 0xFFFFFFFF
#define IDLE_TIMEOUT_MS 5000U
#define SCI_RECEIVE_LENGTH  1U
#define USB_UART sciREG1

uint8_t rx_data[MAX_DATA_LEN];
/* Byte for sciReceive function */
volatile uint8_t temp_byte = 0;
volatile uint32_t rx_count = 0;
volatile uint32_t systemTick = 0;

uint32_t crc_value = 0xFFFFFFFF;
/* 8 hex digits + null terminator */
char crc_formatBuffer[9] = {0};
/* USER CODE END */

/* USER CODE BEGIN (2) */
void sciDisplayText(sciBASE_t *sci, unsigned char *text);
uint8_t reflect8(uint8_t data);
uint32_t reflect32(uint32_t data);
uint32_t calc_crc32(uint8_t *data, uint32_t len);

void debug_receivedData(sciBASE_t *sci, uint8_t *data, uint32_t len);
void debug_byteCount(sciBASE_t *sci, uint32_t len);
/* USER CODE END */

int main(void)
{
/* USER CODE BEGIN (3) */

    /* initialize sci/sci-lin */
    sciInit();
    /* Init RTI (for 1ms system tick) */
    rtiInit();

    /* Enable RTI compare interrupt and start counter */
    rtiEnableNotification(rtiREG1, rtiNOTIFICATION_COMPARE0);
    rtiStartCounter(rtiREG1, rtiCOUNTER_BLOCK0);

    /* Enable UART RX interrupt */
    sciEnableNotification(USB_UART, SCI_RX_INT);

    /* Enable global interrupts */
    _enable_interrupt_();

    /* Optional: Welcome message */
    sciDisplayText(USB_UART, (uint8_t *)"Sequential CRC Calculator with SCI1 Block Rx Interrupt Started...\r\n");

    /* Start first receive sciReceive Designed for Interrupt Reception */
    sciReceive(USB_UART, SCI_RECEIVE_LENGTH, (uint8_t *)&temp_byte);


    while (1)
    {
        /* Do Nothing */
    }

/* USER CODE END */
}


/* USER CODE BEGIN (4) */

/* Interrupt handler (called automatically by HALCoGen if enabled) */
#pragma WEAK(sciNotification)
void sciNotification(sciBASE_t *sci, uint32 flags)
{
    /* temp_byte contains the received byte */
    if (rx_count < MAX_DATA_LEN)
    {
        rx_data[rx_count] = temp_byte;
        rx_count++;
    }
    /* Start next receive */
    sciReceive(sci, SCI_RECEIVE_LENGTH, (uint8_t *)&temp_byte);
}

/* RTI 1ms Tick Interrupt */
#pragma WEAK(rtiNotification)
void rtiNotification(rtiBASE_t *rtiREG, uint32 notification)
{
    if (rtiREG == rtiREG1 && notification == rtiNOTIFICATION_COMPARE0) {
        systemTick++;
    }

    /* Reached IDLE_TIMEOUT_MS and no data received */
    if((IDLE_TIMEOUT_MS <= systemTick) && (0U == rx_count))
    {
        /* Formatting CRC and sending via UART */
        sciDisplayText(USB_UART, (uint8_t *)"No Data Received, Last Calculated CRC in Hex is : 0x");
        sciDisplayText(USB_UART, (unsigned char *)crc_formatBuffer);
        sciDisplayText(USB_UART, (uint8_t *)"\r\n");
        /* Reset the rti timer */
        systemTick = 0;
    }
    /* Reached IDLE_TIMEOUT_MS and some data received */
    else if((IDLE_TIMEOUT_MS <= systemTick) && (0U != rx_count))
    {
        /* Compute CRC */
        crc_value = calc_crc32(rx_data, rx_count);
        /* Formatting CRC and sending via UART */
        sciDisplayText(USB_UART, (uint8_t *)"Updated CRC in Hex is : 0x");
        sprintf(crc_formatBuffer, "%08X", crc_value);  /* Format CRC as 8-digit upper case hex */
        sciDisplayText(USB_UART, (unsigned char *)crc_formatBuffer);
        sciDisplayText(USB_UART, (uint8_t *)"\r\n");
        /* Reset the rti timer */
        systemTick = 0;
        /* Reset the rti timer */
        rx_count = 0;
    }
    else
    {
        /* Do Nothing */
    }
}

/* Function used to send data through SCI buffer */
void sciDisplayText(sciBASE_t *sci, unsigned char *text)
{
    uint32 length = strlen((const char *)text);

    while(length--)
    {
        while ((sci->FLR & 0x4) == 4); /* wait until busy */
        sciSendByte(sci,*text++);      /* send out text   */
    };
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
void debug_receivedData(sciBASE_t *sci, uint8_t *data, uint32_t len)
{
    char hexByte[6];
    uint32_t i;
    sciDisplayText(sci, (uint8_t *)"\r\nData Received is : ");
    for (i = 0; i < len; i++) {
        sprintf(hexByte, " %02X", data[i]);
        sciDisplayText(sci, (uint8_t *)hexByte);
    }
    sciDisplayText(sci, (uint8_t *)"\r\n");
}

/*
 * Debugging purpose only
 *
 */
void debug_byteCount(sciBASE_t *sci, uint32_t len)
{
    char msg[64];
    sprintf(msg, "\r\nBytes received: %lu\r\n", len);
    sciDisplayText(sci, (uint8_t *)msg);
}
/* USER CODE END */
