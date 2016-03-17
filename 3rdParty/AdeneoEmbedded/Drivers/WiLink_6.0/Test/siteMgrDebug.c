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
/** \file reportReplvl.c
 *  \brief Report level implementation
 * 
 *  \see reportReplvl.h
 */

/** \file   siteMgrDebug.c 
 *  \brief  The siteMgrDebug module.
 *  
 *  \see    siteMgrDebug.h
 */

#include "tidef.h"
#include "osApi.h"
#include "paramOut.h"
#include "siteMgrDebug.h"
#include "siteMgrApi.h"
#include "siteHash.h"
#include "report.h"
#include "CmdDispatcher.h"
#include "DrvMainModules.h"
#include "sme.h"
#include "apConn.h"
#include "healthMonitor.h"
#include "conn.h"
#include "connApi.h"

#ifdef XCC_MODULE_INCLUDED
#include "XCCMngr.h"
#endif


static void printPrimarySite(siteMgr_t *pSiteMgr);

void printSiteTable(siteMgr_t *pSiteMgr, char *desiredSsid);

static void printDesiredParams(siteMgr_t *pSiteMgr, TI_HANDLE hCmdDispatch);

static void printPrimarySiteDesc(siteMgr_t *pSiteMgr, OS_802_11_BSSID *pPrimarySiteDesc);

static void setRateSet(TI_UINT8 maxRate, TRates *pRates);

void printSiteMgrHelpMenu(void);

