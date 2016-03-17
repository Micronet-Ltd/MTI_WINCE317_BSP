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
/** \file  ScanCncnOsSm.h
 *  \brief Scan concentartor OS state-machine public definitions
 *
 *  \see   ScanCncnOsSm.c
 */

#ifndef __SCAN_CNCN_OS_SM_H__
#define __SCAN_CNCN_OS_SM_H__

#include "osTIType.h"

typedef enum
{
    SCAN_CNCN_OS_SM_STATE_IDLE = 0,
    SCAN_CNCN_OS_SM_STATE_SCAN_ON_G,
    SCAN_CNCN_OS_SM_STATE_SCAN_ON_A,
    SCAN_CNCN_OS_SM_NUMBER_OF_STATES
} EScanCncnOsSmStates;

typedef enum
{
    SCAN_CNCN_OS_SM_EVENT_START_SCAN = 0,
    SCAN_CNCN_OS_SM_EVENT_SCAN_COMPLETE,
    SCAN_CNCN_OS_SM_NUMBER_OF_EVENTS
} EScanCncnOsSmEvents;

TI_HANDLE   scanCncnOsSm_Create (TI_HANDLE hScanCncn);
void        scanCncnOsSm_Init (TI_HANDLE hScanCncn);
void        scanCncnOsSm_Destroy (TI_HANDLE hScanCncn);

#endif /* __SCAN_CNCN_OS_SM_H__ */

