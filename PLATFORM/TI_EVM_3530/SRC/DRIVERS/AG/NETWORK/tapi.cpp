//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this sample source code is subject to the terms of the Microsoft
// license agreement under which you licensed this sample source code. If
// you did not accept the terms of the license agreement, you are not
// authorized to use this sample source code. For the terms of the license,
// please see the license agreement between you and Microsoft or, if applicable,
// see the LICENSE.RTF on your install media or the root of your tools installation.
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES.
//

//#include <windows.h>
//#include <tapi.h>
#include <list.hxx>

#include "btagpub.h"
#include "btagnetwork.h"

#include "svsutil.hxx"

#ifdef VMODEM
 #include "comm.h"
 #include "vline.h" 
 #include "mdmVFSM.h"
 #error "VMODEM not supported" 
#endif

#include "tapi.h"

#include "hsdrv.h"
#include "ag_hs_api.h"

#define  TAPI_DBG_PRN           1

#define DEFAULT_TAPI_CELL_LINE          L"Cellular Line"
#define TAPI_API_LOW_VERSION    0x00020000
#define TAPI_API_HIGH_VERSION   0x00020000
#define TAPI_COMMAND_TIMEOUT    8000
#define MIN_RING_INTERVAL       3500

typedef enum _NETWORK_STATE {
    NETWORK_STATE_DISCONNECTED = 0x00,  // No calls
    NETWORK_STATE_CONNECTED = 0x01,     // Either outgoing or incoming call has just connected
    NETWORK_STATE_RINGING = 0x02,       // Incoming call is in ringing state
    NETWORK_STATE_ONHOLD = 0x03,        // No active calls but one on hold
    NETWORK_STATE_INCALL = 0x04         // Active call
} NETWORK_STATE;

typedef struct _TapiCall {
    ULONG ulCallId;                     // ID for call
    USHORT usCallType;                  // Type of call
    HANDLE hWaitEvent;                  // For synchronous calls, event to unblock the wait
    DWORD dwWaitResponse;               // Response to return for synchronous calls
} TapiCall;

typedef ce::list<TapiCall, ce::free_list<5> >    TapiCallList;

class TapiData : public SVSRefObj {
public:
    DWORD           dwState;            // State of TAPI Network Module (see NETWORK_STATE structure)
    HLINEAPP        hLineApp;           // Handle to TAPI Line App
    HLINE           hLine;              // Handle to TAPI Line
    HCALL           hCall;              // Handle to Active TAPI Call
    HCALL           hHoldCall;          // Handle to TAPI Call on hold
    HCALL           hOfferingCall;      // Handle to TAPI Call in offering state
    LPLINECALLINFO  pCallInfo;          // Pointer to TAPI Call Info struct
    DWORD           cbCallInfo;         // Size of buffer (in bytes) pointed to by pCallInfo
    DWORD           dwAPIVersion;       // Version of TAPI API
    HANDLE          hTapiEvent;         // Event that gets signalled when a TAPI message is present
    DWORD           dwWaitTimeout;      // Timeout (in milliseconds) for blocking TAPI commands
    DWORD           dwLastRingTime;     // Last time a RING was sent to headset
    HANDLE          hThread;            // Handle to TAPI event thread
    TapiCallList    CallList;           // List of TAPI calls
        
    BOOL            fShutdown : 1;      // Set when TAPI Network Module is starting to shutdown
    BOOL            fCallIdNotify : 1;  // Set if caller id has been notified to AG
    
    CRITICAL_SECTION csLock;            // Critical Section for Network Module

    TapiData()
    {
        dwState = NETWORK_STATE_DISCONNECTED;

        hLineApp = NULL;
        hLine = NULL;
        hCall = NULL;
        hHoldCall = NULL;
        hOfferingCall = NULL;
        pCallInfo = NULL;
        cbCallInfo = NULL;        
        dwAPIVersion = NULL;
        hTapiEvent = NULL;        
        hThread = NULL;

        dwWaitTimeout = 0;
        dwLastRingTime = 0;

        fShutdown = FALSE;
        fCallIdNotify = FALSE;

        InitializeCriticalSection(&csLock);
    }

    ~TapiData()
    {
        DeleteCriticalSection(&csLock);
    }
};

static TapiData g_Data;
LONG g_Inited = 0;


void BthAGNetworkDeinit(void);


inline void TapiLock(void)
{
    EnterCriticalSection(&g_Data.csLock);    
}

inline void TapiUnlock(void)
{
    LeaveCriticalSection(&g_Data.csLock);
}

//
// Find an outstanding TAPI call
//
TapiCall* FindTapiCall(DWORD dwId)
{
    for (TapiCallList::iterator it = g_Data.CallList.begin(), itEnd = g_Data.CallList.end(); it != itEnd;) {
        if (it->ulCallId == dwId) {
            return &(*it);
        }

        ++it;
    }

    return NULL;
}


//
// Add a TAPI call to the list
//
DWORD AddTapiCall(DWORD dwId, USHORT usType, HANDLE hWaitEvent)
{
    DWORD dwRetVal = ERROR_SUCCESS;
    
    TapiCall call;
    memset(&call, 0, sizeof(call));    
    call.ulCallId = dwId;
    call.usCallType = usType;
    call.hWaitEvent = hWaitEvent;

    if (false == g_Data.CallList.push_front(call)) {
        dwRetVal = ERROR_OUTOFMEMORY;
    }

    return dwRetVal;
}


//
// Remove a TAPI call from the list
//
DWORD DeleteTapiCall(DWORD dwId)
{
    DWORD dwRetVal = ERROR_NOT_FOUND;

    for (TapiCallList::iterator it = g_Data.CallList.begin(), itEnd = g_Data.CallList.end(); it != itEnd;) {
        if (it->ulCallId == dwId) {
            g_Data.CallList.erase(it);
            dwRetVal = ERROR_SUCCESS;
            break;
        }

        ++it;
    }

    return dwRetVal;
}

//
// This function finds the TSP line for cellular.
//
DWORD FindTSPLine(WCHAR* szName, unsigned int cLines, unsigned int* puiCellLine)
{
    DWORD dwRetVal = ERROR_NOT_FOUND;

	RETAILMSG(TAPI_DBG_PRN, (L"BTAGSVC:+%S - '%s'\r\n",__FUNCTION__, szName));
 
    for (DWORD i = 0; i < (DWORD) cLines; i++) {
        LINEEXTENSIONID lineExtensionId;
        DWORD dwVersion;
        
		RETAILMSG(TAPI_DBG_PRN, (L"BTAGSVC:%S, line # %d\r\n",__FUNCTION__, i));

        if (ERROR_SUCCESS == lineNegotiateAPIVersion(g_Data.hLineApp, i, TAPI_API_LOW_VERSION, TAPI_API_HIGH_VERSION, &dwVersion, &lineExtensionId)) {
            LINEDEVCAPS LineDevCaps;
            LineDevCaps.dwTotalSize = sizeof(LineDevCaps);
            if (ERROR_SUCCESS == lineGetDevCaps(g_Data.hLineApp, i, dwVersion, 0, &LineDevCaps)) {
                LINEDEVCAPS* pLineDevCaps = (LINEDEVCAPS*) new BYTE[LineDevCaps.dwNeededSize];
                if (pLineDevCaps) {
                    pLineDevCaps->dwTotalSize = LineDevCaps.dwNeededSize;
                    if (ERROR_SUCCESS == lineGetDevCaps(g_Data.hLineApp, i, dwVersion, 0, pLineDevCaps)) {

						RETAILMSG(TAPI_DBG_PRN, (L"BTAGSVC:%S found line '%s'\r\n",__FUNCTION__, (WCHAR*)((BYTE*)pLineDevCaps + pLineDevCaps->dwLineNameOffset) ));

                        if (0 == wcscmp((WCHAR*)((BYTE*)pLineDevCaps + pLineDevCaps->dwLineNameOffset), szName)) {
                            dwRetVal = ERROR_SUCCESS;
                            *puiCellLine = i;
                            g_Data.dwAPIVersion = dwVersion;
                            delete[] pLineDevCaps;
                            break;
                        }
                    }

                    delete[] pLineDevCaps;
                }
            }
        }
    }

	RETAILMSG(TAPI_DBG_PRN, (L"BTAGSVC:-%S ret = %d\r\n",__FUNCTION__, dwRetVal));
    return dwRetVal;
}

//
// This function opens the cellular TAPI line.
//
DWORD OpenLine(unsigned int uiLine)
{
    DWORD dwRetVal = ERROR_SUCCESS;
    
    dwRetVal = lineOpen(g_Data.hLineApp, 
        uiLine, 
        &g_Data.hLine, 
        g_Data.dwAPIVersion, 
        0, 
        uiLine, 
        LINECALLPRIVILEGE_MONITOR|LINECALLPRIVILEGE_OWNER,
        LINEMEDIAMODE_DATAMODEM,
        NULL);
    if (ERROR_SUCCESS != dwRetVal) {
        ASSERT(0);
        goto exit;
    }
    
    

exit:
    return dwRetVal;
}


