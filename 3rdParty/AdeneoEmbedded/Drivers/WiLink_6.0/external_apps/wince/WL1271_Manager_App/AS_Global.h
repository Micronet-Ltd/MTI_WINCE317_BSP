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
**|     Copyright (c) 1998-2010 Texas Instruments Incorporated           |**
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

#ifndef __AS_GLOBAL_H
#define __AS_GLOBAL_H

#include "tidef.h"
#include "TWDriverScan.h"

typedef struct
{
	WCHAR			SSID[33];
	EScanType		scanType;
	ERadioBand		bandIndex;
	TI_UINT8		numOfProbeReqs;
	ERateMask		bitrate;
	TI_UINT8		nbChannels;
	TI_UINT8		triggeringTid;

} AS_Global_t;

static AS_Global_t gASGlobalData;

TI_UINT8 ASG_GetNumberOfChannels(void);
INT_PTR CALLBACK ASGWndProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);


#endif