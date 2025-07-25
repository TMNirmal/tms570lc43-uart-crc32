#include "HL_sys_common.h"
#include "HL_sys_dma.h"
#include "HL_system.h"

#define D_SIZE 500

uint16 TX_DATA[D_SIZE];         // Transmit buffer in RAM
uint16 RX_DATA[D_SIZE] = {0};   // Receive buffer in RAM

g_dmaCTRL g_dmaCTRLPKT;         // DMA control packet

void loadDataPattern(uint32 psize, uint16* pptr);

int main(void)
{
    systemInit();    // Power up all peripherals
    dmaEnable();     // Enable DMA module

    loadDataPattern(D_SIZE, TX_DATA);

    // Configure DMA control packet for memory-to-memory transfer
    g_dmaCTRLPKT.SADD      = (uint32)TX_DATA;
    g_dmaCTRLPKT.DADD      = (uint32)RX_DATA;
    g_dmaCTRLPKT.CHCTRL    = 0;
    g_dmaCTRLPKT.FRCNT     = 1;             // 1 frame
    g_dmaCTRLPKT.ELCNT     = D_SIZE;        // D_SIZE elements
    g_dmaCTRLPKT.ELDOFFSET = 0;
    g_dmaCTRLPKT.ELSOFFSET = 0;
    g_dmaCTRLPKT.FRDOFFSET = 0;
    g_dmaCTRLPKT.FRSOFFSET = 0;
    g_dmaCTRLPKT.PORTASGN  = PORTA_READ_PORTA_WRITE;
    g_dmaCTRLPKT.RDSIZE    = ACCESS_16_BIT;
    g_dmaCTRLPKT.WRSIZE    = ACCESS_16_BIT;
    g_dmaCTRLPKT.TTYPE     = FRAME_TRANSFER;
    g_dmaCTRLPKT.ADDMODERD = ADDR_INC1;
    g_dmaCTRLPKT.ADDMODEWR = ADDR_INC1;
    g_dmaCTRLPKT.AUTOINIT  = AUTOINIT_OFF;

    dmaSetCtrlPacket(DMA_CH0, g_dmaCTRLPKT);

    // Software trigger for memory-to-memory transfer
    dmaSetChEnable(DMA_CH0, DMA_SW);

    // Wait for DMA transfer to complete
    while (dmaGetInterruptStatus(DMA_CH0, BTC) != TRUE);

    // At this point, RX_DATA should be a copy of TX_DATA
    while (1);
}

void loadDataPattern(uint32 psize, uint16* pptr)
{
    for (uint32 count = 0; count < psize; count++)
    {
        pptr[count] = count;
    }
}
