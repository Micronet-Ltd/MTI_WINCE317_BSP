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
/** \file healthMonitor.c
 *  \brief Firmware Recovery Mechanizem
 *
 */

/****************************************************************************/
/*                                                                          */
/*      MODULE:     healthMonitor.h                                         */
/*      PURPOSE:    Driver interface to OS abstraction layer                */
/*                                                                          */
/****************************************************************************/

#ifndef HEALTHMONITOR_H
#define HEALTHMONITOR_H

#include "paramOut.h"
#include "DrvMainModules.h"


/* Config manager state machine defintions */
typedef enum healthMonitorState_e
{
    HEALTH_MONITOR_STATE_DISCONNECTED,
    HEALTH_MONITOR_STATE_SCANNING,
    HEALTH_MONITOR_STATE_CONNECTED 

} healthMonitorState_e;


/* Public Functions Prototypes */
TI_HANDLE healthMonitor_create         (TI_HANDLE hOs);
void healthMonitor_init                (TStadHandlesList *pStadHandles);
TI_STATUS healthMonitor_SetDefaults    (TI_HANDLE hHealthMonitor, healthMonitorInitParams_t *healthMonitorInitParams);
TI_STATUS healthMonitor_unload         (TI_HANDLE hHealthMonitor);
void healthMonitor_PerformTest         (TI_HANDLE hHealthMonitor, TI_BOOL bTwdInitOccured);
void healthMonitor_setState            (TI_HANDLE hHealthMonitor, healthMonitorState_e state);
void healthMonitor_sendFailureEvent    (TI_HANDLE hHealthMonitor, EFailureEvent failureEvent);
void healthMonitor_printFailureEvents  (TI_HANDLE hHealthMonitor);
TI_STATUS healthMonitor_SetParam       (TI_HANDLE hHealthMonitor, paramInfo_t *pParam);
TI_STATUS healthMonitor_GetParam       (TI_HANDLE hHealthMonitor, paramInfo_t *pParam);


#endif
