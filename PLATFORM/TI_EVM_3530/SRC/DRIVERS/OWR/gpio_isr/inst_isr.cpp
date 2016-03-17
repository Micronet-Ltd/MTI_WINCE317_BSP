/** =============================================================================
 *
 *  Copyright (c) 2007 Micronet
 *  All rights reserved.
 *
 *  
 *  Title:           CPP module INST_ISR
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

#include <bsp.h>
//#include <tsn.h>

#include <pkfuncs.h>
#include <windows.h>
#include <nkintr.h>
#include <ceddk.h>
#include <oal.h>
#include <oalex.h>
#include <omap35xx.h>
#include <gpio.h>
#include <bus.h>
#include <oal_prcm.h>
//#include <oal_mem.h>

#include "owrdrv.h"
//#include "inst_isr.h"
//#include "inst_isr_dbg.h"
//#include "gpio_isr.h"

// Globals
#define MAX_ISR_INSTANCES  16
#define INVALID_INST_ISR_HANDLE  ((FPTR_INST_ISR)(-1))
INSTALLABLE_ISR_INFO  instIsrInfo[MAX_ISR_INSTANCES];


DWORD ISRHandler( DWORD InstanceIndex )
{
   if( (InstanceIndex < MAX_ISR_INSTANCES) && 
       (instIsrInfo[InstanceIndex].fPrtIsrHandler != NULL) &&
       (instIsrInfo[InstanceIndex].fPrtIsrHandler != INVALID_INST_ISR_HANDLE )
	  )
      return instIsrInfo[InstanceIndex].fPrtIsrHandler(instIsrInfo[InstanceIndex].pIsrInfo) ;
    else
      return SYSINTR_CHAIN;
}



DWORD CreateInstance( void )
{

    DWORD dwCount;
    // Search For empty ISR Handle.
    for( dwCount=0; dwCount < MAX_ISR_INSTANCES; dwCount++)
	{
	 if( instIsrInfo[dwCount].fPrtIsrHandler == NULL )
            break;
	}

	if( dwCount == 0 )
	{
      ISRInitDebugSerial();
	}

	OWRISRMSG(OWR_ISR_DBG, (L"INST_ISR: CreateInstance # %d\r\n", dwCount));

    if (dwCount>=MAX_ISR_INSTANCES) // FULL.
        return (DWORD)-1;
    else
        instIsrInfo[dwCount].fPrtIsrHandler = INVALID_INST_ISR_HANDLE;

    return dwCount;

}

void DestroyInstance( DWORD InstanceIndex )
{
    if (InstanceIndex<MAX_ISR_INSTANCES && instIsrInfo[InstanceIndex].fPrtIsrHandler !=NULL )
	{
       instIsrInfo[InstanceIndex].fPrtIsrHandler = NULL;
	   instIsrInfo[InstanceIndex].pIsrInfo = NULL;
	   instIsrInfo[InstanceIndex].isrType = NO_INST_ISR;
	}

	return;
}

#include <string.h>
BOOL IOControl( DWORD InstanceIndex, DWORD IoControlCode, LPVOID pInBuf, DWORD InBufSize, LPVOID pOutBuf, DWORD OutBufSize, LPDWORD pBytesReturned )
{
	//WCHAR  sBuf[128];

	switch( IoControlCode )
	{ 
      //BOOL  fWasEnabled;

     case IOCTL_ISR_GPIO_INFO:
		if( pInBuf )
		{
         OWRISRMSG(OWR_ISR_DBG, (L"INP_ISR: IOCTL_ISR_GPIO_INFO, in = 0x%X\r\n", *(DWORD *)pInBuf));
		}

		if( pOutBuf )
		{
		 *(LPDWORD)pBytesReturned = (DWORD)sizeof(DWORD);
		 *(LPDWORD)pOutBuf = gpio163Info();
		  OWRISRMSG(OWR_ISR_DBG, (L"INP_ISR: IOCTL_ISR_GPIO_INFO, inpItrCnt = %d\r\n", *(LPDWORD)pOutBuf));
		}
		break;

     case IOCTL_ISR_GPIO_GET_DATA:
		 {
           OWR_INPUT_STATE   *pOWRInputStateOut;

			if( ( pOutBuf )&&( OutBufSize == sizeof(OWR_INPUT_STATE)) )
			{
             pOWRInputStateOut = (OWR_INPUT_STATE *)pOutBuf;
		     memcpy((char*)pOWRInputStateOut, (const char*)&g_OwrInputState, sizeof(OWR_INPUT_STATE));
             *(LPDWORD)pBytesReturned = (DWORD)sizeof(OWR_INPUT_STATE);
			  OWRISRMSG(OWR_ISR_DBG, (L"INP_ISR: IOCTL_ISR_GPIO_GET_DATA\r\n"));
			}
			else
			{
             OWRISRMSG(OWR_ISR_DBG, (L"INST_ISR: IOCTL_ISR_GPIO_GET_DATA, instance = %d, size error\r\n", InstanceIndex));
			 return FALSE;
			}

		 }break;

     case IOCTL_ISR_TIMER_INFO:
		if( pInBuf )
		{
         OWRISRMSG(OWR_ISR_DBG, (L"INP_ISR: IOCTL_ISR_TIMER_INFO, in = 0x%X\r\n", *(DWORD *)pInBuf));
		}

		if( pOutBuf )
		{
		 *(LPDWORD)pBytesReturned = (DWORD)sizeof(DWORD);
		 *(LPDWORD)pOutBuf = gpt4Info();
		  OWRISRMSG(OWR_ISR_DBG, (L"INP_ISR: IOCTL_ISR_TIMER_INFO, TimerItrCnt = %d\r\n", *(LPDWORD)pOutBuf));
		}
		break;

     case IOCTL_ISR_TIMER_GET_DATA:
		 {
           OWR_TIMER_STATE   *pOWRGptStateOut;

			if( ( pOutBuf )&&( OutBufSize == sizeof(OWR_TIMER_STATE)) )
			{
             pOWRGptStateOut = (OWR_TIMER_STATE *)pOutBuf;
		     memcpy((char*)pOWRGptStateOut, (const char*)&g_OwrGptState, sizeof(OWR_TIMER_STATE));
             *(LPDWORD)pBytesReturned = (DWORD)sizeof(OWR_TIMER_STATE);
			  OWRISRMSG(OWR_ISR_DBG, (L"INP_ISR: IOCTL_ISR_TIMER_GET_DATA\r\n"));
			}
			else
			{
             OWRISRMSG(OWR_ISR_DBG, (L"INST_ISR: IOCTL_ISR_TIMER_GET_DATA, instance = %d, size error\r\n", InstanceIndex));
			 return FALSE;
			}

		 }break;

     case IOCTL_ISR_GPIO_UNLOAD:
	 	break;

     case IOCTL_ISR_GPIO_CFG:
		 {
          //IOCTL_HAL_OALPATOVA_IN pPA;
		  //DWORD                  dwSize;
          OMAP_GPIO_ISR_INFO *pOWRGpioInfo;
  	      OMAP_GPIO_ISR_INFO *pOWRGpioInfoOut;
		  OMAP_GPIO_ISR_INFO *pOWRGpioInfoPrivate;

		   //pOWRGpioInfo = *(OWR_OMAP_GPIO_INFO **)pInBuf;
		   pOWRGpioInfo = (OMAP_GPIO_ISR_INFO *)pInBuf;
		   pOWRGpioInfoOut = (OMAP_GPIO_ISR_INFO *)pOutBuf;

		   if( ( InBufSize != sizeof(OMAP_GPIO_ISR_INFO) )||
			   ( OutBufSize!= sizeof(OMAP_GPIO_ISR_INFO) )||
               ( pInBuf == NULL )||
			   ( pOutBuf == NULL )
			  )
			{
             OWRISRMSG(OWR_ISR_DBG, (L"INST_ISR: IOCTL_ISR_GPIO_CFG, instance = %d, size error\r\n", InstanceIndex));
			 return FALSE;
			}

		   
		   if( instIsrInfo[InstanceIndex].fPrtIsrHandler == INVALID_INST_ISR_HANDLE )
		   {

			   switch(pOWRGpioInfo->input) 
			   {
			    case GPIO_2:
				 {
				   if( ( pOWRGpioInfoPrivate = gpio2Cfg(pOWRGpioInfo)) == NULL )
				   {
					OWRISRMSG(OWR_ISR_DBG, (L"INST_ISR: IOCTL_ISR_GPIO_CFG Failure\r\n"));
					return FALSE;
				   }
				   else
				   {
                    OWRISRMSG(OWR_ISR_DBG, (L"INST_ISR: IOCTL_ISR_GPIO_CFG, instance = %d, inp = %d\r\n", InstanceIndex, pOWRGpioInfo->input));
				   }

				   memcpy((char*)pOWRGpioInfoOut, (const char*)pOWRGpioInfo, sizeof(OMAP_GPIO_ISR_INFO));
                   *(LPDWORD)pBytesReturned = (DWORD)sizeof(OMAP_GPIO_ISR_INFO);
				   
				   instIsrInfo[InstanceIndex].fPrtIsrHandler = gpio2ISRHandler;
				   instIsrInfo[InstanceIndex].pIsrInfo = pOWRGpioInfoPrivate;
				   instIsrInfo[InstanceIndex].isrType = GPIO_ISR;

				 } break;

			    case GPIO_163:
				case GPIO_96: 
				 {
				   if( ( pOWRGpioInfoPrivate = gpio163Cfg(pOWRGpioInfo)) == NULL )
				   {
					OWRISRMSG(OWR_ISR_DBG, (L"INST_ISR: IOCTL_ISR_GPIO_CFG Failure\r\n"));
					return FALSE;
				   }
				   else
				   {
                    OWRISRMSG(OWR_ISR_DBG, (L"INST_ISR: IOCTL_ISR_GPIO_CFG, instance = %d, inp = %d\r\n", InstanceIndex, pOWRGpioInfo->input));
				   }

				   memcpy((char*)pOWRGpioInfoOut, (const char*)pOWRGpioInfo, sizeof(OMAP_GPIO_ISR_INFO));
                   *(LPDWORD)pBytesReturned = (DWORD)sizeof(OMAP_GPIO_ISR_INFO);

				   instIsrInfo[InstanceIndex].fPrtIsrHandler = gpio163ISRHandler;
				   instIsrInfo[InstanceIndex].pIsrInfo = pOWRGpioInfoPrivate;
				   instIsrInfo[InstanceIndex].isrType = GPIO_ISR;

				 } break;

				default: 
				 {
				  OWRISRMSG(OWR_ISR_DBG, (L"INST_ISR: IOCTL_ISR_GPIO_CFG unknown GPIO\r\n"));
				  return FALSE;
				 }break;
			  }


		   }
		   else
		   {
             OWRISRMSG(OWR_ISR_DBG, (L"INST_ISR: IOCTL_ISR_GPIO_CFG I/O Instance Failure\r\n"));
		   }

		 
		 } break; 

	 case IOCTL_ISR_TIMER_CFG:			 
		 {
          OMAP_GPT_ISR_INFO *pOWRGptInfo;
  	      OMAP_GPT_ISR_INFO *pOWRGptInfoOut;
		  OMAP_GPT_ISR_INFO *pOWRGptInfoPrivate;

		   //pOWRGpioInfo = *(OWR_OMAP_GPIO_INFO **)pInBuf;
		   pOWRGptInfo = (OMAP_GPT_ISR_INFO *)pInBuf;
		   pOWRGptInfoOut = (OMAP_GPT_ISR_INFO *)pOutBuf;

		   if( ( InBufSize != sizeof(OMAP_GPT_ISR_INFO) )||
			   ( OutBufSize!= sizeof(OMAP_GPT_ISR_INFO) )||
               ( pInBuf == NULL )||
			   ( pOutBuf == NULL )
			  )
			{
             OWRISRMSG(OWR_ISR_DBG, (L"INST_ISR: IOCTL_ISR_TIMER_CFG, instance = %d, size error\r\n", InstanceIndex));
			 return FALSE;
			}

		   if( instIsrInfo[InstanceIndex].fPrtIsrHandler != INVALID_INST_ISR_HANDLE )
		   {
             OWRISRMSG(OWR_ISR_DBG, (L"INST_ISR: IOCTL_ISR_TIMER_CFG, instance = %d, Handler error\r\n", InstanceIndex));
			 return FALSE;
		   }
		   
		   if( ( pOWRGptInfo->gpTimerNum >= 3)&&( pOWRGptInfo->gpTimerNum <= 7) )
		   {
				   if( ( pOWRGptInfoPrivate = gpt4Cfg(pOWRGptInfo)) == NULL )
				   {
					OWRISRMSG(OWR_ISR_DBG, (L"INST_ISR: IOCTL_ISR_TIMER_CFG Failure\r\n"));
					return FALSE;
				   }
				   else
				   {
                    OWRISRMSG(OWR_ISR_DBG, (L"INST_ISR: IOCTL_ISR_TIMER_CFG, instance = %d, num = %d\r\n", InstanceIndex, pOWRGptInfo->gpTimerNum));
				   }

				   memcpy((char*)pOWRGptInfoOut, (const char*)pOWRGptInfo, sizeof(OMAP_GPT_ISR_INFO));
                   *(LPDWORD)pBytesReturned = (DWORD)sizeof(OMAP_GPT_ISR_INFO);

				   instIsrInfo[InstanceIndex].fPrtIsrHandler = gpt4ISRHandler;
				   instIsrInfo[InstanceIndex].pIsrInfo = pOWRGptInfoPrivate;
				   instIsrInfo[InstanceIndex].isrType = GPTIMER_ISR;
    
		   }
			
			
		 } break;
		 
     case IOCTL_ISR_OWR_RESET_PRESENCE:
		 {
           owrIsrSetResetPulse();
		   OWRISRMSG(OWR_ISR_DBG, (L"INST_ISR: IOCTL_ISR_OWR_RESET_PRESENCE, instance = %d\r\n", InstanceIndex));
		 } break;

     case IOCTL_ISR_OWR_DETECT_PULSE:
		 {
           BOOL pulseDetect;

           if( ( pInBuf )&& (InBufSize == sizeof(BOOL) ) )
		   {
            pulseDetect = *(BOOL *)pInBuf;
			owrIsrSetDetectPulse(pulseDetect);
		   }
		   else
		   {
            OWRISRMSG(OWR_ISR_DBG, (L"INST_ISR: IOCTL_ISR_OWR_DETECT_PULSE, instance = %d, Invalid input params\r\n", InstanceIndex));
            return(FALSE);
		   }

		 } break;


     case IOCTL_ISR_OWR_ROM_CMD:
		 {
           UINT8   *romCmd;

           if( ( pInBuf )&& (InBufSize < OWR_ISR_MAX_DATA_LEN ) )
		   {
            romCmd = (UINT8 *)pInBuf;

			if( owrIsrWriteData(romCmd, InBufSize) )
			{
			 OWRISRMSG(OWR_ISR_DBG, (L"INST_ISR: IOCTL_ISR_OWR_ROM_CMD, instance = %d, send read ROM cmd Ok, [0x%02X], size = %d\r\n", InstanceIndex, romCmd[0], InBufSize));
			}
			else
			{
			 OWRISRMSG(OWR_ISR_DBG, (L"INST_ISR: IOCTL_ISR_OWR_ROM_CMD, instance = %d, send read ROM cmd Failure, [0x%02X], size = %d\r\n", InstanceIndex, romCmd[0], InBufSize));
			 return(FALSE);
			}

		   }
		   else
		   {
            OWRISRMSG(OWR_ISR_DBG, (L"INST_ISR: IOCTL_ISR_OWR_ROM_CMD, instance = %d, Invalid input params\r\n", InstanceIndex));
            return(FALSE);
		   }

		 } break;


     case IOCTL_ISR_OWR_READ:
		 {
           UINT8   dataLen;

           if( ( pInBuf )&& (InBufSize == sizeof(DWORD)) )
		   {
            dataLen = (UINT8) *(DWORD *)pInBuf;

			if( owrIsrReadData(dataLen) )
			{
			 OWRISRMSG(OWR_ISR_DBG, (L"INST_ISR: IOCTL_ISR_OWR_READ, instance = %d, read data Ok, len = %d\r\n", InstanceIndex, dataLen));
			}
			else
			{
			 OWRISRMSG(OWR_ISR_DBG, (L"INST_ISR: IOCTL_ISR_OWR_READ, instance = %d, read data Failure, len = %d\r\n", InstanceIndex, dataLen));
			 return(FALSE);
			}

		   }
		   else
		   {
            OWRISRMSG(OWR_ISR_DBG, (L"INST_ISR: IOCTL_ISR_OWR_READ, instance = %d, Invalid input params\r\n", InstanceIndex));
            return(FALSE);
		   }

		 } break;

	 case IOCTL_ISR_TEST_1:
		 {
			 INT32    i;
			 UINT8    romValue[8];
			 BOOL     ret = TRUE;

          // Initiate "Reset" pulse for 480 us.
			 owrIsrSetResetPulse();
			 Sleep(10);

          // Send read ROM command (0x33)
			 romValue[0] = 0x33;
			if( owrIsrWriteData(romValue ,1) )
			{
			 OWRISRMSG(OWR_ISR_DBG, (L"INST_ISR: IOCTL_ISR_TEST_1, instance = %d, send read ROM cmd Ok\r\n", InstanceIndex));
			}
			else
			{
			 OWRISRMSG(OWR_ISR_DBG, (L"INST_ISR: IOCTL_ISR_TEST_1, instance = %d, send read ROM cmd Failure\r\n", InstanceIndex));
			}

            Sleep(10);

			// Recieve data
			ret = owrIsrReadData(8);

			Sleep(10);

			if( ret == FALSE )
			{
             OWRISRMSG(OWR_ISR_DBG, (L"INST_ISR: IOCTL_ISR_TEST_1, instance = %d, rcv data Failure\r\n", InstanceIndex)); 
			}
			else
			{
			 OWRISRMSG( OWR_ISR_DBG, (L"INST_ISR: IOCTL_ISR_TEST_1, instance = %d, rcv data OK:  \r\n", InstanceIndex)); 
			 //OWR_ISR_MAX_DATA_LEN
             for (i=7; i>=0; i--)
              OWRISRMSG(OWR_ISR_DBG, (L"[0x%02X]", g_OwrGptState.readData[i] )); 
			}

			OWRISRMSG(OWR_ISR_DBG, (L"\r\n")); 
#if 0
			// Log
            for (i=0; i < OWR_TIMER_LOG_LEN; i++)
			{
              if( g_OwrGptState.log[i] != 0x00 )
              OWRISRMSG(OWR_ISR_DBG, (L"%c", g_OwrGptState.log[i] )); 
			}

			OWRISRMSG(OWR_ISR_DBG, (L"\r\n")); 
#endif
			
		 } break;

     default:
		//ISRWriteDebugString(L"INP_ISR: Invalid IOCTL\r\n");
		//RETAILMSG(0, (L"INP_ISR: Invalid IOCTL\r\n"));
		return FALSE;
    }
	return TRUE;
}


// Debug UART registers
//static OMAP_UART_REGS  *s_pUartRegs = NULL; 


BOOL __stdcall DllEntry( HINSTANCE hinstDll, DWORD dwReason, LPVOID lpReserved )
{
  DWORD     dwCount;

    switch ( dwReason ) 
	{
     case DLL_PROCESS_ATTACH:
	 {
      for( dwCount = 0; dwCount < MAX_ISR_INSTANCES; dwCount++)
	  {
       instIsrInfo[dwCount].isrType = NO_INST_ISR;
	   instIsrInfo[dwCount].pIsrInfo = NULL;
	   instIsrInfo[dwCount].fPrtIsrHandler = NULL;
	  }

	 }break;

     case DLL_PROCESS_DETACH:
    
        break;
    }

	return TRUE;
}