/*	Function implementation */
void siteMgrDebugFunction (TI_HANDLE         hSiteMgr, 
                           TStadHandlesList *pStadHandles,
                           TI_UINT32         funcType, 
                           void             *pParam)
{
	siteMgr_t *pSiteMgr = (siteMgr_t *)hSiteMgr;
	paramInfo_t		param;
	TSsid			newDesiredSsid;
	TI_UINT8		value;
	TI_UINT8		i;
	OS_802_11_BSSID primarySiteDesc;
	TRates			ratesSet;


	newDesiredSsid.len = 5;
	os_memoryCopy(pSiteMgr->hOs, (void *)newDesiredSsid.str, "yaeli", 5);

	
	switch (funcType)
	{
	case SITE_MGR_DEBUG_HELP_MENU:
		printSiteMgrHelpMenu();
		break;

	case PRIMARY_SITE_DBG:
		printPrimarySite(pSiteMgr);
		break;

	case SITE_TABLE_DBG:
		printSiteTable(pSiteMgr, NULL);
		break;

	case DESIRED_PARAMS_DBG:
		printDesiredParams(pSiteMgr, pStadHandles->hCmdDispatch);
		break;

	case GET_PRIMARY_SITE_DESC_DBG:
		param.paramType = SITE_MGR_GET_SELECTED_BSSID_INFO;
		param.content.pSiteMgrPrimarySiteDesc = &primarySiteDesc;
		cmdDispatch_GetParam(pStadHandles->hCmdDispatch, &param);
		printPrimarySiteDesc(pSiteMgr, &primarySiteDesc);
		break;

	case SET_RSN_DESIRED_CIPHER_SUITE_DBG:
		param.paramType = RSN_ENCRYPTION_STATUS_PARAM;
		value = *((TI_UINT32 *)pParam);
		param.content.rsnEncryptionStatus = (ECipherSuite)value;
		cmdDispatch_SetParam(pStadHandles->hCmdDispatch, &param); 
		WLAN_OS_REPORT(("\nSetting RSN_DESIRED_CIPHER_SUITE_PARAM : %d\r\n", value));
		break;

	case GET_RSN_DESIRED_CIPHER_SUITE_DBG:
		param.paramType = RSN_ENCRYPTION_STATUS_PARAM;
		cmdDispatch_GetParam(pStadHandles->hCmdDispatch, &param);
		WLAN_OS_REPORT(("\nGetting RSN_DESIRED_CIPHER_SUITE_PARAM: %d\r\n", param.content.rsnEncryptionStatus));
		break;
 
	case SET_RSN_DESIRED_AUTH_TYPE_DBG:
		param.paramType = RSN_EXT_AUTHENTICATION_MODE;
		value = *((TI_UINT32 *)pParam);
		param.content.rsnDesiredAuthType = (EAuthSuite)value;
		cmdDispatch_SetParam(pStadHandles->hCmdDispatch, &param); 
		if (value == RSN_AUTH_OPEN)
			WLAN_OS_REPORT(("\nSetting RSN_DESIRED_AUTH_TYPE_PARAM:	RSN_AUTH_OPEN\r\n"));
		else if (value == RSN_AUTH_SHARED_KEY)
			WLAN_OS_REPORT(("\nSetting RSN_DESIRED_AUTH_TYPE_PARAM:	RSN_AUTH_SHARED_KEY\r\n"));
		else if (value == RSN_AUTH_AUTO_SWITCH)
			WLAN_OS_REPORT(("\nSetting RSN_DESIRED_AUTH_TYPE_PARAM:	RSN_AUTH_AUTO_SWITCH\r\n"));
		else 
			WLAN_OS_REPORT(("\nSetting RSN_DESIRED_AUTH_TYPE_PARAM:	Invalid: %d\r\n", value));
		break;

	case GET_RSN_DESIRED_AUTH_TYPE_DBG:
		param.paramType = RSN_EXT_AUTHENTICATION_MODE;
		cmdDispatch_GetParam(pStadHandles->hCmdDispatch, &param);
		if (param.content.rsnDesiredAuthType == RSN_AUTH_OPEN)
			WLAN_OS_REPORT(("\nGetting RSN_DESIRED_AUTH_TYPE_PARAM:	RSN_AUTH_OPEN\r\n"));
		else if (param.content.rsnDesiredAuthType == RSN_AUTH_SHARED_KEY)
			WLAN_OS_REPORT(("\nGetting RSN_DESIRED_AUTH_TYPE_PARAM:	RSN_AUTH_SHARED_KEY\r\n"));
		else if (param.content.rsnDesiredAuthType == RSN_AUTH_AUTO_SWITCH)
			WLAN_OS_REPORT(("\nGetting RSN_DESIRED_AUTH_TYPE_PARAM:	RSN_AUTH_AUTO_SWITCH\r\n"));
		else 
			WLAN_OS_REPORT(("\nGetting RSN_DESIRED_AUTH_TYPE_PARAM:	Invalid: %d\r\n", param.content.rsnDesiredAuthType));

		break;

	case GET_CONNECTION_STATUS_DBG:
		param.paramType = SME_CONNECTION_STATUS_PARAM;
		cmdDispatch_GetParam(pStadHandles->hCmdDispatch, &param);
		if (param.content.smeSmConnectionStatus == eDot11Idle)
			WLAN_OS_REPORT(("\nGetting SITE_MGR_CONNECTION_STATUS_PARAM:	STATUS_IDLE\r\n"));
		else if (param.content.smeSmConnectionStatus == eDot11Scaning)
			WLAN_OS_REPORT(("\nGetting SITE_MGR_CONNECTION_STATUS_PARAM:	STATUS_SCANNING\r\n"));
		else if (param.content.smeSmConnectionStatus == eDot11Connecting)
			WLAN_OS_REPORT(("\nGetting SITE_MGR_CONNECTION_STATUS_PARAM:	STATUS_CONNECTING\r\n"));
		else if (param.content.smeSmConnectionStatus == eDot11Associated)
			WLAN_OS_REPORT(("\nGetting SITE_MGR_CONNECTION_STATUS_PARAM:	STATUS_ASSOCIATED\r\n"));
		else if (param.content.smeSmConnectionStatus == eDot11Disassociated)
			WLAN_OS_REPORT(("\nGetting SITE_MGR_CONNECTION_STATUS_PARAM:	STATUS_DIS_ASSOCIATED\r\n"));
        else if (param.content.smeSmConnectionStatus == eDot11RadioDisabled)
            WLAN_OS_REPORT(("\nGetting SITE_MGR_CONNECTION_STATUS_PARAM:	STATUS_RADIO_DISABLED\r\n"));
        else 
			WLAN_OS_REPORT(("\nGetting SITE_MGR_CONNECTION_STATUS_PARAM:	STATUS_ERROR\r\n"));
		break;

	case SET_SUPPORTED_RATE_SET_DBG:
		param.paramType = SITE_MGR_DESIRED_SUPPORTED_RATE_SET_PARAM;
		value = *((TI_UINT32 *)pParam);
		setRateSet(value, &ratesSet);
		os_memoryCopy(pSiteMgr->hOs, &(param.content.siteMgrDesiredSupportedRateSet), &(ratesSet), sizeof(TRates));
		WLAN_OS_REPORT(("\nSetting SET_SUPPORTED_RATE_SET_DBG\r\n"));
		cmdDispatch_SetParam(pStadHandles->hCmdDispatch, &param);
		break;

	case GET_SUPPORTED_RATE_SET_DBG:
		param.paramType = SITE_MGR_DESIRED_SUPPORTED_RATE_SET_PARAM;
		cmdDispatch_GetParam(pStadHandles->hCmdDispatch, &param);
		WLAN_OS_REPORT(("\nGetting SITE_MGR_DESIRED_SUPPORTED_RATE_SET_PARAM\r\n"));
		if(param.content.siteMgrDesiredSupportedRateSet.len == 0)
			WLAN_OS_REPORT(("\nNo rates defined\r\n"));
		else 
		{	
			/* It looks like it never happens. Anyway decided to check */
            if ( param.content.siteMgrDesiredSupportedRateSet.len > DOT11_MAX_SUPPORTED_RATES )
            {
                WLAN_OS_REPORT(("siteMgrDebugFunction. param.content.siteMgrDesiredSupportedRateSet.len=%d exceeds the limit %d\r\n",
                         param.content.siteMgrDesiredSupportedRateSet.len, DOT11_MAX_SUPPORTED_RATES));
                handleRunProblem(PROBLEM_BUF_SIZE_VIOLATION);
                param.content.siteMgrDesiredSupportedRateSet.len = DOT11_MAX_SUPPORTED_RATES;
            }
            for (i = 0; i < param.content.siteMgrDesiredSupportedRateSet.len; i++)
				WLAN_OS_REPORT(("\nRate %d is 0x%X\r\n", i +1, param.content.siteMgrDesiredSupportedRateSet.ratesString[i]));
		}
		break;

	case SET_MLME_LEGACY_AUTH_TYPE_DBG:
		param.paramType = MLME_LEGACY_TYPE_PARAM;
		value = *((TI_UINT32 *)pParam);
		param.content.mlmeLegacyAuthType = (legacyAuthType_e)value;
		cmdDispatch_SetParam(pStadHandles->hCmdDispatch, &param); 
		if (value == AUTH_LEGACY_OPEN_SYSTEM)
			WLAN_OS_REPORT(("\nSetting MLME_LEGACY_TYPE_PARAM:	AUTH_LEGACY_OPEN_SYSTEM\r\n"));
		else if (value == AUTH_LEGACY_SHARED_KEY)
			WLAN_OS_REPORT(("\nSetting MLME_LEGACY_TYPE_PARAM:	AUTH_LEGACY_SHARED_KEY\r\n"));
		else if (value == AUTH_LEGACY_AUTO_SWITCH)
			WLAN_OS_REPORT(("\nSetting MLME_LEGACY_TYPE_PARAM:	AUTH_LEGACY_AUTO_SWITCH\r\n"));
		else 
			WLAN_OS_REPORT(("\nSetting MLME_LEGACY_TYPE_PARAM:	Invalid: %d\r\n", value));
		break;

	case GET_MLME_LEGACY_AUTH_TYPE_DBG:
		param.paramType = MLME_LEGACY_TYPE_PARAM;
		cmdDispatch_GetParam(pStadHandles->hCmdDispatch, &param);
		if (param.content.mlmeLegacyAuthType == AUTH_LEGACY_OPEN_SYSTEM)
			WLAN_OS_REPORT(("\nGetting MLME_LEGACY_TYPE_PARAM:	AUTH_LEGACY_OPEN_SYSTEM\r\n"));
		else if (param.content.rsnDesiredAuthType == AUTH_LEGACY_SHARED_KEY)
			WLAN_OS_REPORT(("\nGetting MLME_LEGACY_TYPE_PARAM:	AUTH_LEGACY_SHARED_KEY\r\n"));
		else if (param.content.rsnDesiredAuthType == AUTH_LEGACY_AUTO_SWITCH)
			WLAN_OS_REPORT(("\nGetting MLME_LEGACY_TYPE_PARAM:	AUTH_AUTO_SWITCH\r\n"));
		else 
			WLAN_OS_REPORT(("\nGetting MLME_LEGACY_TYPE_PARAM:	Invalid: %d\r\n", param.content.rsnDesiredAuthType));

		break;


	case RADIO_STAND_BY_CHANGE_STATE:
		WLAN_OS_REPORT(("\nChange GPIO-13 State...\r\n"));
		break;
		

    case PRINT_FAILURE_EVENTS:
        {

		WLAN_OS_REPORT(("\r\n PRINT HEALTH MONITOR LOG\r\n"));
		healthMonitor_printFailureEvents (pStadHandles->hHealthMonitor);
		apConn_printStatistics(pStadHandles->hAPConnection);
        conn_ibssPrintStatistics(pStadHandles->hConn);
        if (((conn_t*)pStadHandles->hConn)->currentConnType==CONNECTION_INFRA)
        {
            switch (((conn_t*)pStadHandles->hConn)->state)
            {
            case   0:  WLAN_OS_REPORT((" CONN state is IDLE\r\n")); 
                break;       
             case   1:  WLAN_OS_REPORT((" CONN state is SCR_WAIT\r\n")); 
                break;             
             case   2:  WLAN_OS_REPORT((" CONN state is WAIT_JOIN_CMPLT\r\n")); 
                break;            
             case   3:  WLAN_OS_REPORT((" CONN state is MLME_WAIT\r\n")); 
                break;           
             case   4:  WLAN_OS_REPORT((" CONN state is RSN_WAIT\r\n")); 
                break;            
             case   5:  WLAN_OS_REPORT((" CONN state is CONFIG_HW\r\n")); 
                break;        
             case   6:  WLAN_OS_REPORT((" CONN state is CONNECTED\r\n")); 
                break; 
            case   7:  WLAN_OS_REPORT((" CONN state is DISASSOCC\r\n")); 
               break; 
            default:
                break;
            }
        }
        }
        break;

	case FORCE_HW_RESET_RECOVERY:
		WLAN_OS_REPORT(("\r\n Currently not supported!\r\n"));
		break;

	case FORCE_SOFT_RECOVERY:
		WLAN_OS_REPORT(("\r\n FORCE Full Recovery (Soft)\r\n"));
		break;


	case PERFORM_HEALTH_TEST:
		WLAN_OS_REPORT(("\r\n PERFORM_HEALTH_TEST \r\n"));
		healthMonitor_PerformTest(pStadHandles->hHealthMonitor, TI_FALSE);	
		break;

	case PRINT_SITE_TABLE_PER_SSID:
		printSiteTable(pSiteMgr, (char*)pParam);
		break;

	case SET_DESIRED_CHANNEL:
		param.paramType = SITE_MGR_DESIRED_CHANNEL_PARAM;
		param.content.siteMgrDesiredChannel = *(TI_UINT8*)pParam;
		siteMgr_setParam(pStadHandles->hSiteMgr, &param);
		break;

	default:
		WLAN_OS_REPORT(("Invalid function type in Debug Site Manager Function Command: %d\r\n\r\n", funcType));
		break;
	}
} 

