/** =============================================================================
 *
 *  Copyright (c) 2007 Micronet
 *  All rights reserved.
 *
 *  
 *  Title:           One Wire High-Level C module
 *  Author(s):       Michael Streshinsky
 *  Creation Date:   17-Dec-2009
 *
 *  Revision History:
 *
 * Date         Author                     Description	
 * ---------------------------------------------------------------------------
 *
 * ============================================================================= 
 */

/********************** INCLUDES **********************************************/

#include <windows.h>
#include <ceddk.h>
#include <ceddkex.h>
//#include <initguid.h>
//#include <gpio.h>
//#include <bus.h>
#include "omap35xx.h"
//#include "oal.h"
//#include <oal.h>
//#include <oalex.h>
//#include <omap35xx.h>
//#include "oal_io.h"
//#include "oal_memory.h"
//#include <oal_prcm.h>
//#include <aio_api.h>

//#include "..\gpio_isr\gpio_isr.h"
//#include "..\gpio_isr\gpt_isr.h"
#include "owrdrv.h"
#include "iButton.h"

/********************** LOCAL CONSTANTS ***************************************/
/*********** LOCAL STRUCTURES, ENUMS, AND TYPEDEFS ****************************/
/****************** STATIC FUNCTION PROTOTYPES *********************************/
UINT8 OWCalcCRC8(UINT8 inData, UINT8 seed);
BOOL OWCheckRomCRC(UINT8 *romValue);

/*********************** LOCAL MACROS *****************************************/
/********************* GLOBAL VARIABLES ***************************************/
/********************* STATIC VARIABLES ***************************************/
CRITICAL_SECTION    s_iButtonCS;

/******************** FUNCTION DEFINITIONS ************************************/


/** 
 * @fn UINT8 OWCalcCRC8(UINT8 data, UINT8 seed)
 *  
 * @param UINT8 data, UINT8 seed
 *
 * @return TRUE on success
 *
 *
 * @brief Calculate CRC8
 *    
 */  

UINT8 OWCalcCRC8(UINT8 inData, UINT8 seed)
{
    UINT8 bitsLeft;
    UINT8 temp;

    for (bitsLeft = 8; bitsLeft > 0; bitsLeft--)
    {
        temp = ((seed ^ inData) & 0x01);
        if (temp == 0)
        {
            seed >>= 1;
        }
        else
        {
            seed ^= 0x18;
            seed >>= 1;
            seed |= 0x80;
        }
        inData >>= 1;
    }
    return seed;    
}



/** 
 * @fn UINT8 OWCalcCRC8(UINT8 data, UINT8 seed)
 *  
 * @param UINT8 data, UINT8 seed
 *
 * @return TRUE on success
 *
 *
 * @brief Calculate and check CRC of 64 bit ROM identifier.
 *    
 */  

BOOL OWCheckRomCRC(UINT8 *romValue)
{
    UINT8 i;
    UINT8 crc8 = 0;
	DWORD sum = 0;
    
    if( romValue == NULL )
      return FALSE;
    
    for (i = 0; i < 7; i++)
    {
        crc8 = OWCalcCRC8(*romValue, crc8);
		sum += *romValue;
        romValue++;
    }
   
	if( sum == 0 )
	{
     RETAILMSG(OWR_DRV_DBG, (L"OWCheckRomCRC, sum = 0\r\n"));
	 return FALSE;
	}

	// 0x%02X
	RETAILMSG(OWR_DRV_DBG, (L"OWCheckRomCRC = 0x%02X\r\n", crc8));

    if (crc8 == (*romValue))
    {
     return TRUE;
    }
	else
	{
     return FALSE;
	}
}


BOOL iButtonInit(void)
{
  RETAILMSG(OWR_DRV_DBG, (L"+iButtonInit\r\n"));

  InitializeCriticalSection(&s_iButtonCS);

  if( !createIbtnTranslTable() )
  {
   RETAILMSG(OWR_DRV_DBG, (L"iButtonInit: createIbtnTranslTable failure\r\n"));
   return(FALSE);
  }
  else
  {
   RETAILMSG(OWR_DRV_DBG, (L"iButtonInit: createIbtnTranslTable OK\r\n"));
   return(TRUE);
  }

  RETAILMSG(OWR_DRV_DBG, (L"-iButtonInit\r\n"));
}