//
// This functions gets the current call info
//
DWORD GetCallInfo(HCALL hCall)
{
    LRESULT lResult;
    DWORD dwRetVal = ERROR_SUCCESS;
    
    if ((0 == g_Data.cbCallInfo) || (!g_Data.pCallInfo)) {
        // First time, need to alloc enough memory
        const DWORD c_dwFirstSize = sizeof(LINECALLINFO) + 256;
        g_Data.pCallInfo = (LPLINECALLINFO) new BYTE[c_dwFirstSize];
        if (! g_Data.pCallInfo) {
            dwRetVal = ERROR_OUTOFMEMORY;
            DEBUGMSG(ZONE_ERROR, (L"BTAGSVC: Out of memory.\n"));
            goto exit;
        }
        g_Data.cbCallInfo = c_dwFirstSize;
    }

    g_Data.pCallInfo->dwTotalSize = g_Data.cbCallInfo;
    lResult = lineGetCallInfo(hCall, g_Data.pCallInfo);
    if (LINEERR_NOMEM == lResult) {
        g_Data.cbCallInfo = g_Data.pCallInfo->dwNeededSize;
        // Need more memory, free and realloc.
        delete[] g_Data.pCallInfo;                
        g_Data.pCallInfo = (LPLINECALLINFO) new BYTE[g_Data.cbCallInfo];
        if (! g_Data.pCallInfo) {
            g_Data.cbCallInfo = 0;
            dwRetVal = ERROR_OUTOFMEMORY;
            DEBUGMSG(ZONE_ERROR, (L"BTAGSVC: Out of memory.\n"));
            goto exit;
        }        
    }
    else if (0 != lResult) {
        dwRetVal = (DWORD) lResult;
        goto exit;
    }

exit:
    return dwRetVal;
}


//
// This function puts the active call on hold and blocks until this operation is complete.
//
DWORD BlockingTapiCall(LONG lCallId)
{
    DWORD dwRetVal = ERROR_SUCCESS;
    DWORD dwTimeout = g_Data.dwWaitTimeout;
    DWORD dwErr;
    
    HANDLE h = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (! h) {
        dwRetVal = GetLastError();
        DEBUGMSG(ZONE_ERROR, (L"BTAGSVC: BlockingTapiCall - CreateEvent Failed: %d\n", dwRetVal));
        goto exit;
    }
    
    dwRetVal = AddTapiCall(lCallId, 0, h);
    if (ERROR_SUCCESS != dwRetVal) {
        goto exit;
    }

    g_Data.AddRef();
    TapiUnlock();
    
    dwErr = WaitForSingleObject(h, dwTimeout);
    
    TapiLock();
    g_Data.DelRef();

    if (WAIT_OBJECT_0 == dwErr) {
        TapiCall* pCall = FindTapiCall(lCallId);
        if (! pCall) {
            dwRetVal = ERROR_NOT_FOUND;
            goto exit;
        }
        
        dwRetVal = pCall->dwWaitResponse;
        if (dwRetVal == ERROR_SUCCESS) {
            // Call state change should have already updated this
            ASSERT(g_Data.hCall == NULL);   
            ASSERT(g_Data.dwState == NETWORK_STATE_ONHOLD);
        }
    }
    else {
        // Timeout or unexpected failure
        dwRetVal = ERROR_TIMEOUT;
    }

exit:
    if (h) {
        CloseHandle(h);
    }
    
    return dwRetVal;
}


//
// This function is called when TAPI device state has changed
//
void TapiEventDevStateChange(LPLINEMESSAGE pMsg)
{
  RETAILMSG(TAPI_DBG_PRN, (L"BTAGSVC:+%S\r\n",__FUNCTION__));

    if (LINEDEVSTATE_RINGING == pMsg->dwParam1) {
        // Ringing cycle
        DEBUGMSG(ZONE_NETWORK, (L"BTAGSVC: Network - RING.\n"));
		RETAILMSG(TAPI_DBG_PRN, (L"BTAGSVC:%S, Network - RING\r\n",__FUNCTION__));
        g_Data.dwState = NETWORK_STATE_RINGING;

        //g_Data.hOfferingCall = hCall;
        //g_Data.dwLastRingTime = 0;

        DWORD dwCurrTime = GetTickCount();
        if (!g_Data.dwLastRingTime || ((dwCurrTime - g_Data.dwLastRingTime) >= MIN_RING_INTERVAL)) {
            g_Data.dwLastRingTime = dwCurrTime;
            
            TapiUnlock();
            BthAGOnNetworkEvent(NETWORK_EVENT_RING, NULL, 0);
			BthAGSendEvent(EV_AG_NET_RING, 0);
            TapiLock();
        }
    }
}


//
// This function is called when Tapi Call Info has changed
//
void TapiEventCallInfo(LPLINEMESSAGE pMsg) 
{
#ifndef VMODEM
    if (!g_Data.fCallIdNotify && (pMsg->dwParam1 & LINECALLINFOSTATE_CALLERID)) {
        HCALL hCall = (HCALL) pMsg->hDevice;

        DEBUGMSG(ZONE_NETWORK, (L"BTAGSVC: Network - Need to indicate Caller Id.\n"));
		RETAILMSG(TAPI_DBG_PRN, (L"BTAGSVC:+%S, Network - Need to indicate Caller Id.\r\n",__FUNCTION__));
        
        CHAR szNumber[MAX_PHONE_NUMBER];
        szNumber[0] = 0; // Null terminate

        if (ERROR_SUCCESS != GetCallInfo(hCall)) {
            DEBUGMSG(ZONE_WARN, (L"BTAGSVC: Query for caller id failed.\n"));
            return;
        }

#if 0
        if (LINEMEDIAMODE_INTERACTIVEVOICE != g_Data.pCallInfo->dwMediaMode) {
            DEBUGMSG(ZONE_WARN, (L"BTAGSVC: Call is of wrong media mode: 0x%X\n", g_Data.pCallInfo->dwMediaMode));
            return;
        }                    
#endif

        if (g_Data.pCallInfo->dwCallerIDOffset > 0) {
            DWORD cbWritten = WideCharToMultiByte(CP_ACP, 0, 
                                    (WCHAR*)((BYTE*)g_Data.pCallInfo + g_Data.pCallInfo->dwCallerIDOffset), 
                                    g_Data.pCallInfo->dwCallerIDSize/2,
                                    szNumber,
                                    MAX_PHONE_NUMBER,
                                    NULL, NULL);

            g_Data.fCallIdNotify = TRUE;
                        
            TapiUnlock();
            BthAGOnNetworkEvent(NETWORK_EVENT_CALL_INFO, (LPVOID)szNumber, cbWritten);
            TapiLock();
        }
    }
#endif
}


//
// This function is called when an asychronous TAPI command completes
//
void TapiEventLineReply(LPLINEMESSAGE pMsg)
{
    TapiCall* pCall = FindTapiCall(pMsg->dwParam1);
    if (! pCall) {
        DEBUGMSG(ZONE_NETWORK, (L"BTAGSVC: Network - TapiEventLineReply: Reply for non-existent call.\n"));
		RETAILMSG(1, (L"BTAGSVC:%S, Network - TapiEventLineReply: Reply for non-existent call.\r\n",__FUNCTION__));
        ASSERT(0);
        return;
    }

    if (pCall->hWaitEvent) {
        // We are internally making a TAPI call and waiting
        // for the response before returning to AG.
        
        DEBUGMSG(ZONE_NETWORK, (L"BTAGSVC: Network - TapiEventLineReply: Reply for synchronous call.\n"));
		RETAILMSG(TAPI_DBG_PRN, (L"BTAGSVC:%S, Network - TapiEventLineReply: Reply for synchronous call.\r\n",__FUNCTION__));
        
        pCall->dwWaitResponse = pMsg->dwParam2;
        SetEvent(pCall->hWaitEvent);
    }
    else {
        // AG is making async network call.

        ASSERT(pCall->ulCallId > 0);
        ASSERT(pCall->usCallType);
 
        DWORD dwResponse = pMsg->dwParam2;
		RETAILMSG(TAPI_DBG_PRN, (L"BTAGSVC:%S, Network - AG is making async network call, resp = %d\r\n",__FUNCTION__, dwResponse));

        if (dwResponse != ERROR_SUCCESS) {
            TapiUnlock();

            NetworkCallFailedInfo CallFailInfo;
            memset(&CallFailInfo, 0, sizeof(CallFailInfo));

            CallFailInfo.usCallType = pCall->usCallType;
            CallFailInfo.dwStatus = dwResponse;

			RETAILMSG(TAPI_DBG_PRN, (L"BTAGSVC:%S, Network - NETWORK_EVENT_FAILED\r\n",__FUNCTION__));

            BthAGOnNetworkEvent(NETWORK_EVENT_FAILED, (LPVOID)&CallFailInfo, sizeof(CallFailInfo));
            
            TapiLock();
        }

        DeleteTapiCall(pMsg->dwParam1);
    }
}