static void printPrimarySite(siteMgr_t *pSiteMgr)
{
	siteEntry_t *pSiteEntry;
    TI_UINT8	len;
	char	ssid[MAX_SSID_LEN + 1];
	
	pSiteEntry = pSiteMgr->pSitesMgmtParams->pPrimarySite;
	
	if (pSiteEntry == NULL)
	{
		WLAN_OS_REPORT(("\r\n************************	PRIMARY SITE IS NULL	****************************\r\n"));
		return;
	}

	WLAN_OS_REPORT(("\r\n************************	PRIMARY SITE	****************************\r\n"));
	
	WLAN_OS_REPORT(("BSSID			%2X-%2X-%2X-%2X-%2X-%2X	",	
														pSiteEntry->bssid[0], 
														pSiteEntry->bssid[1], 
														pSiteEntry->bssid[2], 
														pSiteEntry->bssid[3], 
														pSiteEntry->bssid[4], 
														pSiteEntry->bssid[5]));
    len = pSiteEntry->ssid.len;
    /* It looks like it never happens. Anyway decided to check */
    if ( pSiteEntry->ssid.len > MAX_SSID_LEN )
    {
        WLAN_OS_REPORT(("printPrimarySite. pSiteEntry->ssid.len=%d exceeds the limit %d\r\n",
                   pSiteEntry->ssid.len, MAX_SSID_LEN));
        handleRunProblem(PROBLEM_BUF_SIZE_VIOLATION);
        len = MAX_SSID_LEN;
    }
	os_memoryCopy(pSiteMgr->hOs, ssid, (void *)pSiteEntry->ssid.str, len);
	ssid[len] = '\0';
	WLAN_OS_REPORT(("SSID			%s\r\n", ssid));

	if (pSiteEntry->bssType == BSS_INFRASTRUCTURE)
		WLAN_OS_REPORT(("BSS Type		INFRASTRUCTURE\r\n"));
	else if (pSiteEntry->bssType == BSS_INDEPENDENT)
		WLAN_OS_REPORT(("BSS Type		IBSS\r\n"));
	else if (pSiteEntry->bssType == BSS_ANY)
		WLAN_OS_REPORT(("BSS Type		ANY\r\n"));
	else
		WLAN_OS_REPORT(("BSS Type		INVALID\r\n"));


	WLAN_OS_REPORT(("Channel			%d\r\n", pSiteEntry->channel));

	WLAN_OS_REPORT(("\r\n"));

	switch (pSiteEntry->maxBasicRate)
	{
	case DRV_RATE_1M:
		WLAN_OS_REPORT(("Max Basic Rate		RATE_1M_BIT\r\n"));
		break;

	case DRV_RATE_2M:
		WLAN_OS_REPORT(("Max Basic Rate		RATE_2M_BIT\r\n"));
		break;

	case DRV_RATE_5_5M:
		WLAN_OS_REPORT(("Max Basic Rate		RATE_5_5M_BIT\r\n"));
		break;

	case DRV_RATE_11M:
		WLAN_OS_REPORT(("Max Basic Rate		RATE_11M_BIT\r\n"));
		break;

	case DRV_RATE_6M:
		WLAN_OS_REPORT(("Max Basic Rate		RATE_6M_BIT\r\n"));
		break;

	case DRV_RATE_9M:
		WLAN_OS_REPORT(("Max Basic Rate		RATE_9M_BIT\r\n"));
		break;

	case DRV_RATE_12M:
		WLAN_OS_REPORT(("Max Basic Rate		RATE_12M_BIT\r\n"));
		break;

	case DRV_RATE_18M:
		WLAN_OS_REPORT(("Max Basic Rate		RATE_18M_BIT\r\n"));
		break;

	case DRV_RATE_24M:
		WLAN_OS_REPORT(("Max Basic Rate		RATE_24M_BIT\r\n"));
		break;

	case DRV_RATE_36M:
		WLAN_OS_REPORT(("Max Basic Rate		RATE_36M_BIT\r\n"));
		break;

	case DRV_RATE_48M:
		WLAN_OS_REPORT(("Max Basic Rate		RATE_48M_BIT\r\n"));
		break;

	case DRV_RATE_54M:
		WLAN_OS_REPORT(("Max Basic Rate		RATE_54M_BIT\r\n"));
		break;

	default:
		WLAN_OS_REPORT(("Max Basic Rate		INVALID,  0x%X\r\n", pSiteEntry->maxBasicRate));
		break;
	}

	switch (pSiteEntry->maxActiveRate)
	{
	case DRV_RATE_1M:
		WLAN_OS_REPORT(("Max Active Rate		RATE_1M_BIT\r\n"));
		break;

	case DRV_RATE_2M:
		WLAN_OS_REPORT(("Max Active Rate		RATE_2M_BIT\r\n"));
		break;

	case DRV_RATE_5_5M:
		WLAN_OS_REPORT(("Max Active Rate		RATE_5_5M_BIT\r\n"));
		break;

	case DRV_RATE_11M:
		WLAN_OS_REPORT(("Max Active Rate		RATE_11M_BIT\r\n"));
		break;

	case DRV_RATE_22M:
		WLAN_OS_REPORT(("Max Active Rate		RATE_22M_BIT\r\n"));
		break;

	case DRV_RATE_6M:
		WLAN_OS_REPORT(("Max Active Rate		RATE_6M_BIT\r\n"));
		break;

	case DRV_RATE_9M:
		WLAN_OS_REPORT(("Max Active Rate		RATE_9M_BIT\r\n"));
		break;

	case DRV_RATE_12M:
		WLAN_OS_REPORT(("Max Active Rate		RATE_12M_BIT\r\n"));
		break;

	case DRV_RATE_18M:
		WLAN_OS_REPORT(("Max Active Rate		RATE_18M_BIT\r\n"));
		break;

	case DRV_RATE_24M:
		WLAN_OS_REPORT(("Max Active Rate		RATE_24M_BIT\r\n"));
		break;

	case DRV_RATE_36M:
		WLAN_OS_REPORT(("Max Active Rate		RATE_36M_BIT\r\n"));
		break;

	case DRV_RATE_48M:
		WLAN_OS_REPORT(("Max Active Rate		RATE_48M_BIT\r\n"));
		break;

	case DRV_RATE_54M:
		WLAN_OS_REPORT(("Max Active Rate		RATE_54M_BIT\r\n"));
		break;

	default:
		WLAN_OS_REPORT(("Max Active Rate		INVALID,  0x%X\r\n", pSiteEntry->maxActiveRate));
		break;
	}

	WLAN_OS_REPORT(("\r\n"));

	if (pSiteEntry->probeModulation == DRV_MODULATION_QPSK)
		WLAN_OS_REPORT(("Probe Modulation	QPSK\r\n"));
	else if (pSiteEntry->probeModulation == DRV_MODULATION_CCK)
		WLAN_OS_REPORT(("Probe Modulation	CCK\r\n"));
	else if (pSiteEntry->probeModulation == DRV_MODULATION_PBCC)
		WLAN_OS_REPORT(("Probe Modulation	PBCC\r\n"));
	else if (pSiteEntry->probeModulation == DRV_MODULATION_OFDM)
		WLAN_OS_REPORT(("Probe Modulation	OFDM\r\n"));
	else
		WLAN_OS_REPORT(("Probe Modulation	INVALID, %d\r\n", pSiteEntry->probeModulation));

	if (pSiteEntry->beaconModulation == DRV_MODULATION_QPSK)
		WLAN_OS_REPORT(("Beacon Modulation	QPSK\r\n"));
	else if (pSiteEntry->beaconModulation == DRV_MODULATION_CCK)
		WLAN_OS_REPORT(("Beacon Modulation	CCK\r\n"));
	else if (pSiteEntry->beaconModulation == DRV_MODULATION_PBCC)
		WLAN_OS_REPORT(("Beacon Modulation	PBCC\r\n"));
	else if (pSiteEntry->beaconModulation == DRV_MODULATION_OFDM)
		WLAN_OS_REPORT(("Beacon Modulation	OFDM\r\n"));
	else
		WLAN_OS_REPORT(("Beacon Modulation	INVALID, %d\r\n", pSiteEntry->beaconModulation));

	WLAN_OS_REPORT(("\r\n"));

	if (pSiteEntry->privacy == TI_TRUE)
		WLAN_OS_REPORT(("Privacy							        On\r\n\r\n"));
	else
		WLAN_OS_REPORT(("Privacy							        Off\r\n\r\n"));

	if (pSiteEntry->currentPreambleType == PREAMBLE_SHORT)
		WLAN_OS_REPORT(("Cap Preamble Type     Short\r\n"));
	else if (pSiteEntry->currentPreambleType == PREAMBLE_LONG)
		WLAN_OS_REPORT(("Cap Preamble Type     Long\r\n"));
	else
		WLAN_OS_REPORT(("Preamble	INVALID, %d\r\n", pSiteEntry->currentPreambleType));


	if(pSiteEntry->barkerPreambleType == PREAMBLE_UNSPECIFIED)
		WLAN_OS_REPORT(("Barker preamble Type		Unspecified\r\n"));
	else if(pSiteEntry->barkerPreambleType == PREAMBLE_SHORT)
		WLAN_OS_REPORT(("Barker_Preamble Type		Short\r\n"));
	else
		WLAN_OS_REPORT(("Barker_Preamble Type		Long\r\n"));

	if(pSiteEntry->currentSlotTime == PHY_SLOT_TIME_SHORT)
		WLAN_OS_REPORT(("Slot time type					   Short\r\n"));
	else
		WLAN_OS_REPORT(("Slot time type					   Long\r\n"));


	WLAN_OS_REPORT(("\r\n"));

	WLAN_OS_REPORT(("Beacon interval		%d\r\n", pSiteEntry->beaconInterval));

	WLAN_OS_REPORT(("Local Time Stamp	%d\r\n", pSiteEntry->localTimeStamp));

	WLAN_OS_REPORT(("rssi			%d\r\n", pSiteEntry->rssi));

	WLAN_OS_REPORT(("\r\n"));

	WLAN_OS_REPORT(("Fail status		%d\r\n", pSiteEntry->failStatus));

	WLAN_OS_REPORT(("\r\n---------------------------------------------------------------\r\n\r\n", NULL)); 

}