BOOL iButtonRead(OMAP_GPIO_ONE_WIRE *pOWR_OMAP, UINT8 romValue[])
{
	//OWR_INPUT_STATE   lOWRInputState;
	//OWR_TIMER_STATE   lOWRGptState;
	//DWORD             dwLen, readDataLen;
	//DWORD             dwReason;
	UINT8             dwRomCmd[1];
	//INT32             i;
	BOOL              pulseDetect; 
	DWORD             dwLen; 
	BOOL              ret;

	EnterCriticalSection(&s_iButtonCS);

    // Guard by critical section !
	// iButton as HW device is non-reentrable in its nature
    // Disable GPIO Interrupt

	//GPIOSetMode(s_OWR_OMAP.owrOmapGpio.hGpio, s_OWR_OMAP.owrOmapGpio.input, GPIO_DIR_INPUT);
    // Disable GPIO Input interrupt, ready to detect impulse
#if 1
    pulseDetect = FALSE;
	if( !KernelLibIoControl(pOWR_OMAP->owrIsrHandle_gpio, IOCTL_ISR_OWR_DETECT_PULSE, (LPVOID)&pulseDetect, sizeof(BOOL), NULL, 0, (LPDWORD)&dwLen) )
	{
	 RETAILMSG(OWR_DRV_DBG, (L"iButtonRead: KernelLibIoControl, IOCTL_ISR_OWR_DETECT_PULSE failure\r\n"));
	 return(FALSE);
	}
#endif

    RETAILMSG(OWR_DRV_DBG, (L"iButtonRead: Started\r\n"));

	memset(romValue, 0xFF, 8);


	dwRomCmd[0] = 0x33;
	ret = FALSE;
	if( owrResetPresence(pOWR_OMAP) )
	{
		if( owrWrite(pOWR_OMAP, dwRomCmd, 1) )
		{
			 if( owrRead(pOWR_OMAP, romValue, 8) )
			 {

			  ret = TRUE;
			 }
		}
	}
     
    
	// GPIOSetMode(s_OWR_OMAP.owrOmapGpio.hGpio, s_OWR_OMAP.owrOmapGpio.input, GPIO_DIR_INPUT|GPIO_INT_HIGH_LOW);
	if( !KernelLibIoControl(pOWR_OMAP->owrIsrHandle_gpio, IOCTL_ISR_OWR_DETECT_PULSE, (LPVOID)&g_pulseDetect, sizeof(BOOL), NULL, 0, (LPDWORD)&dwLen) )
	{
	 RETAILMSG(OWR_DRV_DBG, (L"iButtonRead: KernelLibIoControl, IOCTL_ISR_OWR_DETECT_PULSE failure\r\n"));
	 return(FALSE);
	}

#if 0
	  RETAILMSG(OWR_DRV_DBG, (L"iButtonRead: read data Result:\r\n"));
	  for (i=7; i>=0; i--)
	   RETAILMSG(OWR_DRV_DBG, (L"[0x%02X]", romValue[i] )); 

	  RETAILMSG(OWR_DRV_DBG, (L"\r\n"));
#endif

	// Verify checksum
	if( ret == TRUE )
	 ret = OWCheckRomCRC(romValue);
	else
	{
     RETAILMSG(OWR_DRV_DBG, (L"iButtonRead: checksum failure\r\n"));
	}

	LeaveCriticalSection(&s_iButtonCS);

    return(ret);

}



BOOL iButtonCheckPresence(OMAP_GPIO_ONE_WIRE *pOWR_OMAP)
{
	//OWR_INPUT_STATE   lOWRInputState;
	//OWR_TIMER_STATE   lOWRGptState;
	//DWORD             dwLen, readDataLen;
	//DWORD             dwReason;
	INT32             i;
	BOOL              pulseDetect; 
	DWORD             dwLen; 
	BOOL              ret;

	EnterCriticalSection(&s_iButtonCS);

    // Guard by critical section !
	// iButton as HW device is non-reentrable in its nature
    // Disable GPIO Interrupt

	//GPIOSetMode(s_OWR_OMAP.owrOmapGpio.hGpio, s_OWR_OMAP.owrOmapGpio.input, GPIO_DIR_INPUT);
    // Disable GPIO Input interrupt, ready to detect impulse
#if 1
    pulseDetect = FALSE;
	if( !KernelLibIoControl(pOWR_OMAP->owrIsrHandle_gpio, IOCTL_ISR_OWR_DETECT_PULSE, (LPVOID)&pulseDetect, sizeof(BOOL), NULL, 0, (LPDWORD)&dwLen) )
	{
	 RETAILMSG(OWR_DRV_DBG, (L"iButtonCheckPresence: KernelLibIoControl, IOCTL_ISR_OWR_DETECT_PULSE failure\r\n"));
	 return(FALSE);
	}
#endif

    RETAILMSG(OWR_DRV_DBG, (L"iButtonCheckPresence: Started\r\n"));

	for(i = 0; i < 3; i++ )
	{
	 ret = owrResetPresence(pOWR_OMAP);

	 if( ret == TRUE )
      break;
	 else
       Sleep(20);
	}
    
    // Enable GPIO Input interrupt, ready to detect impulse
	if( !KernelLibIoControl(pOWR_OMAP->owrIsrHandle_gpio, IOCTL_ISR_OWR_DETECT_PULSE, (LPVOID)&g_pulseDetect, sizeof(BOOL), NULL, 0, (LPDWORD)&dwLen) )
	{
	 RETAILMSG(OWR_DRV_DBG, (L"iButtonCheckPresence: KernelLibIoControl, IOCTL_ISR_OWR_DETECT_PULSE failure\r\n"));
	 return(FALSE);
	}


	LeaveCriticalSection(&s_iButtonCS);

    return(ret);

}
