/***************************************************************************
**+----------------------------------------------------------------------+**
**|                                ****                                  |**
**|                                ****                                  |**
**|                                ******o***                            |**
**|                          ********_///_****                           |**
**|                           ***** /_//_/ ****                          |**
**|                            ** ** (__/ ****                           |**
**|                                *********                             |**
**|                                 ****                                 |**
**|                                  ***                                 |**
**|                                                                      |**
**|     Copyright (c) 1998 - 2009 Texas Instruments Incorporated         |**
**|                        ALL RIGHTS RESERVED                           |**
**|                                                                      |**
**| Permission is hereby granted to licensees of Texas Instruments       |**
**| Incorporated (TI) products to use this computer program for the sole |**
**| purpose of implementing a licensee product based on TI products.     |**
**| No other rights to reproduce, use, or disseminate this computer      |**
**| program, whether in part or in whole, are granted.                   |**
**|                                                                      |**
**| TI makes no representation or warranties with respect to the         |**
**| performance of this computer program, and specifically disclaims     |**
**| any responsibility for any damages, special or consequential,        |**
**| connected with the use of this program.                              |**
**|                                                                      |**
**+----------------------------------------------------------------------+**
***************************************************************************/
 
/** \file   TwIfDebug.c 
 *  \brief  TwIf debug commands 
 *
 * TwIf debug commands 
 * 
 *  \see    TwIf.c, TwIf.h
 */

#include "tidef.h"
#include "report.h"
#include "TwIf.h"
#include "TwIfDebug.h"
#include "TWDriverInternal.h"


static void printTwIfDbgFunctions (void);


/** 
 * \fn     twifDebugFunction
 * \brief  Main TwIf debug function
 * 
 * Main TwIf debug function
 * 
 * \param  hTwIf     - handle to the TWIF object
 * \param  uFuncType - the specific debug function
 * \param  pParam    - parameters for the debug function
 * \return None
 */
void twifDebugFunction (TI_HANDLE hTWD, TI_UINT32 uFuncType, void *pParam)
{
    TTwd      *pTWD  = (TTwd *)hTWD;
    TI_HANDLE  hTwIf = pTWD->hTwIf;

    switch (uFuncType)
    {
    case DBG_TWIF_PRINT_HELP:
        printTwIfDbgFunctions();
        break;

    case DBG_TWIF_PRINT_INFO:
        twIf_PrintModuleInfo (hTwIf);
        break;

    case DBG_TWIF_PRINT_WAKE_LOCK_COUNTER:
        twIf_PrintWakeLockCounter(hTwIf);
        break;

	default:
   		WLAN_OS_REPORT(("Invalid function type in TWIF debug function: %d\n", uFuncType));
        break;
    }
}


/** 
 * \fn     printTwIfDbgFunctions
 * \brief  Print the TwIf debug menu
 * 
 * Print the TwIf debug menu
 * 
 * \return None
 */
static void printTwIfDbgFunctions (void)
{
    WLAN_OS_REPORT(("   TwIf Debug Functions   \n"));
	WLAN_OS_REPORT(("--------------------------\n"));
	WLAN_OS_REPORT(("2300 - Print the TwIf Debug Help\n"));
	WLAN_OS_REPORT(("2301 - Print the TwIf Information\n"));
	WLAN_OS_REPORT(("2302 - Print the wake_lock counter"));
}