//
// This function is called when a TAPI call state changes
//
void TapiEventCallStateChange(LPLINEMESSAGE pMsg)
{
    HCALL hCall = (HCALL) pMsg->hDevice;

	RETAILMSG(TAPI_DBG_PRN, (L"BTAGSVC:+%S, hCall = 0x%X, param1 = 0x%X\r\n",__FUNCTION__, hCall, pMsg->dwParam1));

#ifndef VMODEM
    if (ERROR_SUCCESS != GetCallInfo(hCall)) {
        DEBUGMSG(ZONE_WARN, (L"BTAGSVC: Query for caller id failed.\n"));
		RETAILMSG(1, (L"BTAGSVC:%S, Query for caller id failed.\r\n",__FUNCTION__));
        goto exit;
    }
#endif

    // Make sure is of the correct media mode
#if 0
    if (LINEMEDIAMODE_INTERACTIVEVOICE != g_Data.pCallInfo->dwMediaMode) {
        DEBUGMSG(ZONE_WARN, (L"BTAGSVC: Call is of wrong media mode: 0x%X\n", g_Data.pCallInfo->dwMediaMode));
		RETAILMSG(1, (L"BTAGSVC:%S, Call is of wrong media mode: 0x%X\r\n",__FUNCTION__, g_Data.pCallInfo->dwMediaMode));
        goto exit;
    }            
#endif

    if (LINECALLSTATE_DISCONNECTED == pMsg->dwParam1) {
        if ((g_Data.hCall == hCall) ||              // active call disconnected or
            (g_Data.hHoldCall == hCall) ||          // held call disconnected or
            (g_Data.hOfferingCall == hCall)) {      // offering call disconnected
            
            if (g_Data.hCall == hCall) {
                g_Data.hCall = NULL;
            }
            else if (g_Data.hHoldCall == hCall) {
                g_Data.hHoldCall = NULL;
            }
            else if (g_Data.hOfferingCall == hCall) {
                g_Data.hOfferingCall = NULL;
            }

            if (NETWORK_STATE_RINGING == g_Data.dwState) {
                DEBUGMSG(ZONE_NETWORK, (L"BTAGSVC: Network - Reject call.\n"));
				RETAILMSG(TAPI_DBG_PRN, (L"BTAGSVC:%S, Network - Reject call\r\n",__FUNCTION__));
                TapiUnlock();
                BthAGOnNetworkEvent(NETWORK_EVENT_CALL_REJECT, NULL, 0);
                TapiLock();
            }
            else {
                DEBUGMSG(ZONE_NETWORK, (L"BTAGSVC: Network - Hang-up call.\n"));
				RETAILMSG(TAPI_DBG_PRN, (L"BTAGSVC:%S, Network - Hang-up call.\r\n",__FUNCTION__));
                
                DWORD dwCalls = (g_Data.hCall ? 1 : 0) + (g_Data.hHoldCall ? 1 : 0) + (g_Data.hOfferingCall ? 1 : 0);
                
                TapiUnlock();
                if (LINEDISCONNECTMODE_BUSY == pMsg->dwParam2) {
                    BthAGOnNetworkEvent(NETWORK_EVENT_CALL_BUSY, (LPVOID)dwCalls, sizeof(DWORD));
                }
                else {
                    BthAGOnNetworkEvent(NETWORK_EVENT_CALL_DISCONNECT, (LPVOID)dwCalls, sizeof(DWORD));
                }
                TapiLock();
            }

            if (g_Data.hCall) {
                g_Data.dwState = NETWORK_STATE_INCALL;
            }
            else if (g_Data.hHoldCall) {
                g_Data.dwState = NETWORK_STATE_ONHOLD;
            }
            else {
                g_Data.dwState = NETWORK_STATE_DISCONNECTED;
            }
        }
    }
    else if (LINECALLSTATE_BUSY == pMsg->dwParam1) {
        if (g_Data.hOfferingCall == hCall) {
            // Busy
            DEBUGMSG(ZONE_NETWORK, (L"BTAGSVC: Network - Busy signal.\n"));
			RETAILMSG(TAPI_DBG_PRN, (L"BTAGSVC:%S, Network - Busy signal.\r\n",__FUNCTION__));
            g_Data.hOfferingCall = NULL;

            DWORD dwCalls = (g_Data.hCall ? 1 : 0) + (g_Data.hHoldCall ? 1 : 0) + (g_Data.hOfferingCall ? 1 : 0);

            TapiUnlock();
            BthAGOnNetworkEvent(NETWORK_EVENT_CALL_BUSY, (LPVOID)dwCalls, sizeof(DWORD));
            TapiLock();

            if (g_Data.hHoldCall) {
                g_Data.dwState = NETWORK_STATE_ONHOLD;
            }
            else {
                g_Data.dwState = NETWORK_STATE_DISCONNECTED;
            }
        }
    }
    else if (LINECALLSTATE_CONNECTED == pMsg->dwParam1) {                
        if (g_Data.hOfferingCall == hCall) {
            DEBUGMSG(ZONE_NETWORK, (L"BTAGSVC: Network - Offering call connected.\n"));            
			RETAILMSG(TAPI_DBG_PRN, (L"BTAGSVC:%S, Network - Offering call connected.\r\n",__FUNCTION__));

            if (g_Data.hCall) {
                // If we already have an active call, it will be going on hold
                //g_Data.hHoldCall = g_Data.hCall;
            }

            g_Data.hCall = hCall;
            g_Data.hOfferingCall = NULL;

            TapiUnlock();
            BthAGOnNetworkEvent(NETWORK_EVENT_CALL_CONNECT, NULL, 0);
            TapiLock();

            g_Data.dwState = NETWORK_STATE_INCALL;
        }
        else if (g_Data.hHoldCall == hCall) {
            DEBUGMSG(ZONE_NETWORK, (L"BTAGSVC: Network - Held call connected.\n")); 
			RETAILMSG(TAPI_DBG_PRN, (L"BTAGSVC:%S, Network - Held call connected.\r\n",__FUNCTION__));
            
            g_Data.hHoldCall = g_Data.hCall;
            g_Data.hCall = hCall;
            g_Data.dwState = NETWORK_STATE_INCALL;
        }
        else if (g_Data.hCall == hCall) {
            g_Data.dwState = NETWORK_STATE_INCALL;
        }
		else
		{
			// Just update state (when switching from modem voice mode)
            g_Data.hCall = hCall;
            g_Data.hOfferingCall = NULL;
            g_Data.dwState = NETWORK_STATE_INCALL;
		}
    }
    else if (LINECALLSTATE_ONHOLD == pMsg->dwParam1) {
        // Call put on hold
        DEBUGMSG(ZONE_NETWORK, (L"BTAGSVC: Network - Call put on hold.\n"));
		RETAILMSG(TAPI_DBG_PRN, (L"BTAGSVC:%S, Network - Call put on hold.\r\n",__FUNCTION__));
        
        if ((g_Data.hCall == hCall) && g_Data.hHoldCall) {
            // If we already have a call on hold, it must be becoming active
            g_Data.hCall = g_Data.hHoldCall;
        }
        else if (g_Data.hCall == hCall) {
            // An active call is placed on hold
            g_Data.hCall = NULL;
            g_Data.dwState = NETWORK_STATE_ONHOLD;
        }

        g_Data.hHoldCall = hCall;
    }
    else if (LINECALLSTATE_DIALING == pMsg->dwParam1) {
        // Outgoing call
        DEBUGMSG(ZONE_NETWORK, (L"BTAGSVC: Network - outgoing call.\n"));
		RETAILMSG(TAPI_DBG_PRN, (L"BTAGSVC:%S, Network - outgoing call.\r\n",__FUNCTION__));
        
        g_Data.hOfferingCall = hCall;
        g_Data.dwLastRingTime = 0;

        if (g_Data.dwState == NETWORK_STATE_DISCONNECTED) {
            g_Data.dwState = NETWORK_STATE_CONNECTED;
        }

#ifndef VMODEM
        LONG lErr = lineSetCallPrivilege(g_Data.hOfferingCall, LINECALLPRIVILEGE_OWNER);
        if (0 != lErr) {
            DEBUGMSG(ZONE_WARN, (L"BTAGSVC: Network - Error calling lineSetCallPrivilege: %d.\n", lErr));
			RETAILMSG(TAPI_DBG_PRN, (L"BTAGSVC:%S, Network - Error calling lineSetCallPrivilege: %d.\r\n",__FUNCTION__, lErr));
        }
#endif        
        TapiUnlock();
        BthAGOnNetworkEvent(NETWORK_EVENT_CALL_OUT, NULL, 0);
        TapiLock();
    }
    else if (LINECALLSTATE_OFFERING == pMsg->dwParam1) {
        // Incoming call
        DEBUGMSG(ZONE_NETWORK, (L"BTAGSVC: Network - incoming call.\n"));
		RETAILMSG(TAPI_DBG_PRN, (L"BTAGSVC:%S, Network - incoming call.\r\n",__FUNCTION__));
            
        CHAR szNumber[MAX_PHONE_NUMBER];
        szNumber[0] = 0; // Null terminate

        g_Data.fCallIdNotify = FALSE;
        DWORD cbWritten = 0;
#ifndef VMODEM
        if (g_Data.pCallInfo->dwCallerIDOffset > 0) {
            cbWritten = WideCharToMultiByte(CP_ACP, 0, 
                            (WCHAR*)((BYTE*)g_Data.pCallInfo + g_Data.pCallInfo->dwCallerIDOffset), 
                            g_Data.pCallInfo->dwCallerIDSize/2,
                            szNumber,
                            MAX_PHONE_NUMBER,
                            NULL, NULL);

            g_Data.fCallIdNotify = TRUE;
        }
#else

#endif        
        g_Data.hOfferingCall = hCall;
        g_Data.dwLastRingTime = 0;

        if (g_Data.dwState == NETWORK_STATE_DISCONNECTED) {
            //g_Data.dwState = NETWORK_STATE_CONNECTED;
			g_Data.dwState = NETWORK_STATE_RINGING;
        }
        
#ifndef VMODEM
        LONG lErr = lineSetCallPrivilege(g_Data.hOfferingCall, LINECALLPRIVILEGE_OWNER);
        if (0 != lErr) {
            DEBUGMSG(ZONE_WARN, (L"BTAGSVC: Network - Error calling lineSetCallPrivilege: %d.\n", lErr));
			RETAILMSG(TAPI_DBG_PRN, (L"BTAGSVC:%S, Network - Error calling lineSetCallPrivilege: %d.\r\n",__FUNCTION__, lErr));
        }
#endif        
        TapiUnlock();
		//BthAGSendEvent(EV_AG_NET_RING, 0);
        BthAGOnNetworkEvent(NETWORK_EVENT_CALL_IN, szNumber, cbWritten);
		BthAGOnNetworkEvent(NETWORK_EVENT_RING, NULL, 0);
        TapiLock();
    }     

#ifndef VMODEM
exit: 
#endif
    if ((LINECALLSTATE_IDLE == pMsg->dwParam1) || 
        (LINECALLSTATE_DISCONNECTED == pMsg->dwParam1)) {

        // Call has gone away - must deallocate the call handle that TAPI created for us in LINE_APPNEWCALL
#ifndef VMODEM
        lineDeallocateCall(hCall);
#else
	    vlineDeallocateCall(hCall);
#endif
    }


	RETAILMSG(TAPI_DBG_PRN, (L"BTAGSVC:-%S\r\n",__FUNCTION__));
    return;
}


