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

/** 
 * \file  PowerMgrKeepAlive.h
 * \brief user keep-alive declarations
 */

#ifndef __POWER_MGR_KEEP_ALIVE_H__
#define __POWER_MGR_KEEP_ALIVE_H__

TI_HANDLE powerMgrKL_create (TI_HANDLE hOS);
void powerMgrKL_destroy (TI_HANDLE hPowerMgrKL);
void powerMgrKL_init (TI_HANDLE hPowerMgrKL, TStadHandlesList *pStadHandles);
void powerMgrKL_setDefaults (TI_HANDLE hPowerMgrKL);
TI_STATUS powerMgrKL_start (TI_HANDLE hPowerMgrKL);
TI_STATUS powerMgrKL_stop (TI_HANDLE hPowerMgrKL, TI_BOOL bDisconnect);
TI_STATUS powerMgrKL_setParam (TI_HANDLE hPowerMgrKL, paramInfo_t *pParam);
TI_STATUS powerMgrKL_getParam (TI_HANDLE hPowerMgrKL, paramInfo_t *pParam);

#endif /* __POWER_MGR_KEEP_ALIVE_H__ */

