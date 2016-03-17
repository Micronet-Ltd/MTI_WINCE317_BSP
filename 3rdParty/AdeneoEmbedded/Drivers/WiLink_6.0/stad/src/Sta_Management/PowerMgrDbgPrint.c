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
/** \file PowerMgrDbgPrint.c
 *  \brief Includes primtoputs for debugging the power manager module.
 *  \
 *  \date 29-Aug-2006
 */

#ifndef __POWER_MGR_DBG_PRINT__
#define __POWER_MGR_DBG_PRINT__

#define __FILE_ID__  FILE_ID_72
#include "tidef.h"
#include "report.h"
#include "osApi.h"
#include "PowerMgr.h"

#ifdef TI_DBG

#define MAX_DESC_LENGTH 32

void powerMgrPrintPriorities( TI_HANDLE hPowerMgr, powerMngModePriority_t* pPriorities );


static char booleanDesc[ 2 ][ MAX_DESC_LENGTH ] = {"no", "yes"};
static char powerModeDesc[ POWER_MODE_MAX ][ MAX_DESC_LENGTH ] = 
                        { "Auto", "Active", "Short doze", "Long doze", "PS only" };
static char powerPolicyDesc[ POWERAUTHO_POLICY_NUM ][ MAX_DESC_LENGTH ] = 
                        { "ELP", "PD", "AWAKE" };
static char priorityDesc[ POWER_MANAGER_MAX_PRIORITY ][ MAX_DESC_LENGTH ] = 
                        { "User priority", "Soft-Gemini priority" };
static char psStatusDesc[ POWER_SAVE_STATUS_NUMBER ][ MAX_DESC_LENGTH ]=
                        { "Enter fail", "Enter success", "Exit fail", "Exit succes" };

/****************************************************************************************
*                        PowerMgr_printObject                                                          *
****************************************************************************************
DESCRIPTION: print configuration of the PowerMgr object - use for debug!
                                                                                                                              
INPUT:          - hPowerMgr             - Handle to the Power Manager
OUTPUT:     
RETURN:    void.\n
****************************************************************************************/
void PowerMgr_printObject( TI_HANDLE hPowerMgr )
{
    PowerMgr_t *pPowerMgr = (PowerMgr_t*)hPowerMgr;

    WLAN_OS_REPORT(("------------ Power Manager Object ------------\n\n"));
    WLAN_OS_REPORT(("PS enabled: %s, desired power mode profile: %s, last power mode profile: %s\n",
                    booleanDesc[ pPowerMgr->psEnable ], powerModeDesc[ pPowerMgr->desiredPowerModeProfile ],
                    powerModeDesc[ pPowerMgr->lastPowerModeProfile ]));
    WLAN_OS_REPORT(("Default power policy: %ss, PS power policy: %s\n", 
                    powerPolicyDesc[ pPowerMgr->defaultPowerLevel ],
                    powerPolicyDesc[ pPowerMgr->PowerSavePowerLevel ]));
    WLAN_OS_REPORT(("Current priority: %s\n", priorityDesc[ pPowerMgr->powerMngPriority ]));
    powerMgrPrintPriorities( hPowerMgr, pPowerMgr->powerMngModePriority );
    WLAN_OS_REPORT(("\n------------ auto mode parameters ------------\n"));
    WLAN_OS_REPORT(("Interval: %d, active threshold: %d, doze threshold: %d, doze mode: %s\n\n",
                    pPowerMgr->autoModeInterval, pPowerMgr->autoModeActiveTH, pPowerMgr->autoModeDozeTH,
                    powerModeDesc[ pPowerMgr->autoModeDozeMode ]));
    WLAN_OS_REPORT(("Beacon listen interval:%d, DTIM listen interval:%d, last PS status: %s\n\n",
                    pPowerMgr->beaconListenInterval, pPowerMgr->dtimListenInterval,
                    psStatusDesc[ pPowerMgr->lastPsTransaction ]));
    WLAN_OS_REPORT(("------------ Handles ------------\n"));
    WLAN_OS_REPORT(("%-15s: 0x%x, %-15s:0x%x\n","hOS", pPowerMgr->hOS, "hTWD", pPowerMgr->hTWD));
    WLAN_OS_REPORT(("%-15s: 0x%x, %-15s:0x%x\n","hReport", pPowerMgr->hReport, "hTrafficMonitor", pPowerMgr->hTrafficMonitor));
    WLAN_OS_REPORT(("%-15s: 0x%x, %-15s:0x%x\n","hSiteMgr", pPowerMgr->hSiteMgr, "hTWD", pPowerMgr->hTWD));
    WLAN_OS_REPORT(("%-15s: 0x%x, %-15s:0x%x\n","hRetryPsTimer", pPowerMgr->hRetryPsTimer, "hActiveTMEvent", pPowerMgr->passToActiveTMEvent));
    WLAN_OS_REPORT(("%-15s: 0x%x\n", "hDozeTMEvent", pPowerMgr->passToDozeTMEvent));
}

void powerMgrPrintPriorities( TI_HANDLE hPowerMgr, powerMngModePriority_t* pPriorities )
{
    int i;

    for ( i = 0; i < POWER_MANAGER_MAX_PRIORITY; i++ )
    {
        WLAN_OS_REPORT(("Priority: %-15s, enabled: %s, power mode: %s\n",
                        priorityDesc[ i ], booleanDesc[ pPriorities[ i ].priorityEnable ],
                         powerModeDesc[ pPriorities[ i ].powerMode ]));
    }
}
#endif /* TI_DBG */

#endif /* __POWER_MGR_DBG_PRINT__ */