typedef void (*PFNEVENTPROC)(LPLINEMESSAGE pMsg);

typedef struct _TAPI_EVENT_TBL {
    DWORD dwCommand;
    PFNEVENTPROC pfnHandler;
} TAPI_EVENT_TBL, *PTAPI_EVENT_TBL;

const TAPI_EVENT_TBL TapiEventTable[] = {
    LINE_LINEDEVSTATE, TapiEventDevStateChange,
    LINE_CALLINFO, TapiEventCallInfo,
    LINE_REPLY, TapiEventLineReply,
    LINE_CALLSTATE, TapiEventCallStateChange,    
};

#define TAPI_EVENT_TBL_LENGTH     (sizeof(TapiEventTable)/sizeof(TAPI_EVENT_TBL))

//
// This function is a thread that listens for messages from TAPI and processes them.
//
DWORD WINAPI TapiEventThread(LPVOID pv)
{
#ifdef VMODEM
	LINEMESSAGE msg = {0};
#endif

    DEBUGMSG(ZONE_NETWORK, (L"BTAGSVC: ++TapiEventThread\n"));
	RETAILMSG(TAPI_DBG_PRN, (L"BTAGSVC:+%S, fShutdown = %d\r\n",__FUNCTION__, g_Data.fShutdown));

    TapiLock();
    
    while (! g_Data.fShutdown) {
        LONG lResult;
        DWORD dwWait;
        HANDLE h = g_Data.hTapiEvent;
#ifndef VMODEM
        LINEMESSAGE msg;
#endif
        
        TapiUnlock();
        dwWait = WaitForSingleObject(h, INFINITE);
        TapiLock();

        if (g_Data.fShutdown) {
            DEBUGMSG(ZONE_NETWORK, (L"BTAGSVC: Network - Shutting down TapiEventThread.\n"));
			RETAILMSG(TAPI_DBG_PRN, (L"BTAGSVC:%S, Network - Shutting down TapiEventThread.\r\n",__FUNCTION__));
            break;
        }

#ifndef VMODEM
        lResult = lineGetMessage(g_Data.hLineApp, &msg, 0);
        if (lResult != 0) {
			RETAILMSG(TAPI_DBG_PRN, (L"BTAGSVC:%S, lineGetMessage ret =  %d\r\n",__FUNCTION__, lResult));
            ASSERT(0);
            continue;
        }
#else
		lResult = vlineGetMessage(&msg);
        if (lResult != 0) {
			RETAILMSG(TAPI_DBG_PRN, (L"BTAGSVC:%S, vlineGetMessage ret =  %d\r\n",__FUNCTION__, lResult));
            ASSERT(0);
            continue;
        }
#endif

        for (int i = 0; i < TAPI_EVENT_TBL_LENGTH; i++) {
            if (msg.dwMessageID == TapiEventTable[i].dwCommand) {
				RETAILMSG(TAPI_DBG_PRN, (L"BTAGSVC:%S, call TAPI event # %d handler \r\n",__FUNCTION__, TapiEventTable[i].dwCommand));
                TapiEventTable[i].pfnHandler(&msg);
                break;
            }
        }
    }

    if (g_Data.pCallInfo) {
        delete[] g_Data.pCallInfo;
        g_Data.pCallInfo = NULL;
    }

    TapiUnlock();

    DEBUGMSG(ZONE_NETWORK, (L"BTAGSVC: --TapiEventThread\n"));
	RETAILMSG(TAPI_DBG_PRN, (L"BTAGSVC:-%S\r\n",__FUNCTION__));
    
    return 0;
}


