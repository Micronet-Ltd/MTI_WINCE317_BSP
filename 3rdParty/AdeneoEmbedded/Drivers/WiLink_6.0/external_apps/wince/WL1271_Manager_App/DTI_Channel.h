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
#ifndef __DTI_CHANNEL_H
#define __DTI_CHANNEL_H


#include "tidef.h"
#include "TWDriverScan.h"


INT_PTR CALLBACK MISCWndProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

typedef struct
{
	ERadioBand	bandIndex;
	TI_UINT8	bandNumber;
	TI_INT8		bssThreshold;
	TI_UINT8	channelNumberForDiscCycle;
	TI_UINT8	nbChannels;
	TI_UINT8	channelArray[30];
} DTI_Channel_t;


#endif