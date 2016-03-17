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
#define __FILE_ID__  FILE_ID_137
#include <windows.h>
#include <winsock.h>
#include <ceddk.h>
#include <ceddkex.h>
#if defined(BSP_EVM_DM_AM_37X)
#include <sdk_gpio.h>
#else
#include <gpio.h>
#endif
#include <ndis.h>
#include <Nuiouser.h> 

#include "ostitype.h"
#include "WlanDrvIf.h"
#include "EvHandler.h"
#include "osApi.h"
#include "osdot11.h"
#include "osEntry.h"
#include "Report.h"
#include "CmdHndlr.h"
#include "cmdinterpret.h"
#include "cmdinterpretoid.h"
#include "cmdDispatcher.h"
#include "cu_event.h"
#include "privcmd.h"

#ifdef TI_DBG
#include "tracebuf_api.h"
int g_oidPrint=0;
extern unsigned long  TiDebugFlag;
#endif

static TI_INT32 AddWepKey(TI_HANDLE hCmdInterpret, void *pBuffer);
static TI_INT32 cmdInterpret_initEvents(TI_HANDLE hCmdInterpret);
static TI_INT32 cmdInterpret_Event(IPC_EV_DATA* pData);
static TI_INT32 cmdInterpret_unregisterEvents(TI_HANDLE hCmdInterpret);


/** \brief Send Logger Data
 * 
 * \param  data 	- Pointer to data buffer which is sent to Logger
 * \param  len    	- Length of data sent to Logger
 * \return void
 * 
 * \par Description
 * Send Data to Logger via NW - for user to see
 * 
 * \sa
 */ 
VOID SendLoggerData(TI_UINT8* data, TI_UINT16 len);
/** \brief Set Debug output to Ticon
 * 
 * \param  value	- TRUE: Indicates that redirection of output to Ticon should be done ; FALSE otherwise
 * \return void 
 * 
 * \par Description
 * Redirect output of Ticon
 * 
 * \sa
 */ 
VOID os_setDebugOutputToTicon(TI_BOOL value);
/** \brief Set Debug output to Logger
 * 
 * \param  value	- TRUE: Indicates that redirection of output to Logger should be done ; FALSE otherwise
 * \return void 
 * 
 * \par Description
 * Redirect output of Ticon
 * 
 * \sa
 */ 

static TI_UINT16 LoggerId;

TI_HANDLE cmdInterpret_Create(TI_HANDLE hOs)
{
    cmdInterpret_t *pCmdInterpret;

    /* Allocate memory for object */
    pCmdInterpret = os_memoryAlloc(hOs, sizeof(cmdInterpret_t));

    /* In case of failure -> return NULL */
    if (!pCmdInterpret)
    {
        PRINT(DBG_NDIS_OIDS_ERROR, "cmdInterpret_init: failed to allocate memory...aborting\r\n");
        return (TI_HANDLE)NULL;
    }

    /* Clear all fields in cmdInterpreter module object */
    os_memoryZero(hOs, pCmdInterpret, sizeof (cmdInterpret_t));

    /* Save handlers */
    pCmdInterpret->hOs = hOs;

    /* Return pointer to object */
    return (TI_HANDLE)pCmdInterpret;
}

TI_STATUS cmdInterpret_Destroy (TI_HANDLE hCmdInterpret, TI_HANDLE hEvHandler)
{
    cmdInterpret_t *pCmdInterpret = (cmdInterpret_t *)hCmdInterpret;

    /* Unregister events */
    cmdInterpret_unregisterEvents ((TI_HANDLE)pCmdInterpret);

    /* Release allocated memory */
    os_memoryFree (pCmdInterpret->hOs, pCmdInterpret, sizeof(cmdInterpret_t));

    return TI_OK;
}

void cmdInterpret_Init (TI_HANDLE hCmdInterpret, TStadHandlesList *pStadHandles)
{

    cmdInterpret_t *pCmdInterpret = (cmdInterpret_t *)hCmdInterpret;
#ifdef TI_DBG
    TiDebugFlag = (((DBG_NDIS_OIDS | DBG_INIT) << 16) | 
				   DBG_INIT |
				   /* DBG_SEV_INFO   |  
				   DBG_SEV_LOUD      | 
				   DBG_SEV_VERY_LOUD | 
				   DBG_SEV_WARNING   |  */
				   DBG_SEV_ERROR     | 
				   DBG_SEV_FATAL_ERROR
				   );
#endif

    pCmdInterpret->hCmdHndlr    = pStadHandles->hCmdHndlr;
    pCmdInterpret->hEvHandler   = pStadHandles->hEvHandler;
    pCmdInterpret->hCmdDispatch = pStadHandles->hCmdDispatch;
	pCmdInterpret->hReport		= pStadHandles->hReport;
    /* Register to driver events */
    cmdInterpret_initEvents (hCmdInterpret);
}


static TI_INT32 cmdGetAppIndexByPort(cmdInterpret_t *pCmdInterpret, TI_UINT16 port)
{
	TI_UINT32 i;

	for(i=0; i<TI_MAX_APPLICATIONS; i++) {
		if(pCmdInterpret->appEvents[i].id == port)
			return i;
	}

	PRINTF(DBG_NDIS_OIDS_ERROR, ("TIWLN: cmdGetAppIndexByPort(): Can't find application entry(id=%X)\r\n", port));

	return -1;
}


int cmdInterpret_ServiceCompleteCB (TI_HANDLE hCmdInterpret, int status, void *buffer)
{
    cmdInterpret_t *pCmdInterpret = (cmdInterpret_t *)hCmdInterpret;

    if (pCmdInterpret->pAsyncCmd == NULL)
    {
        os_printf ("cmdInterpret_ServiceCompleteCB: AsyncCmd is NULL!!\r\n");
        return TI_NOK;
    }

    pCmdInterpret->pAsyncCmd->return_code = status;

    /* Copy local buffer back to OS pointer according to specific OS structure */
	os_memoryCopyToUser (pCmdInterpret->hOs, pCmdInterpret->pAsyncCmd->buffer1, 
						 pCmdInterpret->pAsyncCmd->local_buffer, pCmdInterpret->pAsyncCmd->buffer2_len);
    pCmdInterpret->pAsyncCmd = NULL;

    /* Call the Cmd module to complete command processing */
    cmdHndlr_Complete (pCmdInterpret->hCmdHndlr);

    /* Call commands handler to continue handling further commands if queued */
    cmdHndlr_HandleCommands (pCmdInterpret->hCmdHndlr);

    return TI_OK;
}


TI_STATUS cmdProcessProprietaryOid(TI_HANDLE hCmdInterpret, TConfigCommand *cmdObj, 
								   PUCHAR inBuf, UINT32 inLen, PUCHAR outBuf, 
								   UINT32 outLen, UINT32* status, UINT32* written)
{
	cmdInterpret_t *pCmdInterpret  = (cmdInterpret_t *)hCmdInterpret;
	ti_private_cmd_t* pcmd = (ti_private_cmd_t*) inBuf;
	ti_private_cmd_t my_cmd;
	paramInfo_t Param;
	PENABLE_DISABLE_EVENT endis;
	TI_STATUS res = TI_OK, i;

    os_memoryCopyFromUser(pCmdInterpret->hOs, &my_cmd, inBuf, sizeof(ti_private_cmd_t));

	*status = NDIS_STATUS_SUCCESS;
	*written = my_cmd.in_buffer_len;
    
	switch(pcmd->cmd) {
	case TIWLN_EVENTS_REGISTRATION:
		PRINTF(DBG_NDIS_OIDS_LOUD, ("App EVENT registration. App id=%X\r\n", *(TI_UINT16*)(inBuf+sizeof(ti_private_cmd_t))));

		for(i=0; i<TI_MAX_APPLICATIONS; i++)
			if(!pCmdInterpret->appEvents[i].id) {

				pCmdInterpret->appEvents[i].id = *(TI_UINT16*)(inBuf+sizeof(ti_private_cmd_t));
				os_memoryZero(pCmdInterpret->hOs, pCmdInterpret->appEvents[i].events, 
							  sizeof(pCmdInterpret->appEvents[i].events));

				break;

			}

		return TI_OK;

	case TIWLN_EVENTS_UNREGISTER:
		PRINTF(DBG_NDIS_OIDS_LOUD, ("App UNREGISTER. App id=%X\r\n", *(TI_UINT16*)(inBuf+sizeof(ti_private_cmd_t))));

		i = cmdGetAppIndexByPort(pCmdInterpret, *(TI_UINT16*)(inBuf+sizeof(ti_private_cmd_t)));

		if(i != -1) {
			pCmdInterpret->appEvents[i].id = 0;
		}

		return TI_OK;

	case TIWLN_DISABLE_EVENT:
		endis = (PENABLE_DISABLE_EVENT) (inBuf+sizeof(ti_private_cmd_t));
		PRINTF(DBG_NDIS_OIDS_LOUD, ("App DISABLE EVENT. App id=%X, event=%X\r\n", endis->id, endis->mask));

		if(endis->mask == IPC_EVENT_LOGGER)
			os_setDebugOutputToLogger(FALSE);

		i = cmdGetAppIndexByPort(pCmdInterpret, endis->id);

		if(i != -1)
			pCmdInterpret->appEvents[i].events[endis->mask] = 0;

		return TI_OK;


	case TIWLN_ENABLE_EVENT:
		endis = (PENABLE_DISABLE_EVENT) (inBuf+sizeof(ti_private_cmd_t));
		PRINTF(DBG_NDIS_OIDS_LOUD, ("App ENABLE EVENT. App id=%X, event=%X\r\n", endis->id, endis->mask));

		if(endis->mask == IPC_EVENT_LOGGER) {
			os_setDebugOutputToLogger(TRUE);
			LoggerId = endis->id;
		}

		i = cmdGetAppIndexByPort(pCmdInterpret, endis->id);

		if(i != -1)
			pCmdInterpret->appEvents[i].events[endis->mask] = 1;

		return TI_OK;

	case TIWLN_EVENTS_DEBUG_LEVEL:
		return TI_OK;

	case TIWLN_REDIR_TO_TICON:
		PRINTF(DBG_NDIS_OIDS_LOUD, ("Redirecting driver output to ticon window... \r\n"));
		os_setDebugOutputToTicon(TRUE);
		return TI_OK;
	}

	/*
	os_printf ("in_buffer =  0x%x (len = %d)\r\n",my_command.in_buffer,(unsigned int)my_command.in_buffer_len);
	os_printf ("out_buffer =  0x%x (len = %d)\r\n",my_command.out_buffer,(unsigned int)my_command.out_buffer_len);
	*/

	Param.paramType = my_cmd.cmd;

	if (IS_PARAM_ASYNC(my_cmd.cmd)) {
		Param.content.interogateCmdCBParams.hCb  =  (TI_HANDLE)pCmdInterpret;
		Param.content.interogateCmdCBParams.fCb  =  (void*)cmdInterpret_ServiceCompleteCB;
	
		 cmdObj->buffer2_len = my_cmd.out_buffer_len;
		 cmdObj->local_buffer = os_memoryAlloc (pCmdInterpret->hOs, my_cmd.out_buffer_len);
	
		 Param.content.interogateCmdCBParams.pCb  =  cmdObj->local_buffer;
	
		 if ((my_cmd.in_buffer) && (my_cmd.in_buffer_len)) {
			 os_memoryCopyFromUser(pCmdInterpret->hOs, cmdObj->local_buffer, my_cmd.in_buffer,my_cmd.in_buffer_len);
		 }	
	}
	else {
		if ((my_cmd.in_buffer) && (my_cmd.in_buffer_len)) {
		my_cmd.in_buffer = (void*) (inBuf + sizeof(ti_private_cmd_t) );

        if(IS_ALLOC_NEEDED_PARAM(my_cmd.cmd))
        {
            *(void**)&Param.content = os_memoryAlloc(pCmdInterpret->hOs, my_cmd.in_buffer_len);
            os_memoryCopyFromUser(pCmdInterpret->hOs, *(void**)&Param.content, my_cmd.in_buffer, my_cmd.in_buffer_len);
        }
        else
        {
            os_memoryCopyFromUser(pCmdInterpret->hOs, &Param.content,          my_cmd.in_buffer, my_cmd.in_buffer_len);
        }
	}
	}

	if (my_cmd.flags & PRIVATE_CMD_SET_FLAG) {
		Param.paramLength = my_cmd.in_buffer_len;
		res = cmdDispatch_SetParam (pCmdInterpret->hCmdDispatch, &Param);
	} 

	else if (pcmd->flags & PRIVATE_CMD_GET_FLAG) {

			*written = my_cmd.out_buffer_len;
			my_cmd.out_buffer = (void*) (inBuf);

			/* os_printf ("Calling getParam\r\n"); */
			Param.paramLength = my_cmd.out_buffer_len;
			res = cmdDispatch_GetParam (pCmdInterpret->hCmdDispatch, &Param);

			if(res == EXTERNAL_GET_PARAM_DENIED) {
				*status  = NDIS_STATUS_INVALID_DATA;
				return res;
			}

			if(res == NO_SITE_SELECTED_YET)
				res = TI_OK;

			/* 
			this is for cmd that want to check the size of memory that they need to 
			allocate for the actual data.
			*/
			if(Param.paramLength && (pcmd->out_buffer_len == 0)) {
				my_cmd.out_buffer_len = Param.paramLength;  
			}
	}
	else {
		*status  = NDIS_STATUS_INVALID_DATA;
		res = TI_NOK;
	}

	if(res == TI_OK) {

		if(IS_PARAM_ASYNC(my_cmd.cmd)) {
			pCmdInterpret->pAsyncCmd = cmdObj; /* Save command handle for completion CB */
			res = COMMAND_PENDING;
		}
		else {
		if ((my_cmd.out_buffer) && (my_cmd.out_buffer_len)) {
			if ((my_cmd.in_buffer) && (my_cmd.in_buffer_len)) {
					if(IS_ALLOC_NEEDED_PARAM(my_cmd.cmd)) {
						os_memoryCopyToUser (pCmdInterpret->hOs, my_cmd.out_buffer, *(void**)&Param.content, 
											 my_cmd.out_buffer_len);
						os_memoryFree(pCmdInterpret->hOs, *(void**)&Param.content, my_cmd.in_buffer_len);
					}
					else
						os_memoryCopyToUser (pCmdInterpret->hOs, my_cmd.out_buffer, &Param.content, my_cmd.out_buffer_len);
			}
			else
				os_memoryCopyToUser (pCmdInterpret->hOs, my_cmd.out_buffer, &Param.content, my_cmd.out_buffer_len);
		}
		}
	}
	else {
		*written = 0;
	}

	return res;
}


TI_STATUS cmdInterpret_CapabilityGet(cmdInterpret_t *pCmdInterpret, PUCHAR pData, PULONG Length)
{
    TWlanDrvIfObjPtr              pAdapter = (TWlanDrvIfObjPtr)pCmdInterpret->hOs;
	NDIS_802_11_CAPABILITY        *capability_802_11 = (NDIS_802_11_CAPABILITY *)pData;
	rsnAuthEncrCapability_t       rsnAuthEncrCap;
	OS_802_11_ENCRYPTION_STATUS   encrStatus = os802_11EncryptionDisabled;
	ULONG                         neededLength = 0;
	UINT                          i = 0;
	paramInfo_t                   Param;
	TI_STATUS                     status;

	/* If length of the input buffer less than needed length, do nothing, */
	/* return the needed length                                           */
    neededLength = FIELD_OFFSET(NDIS_802_11_CAPABILITY, AuthenticationEncryptionSupported) + (MAX_AUTH_ENCR_PAIR * sizeof(NDIS_802_11_AUTHENTICATION_ENCRYPTION));
	if (*Length < neededLength)
	{
		*Length = neededLength;
		return TI_NOK;
	}
	NdisZeroMemory (pData, neededLength);
	/* Fill Param fields and get the 802_11 capability information */
	Param.paramType   = RSN_AUTH_ENCR_CAPABILITY;
	Param.paramLength = neededLength;
	Param.content.pRsnAuthEncrCapability = &rsnAuthEncrCap;
	status = cmdDispatch_GetParam (pCmdInterpret->hCmdDispatch, &Param);
	if (status != TI_OK)
	{
		/* return the default values only */
		/* PMKIDs is 0, AUTH/Encr pairs is 1, Auth/Encr is OPEN/NONE (0/0) */
		capability_802_11->Length     = sizeof(OS_802_11_CAPABILITY);
		capability_802_11->Version    = OID_CAPABILITY_VERSION;
		capability_802_11->NoOfPMKIDs = 0;
		capability_802_11->NoOfAuthEncryptPairsSupported = 1;
		capability_802_11->AuthenticationEncryptionSupported[0].AuthModeSupported  =  os802_11AuthModeOpen;
		capability_802_11->AuthenticationEncryptionSupported[0].EncryptStatusSupported = os802_11EncryptionDisabled;
		*Length = sizeof(OS_802_11_CAPABILITY);
		return TI_OK;
	}
    /* Update the needed length according to the actual data size */
    neededLength = FIELD_OFFSET(NDIS_802_11_CAPABILITY, AuthenticationEncryptionSupported) + 
                   (rsnAuthEncrCap.NoOfAuthEncrPairSupported * sizeof(NDIS_802_11_AUTHENTICATION_ENCRYPTION));

    /* Copy the received info to the OS_802_11_CAPABILITY needed format */
	capability_802_11->Length     = neededLength;
	capability_802_11->Version    = OID_CAPABILITY_VERSION;
	capability_802_11->NoOfPMKIDs = rsnAuthEncrCap.NoOfPMKIDs;
	capability_802_11->NoOfAuthEncryptPairsSupported = rsnAuthEncrCap.NoOfAuthEncrPairSupported;

	/* Convert received cipher suite type to encr.status type */
	for (i = 0; i < rsnAuthEncrCap.NoOfAuthEncrPairSupported; i ++)
	{
		capability_802_11->AuthenticationEncryptionSupported[i].AuthModeSupported = 
		(OS_802_11_AUTHENTICATION_MODE)rsnAuthEncrCap.authEncrPairs[i].authenticationMode;
		switch (rsnAuthEncrCap.authEncrPairs[i].cipherSuite)
		{
    		case TWD_CIPHER_NONE:
    			encrStatus = os802_11EncryptionDisabled;
    			break;
    
    		case TWD_CIPHER_WEP:
    			encrStatus = os802_11WEPEnabled;
    			break;
    
    		case TWD_CIPHER_TKIP:
    			encrStatus = os802_11Encryption2Enabled;
    			break;
    
    		case TWD_CIPHER_AES_CCMP:
    			encrStatus = os802_11Encryption3Enabled;
    			break;
    
    		default:
    			encrStatus = os802_11EncryptionDisabled;
    			break;

		}
		capability_802_11->AuthenticationEncryptionSupported[i].EncryptStatusSupported = encrStatus;
    }

    /* Success; return the actual length of the written data */
	*Length = neededLength;
	return status;

} /* cmdInterpret_CapabilityGet() */

TI_STATUS cmdInterpret_AssociationInfoGet(cmdInterpret_t *pCmdInterpret, PUCHAR pData, PULONG Length)
{
    TWlanDrvIfObjPtr                    pAdapter = (TWlanDrvIfObjPtr)pCmdInterpret->hOs;
	OS_802_11_ASSOCIATION_INFORMATION   *assocInformation;
	UINT8                               *requestIEs;
	UINT8                               *responseIEs;
	ULONG                               TotalLength = 0,retValue;
	paramInfo_t                         param;

	param.paramType   = ASSOC_ASSOCIATION_INFORMATION_PARAM;
	param.paramLength = *Length;
	retValue    = cmdDispatch_GetParam (pCmdInterpret->hCmdDispatch, &param);
	TotalLength = sizeof(OS_802_11_ASSOCIATION_INFORMATION) + 
				  param.content.assocAssociationInformation.RequestIELength +
				  param.content.assocAssociationInformation.ResponseIELength;
	if (TotalLength <= *Length)
	{
		NdisMoveMemory(pData, (UINT8 *)&param.content, sizeof(OS_802_11_ASSOCIATION_INFORMATION));
		assocInformation = (OS_802_11_ASSOCIATION_INFORMATION*)pData;
		requestIEs = (UINT8*)pData + sizeof(OS_802_11_ASSOCIATION_INFORMATION);
		if (assocInformation->RequestIELength > 0)
		{
			NdisMoveMemory(requestIEs, (UINT8*)assocInformation->OffsetRequestIEs, assocInformation->RequestIELength); 
			assocInformation->OffsetRequestIEs = sizeof(OS_802_11_ASSOCIATION_INFORMATION);
		}
		if (assocInformation->ResponseIELength > 0)
		{
			responseIEs = requestIEs + assocInformation->RequestIELength;
			NdisMoveMemory(responseIEs, (UINT8*)assocInformation->OffsetResponseIEs, assocInformation->ResponseIELength); 
			assocInformation->OffsetResponseIEs = assocInformation->OffsetRequestIEs + assocInformation->RequestIELength;
		}
		PRINTF(DBG_NDIS_OIDS_LOUD, ("UtilAssociationInfoGet: pData=%p, "
								"OffsetRequestIEs=0x%lx, OffsetResponseIEs=0x%lx\r\n",
								pData, (long)assocInformation->OffsetRequestIEs, 
								(long)assocInformation->OffsetResponseIEs));
	} 
    else
	{
		retValue = TI_NOK; /*Means that more buffer space needed*/
	}
	*Length = TotalLength;

	return retValue;

}