//
// This function is called to initialize the AG's Network Component.
//
DWORD BthAGNetworkInit(HINSTANCE hInstance)
{
    DWORD dwRetVal = ERROR_SUCCESS;
    HKEY hk;
#ifndef VMODEM
#if 0
	DWORD cLines;
    unsigned int uiCellLine;    
    LINEINITIALIZEEXPARAMS LineInitializeExParams;
	WCHAR wszName[MAX_PATH];
#endif
#else
	VLINE_PARAM_s   vlineParams;
#endif
    DWORD dwVersion = TAPI_API_HIGH_VERSION;
    BOOL fUseDefaultLine = TRUE;

    DEBUGMSG(ZONE_NETWORK, (L"BTAGSVC: ++BthAGNetworkInit\n"));
	RETAILMSG(TAPI_DBG_PRN, (L"BTAGSVC:+%S, Ver # 1.00\r\n",__FUNCTION__));

    if (InterlockedExchange(&g_Inited, 1)) {
        return dwRetVal;
    }

    TapiLock();

#ifndef VMODEM
#if 0
    LineInitializeExParams.dwTotalSize = sizeof(LineInitializeExParams);
    LineInitializeExParams.dwOptions = LINEINITIALIZEEXOPTION_USEEVENT;    
    dwRetVal = lineInitializeEx(&g_Data.hLineApp, hInstance, NULL, L"BTAGSVC", &cLines, &dwVersion, &LineInitializeExParams);
    if (ERROR_SUCCESS != dwRetVal) {
        ASSERT(0);
        goto exit;
    }
#endif
#else 

#if 0
	dwRetVal = vlineInit(&vlineParams);
    if (ERROR_SUCCESS != dwRetVal) {
		RETAILMSG(1, (L"BTAGSVC:%S, failed, ret = 0x%X \r\n",__FUNCTION__, dwRetVal ));
        ASSERT(0);
        goto exit;
    }
#endif
#endif

#ifndef VMODEM
#if 0
	g_Data.hTapiEvent = LineInitializeExParams.Handles.hEvent;
#endif
#else
	g_Data.hTapiEvent = vlineParams.hTapiEvent;
#endif

	g_Data.fShutdown = FALSE;

	//RETAILMSG(TAPI_DBG_PRN, (L"BTAGSVC:%S, num of lines = %d, ver = 0x%X, hTapiEvent = 0x%X \r\n",__FUNCTION__, cLines, dwVersion, g_Data.hTapiEvent ));
#if 0
    g_Data.hThread = CreateThread(NULL, 0, TapiEventThread, NULL, 0, NULL);
    if (! g_Data.hThread) {
        ASSERT(0);
        goto exit;
    }
#endif

#ifndef VMODEM
#if 0
    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, RK_AUDIO_GATEWAY, 0, 0, &hk)) {        
        DWORD cdwBytes = MAX_PATH;
        if (ERROR_SUCCESS == RegQueryValueEx(hk, _T("TapiLineName"), 0, NULL, (PBYTE)wszName, &cdwBytes)) {
            fUseDefaultLine = FALSE;
        }

        RegCloseKey(hk);
    }

	RETAILMSG(TAPI_DBG_PRN, (L"BTAGSVC:%S, use default line: %d\r\n",__FUNCTION__, fUseDefaultLine));

    if (fUseDefaultLine) {
		RETAILMSG(TAPI_DBG_PRN, (L"BTAGSVC:%S, look for TAPI cell line: %s\r\n",__FUNCTION__, DEFAULT_TAPI_CELL_LINE));
        dwRetVal = FindTSPLine(DEFAULT_TAPI_CELL_LINE, cLines, &uiCellLine);
    }
    else {
		RETAILMSG(TAPI_DBG_PRN, (L"BTAGSVC:%S, look for TAPI cell line: %s\r\n",__FUNCTION__, wszName));
        dwRetVal = FindTSPLine(wszName, cLines, &uiCellLine);
    }
    
    if (ERROR_SUCCESS != dwRetVal) {
        DEBUGMSG(ZONE_ERROR, (L"BTAGSVC: Error finding cellular line: %d\n", dwRetVal));
        RETAILMSG(TAPI_DBG_PRN, (L"BTAGSVC:%S, Error finding cellular line: %d\r\n",__FUNCTION__, dwRetVal));
        goto exit;
    }

    dwRetVal = OpenLine(uiCellLine);
    if (ERROR_SUCCESS != dwRetVal) {
        DEBUGMSG(ZONE_ERROR, (L"BTAGSVC: Error opening line: %d\n", dwRetVal));
		RETAILMSG(TAPI_DBG_PRN, (L"BTAGSVC:%S, Error opening line: %d\r\n",__FUNCTION__, dwRetVal));
        goto exit;
    }

    // Specify additional messages we would like to receive in TapiEventThread
    dwRetVal = lineSetStatusMessages(g_Data.hLine, 
        LINEDEVSTATE_RINGING | LINEDEVSTATE_CONNECTED | LINEDEVSTATE_DISCONNECTED, 
        0);
    if (ERROR_SUCCESS != dwRetVal) {
        DEBUGMSG(ZONE_ERROR, (L"BTAGSVC: Error setting line status messages: %d\n", dwRetVal));
		RETAILMSG(TAPI_DBG_PRN, (L"BTAGSVC:%S, Error setting line status messages: %d\r\n",__FUNCTION__, dwRetVal));
        goto exit;
    }
#endif
#else 
#endif

    g_Data.dwWaitTimeout = TAPI_COMMAND_TIMEOUT;

    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, RK_AUDIO_GATEWAY, 0, 0, &hk)) {
        DWORD dwData;
        DWORD cdwBytes = sizeof(dwData);
        if (ERROR_SUCCESS == RegQueryValueEx(hk, _T("TapiAsyncTimeoutSeconds"), 0, NULL, (PBYTE)&dwData, &cdwBytes)) {
            g_Data.dwWaitTimeout = (dwData * 1000);
        }

        RegCloseKey(hk);
    }

#if 0
	vlineStatusQuery();
#endif
    DEBUGMSG(ZONE_NETWORK, (L"BTAGSVC: Network Module was successfullly initialized.\n"));
	RETAILMSG(TAPI_DBG_PRN, (L"BTAGSVC:%S, Network Module was successfullly initialized.\r\n",__FUNCTION__));

//exit:
    TapiUnlock();
    
    if (ERROR_SUCCESS != dwRetVal) {
        BthAGNetworkDeinit();
    }

//	BthAGNetRegEvent(6);
    DEBUGMSG(ZONE_NETWORK, (L"BTAGSVC: --BthAGNetworkInit\n"));
	RETAILMSG(TAPI_DBG_PRN, (L"BTAGSVC:-%S\r\n",__FUNCTION__));
    
    return dwRetVal;
}


//
// This function deinitializes the AG's Network Component.
//
void BthAGNetworkDeinit(void)
{
    DEBUGMSG(ZONE_NETWORK, (L"BTAGSVC: ++BthAGNetworkDeinit\n"));
	RETAILMSG(TAPI_DBG_PRN, (L"BTAGSVC:+%S\r\n",__FUNCTION__));

    if (0 == g_Inited) {
        return; // Not initialized
    }

    TapiLock();
    
    if (g_Data.hThread) {
        HANDLE h = g_Data.hThread;
        g_Data.fShutdown = TRUE;
        SetEvent(g_Data.hTapiEvent);

        TapiUnlock();
        WaitForSingleObject(h, INFINITE);
        TapiLock();

        while (g_Data.GetRefCount() > 1) {
            DEBUGMSG(ZONE_NETWORK, (L"BTAGSVC: Sleeping while Tapi Network Component ref count is still active.\n"));
			RETAILMSG(TAPI_DBG_PRN, (L"BTAGSVC:%S, Sleeping while Tapi Network Component ref count is still active.\r\n",__FUNCTION__));
            TapiUnlock();
            Sleep(500);
            TapiLock();
        }

        // Wait for all calls to return
        int count = 0;
        while ((count++ > 10) && g_Data.CallList.size()) {
            DEBUGMSG(ZONE_NETWORK, (L"BTAGSVC: Sleeping a bit since all Tapi calls have not been returned.\n"));
			RETAILMSG(TAPI_DBG_PRN, (L"BTAGSVC:%S, Sleeping a bit since all Tapi calls have not been returned.\r\n",__FUNCTION__));
            TapiUnlock();
            Sleep(500);
            TapiLock();
        }

        // If not all calls have returned by this point, remove them
        if (g_Data.CallList.size()) {
            DEBUGMSG(ZONE_WARN, (L"BTAGSVC: Forcefully deleting remaining Tapi calls\n"));
			RETAILMSG(TAPI_DBG_PRN, (L"BTAGSVC:%S, Forcefully deleting remaining Tapi calls.\r\n",__FUNCTION__));
            g_Data.CallList.clear();
        }

        CloseHandle(h);
    }

#if 0

#ifndef VMODEM
    lineClose(g_Data.hLine);
    lineShutdown(g_Data.hLineApp);
#else
	vlineDeInit();
#endif

#endif

    TapiUnlock();

    InterlockedExchange(&g_Inited, 0);
    
    DEBUGMSG(ZONE_NETWORK, (L"BTAGSVC: --BthAGNetworkDeinit\n"));
	RETAILMSG(TAPI_DBG_PRN, (L"BTAGSVC:-%S\r\n",__FUNCTION__));
}


