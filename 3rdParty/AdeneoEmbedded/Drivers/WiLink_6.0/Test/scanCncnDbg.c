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
/** \file  ScanCncnDbg.c
 *  \brief This file include the scan concentrator debug module implementation
 *
 *  \see   ScanCncn.h, ScanCncn.c
 */


#include "ScanCncn.h"
#include "scanCncnDbg.h"
#include "report.h"
#include "TWDriver.h"


/**
 * \\n
 * \date 14-Feb-2005\n
 * \brief Main scan concentrator debug function
 *
 * Function Scope \e Public.\n
 * \param hScanCncn - handle to the scan concentrator object.\n
 * \param funcType - the specific debug function.\n
 * \param pParam - parameters for the debug function.\n
 */
void scanConcentratorDebugFunction( TI_HANDLE hScanCncn, TI_HANDLE hTWD, TI_UINT32 funcType, void *pParam )
{
    switch (funcType)
    {
    case DBG_SCAN_CNCN_PRINT_HELP:
        printScanConcentratorDbgFunctions();
        break;

    case DBG_SCAN_SRV_PRINT_STATUS:
        TWD_PrintMacServDebugStatus (hTWD);
        break;

    default:
        WLAN_OS_REPORT(("Invalid function type in scan concentrator debug function: %d\n", funcType));
        break;
    }
}

/**
 * \\n
 * \date 14-Feb-2005\n
 * \brief Prints scan concentrator debug menu
 *
 * Function Scope \e Public.\n
 */
void printScanConcentratorDbgFunctions(void)
{
    WLAN_OS_REPORT(("   Scan Concentrator Debug Functions   \n"));
    WLAN_OS_REPORT(("---------------------------------------\n"));
    WLAN_OS_REPORT(("1400 - Print the scan concentrator Debug Help\n"));
    WLAN_OS_REPORT(("1401 - Print the scan SRV status\n"));
}

