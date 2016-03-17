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
 
/***************************************************************************/
/*                                                                         */
/*    MODULE:   HwInit_api.h                                               */
/*    PURPOSE:  HwInit module Header file                             	   */
/*                                                                         */
/***************************************************************************/
#ifndef _HW_INIT_API_H_
#define _HW_INIT_API_H_

#include "TWDriver.h"

typedef TI_STATUS (*TFinalizeCb) (TI_HANDLE);

typedef struct
{
    TI_UINT8   MacClock;
    TI_UINT8   ArmClock;
    TI_BOOL    FirmwareDebug;

} TBootAttr;


/* Callback function definition for EndOfRecovery */
typedef void (* TEndOfHwInitCb) (TI_HANDLE handle);


TI_HANDLE hwInit_Create (TI_HANDLE hOs);
TI_STATUS hwInit_Init   (TI_HANDLE hHwInit,
                         TI_HANDLE hReport, 
                         TI_HANDLE hTimer, 
                         TI_HANDLE hTWD, 
                         TI_HANDLE hFinalizeDownload, 
                         TFinalizeCb fFinalizeDownload, 
                         TEndOfHwInitCb fInitHwCb);
TI_STATUS hwInit_SetNvsImage (TI_HANDLE hHwInit, TI_UINT8 *pbuf, TI_UINT32 length);
TI_STATUS hwInit_SetFwImage (TI_HANDLE hHwInit, TFileInfo *pFileInfo);
TI_STATUS hwInit_Destroy (TI_HANDLE hHwInit);
TI_STATUS hwInit_Boot (TI_HANDLE hHwInit);
TI_STATUS hwInit_LoadFw (TI_HANDLE hHwInit);
TI_STATUS hwInit_ReadRadioParams (TI_HANDLE hHwInit);
TI_STATUS hwInit_WriteIRQPolarity(TI_HANDLE hHwInit);
TI_STATUS hwInit_InitPolarity(TI_HANDLE hHwInit);


#endif /* _HW_INIT_API_H_ */