TI_INT32 cmdInterpret_convertAndExecute (TI_HANDLE hCmdInterpret, TConfigCommand *cmdObj)
{
    cmdInterpret_t      *pCmdInterpret  = (cmdInterpret_t *)hCmdInterpret;
    TI_STATUS           res             = TI_OK;
    NDIS_STATUS         ndisStatus      = NDIS_STATUS_SUCCESS;
    paramInfo_t         Param;
    OS_802_11_BSSID     SelectedBSSID;
    TIWLN_STATISTICS*   pStats;
    TWlanDrvIfObjPtr    pAdapter     = (TWlanDrvIfObjPtr)pCmdInterpret->hOs;
    TTxDataCounters     txCounters[MAX_NUM_OF_AC];
	UINT32 written;
    OS_802_11_CONFIGURATION  siteMgrConfiguration;


__try 
{
    if (cmdObj->cmd == OID_TI_WILINK_IOCTL) 
    {
        PRINTF(DBG_NDIS_OIDS_LOUD, ("TIWL: OID_TI_WILINK_IOCTL: cbInfoBuffer (%d) pcbNeeded (%d)\r\n",
                                         cmdObj->buffer1_len, *cmdObj->param4));
    }
    /* tb_sprintf("cmdObj=0x%x cmd=0x%x b1=0x%x b1_len=%d\r\n",cmdObj, cmdObj->cmd, cmdObj->buffer1, cmdObj->buffer1_len); */
    if (g_oidPrint)
    {
        WLAN_OS_REPORT(("cmdObj=0x%x cmd=0x%x b1=0x%x b1_len=%d\r\n",cmdObj, cmdObj->cmd, cmdObj->buffer1, cmdObj->buffer1_len));
    }
	TRACE4(pCmdInterpret->hReport,REPORT_SEVERITY_INFORMATION,"cmdObj=0x%x cmd=0x%x b1=0x%x b1_len=%d\r\n",cmdObj, cmdObj->cmd, cmdObj->buffer1, cmdObj->buffer1_len);
	os_memoryZero(pCmdInterpret->hOs, &Param, sizeof(paramInfo_t));   
    *cmdObj->param3 = sizeof(TI_UINT32);
    *cmdObj->param4 = 0;
    switch (cmdObj->cmd) 
    {
	#ifdef AKU30_SUPPORT
		/* for wpa2 */
		case OID_802_11_CAPABILITY:
        {
            *cmdObj->param4 = cmdObj->buffer1_len;
            res = cmdInterpret_CapabilityGet(pCmdInterpret, cmdObj->buffer1, cmdObj->param4);
            if (res == TI_NOK)
            {
                if (*cmdObj->param4 > cmdObj->buffer1_len) /* need more space*/
                {
                    PRINTF(DBG_NDIS_OIDS_VERY_LOUD, ("Buffer for parameter 0x%X is bigger then Param size\r\n", Param.paramType));
                }
                else /* error */
                {
                    *cmdObj->param4 = 0;
                    *cmdObj->param3 = 0;
                }
            }
            else /* OK */
            {
                *cmdObj->param3 = *cmdObj->param4;
                *cmdObj->param4 = 0;
            }
			break;
        }
        case OID_802_11_PMKID:
            Param.paramType = RSN_PMKID_LIST;

            if (cmdObj->flags == OS_OID_SET_FLAG)
            {
                Param.paramLength = cmdObj->buffer1_len;
                os_memoryCopy(pCmdInterpret->hOs, &Param.content, cmdObj->buffer1, cmdObj->buffer1_len);
                *cmdObj->param3 = cmdObj->buffer1_len;
                res = cmdDispatch_SetParam (pCmdInterpret->hCmdDispatch, &Param);
            }
            else
            {
                PRINT(DBG_NDIS_OIDS_LOUD, "TIWL: OID_802_11_PMKID GET\r\n");
                Param.paramLength = sizeof(OS_802_11_PMKID);
                res = cmdDispatch_GetParam (pCmdInterpret->hCmdDispatch, &Param);
                if (res == TI_OK)
                {
                     if (Param.paramLength > cmdObj->buffer1_len)
                    {
                         PRINTF(DBG_NDIS_OIDS_VERY_LOUD, ("Buffer for parameter 0x%X is bigger then Param size\r\n", Param.paramType));
                         *cmdObj->param4 = Param.paramLength;
                    }
                    else
                    {
                        os_memoryCopy(pCmdInterpret->hOs, cmdObj->buffer1, &Param.content.rsnPMKIDList, Param.paramLength);
                        *cmdObj->param3 = cmdObj->buffer1_len;
                    }
                }
                else
                {
                    PRINTF(DBG_NDIS_OIDS_ERROR, ("cmdInterpret: ERROR on return from get param=%d, status=%d\r\n", Param.paramType, res));
                    *cmdObj->param3 = 0;
                }
            }
            break;

		case  OID_FSW_802_11_AVAILABLE_OPTIONS:
			PRINT(DBG_NDIS_OIDS_LOUD, "TIWL: TIWLN_802_11_AVAILABLE_OPTIONS_GET \r\n");
            Param.paramType = RSN_WPA_PROMOTE_AVAILABLE_OPTIONS;
            Param.paramLength = sizeof(TI_UINT32);
            res = cmdDispatch_GetParam (pCmdInterpret->hCmdDispatch, &Param);
            if (res == TI_OK)
            {   
                if (Param.paramLength > cmdObj->buffer1_len)
                {
                    PRINTF(DBG_NDIS_OIDS_VERY_LOUD, ("Buffer for parameter 0x%X is bigger then Param size\r\n", Param.paramType));
                    *cmdObj->param4 = Param.paramLength;
                }
                else
                {
                    os_memoryCopy(pCmdInterpret->hOs, cmdObj->buffer1, &Param.content.rsnWPAMixedModeSupport, Param.paramLength);
                    *cmdObj->param3 = cmdObj->buffer1_len;
                }
            }
            else
            {
                PRINTF(DBG_NDIS_OIDS_ERROR, ("cmdInterpret: ERROR on return from get param=%d, status=%d\r\n", Param.paramType, res));
                *cmdObj->param3 = 0;
            }
			break;

        case  OID_FSW_802_11_OPTIONS:
            Param.paramType = RSN_WPA_PROMOTE_OPTIONS;
            Param.paramLength = sizeof(TI_UINT32);
            if (cmdObj->flags == OS_OID_SET_FLAG)
            {
                PRINT(DBG_NDIS_OIDS_LOUD, "TIWL: TIWLN_802_11_WPA_OPTIONS_SET\r\n");
                Param.content.rsnWPAPromoteFlags = *((TI_UINT32*)cmdObj->buffer1);
                Param.paramLength = sizeof (TI_UINT32);
                res = cmdDispatch_SetParam (pCmdInterpret->hCmdDispatch, &Param);
            }
            else
            {
    			PRINT(DBG_NDIS_OIDS_LOUD, "TIWL: TIWLN_802_11_WPA_OPTIONS_GET\r\n");
                res = cmdDispatch_GetParam (pCmdInterpret->hCmdDispatch, &Param);
                if (res == TI_OK)
                {   
                    if (Param.paramLength > cmdObj->buffer1_len)
                    {
                        PRINTF(DBG_NDIS_OIDS_VERY_LOUD, ("Buffer for parameter 0x%X is bigger then Param size\r\n", Param.paramType));
                        *cmdObj->param4 = Param.paramLength;
                    }
                    else
                    {
                        os_memoryCopy(pCmdInterpret->hOs, cmdObj->buffer1, &Param.content.rsnWPAPromoteFlags, Param.paramLength);
                        *cmdObj->param3 = cmdObj->buffer1_len;
                    }
                }
                else
                {
                    PRINTF(DBG_NDIS_OIDS_ERROR, ("cmdInterpret: ERROR on return from get param=%d, status=%d\r\n", Param.paramType, res));
                    *cmdObj->param3 = 0;
                }
            }
			break;
			
	#endif /* AKU30_SUPPORT */

        /*
		 * NDISUIO interface
         * OID_TI_WILINK_IOCTL uses the IOCTL_NDIS_QUERY_OID function to transport
         * the request from User space to this driver.  The NDISUIO protocol layer
         * processes the IOCTL_NDIS_QUERY_OID and passes us a pointer to a common
         * parameter buffer for both input/output.  The first dword in this buffer
         * contains the TI Proprietary IOCTL code.  Subsequent bytes contain any
         * associated data required by this IOCTL.
		 */
    case OID_TI_WILINK_IOCTL:
        res = cmdProcessProprietaryOid(hCmdInterpret, cmdObj, cmdObj->buffer1, cmdObj->buffer1_len, 
								 cmdObj->buffer2, cmdObj->buffer2_len, &ndisStatus, &written);

		*cmdObj->param3 = written;
		break;
    
	case OID_GEN_SUPPORTED_LIST:
		PRINT(DBG_NDIS_OIDS_LOUD, "TIWL: OID_GEN_SUPPORTED_LIST\r\n");
        if (sizeof(SupportedOids) > cmdObj->buffer1_len)
        {
            PRINTF(DBG_NDIS_OIDS_VERY_LOUD, ("Buffer for parameter 0x%X is bigger then Param size\r\n", OID_GEN_SUPPORTED_LIST));
            *cmdObj->param4 = Param.paramLength;
        }
        else
        {
            os_memoryCopy(pCmdInterpret->hOs, cmdObj->buffer1, SupportedOids, sizeof(SupportedOids));
            *cmdObj->param3 = sizeof(SupportedOids);
        }
		break;
    
	case OID_GEN_HARDWARE_STATUS:
		PRINT(DBG_NDIS_OIDS_LOUD, "TIWL: OID_GEN_HARDWARE_STATUS\r\n");
        if (cmdObj->buffer1_len >= sizeof(TI_UINT32))
        {
            *(TI_INT32*)cmdObj->buffer1 = NdisHardwareStatusReady;
        }
        else
        {
            PRINTF(DBG_NDIS_OIDS_VERY_LOUD, ("cmdInterpret: Wrong buffer size in OID_GEN_HARDWARE_STATUS\r\n"));
            *cmdObj->param4 = sizeof(TI_UINT32);
        }
		break;

	case OID_GEN_MEDIA_SUPPORTED:
	case OID_GEN_MEDIA_IN_USE:
		PRINT(DBG_NDIS_OIDS_LOUD, "TIWL: OID_GEN_MEDIA_SUPPORTED\r\n");
        if (cmdObj->buffer1_len >= sizeof(TI_UINT32))
        {
            *(TI_INT32*)cmdObj->buffer1 = NdisMedium802_3;
        }
        else
        {
            PRINTF(DBG_NDIS_OIDS_VERY_LOUD, ("cmdInterpret: Wrong buffer size in OID_GEN_MEDIA_IN_USE\r\n"));
            *cmdObj->param4 = sizeof(TI_UINT32);
        }
		break;

    case OID_GEN_MEDIA_CAPABILITIES:
        PRINT(DBG_NDIS_OIDS_VERY_LOUD, "TIWL: OID_GEN_MEDIA_CAPABILITIES\r\n");
        if (cmdObj->buffer1_len >= sizeof(TI_UINT32))
        {
            *(TI_INT32*)cmdObj->buffer1 = (NDIS_MEDIA_CAP_TRANSMIT | NDIS_MEDIA_CAP_RECEIVE);
        }
        else
        {
            PRINTF(DBG_NDIS_OIDS_VERY_LOUD, ("cmdInterpret: Wrong buffer size in OID_GEN_MEDIA_CAPABILITIES\r\n"));
            *cmdObj->param4 = sizeof(TI_UINT32);
        }
        break;

    case OID_GEN_PHYSICAL_MEDIUM:
		PRINT(DBG_NDIS_OIDS_LOUD, "TIWL: OID_GEN_PHYSICAL_MEDIUM\r\n");
        if (cmdObj->buffer1_len >= sizeof(TI_UINT32))
        {
            *(TI_INT32*)cmdObj->buffer1 = NdisPhysicalMediumWirelessLan;
        }
        else
        {
            PRINTF(DBG_NDIS_OIDS_VERY_LOUD, ("cmdInterpret: Wrong buffer size in OID_GEN_PHYSICAL_MEDIUM\r\n"));
            *cmdObj->param4 = sizeof(TI_UINT32);
        }
		break;
    
	case OID_GEN_MEDIA_CONNECT_STATUS:
        if (cmdObj->buffer1_len >= sizeof(TI_UINT32))
        {
            *(TI_UINT32*)cmdObj->buffer1 = pAdapter->LinkStatus;
            PRINTF(DBG_NDIS_OIDS_LOUD, ("TIWL: OID_GEN_MEDIA_CONNECT_STATUS %d\r\n", *(TI_UINT32 *)cmdObj->buffer1));
        }
        else
        {
            PRINTF(DBG_NDIS_OIDS_VERY_LOUD, ("cmdInterpret: Wrong buffer size in OID_GEN_MEDIA_CONNECT_STATUS\r\n"));
            *cmdObj->param4 = sizeof(TI_UINT32);
        }
		break;

	case OID_GEN_MAXIMUM_LOOKAHEAD:
        if (cmdObj->buffer1_len >= sizeof(TI_UINT32))
        {
            *(TI_UINT32*)cmdObj->buffer1 = pAdapter->etherMaxPayloadSize;
            PRINTF(DBG_NDIS_OIDS_LOUD, ("TIWL: OID_GEN_MAXIMUM_LOOKAHEAD %d\r\n", *(TI_UINT32 *)cmdObj->buffer1));
        }
        else
        {
            PRINTF(DBG_NDIS_OIDS_VERY_LOUD, ("cmdInterpret: Wrong buffer size in OID_GEN_MAXIMUM_LOOKAHEAD\r\n"));
            *cmdObj->param4 = sizeof(TI_UINT32);
        }
		break;

    case OID_GEN_CURRENT_LOOKAHEAD:
        if (cmdObj->flags == OS_OID_SET_FLAG)
        {
            /*
		    /* We always indicate the entire packet regardless of the
		    /* lookahead size.  So this request should be politely ignored.
            */
            res = TI_NOK;
        }
        else
        {
            if (cmdObj->buffer1_len >= sizeof(TI_UINT32))
            {
                *(TI_UINT32*)cmdObj->buffer1 = pAdapter->etherMaxPayloadSize;
                PRINTF(DBG_NDIS_OIDS_LOUD, ("TIWL: OID_GEN_CURRENT_LOOKAHEAD %d\r\n", *(TI_UINT32 *)cmdObj->buffer1));
            }
            else
            {
                PRINTF(DBG_NDIS_OIDS_VERY_LOUD, ("cmdInterpret: Wrong buffer size in OID_GEN_CURRENT_LOOKAHEAD\r\n"));
                *cmdObj->param4 = sizeof(TI_UINT32);
            }
        }
		break;

	case OID_GEN_MAXIMUM_FRAME_SIZE:
        if (cmdObj->buffer1_len >= sizeof(TI_UINT32))
        {
            *(TI_UINT32*)cmdObj->buffer1 = pAdapter->etherMaxPayloadSize;
            PRINTF(DBG_NDIS_OIDS_LOUD, ("TIWL: OID_GEN_MAXIMUM_FRAME_SIZE %d\r\n", *(TI_UINT32 *)cmdObj->buffer1));
        }
        else
        {
            PRINTF(DBG_NDIS_OIDS_VERY_LOUD, ("cmdInterpret: Wrong buffer size in OID_GEN_MAXIMUM_FRAME_SIZE\r\n"));
            *cmdObj->param4 = sizeof(TI_UINT32);
        }
		break;

	case OID_GEN_TRANSMIT_BLOCK_SIZE:
        if (cmdObj->buffer1_len >= sizeof(TI_UINT32))
        {
            *(TI_UINT32*)cmdObj->buffer1 = pAdapter->etherMaxPayloadSize;
            PRINTF(DBG_NDIS_OIDS_LOUD, ("TIWL: OID_GEN_TRANSMIT_BLOCK_SIZE %d\r\n", *(TI_UINT32 *)cmdObj->buffer1));
        }
        else
        {
            PRINTF(DBG_NDIS_OIDS_VERY_LOUD, ("cmdInterpret: Wrong buffer size in OID_GEN_TRANSMIT_BLOCK_SIZE\r\n"));
            *cmdObj->param4 = sizeof(TI_UINT32);
        }
		break;

	case OID_GEN_RECEIVE_BLOCK_SIZE:
        if (cmdObj->buffer1_len >= sizeof(TI_UINT32))
        {
            *(TI_UINT32*)cmdObj->buffer1 = pAdapter->etherMaxPayloadSize;
            PRINTF(DBG_NDIS_OIDS_LOUD, ("TIWL: OID_GEN_RECEIVE_BLOCK_SIZE %d\r\n", *(TI_UINT32 *)cmdObj->buffer1));
        }
        else
        {
            PRINTF(DBG_NDIS_OIDS_VERY_LOUD, ("cmdInterpret: Wrong buffer size in OID_GEN_RECEIVE_BLOCK_SIZE\r\n"));
            *cmdObj->param4 = sizeof(TI_UINT32);
        }
		break;

	case OID_GEN_MAXIMUM_TOTAL_SIZE:
        if (cmdObj->buffer1_len >= sizeof(TI_UINT32))
        {
            *(TI_UINT32*)cmdObj->buffer1 = pAdapter->etherMaxPayloadSize + GEN_MAXIMUM_EXTRA_TOTAL_SIZE;
            PRINTF(DBG_NDIS_OIDS_LOUD, ("TIWL: OID_GEN_MAXIMUM_TOTAL_SIZE %d\r\n", *(TI_UINT32 *)cmdObj->buffer1));
        }
        else
        {
            PRINTF(DBG_NDIS_OIDS_VERY_LOUD, ("cmdInterpret: Wrong buffer size in OID_GEN_MAXIMUM_TOTAL_SIZE\r\n"));
            *cmdObj->param4 = sizeof(TI_UINT32);
        }
		break;

	case OID_GEN_TRANSMIT_BUFFER_SPACE:
        if (cmdObj->buffer1_len >= sizeof(TI_UINT32))
        {
            *(TI_INT32*)cmdObj->buffer1 = TRANSMIT_BUFFER_SPACE;
            PRINTF(DBG_NDIS_OIDS_LOUD, ("TIWL: OID_GEN_TRANSMIT_BUFFER_SPACE %d\r\n", *(TI_UINT32 *)cmdObj->buffer1));
        }
        else
        {
            PRINTF(DBG_NDIS_OIDS_VERY_LOUD, ("cmdInterpret: Wrong buffer size in OID_GEN_TRANSMIT_BUFFER_SPACE\r\n"));
            *cmdObj->param4 = sizeof(TI_UINT32);
        }
		break;

	case OID_GEN_RECEIVE_BUFFER_SPACE:
        if (cmdObj->buffer1_len >= sizeof(TI_UINT32))
        {
            *(TI_INT32*)cmdObj->buffer1 = RECEIVE_BUFFER_SPACE;
            PRINTF(DBG_NDIS_OIDS_LOUD, ("TIWL: OID_GEN_RECEIVE_BUFFER_SPACE %d\r\n", *(TI_UINT32 *)cmdObj->buffer1));
        }
        else
        {
            PRINTF(DBG_NDIS_OIDS_VERY_LOUD, ("cmdInterpret: Wrong buffer size in OID_GEN_RECEIVE_BUFFER_SPACE\r\n"));
            *cmdObj->param4 = sizeof(TI_UINT32);
        }
		break;

	case OID_GEN_MAXIMUM_SEND_PACKETS:
        if (cmdObj->buffer1_len >= sizeof(TI_UINT32))
        {
            *(TI_INT32*)cmdObj->buffer1 = MAXIMUM_SEND_PACKETS;
            PRINTF(DBG_NDIS_OIDS_LOUD, ("TIWL: OID_GEN_MAXIMUM_SEND_PACKETS %d\r\n", *(TI_UINT32 *)cmdObj->buffer1));
        }
        else
        {
            PRINTF(DBG_NDIS_OIDS_VERY_LOUD, ("cmdInterpret: Wrong buffer size in OID_GEN_MAXIMUM_SEND_PACKETS\r\n"));
            *cmdObj->param4 = sizeof(TI_UINT32);
        }
		break;

	case OID_GEN_VENDOR_DESCRIPTION:
        if (cmdObj->buffer1_len >= VENDOR_DESCRIPTION_LENGTH)
        {
            void* pVendorName = VENDOR_DESCRIPTION;
            
            os_memoryCopy(pCmdInterpret->hOs, cmdObj->buffer1, pVendorName, VENDOR_DESCRIPTION_LENGTH);
            *cmdObj->param3 = VENDOR_DESCRIPTION_LENGTH;
            PRINTF(DBG_NDIS_OIDS_LOUD, ("TIWL: OID_GEN_VENDOR_DESCRIPTION %s, len %d\r\n", (UCHAR *)cmdObj->buffer1, cmdObj->buffer1_len));
        }
        else
        {
            PRINTF(DBG_NDIS_OIDS_VERY_LOUD, ("cmdInterpret: Wrong buffer size in OID_GEN_VENDOR_DESCRIPTION\r\n"));
            *cmdObj->param4 = VENDOR_DESCRIPTION_LENGTH;
        }
		break;

    case OID_GEN_VENDOR_ID:
        if (cmdObj->buffer1_len >= VENDOR_ID_LENGTH)
        {
            ((TI_UINT8*)cmdObj->buffer1)[0] = pAdapter->PermanentAddr[0];
            ((TI_UINT8*)cmdObj->buffer1)[1] = pAdapter->PermanentAddr[1];
            ((TI_UINT8*)cmdObj->buffer1)[2] = pAdapter->PermanentAddr[2];
            ((TI_UINT8*)cmdObj->buffer1)[3] = 0;
            *cmdObj->param3 = VENDOR_ID_LENGTH;
            PRINTF(DBG_NDIS_OIDS_LOUD, ("TIWL: OID_GEN_VENDOR_ID %s, len %d\r\n", (UCHAR *)cmdObj->buffer1, cmdObj->buffer1_len));
        }
        else
        {
            PRINTF(DBG_NDIS_OIDS_VERY_LOUD, ("cmdInterpret: Wrong buffer size in OID_GEN_VENDOR_ID\r\n"));
            *cmdObj->param4 = VENDOR_ID_LENGTH;
        }
		break;

    case OID_GEN_VENDOR_DRIVER_VERSION:
        if (cmdObj->buffer1_len >= sizeof(TI_UINT32))
        {
            *(TI_INT32*)cmdObj->buffer1 = VENDOR_DRIVER_VERSION;
            PRINTF(DBG_NDIS_OIDS_LOUD, ("TIWL: OID_GEN_VENDOR_DRIVER_VERSION %d\r\n", *(TI_UINT32 *)cmdObj->buffer1));
        }
        else
        {
            PRINTF(DBG_NDIS_OIDS_VERY_LOUD, ("cmdInterpret: Wrong buffer size in OID_GEN_VENDOR_DRIVER_VERSION\r\n"));
            *cmdObj->param4 = sizeof(TI_UINT32);
        }
		break;

    case OID_GEN_DRIVER_VERSION:

        if (cmdObj->buffer1_len >= sizeof(TI_UINT16))
        {
            *(TI_UINT16*)cmdObj->buffer1 = 0x0501;  /* NDIS 5.1 */
            *cmdObj->param3 = sizeof(TI_UINT16);
        }
        else
        {
            PRINTF(DBG_NDIS_OIDS_VERY_LOUD, ("cmdInterpret: Wrong buffer size in OID_GEN_DRIVER_VERSION\r\n"));
            *cmdObj->param4 = sizeof(TI_UINT16);
        }
		break;

    case OID_GEN_LINK_SPEED:
        if (cmdObj->buffer1_len >= sizeof(TI_UINT32))
        {
            if (!pAdapter->LinkSpeed)
            {
                 *(TI_INT32*)cmdObj->buffer1 = LINK_SPEED;
            }
            else
            {
                *(TI_UINT32*)cmdObj->buffer1 = pAdapter->LinkSpeed;
            }
            PRINTF(DBG_NDIS_OIDS_LOUD, ("TIWL: OID_GEN_LINK_SPEED %d\r\n", *(TI_UINT32 *)cmdObj->buffer1));
        }
        else
        {
            PRINTF(DBG_NDIS_OIDS_VERY_LOUD, ("cmdInterpret: Wrong buffer size in OID_GEN_LINK_SPEED\r\n"));
            *cmdObj->param4 = sizeof(TI_UINT32);
        }
		break;

    case OID_GEN_CURRENT_PACKET_FILTER:
        if (cmdObj->flags == OS_OID_SET_FLAG)
        {
            PRINT(DBG_NDIS_OIDS_LOUD, "TIWL: (SET) OID_GEN_CURRENT_PACKET_FILTER\r\n");           
            /* 
			 * See if the packet filter is asking us to filter on something we
             * cannot handle.  If so show the packet filter is not supported...
			 */
            if (*((TI_UINT32*)cmdObj->buffer1) & ~(NDIS_PACKET_TYPE_DIRECTED      |
										NDIS_PACKET_TYPE_MULTICAST     |
										NDIS_PACKET_TYPE_BROADCAST     | 
									 /* we are not supporting promiscuous mode */
                                     /* NDIS_PACKET_TYPE_PROMISCUOUS   | */
										NDIS_PACKET_TYPE_ALL_MULTICAST)) 
            {
                PRINTF(DBG_NDIS_OIDS_WARNING, ("TIWLN: Filter not supported 0x%x\r\n", *(TI_UINT32 *)cmdObj->buffer1));
                *cmdObj->param3 = 0;
                ndisStatus = NDIS_STATUS_NOT_SUPPORTED;
            }
            else 
            {
                /* Save the packet filter. */
                pAdapter->PacketFilter = *((TI_UINT32*)cmdObj->buffer1);
            }
        }
        else
        {
            if (cmdObj->buffer1_len >= sizeof(TI_UINT32))
            {
                PRINTF(DBG_NDIS_OIDS_LOUD, ("TIWL: OID_GEN_CURRENT_PACKET_FILTER %d\r\n", *(TI_UINT32 *)cmdObj->buffer1));
            }
            else
            {
                PRINTF(DBG_NDIS_OIDS_VERY_LOUD, ("cmdInterpret: Wrong buffer size in OID_GEN_CURRENT_PACKET_FILTER\r\n"));
                *cmdObj->param4 = sizeof(TI_UINT32);
            }
        }
		break;

	case OID_GEN_MAC_OPTIONS:
		if (cmdObj->buffer1_len >= sizeof(TI_UINT32))
        {
            *(TI_INT32*)cmdObj->buffer1 = ( NDIS_MAC_OPTION_TRANSFERS_NOT_PEND  | 
                                            NDIS_MAC_OPTION_COPY_LOOKAHEAD_DATA |
                                            NDIS_MAC_OPTION_NO_LOOPBACK         |
                                            NDIS_MAC_OPTION_FULL_DUPLEX         |
                                            NDIS_MAC_OPTION_8021P_PRIORITY
                                            );
            PRINTF(DBG_NDIS_OIDS_LOUD, ("TIWL: OID_GEN_MAC_OPTIONS %d\r\n", *(TI_UINT32 *)cmdObj->buffer1));
        }
        else
        {
            PRINTF(DBG_NDIS_OIDS_VERY_LOUD, ("cmdInterpret: Wrong buffer size in OID_GEN_CURRENT_PACKET_FILTER\r\n"));
            *cmdObj->param4 = sizeof(TI_UINT32);
        }
		break;

    case OID_GEN_XMIT_OK:
        Param.paramType = TX_CTRL_COUNTERS_PARAM;
        Param.paramLength = sizeof(TTxDataCounters);
        Param.content.pTxDataCounters = &txCounters[0];
        res = cmdDispatch_GetParam (pCmdInterpret->hCmdDispatch, &Param);
        if (res == TI_OK)
        {   
            if (cmdObj->buffer1_len < sizeof(TI_UINT32))
            {
                PRINTF(DBG_NDIS_OIDS_VERY_LOUD, ("Buffer for parameter 0x%X is bigger then Param size\r\n", Param.paramType));
                *cmdObj->param4 = sizeof(TI_UINT32);
            }
            else
            {
                *(TI_UINT32*)cmdObj->buffer1 = Param.content.pTxDataCounters->XmitOk;
                PRINTF(DBG_NDIS_OIDS_LOUD, ("TIWL: OID_GEN_XMIT_OK %d, len %d\r\n", *(TI_UINT32 *)cmdObj->buffer1, cmdObj->buffer1_len));
            }
        }
        else
        {
            PRINTF(DBG_NDIS_OIDS_ERROR, ("cmdInterpret: ERROR on return from get param=%d, status=%d\r\n", Param.paramType, res));
            *cmdObj->param3 = 0;
            ndisStatus = NDIS_STATUS_NOT_SUPPORTED;
        }
		break;

    case OID_GEN_RCV_OK:
        Param.paramType = RX_DATA_COUNTERS_PARAM;
        Param.paramLength = sizeof(rxDataCounters_t);
        res = cmdDispatch_GetParam (pCmdInterpret->hCmdDispatch, &Param);       
        if (res == TI_OK)
        {   
            if (cmdObj->buffer1_len < sizeof(TI_UINT32))
            {
                PRINTF(DBG_NDIS_OIDS_VERY_LOUD, ("Buffer for parameter 0x%X is bigger then Param size\r\n", Param.paramType));
                *cmdObj->param4 = sizeof(TI_UINT32);
            }
            else
            {
                *(TI_UINT32*)cmdObj->buffer1 = Param.content.siteMgrTiWlanCounters.RecvOk;
                PRINTF(DBG_NDIS_OIDS_LOUD, ("TIWL: OID_GEN_RCV_OK %d, len %d\r\n", *(TI_UINT32 *)cmdObj->buffer1, cmdObj->buffer1_len));
            }
        }
        else
        {
            PRINTF(DBG_NDIS_OIDS_ERROR, ("cmdInterpret: ERROR on return from get param=%d, status=%d\r\n", Param.paramType, res));
            *cmdObj->param3 = 0;
        }
		break;

    case OID_GEN_XMIT_ERROR:
        Param.paramType = TX_CTRL_COUNTERS_PARAM;
        Param.paramLength = sizeof(TTxDataCounters);
        Param.content.pTxDataCounters = &txCounters[0];
        res = cmdDispatch_GetParam (pCmdInterpret->hCmdDispatch, &Param);
        if (res == TI_OK)
        {   
            if (cmdObj->buffer1_len < sizeof(TI_UINT32))
            {
                PRINTF(DBG_NDIS_OIDS_VERY_LOUD, ("Buffer for parameter 0x%X is bigger then Param size\r\n", Param.paramType));
                *cmdObj->param4 = sizeof(TI_UINT32);
            }
            else
            {
                *(TI_UINT32*)cmdObj->buffer1 = Param.content.pTxDataCounters->RetryFailCounter +
                                   Param.content.pTxDataCounters->NoLinkCounter +
                                   Param.content.pTxDataCounters->TxTimeoutCounter +
                                   Param.content.pTxDataCounters->OtherFailCounter;
                PRINTF(DBG_NDIS_OIDS_LOUD, ("TIWL: OID_GEN_XMIT_ERROR %d, len %d\r\n", *(TI_UINT32 *)cmdObj->buffer1, cmdObj->buffer1_len));
            }
        }
        else
        {
            PRINTF(DBG_NDIS_OIDS_ERROR, ("cmdInterpret: ERROR on return from get param=%d, status=%d\r\n", Param.paramType, res));
            *cmdObj->param3 = 0;
        }
		break;

    case OID_GEN_RCV_ERROR:
        Param.paramType = RX_DATA_COUNTERS_PARAM;
        Param.paramLength = sizeof(rxDataCounters_t);
        res = cmdDispatch_GetParam (pCmdInterpret->hCmdDispatch, &Param);
        if (res == TI_OK)
        {   
            if (cmdObj->buffer1_len < sizeof(TI_UINT32))
            {
                PRINTF(DBG_NDIS_OIDS_VERY_LOUD, ("Buffer for parameter 0x%X is bigger then Param size\r\n", Param.paramType));
                *cmdObj->param4 = sizeof(TI_UINT32);
            }
            else
            {
                *(TI_UINT32*)cmdObj->buffer1 =  Param.content.siteMgrTiWlanCounters.AssocRejects +
                                                Param.content.siteMgrTiWlanCounters.AssocTimeouts +
                                                Param.content.siteMgrTiWlanCounters.AuthRejects +
                                                Param.content.siteMgrTiWlanCounters.AuthTimeouts +
                                                Param.content.siteMgrTiWlanCounters.FcsErrors +
                                                Param.content.siteMgrTiWlanCounters.RecvError;
                PRINTF(DBG_NDIS_OIDS_LOUD, ("TIWL: OID_GEN_RCV_ERROR %d, len %d\r\n", *(TI_UINT32 *)cmdObj->buffer1, cmdObj->buffer1_len));
            }
        }
        else
        {
            PRINTF(DBG_NDIS_OIDS_ERROR, ("cmdInterpret: ERROR on return from get param=%d, status=%d\r\n", Param.paramType, res));
            *cmdObj->param3 = 0;
        }
		break;

    case OID_GEN_RCV_NO_BUFFER:
        Param.paramType = RX_DATA_COUNTERS_PARAM;
        Param.paramLength = sizeof(rxDataCounters_t);
        res = cmdDispatch_GetParam (pCmdInterpret->hCmdDispatch, &Param);
        if (res == TI_OK)
        {   
            if (cmdObj->buffer1_len < sizeof(TI_UINT32))
            {
                PRINTF(DBG_NDIS_OIDS_VERY_LOUD, ("Buffer for parameter 0x%X is bigger then Param size\r\n", Param.paramType));
                *cmdObj->param4 = sizeof(TI_UINT32);
            }
            else
            {
                *(TI_UINT32*)cmdObj->buffer1 = Param.content.siteMgrTiWlanCounters.RecvNoBuffer;
                PRINTF(DBG_NDIS_OIDS_LOUD, ("TIWL: OID_GEN_RCV_NO_BUFFER %d, len %d\r\n", *(TI_UINT32 *)cmdObj->buffer1, cmdObj->buffer1_len));
            }
        }
        else
        {
            PRINTF(DBG_NDIS_OIDS_ERROR, ("cmdInterpret: ERROR on return from get param=%d, status=%d\r\n", Param.paramType, res));
            *cmdObj->param3 = 0;
        }
		break;
      
    case OID_GEN_DIRECTED_BYTES_XMIT:
        Param.paramType = TX_CTRL_COUNTERS_PARAM;
        Param.paramLength = sizeof(TTxDataCounters);
        Param.content.pTxDataCounters = &txCounters[0];
        res = cmdDispatch_GetParam (pCmdInterpret->hCmdDispatch, &Param);
        if (res == TI_OK)
        {   
            if (cmdObj->buffer1_len < sizeof(TI_UINT32))
            {
                PRINTF(DBG_NDIS_OIDS_VERY_LOUD, ("Buffer for parameter 0x%X is bigger then Param size\r\n", Param.paramType));
                *cmdObj->param4 = sizeof(TI_UINT32);
            }
            else
            {
                *(TI_UINT32*)cmdObj->buffer1 = Param.content.pTxDataCounters->DirectedBytesXmit;
                PRINTF(DBG_NDIS_OIDS_LOUD, ("TIWL: OID_GEN_DIRECTED_BYTES_XMIT %d, len %d\r\n", *(TI_UINT32 *)cmdObj->buffer1, cmdObj->buffer1_len));
            }
        }
        else
        {
            PRINTF(DBG_NDIS_OIDS_ERROR, ("cmdInterpret: ERROR on return from get param=%d, status=%d\r\n", Param.paramType, res));
            *cmdObj->param3 = 0;
        }
		break;

    case OID_GEN_DIRECTED_FRAMES_XMIT:
        Param.paramType = TX_CTRL_COUNTERS_PARAM;
        Param.paramLength = sizeof(TTxDataCounters);
        Param.content.pTxDataCounters = &txCounters[0];
        res = cmdDispatch_GetParam (pCmdInterpret->hCmdDispatch, &Param);
        if (res == TI_OK)
        {   
            if (cmdObj->buffer1_len < sizeof(TI_UINT32))
            {
                PRINTF(DBG_NDIS_OIDS_VERY_LOUD, ("Buffer for parameter 0x%X is bigger then Param size\r\n", Param.paramType));
                *cmdObj->param4 = sizeof(TI_UINT32);
            }
            else
            {
                *(TI_UINT32*)cmdObj->buffer1 = Param.content.pTxDataCounters->DirectedFramesXmit;
                PRINTF(DBG_NDIS_OIDS_LOUD, ("TIWL: OID_GEN_DIRECTED_FRAMES_XMIT %d, len %d\r\n", *(TI_UINT32 *)cmdObj->buffer1, cmdObj->buffer1_len));
            }
        }
        else
        {
            PRINTF(DBG_NDIS_OIDS_ERROR, ("cmdInterpret: ERROR on return from get param=%d, status=%d\r\n", Param.paramType, res));
            *cmdObj->param3 = 0;
        }
		break;

    case OID_GEN_MULTICAST_BYTES_XMIT:
        Param.paramType = TX_CTRL_COUNTERS_PARAM;
        Param.paramLength = sizeof(TTxDataCounters);
        Param.content.pTxDataCounters = &txCounters[0];
        res = cmdDispatch_GetParam (pCmdInterpret->hCmdDispatch, &Param);
        if (res == TI_OK)
        {   
            if (cmdObj->buffer1_len < sizeof(TI_UINT32))
            {
                PRINTF(DBG_NDIS_OIDS_VERY_LOUD, ("Buffer for parameter 0x%X is bigger then Param size\r\n", Param.paramType));
                *cmdObj->param4 = sizeof(TI_UINT32);
            }
            else
            {
                *(TI_UINT32*)cmdObj->buffer1 = Param.content.pTxDataCounters->MulticastBytesXmit;
                PRINTF(DBG_NDIS_OIDS_LOUD, ("TIWL: OID_GEN_MULTICAST_BYTES_XMIT %d, len %d\r\n", *(TI_UINT32 *)cmdObj->buffer1, cmdObj->buffer1_len));
            }
        }
        else
        {
            PRINTF(DBG_NDIS_OIDS_ERROR, ("cmdInterpret: ERROR on return from get param=%d, status=%d\r\n", Param.paramType, res));
            *cmdObj->param3 = 0;
        }
		break;

    case OID_GEN_MULTICAST_FRAMES_XMIT:
        Param.paramType = TX_CTRL_COUNTERS_PARAM;
        Param.paramLength = sizeof(TTxDataCounters);
        Param.content.pTxDataCounters = &txCounters[0];
        res = cmdDispatch_GetParam (pCmdInterpret->hCmdDispatch, &Param);
        if (res == TI_OK)
        {   
            if (cmdObj->buffer1_len < sizeof(TI_UINT32))
            {
                PRINTF(DBG_NDIS_OIDS_VERY_LOUD, ("Buffer for parameter 0x%X is bigger then Param size\r\n", Param.paramType));
            }
            else
            {
                *(TI_UINT32*)cmdObj->buffer1 = Param.content.pTxDataCounters->MulticastFramesXmit;
                PRINTF(DBG_NDIS_OIDS_LOUD, ("TIWL: OID_GEN_MULTICAST_FRAMES_XMIT %d, len %d\r\n", *(TI_UINT32 *)cmdObj->buffer1, cmdObj->buffer1_len));
            }
        }
        else
        {
            PRINTF(DBG_NDIS_OIDS_ERROR, ("cmdInterpret: ERROR on return from get param=%d, status=%d\r\n", Param.paramType, res));
            *cmdObj->param3 = 0;
        }
		break;

    case OID_GEN_BROADCAST_BYTES_XMIT:
        Param.paramType = TX_CTRL_COUNTERS_PARAM;
        Param.paramLength = sizeof(TTxDataCounters);
        Param.content.pTxDataCounters = &txCounters[0];
        res = cmdDispatch_GetParam (pCmdInterpret->hCmdDispatch, &Param);
        if (res == TI_OK)
        {   
            if (cmdObj->buffer1_len < sizeof(TI_UINT32))
            {
                PRINTF(DBG_NDIS_OIDS_VERY_LOUD, ("Buffer for parameter 0x%X is bigger then Param size\r\n", Param.paramType));
                *cmdObj->param4 = sizeof(TI_UINT32);
            }
            else
            {
                *(TI_UINT32*)cmdObj->buffer1 = Param.content.pTxDataCounters->BroadcastBytesXmit;
                PRINTF(DBG_NDIS_OIDS_LOUD, ("TIWL: OID_GEN_BROADCAST_BYTES_XMIT %d, len %d\r\n", *(TI_UINT32 *)cmdObj->buffer1, cmdObj->buffer1_len));
            }
        }
        else
        {
            PRINTF(DBG_NDIS_OIDS_ERROR, ("cmdInterpret: ERROR on return from get param=%d, status=%d\r\n", Param.paramType, res));
            *cmdObj->param3 = 0;
        }
		break;

    case OID_GEN_BROADCAST_FRAMES_XMIT:
        Param.paramType = TX_CTRL_COUNTERS_PARAM;
        Param.paramLength = sizeof(TTxDataCounters);
        Param.content.pTxDataCounters = &txCounters[0];
        res = cmdDispatch_GetParam (pCmdInterpret->hCmdDispatch, &Param);
        if (res == TI_OK)
        {   
            if (cmdObj->buffer1_len < sizeof(TI_UINT32))
            {
                PRINTF(DBG_NDIS_OIDS_VERY_LOUD, ("Buffer for parameter 0x%X is bigger then Param size\r\n", Param.paramType));
                *cmdObj->param4 = sizeof(TI_UINT32);
            }
            else
            {
                *(TI_UINT32*)cmdObj->buffer1 = Param.content.pTxDataCounters->BroadcastFramesXmit;
                PRINTF(DBG_NDIS_OIDS_LOUD, ("TIWL: OID_GEN_BROADCAST_FRAMES_XMIT %d, len %d\r\n", *(TI_UINT32 *)cmdObj->buffer1, cmdObj->buffer1_len));
            }
        }
        else
        {
            PRINTF(DBG_NDIS_OIDS_ERROR, ("cmdInterpret: ERROR on return from get param=%d, status=%d\r\n", Param.paramType, res));
            *cmdObj->param3 = 0;
        }
		break;

    case OID_GEN_DIRECTED_BYTES_RCV:
        Param.paramType = RX_DATA_COUNTERS_PARAM;
        Param.paramLength = sizeof(rxDataCounters_t);
        res = cmdDispatch_GetParam (pCmdInterpret->hCmdDispatch, &Param);
        if (res == TI_OK)
        {   
            if (cmdObj->buffer1_len < sizeof(TI_UINT32))
            {
                PRINTF(DBG_NDIS_OIDS_VERY_LOUD, ("Buffer for parameter 0x%X is bigger then Param size\r\n", Param.paramType));
                *cmdObj->param4 = sizeof(TI_UINT32);
            }
            else
            {
                *(TI_UINT32*)cmdObj->buffer1 = Param.content.siteMgrTiWlanCounters.DirectedBytesRecv;
                PRINTF(DBG_NDIS_OIDS_LOUD, ("TIWL: OID_GEN_DIRECTED_BYTES_RCV %d, len %d\r\n", *(TI_UINT32 *)cmdObj->buffer1, cmdObj->buffer1_len));
            }
        }
        else
        {
            PRINTF(DBG_NDIS_OIDS_ERROR, ("cmdInterpret: ERROR on return from get param=%d, status=%d\r\n", Param.paramType, res));
           *cmdObj->param3 = 0;
        }
		break;

    case OID_GEN_DIRECTED_FRAMES_RCV:
        Param.paramType = RX_DATA_COUNTERS_PARAM;
        Param.paramLength = sizeof(rxDataCounters_t);
        res = cmdDispatch_GetParam (pCmdInterpret->hCmdDispatch, &Param);
        if (res == TI_OK)
        {   
            if (cmdObj->buffer1_len < sizeof(TI_UINT32))
            {
                PRINTF(DBG_NDIS_OIDS_VERY_LOUD, ("Buffer for parameter 0x%X is bigger then Param size\r\n", Param.paramType));
            }
            else
            {
                *(TI_UINT32*)cmdObj->buffer1 = Param.content.siteMgrTiWlanCounters.DirectedFramesRecv;
                PRINTF(DBG_NDIS_OIDS_LOUD, ("TIWL: OID_GEN_DIRECTED_FRAMES_RCV %d, len %d\r\n", *(TI_UINT32 *)cmdObj->buffer1, cmdObj->buffer1_len));
            }
        }
        else
        {
            PRINTF(DBG_NDIS_OIDS_ERROR, ("cmdInterpret: ERROR on return from get param=%d, status=%d\r\n", Param.paramType, res));
           *cmdObj->param3 = 0;
        }
		break;

    case OID_GEN_MULTICAST_BYTES_RCV:
        Param.paramType = RX_DATA_COUNTERS_PARAM;
        Param.paramLength = sizeof(rxDataCounters_t);
        res = cmdDispatch_GetParam (pCmdInterpret->hCmdDispatch, &Param);
        if (res == TI_OK)
        {   
            if (cmdObj->buffer1_len < sizeof(TI_UINT32))
            {
                PRINTF(DBG_NDIS_OIDS_VERY_LOUD, ("Buffer for parameter 0x%X is bigger then Param size\r\n", Param.paramType));
                *cmdObj->param4 = sizeof(TI_UINT32);
            }
            else
            {
                *(TI_UINT32*)cmdObj->buffer1 = Param.content.siteMgrTiWlanCounters.MulticastBytesRecv;
                PRINTF(DBG_NDIS_OIDS_LOUD, ("TIWL: OID_GEN_MULTICAST_BYTES_RCV %d, len %d\r\n", *(TI_UINT32 *)cmdObj->buffer1, cmdObj->buffer1_len));
            }
        }
        else
        {
            PRINTF(DBG_NDIS_OIDS_ERROR, ("cmdInterpret: ERROR on return from get param=%d, status=%d\r\n", Param.paramType, res));
           *cmdObj->param3 = 0;
        }
		break;

    case OID_GEN_MULTICAST_FRAMES_RCV:
        Param.paramType = RX_DATA_COUNTERS_PARAM;
        Param.paramLength = sizeof(rxDataCounters_t);
        res = cmdDispatch_GetParam (pCmdInterpret->hCmdDispatch, &Param);
        if (res == TI_OK)
        {   
            if (cmdObj->buffer1_len < sizeof(TI_UINT32))
            {
                PRINTF(DBG_NDIS_OIDS_VERY_LOUD, ("Buffer for parameter 0x%X is bigger then Param size\r\n", Param.paramType));
                *cmdObj->param4 = sizeof(TI_UINT32);
            }
            else
            {
                *(TI_UINT32*)cmdObj->buffer1 = Param.content.siteMgrTiWlanCounters.MulticastFramesRecv;
                PRINTF(DBG_NDIS_OIDS_LOUD, ("TIWL: OID_GEN_MULTICAST_FRAMES_RCV %d, len %d\r\n", *(TI_UINT32 *)cmdObj->buffer1, cmdObj->buffer1_len));
            }
        }
        else
        {
            PRINTF(DBG_NDIS_OIDS_ERROR, ("cmdInterpret: ERROR on return from get param=%d, status=%d\r\n", Param.paramType, res));
           *cmdObj->param3 = 0;
        }
		break;

    case OID_GEN_BROADCAST_BYTES_RCV:
        Param.paramType = RX_DATA_COUNTERS_PARAM;
        Param.paramLength = sizeof(rxDataCounters_t);
        res = cmdDispatch_GetParam (pCmdInterpret->hCmdDispatch, &Param);
        if (res == TI_OK)
        {   
            if (cmdObj->buffer1_len < sizeof(TI_UINT32))
            {
                PRINTF(DBG_NDIS_OIDS_VERY_LOUD, ("Buffer for parameter 0x%X is bigger then Param size\r\n", Param.paramType));
                *cmdObj->param4 = sizeof(TI_UINT32);
            }
            else
            {
                *(TI_UINT32*)cmdObj->buffer1 = Param.content.siteMgrTiWlanCounters.BroadcastBytesRecv;
                PRINTF(DBG_NDIS_OIDS_LOUD, ("TIWL: OID_GEN_BROADCAST_BYTES_RCV %d, len %d\r\n", *(TI_UINT32 *)cmdObj->buffer1, cmdObj->buffer1_len));
            }
        }
        else
        {
            PRINTF(DBG_NDIS_OIDS_ERROR, ("cmdInterpret: ERROR on return from get param=%d, status=%d\r\n", Param.paramType, res));
           *cmdObj->param3 = 0;
        }
		break;

    case OID_GEN_BROADCAST_FRAMES_RCV:
        Param.paramType = RX_DATA_COUNTERS_PARAM;
        Param.paramLength = sizeof(rxDataCounters_t);
        res = cmdDispatch_GetParam (pCmdInterpret->hCmdDispatch, &Param);
        if (res == TI_OK)
        {   
            if (cmdObj->buffer1_len < sizeof(TI_UINT32))
            {
                PRINTF(DBG_NDIS_OIDS_VERY_LOUD, ("Buffer for parameter 0x%X is bigger then Param size\r\n", Param.paramType));
                *cmdObj->param4 = sizeof(TI_UINT32);
            }
            else
            {
                *(TI_UINT32*)cmdObj->buffer1 = Param.content.siteMgrTiWlanCounters.BroadcastFramesRecv;
                PRINTF(DBG_NDIS_OIDS_LOUD, ("TIWL: OID_GEN_BROADCAST_FRAMES_RCV %d, len %d\r\n", *(TI_UINT32 *)cmdObj->buffer1, cmdObj->buffer1_len));
            }
        }
        else
        {
            PRINTF(DBG_NDIS_OIDS_ERROR, ("cmdInterpret: ERROR on return from get param=%d, status=%d\r\n", Param.paramType, res));
           *cmdObj->param3 = 0;
        }
		break;

    case OID_GEN_RCV_CRC_ERROR:
        Param.paramType = RX_DATA_COUNTERS_PARAM;
        Param.paramLength = sizeof(rxDataCounters_t);
        res = cmdDispatch_GetParam (pCmdInterpret->hCmdDispatch, &Param);
        if (res == TI_OK)
        {   
            if (cmdObj->buffer1_len < sizeof(TI_UINT32))
            {
                PRINTF(DBG_NDIS_OIDS_VERY_LOUD, ("Buffer for parameter 0x%X is bigger then Param size\r\n", Param.paramType));
                *cmdObj->param4 = sizeof(TI_UINT32);
            }
            else
            {
                *(TI_UINT32*)cmdObj->buffer1 = Param.content.siteMgrTiWlanCounters.FcsErrors;
                PRINTF(DBG_NDIS_OIDS_LOUD, ("TIWL: OID_GEN_RCV_CRC_ERROR %d, len %d\r\n", *(TI_UINT32 *)cmdObj->buffer1, cmdObj->buffer1_len));
            }
        }
        else
        {
            PRINTF(DBG_NDIS_OIDS_ERROR, ("cmdInterpret: ERROR on return from get param=%d, status=%d\r\n", Param.paramType, res));
           *cmdObj->param3 = 0;
        }
		break;

    case OID_GEN_INIT_TIME_MS:
        if (cmdObj->buffer1_len >= sizeof(TI_UINT32))
        {
            *(TI_UINT32*)cmdObj->buffer1 = pAdapter->InitTime;
            PRINTF(DBG_NDIS_OIDS_LOUD, ("TIWL: OID_GEN_INIT_TIME_MS %d\r\n", *(TI_UINT32 *)cmdObj->buffer1));
        }
        else
        {
            PRINTF(DBG_NDIS_OIDS_VERY_LOUD, ("cmdInterpret: Wrong buffer size in OID_GEN_MAXIMUM_TOTAL_SIZE\r\n"));
            *cmdObj->param4 = sizeof(TI_UINT32);
        }
		break;

	case OID_GEN_MEDIA_SENSE_COUNTS:
        *(TI_UINT32*)cmdObj->buffer1 = 0;
        PRINTF(DBG_NDIS_OIDS_LOUD, ("TIWL: OID_GEN_MEDIA_SENSE_COUNTS %d\r\n", *(TI_UINT32 *)cmdObj->buffer1));
        break;

    case OID_802_3_CURRENT_ADDRESS:
        if (ETH_ADDR_SIZE > cmdObj->buffer1_len)
        {
            PRINTF(DBG_NDIS_OIDS_VERY_LOUD, ("Buffer for parameter 0x%X is bigger then Param size\r\n", OID_802_3_CURRENT_ADDRESS));
            *cmdObj->param4 = ETH_ADDR_SIZE;
        }
        else
        {
            os_memoryCopy(pCmdInterpret->hOs, cmdObj->buffer1, pAdapter->CurrentAddr, ETH_ADDR_SIZE);
            *cmdObj->param3 = ETH_ADDR_SIZE;
        }
        PRINTF(DBG_NDIS_OIDS_LOUD, ("TIWL: OID_802_3_CURRENT_ADDRESS %02x:%02x%02x%02x%02x%02x, len %d\r\n",
                                    *((UCHAR *)cmdObj->buffer1), *((UCHAR *)cmdObj->buffer1 + 1),
                                    *((UCHAR *)cmdObj->buffer1 + 2), *((UCHAR *)cmdObj->buffer1 + 3),
                                    *((UCHAR *)cmdObj->buffer1 + 4), *((UCHAR *)cmdObj->buffer1 + 5),
                                    cmdObj->buffer1_len));

		break;

    case OID_802_3_PERMANENT_ADDRESS:
        if (ETH_ADDR_SIZE > cmdObj->buffer1_len)
        {
            PRINTF(DBG_NDIS_OIDS_VERY_LOUD, ("Buffer for parameter 0x%X is bigger then Param size\r\n", OID_802_3_CURRENT_ADDRESS));
            *cmdObj->param4 = ETH_ADDR_SIZE;
        }
        else
        {
            os_memoryCopy(pCmdInterpret->hOs, cmdObj->buffer1, pAdapter->PermanentAddr, ETH_ADDR_SIZE);
            *cmdObj->param3 = ETH_ADDR_SIZE;
        }
        PRINTF(DBG_NDIS_OIDS_LOUD, ("TIWL: OID_802_3_PERMANENT_ADDRESS %02x:%02x%02x%02x%02x%02x, len %d\r\n",
                                    *((UCHAR *)cmdObj->buffer1), *((UCHAR *)cmdObj->buffer1 + 1),
                                    *((UCHAR *)cmdObj->buffer1 + 2), *((UCHAR *)cmdObj->buffer1 + 3),
                                    *((UCHAR *)cmdObj->buffer1 + 4), *((UCHAR *)cmdObj->buffer1 + 5),
                                    cmdObj->buffer1_len));
		break;

    case OID_802_3_MAXIMUM_LIST_SIZE:
        if (cmdObj->buffer1_len >= sizeof(TI_UINT32))
        {
            *(TI_UINT32*)cmdObj->buffer1 = MAX_MULTICAST_ADDRESSES;
            PRINTF(DBG_NDIS_OIDS_LOUD, ("TIWL: OID_802_3_MAXIMUM_LIST_SIZE %d\r\n", *(TI_UINT32 *)cmdObj->buffer1));
        }
        else
        {
            PRINTF(DBG_NDIS_OIDS_VERY_LOUD, ("cmdInterpret: Wrong buffer size in OID_802_3_MAXIMUM_LIST_SIZE\r\n"));
            *cmdObj->param4 = sizeof(TI_UINT32);
        }
		break;

    case OID_802_3_RCV_ERROR_ALIGNMENT:
        *(TI_UINT32*)cmdObj->buffer1 = 0;
        PRINTF(DBG_NDIS_OIDS_LOUD, ("TIWL: OID_802_3_RCV_ERROR_ALIGNMENT %d\r\n", *(TI_UINT32 *)cmdObj->buffer1));
		break;

    case OID_802_3_XMIT_ONE_COLLISION:
        *(TI_UINT32*)cmdObj->buffer1 = 0;
        PRINTF(DBG_NDIS_OIDS_LOUD, ("TIWL: OID_802_3_XMIT_ONE_COLLISION %d\r\n", *(TI_UINT32 *)cmdObj->buffer1));
		break;

    case OID_802_3_XMIT_MORE_COLLISIONS:
        *(TI_UINT32*)cmdObj->buffer1 = 0;
        PRINTF(DBG_NDIS_OIDS_LOUD, ("TIWL: OID_802_3_XMIT_MORE_COLLISIONS %d\r\n", *(TI_UINT32 *)cmdObj->buffer1));
		break;

    case OID_802_3_MULTICAST_LIST:
		PRINT(DBG_NDIS_OIDS_LOUD, "TIWL: (SET) OID_802_3_MULTICAST_LIST\r\n");

        if (cmdObj->buffer1_len > ETH_ADDR_SIZE * MAX_MULTICAST_ADDRESSES) 
        {
			PRINT(DBG_NDIS_OIDS_VERY_LOUD, "...LIST FULL\r\n");
			*cmdObj->param4 = ETH_ADDR_SIZE * MAX_MULTICAST_ADDRESSES;
		}
        else
        {
    		/* Save the multicast list.*/
            os_memoryCopy(pCmdInterpret->hOs, &pAdapter->MulticastTable, cmdObj->buffer1, cmdObj->buffer1_len);   
    		pAdapter->NumMulticastAddresses = cmdObj->buffer1_len / ETH_ADDR_SIZE;
    		*cmdObj->param3 = cmdObj->buffer1_len;
        }
		break;

	case OID_802_11_BSSID:

        if (cmdObj->flags == OS_OID_SET_FLAG)
        {
            /* If MAC address is zeroes -> connect to "ANY" BSSID */
           if (MAC_NULL ((TI_UINT8*)cmdObj->buffer1))
           {
               /* Convert to "FF:FF:FF:FF:FF:FF" since this driver requires this value */
               MAC_COPY (Param.content.siteMgrDesiredBSSID, "\xff\xff\xff\xff\xff\xff");
           } 
           else
           {
               MAC_COPY (Param.content.siteMgrDesiredBSSID, (TI_UINT8*)cmdObj->buffer1);
           }
           Param.paramType   = SITE_MGR_DESIRED_BSSID_PARAM;
           Param.paramLength = sizeof(TMacAddr);
           *cmdObj->param3   = sizeof (TMacAddr);
           res = cmdDispatch_SetParam ( pCmdInterpret->hCmdDispatch, &Param );           
           /* also set it to the SME */
           Param.paramType   = SME_DESIRED_BSSID_PARAM;
           res = cmdDispatch_SetParam ( pCmdInterpret->hCmdDispatch, &Param );
        }
        else
        {
    		if(pAdapter->LinkStatus == NdisMediaStateDisconnected)
    		{
                MAC_COPY (Param.content.siteMgrDesiredBSSID, "\x00\x00\x00\x00\x00\x00");
                *cmdObj->param3 = MAC_ADDR_LEN;
    		}
            else
            {
                Param.paramType   = SITE_MGR_CURRENT_BSSID_PARAM;
                Param.paramLength = sizeof(TMacAddr);
                res = cmdDispatch_GetParam (pCmdInterpret->hCmdDispatch, &Param);

				if(res == NO_SITE_SELECTED_YET)
					res = TI_OK;

                if (res == TI_OK)
                {   
                    if (cmdObj->buffer1_len < sizeof(TMacAddr))
                    {
                        PRINTF(DBG_NDIS_OIDS_VERY_LOUD, ("Buffer for parameter 0x%X is bigger then Param size\r\n", Param.paramType));
                        *cmdObj->param4 = MAC_ADDR_LEN;
                    }
                    else
                    {
                        os_memoryCopy(pCmdInterpret->hOs, cmdObj->buffer1, Param.content.siteMgrDesiredBSSID, sizeof(TMacAddr));
                        *cmdObj->param3 = MAC_ADDR_LEN;
                        PRINTF(DBG_NDIS_OIDS_LOUD, ("TIWL: OID_802_11_BSSID %02x:%02x%02x%02x%02x%02x, len %d\r\n",
                                            *((UCHAR *)cmdObj->buffer1), *((UCHAR *)cmdObj->buffer1 + 1),
                                            *((UCHAR *)cmdObj->buffer1 + 2), *((UCHAR *)cmdObj->buffer1 + 3),
                                            *((UCHAR *)cmdObj->buffer1 + 4), *((UCHAR *)cmdObj->buffer1 + 5),
                                            cmdObj->buffer1_len));
                    }
                }
                else
                {
                    PRINTF(DBG_NDIS_OIDS_ERROR, ("cmdInterpret: ERROR on return from get param=%d, status=%d\r\n", Param.paramType, res));
                   *cmdObj->param3 = 0;
                }
            }
        }
        break;

    case OID_802_11_SSID:
    {
        OS_802_11_SSID* pSsid;

        pSsid = (OS_802_11_SSID*) cmdObj->buffer1;
        if (cmdObj->flags == OS_OID_SET_FLAG)
        {           
            if (pSsid->SsidLength == 0)
            {   
                /* The driver should support setting the SSID to NULL string -> any SSID */
                os_memoryCopy(pCmdInterpret->hOs, &Param.content.siteMgrDesiredSSID.str, "\00", 1);
                Param.content.siteMgrDesiredSSID.len = 0;
            }
            else
        	{
            if (OS_802_11_SSID_JUNK (pSsid->Ssid, pSsid->SsidLength))
            {
//                    WLAN_OS_REPORT(("set OID_802_11_SSID |JUNK|\r\n"));
            }
            else
            {
                    WLAN_OS_REPORT(("set OID_802_11_SSID |%s|\r\n",pSsid->Ssid));
            }
                os_memoryCopy(pCmdInterpret->hOs, &Param.content.siteMgrDesiredSSID.str, pSsid->Ssid, pSsid->SsidLength);
                Param.content.siteMgrDesiredSSID.len = pSsid->SsidLength;
                *cmdObj->param3 = cmdObj->buffer1_len;
            }
            Param.paramType = SITE_MGR_DESIRED_SSID_PARAM;
            Param.paramLength = sizeof (TSsid);
            res = cmdDispatch_SetParam (pCmdInterpret->hCmdDispatch, &Param);

            /* also set the desired SSID to the SME */
            Param.paramType = SME_DESIRED_SSID_ACT_PARAM;
            Param.paramLength = sizeof (TSsid);
            res = cmdDispatch_SetParam (pCmdInterpret->hCmdDispatch, &Param);
        }
        else
        {
            Param.paramType = SITE_MGR_CURRENT_SSID_PARAM;
            res = cmdDispatch_GetParam (pCmdInterpret->hCmdDispatch, &Param);

			if(res == NO_SITE_SELECTED_YET)
				res = TI_OK;

            os_memoryCopy(pCmdInterpret->hOs, pSsid->Ssid, &Param.content.siteMgrCurrentSSID.str[0], Param.content.siteMgrCurrentSSID.len);
            pSsid->SsidLength = Param.content.siteMgrCurrentSSID.len;
            *cmdObj->param3 = sizeof(OS_802_11_SSID);
            PRINTF(DBG_NDIS_OIDS_LOUD, ("TIWL: OID_802_11_SSID %s, len %d\r\n", (TI_UINT8*)cmdObj->buffer1, cmdObj->buffer1_len));
        }
        break;
    }
	case OID_802_11_NETWORK_TYPES_SUPPORTED:
        Param.paramType = SITE_MGR_DESIRED_DOT11_MODE_PARAM;
        Param.paramLength = sizeof(TI_UINT32);        
        res = cmdDispatch_GetParam (pCmdInterpret->hCmdDispatch, &Param);
        if (res == TI_OK)
        {   
            if (cmdObj->buffer1_len < sizeof(TI_UINT32))
            {
                PRINTF(DBG_NDIS_OIDS_VERY_LOUD, ("Buffer for parameter 0x%X is bigger then Param size\r\n", Param.paramType));
            }
            else
            {
                *(TI_UINT32*)cmdObj->buffer1 = Param.content.siteMgrDot11Mode;
                PRINTF(DBG_NDIS_OIDS_LOUD, ("TIWL: OID_802_11_NETWORK_TYPES_SUPPORTED %d\r\n", *(TI_UINT32 *)cmdObj->buffer1));
            }
        }
        else
        {
            PRINTF(DBG_NDIS_OIDS_ERROR, ("cmdInterpret: ERROR on return from get param=%d, status=%d\r\n", Param.paramType, res));
           *cmdObj->param3 = 0;
        }
        break;

    case OID_802_11_NETWORK_TYPE_IN_USE:
        if (cmdObj->flags == OS_OID_SET_FLAG)
        {
            Param.paramType = SITE_MGR_DESIRED_DOT11_MODE_PARAM;
            Param.paramLength = sizeof(TI_UINT32); 
            Param.content.siteMgrDot11Mode = *((TI_UINT32*)cmdObj->buffer1);
            res = cmdDispatch_SetParam (pCmdInterpret->hCmdDispatch, &Param);
        }
        else
        {
            Param.paramType = SITE_MGR_NETWORK_TYPE_IN_USE;
            Param.paramLength = sizeof(TI_UINT32);
            res = cmdDispatch_GetParam (pCmdInterpret->hCmdDispatch, &Param);
            if (res == TI_OK)
            {   
                if (cmdObj->buffer1_len < sizeof(TI_UINT32))
                {
                    PRINTF(DBG_NDIS_OIDS_VERY_LOUD, ("Buffer for parameter 0x%X is bigger then Param size\r\n", Param.paramType));
                    *cmdObj->param4 = sizeof(TI_UINT32);
                }
                else
                {
                    *(TI_UINT32*)cmdObj->buffer1 = Param.content.siteMgrDot11Mode;
                    PRINTF(DBG_NDIS_OIDS_LOUD, ("TIWL: SITE_MGR_NETWORK_TYPE_IN_USE %d\r\n", *(TI_UINT32 *)cmdObj->buffer1));
                }
            }
            else
            {
                PRINTF(DBG_NDIS_OIDS_ERROR, ("cmdInterpret: ERROR on return from get param=%d, status=%d\r\n", Param.paramType, res));
               *cmdObj->param3 = 0;
            }
        }
        break;
        
    case OID_802_11_TX_POWER_LEVEL:
        Param.paramType = REGULATORY_DOMAIN_CURRENT_TX_POWER_IN_DBM_PARAM;
        Param.paramLength = sizeof(TI_INT8);
        if (cmdObj->flags == OS_OID_SET_FLAG)
        {
            TI_UINT32 uMW;
            TI_INT8 nPower, nDbm;

            /* converting power to DBM */
            uMW = *(TI_UINT32*)cmdObj->buffer1;
            for (nPower=1; uMW/10; uMW/=10, nPower++); 
			nDbm = 20 * nPower;

            Param.content.desiredTxPower = nDbm; /* 1/10 DBM*/
            *cmdObj->param3 = sizeof(TI_INT8);
            res = cmdDispatch_SetParam (pCmdInterpret->hCmdDispatch, &Param);
        }
        else
        {
            res = cmdDispatch_GetParam (pCmdInterpret->hCmdDispatch, &Param);
            if (res == TI_OK)
            {   
                if (cmdObj->buffer1_len < sizeof(TI_INT8))
                {
                    PRINTF(DBG_NDIS_OIDS_VERY_LOUD, ("Buffer for parameter 0x%X is bigger then Param size\r\n", Param.paramType));
                    *cmdObj->param4 = sizeof(TI_INT8);
                }
                else
                {
                    *(TI_INT8*)cmdObj->buffer1 = Param.content.desiredTxPower;
                    PRINTF(DBG_NDIS_OIDS_LOUD, ("TIWL: OID_802_11_TX_POWER_LEVEL %d\r\n", *(TI_UINT8 *)cmdObj->buffer1));
                }
            }
            else
            {
                PRINTF(DBG_NDIS_OIDS_ERROR, ("cmdInterpret: ERROR on return from get param=%d, status=%d\r\n", Param.paramType, res));
               *cmdObj->param3 = 0;
            }
        }
		break;

	case OID_802_11_RSSI: 
    {
        Param.paramType = SITE_MGR_GET_SELECTED_BSSID_INFO;
        Param.paramLength = sizeof(TI_INT8);
        SelectedBSSID.Rssi = 0;
        Param.content.pSiteMgrPrimarySiteDesc = &SelectedBSSID;
        res = cmdDispatch_GetParam (pCmdInterpret->hCmdDispatch, &Param);
        if (res == TI_OK)
        {   
            if (cmdObj->buffer1_len < sizeof(TI_INT32))
            {
                PRINTF(DBG_NDIS_OIDS_VERY_LOUD, ("Buffer for parameter 0x%X is bigger then Param size\r\n", Param.paramType));
                *cmdObj->param4 = sizeof(TI_UINT32);
            }
            else
            {
                *(TI_INT32*)cmdObj->buffer1 = SelectedBSSID.Rssi;
                PRINTF(DBG_NDIS_OIDS_LOUD, ("TIWL: OID_802_11_RSSI %d\r\n", *(TI_UINT8 *)cmdObj->buffer1));
            }
        }
        else
        {
            PRINTF(DBG_NDIS_OIDS_ERROR, ("cmdInterpret: ERROR on return from get param=%d, status=%d\r\n", Param.paramType, res));
           *cmdObj->param3 = 0;
        }
		break;
    }

    case OID_802_11_RSSI_TRIGGER:
        if (cmdObj->flags == OS_OID_SET_FLAG)
        {
            *cmdObj->param3 = sizeof(TI_UINT32);
        }
        else
        {
            *(TI_INT32*)cmdObj->buffer1 = 0;
            PRINTF(DBG_NDIS_OIDS_LOUD, ("TIWL: OID_802_11_RSSI_TRIGGER %d\r\n", *(TI_UINT32 *)cmdObj->buffer1));
        }       
        break;

    case OID_802_11_INFRASTRUCTURE_MODE:
        if (cmdObj->flags == OS_OID_SET_FLAG)
        {
            Param.paramType = SITE_MGR_DESIRED_BSS_TYPE_PARAM;
            Param.paramLength = sizeof(ScanBssType_e);
            *cmdObj->param3 = sizeof(ScanBssType_e);
            Param.content.siteMgrDesiredBSSType = *((ScanBssType_e*)cmdObj->buffer1);
            res = cmdDispatch_SetParam (pCmdInterpret->hCmdDispatch, &Param);
            
            /* Also set it to the SME */
            Param.paramType = SME_DESIRED_BSS_TYPE_PARAM;
            res = cmdDispatch_SetParam (pCmdInterpret->hCmdDispatch, &Param);
        }
        else
        {
            Param.paramType = CTRL_DATA_CURRENT_BSS_TYPE_PARAM;
            Param.paramLength = sizeof(ScanBssType_e);
            res = cmdDispatch_GetParam (pCmdInterpret->hCmdDispatch, &Param);
            if (res == TI_OK)
            {   
                if (cmdObj->buffer1_len < sizeof(ScanBssType_e))
                {
                    PRINTF(DBG_NDIS_OIDS_VERY_LOUD, ("Buffer for parameter 0x%X is bigger then Param size\r\n", Param.paramType));
                    *cmdObj->param4 = sizeof(ScanBssType_e);
                }
                else
                {
                    *(TI_INT32*)cmdObj->buffer1 = (ScanBssType_e)Param.content.ctrlDataCurrentBssType;
                    *cmdObj->param3 = sizeof(ScanBssType_e);
                    PRINTF(DBG_NDIS_OIDS_LOUD, ("TIWL: OID_802_11_FRAGMENTATION_THRESHOLD %d\r\n", *(TI_UINT32 *)cmdObj->buffer1));
                }
            }
            else
            {
                PRINTF(DBG_NDIS_OIDS_ERROR, ("cmdInterpret: ERROR on return from get param=%d, status=%d\r\n", Param.paramType, res));
                *cmdObj->param3 = 0;
            }
        }
        break;

    case OID_802_11_FRAGMENTATION_THRESHOLD:
        Param.paramType = TWD_FRAG_THRESHOLD_PARAM;
        Param.paramLength = sizeof(TI_UINT16);
        if (cmdObj->flags == OS_OID_SET_FLAG)
        {
            TI_UINT16 uFragThreshold = *((TI_UINT16*)cmdObj->buffer1);
            /* make FragThreshold to be always even */
            uFragThreshold = ((uFragThreshold+1)>>1) << 1; 
            Param.content.halCtrlFragThreshold = uFragThreshold;
            *cmdObj->param3 = sizeof(TI_UINT16);
            res = cmdDispatch_SetParam (pCmdInterpret->hCmdDispatch, &Param);
        }
        else
        {
            res = cmdDispatch_GetParam (pCmdInterpret->hCmdDispatch, &Param);
            if (res == TI_OK)
            {   
                if (cmdObj->buffer1_len < sizeof(TI_UINT16))
                {
                    PRINTF(DBG_NDIS_OIDS_VERY_LOUD, ("Buffer for parameter 0x%X is bigger then Param size\r\n", Param.paramType));
                    *cmdObj->param4 = sizeof(TI_UINT16);
                }
                else
                {
                    *(TI_UINT16*)cmdObj->buffer1 = Param.content.halCtrlFragThreshold;
                    *cmdObj->param3 = sizeof(TI_UINT16);
                    PRINTF(DBG_NDIS_OIDS_LOUD, ("TIWL: OID_802_11_FRAGMENTATION_THRESHOLD %d\r\n", *(TI_UINT16 *)cmdObj->buffer1));
                }
            }
            else
            {
                PRINTF(DBG_NDIS_OIDS_ERROR, ("cmdInterpret: ERROR on return from get param=%d, status=%d\r\n", Param.paramType, res));
                *cmdObj->param3 = 0;
            }
        }
		break;

	case OID_802_11_RTS_THRESHOLD:
        Param.paramType = TWD_RTS_THRESHOLD_PARAM;
        Param.paramLength = sizeof(TI_UINT16);
        res = cmdDispatch_GetParam (pCmdInterpret->hCmdDispatch, &Param);
        if (res == TI_OK)
        {   
            if (cmdObj->buffer1_len < sizeof(TI_UINT16))
            {
                PRINTF(DBG_NDIS_OIDS_VERY_LOUD, ("Buffer for parameter 0x%X is bigger then Param size\r\n", Param.paramType));
                *cmdObj->param4 = sizeof(TI_UINT16);
            }
            else
            {
                *(TI_UINT16*)cmdObj->buffer1 = Param.content.halCtrlRtsThreshold;
                *cmdObj->param3 = sizeof(TI_UINT16);
                PRINTF(DBG_NDIS_OIDS_LOUD, ("TIWL: OID_802_11_RTS_THRESHOLD %d\r\n", *(TI_UINT16 *)cmdObj->buffer1));
            }
        }
        else
        {
            PRINTF(DBG_NDIS_OIDS_ERROR, ("cmdInterpret: ERROR on return from get param=%d, status=%d\r\n", Param.paramType, res));
            *cmdObj->param3 = 0;
        }
		break;

	case OID_802_11_NUMBER_OF_ANTENNAS:
        if (cmdObj->buffer1_len >= sizeof(TI_UINT32))
        {
            *(TI_INT32*)cmdObj->buffer1 = NUMBER_OF_ANTENNAS;
            PRINTF(DBG_NDIS_OIDS_LOUD, ("TIWL: OID_802_11_NUMBER_OF_ANTENNAS %d\r\n", *(TI_UINT32 *)cmdObj->buffer1));
        }
        else
        {
            PRINTF(DBG_NDIS_OIDS_VERY_LOUD, ("cmdInterpret: Wrong buffer size in OID_802_3_MAXIMUM_LIST_SIZE\r\n"));
            *cmdObj->param4 = sizeof(TI_UINT32);
        }
		break;

    case OID_802_11_RX_ANTENNA_SELECTED:
        ndisStatus = NDIS_STATUS_NOT_SUPPORTED;
        PRINTF(DBG_NDIS_OIDS_LOUD, ("TIWL: OID_802_11_RX_ANTENNA_SELECTED - NOT_SUPPORTED!\r\n"));       
        break;

    case OID_802_11_TX_ANTENNA_SELECTED:
        ndisStatus = NDIS_STATUS_NOT_SUPPORTED;
        PRINTF(DBG_NDIS_OIDS_LOUD, ("TIWL: OID_802_11_TX_ANTENNA_SELECTED - NOT_SUPPORTED!\r\n"));		

		break;

	case OID_802_11_SUPPORTED_RATES:
        Param.paramType = SITE_MGR_DESIRED_SUPPORTED_RATE_SET_PARAM;
        Param.paramLength = sizeof(TRates);
        res = cmdDispatch_GetParam (pCmdInterpret->hCmdDispatch, &Param);
        if (res == TI_OK)
        {   
            if (cmdObj->buffer1_len < sizeof(TRates))
            {
                PRINTF(DBG_NDIS_OIDS_VERY_LOUD, ("Buffer for parameter 0x%X is bigger then Param size\r\n", Param.paramType));
                *cmdObj->param4 = sizeof(TRates);
            }
            else
            {
                os_memoryCopy(pCmdInterpret->hOs, cmdObj->buffer1, &Param.content.siteMgrDesiredSupportedRateSet, sizeof(TRates));
                *cmdObj->param3 = sizeof(TRates);
                PRINTF(DBG_NDIS_OIDS_LOUD, ("TIWL: OID_802_11_SUPPORTED_RATES %s, len %d\r\n", (TI_UINT8*)cmdObj->buffer1, cmdObj->buffer1_len));
            }
        }
        else
        {
            *cmdObj->param3 = 0;
            ndisStatus = NDIS_STATUS_INVALID_LENGTH;
            PRINTF(DBG_NDIS_OIDS_ERROR, ("cmdInterpret: ERROR on return from get param=%d, status=%d\r\n", Param.paramType, res));
        }
        break;

	case OID_802_11_DESIRED_RATES:
        PRINTF(DBG_NDIS_OIDS_VERY_LOUD, ("OID_802_11_DESIRED_RATES is not supported!\r\n"));
        break;

	case OID_802_11_CONFIGURATION:
        Param.paramType = SITE_MGR_CONFIGURATION_PARAM;
        Param.paramLength = sizeof(OS_802_11_CONFIGURATION);
		Param.content.pSiteMgrConfiguration = &siteMgrConfiguration;

        if (cmdObj->flags == OS_OID_SET_FLAG)
        {
            os_memoryCopy(pCmdInterpret->hOs, Param.content.pSiteMgrConfiguration, cmdObj->buffer1, sizeof(OS_802_11_CONFIGURATION));
            *cmdObj->param3 = sizeof(OS_802_11_CONFIGURATION);
            res = cmdDispatch_SetParam (pCmdInterpret->hCmdDispatch, &Param);
        }
        else
        {
            res = cmdDispatch_GetParam (pCmdInterpret->hCmdDispatch, &Param);
            if (res == TI_OK)
            {   
                if (cmdObj->buffer1_len < sizeof(OS_802_11_CONFIGURATION))
                {
                    PRINTF(DBG_NDIS_OIDS_VERY_LOUD, ("Buffer for parameter 0x%X is bigger then Param size\r\n", Param.paramType));
                    *cmdObj->param4 = sizeof(OS_802_11_CONFIGURATION);
                }
                else
                {
                    os_memoryCopy(pCmdInterpret->hOs, cmdObj->buffer1, Param.content.pSiteMgrConfiguration, sizeof(OS_802_11_CONFIGURATION));
                    *cmdObj->param3 = sizeof(OS_802_11_CONFIGURATION);
                    PRINTF(DBG_NDIS_OIDS_LOUD, ("TIWL: OID_802_11_CONFIGURATION 0x%x, len %d\r\n", *((UINT32*)cmdObj->buffer1), cmdObj->buffer1_len));
                }
            }
            else
            {
                *cmdObj->param3 = 0;
                PRINTF(DBG_NDIS_OIDS_ERROR, ("cmdInterpret: ERROR on return from get param=%d, status=%d\r\n", Param.paramType, res));
            }
        }
        break;

	case OID_802_11_STATISTICS:
        if (cmdObj->buffer1_len >= sizeof(TIWLN_STATISTICS))
        {
            pStats = cmdObj->buffer1;
            *cmdObj->param3 = sizeof(TIWLN_STATISTICS);
            os_memoryZero(pCmdInterpret->hOs, pStats, sizeof(TIWLN_STATISTICS));
            /* currentMACAddress */
            os_memoryCopy(pCmdInterpret->hOs, &pStats->currentMACAddress, pAdapter->CurrentAddr, ETH_ADDR_SIZE);
            /* PowerMode */
            Param.paramType = POWER_MGR_POWER_MODE;
            Param.paramLength = sizeof(PowerMgr_PowerMode_e);
            res = cmdDispatch_GetParam (pCmdInterpret->hCmdDispatch, &Param);
            if (res == TI_OK)
            {
                pStats->PowerMode = (PowerMgr_PowerMode_e)Param.content.PowerMode;
            }
            else
            {
                PRINTF(DBG_NDIS_OIDS_ERROR, ("cmdInterpret: ERROR on return from get param=%d, status=%d\r\n", Param.paramType, res));
                *cmdObj->param3 = 0;
                break;
            }
            /* dot11DesiredSSID */
            Param.paramType = SITE_MGR_CURRENT_SSID_PARAM;
            Param.paramLength = sizeof(TSsid);
            res = cmdDispatch_GetParam (pCmdInterpret->hCmdDispatch, &Param);
            if (res == TI_OK)
            {   
                os_memoryCopy(pCmdInterpret->hOs, &pStats->dot11DesiredSSID, &Param.content.siteMgrCurrentSSID, sizeof(TSsid));
            }
            else
            {
                PRINTF(DBG_NDIS_OIDS_ERROR, ("cmdInterpret: ERROR on return from get param=%d, status=%d\r\n", Param.paramType, res));
                *cmdObj->param3 = 0;
                break;
            }
            /* dot11CurrentChannel */
            Param.paramType = SITE_MGR_CURRENT_CHANNEL_PARAM;
            Param.paramLength = sizeof(TI_UINT8);
            res = cmdDispatch_GetParam (pCmdInterpret->hCmdDispatch, &Param);
            if (res == TI_OK)
            {
                pStats->dot11CurrentChannel = (TI_UINT8)Param.content.siteMgrCurrentChannel;
            }
            else
            {
                PRINTF(DBG_NDIS_OIDS_ERROR, ("cmdInterpret: ERROR on return from get param=%d, status=%d\r\n", Param.paramType, res));
                *cmdObj->param3 = 0;
                break;
            }
            /* AuthenticationMode */
            Param.paramType = RSN_EXT_AUTHENTICATION_MODE;
            Param.paramLength = sizeof(EExternalAuthMode);
            res = cmdDispatch_GetParam (pCmdInterpret->hCmdDispatch, &Param);
            if (res == TI_OK)
            {
                pStats->AuthenticationMode = (EExternalAuthMode)Param.content.rsnExtAuthneticationMode;
            }
            else
            {
                PRINTF(DBG_NDIS_OIDS_ERROR, ("cmdInterpret: ERROR on return from get param=%d, status=%d\r\n", Param.paramType, res));
                *cmdObj->param3 = 0;
                break;
            }
            /* RTSThreshold */
            Param.paramType = TWD_RTS_THRESHOLD_PARAM;
            Param.paramLength = sizeof(TI_UINT16);
            res = cmdDispatch_GetParam (pCmdInterpret->hCmdDispatch, &Param);
            if (res == TI_OK)
            {
                pStats->RTSThreshold = (TI_UINT16)Param.content.halCtrlRtsThreshold;
            }
            else
            {
                PRINTF(DBG_NDIS_OIDS_ERROR, ("cmdInterpret: ERROR on return from get param=%d, status=%d\r\n", Param.paramType, res));
                *cmdObj->param3 = 0;
                break;
            }
            /* FragmentationThreshold */
            Param.paramType = TWD_FRAG_THRESHOLD_PARAM;
            Param.paramLength = sizeof(TI_UINT16);
            res = cmdDispatch_GetParam (pCmdInterpret->hCmdDispatch, &Param);
            if (res == TI_OK)
            {   
                 pStats->FragmentationThreshold = (TI_UINT16)Param.content.halCtrlFragThreshold;
            }
            else
            {
                PRINTF(DBG_NDIS_OIDS_ERROR, ("cmdInterpret: ERROR on return from get param=%d, status=%d\r\n", Param.paramType, res));
                *cmdObj->param3 = 0;
                break;
            }
            /* TxPowerDbm */
            Param.paramType = REGULATORY_DOMAIN_CURRENT_TX_POWER_IN_DBM_PARAM;
            Param.paramLength = sizeof(TI_INT8);
            res = cmdDispatch_GetParam (pCmdInterpret->hCmdDispatch, &Param);
            if (res == TI_OK)
            {   
                pStats->TxPowerDbm = (TI_INT8)Param.content.desiredTxPower;
            }
            else
            {
                PRINTF(DBG_NDIS_OIDS_ERROR, ("cmdInterpret: ERROR on return from get param=%d, status=%d\r\n", Param.paramType, res));
                *cmdObj->param3 = 0;
                break;
            }
            /* dot11BSSType */
            Param.paramType = CTRL_DATA_CURRENT_BSS_TYPE_PARAM;
            Param.paramLength = sizeof(ScanBssType_e);
            res = cmdDispatch_GetParam (pCmdInterpret->hCmdDispatch, &Param);
            if (res == TI_OK)
            {   
               pStats->dot11BSSType = (ScanBssType_e)Param.content.ctrlDataCurrentBssType;
            }
            else
            {
                PRINTF(DBG_NDIS_OIDS_ERROR, ("cmdInterpret: ERROR on return from get param=%d, status=%d\r\n", Param.paramType, res));
                *cmdObj->param3 = 0;
                break;
            }
            /* WEPStatus */
            Param.paramType = RSN_ENCRYPTION_STATUS_PARAM;
            Param.paramLength = sizeof(ECipherSuite);
            res = cmdDispatch_GetParam (pCmdInterpret->hCmdDispatch, &Param);
            if (res == TI_OK)
            {   
               pStats->WEPStatus = (ECipherSuite)Param.content.rsnEncryptionStatus;
            }
            else
            {
                PRINTF(DBG_NDIS_OIDS_ERROR, ("cmdInterpret: ERROR on return from get param=%d, status=%d\r\n", Param.paramType, res));
                *cmdObj->param3 = 0;
                break;
            }
            /* dot11State */
            Param.paramType = SME_CONNECTION_STATUS_PARAM;
            Param.paramLength = sizeof(TIWLN_DOT11_STATUS);
            res = cmdDispatch_GetParam (pCmdInterpret->hCmdDispatch, &Param);
            if (res == TI_OK)
            {   
               pStats->dot11State = (TIWLN_DOT11_STATUS)Param.content.smeSmConnectionStatus;
            }
            else
            {
                PRINTF(DBG_NDIS_OIDS_ERROR, ("cmdInterpret: ERROR on return from get param=%d, status=%d\r\n", Param.paramType, res));
                *cmdObj->param3 = 0;
                break;
            }
            /* dot11CurrentTxRate */
            pStats->dot11CurrentTxRate = pAdapter->LinkSpeed/5000;
            /* bShortPreambleUsed */
            Param.paramType = SITE_MGR_CURRENT_PREAMBLE_TYPE_PARAM;
            Param.paramLength = sizeof(EPreamble);
            res = cmdDispatch_GetParam (pCmdInterpret->hCmdDispatch, &Param);
            if (res == TI_OK)
            {   
               pStats->bShortPreambleUsed = (TI_BOOLEAN)Param.content.siteMgrCurrentPreambleType;
            }
            else
            {
                PRINTF(DBG_NDIS_OIDS_ERROR, ("cmdInterpret: ERROR on return from get param=%d, status=%d\r\n", Param.paramType, res));
                *cmdObj->param3 = 0;
                break;
            }
            /* targetAP */
            Param.paramType = SITE_MGR_GET_SELECTED_BSSID_INFO;
            Param.paramLength = sizeof(OS_802_11_BSSID);
            Param.content.pSiteMgrPrimarySiteDesc = &pStats->targetAP;
            res = cmdDispatch_GetParam (pCmdInterpret->hCmdDispatch, &Param);
            if (res != TI_OK)
            {
                PRINTF(DBG_NDIS_OIDS_ERROR, ("cmdInterpret: ERROR on return from get param=%d, status=%d\r\n", Param.paramType, res));
                *cmdObj->param3 = 0;
                break;
            }
		    pStats->RxLevel = pStats->targetAP.Rssi;
            /* TxAntenna */
            /* Not Supported ! */

            /* RxAntenna */
            /* Not Supported ! */

            #ifdef CCX_MODULE_INCLUDED	 
                /* dwSecuritySuit */
                Param.paramType = RSN_CCX_NETWORK_EAP;
                Param.paramLength = sizeof(OS_CCX_NETWORK_EAP);
                res = cmdDispatch_GetParam (pCmdInterpret->hCmdDispatch, &Param);
                if (res == TI_OK)
                {   
                   pStats->dwSecuritySuit = (OS_CCX_NETWORK_EAP)Param.content.networkEap;
                }
                else
                {
                    PRINTF(DBG_NDIS_OIDS_ERROR, ("cmdInterpret: ERROR on return from get param=%d, status=%d\r\n", Param.paramType, res));
                    *cmdObj->param3 = 0;
                    break;
                }    
                if ((pStats->dwSecuritySuit==OS_CCX_NETWORK_EAP_ON) && (pStats->WEPStatus==os802_11WEPEnabled))
                {
                    pStats->dwSecuritySuit = TIWLN_STAT_SECURITY_RESERVE_1;
                } 
                else
            #else
                {
                    pStats->dwSecuritySuit = 0;
                }
            #endif
            /* dwSecurityState */
            Param.paramType = RSN_SECURITY_STATE_PARAM;
            Param.paramLength = sizeof(TI_UINT32);
            res = cmdDispatch_GetParam (pCmdInterpret->hCmdDispatch, &Param);
            if (res == TI_OK)
            {   
               pStats->dwSecurityState = (TI_UINT32)Param.content.rsnAuthState;
            }
            else
            {
                PRINTF(DBG_NDIS_OIDS_ERROR, ("cmdInterpret: ERROR on return from get param=%d, status=%d\r\n", Param.paramType, res));
                *cmdObj->param3 = 0;
                break;
            }
            /* dwSecurityAuthStatus */
            pStats->dwSecurityAuthStatus = 0;
            /* dwFeatureSuit */
            pStats->dwFeatureSuit = 0;
            /* tiCounters */
            Param.paramType = SITE_MGR_TI_WLAN_COUNTERS_PARAM;
            Param.paramLength = sizeof(TIWLN_COUNTERS);
            res = cmdDispatch_GetParam (pCmdInterpret->hCmdDispatch, &Param);
            if (res == TI_OK)
            {   
                os_memoryCopy(pCmdInterpret->hOs, &pStats->tiCounters, &Param.content.siteMgrTiWlanCounters, sizeof(TIWLN_COUNTERS));
            }
            else
            {
                PRINTF(DBG_NDIS_OIDS_ERROR, ("cmdInterpret: ERROR on return from get param=%d, status=%d\r\n", Param.paramType, res));
                *cmdObj->param3 = 0;
                break;
            }
            Param.paramType = MLME_BEACON_RECV;
            Param.paramLength = sizeof(TI_UINT32);
            res = cmdDispatch_GetParam (pCmdInterpret->hCmdDispatch, &Param);
            if (res == TI_OK)
            {   
                pStats->tiCounters.BeaconsRecv = (TI_UINT32)Param.content.siteMgrTiWlanCounters.BeaconsRecv;
            }
            else
            {
                PRINTF(DBG_NDIS_OIDS_ERROR, ("cmdInterpret: ERROR on return from get param=%d, status=%d\r\n", Param.paramType, res));
                *cmdObj->param3 = 0;
                break;
            }
        }
        else
        {
            PRINTF(DBG_NDIS_OIDS_VERY_LOUD, ("Buffer for parameter 0x%X is bigger then Param size\r\n", Param.paramType));
            *cmdObj->param4 = sizeof(TIWLN_STATISTICS);
        }
        break;
        /* end of if (TIWLN_STATISTICS...) */

    case OID_802_11_ADD_WEP:
        PRINT(DBG_NDIS_OIDS_LOUD, "TIWL: (SET) OID_802_11_ADD_WEP\r\n");
        res = AddWepKey(hCmdInterpret, cmdObj->buffer1);
        if (res != TI_OK)
        {
            *cmdObj->param3 = 0;
        }
        else
        {
            *cmdObj->param3 = cmdObj->buffer1_len;
        }
        break;

    case OID_802_11_REMOVE_WEP:
        {
            UINT32          keyIndex = *((TI_UINT32*)cmdObj->buffer1);
            OS_802_11_KEY  key;
            
            PRINT(DBG_NDIS_OIDS_LOUD, "TIWL: (SET) OID_802_11_REMOVE_WEP\r\n");
            /* Convert the Key index to match OS_802_11_KEY index */
            os_memoryZero(pCmdInterpret->hOs, &key, sizeof(OS_802_11_KEY));           
            key.KeyIndex = keyIndex;           
            os_memoryCopy(pCmdInterpret->hOs, &Param.content.rsnOsKey, &key, sizeof(OS_802_11_KEY));
            Param.paramType = RSN_REMOVE_KEY_PARAM;
            Param.paramLength = sizeof(OS_802_11_KEY);
            *cmdObj->param3 = cmdObj->buffer1_len;
            res = cmdDispatch_SetParam (pCmdInterpret->hCmdDispatch, &Param);
        }
        break;

    case OID_802_11_ADD_KEY:
        {
            OS_802_11_KEY* pKey = (OS_802_11_KEY*)cmdObj->buffer1;

    		PRINT(DBG_NDIS_OIDS_LOUD, "TIWL: (SET) OID_802_11_ADD_KEY\r\n");    	
            os_memoryCopy(pCmdInterpret->hOs, &Param.content.rsnOsKey, pKey, sizeof(OS_802_11_KEY));
            Param.paramType = RSN_ADD_KEY_PARAM;
            Param.paramLength = sizeof(OS_802_11_KEY);
            res = cmdDispatch_SetParam (pCmdInterpret->hCmdDispatch, &Param);
            if (res == TI_OK)
            {
                *cmdObj->param3 = cmdObj->buffer1_len;
            }
            else
            {
                *cmdObj->param3 = 0;
                ndisStatus = NDIS_STATUS_INVALID_DATA;
            }
   	    }
		break;

    case OID_802_11_REMOVE_KEY:
        {
            OS_802_11_REMOVE_KEY    *pRemoveKey = (OS_802_11_REMOVE_KEY*)cmdObj->buffer1;
            OS_802_11_KEY           key;

            PRINT(DBG_NDIS_OIDS_LOUD, "TIWL: (SET) OID_802_11_REMOVE_KEY\r\n");
        	key.KeyIndex = pRemoveKey->KeyIndex;
            os_memoryCopy(pCmdInterpret->hOs, key.BSSID, (void *)pRemoveKey->BSSID, sizeof(key.BSSID));
            os_memoryCopy(pCmdInterpret->hOs, &Param.content.rsnOsKey, &key, sizeof(OS_802_11_KEY));
            Param.paramType = RSN_REMOVE_KEY_PARAM;
            Param.paramLength = sizeof(OS_802_11_KEY);
            *cmdObj->param3 = cmdObj->buffer1_len;
            res = cmdDispatch_SetParam (pCmdInterpret->hCmdDispatch, &Param);
        }
		break;

    case OID_802_11_BSSID_LIST:
    {
		TI_UINT32 neededSize=0;

        /* First get the amount of memory required to hold the entire BSSID list by setting the length to 0 */

		Param.paramType   = SCAN_CNCN_BSSID_LIST_SIZE_PARAM;
        Param.paramLength = 0;
        res = cmdDispatch_GetParam (pCmdInterpret->hCmdDispatch, &Param );
		neededSize = Param.content.uBssidListSize;

		/* 4 means that no sites were found */
		if (4 == neededSize ) 
		{
			*cmdObj->param4 = 0;
			break;
		}

		if (neededSize > cmdObj->buffer1_len)
		{
			*cmdObj->param4 = neededSize;
			break;
		}

        /* And retrieve the list */
		*cmdObj->param3 	= neededSize;
		Param.paramType   	= SCAN_CNCN_BSSID_LIST_PARAM;
		Param.paramLength 	= neededSize;
		Param.content.pBssidList = cmdObj->buffer1;
        res = cmdDispatch_GetParam (pCmdInterpret->hCmdDispatch, &Param );
        
        PRINTF(DBG_NDIS_OIDS_LOUD, ("TIWL: OID_802_11_BSSID_LIST 0x%x, len %d\r\n", *((UINT32 *)cmdObj->buffer1), Param.paramLength));
        break;
    }

    case OID_802_11_AUTHENTICATION_MODE:
        Param.paramType = RSN_EXT_AUTHENTICATION_MODE;
        Param.paramLength = sizeof(EExternalAuthMode);
        if (cmdObj->flags == OS_OID_SET_FLAG)
        {
                Param.content.rsnExtAuthneticationMode = *((EExternalAuthMode*)cmdObj->buffer1);
                res = cmdDispatch_SetParam (pCmdInterpret->hCmdDispatch, &Param);
                if (res == TI_OK) 
                {
                    PRINTF(DBG_NDIS_OIDS_VERY_LOUD, ("TIWL: (SET)OID_802_11_AUTHENTICATION_MODE 0x%x, len %d\r\n", *(UINT32*)cmdObj->buffer1, cmdObj->buffer1_len));
                    *cmdObj->param3 = cmdObj->buffer1_len;
                }
        }
        else
            {   
                if (cmdObj->buffer1_len < sizeof(EExternalAuthMode))
                {
                    PRINTF(DBG_NDIS_OIDS_VERY_LOUD, ("Buffer for parameter 0x%X is bigger then Param size\r\n", Param.paramType));
                    *cmdObj->param4 = sizeof(EExternalAuthMode);
                }
                else
                {
                res = cmdDispatch_GetParam (pCmdInterpret->hCmdDispatch, &Param);   
                if (res == TI_OK)
                {   
                    *(TI_INT32*)cmdObj->buffer1 = (EExternalAuthMode)Param.content.rsnExtAuthneticationMode;
                    *cmdObj->param3 = sizeof(EExternalAuthMode);
                    PRINTF(DBG_NDIS_OIDS_LOUD, ("TIWL: OID_802_11_AUTHENTICATION_MODE %d\r\n", *(TI_INT32 *)cmdObj->buffer1));
                }
            else
            {
                    PRINTF(DBG_NDIS_OIDS_ERROR, ("cmdInterpret: ERROR on return from get param=%d, status=%d\r\n", Param.paramType, res));
                *cmdObj->param3 = 0;
                }
            }
        }
        break;
	
    case OID_802_11_PRIVACY_FILTER:
        if (cmdObj->flags == OS_OID_SET_FLAG)
        {
            Param.paramType = RSN_ENCRYPTION_STATUS_PARAM;
            Param.paramLength = sizeof(ECipherSuite);
            res = cmdDispatch_GetParam (pCmdInterpret->hCmdDispatch, &Param);    
            if ((Param.content.rsnExtAuthneticationMode && (*((TI_UINT32*)cmdObj->buffer1) == os802_11PrivFilter8021xWEP)) || (res != TI_OK))
            {   
                *cmdObj->param3 = 0;
                break;
            }        
        	if ((!Param.content.rsnExtAuthneticationMode) && (res == TI_OK))
        	{
        		Param.content.rxDataExcludeUnencrypted = TI_FALSE;
                Param.paramType = RX_DATA_EXCLUDE_UNENCRYPTED_PARAM;
                Param.paramLength = sizeof(TI_BOOL);
                res = cmdDispatch_GetParam (pCmdInterpret->hCmdDispatch, &Param);    
        	}
        }
        else
        {
            Param.paramType = RSN_ENCRYPTION_STATUS_PARAM;
            Param.paramLength = sizeof(ECipherSuite);
            res = cmdDispatch_GetParam (pCmdInterpret->hCmdDispatch, &Param);    
            if (res == TI_OK)
            {   
                if (cmdObj->buffer1_len < sizeof(TI_UINT32))
                {
                    PRINTF(DBG_NDIS_OIDS_VERY_LOUD, ("Buffer for parameter 0x%X is bigger then Param size\r\n", Param.paramType));
                    *cmdObj->param4 = sizeof(TI_UINT32);
                }
                else
                {
                    if (Param.content.rsnExtAuthneticationMode)
                	{
                		*(TI_INT32*)cmdObj->buffer1 = os802_11PrivFilterAcceptAll;
                	} 
                    else
                	{
                		*(TI_INT32*)cmdObj->buffer1 = os802_11PrivFilter8021xWEP;
                	}
                    PRINTF(DBG_NDIS_OIDS_LOUD, ("TIWL: OID_802_11_PRIVACY_FILTER %d\r\n", *(TI_INT32 *)cmdObj->buffer1));
                }
            }
            else
            {
                PRINTF(DBG_NDIS_OIDS_ERROR, ("cmdInterpret: ERROR on return from get param=%d, status=%d\r\n", Param.paramType, res));
                *cmdObj->param3 = 0;
            }
        }
        break;

    case OID_802_11_BSSID_LIST_SCAN:
    {
		TScanParams scanParams;
		Param.content.pScanParams = &scanParams;

		Param.paramType = SCAN_CNCN_BSSID_LIST_SCAN_PARAM; 

		/* Defualt params - scan active for All ssid */
		Param.content.pScanParams->desiredSsid.len = 0;
		Param.content.pScanParams->scanType = SCAN_TYPE_TRIGGERED_ACTIVE;
		
        res = cmdDispatch_SetParam (pCmdInterpret->hCmdDispatch, &Param );
            CHECK_PENDING_RESULT(res,Param)
        }
		break;

    case OID_802_11_RELOAD_DEFAULTS:
        PRINT(DBG_NDIS_OIDS_LOUD, "TIWL: (SET) OID_802_11_RELOAD_DEFAULTS\r\n");
        if (cmdObj->buffer1_len < sizeof(NDIS_802_11_RELOAD_DEFAULTS)) 
        {
            *cmdObj->param4 = sizeof(NDIS_802_11_RELOAD_DEFAULTS);
            break;
        }       
        Param.paramType = RSN_EXT_AUTHENTICATION_MODE;
        Param.paramLength = sizeof(EExternalAuthMode);
        res = cmdDispatch_GetParam (pCmdInterpret->hCmdDispatch, &Param);
        if (res == TI_OK)
        {   
            if (Param.content.rsnExtAuthneticationMode == Ndis802_11AuthModeShared) 
            {
                if (*(NDIS_802_11_RELOAD_DEFAULTS*)cmdObj->buffer1 == Ndis802_11ReloadWEPKeys) 
                {
                    TI_INT32 i;
                    for ( i = 0; i < DOT11_MAX_DEFAULT_WEP_KEYS; i++ ) 
                    {
                        AddWepKey(hCmdInterpret, (TI_UINT8*)&pAdapter->DefaultWepKeys[i]);
                    }
                    *cmdObj->param3 = cmdObj->buffer1_len;
                }
                else
                {
                    ndisStatus = NDIS_STATUS_NOT_SUPPORTED;
                }
            }
        }
        else
        {
            PRINTF(DBG_NDIS_OIDS_ERROR, ("cmdInterpret: ERROR on return from get param=%d, status=%d\r\n", Param.paramType, res));
            *cmdObj->param3 = 0;
        }
        break;
	
   case OID_802_11_ENCRYPTION_STATUS:
        Param.paramType = RSN_ENCRYPTION_STATUS_PARAM;
        Param.paramLength = sizeof(ECipherSuite);
        if (cmdObj->flags == OS_OID_SET_FLAG)
        {
            PRINT(DBG_NDIS_OIDS_LOUD, "TIWL: (SET) OID_802_11_WEP_STATUS\r\n");
    
    		/* Convert from cipher suite to encryption status */
    		/* support 13 mode */
    		switch (*((NDIS_802_11_WEP_STATUS*)cmdObj->buffer1))
    		{
    		/*
    		 * NDIS_802_11EncryptionDisabled
    		 * 1. Open system: none
    		 * 2. Shared Key: none
    		 */
    		case os802_11WEPDisabled:
    			Param.content.rsnEncryptionStatus = TWD_CIPHER_NONE;
    			break;
    		/*
    		 * Ndis802_11Encryption1Enabled
    		 * 1. Open System: WEP
    		 * 2. Shared Key: WEP
    		 */
    		case os802_11WEPEnabled: 
    			Param.content.rsnEncryptionStatus = TWD_CIPHER_WEP;
    			break; 
    		/*
    		 * Ndis802_11Encryption2Enabled
    		 * 1. WPA: TKIP
    		 * 2. WPA-PSK: TKIP
    		 * 3. WPA-None: TKIP
    		 * 4. WPA2: TKIP
    		 * 5. WPA-PSK2: TKIP
    		 */
    		case os802_11Encryption2Enabled:
    			Param.content.rsnEncryptionStatus = TWD_CIPHER_TKIP;
    			break;
    		/*
    		 * Ndis802_11Encryption3Enabled
    		 * 1. WPA: AES
    		 * 2. WPA-PSK: AES
    		 * 3. WPA-None: AES
    		 * 4. WPA2: AES
    		 * 5. WPA-PSK2: AES
    		 */
    		case os802_11Encryption3Enabled:
    			Param.content.rsnEncryptionStatus = TWD_CIPHER_AES_CCMP;
    			break;
    	
    		default: 
    			res = TI_NOK;
    			break;
    		}
            if (res == TI_OK)
            {
                *cmdObj->param3 = sizeof(ECipherSuite);
                res = cmdDispatch_SetParam (pCmdInterpret->hCmdDispatch, &Param);    
            }
        }
        else
        {
            if (cmdObj->buffer1_len < sizeof(TI_UINT32))
            {
                PRINTF(DBG_NDIS_OIDS_ERROR, ("Buffer for parameter 0x%X is bigger then Param size\r\n", Param.paramType));
                *cmdObj->param4 = sizeof(TI_UINT32);
            }
            else
            {
                res = cmdDispatch_GetParam (pCmdInterpret->hCmdDispatch, &Param);   
                if (res == TI_OK)
                {
                    /* Convert from cipher suite to encryption status */
                    switch ((ECipherSuite)Param.content.rsnExtAuthneticationMode)
                    {
                    case TWD_CIPHER_NONE: 
                        *(OS_802_11_WEP_STATUS*)cmdObj->buffer1 = os802_11WEPDisabled;
                    break;
    
                    case TWD_CIPHER_WEP: 
                    case TWD_CIPHER_WEP104: 
                        *(OS_802_11_WEP_STATUS*)cmdObj->buffer1 = os802_11WEPEnabled;  
                        break;   
    
                    case TWD_CIPHER_TKIP:
                        Param.paramType = RSN_SECURITY_STATE_PARAM;
                        Param.paramLength = sizeof(TI_UINT32);
                        res = cmdDispatch_GetParam (pCmdInterpret->hCmdDispatch, &Param);
                        if (res == TI_OK)
                        { 
                            if (Param.content.rsnAuthState == eSecurityStateStarting)
                            {
                                *(OS_802_11_WEP_STATUS*)cmdObj->buffer1 = os802_11Encryption2KeyAbsent;
                            } 
                            else 
                            {
                                *(OS_802_11_WEP_STATUS*)cmdObj->buffer1 = os802_11Encryption2Enabled;
                            }
                        }
                        else
                        {
                            PRINTF(DBG_NDIS_OIDS_ERROR, ("cmdInterpret: ERROR on return from get param=%d, status=%d\r\n", Param.paramType, res));
                            *cmdObj->param3 = 0;
                        }
                        break; 
    
                    case TWD_CIPHER_AES_WRAP:             
                    case TWD_CIPHER_AES_CCMP: 
                        Param.paramType = RSN_SECURITY_STATE_PARAM;
                        Param.paramLength = sizeof(TI_UINT32);
                        res = cmdDispatch_GetParam (pCmdInterpret->hCmdDispatch, &Param);   
                        if (res == TI_OK)
                        { 
                            if (Param.content.rsnAuthState == eSecurityStateStarting)
                            {
                                *(OS_802_11_WEP_STATUS*)cmdObj->buffer1 = os802_11Encryption3KeyAbsent;
                            } 
                            else 
                            {
                                *(OS_802_11_WEP_STATUS*)cmdObj->buffer1 = os802_11Encryption3Enabled;
                            }
                        }
                        else
                        {
                            PRINTF(DBG_NDIS_OIDS_ERROR, ("cmdInterpret: ERROR on return from get param=%d, status=%d\r\n", Param.paramType, res));
                            *cmdObj->param3 = 0;
                        }
                        break; 
    
                    case TWD_CIPHER_CKIP:     
                    case TWD_CIPHER_UNKNOWN:
                    default: 
                        *(OS_802_11_WEP_STATUS*)cmdObj->buffer1 = os802_11WEPNotSupported;
                        break;
                    }
                    PRINTF(DBG_NDIS_OIDS_LOUD, ("TIWL: OID_802_11_ENCRYPTION_STATUS %d\r\n", *(TI_INT32 *)cmdObj->buffer1));
                }
            else
            {
                *cmdObj->param3 = 0;
                    PRINTF(DBG_NDIS_OIDS_ERROR, ("cmdInterpret: ERROR on return from get param=%d, status=%d\r\n", Param.paramType, res));
                }
            }
        }
		break;

    case OID_802_11_ASSOCIATION_INFORMATION:
        *cmdObj->param4 = cmdObj->buffer1_len;
        res = cmdInterpret_AssociationInfoGet(pCmdInterpret, cmdObj->buffer1, cmdObj->param4);
        if (res == TI_NOK)
        {
            if (*cmdObj->param4 > cmdObj->buffer1_len) /* need more space*/
            {
                PRINTF(DBG_NDIS_OIDS_WARNING, ("Buffer for parameter 0x%X is bigger then Param size\r\n", Param.paramType));
            }
            else /* error */
            {
                *cmdObj->param4 = 0;
                *cmdObj->param3 = 0;
            }
        }
        else /* OK */
        {
            *cmdObj->param3 = *cmdObj->param4;
            *cmdObj->param4 = 0;
        }
        break;

    case OID_802_11_DISASSOCIATE:
        {
            OS_802_11_SSID FakeSsid;
            UINT32 uLoopIndex;
            PRINT(DBG_NDIS_OIDS_LOUD, "TIWL: (SET) OID_802_11_DISASSOCIATE\r\n");
        	/*
        	 * Clean up desired SSID value
        	*/
        	for (uLoopIndex = 0; uLoopIndex < MAX_SSID_LEN; uLoopIndex++)
        		FakeSsid.Ssid[uLoopIndex] = (uLoopIndex+1);       
        	FakeSsid.SsidLength = MAX_SSID_LEN;
            os_memoryCopy(pCmdInterpret->hOs, &Param.content.siteMgrDesiredSSID.str, FakeSsid.Ssid, MAX_SSID_LEN);
            Param.content.siteMgrDesiredSSID.len = MAX_SSID_LEN;
            Param.paramType = SITE_MGR_DESIRED_SSID_PARAM;
            Param.paramLength = sizeof (TSsid);
            *cmdObj->param3 = MAX_SSID_LEN;
            res = cmdDispatch_SetParam (pCmdInterpret->hCmdDispatch, &Param);
            
            /* also set it to the SME */
            Param.paramType = SME_DESIRED_SSID_ACT_PARAM;
            res = cmdDispatch_SetParam (pCmdInterpret->hCmdDispatch, &Param);
        }
        break;

    case OID_PNP_CAPABILITIES:
        {
            PNDIS_PNP_CAPABILITIES pNdisPnpCapabilities;
            pNdisPnpCapabilities = (PNDIS_PNP_CAPABILITIES)cmdObj->buffer1;
            if (cmdObj->buffer1_len < sizeof(NDIS_PNP_CAPABILITIES))
            {
                *cmdObj->param4 = sizeof(NDIS_PNP_CAPABILITIES);
                break;
            }

            pNdisPnpCapabilities->Flags = NDIS_DEVICE_WAKE_UP_ENABLE;
            pNdisPnpCapabilities->WakeUpCapabilities.MinMagicPacketWakeUp = NdisDeviceStateUnspecified;
            pNdisPnpCapabilities->WakeUpCapabilities.MinPatternWakeUp     = NdisDeviceStateD2;
            pNdisPnpCapabilities->WakeUpCapabilities.MinLinkChangeWakeUp  = NdisDeviceStateUnspecified;
            PRINTF(DBG_NDIS_OIDS_LOUD, ("TIWL: OID_PNP_CAPABILITIES \r\n"));
        }
        *cmdObj->param3 = sizeof(NDIS_PNP_CAPABILITIES);
        break;

	case OID_PNP_QUERY_POWER:
        {
			PNDIS_DEVICE_POWER_STATE NdisDeviceState = (PNDIS_DEVICE_POWER_STATE)cmdObj->buffer1;
        WLAN_OS_REPORT(("TIWL: (GET)OID_PNP_QUERY_POWER %d\r\n", *((UINT32 *)cmdObj->buffer1))); 
        if ( *NdisDeviceState == NdisDeviceStateD3  )
        {
            *NdisDeviceState = pAdapter->powerModeState;
        }
        else
        {
            ndisStatus = NDIS_STATUS_NOT_SUPPORTED;
        }
        break;
    }

    case OID_802_11_POWER_MODE:
        /*
         * Remap POWER_MODE_AUTO
         * Ndis802_11PowerModeCAM
         * Specifies continuous access mode (CAM). When the power mode is set to CAM, the device is always on.
         */

        /*
         * map POWER_MODE_LONG_DOZE
         * Ndis802_11PowerModeMAX_PSP
         * Specifies maximum (MAX) power saving. A power mode of MAX results in the greatest power savings for the 802.11 NIC radio.
         */

        /*
         * map POWER_MODE_SHORT_DOZE
         * Ndis802_11PowerModeFast_PSP
         * Specifies fast power-saving mode. This power mode provides the best combination of network performance and power usage.
         */

        /*
         * map POWER_MODE_ACTIVE
         * Ndis802_11PowerModeMax
         * Specifies the upper bound.
         */
        Param.paramType = POWER_MGR_POWER_MODE;
        Param.paramLength = sizeof(PowerMgr_PowerMode_e);
        if (cmdObj->flags == OS_OID_SET_FLAG)
        {
            switch (*((UINT32*)cmdObj->buffer1)) {
                case Ndis802_11PowerModeMax:
                /**< In this mode there is no power save, the host interface & the radio
                * is always active. The TNET is constantly awake. This mode is used,
                * for example, when the device is powered from an AC power source,
                * and provides maximum throughput and minimal latency.*/
                PRINTF(DBG_NDIS_OIDS_LOUD, ("SET POWER_MODE_ACTIVE (Ndis802_11PowerModeCAM)\r\n"));
                Param.content.powerMngPowerMode.PowerMode = POWER_MODE_ACTIVE;
                break;
            case Ndis802_11PowerModeFast_PSP:
                /**< In this mode the system is going to ELP state and awakes (by the
                 * FW) every beacon. The F/W wakes up the host on every Beacon passes
                 * the Beacon to the driver and returns to ELP Doze as soon as possible.*/
                PRINTF(DBG_NDIS_OIDS_LOUD, ("SET POWER_MODE_SHORT_DOZE (NdisDeviceStateD1)\r\n"));
                Param.content.powerMngPowerMode.PowerMode = POWER_MODE_SHORT_DOZE;
                break;
            case Ndis802_11PowerModeMAX_PSP:
                /**< In this mode the system is going to ELP state and awakes (by the
                * FW) every DTIM or listen interval. This mode consumes low power,
                * while still waking-up for Beacons once in a while. The system spends
                * a lot of time in ELP-Doze, and the F/W rarely wakes up the host. */
                PRINTF(DBG_NDIS_OIDS_LOUD, ("SET POWER_MODE_LONG_DOZE (Ndis802_11PowerModeFast_PSP)\r\n"));
                Param.content.powerMngPowerMode.PowerMode = POWER_MODE_LONG_DOZE;
                break;
            case Ndis802_11PowerModeCAM:
                Param.content.powerMngPowerMode.PowerMode = POWER_MODE_AUTO;
                break;
            default:
                PRINTF(DBG_NDIS_OIDS_LOUD, ("POWER_MODE unknown\r\n"));
                Param.content.powerMngPowerMode.PowerMode = POWER_MODE_AUTO;
                break;
            }
            *cmdObj->param3 = sizeof(PowerMgr_PowerMode_e);
            res = cmdDispatch_SetParam (pCmdInterpret->hCmdDispatch, &Param);
        }
        else
        {           
            res = cmdDispatch_GetParam (pCmdInterpret->hCmdDispatch, &Param);   
            if (res == TI_OK)
            {
                switch ((PowerMgr_PowerMode_e)Param.content.PowerMode){
                case POWER_MODE_AUTO:
                   /*
                    * In this mode the power manager module is toggle states
                    * (ACTIVE, SHORT_DOZE and LONG_DOZE) by its own inner algorithm. 
                    */
                    PRINTF(DBG_NDIS_OIDS_LOUD, ("POWER_MODE_AUTO\r\n"));
                    *((UINT32 *)cmdObj->buffer1) = Ndis802_11PowerModeCAM;
                    break;
                case POWER_MODE_ACTIVE:
                   /*
                    * In this mode there is no power save, the host interface & the radio
                    * is always active. The TNET is constantly awake. This mode is used,
                    * for example, when the device is powered from an AC power source,
                    * and provides maximum throughput and minimal latency.
                    */
                    PRINTF(DBG_NDIS_OIDS_LOUD, ("POWER_MODE_ACTIVE\r\n"));
                    *((UINT32 *)cmdObj->buffer1) = Ndis802_11PowerModeMax;
                    break;
                case POWER_MODE_SHORT_DOZE:
                    /*
                     * In this mode the system is going to ELP state and awakes (by the
                     * FW) every beacon. The F/W wakes up the host on every Beacon passes
                     * the Beacon to the driver and returns to ELP Doze as soon as possible.
                     */
                    PRINTF(DBG_NDIS_OIDS_LOUD, ("POWER_MODE_SHORT_DOZE\r\n"));
                    *((UINT32 *)cmdObj->buffer1) = Ndis802_11PowerModeFast_PSP;
                    break;
                case POWER_MODE_LONG_DOZE:
                    /* 
                    * In this mode the system is going to ELP state and awakes (by the
                    * FW) every DTIM or listen interval. This mode consumes low power,
                    * while still waking-up for Beacons once in a while. The system spends
                    * a lot of time in ELP-Doze, and the F/W rarely wakes up the host. 
                    */
                    PRINTF(DBG_NDIS_OIDS_LOUD, ("POWER_MODE_LONG_DOZE\r\n"));
                    *((UINT32 *)cmdObj->buffer1) = Ndis802_11PowerModeMAX_PSP;
                    break;
                default:
                    PRINTF(DBG_NDIS_OIDS_LOUD, ("POWER_MODE unknown\r\n"));
                    *((UINT32 *)cmdObj->buffer1) = Ndis802_11PowerModeCAM;
                    break;
                }
            }
            else
            {
                PRINTF(DBG_NDIS_OIDS_ERROR, ("cmdInterpret: ERROR on return from get param=%d, status=%d\r\n", Param.paramType, res));
                *cmdObj->param3 = 0;
            }
        }
        PRINTF(DBG_NDIS_OIDS_LOUD, ("TIWL: OID_802_11_POWER_MODE %d\r\n", *((UINT32 *)cmdObj->buffer1)));
        break;

    case OID_PNP_ADD_WAKE_UP_PATTERN:
    case OID_PNP_REMOVE_WAKE_UP_PATTERN:

        PRINT(DBG_NDIS_OIDS_LOUD, (cmdObj->cmd == OID_PNP_ADD_WAKE_UP_PATTERN ? 
                "TIWL: (SET) OID_PNP_ADD_WAKE_UP_PATTERN\r\n" : "TIWL: (SET) OID_PNP_REMOVE_WAKE_UP_PATTERN\r\n"));
        {
            TI_UINT8 maskIter;
            TI_UINT8 patternIter;
            TRxDataFilterRequest outRequest;
            PNDIS_PM_PACKET_PATTERN inRequest = (PNDIS_PM_PACKET_PATTERN)cmdObj->buffer1;
            /* Mask immediately follows the packet pattern struct in the info buffer */
            TI_UINT8 *inMask = ((TI_UINT8*)cmdObj->buffer1) + sizeof(*inRequest);
            /* Pattern is located PatternOffset bytes after the beginning of the info buffer */
            TI_UINT8 * inPattern = ((TI_UINT8 *)cmdObj->buffer1) + inRequest->PatternOffset;
            if (inRequest->PatternOffset + inRequest->PatternSize > cmdObj->buffer1_len)
            {
                ndisStatus = NDIS_STATUS_INVALID_LENGTH;
                break;
            }
            /* No offsetting is supported with the NDIS interface */
            outRequest.offset = 0;
            outRequest.maskLength = (TI_UINT8)(inRequest->MaskSize);
            os_memoryCopy(pCmdInterpret->hOs, outRequest.mask, inMask, outRequest.maskLength);
            /* the pattern isn't received in compact form, i.e., there are redundant /*
            /* characters for each unset bit in the mask */
            patternIter = maskIter = 0;
            for (; ((maskIter < outRequest.maskLength * BIT_TO_BYTE_FACTOR) &&
                   (maskIter < inRequest->PatternSize)); ++maskIter)
            {
                /* which byte in the mask and which bit in the byte we're at */
                TI_INT32 bit = maskIter % BIT_TO_BYTE_FACTOR;
                TI_INT32 byte = maskIter / BIT_TO_BYTE_FACTOR;

                /* is the bit in the mask set */
                TI_BOOL isSet = ((outRequest.mask[byte] & (1 << bit)) == (1 << bit));

                if (isSet)
                {
                    outRequest.pattern[patternIter++] = inPattern[maskIter];
                }
            }
            outRequest.patternLength = patternIter;
            if (outRequest.maskLength > MAX_DATA_FILTER_MASK_LENGTH || 
                outRequest.patternLength > MAX_DATA_FILTER_PATTERN_LENGTH)
            {
                ndisStatus = NDIS_STATUS_INVALID_DATA;
                break;
            }           
            if (cmdObj->cmd == OID_PNP_ADD_WAKE_UP_PATTERN)
            {
                Param.paramType = RX_DATA_ADD_RX_DATA_FILTER;
                Param.paramLength = sizeof(TRxDataFilterRequest);
                os_memoryCopy(pCmdInterpret->hOs, &Param.content.rxDataFilterRequest, &outRequest, sizeof(TRxDataFilterRequest));
                
            }
            else
            {
                Param.paramType = RX_DATA_REMOVE_RX_DATA_FILTER;
                Param.paramLength = sizeof(TRxDataFilterRequest);
                os_memoryCopy(pCmdInterpret->hOs, &Param.content.rxDataFilterRequest, &outRequest, sizeof(TRxDataFilterRequest));
            }
            res = cmdDispatch_SetParam (pCmdInterpret->hCmdDispatch, &Param);
            switch (res)
            {
                case RX_NO_AVAILABLE_FILTERS:
                    ndisStatus = NDIS_STATUS_RESOURCES;
                    break;

                case RX_FILTER_DOES_NOT_EXIST:
                case RX_FILTER_ALREADY_EXISTS:
                case TI_OK:
                    *cmdObj->param3 = cmdObj->buffer1_len;
                    break;

                case TI_NOK:
                default:
                    *cmdObj->param3 = 0;
                    break;
            }
        }
        break;

    case OID_PNP_ENABLE_WAKE_UP:
        {
            TI_UINT8 uFlags = *((TI_UINT8*)cmdObj->buffer1);
            TI_BOOL bEnabled = ((uFlags & NDIS_PNP_WAKE_UP_PATTERN_MATCH) == NDIS_PNP_WAKE_UP_PATTERN_MATCH);

            PRINT(DBG_NDIS_OIDS_LOUD, "TIWL: (SET) OID_PNP_ENABLE_WAKE_UP\r\n");    
            Param.paramType = RX_DATA_ENABLE_DISABLE_RX_DATA_FILTERS;
            Param.paramLength = sizeof(TI_BOOL);
            *cmdObj->param3 = sizeof(TI_BOOL);
            Param.content.rxDataFilterEnableDisable = bEnabled;
            res = cmdDispatch_SetParam (pCmdInterpret->hCmdDispatch, &Param);
        }
        break;

    case OID_802_11_TEST:
        {
            NDIS_802_11_TEST *pTest = (NDIS_802_11_TEST*)cmdObj->buffer1;
            NDIS_802_11_RSSI rssi;
            TI_UINT32 dwRetSize;

            PRINT(DBG_NDIS_OIDS_LOUD, "TIWL: (SET) OID_802_11_TEST\r\n");   

        	switch (pTest->Type)
        	{
            	case 1:
            		NdisMIndicateStatus(pAdapter->MiniportHandle, 
            							NDIS_STATUS_MEDIA_SPECIFIC_INDICATION, &(pTest->AuthenticationEvent), 
            							pTest->Length - sizeof(pTest->Length) - sizeof(pTest->Type));
            		break;
            	case 2:
            		dwRetSize = sizeof(NDIS_802_11_RSSI);
    
                    Param.paramType = SITE_MGR_GET_SELECTED_BSSID_INFO;
                    Param.paramLength = sizeof(OS_802_11_BSSID);
                    Param.content.pSiteMgrPrimarySiteDesc = &SelectedBSSID;
                    res = cmdDispatch_GetParam (pCmdInterpret->hCmdDispatch, &Param);
                    if (res == TI_OK)
                    {   
                       rssi = Param.content.pSiteMgrPrimarySiteDesc->Rssi;
                       NdisMIndicateStatus(pAdapter->MiniportHandle, OS_STATUS_MEDIA_SPECIFIC_INDICATION, &rssi, sizeof(NDIS_802_11_RSSI));
                    }
            		break;

            	default:
            		res = 0;
        	}
        }
        break;

        /* 
         * Unhandled OIDs.
         * Not sure where these are coming from.
         */

    case OID_GEN_MACHINE_NAME:
        /*
         * From http://www.osronline.com/DDKx/network/22genoid_34z6.htm:
         *  The OID_GEN_MACHINE_NAME OID, which is optional, indicates the system name.
         *  The information buffer passed in this request contains an array of Unicode
         *  characters that represents the system name. The InformationBufferLength
         *  supplied to MiniportSetInformation specifies the length of this array in
         *  bytes, not including a NULL terminator.
         */
        ndisStatus = NDIS_STATUS_SUCCESS;
        *cmdObj->param3 = cmdObj->buffer1_len;
        break;
#if 0 
    case OID_GEN_TRANSPORT_HEADER_OFFSET:
        WLAN_OS_REPORT(("TIWL: OID_GEN_TRANSPORT_HEADER_OFFSET len=%d val=%d ",cmdObj->buffer1_len,*((TI_UINT32*)cmdObj->buffer1)));
        break;


	case OID_802_11_LATENCY_SENSITIVE_MODE:
		Param.paramType = QOS_MNGR_PS_RX_STREAMING;
		Param.paramLength = sizeof(TPsRxStreaming);
		if (cmdObj->flags == OS_OID_SET_FLAG)
		{
			WLAN_OS_REPORT(("TIWL: OID_802_11_LATENCY_SENSITIVE_MODE SET\r\n"));
			Param.content.tPsRxStreaming.uStreamPeriod = ((PNDIS_802_11_LSM_PARAMETERS)cmdObj->buffer1)->PacketLatency;
			Param.content.tPsRxStreaming.bEnabled 	   = ((PNDIS_802_11_LSM_PARAMETERS)cmdObj->buffer1)->Mode;
			Param.content.tPsRxStreaming.uTid = 6; /*currently as Cisco requiered*/
			Param.content.tPsRxStreaming.uTxTimeout = 0;
            *cmdObj->param3 = cmdObj->buffer1_len;
			res = cmdDispatch_SetParam (pCmdInterpret->hCmdDispatch, &Param);
		}
		else
		{
			WLAN_OS_REPORT(("TIWL: OID_802_11_LATENCY_SENSITIVE_MODE GET\r\n"));
			res = cmdDispatch_GetParam (pCmdInterpret->hCmdDispatch, &Param);
			if (res == TI_OK)
			{
				 if (Param.paramLength > cmdObj->buffer1_len)
				{
					 PRINTF(DBG_NDIS_OIDS_VERY_LOUD, ("Buffer for parameter 0x%X is bigger then Param size\r\n", Param.paramType));
					 *cmdObj->param4 = Param.paramLength;
				}
				else
				{
					((PNDIS_802_11_LSM_PARAMETERS)cmdObj->buffer1)->PacketLatency = Param.content.tPsRxStreaming.uStreamPeriod;
					((PNDIS_802_11_LSM_PARAMETERS)cmdObj->buffer1)->Mode = Param.content.tPsRxStreaming.bEnabled;
					*cmdObj->param3 = cmdObj->buffer1_len;
				}
			}
		}
		break;

	case OID_802_11_RADIO_POWER_STATE:
		Param.paramType = SME_RADIO_ON_PARAM;
		Param.paramLength = sizeof(TI_BOOL);
		if (cmdObj->flags == OS_OID_SET_FLAG)
		{
			WLAN_OS_REPORT(("TIWL: OID_802_11_RADIO_POWER_STATE SET\r\n"));
			Param.content.smeRadioOn = *((TI_BOOL*)cmdObj->buffer1);
			*cmdObj->param3 = cmdObj->buffer1_len;
			res = cmdDispatch_SetParam (pCmdInterpret->hCmdDispatch, &Param);
		}
		else
		{
			WLAN_OS_REPORT(("TIWL: OID_802_11_RADIO_POWER_STATE GET\r\n"));
			res = cmdDispatch_GetParam (pCmdInterpret->hCmdDispatch, &Param);
			if (res == TI_OK)
			{
				 if (Param.paramLength > cmdObj->buffer1_len)
				{
					 PRINTF(DBG_NDIS_OIDS_VERY_LOUD, ("Buffer for parameter 0x%X is bigger then Param size\r\n", Param.paramType));
					 *cmdObj->param4 = Param.paramLength;
				}
				else
				{
					os_memoryCopy(pCmdInterpret->hOs, cmdObj->buffer1, &Param.content.smeRadioOn, Param.paramLength);
					*cmdObj->param3 = cmdObj->buffer1_len;
				}
			}
		}
		break;

#endif    
	default:
		PRINTF(DBG_NDIS_OIDS_LOUD, ("TIWL: UNSUPPORTED OID (%X)\r\n", cmdObj->cmd));
		ndisStatus = NDIS_STATUS_NOT_SUPPORTED;
		break;
	}
    if (*cmdObj->param4  != 0)
    {
        *cmdObj->param3 = 0;
        ndisStatus = NDIS_STATUS_INVALID_LENGTH;
    }
    else if (*cmdObj->param3 == 0)
    { 
        if (ndisStatus == NDIS_STATUS_SUCCESS)
        {
            ndisStatus = NDIS_STATUS_FAILURE;
        }
    }
    cmdObj->return_code = ndisStatus;

    if (g_oidPrint)
    {
        WLAN_OS_REPORT(("%s cmd=0x%x ndisStatus=0x%x b1=0x%x len=%d needed=%d\r\n",
                        (cmdObj->flags == OS_OID_SET_FLAG) ? "Set" : "",
                        cmdObj->cmd, ndisStatus,*(TI_INT32*)cmdObj->buffer1,*cmdObj->param3,*cmdObj->param4));
    }
	TRACE5(pCmdInterpret->hReport,REPORT_SEVERITY_INFORMATION,"cmd=0x%x ndisStatus=0x%x b1=0x%x len=%d needed=%d\r\n",
                        cmdObj->cmd, ndisStatus,*(TI_INT32*)cmdObj->buffer1,*cmdObj->param3,*cmdObj->param4);
} __except (EXCEPTION_EXECUTE_HANDLER) 
{
    tb_printf();
}

	return res;

} /* cmdInterpret_convertAndExecute() */


