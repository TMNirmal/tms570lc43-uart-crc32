#include "HL_sys_common.h"
#include "HL_sci.h"
#include "HL_sys_dma.h"
#include "HL_system.h"

#define RX_BUFFER_SIZE 8
#define DMA_SCI3_RX    DMA_REQ30

uint8_t rx_buffer[RX_BUFFER_SIZE];
g_dmaCTRL g_dmaCTRLPKT_RX;

void setup_dma_rx(void);

int main(void)
{
    systemInit();      // Ensure all peripherals are powered up!
    sciInit();         // Initialize SCI3
    dmaEnable();       // Enable DMA module

    // DMA RX setup
    dmaReqAssign(DMA_CH0, DMA_SCI3_RX);
    setup_dma_rx();

    while (1)
    {
        // Wait for DMA RX block complete
        if (dmaGetInterruptStatus(DMA_CH0, BTC) == TRUE)
        {
            // Echo received data back (polling TX)
            for (int i = 0; i < RX_BUFFER_SIZE; i++)
                sciSendByte(sciREG3, rx_buffer[i]);

            // Clear BTC flag and re-arm DMA for next block
            dmaREG->BTCFLAG = (1U << DMA_CH0);
            setup_dma_rx();
        }
    }
}

void setup_dma_rx(void)
{
    g_dmaCTRLPKT_RX.SADD      = (uint32)&(sciREG3->RD);   // Source: SCI3 RX register
    g_dmaCTRLPKT_RX.DADD      = (uint32)rx_buffer;        // Destination: rx_buffer in RAM
    g_dmaCTRLPKT_RX.CHCTRL    = 0;
    g_dmaCTRLPKT_RX.FRCNT     = RX_BUFFER_SIZE;           // Number of bytes to receive
    g_dmaCTRLPKT_RX.ELCNT     = 1;
    g_dmaCTRLPKT_RX.ELDOFFSET = 0;
    g_dmaCTRLPKT_RX.ELSOFFSET = 0;
    g_dmaCTRLPKT_RX.FRDOFFSET = 0;
    g_dmaCTRLPKT_RX.FRSOFFSET = 0;
    g_dmaCTRLPKT_RX.PORTASGN  = PORTB_READ_PORTA_WRITE;
    g_dmaCTRLPKT_RX.RDSIZE    = ACCESS_8_BIT;
    g_dmaCTRLPKT_RX.WRSIZE    = ACCESS_8_BIT;
    g_dmaCTRLPKT_RX.TTYPE     = FRAME_TRANSFER;
    g_dmaCTRLPKT_RX.ADDMODERD = ADDR_FIXED;               // Source fixed (SCI3->RD)
    g_dmaCTRLPKT_RX.ADDMODEWR = ADDR_INC1;                // Dest increment (buffer)
    g_dmaCTRLPKT_RX.AUTOINIT  = AUTOINIT_OFF;

    dmaSetCtrlPacket(DMA_CH0, g_dmaCTRLPKT_RX);
    dmaSetChEnable(DMA_CH0, DMA_HW);                      // Hardware trigger
    sciREG3->SETINT |= (1 << 17);                         // Enable RX DMA request
}
