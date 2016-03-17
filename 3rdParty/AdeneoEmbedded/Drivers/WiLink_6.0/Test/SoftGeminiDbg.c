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
/** \file ScanCncnDbg.c
 *  \brief This file include the soft gemini debug module implementation
 *  \
 *  \date 14-Dec-2005
 */

#include "SoftGeminiApi.h"
#include "SoftGeminiDbg.h"
#include "report.h"
#include "osApi.h"


/**
 * \\n
 * \date 14-Dec-2005\n
 * \brief Main soft gemini debug function
 *
 * Function Scope \e Public.\n
 * \param hSoftGemini - handle to the soft gemini object.\n
 * \param funcType - the specific debug function.\n
 * \param pParam - parameters for the debug function.\n
 */
void SoftGeminiDebugFunction( TI_HANDLE hSoftGemini,TI_UINT32 funcType, void *pParam )
{
  	switch (funcType)
	{
	case DBG_SG_PRINT_HELP:
		printSoftGeminiDbgFunctions();
		break;

	case DBG_SG_PRINT_PARAMS:  
		printSoftGeminiParams(hSoftGemini);
		break;
		
	case DBG_SG_SENSE_MODE:
		SoftGemini_SenseIndicationCB(hSoftGemini,(char*) pParam,1);
		break;

	case DBG_SG_PROTECTIVE_MODE:
		SoftGemini_ProtectiveIndicationCB(hSoftGemini,(char*) pParam,1);
		break;

	default:
		WLAN_OS_REPORT(("Invalid function type in soft gemini debug function: %d\n", funcType));
		break;
	}
}

/**
 * \\n
 * \date 14-Dec-2005\n
 * \brief Prints soft gemini debug menu
 *
 * Function Scope \e Public.\n
 */
void printSoftGeminiDbgFunctions(void)
{
    WLAN_OS_REPORT(("   Scan Concentrator Debug Functions   \n"));
	WLAN_OS_REPORT(("---------------------------------------\n"));
	WLAN_OS_REPORT(("1800 - Print the soft gemini Debug Help\n"));
	WLAN_OS_REPORT(("1801 - Print soft gemini parameters    \n"));
	WLAN_OS_REPORT(("1802 - Trigger Sense Mode (enable - 1)	\n"));
	WLAN_OS_REPORT(("1803 - Trigger Protective Mode	(ON - 1)\n"));

}

/**
 * \\n
 * \date 14-Dec-2005\n
 * \brief Performs a driver scan from within an app scan.\n
 *
 * Function Scope \e Public.\n
 * \param hSoftGemini - handle to the soft gemini object.\n
 */
void printSoftGeminiParams( TI_HANDLE hSoftGemini )
{
   SoftGemini_printParams(hSoftGemini);
}