void *cmdInterpret_GetStat (TI_HANDLE hCmdInterpret)
{
    return NULL;
}


static TI_INT32 AddWepKey(TI_HANDLE hCmdInterpret, void *pBuffer)
{
    cmdInterpret_t *pCmdInterpret = (cmdInterpret_t *)hCmdInterpret;
    paramInfo_t Param;
    OS_802_11_WEP* pWep;
    OS_802_11_KEY  key;
    UINT32         keyIndexTxRx;
    TWlanDrvIfObjPtr pAdapter = (TWlanDrvIfObjPtr)pCmdInterpret->hOs;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;
    TI_STATUS res = TI_NOK;
       
    pWep = (OS_802_11_WEP*)pBuffer;
    
    if ((pWep->KeyIndex & 0x3FFFFFFF) > 3)
    {
        return NDIS_STATUS_FAILURE;
    }

    key.Length = pWep->Length;
    
    /* Convert the Key index to match OS_802_11_KEY index */
    keyIndexTxRx = (pWep->KeyIndex & 0x80000000); 
    key.KeyIndex = keyIndexTxRx | /*(keyIndexTxRx>>1) |*/ (pWep->KeyIndex & 0x3FFFFFFF);
    key.KeyLength = pWep->KeyLength;  
    
    os_memoryCopy(pCmdInterpret->hOs, key.KeyMaterial, (void *)pWep->KeyMaterial, pWep->KeyLength);
    
    /* Set the MAC Address to zero for WEP */
    os_memoryZero(pCmdInterpret->hOs, key.BSSID, sizeof(key.BSSID));
    
    os_memoryCopy(pCmdInterpret->hOs, &Param.content.rsnOsKey, &key, sizeof(OS_802_11_KEY));
    Param.paramType = RSN_ADD_KEY_PARAM;
    Param.paramLength = sizeof(OS_802_11_KEY);
    
    res = cmdDispatch_SetParam (pCmdInterpret->hCmdDispatch, &Param);

    return res;
}


