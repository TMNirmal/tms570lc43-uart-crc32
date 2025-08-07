/**
 ******************************************************************************
 * @file    uart_crc32_dma_main.c
 * @brief   UART CRC32 Calculation with 5-Second Window (DMA RX, SCI3)
 *
 * @details
 *   - Receives UART data on SCI3 using DMA (1 byte at a time, re-armed after each byte).
 *   - Buffers all received bytes and updates CRC32 incrementally.
 *   - After 5 seconds of idle, sends the CRC32 value via UART TX (polling).
 *   - If no data is received in a window, the last calculated CRC is resent.
 *   - Prints a welcome message at startup.
 *   - Designed for LAUNCHXL2-570LC43 board, using FTDI USB-to-UART.
 *   - Baud rate: 26042 (set in HALCoGen and terminal).
 *   - DMA channel, request, and control packet are set up for SCI3 RX.
 *   - DMA is re-armed for every byte, so reception is continuous.
 *
 * @note
 *   - **Cache must be disabled** in the R5-MPU-PMU tab for DMA to work.
 *   - For best practice, reserve .dmaRAM in the linker file.
 *   - For reliable operation, use baud rates <= 50000.
 *
 * @author  Nirmal Thyvalappil Muraleedharan
 * @date    07.08.2025
 ******************************************************************************
 */
/* USER CODE BEGIN (0) */
#include "HL_sys_common.h"
#include "HL_system.h"
#include "HL_sys_dma.h"
#include "HL_sci.h"
#include "HL_rti.h"
#include "string.h"
#include <stdio.h>

/* USER CODE END */

/* USER CODE BEGIN (1) */
#define DMA_SCI3_RX  DMA_REQ30
#define SCI_SET_RX_DMA      (1<<17)
#define SCI_SET_RX_DMA_ALL  (1<<18)

#if ((__little_endian__ == 1) || (__LITTLE_ENDIAN__ == 1))
#define SCI3_RX_ADDR ((uint32_t)(&(sciREG3->RD)))
#else
#define SCI3_RX_ADDR ((uint32_t)(&(sciREG3->RD)) + 3)
#endif

#define POLYNOMIAL 0x04C11DB7
#define INIT_CRC 0xFFFFFFFF
#define FINAL_XOR 0xFFFFFFFF
#define IDLE_TIMEOUT_MS 5000U
#define SCI_RECEIVE_LENGTH  1U
#define USB_UART sciREG3

/* Byte for sciReceive function */
volatile uint8_t rx_byte = 0;
volatile uint32_t rx_count = 0;
volatile uint32_t systemTick = 0;

uint32_t crc_value = INIT_CRC;
/* 8 hex digits + null terminator */
char crc_formatBuffer[9] = {0};

g_dmaCTRL g_dmaCTRLPKT;

/* USER CODE END */

/* USER CODE BEGIN (2) */
void sciDisplayText(sciBASE_t *sci, unsigned char *text);
uint8_t reflect8(uint8_t data);
uint32_t reflect32(uint32_t data);
uint32_t calc_crc32(uint8_t *data, uint32_t len);
uint32_t update_crc32(uint32_t crc, uint8_t data);

void debug_receivedData(sciBASE_t *sci, uint8_t *data, uint32_t len);
void debug_byteCount(sciBASE_t *sci, uint32_t len);
void sci3DisplayText(unsigned char *text);
void setup_dma(void);
void start_reception(void);
/* USER CODE END */


/**
 * @brief  Main function: Initializes SCI3 and DMA, prints welcome message,
 *         and continuously echoes received characters using DMA RX and polling TX.
 */
void main(void)
{
/* USER CODE BEGIN (3) */

    /* initialize sci/sci-lin */
    sciInit();

    /* Wait for SCI3 to be ready */
    while (((sciREG3->FLR & SCI_TX_INT) == 0U) || ((sciREG3->FLR & 0x4) == 0x4))
    {
        /* Wait */
    }

    /* Init RTI (for 1ms system tick) */
    rtiInit();
    /* Enable RTI compare interrupt and start counter */
    rtiEnableNotification(rtiREG1, rtiNOTIFICATION_COMPARE0);
    rtiStartCounter(rtiREG1, rtiCOUNTER_BLOCK0);
    /* Enable global interrupts */
    _enable_interrupt_();
    /* Optional: Welcome message */
    sciDisplayText(USB_UART, (uint8_t *)"Sequential CRC Calculator with SCI1 Block Rx DMA Started...\r\n");

    /* Setup DMA for SCI3 */
    setup_dma();

    /* Start the receive */
    start_reception();

    while(1)
    {

        /* Wait for byte */
        if(dmaGetInterruptStatus(DMA_CH1, BTC) == TRUE)
        {
            /* Update CRC with each received byte */
            crc_value = update_crc32(crc_value, rx_byte);

            /* Increment the received byte count */
            rx_count++;

            /* Clear the BTC flag immediately! */
            dmaREG->BTCFLAG = (1U << DMA_CH1);

            /* Start next receive */
            start_reception();
        }
    }
/* USER CODE END */
}