//
// This function dials a number through TAPI.
//
DWORD BthAGNetworkDialNumber(LPWSTR pwszNumber)
{
    DWORD dwRetVal = ERROR_SUCCESS;
#ifdef VMODEM
    static LONG lCallId=0;
#endif
    DEBUGMSG(ZONE_NETWORK, (L"BTAGSVC: ++BthAGNetworkDialNumber\n"));
	RETAILMSG(TAPI_DBG_PRN, (L"BTAGSVC:+%S, num = %s\r\n",__FUNCTION__, pwszNumber));

    TapiLock();


    if (NETWORK_STATE_INCALL == g_Data.dwState) {
        // We are already in an active call.  Put this call on hold first.

		RETAILMSG(TAPI_DBG_PRN, (L"BTAGSVC:%S, netw_state = NETWORK_STATE_INCALL\r\n",__FUNCTION__));
#if 0 // Meanwhile line hold is not supported       
        ASSERT(g_Data.hHoldCall == NULL);

#ifndef VMODEM
        LONG lCallId = lineHold(g_Data.hCall);
        if (lCallId < 0) {
            DEBUGMSG(ZONE_ERROR, (L"BTAGSVC: BthAGNetworkDialNumber - call to lineHold failed: %d.\n", lCallId));
			RETAILMSG(TAPI_DBG_PRN, (L"BTAGSVC:%S, BthAGNetworkDialNumber - call to lineHold failed: %d.\r\n",__FUNCTION__, lCallId));
        }
#endif        
        dwRetVal = BlockingTapiCall(lCallId);
        if (ERROR_SUCCESS != dwRetVal) {
            DEBUGMSG(ZONE_ERROR, (L"BTAGSVC: BthAGNetworkDialNumber - call to ActiveCallToHoldBlocking failed: %d.\n", dwRetVal));
			RETAILMSG(TAPI_DBG_PRN, (L"BTAGSVC:%S, BthAGNetworkDialNumber - call to ActiveCallToHoldBlocking failed: %d.\r\n",__FUNCTION__, dwRetVal));
        }
        else {
            // Call state change should have already updated this
            ASSERT(g_Data.hCall == NULL);   
            ASSERT(g_Data.dwState == NETWORK_STATE_ONHOLD);
        }
#endif
    }

    if ((NETWORK_STATE_DISCONNECTED == g_Data.dwState) ||   // Outgoing call
        (NETWORK_STATE_ONHOLD == g_Data.dwState)            // Outgoing call with call on hold
        ) {
		RETAILMSG(TAPI_DBG_PRN, (L"BTAGSVC:%S, netw_state = NETWORK_STATE_DISCONNECTED\r\n",__FUNCTION__));
		RETAILMSG(TAPI_DBG_PRN, (L"BTAGSVC:%S, hCall = 0x%X\r\n",__FUNCTION__, g_Data.hCall));

        ASSERT(g_Data.hCall == NULL);
         
#ifndef VMODEM
        LONG lCallId = lineMakeCall(g_Data.hLine, &g_Data.hCall, pwszNumber, 0, NULL);
        if (lCallId < 0) {
            dwRetVal = lCallId;
            DEBUGMSG(ZONE_ERROR, (L"BTAGSVC: BthAGNetworkDialNumber - call to lineMakeCall failed: %d.\n", dwRetVal));            
			RETAILMSG(TAPI_DBG_PRN, (L"BTAGSVC:%S, BthAGNetworkDialNumber - call to lineMakeCal failed: %d.\r\n",__FUNCTION__, dwRetVal));
            goto exit;
        }
#else

		lCallId = vlineMakeCall(pwszNumber);
		if(lCallId < 0)
		{
            dwRetVal = lCallId;
            DEBUGMSG(ZONE_ERROR, (L"BTAGSVC: BthAGNetworkDialNumber - call to vlineMakeCall failed: %d.\n", dwRetVal));            
			RETAILMSG(TAPI_DBG_PRN, (L"BTAGSVC:%S, BthAGNetworkDialNumber - call to vlineMakeCal failed: %d.\r\n",__FUNCTION__, dwRetVal));
            goto exit;
		}

#endif

#ifndef VMODEM
        dwRetVal = AddTapiCall(lCallId, CALL_TYPE_DIAL, NULL);
#endif
    }
    
exit:
    TapiUnlock();

    DEBUGMSG(ZONE_NETWORK, (L"BTAGSVC: --BthAGNetworkDialNumber\n"));
	RETAILMSG(TAPI_DBG_PRN, (L"BTAGSVC:-%S, ret = %d\r\n",__FUNCTION__, dwRetVal));
    
    return dwRetVal;    
}


//
// This function hangs up one or more calls in the specified states
//
DWORD BthAGNetworkDropCall(DWORD dwFlags)
{
 DWORD dwRetVal = ERROR_SUCCESS;
#ifdef VMODEM
 LONG lCallId;
#endif

    DEBUGMSG(ZONE_NETWORK, (L"BTAGSVC: ++BthAGNetworkDropCall\n"));
	RETAILMSG(TAPI_DBG_PRN, (L"BTAGSVC:+%S\r\n",__FUNCTION__));

    TapiLock();
#if 0
    if (g_Data.dwState > NETWORK_STATE_DISCONNECTED) {
        if (g_Data.hCall && g_Data.hHoldCall && (dwFlags & NETWORK_FLAGS_DROP_ACTIVE)) {
            // We have an active call and a call on hold and we are dropping the active call.
            // In this case, we want to unhold the other call.

			RETAILMSG(TAPI_DBG_PRN, (L"BTAGSVC:%S, We have an active call and a call on hold\r\n",__FUNCTION__));
#ifndef VMODEM
            LONG lCallId = lineDrop(g_Data.hCall, NULL, 0);
            if (lCallId < 0) {
                dwRetVal = lCallId;
                DEBUGMSG(ZONE_ERROR, (L"BTAGSVC: BthAGNetworkDropCall - call to lineDrop failed: %d.\n", dwRetVal));
				RETAILMSG(1, (L"BTAGSVC:%S, BthAGNetworkDropCall - call to lineDrop failed: %d.\r\n",__FUNCTION__, dwRetVal));
                goto exit;
            }
#else
            lCallId = vlineDrop(g_Data.hCall);
            if (lCallId < 0) {
                dwRetVal = lCallId;
                DEBUGMSG(ZONE_ERROR, (L"BTAGSVC: BthAGNetworkDropCall - call to vlineDrop failed: %d.\n", dwRetVal));
				RETAILMSG(1, (L"BTAGSVC:%S, BthAGNetworkDropCall - call to vlineDrop failed: %d.\r\n",__FUNCTION__, dwRetVal));
                goto exit;
            }
#endif

#ifndef VMODEM
            dwRetVal = BlockingTapiCall(lCallId);
            if (ERROR_SUCCESS != dwRetVal) {
                DEBUGMSG(ZONE_ERROR, (L"BTAGSVC: BthAGNetworkDropCall - call to BlockingTapiCall failed: %d.\n", dwRetVal));
				RETAILMSG(1, (L"BTAGSVC:%S, BthAGNetworkDropCall - call to BlockingTapiCall failed: %d.\r\n",__FUNCTION__, dwRetVal));
                goto exit;
            }
#endif
            g_Data.AddRef();
            TapiUnlock();
            
            dwRetVal = BthAGNetworkUnholdCall();
            
            TapiLock();  
            g_Data.DelRef();

            if (ERROR_SUCCESS != dwRetVal) {
                DEBUGMSG(ZONE_ERROR, (L"BTAGSVC: BthAGNetworkDropCall - call to BthAGNetworkUnholdCall failed: %d.\n", dwRetVal));
				RETAILMSG(1, (L"BTAGSVC:%S, BthAGNetworkDropCall - call to BthAGNetworkUnholdCall failed: %d.\r\n",__FUNCTION__, dwRetVal));
                goto exit;
            }
        }
        else {
            // Normal case

			RETAILMSG(TAPI_DBG_PRN, (L"BTAGSVC:%S, Normal case\r\n",__FUNCTION__));

            HCALL hCall = g_Data.hCall;
            if (! hCall) {
                hCall = g_Data.hOfferingCall;
            }

#ifndef VMODEM
            LONG lCallId;
#endif
            
            if( hCall && (dwFlags & (NETWORK_FLAGS_DROP_ACTIVE|NETWORK_FLAGS_DROP_OFFERING)) ) {
				RETAILMSG(TAPI_DBG_PRN, (L"BTAGSVC:%S, DROP_ACTIVE\r\n",__FUNCTION__));
#ifndef VMODEM
                lCallId = lineDrop(hCall, NULL, 0);
                if (lCallId < 0) {
                    dwRetVal = lCallId;
                    DEBUGMSG(ZONE_ERROR, (L"BTAGSVC: BthAGNetworkDropCall - call to lineDrop failed: %d.\n", dwRetVal));
					RETAILMSG(1, (L"BTAGSVC:%S, BthAGNetworkDropCall - call to lineDrop failed: %d.\r\n",__FUNCTION__, dwRetVal));
                    goto exit;
                }
#else
				lCallId = vlineDrop(hCall);
                if (lCallId < 0) {
                    dwRetVal = lCallId;
                    DEBUGMSG(ZONE_ERROR, (L"BTAGSVC: BthAGNetworkDropCall - call to vlineDrop failed: %d.\n", dwRetVal));
					RETAILMSG(1, (L"BTAGSVC:%S, BthAGNetworkDropCall - call to vlineDrop failed: %d.\r\n",__FUNCTION__, dwRetVal));
                    goto exit;
                }
			if( dwFlags & NETWORK_FLAGS_DROP_ACTIVE ) 	
			 BthAGSendEvent(EV_AG_NET_END_CALL, 0);
			if( dwFlags & NETWORK_FLAGS_DROP_OFFERING )
             BthAGSendEvent(EV_AG_NET_REJECT_INCALL, 0);
#endif

#ifndef VMODEM
                dwRetVal = AddTapiCall(lCallId, CALL_TYPE_DROP, NULL);
                if (ERROR_SUCCESS != dwRetVal) {
                    goto exit;
                }
#endif
            }

            hCall = g_Data.hHoldCall;

            if (hCall && (dwFlags & NETWORK_FLAGS_DROP_HOLD)) {
				RETAILMSG(TAPI_DBG_PRN, (L"BTAGSVC:%S, DROP_HOLD\r\n",__FUNCTION__));
#ifndef VMODEM
                lCallId = lineDrop(hCall, NULL, 0);
                if (lCallId < 0) {
                    dwRetVal = lCallId;
                    DEBUGMSG(ZONE_ERROR, (L"BTAGSVC: BthAGNetworkDropCall - call to lineDrop failed: %d.\n", dwRetVal));
					RETAILMSG(1, (L"BTAGSVC:%S, BthAGNetworkDropCall - call to lineDrop failed: %d.\r\n",__FUNCTION__, dwRetVal));
                    goto exit;
                }
#else
                lCallId = vlineDrop(hCall);
                if (lCallId < 0) {
                    dwRetVal = lCallId;
                    DEBUGMSG(ZONE_ERROR, (L"BTAGSVC: BthAGNetworkDropCall - call to vlineDrop failed: %d.\n", dwRetVal));
					RETAILMSG(1, (L"BTAGSVC:%S, BthAGNetworkDropCall - call to vlineDrop failed: %d.\r\n",__FUNCTION__, dwRetVal));
                    goto exit;
                }

#endif

#ifndef VMODEM
                dwRetVal = AddTapiCall(lCallId, CALL_TYPE_DROP, NULL);
                if (ERROR_SUCCESS != dwRetVal) {
                    goto exit;
                }
#endif
            }
        }
    }
    else {
        dwRetVal = ERROR_NOT_READY;
		RETAILMSG(TAPI_DBG_PRN, (L"BTAGSVC:%S, BthAGNetworkDropCall - ERROR_NOT_READY\r\n",__FUNCTION__));
    }

exit:    
#else  /* 0 */
	//BthAGSendEvent(EV_AG_HS_BUTTON_PRESS, 0);
#endif /* 0 */

    TapiUnlock();

    DEBUGMSG(ZONE_NETWORK, (L"BTAGSVC: --BthAGNetworkDropCall\n"));
	RETAILMSG(TAPI_DBG_PRN, (L"BTAGSVC:-%S, ret = %d\r\n",__FUNCTION__, dwRetVal));

    return dwRetVal;    
}


