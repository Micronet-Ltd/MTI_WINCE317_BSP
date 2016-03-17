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

/****************************************************************************/
/*                                                                          */
/*    MODULE:   IPC_STA.h                                                   */
/*    PURPOSE:                                                              */
/*                                                                          */
/****************************************************************************/
#ifndef _IPC_STA_H_
#define _IPC_STA_H_

#include "cu_osapi.h"
#include "oserr.h"


/* defines */
/***********/


/* types */
/*********/

/* functions */
/*************/
THandle IpcSta_Create(const PS8 device_name);
VOID IpcSta_Destroy(THandle hIpcSta);
S32 IPC_STA_Private_Send(THandle hIpcSta, U32 ioctl_cmd, PVOID bufIn, U32 sizeIn, PVOID bufOut, U32 sizeOut);
S32 IpcSta_Send_Oid ( U32 oid, void* pInBuffer, U32 uInBufSize, void* pOutBuffer, U32 uOutBufSize, U32* puBytesReturned );
S32 IPC_STA_Wext_Send(THandle hIpcSta, U32 wext_request_id, PVOID p_iwreq_data, U32 len);
#endif  /* _IPC_STA_H_ */

