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
/*    MODULE:   IPC_Event.h                                                 */
/*    PURPOSE:                                                              */
/*                                                                          */
/****************************************************************************/
#ifndef _IPC_EVENT_H_
#define _IPC_EVENT_H_

#include "ostitype.h"
#include "cu_event.h"

/* defines */
/***********/

/* types */
/*********/


/* functions */
/*************/
THandle IpcEvent_Create(VOID);
VOID IpcEvent_Destroy(THandle hIpcEvent);

S32 IpcEvent_EnableEvent(THandle hIpcEvent, U32 event);
S32 IpcEvent_DisableEvent(THandle hIpcEvent, U32 event);
S32 IpcEvent_UpdateDebugLevel(THandle hIpcEvent, S32 debug_level);

#endif  /* _IPC_EVENT_H_ */
        