//
// This function answers the current incoming call.
//
DWORD BthAGNetworkAnswerCall(void)
{
    DWORD dwRetVal = ERROR_SUCCESS;
#ifdef VMODEM
 LONG lCallId;
#endif

    DEBUGMSG(ZONE_NETWORK, (L"BTAGSVC: ++BthAGNetworkAnswerCall\n"));
	RETAILMSG(TAPI_DBG_PRN, (L"BTAGSVC:+%S\r\n",__FUNCTION__));

    TapiLock();
#if 0

    if ((NETWORK_STATE_RINGING == g_Data.dwState) ||    // incoming call
        (NETWORK_STATE_ONHOLD == g_Data.dwState)        // call-waiting
        ) {

        ASSERT(g_Data.hCall == NULL);
        
#ifndef VMODEM
        LONG lErr = lineSetCallPrivilege(g_Data.hOfferingCall, LINECALLPRIVILEGE_OWNER);
        if (0 != lErr) {
            DEBUGMSG(ZONE_WARN, (L"BTAGSVC: Network - Error calling lineSetCallPrivilege: %d.\n", lErr));
			RETAILMSG(1, (L"BTAGSVC:%S, Network - Error calling lineSetCallPrivilege: %d.\r\n",__FUNCTION__, lErr));
        }
        
        LONG lCallId = lineAnswer(g_Data.hOfferingCall, NULL, 0);
        if (lCallId < 0) {
            dwRetVal = lCallId;
            DEBUGMSG(ZONE_ERROR, (L"BTAGSVC: BthAGNetworkAnswerCall - call to lineAnswer failed: %d.\n", dwRetVal));
			RETAILMSG(1, (L"BTAGSVC:%S, call to lineAnswer failed: %d.\r\n",__FUNCTION__, dwRetVal));
            goto exit;
        }
#else
        lCallId = vlineAnswer(g_Data.hOfferingCall);
        if (lCallId < 0) {
            dwRetVal = lCallId;
            DEBUGMSG(ZONE_ERROR, (L"BTAGSVC: BthAGNetworkAnswerCall - call to vlineAnswer failed: %d.\n", dwRetVal));
			RETAILMSG(1, (L"BTAGSVC:%S, call to vlineAnswer failed: %d.\r\n",__FUNCTION__, dwRetVal));
            goto exit;
        }
        //TapiUnlock();
        //BthAGOnNetworkEvent(NETWORK_EVENT_CALL_CONNECT, szNumber, cbWritten);
        //TapiLock();

	    BthAGSendEvent(EV_AG_NET_ANSWER, 0);
#endif

#ifndef VMODEM
        dwRetVal = AddTapiCall(lCallId, CALL_TYPE_ANSWER, NULL);
        if (ERROR_SUCCESS != dwRetVal) {
            goto exit;
        }        
#endif
    }
    else {
        dwRetVal = ERROR_NOT_READY;
    }

exit:
#else  /* 0 */
	  //BthAGSendEvent(EV_AG_HS_BUTTON_PRESS, 0);
#endif /* 0 */

    TapiUnlock();

    DEBUGMSG(ZONE_NETWORK, (L"BTAGSVC: --BthAGNetworkAnswerCall\n"));
	RETAILMSG(TAPI_DBG_PRN, (L"BTAGSVC:-%S\r\n",__FUNCTION__));

    return dwRetVal;    
}


//
// This function puts the active call on hold
//
DWORD BthAGNetworkHoldCall(void)
{
    DWORD dwRetVal = ERROR_SUCCESS;

    DEBUGMSG(ZONE_NETWORK, (L"BTAGSVC: ++BthAGNetworkHoldCall\n"));
	RETAILMSG(TAPI_DBG_PRN, (L"BTAGSVC:+%S\r\n",__FUNCTION__));

// meanwhile hold not supported        
#ifndef VMODEM

    TapiLock();

    if (NETWORK_STATE_INCALL == g_Data.dwState) {
        ASSERT(g_Data.hCall);
        LONG lErr = lineSetCallPrivilege(g_Data.hCall, LINECALLPRIVILEGE_OWNER);
        if (0 != lErr) {
            DEBUGMSG(ZONE_WARN, (L"BTAGSVC: Network - Error calling lineSetCallPrivilege: %d.\n", lErr));
			RETAILMSG(1, (L"BTAGSVC:%S, Network - Error calling lineSetCallPrivilege: %d\r\n",__FUNCTION__, lErr));
        }
        
        LONG lCallId = lineHold(g_Data.hCall);
        if (lCallId < 0) {
            dwRetVal = lCallId;
            DEBUGMSG(ZONE_ERROR, (L"BTAGSVC: BthAGNetworkHoldCall - call to lineHold failed: %d.\n", dwRetVal));
			RETAILMSG(1, (L"BTAGSVC:%S, call to lineHold failed: %d.\r\n",__FUNCTION__, dwRetVal));
            goto exit;
        }

        dwRetVal = AddTapiCall(lCallId, CALL_TYPE_HOLD, NULL);
        if (ERROR_SUCCESS != dwRetVal) {
            goto exit;
        }
    }
    else {
        dwRetVal = ERROR_NOT_READY;
    }

exit:
    TapiUnlock();
#endif

    DEBUGMSG(ZONE_NETWORK, (L"BTAGSVC: --BthAGNetworkHoldCall\n"));
	RETAILMSG(TAPI_DBG_PRN, (L"BTAGSVC:-%S, ret = %d\r\n",__FUNCTION__, dwRetVal));

    return dwRetVal;    
}


