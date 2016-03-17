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

#include <windows.h>
#include "btagpub.h"

#include "hsdrv.h"
#include "ag_hs_api.h"

#ifndef PREFAST_ASSERT
#define PREFAST_ASSERT(x) ASSERT(x)
#endif


// This function is called to initialize the AG
DWORD BthAGPhoneExtInit(void)
{
    return ERROR_SUCCESS;
}

// This function is called to deinitialize the AG
void BthAGPhoneExtDeinit(void)
{
}

// This function provides informational events related to the phone
void BthAGPhoneExtEvent(DWORD dwEvent, DWORD dwParam, VOID* pvParam2)
{
	RETAILMSG(1, (L"HS_DRV:+%S, event = 0x%X, param = 0x%X\r\n",__FUNCTION__, dwEvent, dwParam));

    switch (dwEvent) {
        case AG_PHONE_EVENT_VOICE_RECOG:
			//BthAGSendEvent(dwEvent, dwParam);
            DEBUGMSG(ZONE_PHONEUI, (L"BTAGSVC: *** Audio Gateway voice recognition has been updated to: %d ***\n", dwParam));
            break;

        case AG_PHONE_EVENT_SPEAKER_VOLUME:
			BthAGSendEvent(EV_AG_HS_SPEAKER_GAIN, dwParam);
            DEBUGMSG(ZONE_PHONEUI, (L"BTAGSVC: *** Audio Gateway speaker volume has been updated to: %d ***\n", dwParam));
            break;

        case AG_PHONE_EVENT_MIC_VOLUME:
			BthAGSendEvent(EV_AG_HS_MIC_GAIN, dwParam);
            DEBUGMSG(ZONE_PHONEUI, (L"BTAGSVC: *** Audio Gateway mic volume has been updated to: %d ***\n", dwParam));
            break;

        case AG_PHONE_EVENT_BT_CTRL:
			BthAGSendEvent(EV_AG_HS_CTRL, dwParam);
#if 0
			if( dwParam == 0 ) // Close Ctrl Channel
			{
             vlineCloseModemPort();
			}
			else
			{
             vlineOpenModemPort();
			}
#endif
            DEBUGMSG(ZONE_PHONEUI, (L"BTAGSVC: *** Audio Gateway Bluetooth control connection has been updated to: %d ***\n", dwParam));
            break;

        case AG_PHONE_EVENT_BT_AUDIO:
			BthAGSendEvent(EV_AG_HS_AUDIO, dwParam);
            DEBUGMSG(ZONE_PHONEUI, (L"BTAGSVC: *** Audio Gateway Bluetooth audio connection has been updated to: %d ***\n", dwParam));
            break;

        default:
            DEBUGMSG(ZONE_PHONEUI, (L"BTAGSVC: Unknown PhoneExt event: %d param:%d.\n", dwEvent, dwParam));
    }

  RETAILMSG(1, (L"HS_DRV:-%S\r\n",__FUNCTION__));
}

// This function returns TRUE if the call should be taken by Bluetooth AG
BOOL BthAGOverrideCallOut(BOOL fHandsfree)
{
    HKEY hk;

    DWORD dwErr = RegOpenKeyEx(HKEY_LOCAL_MACHINE, RK_AUDIO_GATEWAY, 0, 0, &hk);
    if (dwErr == ERROR_SUCCESS) {
        DWORD cdwBytes = sizeof(DWORD);
        DWORD dwData = 0;
        
        dwErr = RegQueryValueEx(hk, _T("OutgoingCallOption"), 0, NULL, (PBYTE)&dwData, &cdwBytes);
        if (dwErr == ERROR_SUCCESS) {
            if (dwData == 1) {
                // Take in headset
                RegCloseKey(hk);
                return TRUE;
            }
            else if (dwData == 2) {
                // Take in handset
                RegCloseKey(hk);
                return FALSE;
            }
        }
    }

    RegCloseKey(hk);

    return (IDNO == MessageBox(NULL, L"Switch audio back to phone?", L"Bluetooth Headset", MB_YESNO | MB_ICONQUESTION | MB_SETFOREGROUND));
}

// This function returns TRUE if the call should be taken by Bluetooth AG
BOOL BthAGOverrideCallIn(BOOL fHandsfree)
{
    return TRUE;
}

// This function associates a phone number with a name (pszName is a buffer of size MAX_DISPLAY_NAME)
BOOL BthAGGetNameByPhoneNumber(LPCWSTR pwszNumber, LPWSTR pwszName)
{
    return FALSE;
}

// This function is called to get speed dial info for the specified hands-free memory index
BOOL BthAGGetSpeedDial(unsigned short usIndex, LPWSTR pwszNumber)
{
    return FALSE;
}

// This function is called to get the last dialed number on the phone (pszNumber is a buffer of size MAX_PHONE_NUMBER)
BOOL BthAGGetLastDialed(LPWSTR pwszNumber)
{
  //errno_t  errCode;
  RETAILMSG(1, (L"BTAGSVC:+%S, pwszNumber = %p\r\n",__FUNCTION__, pwszNumber));

    if( pwszNumber == NULL )
      return(FALSE);

 	// L">\"LD\"1"
	RETAILMSG(1, (L"BTAGSVC:%S, last-dial num = %s\r\n",__FUNCTION__, L">\"LD\"1"));
	wcscpy(pwszNumber, L">\"LD\"1" );

  RETAILMSG(1, (L"BTAGSVC:-%S, last-dial num = %s\r\n",__FUNCTION__, pwszNumber));
  return TRUE;
}

// This function registers callbacks for the state of the cell service.  The callback pfn must be called once immediately
// to set the initial service state and then only when there is a change in service state.
DWORD BthAGSetServiceCallback(PFN_PhoneExtServiceCallback pfn)
{
    PREFAST_ASSERT(pfn);
    pfn(TRUE); // We have cell service
    return ERROR_SUCCESS;
}


