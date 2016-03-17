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
/*																			*/
/*		MODULE:																*/
/*    PURPOSE:																*/
/*												   							*/ 						
/****************************************************************************/
/* #include "osTITypes.h" */
#include "osApi.h"
#include "rsnApi.h"
#include "rsn.h"
#include "report.h"
#include "paramOut.h"
#include "rsnDbg.h"
#ifdef XCC_MODULE_INCLUDED
#include "XCCMngr.h"
#endif

void printRsnDbgFunctions(void);
void printRogueApTable(TI_HANDLE hRogueAp);

static TI_UINT8 infoBuf[480];

/*************************************************************************
 *																		 *
 *************************************************************************
DESCRIPTION:                  
                                                      
INPUT:       

OUTPUT:      

RETURN:     
                                                   
************************************************************************/
void rsnDebugFunction(TI_HANDLE hRsn, TI_UINT32 funcType, void *pParam)
{
	paramInfo_t 	param, *pRsnParam;
	TI_UINT32			value;
    rsnAuthEncrCapability_t rsnAuthEncrCap;

	switch (funcType)
	{
	case DBG_RSN_PRINT_HELP:
		printRsnDbgFunctions();
		break;

	case DBG_RSN_SET_DESIRED_AUTH:
		WLAN_OS_REPORT(("RSN DBG - Set desired Authentication suite \n"));
		value = *(TI_UINT32*)pParam;

		param.paramType = RSN_EXT_AUTHENTICATION_MODE;
		param.content.rsnDesiredAuthType = (EAuthSuite)value;

		rsn_setParam(hRsn, &param);
		break;

	case DBG_RSN_SET_DESIRED_CIPHER:
		WLAN_OS_REPORT(("RSN DBG - Set desired cipher suite \n"));
		value = *(TI_UINT32*)pParam;

		param.paramType = RSN_ENCRYPTION_STATUS_PARAM;
		param.content.rsnEncryptionStatus = (ECipherSuite)value;

		rsn_setParam(hRsn, &param);
		break;
	

	case DBG_RSN_GEN_MIC_FAILURE_REPORT:
		value = *(TI_UINT32*)pParam;
		/* generate unicast mic failure report to the OS and to the RSN module */
		rsn_reportMicFailure(hRsn, (TI_UINT8*)&value,1);
		break;

	case DBG_RSN_GET_PARAM_802_11_CAPABILITY:
			
		param.paramType = RSN_AUTH_ENCR_CAPABILITY;
        param.content.pRsnAuthEncrCapability = &rsnAuthEncrCap;
	
        /* Get 802_11 capability info */
		rsn_getParam(hRsn, &param);
		break;
		
	case DBG_RSN_GET_PMKID_CACHE:
		
		pRsnParam = (paramInfo_t *)&infoBuf;
		pRsnParam->paramType = RSN_PMKID_LIST;
		pRsnParam->paramLength = 480;

		/* Get PMKID list */
		rsn_getParam(hRsn, pRsnParam);
		break;

	case DBG_RSN_RESET_PMKID_CACHE:
		
		rsn_resetPMKIDList(hRsn);

		break;
#ifdef XCC_MODULE_INCLUDED
    case DBG_RSN_PRINT_ROGUE_AP_TABLE:
        printRogueApTable(((XCCMngr_t*)((rsn_t*)hRsn)->hXCCMngr)->hRogueAp);
        break;
#endif

    case DBG_RSN_SET_PORT_STATUS:
        WLAN_OS_REPORT(("Setting PORT STATUS to open\n"));
        rsn_setPortStatus(hRsn,TI_TRUE);
        break;

    case DBG_RSN_PRINT_PORT_STATUS:
        {
            TI_BOOL portStatus = TI_FALSE;
            portStatus = rsn_getPortStatus(((rsn_t*)hRsn));
            WLAN_OS_REPORT(("\n\nPORT is %s !!\n",(portStatus)?"OPEN":"CLOSE"));
        }
       
        break;
	default:
		WLAN_OS_REPORT(("Invalid function type in RSN Function Command: %d\n", funcType));
		break;
	}

} 


void printRsnDbgFunctions(void)
{
	WLAN_OS_REPORT(("   Rsn Debug Functions   \n"));
	WLAN_OS_REPORT(("-------------------------\n"));

	WLAN_OS_REPORT(("702 - Set default key id \n"));
	WLAN_OS_REPORT(("703 - Set desired Authentication suite \n"));
	WLAN_OS_REPORT(("704 - Set desired cipher suite \n"));

	WLAN_OS_REPORT(("706 - Generate MIC FAILURE report (after 2 clicks less then 1 minute - disassociate)\n"));
	WLAN_OS_REPORT(("707 - Get 802.11 authentication/encryption capability\n"));
	WLAN_OS_REPORT(("708 - Get PMKID cache \n"));
	WLAN_OS_REPORT(("709 - ReSet PMKID cache  \n"));
	WLAN_OS_REPORT(("710 - Print Rogue AP table\n"));


}
