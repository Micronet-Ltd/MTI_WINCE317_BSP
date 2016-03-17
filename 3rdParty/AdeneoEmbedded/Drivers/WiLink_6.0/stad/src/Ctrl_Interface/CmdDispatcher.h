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

/** \file   CmdDispatcher.h
 *  \brief  CmdDispatcher module header file.                                  
 *
 *  \see    CmdDispatcher.c
 */

#ifndef _CMD_DISPATCHER_H_
#define _CMD_DISPATCHER_H_

#include "DrvMainModules.h"


/* Public Functions Prototypes */
/* =========================== */

TI_HANDLE cmdDispatch_Create   (TI_HANDLE hOs);
void      cmdDispatch_Init     (TStadHandlesList *pStadHandles);
TI_STATUS cmdDispatch_Destroy  (TI_HANDLE hCmdDispatch);
TI_STATUS cmdDispatch_SetParam (TI_HANDLE hCmdDispatch, void *param);                         
TI_STATUS cmdDispatch_GetParam (TI_HANDLE hCmdDispatch, void *param);


#endif  /* _CMD_DISPATCHER_H_ */