void printSiteTable(siteMgr_t *pSiteMgr, char *desiredSsid)
{
	TI_UINT8	i, numOfSites = 0;
	siteEntry_t *pSiteEntry;	
	char	ssid[MAX_SSID_LEN + 1];
    siteTablesParams_t      *pCurrentSiteTable = pSiteMgr->pSitesMgmtParams->pCurrentSiteTable;
    TI_UINT8                   tableIndex=2;

    WLAN_OS_REPORT(("\r\n\r\n************************	SITE TABLE	****************************\r\n\r\n\r\n"));
	
    /* It looks like it never happens. Anyway decided to check */
    if ( pCurrentSiteTable->maxNumOfSites > MAX_SITES_BG_BAND )
    {
        WLAN_OS_REPORT(("printSiteTable. pCurrentSiteTable->maxNumOfSites=%d exceeds the limit %d\r\n",
                   pCurrentSiteTable->maxNumOfSites, MAX_SITES_BG_BAND));
        handleRunProblem(PROBLEM_BUF_SIZE_VIOLATION);
        pCurrentSiteTable->maxNumOfSites = MAX_SITES_BG_BAND;
    }

    do
	{
        tableIndex--;
		for (i = 0; i < pCurrentSiteTable->maxNumOfSites; i++)
		{
			pSiteEntry = &(pCurrentSiteTable->siteTable[i]);
	
			if (pSiteEntry->siteType == SITE_NULL)
				continue;
            /* It looks like it never happens. Anyway decided to check */
            if ( pCurrentSiteTable->siteTable[i].ssid.len > MAX_SSID_LEN )
            {
                WLAN_OS_REPORT(("printSiteTable. pCurrentSiteTable->siteTable[%d].ssid.len=%d exceeds the limit %d\r\n",
                         i, pCurrentSiteTable->siteTable[i].ssid.len, MAX_SSID_LEN));
                handleRunProblem(PROBLEM_BUF_SIZE_VIOLATION);
                pCurrentSiteTable->siteTable[i].ssid.len = MAX_SSID_LEN;
            }
			os_memoryCopy(pSiteMgr->hOs ,ssid, (void *)pCurrentSiteTable->siteTable[i].ssid.str, pCurrentSiteTable->siteTable[i].ssid.len);
			ssid[pCurrentSiteTable->siteTable[i].ssid.len] = '\0';
			
			if (desiredSsid != NULL)
			{
				int desiredSsidLength = 0;
				char * tmp = desiredSsid;

				while (tmp != '\0')
				{
					desiredSsidLength++;
					tmp++;
				}

				if (os_memoryCompare(pSiteMgr->hOs, (TI_UINT8 *)ssid, (TI_UINT8 *)desiredSsid, desiredSsidLength))
					continue;
			}
			
			WLAN_OS_REPORT(("SSID	%s\r\n\r\n", ssid));
	
			 
 
			if (pSiteEntry->siteType == SITE_PRIMARY)
				WLAN_OS_REPORT( ("	 ENTRY PRIMARY %d \r\n", numOfSites));
			else
				WLAN_OS_REPORT( ("	ENTRY %d\r\n", i));
	
			WLAN_OS_REPORT(("BSSID			%2X-%2X-%2X-%2X-%2X-%2X	\r\n",	
																pCurrentSiteTable->siteTable[i].bssid[0], 
																pCurrentSiteTable->siteTable[i].bssid[1], 
																pCurrentSiteTable->siteTable[i].bssid[2], 
																pCurrentSiteTable->siteTable[i].bssid[3], 
																pCurrentSiteTable->siteTable[i].bssid[4], 
																pCurrentSiteTable->siteTable[i].bssid[5]));
		
		
			if (pCurrentSiteTable->siteTable[i].bssType == BSS_INFRASTRUCTURE)
				WLAN_OS_REPORT(("BSS Type		INFRASTRUCTURE\r\n\r\n"));
			else if (pCurrentSiteTable->siteTable[i].bssType == BSS_INDEPENDENT)
				WLAN_OS_REPORT(("BSS Type		IBSS\r\n\r\n"));
			else if (pCurrentSiteTable->siteTable[i].bssType == BSS_ANY)
				WLAN_OS_REPORT(("BSS Type		ANY\r\n\r\n"));
			else
				WLAN_OS_REPORT(("BSS Type		INVALID\r\n\r\n"));
		
		
			WLAN_OS_REPORT(("Channel			%d\r\n", pCurrentSiteTable->siteTable[i].channel));
		
			WLAN_OS_REPORT(("\r\n"));
		
			switch (pCurrentSiteTable->siteTable[i].maxBasicRate)
			{
			case DRV_RATE_1M:
				WLAN_OS_REPORT(("Max Basic Rate		RATE_1M_BIT\r\n"));
				break;
	
			case DRV_RATE_2M:
				WLAN_OS_REPORT(("Max Basic Rate		RATE_2M_BIT\r\n"));
				break;
	
			case DRV_RATE_5_5M:
				WLAN_OS_REPORT(("Max Basic Rate		RATE_5_5M_BIT\r\n"));
				break;
	
			case DRV_RATE_11M:
				WLAN_OS_REPORT(("Max Basic Rate		RATE_11M_BIT\r\n"));
				break;
	
			case DRV_RATE_6M:
				WLAN_OS_REPORT(("Max Basic Rate		RATE_6M_BIT\r\n"));
				break;
		
			case DRV_RATE_9M:
				WLAN_OS_REPORT(("Max Basic Rate		RATE_9M_BIT\r\n"));
				break;
		
			case DRV_RATE_12M:
				WLAN_OS_REPORT(("Max Basic Rate		RATE_12M_BIT\r\n"));
				break;
		
			case DRV_RATE_18M:
				WLAN_OS_REPORT(("Max Basic Rate		RATE_18M_BIT\r\n"));
				break;
		
			case DRV_RATE_24M:
				WLAN_OS_REPORT(("Max Basic Rate		RATE_24M_BIT\r\n"));
				break;
		
			case DRV_RATE_36M:
				WLAN_OS_REPORT(("Max Basic Rate		RATE_36M_BIT\r\n"));
				break;
		
			case DRV_RATE_48M:
				WLAN_OS_REPORT(("Max Basic Rate		RATE_48M_BIT\r\n"));
				break;
		
			case DRV_RATE_54M:
				WLAN_OS_REPORT(("Max Basic Rate		RATE_54M_BIT\r\n"));
				break;

			default:
					WLAN_OS_REPORT(("Max Basic Rate		INVALID,  0x%X\r\n", pCurrentSiteTable->siteTable[i].maxBasicRate));
				break;
			}
	
				switch (pCurrentSiteTable->siteTable[i].maxActiveRate)
			{
			case DRV_RATE_1M:
				WLAN_OS_REPORT(("Max Active Rate		RATE_1M_BIT\r\n"));
				break;
	
			case DRV_RATE_2M:
				WLAN_OS_REPORT(("Max Active Rate		RATE_2M_BIT\r\n"));
				break;
	
			case DRV_RATE_5_5M:
				WLAN_OS_REPORT(("Max Active Rate		RATE_5_5M_BIT\r\n"));
				break;
	
			case DRV_RATE_11M:
				WLAN_OS_REPORT(("Max Active Rate		RATE_11M_BIT\r\n"));
				break;
	
			case DRV_RATE_22M:
				WLAN_OS_REPORT(("Max Active Rate		RATE_22M_BIT\r\n"));
				break;

			case DRV_RATE_6M:
				WLAN_OS_REPORT(("Max Active Rate		RATE_6M_BIT\r\n"));
				break;
		
			case DRV_RATE_9M:
				WLAN_OS_REPORT(("Max Active Rate		RATE_9M_BIT\r\n"));
				break;
		
			case DRV_RATE_12M:
				WLAN_OS_REPORT(("Max Active Rate		RATE_12M_BIT\r\n"));
				break;
		
			case DRV_RATE_18M:
				WLAN_OS_REPORT(("Max Active Rate		RATE_18M_BIT\r\n"));
				break;
		
			case DRV_RATE_24M:
				WLAN_OS_REPORT(("Max Active Rate		RATE_24M_BIT\r\n"));
				break;
		
			case DRV_RATE_36M:
				WLAN_OS_REPORT(("Max Active Rate		RATE_36M_BIT\r\n"));
				break;
		
			case DRV_RATE_48M:
				WLAN_OS_REPORT(("Max Active Rate		RATE_48M_BIT\r\n"));
				break;
		
			case DRV_RATE_54M:
				WLAN_OS_REPORT(("Max Active Rate		RATE_54M_BIT\r\n"));
				break;
	
			default:
					WLAN_OS_REPORT(("Max Active Rate		INVALID,  0x%X\r\n", pCurrentSiteTable->siteTable[i].maxActiveRate));
				break;
			}
	
			WLAN_OS_REPORT(("\r\n"));
	
				if (pCurrentSiteTable->siteTable[i].probeModulation == DRV_MODULATION_QPSK)
				WLAN_OS_REPORT(("Probe Modulation	QPSK\r\n"));
				else if (pCurrentSiteTable->siteTable[i].probeModulation == DRV_MODULATION_CCK)
				WLAN_OS_REPORT(("Probe Modulation	CCK\r\n"));
				else if (pCurrentSiteTable->siteTable[i].probeModulation == DRV_MODULATION_PBCC)
					WLAN_OS_REPORT(("Probe Modulation	PBCC\r\n"));
				else
					WLAN_OS_REPORT(("Probe Modulation	INVALID, %d\r\n", pCurrentSiteTable->siteTable[i].probeModulation));
		
				if (pCurrentSiteTable->siteTable[i].beaconModulation == DRV_MODULATION_QPSK)
					WLAN_OS_REPORT(("Beacon Modulation	QPSK\r\n"));
				else if (pCurrentSiteTable->siteTable[i].beaconModulation == DRV_MODULATION_CCK)
					WLAN_OS_REPORT(("Beacon Modulation	CCK\r\n"));
				else if (pCurrentSiteTable->siteTable[i].beaconModulation == DRV_MODULATION_PBCC)
					WLAN_OS_REPORT(("Beacon Modulation	PBCC\r\n"));
				else
					WLAN_OS_REPORT(("Beacon Modulation	INVALID, %d\r\n", pCurrentSiteTable->siteTable[i].beaconModulation));
		
				WLAN_OS_REPORT(("\r\n"));
		
				if (pCurrentSiteTable->siteTable[i].privacy == TI_TRUE)
				WLAN_OS_REPORT(("Privacy			On\r\n"));
			else
				WLAN_OS_REPORT(("Privacy			Off\r\n"));
	
				if (pCurrentSiteTable->siteTable[i].currentPreambleType == PREAMBLE_SHORT)
				WLAN_OS_REPORT(("Preamble Type		Short\r\n"));
				else if (pCurrentSiteTable->siteTable[i].currentPreambleType == PREAMBLE_LONG)
				WLAN_OS_REPORT(("Preamble Type		Long\r\n"));
			else
					WLAN_OS_REPORT(("Preamble	INVALID, %d\r\n", pCurrentSiteTable->siteTable[i].currentPreambleType));
	
	
			WLAN_OS_REPORT(("\r\n"));
	
				WLAN_OS_REPORT(("Beacon interval		%d\r\n", pCurrentSiteTable->siteTable[i].beaconInterval));
	
				WLAN_OS_REPORT(("Local Time Stamp	%d\r\n", pCurrentSiteTable->siteTable[i].localTimeStamp));
		
				WLAN_OS_REPORT(("rssi			%d\r\n", pCurrentSiteTable->siteTable[i].rssi));
		
				WLAN_OS_REPORT(("\r\n"));
		
				WLAN_OS_REPORT(("Fail status		%d\r\n", pCurrentSiteTable->siteTable[i].failStatus));
		
				WLAN_OS_REPORT(("ATIM Window %d\r\n", pCurrentSiteTable->siteTable[i].atimWindow));
	
			WLAN_OS_REPORT(("\r\n---------------------------------------------------------------\r\n\r\n", NULL)); 
	
			numOfSites++;
		}
	
		WLAN_OS_REPORT(("\r\n		Number Of Sites:	%d\r\n", numOfSites)); 
		WLAN_OS_REPORT(("\r\n---------------------------------------------------------------\r\n", NULL)); 
		
		   if ((pSiteMgr->pDesiredParams->siteMgrDesiredDot11Mode == DOT11_DUAL_MODE) && (tableIndex==1))
		   {   /* change site table */
			   if (pCurrentSiteTable == &pSiteMgr->pSitesMgmtParams->dot11BG_sitesTables)
				  {
                   WLAN_OS_REPORT(("\r\n		dot11A_sitesTables	\r\n")); 

                   pCurrentSiteTable = (siteTablesParams_t *)&pSiteMgr->pSitesMgmtParams->dot11A_sitesTables;
				  }
			   else
				  {
                   WLAN_OS_REPORT(("\r\n		dot11BG_sitesTables	\r\n")); 

                   pCurrentSiteTable = &pSiteMgr->pSitesMgmtParams->dot11BG_sitesTables;
				  }
		   }

    } while (tableIndex>0);
}

