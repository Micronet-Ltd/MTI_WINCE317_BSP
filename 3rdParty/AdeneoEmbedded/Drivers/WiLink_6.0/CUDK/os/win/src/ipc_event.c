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
#include <windows.h>
#ifdef WM_ZOOM2
#include <windev.h>
#endif
#include <winsock2.h>

#include "cu_osapi.h"
#include "oserr.h"

#include "ipc_sta.h"
#include "ipc_event.h"

#include "TWDriver.h"
#include "STADExternalIf.h"

#include "privcmd.h"

#define TI_EVENTS_SOCKET_START			0x9988

static HANDLE hEventThread;
static SOCKET socket_id = 0;

static char buffer[1500];
U16 EventsPort;


static void IpcProcessEventDisassosiated(U8* buffer, U16 len)
{
	OS_802_11_DISASSOCIATE_REASON_T* disas = (OS_802_11_DISASSOCIATE_REASON_T*)buffer;

	switch(disas->eDisAssocType) {

	case OS_DISASSOC_STATUS_UNSPECIFIED:
		os_Printf("Disassociated with **** unspecified *** reason (User/SG/Recovery)\n");
		break;

	case OS_DISASSOC_STATUS_AUTH_REJECT:
		if (disas->uStatusCode == STATUS_PACKET_REJ_TIMEOUT)
		{
			os_error_printf(CU_MSG_ERROR, "CLI Event - Disassociated due to no Auth response \n");
        } 
        else
        {
			os_error_printf(CU_MSG_ERROR, "CLI Event - Disassociated due to Auth response packet with reason = %d\n", 
							disas->uStatusCode);
		}
		break;

		case OS_DISASSOC_STATUS_ASSOC_REJECT:
			if (disas->uStatusCode == STATUS_PACKET_REJ_TIMEOUT) {
				os_error_printf(CU_MSG_ERROR, "CLI Event - Disassociated due to no Assoc response \n");
			} 
			else {
				os_error_printf(CU_MSG_ERROR, "CLI Event - Disassociated due to Assoc response packet with reason = %d\n", 
								disas->uStatusCode);
			}
			break;

		case OS_DISASSOC_STATUS_SECURITY_FAILURE:
			os_error_printf(CU_MSG_ERROR, "CLI Event - Disassociated due to RSN failure\n");
			break;

		case OS_DISASSOC_STATUS_AP_DEAUTHENTICATE:
			os_error_printf(CU_MSG_ERROR, "CLI Event - Disassociated due to AP deAuthenticate packet with reason = %d\n", disas->uStatusCode);
			break;

		case OS_DISASSOC_STATUS_AP_DISASSOCIATE:
			os_error_printf(CU_MSG_ERROR, "CLI Event - Disassociated due to AP disAssoc packet with reason = %d\n", disas->uStatusCode);
			break;

		case OS_DISASSOC_STATUS_ROAMING_TRIGGER:
			os_error_printf(CU_MSG_ERROR, "CLI Event - Disassociated due to roaming trigger = %d\n", disas->uStatusCode);
			break;

		default:
			os_error_printf(CU_MSG_ERROR, "CLI Event - Disassociated with unknown reason = %d\n", disas->eDisAssocType);
			break;
	}
}

void ProcessLoggerMessage(PU8 data, U32 len);
void g_tester_send_event(U8 event_index);