/* Register to receive events */
static TI_INT32 cmdInterpret_initEvents(TI_HANDLE hCmdInterpret)
{
    cmdInterpret_t *pCmdInterpret = (cmdInterpret_t *)(hCmdInterpret);
    IPC_EVENT_PARAMS evParams;
    TI_INT32 i = 0;

    for (i=0; i<IPC_EVENT_MAX; i++)
    {
        evParams.uDeliveryType      = DELIVERY_PUSH;
        evParams.uProcessID         = 0;
        evParams.uEventID           = 0;
        evParams.hUserParam         = hCmdInterpret;
        evParams.pfEventCallback    = cmdInterpret_Event;
        evParams.uEventType = i;
        EvHandlerRegisterEvent (pCmdInterpret->hEvHandler, (TI_UINT8*) &evParams, sizeof(IPC_EVENT_PARAMS));
        pCmdInterpret->hEvents[i]   = evParams.uEventID;
    }

	os_memoryZero(pCmdInterpret->hOs, pCmdInterpret->appEvents, sizeof(pCmdInterpret->appEvents));

    return TI_OK;
}

/* Unregister events */
static TI_INT32 cmdInterpret_unregisterEvents(TI_HANDLE hCmdInterpret)
{
    cmdInterpret_t *pCmdInterpret = (cmdInterpret_t *)(hCmdInterpret);
    IPC_EVENT_PARAMS evParams;
    TI_INT32 i = 0;

    for (i=0; i<IPC_EVENT_MAX; i++)
    {
        evParams.uEventType =  i;
        evParams.uEventID = pCmdInterpret->hEvents[i];
        EvHandlerUnRegisterEvent (pCmdInterpret->hEvHandler, &evParams);
    }

    return TI_OK;
}