static void printDesiredParams(siteMgr_t *pSiteMgr, TI_HANDLE hCmdDispatch)
{  
	paramInfo_t		param;
	
	WLAN_OS_REPORT(("\r\n\r\n*****************************************", NULL));
	WLAN_OS_REPORT(("*****************************************\r\n\r\n", NULL));

	WLAN_OS_REPORT(("Channel			%d\r\n", pSiteMgr->pDesiredParams->siteMgrDesiredChannel));
	
	WLAN_OS_REPORT(("\r\n*****************************************\r\n\r\n", NULL));

	switch (pSiteMgr->pDesiredParams->siteMgrDesiredRatePair.maxBasic)
	{
	case DRV_RATE_1M:
		WLAN_OS_REPORT(("Max Basic Rate		RATE_1M_BIT\r\n"));
		break;

	case DRV_RATE_2M:
		WLAN_OS_REPORT(("Max Basic Rate		RATE_2M_BIT\r\n"));
		break;

	case DRV_RATE_5_5M:
		WLAN_OS_REPORT(("Max Basic Rate		RATE_5_5M_BIT\r\n"));
		break;

	case DRV_RATE_11M:
		WLAN_OS_REPORT(("Max Basic Rate		RATE_11M_BIT\r\n"));
		break;

	case DRV_RATE_22M:
		WLAN_OS_REPORT(("Max Basic Rate		RATE_22M_BIT\r\n"));
		break;

	case DRV_RATE_6M:
		WLAN_OS_REPORT(("Max Basic Rate		RATE_6M_BIT\r\n"));
		break;

	case DRV_RATE_9M:
		WLAN_OS_REPORT(("Max Basic Rate		RATE_9M_BIT\r\n"));
		break;

	case DRV_RATE_12M:
		WLAN_OS_REPORT(("Max Basic Rate		RATE_12M_BIT\r\n"));
		break;

	case DRV_RATE_18M:
		WLAN_OS_REPORT(("Max Basic Rate		RATE_18M_BIT\r\n"));
		break;

	case DRV_RATE_24M:
		WLAN_OS_REPORT(("Max Basic Rate		RATE_24M_BIT\r\n"));
		break;

	case DRV_RATE_36M:
		WLAN_OS_REPORT(("Max Basic Rate		RATE_36M_BIT\r\n"));
		break;

	case DRV_RATE_48M:
		WLAN_OS_REPORT(("Max Basic Rate		RATE_48M_BIT\r\n"));
		break;

	case DRV_RATE_54M:
		WLAN_OS_REPORT(("Max Basic Rate		RATE_54M_BIT\r\n"));
		break;

	default:
		WLAN_OS_REPORT(("Invalid basic rate value	0x%X\r\n", pSiteMgr->pDesiredParams->siteMgrDesiredRatePair.maxBasic));
		break;
	}

	switch (pSiteMgr->pDesiredParams->siteMgrDesiredRatePair.maxActive)
	{
	case DRV_RATE_1M:
		WLAN_OS_REPORT(("Max Active Rate		RATE_1M_BIT\r\n"));
		break;

	case DRV_RATE_2M:
		WLAN_OS_REPORT(("Max Active Rate		RATE_2M_BIT\r\n"));
		break;

	case DRV_RATE_5_5M:
		WLAN_OS_REPORT(("Max Active Rate		RATE_5_5M_BIT\r\n"));
		break;

	case DRV_RATE_11M:
		WLAN_OS_REPORT(("Max Active Rate		RATE_11M_BIT\r\n"));
		break;

	case DRV_RATE_22M:
		WLAN_OS_REPORT(("Max Active Rate		RATE_22M_BIT\r\n"));
		break;

	case DRV_RATE_6M:
		WLAN_OS_REPORT(("Max Active Rate		RATE_6M_BIT\r\n"));
		break;

	case DRV_RATE_9M:
		WLAN_OS_REPORT(("Max Active Rate		RATE_9M_BIT\r\n"));
		break;

	case DRV_RATE_12M:
		WLAN_OS_REPORT(("Max Active Rate		RATE_12M_BIT\r\n"));
		break;

	case DRV_RATE_18M:
		WLAN_OS_REPORT(("Max Active Rate		RATE_18M_BIT\r\n"));
		break;

	case DRV_RATE_24M:
		WLAN_OS_REPORT(("Max Active Rate		RATE_24M_BIT\r\n"));
		break;

	case DRV_RATE_36M:
		WLAN_OS_REPORT(("Max Active Rate		RATE_36M_BIT\r\n"));
		break;

	case DRV_RATE_48M:
		WLAN_OS_REPORT(("Max Active Rate		RATE_48M_BIT\r\n"));
		break;

	case DRV_RATE_54M:
		WLAN_OS_REPORT(("Max Active Rate		RATE_54M_BIT\r\n"));
		break;

	default:
		WLAN_OS_REPORT(("Invalid basic rate value	0x%X\r\n", pSiteMgr->pDesiredParams->siteMgrDesiredRatePair.maxActive));
		break;
	}

	if (pSiteMgr->pDesiredParams->siteMgrDesiredModulationType == DRV_MODULATION_QPSK)
		WLAN_OS_REPORT(("Modulation Type		QPSK\r\n"));
	else if (pSiteMgr->pDesiredParams->siteMgrDesiredModulationType == DRV_MODULATION_CCK)
		WLAN_OS_REPORT(("Modulation Type		CCK\r\n"));
	else if (pSiteMgr->pDesiredParams->siteMgrDesiredModulationType == DRV_MODULATION_PBCC)
		WLAN_OS_REPORT(("Modulation Type		PBCC\r\n"));
	else if (pSiteMgr->pDesiredParams->siteMgrDesiredModulationType == DRV_MODULATION_OFDM)
		WLAN_OS_REPORT(("Modulation Type		OFDM\r\n"));
	else
		WLAN_OS_REPORT(("Invalid Modulation Type	%d\r\n", pSiteMgr->pDesiredParams->siteMgrDesiredModulationType));


	WLAN_OS_REPORT(("\r\n*****************************************\r\n\r\n", NULL));

	param.paramType = RSN_EXT_AUTHENTICATION_MODE;
	cmdDispatch_GetParam(hCmdDispatch, &param);
	if (param.content.rsnDesiredAuthType == RSN_AUTH_OPEN)
		WLAN_OS_REPORT(("Authentication Type	Open System\r\n"));
	else if (param.content.rsnDesiredAuthType == RSN_AUTH_SHARED_KEY)
		WLAN_OS_REPORT(("Authentication Type	Shared Key\r\n"));
	else 
		WLAN_OS_REPORT(("Authentication Type	Invalid: %d\r\n", param.content.rsnDesiredAuthType));

	param.paramType = RSN_ENCRYPTION_STATUS_PARAM;
	cmdDispatch_GetParam(hCmdDispatch, &param);
	if (param.content.rsnEncryptionStatus == TWD_CIPHER_NONE)
		WLAN_OS_REPORT(("WEP 				Off\r\n"));
	else if (param.content.rsnEncryptionStatus == TWD_CIPHER_WEP)
		WLAN_OS_REPORT(("WEP 				On\r\n"));
	else 
		WLAN_OS_REPORT(("WEP 		Invalid: %d\r\n", param.content.rsnEncryptionStatus));
		
	WLAN_OS_REPORT(("\r\n"));


	WLAN_OS_REPORT(("\r\n*****************************************\r\n\r\n", NULL));
	if(pSiteMgr->pDesiredParams->siteMgrDesiredDot11Mode == DOT11_B_MODE)
		WLAN_OS_REPORT(("Desired dot11mode		11b\r\n"));
	else if(pSiteMgr->pDesiredParams->siteMgrDesiredDot11Mode == DOT11_G_MODE)
		WLAN_OS_REPORT(("Desired dot11mode		11g\r\n"));
	else if(pSiteMgr->pDesiredParams->siteMgrDesiredDot11Mode == DOT11_A_MODE)
		WLAN_OS_REPORT(("Desired dot11mode		11a\r\n"));
	else if(pSiteMgr->pDesiredParams->siteMgrDesiredDot11Mode == DOT11_DUAL_MODE)
		WLAN_OS_REPORT(("Desired dot11mode		dual 11a/g\r\n"));
	else 
		WLAN_OS_REPORT(("Desired dot11mode		INVALID\r\n"));

	WLAN_OS_REPORT(("\r\n*****************************************\r\n\r\n", NULL));
	if(pSiteMgr->pDesiredParams->siteMgrDesiredSlotTime == PHY_SLOT_TIME_SHORT)
		WLAN_OS_REPORT(("Desired slot time		short\r\n"));
	else if(pSiteMgr->pDesiredParams->siteMgrDesiredSlotTime == PHY_SLOT_TIME_LONG)
		WLAN_OS_REPORT(("Desired slot time		long\r\n"));
	else
		WLAN_OS_REPORT(("Desired slot time		INVALID\r\n"));


	WLAN_OS_REPORT(("\r\n*****************************************\r\n\r\n", NULL));
	if (pSiteMgr->pDesiredParams->siteMgrDesiredPreambleType == PREAMBLE_SHORT)
		WLAN_OS_REPORT(("Desired Preamble		Short\r\n"));
	else if (pSiteMgr->pDesiredParams->siteMgrDesiredPreambleType == PREAMBLE_LONG)
		WLAN_OS_REPORT(("Desired Preamble	Long\r\n"));
	else
		WLAN_OS_REPORT(("Invalid Desired Preamble	%d\r\n", pSiteMgr->pDesiredParams->siteMgrDesiredPreambleType));

	WLAN_OS_REPORT(("Beacon interval		%d\r\n", pSiteMgr->pDesiredParams->siteMgrDesiredBeaconInterval));

	WLAN_OS_REPORT(("\r\n*****************************************", NULL));
	WLAN_OS_REPORT(("*****************************************\r\n\r\n", NULL));

}



