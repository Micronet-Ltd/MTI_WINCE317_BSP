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
#ifndef _EVHANDLER_H_
#define _EVHANDLER_H_


#include "TI_IPC_Api.h"


typedef struct 
{
    IPC_EV_DATA Array[MAX_SEND_EVENTS];
    TI_UINT32       TailIndex; /* Points to the next free node*/
    TI_UINT32       HeadIndex; /* Points to the first occupied node*/
    TI_UINT32       Counter;
}EV_CYCL_ARRAY, *PEV_CYCL_ARRAY;

typedef struct 
{
    TI_HANDLE		   hOs;
    IPC_EVENT_PARAMS   RegistrationArray[IPC_EVENT_MAX][MAX_REGISTERED_MODULES];
    EV_CYCL_ARRAY      SendEventArray;
    TI_UINT32          LastUMEventType;

}TEvHandlerObj;

/* Upper Interface*/
TI_HANDLE EvHandler_Create         (TI_HANDLE hOs);

TI_UINT32 EvHandlerUnload          (TI_HANDLE hEvHandler);

TI_UINT32 EvHandlerRegisterEvent   (TI_HANDLE hEvHandler, TI_UINT8* pData,   TI_UINT32 Length);

TI_UINT32 EvHandlerUnRegisterEvent (TI_HANDLE hEvHandler, TI_HANDLE uEventID);

TI_UINT32 EvHandlerMaskEvent       (TI_HANDLE hEvHandler, TI_UINT32 uEventID);

TI_UINT32 EvHandlerUnMaskEvent     (TI_HANDLE hEvHandler, TI_UINT32 uEventID);

TI_UINT32 EvHandlerGetEventData    (TI_HANDLE hEvHandler, TI_UINT8* pData, TI_UINT32* pLength);
/* Bottom Interface*/

TI_UINT32 EvHandlerSendEvent       (TI_HANDLE hEvHandler, TI_UINT32 EvType, TI_UINT8* pData, TI_UINT32 Length);


#endif