void IpcEvent_Handle_Event(U8* buffer)
{
	PTI_IPC_EVENT data = (PTI_IPC_EVENT)buffer;

	if(EventsPort != data->id) {
		os_error_printf(CU_MSG_ERROR, "%s error: illegal ID(%X != %X)\n", __FUNCTION__, EventsPort, data->id);
		return;
	}

	g_tester_send_event(data->event);

	switch(data->event) {
	
	case IPC_EVENT_ASSOCIATED:
		os_error_printf(CU_MSG_ERROR, "IPC_EVENT_ASSOCIATED data len=%d\n", data->data_len);
		break;

	case IPC_EVENT_DISASSOCIATED:
		os_error_printf(CU_MSG_ERROR, "IPC_EVENT_DISASSOCIATED data len=%d\n", data->data_len);
		IpcProcessEventDisassosiated(data->data, (U16)data->data_len);
		break;

	case IPC_EVENT_MEDIA_SPECIFIC:
		os_error_printf(CU_MSG_ERROR, "IPC_EVENT_MEDIA_SPECIFIC data len=%d\n", data->data_len);
		break;

	case IPC_EVENT_SCAN_COMPLETE:
		os_error_printf(CU_MSG_ERROR, "IPC_EVENT_SCAN_COMPLETE data len=%d\n", data->data_len);
		break;

	case IPC_EVENT_LINK_SPEED:
		os_error_printf(CU_MSG_ERROR, "IPC_EVENT_LINK_SPEED data len=%d\n", data->data_len);
		break;

	case IPC_EVENT_AUTH_SUCC:
		os_error_printf(CU_MSG_ERROR, "IPC_EVENT_AUTH_SUCC data len=%d\n", data->data_len);
		break;

	case IPC_EVENT_TIMEOUT:
		os_error_printf(CU_MSG_ERROR, "IPC_EVENT_TIMEOUT data len=%d\n", data->data_len);
		break;

	case IPC_EVENT_CCKM_START:
		os_error_printf(CU_MSG_ERROR, "IPC_EVENT_CCKM_START data len=%d\n", data->data_len);
		break;

	case IPC_EVENT_EAPOL:
		os_error_printf(CU_MSG_ERROR, "IPC_EVENT_EAPOL data len=%d\n", data->data_len);
		break;

    case IPC_EVENT_RE_AUTH_STARTED:
        os_error_printf(CU_MSG_ERROR, (PS8)"IpcEvent_PrintEvent - received IPC_EVENT_RE_AUTH_STARTED\n");
        break;

    case IPC_EVENT_RE_AUTH_COMPLETED:
        os_error_printf(CU_MSG_ERROR, (PS8)"IpcEvent_PrintEvent - received IPC_EVENT_RE_AUTH_COMPLETED\n");
        break;
    
	case IPC_EVENT_RE_AUTH_TERMINATED:
        os_error_printf(CU_MSG_ERROR, (PS8)"IpcEvent_PrintEvent - received IPC_EVENT_RE_AUTH_TERMINATED\n");
        break;
	
	case IPC_EVENT_BOUND:
		os_error_printf(CU_MSG_ERROR, "IPC_EVENT_BOUND data len=%d\n", data->data_len);
		break;

	case IPC_EVENT_UNBOUND:
		os_error_printf(CU_MSG_ERROR, "IPC_EVENT_UNBOUND data len=%d\n", data->data_len);
		break;

	case IPC_EVENT_PREAUTH_EAPOL:
		os_error_printf(CU_MSG_ERROR, "IPC_EVENT_PREAUTH_EAPOL data len=%d\n", data->data_len);
		break;

	case IPC_EVENT_LOW_RSSI:
		os_error_printf(CU_MSG_ERROR, "IPC_EVENT_LOW_RSSI data len=%d\n", data->data_len);
		break;

	case IPC_EVENT_TSPEC_STATUS:
	{
		if(data->data_len == sizeof(OS_802_11_QOS_TSPEC_PARAMS))
		{
			OS_802_11_QOS_TSPEC_PARAMS* tspec = (OS_802_11_QOS_TSPEC_PARAMS*) data->data;

			os_error_printf(CU_MSG_ERROR, "CLI Event - IPC_EVENT_TSPEC_STATUS -- (ReasonCode = %d) \n",tspec->uReasonCode);
			os_error_printf(CU_MSG_ERROR, "Tspec Parameters (as received through event handler):\n");
			os_error_printf(CU_MSG_ERROR, "-----------------------------------------------------\n");
			os_error_printf(CU_MSG_ERROR, "userPriority = %d\n",tspec->uUserPriority);
			os_error_printf(CU_MSG_ERROR, "uNominalMSDUsize = %d\n",tspec->uNominalMSDUsize);
			os_error_printf(CU_MSG_ERROR, "uMeanDataRate = %d\n",tspec->uMeanDataRate);
			os_error_printf(CU_MSG_ERROR, "uMinimumPHYRate = %d\n",tspec->uMinimumPHYRate);
			os_error_printf(CU_MSG_ERROR, "uSurplusBandwidthAllowance = %d\n",tspec->uSurplusBandwidthAllowance);
			os_error_printf(CU_MSG_ERROR, "uAPSDFlag = %d\n",tspec->uAPSDFlag);
			os_error_printf(CU_MSG_ERROR, "MinimumServiceInterval = %d\n",tspec->uMinimumServiceInterval);
			os_error_printf(CU_MSG_ERROR, "MaximumServiceInterval = %d\n",tspec->uMaximumServiceInterval);
			os_error_printf(CU_MSG_ERROR, "uMediumTime = %d\n\n",tspec->uMediumTime);
		}
		else
		{
			os_error_printf(CU_MSG_ERROR, "IPC_EVENT_TSPEC_STATUS there is no(or bad) data provided(len=%d)\n", data->data_len);
		}
		break;
	}

	case IPC_EVENT_TSPEC_RATE_STATUS:
	{
		OS_802_11_THRESHOLD_CROSS_INDICATION_PARAMS* tind = (OS_802_11_THRESHOLD_CROSS_INDICATION_PARAMS*) data;

		if(data->data_len == sizeof(OS_802_11_THRESHOLD_CROSS_INDICATION_PARAMS)) {
			os_error_printf(CU_MSG_ERROR, "IPC_EVENT_TSPEC_RATE_STATUS uAC=%d  HighLowFlag=%d  AboveBelowFlag=%d\n",
							tind->uAC, tind->uHighOrLowThresholdFlag, tind->uAboveOrBelowFlag);
		}
		else {
			os_error_printf(CU_MSG_ERROR, "IPC_EVENT_TSPEC_RATE_STATUS there is no(or bad) data provided(len=%d)\n", data->data_len);
		}
		break;
	}

	case IPC_EVENT_MEDIUM_TIME_CROSS:
		os_error_printf(CU_MSG_ERROR, "IPC_EVENT_MEDIUM_TIME_CROSS data len=%d\n", data->data_len);
		break;

	case IPC_EVENT_ROAMING_COMPLETE:
		os_error_printf(CU_MSG_ERROR, "IPC_EVENT_ROAMING_COMPLETE data len=%d\n", data->data_len);
		break;

	case IPC_EVENT_EAP_AUTH_FAILURE:
		os_error_printf(CU_MSG_ERROR, "IPC_EVENT_EAP_AUTH_FAILURE data len=%d\n", data->data_len);
		break;

	case IPC_EVENT_WPA2_PREAUTHENTICATION:
		printf("IPC_EVENT_WPA2_PREAUTHENTICATION data len=%d\n", data->data_len);
		break;

	case IPC_EVENT_TRAFFIC_INTENSITY_THRESHOLD_CROSSED:
	{
		U32 *crossInfo = (U32 *)data->data;
		os_error_printf(CU_MSG_ERROR, "IPC_EVENT_TRAFFIC_INTENSITY_THRESHOLD_CROSSED data len=%d\n", data->data_len);
		os_error_printf(CU_MSG_ERROR, "Threshold(High=0,  Low=1)   crossed= %d\n", crossInfo[0]);
		os_error_printf(CU_MSG_ERROR, "Direction(Above=0, Below=1) crossed= %d\n", crossInfo[1]);
		break;
	}

	case IPC_EVENT_GWSI:
		os_error_printf(CU_MSG_ERROR, "IPC_EVENT_GWSI data len=%d\n", data->data_len);
		break;

	case IPC_EVENT_WPS_SESSION_OVERLAP:
		os_error_printf(CU_MSG_ERROR, "IPC_EVENT_WPS_SESSION_OVERLAP data len=%d\n", data->data_len);
		break;

    case IPC_EVENT_RSSI_SNR_TRIGGER:
        os_error_printf(CU_MSG_ERROR, (PS8)"IPC_EVENT_RSSI_SNR_TRIGGER (index = %d), Data = %d\n", (data->data[2] - 1),data->data[0]);
		break;

	case IPC_EVENT_LOGGER:
#ifdef ETH_SUPPORT    
		ProcessLoggerMessage(data->data, (U16)data->data_len);
#endif
		break;


	default:
		os_error_printf(CU_MSG_ERROR, "UNKNOWN EVENT - %d! \n", data->event);
		break;
	}

}