static void printPrimarySiteDesc(siteMgr_t *pSiteMgr, OS_802_11_BSSID *pPrimarySiteDesc)
{
	TI_UINT8 rateIndex, maxNumOfRates;
	char ssid[MAX_SSID_LEN + 1];

	
	WLAN_OS_REPORT(("\r\n^^^^^^^^^^^^^^^	PRIMARY SITE DESCRIPTION	^^^^^^^^^^^^^^^^^^^\r\n\r\n")); 

	
	/* MacAddress */		
	WLAN_OS_REPORT(("BSSID				0x%X-0x%X-0x%X-0x%X-0x%X-0x%X\r\n",	pPrimarySiteDesc->MacAddress[0], 
																	pPrimarySiteDesc->MacAddress[1], 
																	pPrimarySiteDesc->MacAddress[2], 
																	pPrimarySiteDesc->MacAddress[3], 
																	pPrimarySiteDesc->MacAddress[4], 
																	pPrimarySiteDesc->MacAddress[5])); 

	/* Capabilities */
	WLAN_OS_REPORT(("Capabilities		0x%X\r\n",	pPrimarySiteDesc->Capabilities)); 

	/* SSID */
	os_memoryCopy(pSiteMgr->hOs, ssid, (void *)pPrimarySiteDesc->Ssid.Ssid, pPrimarySiteDesc->Ssid.SsidLength);
	ssid[pPrimarySiteDesc->Ssid.SsidLength] = 0;
	WLAN_OS_REPORT(("SSID				%s\r\n", ssid));

	/* privacy */
	if (pPrimarySiteDesc->Privacy == TI_TRUE)
		WLAN_OS_REPORT(("Privacy				ON\r\n"));
	else
		WLAN_OS_REPORT(("Privacy				OFF\r\n"));

	/* RSSI */				
	WLAN_OS_REPORT(("RSSI					%d\r\n", ((pPrimarySiteDesc->Rssi)>>16)));

	if (pPrimarySiteDesc->InfrastructureMode == os802_11IBSS)
		WLAN_OS_REPORT(("BSS Type				IBSS\r\n"));
	else
		WLAN_OS_REPORT(("BSS Type				INFRASTRUCTURE\r\n"));


	maxNumOfRates = sizeof(pPrimarySiteDesc->SupportedRates) / sizeof(pPrimarySiteDesc->SupportedRates[0]); 
	/* SupportedRates */
	for (rateIndex = 0; rateIndex < maxNumOfRates; rateIndex++)
	{
		if (pPrimarySiteDesc->SupportedRates[rateIndex] != 0)
			WLAN_OS_REPORT(("Rate					0x%X\r\n", pPrimarySiteDesc->SupportedRates[rateIndex]));
	}

	WLAN_OS_REPORT(("\r\n---------------------------------------------------------------\r\n\r\n", NULL)); 

}