VOID SendLoggerData(TI_UINT8* data, TI_UINT16 len)
{
   	WSADATA wsaData;
	SOCKET SendSocket;
	struct sockaddr_in RecvAddr;
	char SendBuf[500];
	int BufLen;

	PTI_IPC_EVENT LogMsg = (PTI_IPC_EVENT) SendBuf;

  /* Initialize Winsock */
  WSAStartup(MAKEWORD(2,2), &wsaData);

  /* Create a socket for sending data */
  SendSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

  /*
   * Set up the RecvAddr structure with the IP address of
   * the receiver (in this example case "123.456.789.1")
   * and the specified port number.
   */
  RecvAddr.sin_family = AF_INET;
  RecvAddr.sin_port = htons(LoggerId);
  RecvAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

  /* Send a datagram to the receiver */

	LogMsg->id = LoggerId;
	LogMsg->data_len = len+6;
	LogMsg->event = IPC_EVENT_LOGGER;

	LogMsg->data[0] = 'W';                   /* Ethernet protocol Prefix */
	LogMsg->data[1] = (unsigned char)((LogMsg->data_len-3) & 0xFF);          /* Message size (first byte) */ 
	LogMsg->data[2] = (unsigned char)(((LogMsg->data_len-3) >> 8) & 0xFF);   /* Message size (second byte) */
	/* Mark that this is log message */ 
	LogMsg->data[3] = 0;
	LogMsg->data[4] = 0;
	LogMsg->data[5] = 0;

	memcpy(&LogMsg->data[6], data, len);

	/* Put '0' in the end of the string */
	LogMsg->data[LogMsg->data_len+6] = 0;
	BufLen = LogMsg->data_len + sizeof(TI_IPC_EVENT);

	sendto(SendSocket, SendBuf, BufLen, 0, (SOCKADDR *) &RecvAddr, sizeof(RecvAddr));

	/* When the application is finished sending, close the socket. */
	closesocket(SendSocket);
	
	/* Clean up and quit. */
	WSACleanup();

}


