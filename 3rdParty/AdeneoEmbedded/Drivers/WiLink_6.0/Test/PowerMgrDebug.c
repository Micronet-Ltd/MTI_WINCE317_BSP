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
/** \file PowerMgrDebug.c
 *  \brief This is the PowerMgrDebug module implementation.
 *  \
 *  \date 13-Jun-2004
 */

/****************************************************************************
 *                                                                          *
 *   MODULE:  PowerMgrDebug                                                      *
 *   PURPOSE: PowerMgrDebug Module implementation.                               *
 *                                                                          *
 ****************************************************************************/

#include "tidef.h"
#include "osApi.h"
#include "report.h"
#include "PowerMgrDebug.h"
#include "PowerMgr_API.h"
#include "PowerMgr.h"


/*****************************************************************************
 **         Private Function prototypes                                     **
 *****************************************************************************/
static void PowerMgrDebug_printAllCommands(void);


/*****************************************************************************
 **         Public Function prototypes                                      **
 *****************************************************************************/
void powerMgrDebugFunction(TI_HANDLE thePowerMgrHandle,
                           TI_UINT32 theDebugFunction,
                           void* theParameter)
{
    switch (theDebugFunction)
    {
    case POWER_MGR_DEBUG_PRINT_ALL_COMMANDS:
        PowerMgrDebug_printAllCommands();
        break;

    case POWER_MGR_DEBUG_START_PS:
        PowerMgr_startPS(thePowerMgrHandle);
        break;

    case POWER_MGR_DEBUG_STOP_PS:
        PowerMgr_stopPS(thePowerMgrHandle, TI_TRUE);
        break;

   case POWER_MGR_PRINT_OBJECTS:
        PowerMgr_printObject(thePowerMgrHandle);
        break;

    default:
        WLAN_OS_REPORT(("(%d) - ERROR - Invalid function type in POWER MANAGER DEBUG Function Command: %d\n",
                        __LINE__,theDebugFunction));
        break;
    }
}

/*****************************************************************************
 **         Private Function prototypes                                     **
 *****************************************************************************/
static void PowerMgrDebug_printAllCommands(void)
{
    WLAN_OS_REPORT(("\n\n"));
    WLAN_OS_REPORT(("POWER MGR DEBUG module commands:\n"));
    WLAN_OS_REPORT(("================================\n"));
    WLAN_OS_REPORT(("syntax         description\n"));
    WLAN_OS_REPORT(("%d -  POWER_MGR_DEBUG_PRINT_ALL_COMMANDS\n", POWER_MGR_DEBUG_PRINT_ALL_COMMANDS));
    WLAN_OS_REPORT(("%d -  POWER_MGR_DEBUG_START_PS\n", POWER_MGR_DEBUG_START_PS));
    WLAN_OS_REPORT(("%d -  POWER_MGR_DEBUG_STOP_PS\n", POWER_MGR_DEBUG_STOP_PS));
    WLAN_OS_REPORT(("%d -  POWER_MGR_PRINT_OBJECTS\n", POWER_MGR_PRINT_OBJECTS));
    WLAN_OS_REPORT(("\n\n"));
}