static DWORD EventsThreadFunc(LPVOID lpParam)
{
    int result;

    do 
	{
        result = recvfrom(socket_id, buffer, 1500, 0, NULL, 0);

        if( (result != 0) && (result != SOCKET_ERROR)) {
			IpcEvent_Handle_Event(buffer);
        }
        else {
            if(result == SOCKET_ERROR) {
                os_error_printf(CU_MSG_ERROR, "%s: error is returned by recvfrom.\n", __FUNCTION__);
            }
            else {
                os_error_printf(CU_MSG_ERROR, "%s: zerro is returned by recvfrom.\n", __FUNCTION__);
            }
            break;
        }
    } while(TRUE);

    return(0); 
}


THandle IpcEvent_Create(VOID)
{
	struct sockaddr_in server_addr;
	WSADATA wsaData;
    WORD wVersionRequested;
	int result, i;

    wVersionRequested = MAKEWORD( 2, 2 );
     
    result = WSAStartup( wVersionRequested, &wsaData );
    if ( result != 0 ) {
        /* Tell the user that we could not find a usable */
        /* WinSock DLL.                                  */
        os_error_printf(CU_MSG_ERROR, "Could not initializate WinSocks.\n" );
        return NULL;
    }
     
    /* Confirm that the WinSock DLL supports 2.2.*/
    /* Note that if the DLL supports versions later    */
    /* than 2.2 in addition to 2.2, it will still return */
    /* 2.2 in wVersion since that is the version we      */
    /* requested.                                        */
     
    if ( LOBYTE( wsaData.wVersion ) != 2 ||
            HIBYTE( wsaData.wVersion ) != 2 ) {
        /* Tell the user that we could not find a usable */
        /* WinSock DLL.                                  */
        WSACleanup( );
        os_error_printf(CU_MSG_ERROR, "Could not find a usable WinSock DLL.\n" );
        return NULL;
    }
     
    /* The WinSock DLL is acceptable. Proceed. */
    /******************/
    /* Open a socket */
    /****************/

    socket_id = socket(PF_INET, SOCK_DGRAM, 0);

    if (!socket_id) {
        /* Error opening socket */
        os_error_printf(CU_MSG_ERROR, "%s: error opening socket socket.\n", __FUNCTION__);
        return NULL;
    }

    /********************/
    /* Bind the socket */
    /******************/
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl (INADDR_ANY);

	for(i=0; i<TI_MAX_APPLICATIONS; i++) {

		EventsPort = (U16) (TI_EVENTS_SOCKET_START + i);
		server_addr.sin_port = htons(EventsPort);

		result = bind(socket_id, (struct sockaddr *)&server_addr, sizeof(server_addr));

		if(result == 0)
			break;
	}

    if (result != 0) {
        /* Error binding socket */
        os_error_printf(CU_MSG_ERROR, "%s: error binding logger socket. Error %d\n", __FUNCTION__, WSAGetLastError());
        closesocket(socket_id);
        return NULL;
    }

	//
	// Create events thread
	//
	hEventThread = CreateThread(NULL, 0, EventsThreadFunc, NULL, 0, NULL);

    if(hEventThread == NULL) {
        os_error_printf(CU_MSG_ERROR, "%s: Can't create Events thread.\n", __FUNCTION__);
		closesocket(socket_id);
		return NULL;
	}

	//
	// Pass port number to the driver
	//
	result = IPC_STA_Private_Send(NULL, TIWLN_EVENTS_REGISTRATION, &EventsPort, sizeof(U16), NULL, 0);
	if(result != OK) {
		closesocket(socket_id);
		TerminateThread(hEventThread, 0);
        CloseHandle(hEventThread);
		return NULL;
	}
		
	return hEventThread;
}