TI_UINT32 cmdSendAppEvent(TI_UINT32 event, TI_UINT16 id, TI_UINT8* data, TI_UINT32 len)
{
	WSADATA wsaData;
	SOCKET SendSocket;
	struct sockaddr_in RecvAddr;
	char SendBuf[500];
	int BufLen;
	
	PTI_IPC_EVENT EvMsg = (PTI_IPC_EVENT) SendBuf;
	
	/* Initialize Winsock */
	WSAStartup(MAKEWORD(2,2), &wsaData);
	
	/* Create a socket for sending data */
	SendSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	
	/* 
     * Set up the RecvAddr structure with the IP address of 
	 * the receiver (in this example case "123.456.789.1") 
	 * and the specified port number. 
     */
	RecvAddr.sin_family = AF_INET;
	RecvAddr.sin_port = htons(id);
	RecvAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	
	/* Send a datagram to the receiver */
	
	EvMsg->id = id;
	EvMsg->data_len = len;
	EvMsg->event = event;

	os_memoryCopy(NULL, EvMsg->data, data, len);

	BufLen = EvMsg->data_len + sizeof(TI_IPC_EVENT);

	sendto(SendSocket, SendBuf, BufLen, 0, (SOCKADDR *) &RecvAddr, sizeof(RecvAddr));

	/* When the application is finished sending, close the socket. */
	closesocket(SendSocket);
	
	/* Clean up and quit. */
	WSACleanup();

	return TI_OK;
}


/* Handle driver events and convert to XXX format */
static TI_INT32 cmdInterpret_Event(IPC_EV_DATA* pData)
{
    IPC_EVENT_PARAMS * pInParam =  (IPC_EVENT_PARAMS *)pData;
    cmdInterpret_t *pCmdInterpret = (cmdInterpret_t *)(pInParam->hUserParam);
	TI_UINT32 rc = TI_OK, i;

	if(pInParam->uEventType <= IPC_EVENT_MAX_OS_EVENT) {

		rc = os_IndicateEvent (pCmdInterpret->hOs, pData);

		if(rc != TI_OK) {
			return rc;
		}

	}

	for(i=0; i<TI_MAX_APPLICATIONS; i++) {

		if(pCmdInterpret->appEvents[i].id && 
		   (pCmdInterpret->appEvents[i].events[pInParam->uEventType] == 1)) {

				rc = cmdSendAppEvent(pInParam->uEventType, pCmdInterpret->appEvents[i].id, 
								&pData->uBuffer[0], pData->uBufferSize);

		}
		
	}
   
    return rc;
}
