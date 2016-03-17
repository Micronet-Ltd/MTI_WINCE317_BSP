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
/** \file SoftGemini.h
 *  \brief BlueTooth-Wlan coexistence module internal header file
 *
 *  \see SoftGemini.c
 */

/***************************************************************************/
/*																			*/
/*	  MODULE:	SoftGemini.h												*/
/*    PURPOSE:	BlueTooth-Wlan coexistence module internal header file			*/
/*																			*/
/***************************************************************************/

#ifndef __SOFT_GEMINI_H__
#define __SOFT_GEMINI_H__

#include "paramOut.h"
#include "SoftGeminiApi.h"


typedef struct 
{
	ESoftGeminiEnableModes  SoftGeminiEnable;						
    ESoftGeminiEnableModes  PsPollFailureLastEnableValue;
	TSoftGeminiParams		SoftGeminiParam;						/* for the FW */
	TI_BOOL					bProtectiveMode;
	TI_BOOL					bDriverEnabled;
    TI_BOOL					bPsPollFailureActive;							/* used to check if we should enable driver when we are switching different enable modes */
	TI_HANDLE				hCtrlData;
	TI_HANDLE				hTWD;
	TI_HANDLE				hReport;
	TI_HANDLE				hOs;
	TI_HANDLE				hSCR;
	TI_HANDLE				hPowerMgr;
	TI_HANDLE               hCmdDispatch;
	TI_HANDLE				hScanCncn;
	TI_HANDLE				hCurrBss;
    TI_HANDLE               hSme;
} SoftGemini_t;

TI_STATUS SoftGemini_handleRecovery(TI_HANDLE hSoftGemini);

#endif /* __SOFT_GEMINI_H__*/