VOID IpcEvent_Destroy(THandle hIpcEvent)
{
	int result;

	result = IPC_STA_Private_Send(NULL, TIWLN_EVENTS_UNREGISTER, &EventsPort, sizeof(U16), NULL, 0);

	closesocket(socket_id);

	if(hEventThread != NULL) {
		TerminateThread(hEventThread, 0);
        CloseHandle(hEventThread);
	}
}


S32 IpcEvent_EnableEvent(THandle hIpcEvent, U32 event)
{
	ENABLE_DISABLE_EVENT endis;

	endis.id = EventsPort;
	endis.mask = event;

	IPC_STA_Private_Send(NULL, TIWLN_ENABLE_EVENT, &endis, sizeof(ENABLE_DISABLE_EVENT), NULL, 0);

	return OK;
}


S32 IpcEvent_DisableEvent(THandle hIpcEvent, U32 event)
{
	ENABLE_DISABLE_EVENT endis;

	endis.id = EventsPort;
	endis.mask = event;

	IPC_STA_Private_Send(NULL, TIWLN_DISABLE_EVENT, &endis, sizeof(ENABLE_DISABLE_EVENT), NULL, 0);

	return OK;
}


S32 IpcEvent_UpdateDebugLevel(THandle hIpcEvent, S32 debug_level)
{
	IPC_STA_Private_Send(NULL, TIWLN_EVENTS_DEBUG_LEVEL, &debug_level, sizeof(U32), NULL, 0);

	return OK;
}
