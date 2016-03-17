// Copyright (c) 2007, 2013 Micronet Ltd. All rights reserved.
// by Vladimir Zatulovsky
//
/*
================================================================================
*             Texas Instruments OMAP(TM) Platform Software
* (c) Copyright Texas Instruments, Incorporated. All Rights Reserved.
*
* Use of this software is controlled by the terms and conditions found
* in the license agreement under which this software has been supplied.
*
================================================================================
*/

//
//  File:  serial.c
//
#include <bsp.h>

//------------------------------------------------------------------------------
//
//  Function:  OEMDebugInit
//
//  Initialize debug serial port
//
BOOL OEMDebugInit()
{
    OMAP_UART_REGS *pUartRegs;

    pUartRegs = OALPAtoUA(OMAP_UART3_REGS_PA);

    // Reset UART & wait until it completes
    OUTREG8(&pUartRegs->SYSC, UART_SYSC_RST);
    while ((INREG8(&pUartRegs->SYSS) & UART_SYSS_RST_DONE) == 0);
    // Set baud rate
    OUTREG8(&pUartRegs->LCR, UART_LCR_DLAB);
    OUTREG8(&pUartRegs->DLL, BSP_UART_DSIUDLL);
    OUTREG8(&pUartRegs->DLH, BSP_UART_DSIUDLH);
    // 8 bit, 1 stop bit, no parity
    OUTREG8(&pUartRegs->LCR, BSP_UART_LCR);
    // Enable FIFO
    OUTREG8(&pUartRegs->FCR, UART_FCR_FIFO_EN);
    // Pool
    OUTREG8(&pUartRegs->IER, 0);
    // Set DTR/RTS signals
    OUTREG8(&pUartRegs->MCR, UART_MCR_DTR|UART_MCR_RTS);
    // Configuration complete so select UART 16x mode
    OUTREG8(&pUartRegs->MDR1, UART_MDR1_UART16);

    return TRUE;
}

//------------------------------------------------------------------------------
//
//  Function:  OEMDebugDeinit
//
//  Close debug serial port
//
VOID OEMDebugDeinit()
{
    OMAP_UART_REGS *pUartRegs;

    pUartRegs = OALPAtoUA(OMAP_UART3_REGS_PA);

    // Wait while FIFO isn't empty
    while ((INREG8(&pUartRegs->LSR) & UART_LSR_TX_SR_E) == 0);
}

//------------------------------------------------------------------------------
//
//  Function:  OEMWriteDebugByte
//
//  Write byte to debug serial port.
//
VOID OEMWriteDebugByte(UINT8 ch)
{
    OMAP_UART_REGS *pUartRegs = OALPAtoUA(OMAP_UART3_REGS_PA);

    // Wait while FIFO is full
    while ((INREG8(&pUartRegs->SSR) & UART_SSR_TX_FIFO_FULL) != 0);
    // Send byte
    OUTREG8(&pUartRegs->THR, ch);
}

//------------------------------------------------------------------------------
//
//  Function:  OEMReadDebugByte
//
//  Input character/byte from debug serial port
//
INT OEMReadDebugByte()
{
    OMAP_UART_REGS *pUartRegs = OALPAtoUA(OMAP_UART3_REGS_PA);
    UINT8 ch = OEM_DEBUG_READ_NODATA;
    UINT8 status;

    status = INREG8(&pUartRegs->LSR);
    if ((status & UART_LSR_RX_FIFO_E) != 0) {
        ch = INREG8(&pUartRegs->RHR);
        if ((status & UART_LSR_RX_ERROR) != 0) ch = OEM_DEBUG_COM_ERROR;
    }
    return (INT)ch;
}

//------------------------------------------------------------------------------
//
//  Function:  OEMWriteDebugString
//
//  Output unicode string to debug serial port
//
VOID OEMWriteDebugString(UINT16 *string)
{
    while (*string != L'\0') OEMWriteDebugByte((UINT8)*string++);
}

//------------------------------------------------------------------------------
//
//  Function:  OEMWriteDebugHex
//
//  Output hex value debug serial port, minimum number of hex characters is specified by depth
//
VOID OEMWriteDebugHex(unsigned long n, long depth)
{
    if (depth) 
	{
        depth--;
    }
    
    if ((n & ~0xf) || depth) 
	{
        OEMWriteDebugHex(n >> 4, depth);
        n &= 0xf;
    }
    
    if (n < 10) 
	{
        OEMWriteDebugByte((BYTE)(n + '0'));
    } 
	else 
	{ 
        OEMWriteDebugByte((BYTE)(n - 10 + 'A'));
    }
}

//------------------------------------------------------------------------------
