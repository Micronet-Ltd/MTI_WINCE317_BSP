/** =============================================================================
 *
 *  Copyright (c) 2007 Micronet
 *  All rights reserved.
 *
 *  
 *  Title:           CPP module inp_isr_dbg
 *  Author(s):       Michael Streshinsky
 *  Creation Date:   1-Dec-2009
 *
 *  Revision Histiory:
 *
 * Date         Author                     Description	
 * ---------------------------------------------------------------------------
 *
 * ============================================================================= 
 */

/********************** INCLUDES **********************************************/
#include <bsp.h>
//#include <tsn.h>

#include <pkfuncs.h>
#include <windows.h>
#include <strsafe.h>
#include <nkintr.h>
#include <ceddk.h>
#include <oal.h>
#include <oalex.h>
#include <omap35xx.h>
#include <gpio.h>
#include <bus.h>
#include <oal_prcm.h>

#include "owrdrv.h"

VOID ISRWriteDebugByte(UINT8 ch);

/********************** LOCAL CONSTANTS ***************************************/
/*********** LOCAL STRUCTURES, ENUMS, AND TYPEDEFS ****************************/
/****************** STATIC FUNCTION PROTOTYPES *********************************/
/*********************** LOCAL MACROS *****************************************/
/********************* GLOBAL VARIABLES ***************************************/
/********************* STATIC VARIABLES ***************************************/
static OMAP_UART_REGS  *s_pUartRegsISR; 
static BOOL bEnableDebugMessages = TRUE;

/******************** FUNCTION DEFINITIONS ************************************/

VOID ISRInitDebugSerial( VOID )
{
  IOCTL_HAL_OALPATOVA_IN pPA;
  //VOID                   *va;
  DWORD                  dwSize;
 // PHYSICAL_ADDRESS   pA;

    //----------------------------------------------------------------------
    //  Initialize UART3
    //----------------------------------------------------------------------

   // s_pUartRegs = OALPAtoUA(OMAP_UART3_REGS_PA);
#if 1
     pPA.pa     = OMAP_UART3_REGS_PA;
     pPA.cached = FALSE;
	 if( !KernelIoControl(IOCTL_HAL_OALPATOVA, (LPVOID)&pPA, sizeof(IOCTL_HAL_OALPATOVA_IN), (LPVOID)&s_pUartRegsISR, sizeof(LPVOID), (LPDWORD)&dwSize) )
	  return;
#endif

#if 0
	pA.QuadPart = OMAP_UART3_REGS_PA;
	s_pUartRegsISR = (OMAP_UART_REGS*)MmMapIoSpace( pA, sizeof(OMAP_UART_REGS), FALSE);
#endif

	// UART 3 was init
#if 0
    // Reset UART & wait until it completes
    OUTREG8(&s_pUartRegsISR->SYSC, UART_SYSC_RST);
    while ((INREG8(&s_pUartRegsISR->SYSS) & UART_SYSS_RST_DONE) == 0);
    // Set baud rate
    OUTREG8(&s_pUartRegsISR->LCR, UART_LCR_DLAB);
    OUTREG8(&s_pUartRegsISR->DLL, BSP_UART_DSIUDLL);
    OUTREG8(&s_pUartRegsISR->DLH, BSP_UART_DSIUDLH);
    // 8 bit, 1 stop bit, no parity
    OUTREG8(&s_pUartRegsISR->LCR, BSP_UART_LCR);
    // Enable FIFO
    OUTREG8(&s_pUartRegsISR->FCR, UART_FCR_FIFO_EN);
    // Pool
    OUTREG8(&s_pUartRegsISR->IER, 0);
    // Set DTR/RTS signals
    OUTREG8(&s_pUartRegsISR->MCR, UART_MCR_DTR|UART_MCR_RTS);
    // Configuration complete so select UART 16x mode
    OUTREG8(&s_pUartRegsISR->MDR1, UART_MDR1_UART16);
#endif
}

//------------------------------------------------------------------------------
//
//  Function:  OEMWriteDebugByte
//
//  Write byte to debug serial port.
//
VOID ISRWriteDebugByte(UINT8 ch)
{

  if (s_pUartRegsISR != NULL && bEnableDebugMessages)
        {
        // Wait while FIFO is full
        while ((INREG8(&s_pUartRegsISR->SSR) & UART_SSR_TX_FIFO_FULL) != 0);
        // Send byte
        OUTREG8(&s_pUartRegsISR->THR, ch);
        }        
}


//------------------------------------------------------------------------------
//
//  Function:  ISRWriteDebugString
//
//  Output unicode string to debug serial port
//
VOID ISRWriteDebugString(UINT16 *string)
{
    while (*string != L'\0') ISRWriteDebugByte((UINT8)*string++);
}


//------------------------------------------------------------------------------
//
//  Function:  ISRDbgPintf
//
//  This function format string and write it to serial debug output.
//
WCHAR s_buffer[512];

VOID ISRDbgPintf(LPCWSTR format, ...)
{
    va_list pArgList;
    //WCHAR buffer[128];

    va_start(pArgList, format);
	wvsprintf(s_buffer, format, pArgList);
	//_vsnwprintf_s(s_buffer, 128, format, pArgList);
    //NKwvsprintfW(buffer, format, pArgList, 128);
    ISRWriteDebugString(s_buffer);
}