//
// This function puts the active call on hold (if it exists) and puts another call (offering or held) in
// the active state.
//
DWORD BthAGNetworkSwapCall(void)
{
    DWORD dwRetVal = ERROR_SUCCESS;
#ifndef VMODEM
    LONG lCallId;
#endif

    DEBUGMSG(ZONE_NETWORK, (L"BTAGSVC: ++BthAGNetworkSwapCall\n"));
	RETAILMSG(TAPI_DBG_PRN, (L"BTAGSVC:+%S\r\n",__FUNCTION__));

#if 0
#ifndef VMODEM

    TapiLock();

    if ((NETWORK_STATE_INCALL == g_Data.dwState) && g_Data.hHoldCall) {
        //
        // We are in an active call with a call on hold
        //
        ASSERT(g_Data.hCall);
        
        
        lCallId = lineSwapHold(g_Data.hCall, g_Data.hHoldCall);
        if (lCallId < 0) {
            dwRetVal = lCallId;
            DEBUGMSG(ZONE_ERROR, (L"BTAGSVC: BthAGNetworkSwapCall - call to lineSwapHold failed: %d.\n", dwRetVal));
            goto exit;
        }

        dwRetVal = AddTapiCall(lCallId, CALL_TYPE_SWAP, NULL);
        if (ERROR_SUCCESS != dwRetVal) {
            goto exit;
        }
    }
    else if ((NETWORK_STATE_INCALL == g_Data.dwState) && g_Data.hOfferingCall) {
        //
        // We are in an active call with an offering call
        //
        ASSERT(g_Data.hCall);
        ASSERT(g_Data.hHoldCall == NULL);

        lCallId = lineHold(g_Data.hCall);
        if (lCallId < 0) {
            DEBUGMSG(ZONE_ERROR, (L"BTAGSVC: BthAGNetworkSwapCall - call to lineHold failed: %d.\n", lCallId));
        }
        
        dwRetVal = BlockingTapiCall(lCallId);
        if (ERROR_SUCCESS != dwRetVal) {
            DEBUGMSG(ZONE_ERROR, (L"BTAGSVC: BthAGNetworkSwapCall - call to ActiveCallToHoldBlocking failed: %d.\n", dwRetVal));
        }
        else {
            // Call state change should have already updated this
            ASSERT(g_Data.hCall == NULL);   
            ASSERT(g_Data.dwState == NETWORK_STATE_ONHOLD);
        }

        g_Data.AddRef();
        TapiUnlock();
        
        dwRetVal = BthAGNetworkAnswerCall();
        
        TapiLock();
        g_Data.DelRef();
        
        if (ERROR_SUCCESS != dwRetVal) {
            DEBUGMSG(ZONE_ERROR, (L"BTAGSVC: BthAGNetworkSwapCall - call to BthAGNetworkAnswerCall failed: %d.\n", dwRetVal));
            goto exit;
        }
    }
    else if (NETWORK_STATE_INCALL == g_Data.dwState) {
        //
        // We are in an active call with no other calls
        //
        ASSERT(g_Data.hCall);

        g_Data.AddRef();
        TapiUnlock();
        
        dwRetVal = BthAGNetworkHoldCall();
        
        TapiLock();
        g_Data.DelRef();

        if (ERROR_SUCCESS != dwRetVal) {
            DEBUGMSG(ZONE_ERROR, (L"BTAGSVC: BthAGNetworkSwapCall - call to BthAGNetworkHoldCall failed: %d.\n", dwRetVal));
            goto exit;
        }
    }
    else if (NETWORK_STATE_ONHOLD == g_Data.dwState) {
        //
        // We are not in an active call and have a call on hold
        //
        ASSERT(g_Data.hCall == NULL);
        ASSERT(g_Data.hHoldCall);

        g_Data.AddRef();
        TapiUnlock();
        
        dwRetVal = BthAGNetworkUnholdCall();
        
        TapiLock();
        g_Data.DelRef();

        if (ERROR_SUCCESS != dwRetVal) {
            DEBUGMSG(ZONE_ERROR, (L"BTAGSVC: BthAGNetworkSwapCall - call to BthAGNetworkUnholdCall failed: %d.\n", dwRetVal));
            goto exit;
        }
    }
    else {
        // Unknown state
        dwRetVal = ERROR_NOT_READY;
        DEBUGMSG(ZONE_ERROR, (L"BTAGSVC: BthAGNetworkSwapCall - Error: We are not in a valid state to perform this operation.\n"));
    }

exit:
    TapiUnlock();

#endif
#else
     //TapiLock();
     //BthAGSendEvent(EV_AG_HS_BUTTON_PRESS, 0);
	 //TapiUnlock();
#endif

    DEBUGMSG(ZONE_NETWORK, (L"BTAGSVC: --BthAGNetworkSwapCall\n"));
	RETAILMSG(TAPI_DBG_PRN, (L"BTAGSVC:-%S, ret = %d\r\n",__FUNCTION__, dwRetVal));

    return dwRetVal;    
}

DWORD BthAGNetworkDropSwapCall(void)
{
    DWORD dwErr = ERROR_SUCCESS;

	RETAILMSG(TAPI_DBG_PRN, (L"BTAGSVC:+%S\r\n",__FUNCTION__));
#ifndef VMODEM
    dwErr = BthAGNetworkDropCall(NETWORK_FLAGS_DROP_ACTIVE);
    if (ERROR_SUCCESS == dwErr) {
        dwErr = BthAGNetworkSwapCall();
    }
#endif
	RETAILMSG(TAPI_DBG_PRN, (L"BTAGSVC:-%S, ret = %d\r\n",__FUNCTION__, dwErr));
    return dwErr;
}

DWORD BthAGNetworkUnholdCall(void)
{
    DWORD dwRetVal = ERROR_SUCCESS;

    DEBUGMSG(ZONE_NETWORK, (L"BTAGSVC: ++BthAGNetworkUnholdCall\n"));
	RETAILMSG(TAPI_DBG_PRN, (L"BTAGSVC:%S\r\n",__FUNCTION__));

#ifndef VMODEM
    TapiLock();

    if (NETWORK_STATE_ONHOLD == g_Data.dwState) {
        LONG lCallId = lineUnhold(g_Data.hHoldCall);
        if (lCallId < 0) {
            dwRetVal = lCallId;
            DEBUGMSG(ZONE_ERROR, (L"BTAGSVC: BthAGNetworkUnholdCall - call to lineUnhold failed: %d.\n", dwRetVal));
            goto exit;
        }

        dwRetVal = AddTapiCall(lCallId, CALL_TYPE_UNHOLD, NULL);
        if (ERROR_SUCCESS != dwRetVal) {
            goto exit;
        }
    }
    else {
        DEBUGMSG(ZONE_ERROR, (L"BTAGSVC: BthAGNetworkUnholdCall - Invalid state current=%d need to be in NETWORK_STATE_ONHOLD.\n", g_Data.dwState));
        dwRetVal = ERROR_NOT_READY;
    }

exit:
    TapiUnlock();

#endif
    DEBUGMSG(ZONE_NETWORK, (L"BTAGSVC: --BthAGNetworkUnholdCall\n"));
	RETAILMSG(TAPI_DBG_PRN, (L"BTAGSVC:-%S, ret = %d\r\n",__FUNCTION__, dwRetVal));

    return dwRetVal;    
}


// This function transmits DTMF codes to TAPI.
DWORD BthAGNetworkTransmitDTMF(LPWSTR pwszDTMF)
{
    LONG dwRetVal = ERROR_SUCCESS;

    DEBUGMSG(ZONE_NETWORK, (L"BTAGSVC: ++BthAGNetworkTransmitDTMF\n"));

    TapiLock();

    if (NETWORK_STATE_INCALL == g_Data.dwState) {
        dwRetVal = lineGenerateDigits(g_Data.hCall, LINEDIGITMODE_DTMF, pwszDTMF, 0);                
    }
    else {
        dwRetVal = ERROR_NOT_READY;
    }

    TapiUnlock();

    DEBUGMSG(ZONE_NETWORK, (L"BTAGSVC: --BthAGNetworkTransmitDTMF\n"));

    return dwRetVal;    
}


// This function gets the call state
DWORD BthAGNetworkGetCallState(PDWORD pdwFlags)
{
	RETAILMSG(TAPI_DBG_PRN, (L"+BTAGSVC:+%S, hCall = %d, hHoldCall = %d, hOfferingCall = %d\r\n",__FUNCTION__, g_Data.hCall, g_Data.hHoldCall, g_Data.hOfferingCall));

    TapiLock();

#if 0
	BthAGSendEvent(EV_AG_HS_BUTTON_PRESS, 0);

    if (g_Data.hCall) {
        *pdwFlags |= NETWORK_FLAGS_STATE_ACTIVE;
		RETAILMSG(TAPI_DBG_PRN, (L"+BTAGSVC:%S, NETWORK_FLAGS_STATE_ACTIVE\r\n",__FUNCTION__));
    }
    if (g_Data.hHoldCall) {
        *pdwFlags |= NETWORK_FLAGS_STATE_HOLD;
		RETAILMSG(TAPI_DBG_PRN, (L"+BTAGSVC:%S, NETWORK_FLAGS_STATE_HOLD\r\n",__FUNCTION__));
    }
    if (g_Data.hOfferingCall) {
        *pdwFlags |= NETWORK_FLAGS_STATE_OFFERING;
		RETAILMSG(TAPI_DBG_PRN, (L"+BTAGSVC:%S, NETWORK_FLAGS_STATE_OFFERING\r\n",__FUNCTION__));
    }   
#else  /* 0 */
	// Always return 0, user application is responisble for (telephone)Network state machine
    *pdwFlags |= 0;
	//BthAGSendEvent(EV_AG_HS_BUTTON_PRESS, 0);
#endif /* 0 */

    TapiUnlock();

	RETAILMSG(TAPI_DBG_PRN, (L"+BTAGSVC:-%S\r\n",__FUNCTION__));

    return ERROR_SUCCESS;
}