/* USER CODE BEGIN (4) */

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
        debug_byteCount(USB_UART, rx_count);
        /* Finalize the CRC */
        crc_value = reflect32(crc_value) ^ FINAL_XOR;
        /* Formatting CRC and sending via UART */
        sciDisplayText(USB_UART, (uint8_t *)"Updated CRC in Hex is : 0x");
        sprintf(crc_formatBuffer, "%08X", crc_value);  /* Format CRC as 8-digit upper case hex */
        sciDisplayText(USB_UART, (unsigned char *)crc_formatBuffer);
        sciDisplayText(USB_UART, (uint8_t *)"\r\n");
        /* Reset the rti timer */
        systemTick = 0;
        /* Reset the rti timer */
        rx_count = 0;
        /* Reset the crc buffer */
        crc_value = INIT_CRC;
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

/* Calculating CRC using incremental method */
uint32_t update_crc32(uint32_t crc, uint8_t data)
{
    crc ^= ((uint32_t)reflect8(data)) << 24;
    for (int j = 0; j < 8; j++)
        crc = (crc & 0x80000000) ? (crc << 1) ^ POLYNOMIAL : (crc << 1);
    return crc;
}

/**
 * @brief  Sends a null-terminated string over SCI3 using polling.
 * @param  text Pointer to the string to send.
 */
void sci3DisplayText(unsigned char *text)
{
    uint32 length = strlen((const char *)text);
    while(length--)
    {
        while ((sciREG3->FLR & 0x4) == 0x4); // Wait until not busy
        sciSendByte(sciREG3, *text++);
    }
}

/**
 * @brief  Sets up DMA channel 1 for single-byte SCI3 RX.
 *         Configures the DMA control packet and enables the hardware trigger.
 */
void setup_dma(void)
{
    dmaReqAssign(DMA_CH1, DMA_SCI3_RX);

    g_dmaCTRLPKT.SADD      = SCI3_RX_ADDR;
    g_dmaCTRLPKT.DADD      = (uint32_t)&rx_byte;
    g_dmaCTRLPKT.CHCTRL    = 0;
    g_dmaCTRLPKT.FRCNT     = 1;
    g_dmaCTRLPKT.ELCNT     = 1;
    g_dmaCTRLPKT.ELDOFFSET = 0;
    g_dmaCTRLPKT.ELSOFFSET = 0;
    g_dmaCTRLPKT.FRDOFFSET = 0;
    g_dmaCTRLPKT.FRSOFFSET = 0;
    g_dmaCTRLPKT.PORTASGN  = PORTB_READ_PORTA_WRITE;
    g_dmaCTRLPKT.RDSIZE    = ACCESS_8_BIT;
    g_dmaCTRLPKT.WRSIZE    = ACCESS_8_BIT;
    g_dmaCTRLPKT.TTYPE     = FRAME_TRANSFER;
    g_dmaCTRLPKT.ADDMODERD = ADDR_FIXED;
    g_dmaCTRLPKT.ADDMODEWR = ADDR_FIXED;
    g_dmaCTRLPKT.AUTOINIT  = AUTOINIT_OFF;

    dmaEnable();
    sciREG3->SETINT |= SCI_SET_RX_DMA | SCI_SET_RX_DMA_ALL;
}

/**
 * @brief  Arms DMA channel 1 for the next single-byte SCI3 RX transfer.
 *         Resets the RX byte and enables the channel.
 */
void start_reception(void)
{
    rx_byte = 0;
    dmaSetCtrlPacket(DMA_CH1, g_dmaCTRLPKT);
    dmaSetChEnable(DMA_CH1, DMA_HW);
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

