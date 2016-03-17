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
/** \file ScanMngrDbg.c
 *  \brief This file include the scan manager debug module implementation
 *  \
 *  \date 29-March-2005
 */

#include "tidef.h"
#include "report.h"
#include "paramOut.h"
#include "scanMngr.h"
#include "ScanMngrDbg.h"
#include "siteMgrApi.h"
#include "DataCtrl_Api.h"


/**
 * \\n
 * \date 29-March-2005\n
 * \brief Main scan manager debug function
 *
 * Function Scope \e Public.\n
 * \param hScanMngr - handle to the scan manager object.\n
 * \param funcType - the specific debug function.\n
 * \param pParam - parameters for the debug function.\n
 * \param hSiteMgr - handle to th esite manager object.\n
 * \param hCtrlData - handle to the data ctrl object.\n
 */
void scanMngrDebugFunction( TI_HANDLE hScanMngr, TI_UINT32 funcType, void *pParam, TI_HANDLE hSiteMgr, TI_HANDLE hCtrlData )
{
  	switch (funcType)
	{
	case DBG_SCAN_MNGR_PRINT_HELP:
		printScanMngrDbgFunctions();
		break;

	case DBG_SCAN_MNGR_START_CONT_SCAN:  
		startContScan( hScanMngr, hSiteMgr, hCtrlData );
		break;
		
	case DBG_SCAN_MNGR_STOP_CONT_SCAN:  
		scanMngr_stopContScan( hScanMngr );
		break;

    case DBG_SCAN_MNGR_START_IMMED_SCAN:
        scanMngr_startImmediateScan( hScanMngr, (1 == *((TI_INT32*)pParam) ? TI_TRUE : TI_FALSE) );
        break;

    case DBG_SCAN_MNGR_STOP_IMMED_SCAN:
        scanMngr_stopImmediateScan( hScanMngr );
        break;

    case DBG_SCAN_MNGR_PRINT_TRACK_LIST:
        scanMngrDebugPrintBSSList( hScanMngr );
        break;
        
    case DBG_SCAN_MNGR_PRINT_STATS:
        scanMngr_statsPrint( hScanMngr );
        break;

    case DBG_SCAN_MNGR_RESET_STATS:
        scanMngr_statsReset( hScanMngr );
        break;

    case DBG_SCAN_MNGR_PIRNT_NEIGHBOR_APS:
        scanMngrDebugPrintNeighborAPList( hScanMngr );
        break;

    case DBG_SCAN_MNGR_PRINT_POLICY:
        scanMngrTracePrintScanPolicy( &(((scanMngr_t*)hScanMngr)->scanPolicy) );
        break;

    case DBG_SCAN_MNGR_PRINT_OBJECT:
        scanMngrDebugPrintObject( hScanMngr );
        break;

    default:
		WLAN_OS_REPORT(("Invalid function type in scan manager debug function: %d\n", funcType));
		break;
	}
}

/**
 * \\n
 * \date 29-March-2005\n
 * \brief Prints scan manager debug menu
 *
 * Function Scope \e Public.\n
 */
void printScanMngrDbgFunctions(void)
{
    WLAN_OS_REPORT(("   Scan Manager Debug Functions   \n"));
	WLAN_OS_REPORT(("---------------------------------------\n"));
	WLAN_OS_REPORT(("1500 - Print the scan manager Debug Help\n"));
	WLAN_OS_REPORT(("1501 - Start continuous scan\n"));
	WLAN_OS_REPORT(("1502 - Stop continuous scan\n"));
    WLAN_OS_REPORT(("1503 - Start immediate scan\n"));
    WLAN_OS_REPORT(("1504 - Stop immediate scan\n"));
    WLAN_OS_REPORT(("1505 - Print tracking list\n"));
    WLAN_OS_REPORT(("1506 - Print statistics\n"));
    WLAN_OS_REPORT(("1507 - Reset statistics\n"));
    WLAN_OS_REPORT(("1508 - Print neighbor APs list\n"));
    WLAN_OS_REPORT(("1509 - Print Scan Policy\n"));
    WLAN_OS_REPORT(("1510 - Print scan manager object\n"));
}

/**
 * \\n
 * \date 29-March-2005\n
 * \brief Starts continuous scan process.\n
 *
 * Function Scope \e Public.\n
 * \param hScanMngr - handle to the scan manager object.\n
 * \param hSiteMgr - handle to the site manager object.\n\
 * \param hCtrlData - handle to the data ctrl object.\n
 */
void startContScan( TI_HANDLE hScanMngr, TI_HANDLE hSiteMgr, TI_HANDLE hCtrlData )
{
    paramInfo_t param;
    ERadioBand  radioBand;

    /* get current band */
    param.paramType = SITE_MGR_RADIO_BAND_PARAM;
    siteMgr_getParam( hSiteMgr, &param );
    radioBand = param.content.siteMgrRadioBand;

    /* get current BSSID */
	param.paramType = CTRL_DATA_CURRENT_BSSID_PARAM;
	ctrlData_getParam( hCtrlData, &param );
    
    /* start continuous scan */
    scanMngr_startContScan( hScanMngr, &(param.content.ctrlDataCurrentBSSID), radioBand );
}