static void setRateSet(TI_UINT8 maxRate, TRates *pRates)
{
	TI_UINT8 i = 0;

	switch (maxRate)
	{

	case DRV_RATE_54M:
		pRates->ratesString[i] = 108;
		i++;

	case DRV_RATE_48M:
		pRates->ratesString[i] = 96;
		i++;

	case DRV_RATE_36M:
		pRates->ratesString[i] = 72;
		i++;

	case DRV_RATE_24M:
		pRates->ratesString[i] = 48;
		i++;

	case DRV_RATE_18M:
		pRates->ratesString[i] = 36;
		i++;

	case DRV_RATE_12M:
		pRates->ratesString[i] = 24;
		i++;

	case DRV_RATE_9M:
		pRates->ratesString[i] = 18;
		i++;

	case DRV_RATE_6M:
		pRates->ratesString[i] = 12;
		i++;

	case DRV_RATE_22M:
		pRates->ratesString[i] = 44;
		i++;

	case DRV_RATE_11M:
		pRates->ratesString[i] = 22;
		pRates->ratesString[i] |= 0x80;
		i++;

	case DRV_RATE_5_5M:
		pRates->ratesString[i] = 11;
		pRates->ratesString[i] |= 0x80;
		i++;

	case DRV_RATE_2M:
		pRates->ratesString[i] = 4;
		pRates->ratesString[i] |= 0x80;
		i++;

	case DRV_RATE_1M:
		pRates->ratesString[i] = 2;
		pRates->ratesString[i] |= 0x80;
		i++;
		break;

	default:
		WLAN_OS_REPORT(("Set Rate Set, invalid max rate %d\r\n", maxRate));
		pRates->len = 0;
	}

	pRates->len = i;

}

