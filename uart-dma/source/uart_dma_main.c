/**
 ******************************************************************************
 * @file    uart_dma_main.c
 * @brief   SCI3 UART DMA RX Echo Example for TMS570LC43x
 *
 * @details
 *   - Receives UART data on SCI3 using DMA (1 byte at a time).
 *   - Echoes each received character back using polling TX.
 *   - Prints a welcome message at startup.
 *   - Designed for LAUNCHXL2-570LC43 board, using FTDI USB-to-UART.
 *   - Baud rate: 26042 (set in HALCoGen and terminal).
 *   - DMA channel, request, and control packet are set up for SCI3 RX.
 *   - DMA is re-armed for every byte, so echo is continuous.
 *
 * @note
 *   - DMA Parameter RAM is not explicitly mapped in the linker file, but DMA works as long as nothing else uses 0xFFF80000.
 *   - For best practice, reserve .dmaRAM in the linker file.
 *   - For reliable operation, use baud rates <= 50000.
 *
 * @author  Nirmal Thyvalappil Muraleedharan
 * @date    07.08.2025
 ******************************************************************************
 */
#include "HL_sys_common.h"
#include "HL_system.h"
#include "HL_sys_dma.h"
#include "HL_sci.h"
#include <string.h>

#define DMA_SCI3_RX  DMA_REQ30
#define SCI_SET_RX_DMA      (1<<17)
#define SCI_SET_RX_DMA_ALL  (1<<18)

#if ((__little_endian__ == 1) || (__LITTLE_ENDIAN__ == 1))
#define SCI3_RX_ADDR ((uint32_t)(&(sciREG3->RD)))
#else
#define SCI3_RX_ADDR ((uint32_t)(&(sciREG3->RD)) + 3)
#endif

uint8_t rx_byte = 0;
g_dmaCTRL g_dmaCTRLPKT;

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

/**
 * @brief  Main function: Initializes SCI3 and DMA, prints welcome message,
 *         and continuously echoes received characters using DMA RX and polling TX.
 */
void main(void)
{
    sciInit();

    // Wait for SCI3 to be ready
    while (((sciREG3->FLR & SCI_TX_INT) == 0U) || ((sciREG3->FLR & 0x4) == 0x4))
    {
        /* Wait */
    }

    // Send welcome message
    sci3DisplayText((unsigned char *)"SCI3 DMA RX Echo Ready!\r\n");

    setup_dma();

    while(1)
    {
        start_reception();

        // Wait for byte
        while(dmaGetInterruptStatus(DMA_CH1, BTC) != TRUE);

        // Echo immediately
        if(rx_byte != 0)
        {
            sciSendByte(sciREG3, rx_byte);
        }
    }
}
