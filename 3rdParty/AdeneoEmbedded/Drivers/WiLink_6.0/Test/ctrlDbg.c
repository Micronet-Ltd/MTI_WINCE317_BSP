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

/***************************************************************************/
/*																									*/
/*		MODULE:																	*/
/*    PURPOSE:										*/
/*									*/
/***************************************************************************/
#include "tidef.h"
#include "Ctrl.h"
#include "DataCtrl_Api.h"
#include "dataCtrlDbg.h"
#include "osApi.h"
#include "report.h"
#include "TWDriver.h"
#include "paramOut.h"

void printCtrlDbgFunctions(void);


/*************************************************************************
 *																		 *
 *************************************************************************
DESCRIPTION:                  
                                                      
INPUT:       

OUTPUT:      

RETURN:     
                                                   
************************************************************************/
void ctrlDebugFunction(TI_HANDLE hCtrlData, TI_UINT32 funcType, void *pParam)
{
	ctrlData_t *pCtrlData = (ctrlData_t *)hCtrlData;
	paramInfo_t paramInfo;

	switch ((ECtrlDbgFunc)funcType)
	{
	case CTRL_PRINT_DBG_FUNCTIONS:
		printCtrlDbgFunctions();
		break;

	case CTRL_PRINT_CTRL_BLOCK:
		WLAN_OS_REPORT(("CTRL DBG - Print Ctrl Block \n\n"));
		ctrlData_printCtrlBlock(pCtrlData);
		break;

	case CTRL_PRINT_TX_PARAMETERS:
		WLAN_OS_REPORT(("CTRL DBG - Print tx paramters \n\n"));
		ctrlData_printTxParameters(pCtrlData);
		break;

	case CTRL_SET_CTS_TO_SELF:
		WLAN_OS_REPORT(("CTRL DBG - Set CtsToSelf = %s\n",((*(TI_UINT8*)pParam > 0) ? "TI_TRUE" : "TI_FALSE")));
		paramInfo.paramType = CTRL_DATA_CURRENT_PROTECTION_STATUS_PARAM;
		paramInfo.content.ctrlDataProtectionEnabled = ((*(TI_UINT8*)pParam > 0) ? 1 : 0);
		ctrlData_setParam(pCtrlData,&paramInfo);
		break;

	default:
		WLAN_OS_REPORT(("Invalid function type in Debug Ctrl Function Command: %d\n\n", funcType));
		break;
	}
} 


void printCtrlDbgFunctions(void)
{
	WLAN_OS_REPORT(("   Ctrl Dbg Functions   \n"));
	WLAN_OS_REPORT(("------------------------\n"));
	WLAN_OS_REPORT(("400 - Print this menu.\n"));
	WLAN_OS_REPORT(("401 - Print Ctrl Block.\n"));
	WLAN_OS_REPORT(("402 - Print Tx parameters.\n"));
  	WLAN_OS_REPORT(("403 - Set CtsToSelf.\n"));
}