void printSiteMgrHelpMenu(void)
{
	WLAN_OS_REPORT(("\r\n\r\n   Site Manager Debug Menu   \r\n"));
	WLAN_OS_REPORT(("------------------------\r\n"));

	WLAN_OS_REPORT(("500 - Help.\r\n"));
	WLAN_OS_REPORT(("501 - Primary Site Parameters.\r\n"));
	WLAN_OS_REPORT(("502 - Sites List.\r\n"));
	WLAN_OS_REPORT(("503 - Desired Parameters.\r\n"));
	WLAN_OS_REPORT(("507 - Set Power save Mode.\r\n"));
	WLAN_OS_REPORT(("508 - Get Power save Mode.\r\n"));
	WLAN_OS_REPORT(("511 - Set Default Key Id.\r\n"));
	WLAN_OS_REPORT(("512 - Get Default Key Id.\r\n"));
	WLAN_OS_REPORT(("513 - Set Key.\r\n"));
	WLAN_OS_REPORT(("514 - Get Key.\r\n"));
	WLAN_OS_REPORT(("515 - Set Cypher Suite.\r\n"));
	WLAN_OS_REPORT(("516 - Get Cypher Suite.\r\n"));
	WLAN_OS_REPORT(("517 - Set Auth Mode.\r\n"));
	WLAN_OS_REPORT(("518 - Get Auth Mode.\r\n"));
	WLAN_OS_REPORT(("519 - Get Primary Site Description.\r\n"));
	WLAN_OS_REPORT(("520 - Get Connection Status.\r\n"));
	WLAN_OS_REPORT(("522 - Get Current Tx Rate.\r\n"));
	WLAN_OS_REPORT(("525 - Set Supported Rate Set.\r\n"));
	WLAN_OS_REPORT(("526 - Get Supported Rate Set.\r\n"));
	WLAN_OS_REPORT(("527 - Set Auth type.\r\n"));
	WLAN_OS_REPORT(("528 - Get Auth type.\r\n"));

	WLAN_OS_REPORT(("        %03d - RADIO_STAND_BY_CHANGE_STATE \r\n", RADIO_STAND_BY_CHANGE_STATE));
	WLAN_OS_REPORT(("        %03d - CONNECT_TO_BSSID \r\n", CONNECT_TO_BSSID));

	WLAN_OS_REPORT(("        %03d - SET_START_CLI_SCAN_PARAM \r\n", SET_START_CLI_SCAN_PARAM));
	WLAN_OS_REPORT(("        %03d - SET_STOP_CLI_SCAN_PARAM \r\n", SET_STOP_CLI_SCAN_PARAM));

	WLAN_OS_REPORT(("        %03d - SET_BROADCAST_BACKGROUND_SCAN_PARAM \r\n", SET_BROADCAST_BACKGROUND_SCAN_PARAM));
	WLAN_OS_REPORT(("        %03d - ENABLE_PERIODIC_BROADCAST_BBACKGROUND_SCAN_PARAM \r\n", ENABLE_PERIODIC_BROADCAST_BACKGROUND_SCAN_PARAM));
	WLAN_OS_REPORT(("        %03d - DISABLE_PERIODIC_BROADCAST_BACKGROUND_SCAN_PARAM \r\n", DISABLE_PERIODIC_BROADCAST_BACKGROUND_SCAN_PARAM));

	WLAN_OS_REPORT(("        %03d - SET_UNICAST_BACKGROUND_SCAN_PARAM \r\n", SET_UNICAST_BACKGROUND_SCAN_PARAM));
	WLAN_OS_REPORT(("        %03d - ENABLE_PERIODIC_UNICAST_BACKGROUND_SCAN_PARAM \r\n", ENABLE_PERIODIC_UNICAST_BACKGROUND_SCAN_PARAM));
	WLAN_OS_REPORT(("        %03d - DISABLE_PERIODIC_UNICAST_BACKGROUND_SCAN_PARAM \r\n", DISABLE_PERIODIC_UNICAST_BACKGROUND_SCAN_PARAM));

	WLAN_OS_REPORT(("        %03d - SET_FOREGROUND_SCAN_PARAM \r\n", SET_FOREGROUND_SCAN_PARAM));
	WLAN_OS_REPORT(("        %03d - ENABLE_PERIODIC_FOREGROUND_SCAN_PARAM \r\n", ENABLE_PERIODIC_FOREGROUND_SCAN_PARAM));
	WLAN_OS_REPORT(("        %03d - DISABLE_PERIODIC_FOREGROUND_SCAN_PARAM \r\n", DISABLE_PERIODIC_FOREGROUND_SCAN_PARAM));
	
	WLAN_OS_REPORT(("        %03d - SET_CHANNEL_NUMBER \r\n", SET_CHANNEL_NUMBER));
	WLAN_OS_REPORT(("        %03d - SET_RSSI_GAP_THRSH \r\n", SET_RSSI_GAP_THRSH));
	WLAN_OS_REPORT(("        %03d - SET_FAST_SCAN_TIMEOUT \r\n", SET_FAST_SCAN_TIMEOUT));
	WLAN_OS_REPORT(("        %03d - SET_INTERNAL_ROAMING_ENABLE \r\n", SET_INTERNAL_ROAMING_ENABLE));

	WLAN_OS_REPORT(("        %03d - PERFORM_HEALTH_TEST \r\n", PERFORM_HEALTH_TEST));
	WLAN_OS_REPORT(("        %03d - PRINT_FAILURE_EVENTS \r\n", PRINT_FAILURE_EVENTS));
	WLAN_OS_REPORT(("        %03d - FORCE_HW_RESET_RECOVERY \r\n", FORCE_HW_RESET_RECOVERY));
	WLAN_OS_REPORT(("        %03d - FORCE_SOFT_RECOVERY \r\n", FORCE_SOFT_RECOVERY));

	WLAN_OS_REPORT(("        %03d - RESET_ROAMING_EVENTS \r\n", RESET_ROAMING_EVENTS));
	WLAN_OS_REPORT(("        %03d - SET_DESIRED_CONS_TX_ERRORS_THREH\r\n", SET_DESIRED_CONS_TX_ERRORS_THREH));
	
	WLAN_OS_REPORT(("        %03d - GET_CURRENT_ROAMING_STATUS \r\n", GET_CURRENT_ROAMING_STATUS));
	

    WLAN_OS_REPORT(("        %03d - TOGGLE_LNA_ON \r\n", TEST_TOGGLE_LNA_ON));
    WLAN_OS_REPORT(("        %03d - TOGGLE_LNA_OFF \r\n", TEST_TOGGLE_LNA_OFF));

	WLAN_OS_REPORT(("        %03d - PRINT_SITE_TABLE_PER_SSID\r\n", PRINT_SITE_TABLE_PER_SSID));
	
	WLAN_OS_REPORT(("        %03d - SET_DESIRED_CHANNEL\r\n", SET_DESIRED_CHANNEL));
	
	WLAN_OS_REPORT(("        %03d - START_PRE_AUTH\r\n", START_PRE_AUTH));

	WLAN_OS_REPORT(("\r\n------------------------\r\n"));
}



