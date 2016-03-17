//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this source code is subject to the terms of the Microsoft end-user
// license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
// If you did not accept the terms of the EULA, you are not authorized to use
// this source code. For a copy of the EULA, please see the LICENSE.RTF on your
// install media.
//
// Portions Copyright (c) Texas Instruments.  All rights reserved.
//
//------------------------------------------------------------------------------
//
//  File: bkldrvmain.c
//
//  Backlight driver source code
//
#include <windows.h>
#include <windev.h>
#include <pnp.h>
#include <pm.h>
#include <strsafe.h>
#include "..\backlight_class.h"
#include "BKLi.h"
#include "..\bl.h"
#include <light_sense.h>

// disable PREFAST warning for use of EXCEPTION_EXECUTE_HANDLER
#pragma warning (disable: 6320)

DBGPARAM dpCurSettings = {
    TEXT("backlight"), {
        TEXT("Backlight"), TEXT("Function"), TEXT("Misc"), TEXT(""),
        TEXT(""), TEXT(""), TEXT(""), TEXT(""),
        TEXT(""), TEXT(""), TEXT(""), TEXT(""),
        TEXT(""), TEXT(""), TEXT("Warning"), TEXT("Error"),
    },
    0xC003
};

extern GPIO_Backlight s_Backlight;


#define BKL_EVENT_REG            0       // registry change
#define BKL_EVENT_POWER_MSG      1       // power status change
#define BKL_EVENT_EXIT           2       // we're exiting
#define BKL_EVENT_DISPLAY_MSG    3       // display device notification
#define BKL_EVENT_LSENS			 4		//must be last - light sensor msq
#define BKL_NUM_EVENTS           5

#define TURNOFFIMMEDIATELY -1

// device notification queue parameters
#define PNP_QUEUE_ENTRIES       1       // assumes we have only 1 display driver interface being advertised
//"The notifications sent to hMsgQ are a sequence of DEVDETAIL structures 
//with extra TCHAR types appended to account for the instance names":
#define PNP_QUEUE_SIZE          (PNP_QUEUE_ENTRIES * (sizeof(DEVDETAIL) + (MAX_NAMELEN * sizeof(TCHAR))))

static const UCHAR   DeviceStateMasks[5]=
{
    DX_MASK(D0),
    DX_MASK(D1),
    DX_MASK(D2),
    DX_MASK(D3),
    DX_MASK(D4),
};
DWORD GetTimeout(BKL_MDD_INFO *pBKLinfo);

BOOL ConvertStringToGuid(LPCTSTR GuidString, GUID *Guid )
{
    // ConvertStringToGuid
    // this routine converts a string into a GUID and returns TRUE if the
    // conversion was successful.

    // Local variables.
    UINT Data4[8];
    int  Count;
    BOOL Ok = FALSE;
    LPWSTR GuidFormat = L"{%08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}";

    DEBUGCHK(Guid != NULL && GuidString != NULL);

    __try 
    {

      if (_stscanf(GuidString, GuidFormat, &Guid->Data1, 
        &Guid->Data2, &Guid->Data3, &Data4[0], &Data4[1], &Data4[2], &Data4[3], 
        &Data4[4], &Data4[5], &Data4[6], &Data4[7]) == 11) 
      {

          for (Count = 0; Count < (sizeof(Data4) / sizeof(Data4[0])); Count++) 
          {
              Guid->Data4[Count] = (UCHAR) Data4[Count];
          }
      }

      Ok = TRUE;
    }
    __except(EXCEPTION_EXECUTE_HANDLER) 
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("exception in convertstringtoguid\r\n")));
    }

    return Ok;
}




void UpdateACStatus(BKL_MDD_INFO *pBKLinfo)
{
    SYSTEM_POWER_STATUS_EX2 SysPower;

    static fFirstTime = TRUE;

    // make sure that GWES APIs ready before calling: 
    if (WAIT_OBJECT_0 != WaitForAPIReady(SH_GDI, INFINITE))
    {
        RETAILMSG(ZONE_ERROR, (TEXT("Backlight driver: WaitForAPIReady failed.\r\n")));
        return;
    }

    if (pBKLinfo->pfnGetSystemPowerStatusEx2) 
    {
        if ((*pBKLinfo->pfnGetSystemPowerStatusEx2)(&SysPower, sizeof(SysPower), FALSE))
        {
            if (SysPower.ACLineStatus & AC_LINE_ONLINE)
            {
                pBKLinfo->fOnAC = TRUE;
            }
            else
            {
                pBKLinfo->fOnAC = FALSE;
            }
        }
        else
        {
            DEBUGMSG(ZONE_BACKLIGHT, (TEXT("GetSystemPowerStstusEx2 failed with error 0x%x.\r\n"), GetLastError()));
            
        }
    }
    else 
    {
        // There are no battery APIs so assume that we are always on AC power.
        pBKLinfo->fOnAC = TRUE;
    }
    
    return;    
    
}
/* 
    A driver that is issued a request to enter a power state not supported by its device 
    enters the next available power state supported. 
    For example, if the Power Manager requests that it enter D2 and does not support D2, 
    the device can enter D3 or D4 instead. 
    If a device is requested to enter D3 and cannot wake up the system then 
    it should enter D4 and power off rather than staying in standby

    All drivers must support at least D0

    Note: since the default PDA power manager will never ask a driver to go to a state
    that the driver has reported it supports, most of this code will never be called.
    The code could be made more efficient (no branching) if deemed necessary

*/
BOOL GetBestSupportedState(BKL_MDD_INFO *pBKLinfo, CEDEVICE_POWER_STATE ReqDx, CEDEVICE_POWER_STATE* SetDx)
{
    BOOL fRet = TRUE;   // assume there's a suitable state we can go to
    
    switch(ReqDx)
    {
        case D0:
            *SetDx = D0;
            break;
        case D1:
            if(pBKLinfo->ucSupportedStatesMask & DeviceStateMasks[D1])
            {
                *SetDx = D1;
            }            
            else if(pBKLinfo->ucSupportedStatesMask & DeviceStateMasks[D2])
            {
                *SetDx = D2;
            }
            else if(pBKLinfo->ucSupportedStatesMask & DeviceStateMasks[D3])
            {
                *SetDx = D3;
            }
            else if(pBKLinfo->ucSupportedStatesMask & DeviceStateMasks[D4])
            {
                *SetDx = D4;
            }
            else
            {
                fRet = FALSE;
            }
            break;
        
        case D2:
            if(pBKLinfo->ucSupportedStatesMask & DeviceStateMasks[D2])
            {
                *SetDx = D2;
            }
            else if(pBKLinfo->ucSupportedStatesMask & DeviceStateMasks[D3])
            {
                *SetDx = D3;
            }
            else if(pBKLinfo->ucSupportedStatesMask & DeviceStateMasks[D4])
            {
                *SetDx = D4;
            }                       
            else
            {
                fRet = FALSE;
            }
            break;

        case D3:
            if(pBKLinfo->ucSupportedStatesMask & DeviceStateMasks[D3])
            {
                *SetDx = D3;
            }
            else if(pBKLinfo->ucSupportedStatesMask & DeviceStateMasks[D4])
            {
                *SetDx = D4;
            }
            else
            {
                fRet = FALSE;
            }
            break;
        case D4:
            if(pBKLinfo->ucSupportedStatesMask & DeviceStateMasks[D4])
            {
                *SetDx = D4;
            }
            else
            {
                fRet = FALSE;
            }
            break;  
        default:
            ASSERT(FALSE);
            break;
            
    }   

    return fRet;
}

/*
    Has the user checked the 'turn on when key pressed...' option for the current power status?
*/
BOOL IsTapOn(BKL_MDD_INFO *pBKLinfo)
{
    if(pBKLinfo->fOnAC)
    {
        return (pBKLinfo->fExternalTapOn? TRUE : FALSE);
    }
    else
    {
        return (pBKLinfo->fBatteryTapOn? TRUE : FALSE);
    }
    
}

DWORD GetTimeout(BKL_MDD_INFO *pBKLinfo)
{
    if(pBKLinfo->fOnAC)
    {
        return pBKLinfo->dwACTimeout;        
    }
    else
    {
        return pBKLinfo->dwBattTimeout;        
    }

}

/*
    Reads the 'turn on when key pressed...' registry settings from the registry
*/
void BacklightUpdateMDDRegSettings(BKL_MDD_INFO *pBKLinfo)
{
    DWORD   retCode;
    BYTE    ValueData[MAX_PATH];
    DWORD    dwType;
    void    *bData = ValueData;
    DWORD   cbData;
    HKEY    hKey;

    DEBUGMSG(ZONE_BACKLIGHT,(TEXT("+BacklightReadMDDReg\r\n")));
    retCode = RegOpenKeyEx (HKEY_CURRENT_USER, BACKLIGHT_REGKEY, 0, KEY_ALL_ACCESS, &hKey);
    if (retCode == ERROR_SUCCESS)
    {
        //Battery Tap
        dwType=REG_DWORD;
        cbData = MAX_PATH;
        retCode = RegQueryValueEx(hKey, TEXT("BacklightOnTap"), NULL, &dwType, (LPBYTE) bData, (LPDWORD)&cbData);
        if (retCode == ERROR_SUCCESS)
        {
            pBKLinfo->fBatteryTapOn = (*(DWORD *)bData );
        }
        //External Tap
        dwType=REG_DWORD;
        cbData = MAX_PATH;
        retCode = RegQueryValueEx(hKey, TEXT("ACBacklightOnTap"), NULL, &dwType, (LPBYTE) bData, (LPDWORD)&cbData);
        if (retCode == ERROR_SUCCESS)
        {
            pBKLinfo->fExternalTapOn = (*(DWORD *)bData );
        }
        //Backlight on battery timeout (we may need to turn the backlight off)
        dwType=REG_DWORD;
        cbData = MAX_PATH;
        retCode = RegQueryValueEx(hKey, TEXT("UseBattery"), NULL, &dwType, (LPBYTE) bData, (LPDWORD)&cbData);
		if(retCode == ERROR_SUCCESS)
		{
			if( 0 == *(DWORD*)bData )
			{
				pBKLinfo->dwBattTimeout = INFINITE;
				retCode = ERROR_SUCCESS;
			}
			else
				retCode = 1;
		}	
		
		if(retCode != ERROR_SUCCESS)
		{
	        dwType=REG_DWORD;
		    cbData = MAX_PATH;        
			retCode = RegQueryValueEx(hKey, TEXT("BatteryTimeout"), NULL, &dwType, (LPBYTE) bData, (LPDWORD)&cbData);
			if (retCode == ERROR_SUCCESS)
			{
				pBKLinfo->dwBattTimeout = (*(DWORD *)bData );
			}
		}
		
        //Backlight on AC timeout
        dwType=REG_DWORD;
        cbData = MAX_PATH;        
        retCode = RegQueryValueEx(hKey, TEXT("UseExt"), NULL, &dwType, (LPBYTE) bData, (LPDWORD)&cbData);
		if(retCode == ERROR_SUCCESS)
		{
			retCode = 1;
			if(0 == *(DWORD*)bData)
			{	
				pBKLinfo->dwACTimeout	= INFINITE;
				retCode					= ERROR_SUCCESS;			
			}
		}	
		
		if(retCode != ERROR_SUCCESS)
		{
	        dwType=REG_DWORD;
		    cbData = MAX_PATH;        
			retCode = RegQueryValueEx(hKey, TEXT("ACTimeout"), NULL, &dwType, (LPBYTE) bData, (LPDWORD)&cbData);
			if (retCode == ERROR_SUCCESS)
			{
				pBKLinfo->dwACTimeout = (*(DWORD *)bData );
			}
		}
		
        dwType=REG_DWORD;
        cbData = MAX_PATH;        
        retCode = RegQueryValueEx(hKey, BACKLIGHT_REG_LS, NULL, &dwType, (LPBYTE) bData, (LPDWORD)&cbData);
        if (retCode == ERROR_SUCCESS)
        {
            pBKLinfo->fDependOnLight = (*(DWORD *)bData );
        }
     }

    if(hKey)
    {
        RegCloseKey(hKey);
    }
	//advbacklight
    // Read device parameters
	s_Backlight.UpdateRegParams();

    DEBUGMSG(ZONE_BACKLIGHT,(TEXT("-BacklightReadMDDReg\r\n")));
    
}

/*
    Ask power manager to set the BKL power state.
    PM will not ask BKL to change states again until
    called with PwrDeviceUnspecified
*/
void BKL_SetDevicePower(BKL_MDD_INFO *pBKLinfo, CEDEVICE_POWER_STATE bklPowerState)
{
	SetDevicePower(pBKLinfo->szName, POWER_NAME, bklPowerState);
    
    return;
}

// Update the backlight reg settings or power status
// Set power requirment for backlight to D4 (off) if 'Tap On' setting unchecked
// and unspecified if 'Tap On' setting just checked. Similarly if 'Turn Off Backlight' timeout option
// has just been selected/deselected:
void UpdateBacklight(BKL_MDD_INFO *pBKLinfo, DWORD dwReason)
{
 /*   BOOL    fTapOnPrev, fTapOnNew;
    DWORD   dwTimeoutPrev, dwTimeoutNew;
    BOOL    fSetPowerOff = FALSE;
    BOOL    fReleasePwrOff = FALSE;
    
    fTapOnPrev = IsTapOn(pBKLinfo);
    dwTimeoutPrev = GetTimeout(pBKLinfo);
*/
    switch(dwReason)
    {
        case BKL_EVENT_REG:
            // Have registry keys changed?
            BacklightUpdateMDDRegSettings(pBKLinfo);  
            break;

        case BKL_EVENT_POWER_MSG:
            // AC Status changed?
            UpdateACStatus(pBKLinfo);
            break;
    };
  /*  
    dwTimeoutNew = GetTimeout(pBKLinfo);
    fTapOnNew = IsTapOn(pBKLinfo);
    
    // 'Tap On' settings just unchecked or 'Turn off backlight' timer option just selected:
    fSetPowerOff = ( ((dwTimeoutPrev != TURNOFFIMMEDIATELY) && (dwTimeoutNew == TURNOFFIMMEDIATELY))
                    || ((fTapOnPrev && !fTapOnNew)));
    // 'Tap On' setting just checked or 'Turn off backlight' timer option just deselected:
    fReleasePwrOff = (((dwTimeoutPrev == TURNOFFIMMEDIATELY) && (dwTimeoutNew != TURNOFFIMMEDIATELY) ) 
                    || (!fTapOnPrev && fTapOnNew));
    
    if(fSetPowerOff)   
    {
        BKL_SetDevicePower(pBKLinfo, D4);
    }
    else if (fReleasePwrOff) 
    {
        BKL_SetDevicePower(pBKLinfo, PwrDeviceUnspecified);
    }    
*/
}
/*

Monitors changes in reg keys 'BacklightOnTap' and 'ACBacklightOnTap'
and changes in power status (AC->DC / DC->AC)

*/
DWORD fnBackLightThread(PVOID pvArgument)
{ 
    DWORD    dwResult;
	MSGQUEUEOPTIONS msgopts	= {0};
    HANDLE  hPwrNotification = NULL;
    HANDLE  hPowerNotificationMsgs = NULL;    
	HANDLE  WaitEvents[BKL_NUM_EVENTS]	= {0};    
    HANDLE  hEventRegistryChange = NULL;
    DWORD   dwSize;    
    DWORD   dwFlags;
    HANDLE  hDisplayNotifications = NULL;
    HANDLE  hDisplayNotificationMsgs = NULL;
    GUID    idInterface;
    BKL_MDD_INFO *pBKLinfo = NULL;
    
	MSGQUEUEOPTIONS		msgLs				= {0};
    HANDLE				hLsenseNotifyMsgs	= 0;    
	HANDLE				LsDevice			= 0;
	TCHAR				MsqLsName[LIGHTSENSE_CONFIG_LEN/2]	= {0};
	DWORD				BklNumEvents		= 0;

    DEBUGMSG(ZONE_BACKLIGHT,(TEXT("+fnBackLightRegThread\r\n")));

    // Verify context
    if(! pvArgument)
    {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: BacklightThread: "
            L"Incorrect context paramer\r\n" ));

        return FALSE;
    }

    pBKLinfo = (BKL_MDD_INFO*) pvArgument;


    // create msg queue for power status change notification (AC->Battery and vice versa)
    memset(&msgopts, 0, sizeof(msgopts));
    msgopts.dwSize = sizeof(msgopts);
    msgopts.dwFlags = 0;
    msgopts.dwMaxMessages = 0;                  // no max number of messages
    msgopts.cbMaxMessage = sizeof(POWER_BROADCAST);   // max size of each msg
    msgopts.bReadAccess = TRUE;
    hPowerNotificationMsgs = CreateMsgQueue(NULL, &msgopts);

    if (!hPowerNotificationMsgs) 
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("BKL: Create message queue failed\r\n")));
        goto exit;
    }
    // request notification of power status changes:
    hPwrNotification = RequestPowerNotifications(hPowerNotificationMsgs, PBT_POWERSTATUSCHANGE);
    if (!hPwrNotification) 
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("BKL: register power notification failed\r\n")));
        goto exit;
    }

    // create msg queue for display device interface notifications:
    msgopts.cbMaxMessage = PNP_QUEUE_SIZE;
    hDisplayNotificationMsgs = CreateMsgQueue(NULL, &msgopts);
    if (!hDisplayNotificationMsgs) 
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("BKL: Create message queue failed\r\n")));
        goto exit;
    }

    if(!ConvertStringToGuid(PMCLASS_DISPLAY, &idInterface)) 
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("can't convert PMCLASS_DISPLAY string to GUID\r\n")));  
        goto exit;
    }

    // get display driver name (required to keep display driver on with SetPowerRequirement):
    if(!(hDisplayNotifications = RequestDeviceNotifications(&idInterface, hDisplayNotificationMsgs, TRUE)))
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("RequestDeviceNotifications failed\r\n")));
        goto exit;
    }    
    
   // Request notification of backlight registry changes:
    hEventRegistryChange = CreateEvent(NULL, FALSE, FALSE, _T("BackLightChangeEvent") );

	if(INVALID_HANDLE_VALUE == hEventRegistryChange)
    {   
        //DEBUGMSG(ZONE_ERROR, (TEXT("BKL: CeFindFirstRegChange failed\r\n")));        
        DEBUGMSG(ZONE_ERROR, (TEXT("BKL: EVENTNAME_BACKLIGHTCHANGEEVENT failed\r\n")));        
        goto exit;
    }
 ///
	    // create msg queue for power status change notification (AC->Battery and vice versa)
    msgLs.dwSize		= sizeof(MSGQUEUEOPTIONS);
    msgLs.dwFlags		= 0;
    msgLs.dwMaxMessages = 0;                  // no max number of messages
    msgLs.cbMaxMessage	= sizeof(AMBIENT_LIGHT);   // max size of each msg
    msgLs.bReadAccess	= TRUE;

	LsDevice = LightSenseOpen( MsqLsName, sizeof(MsqLsName) ); 
    
	RETAILMSG(0, (TEXT("BKL: LightSenseOpen  %s\r\n"), MsqLsName ));

	if( !LsDevice )
	{
       RETAILMSG(1, (TEXT("BKL: LightSenseOpen failed\r\n")));
	}

	if(LsDevice)
	{
		hLsenseNotifyMsgs	= CreateMsgQueue(MsqLsName, &msgLs);

		if (!hLsenseNotifyMsgs) 
		{
			RETAILMSG(1, (TEXT("BKL: Create LightSensor message queue failed\r\n")));
			goto exit;
		}
	}

///
    BacklightUpdateMDDRegSettings(pBKLinfo);
    UpdateACStatus(pBKLinfo);
	s_Backlight.UpdateDutyCycles( pBKLinfo->fOnAC, ( pBKLinfo->fDependOnLight ? pBKLinfo->LSLevel : AMBIENT_LIGHT_HIGH ) );
	s_Backlight.SetPowerState(pBKLinfo->dwCurState);

    WaitEvents[BKL_EVENT_REG] = hEventRegistryChange;
    BklNumEvents++;
	WaitEvents[BKL_EVENT_POWER_MSG] = hPowerNotificationMsgs;
    BklNumEvents++;
    WaitEvents[BKL_EVENT_EXIT] = pBKLinfo->hExitEvent;
    BklNumEvents++;
    WaitEvents[BKL_EVENT_DISPLAY_MSG] = hDisplayNotificationMsgs;
    BklNumEvents++;
    if( hLsenseNotifyMsgs )
	{
		WaitEvents[BKL_EVENT_LSENS]	= hLsenseNotifyMsgs;
		BklNumEvents++;
	}

    pBKLinfo->fExit = FALSE;
    
    while(!pBKLinfo->fExit)
    {
        dwResult = WaitForMultipleObjects(BklNumEvents, &WaitEvents[0], FALSE, INFINITE);
        switch(dwResult)
        {
            case(WAIT_OBJECT_0 + BKL_EVENT_REG):
            {
                RETAILMSG(0,(TEXT("Backlight mdd registry change\r\n")));

                UpdateBacklight(pBKLinfo, BKL_EVENT_REG);
				
				s_Backlight.UpdateDutyCycles( pBKLinfo->fOnAC, ( pBKLinfo->fDependOnLight ? pBKLinfo->LSLevel : AMBIENT_LIGHT_HIGH ) );
				s_Backlight.SetPowerState(pBKLinfo->dwCurState);

                // Tell PDD that the backlight reg settings changed:
//MYS                 BacklightRegChanged( pBKLinfo->dwPddContext );
            }
            break;
            
            case (WAIT_OBJECT_0 + BKL_EVENT_POWER_MSG):
            {
                POWER_BROADCAST PwrMsgBuf;
                
                DEBUGMSG(ZONE_BACKLIGHT,(TEXT("Power status change to/from AC\r\n")));
                if (!ReadMsgQueue(hPowerNotificationMsgs, &PwrMsgBuf, sizeof(PwrMsgBuf), &dwSize, 0, &dwFlags)) 
                {
                    DEBUGMSG(ZONE_BACKLIGHT,(TEXT("ReadMsgQueue failed\r\n")));  
                    ASSERT(FALSE);
                }

                UpdateBacklight(pBKLinfo, BKL_EVENT_POWER_MSG);
                
				s_Backlight.UpdateDutyCycles(  pBKLinfo->fOnAC, ( pBKLinfo->fDependOnLight ? pBKLinfo->LSLevel : AMBIENT_LIGHT_HIGH ) );
DEBUGMSG(ZONE_BACKLIGHT,(TEXT("Power status change %s\r\n"),pBKLinfo->fOnAC ? L"to AC" :L"to Batt"));

                // tell PDD that power source changed:
//MYS                BacklightPwrSrcChanged(pBKLinfo->fOnAC, pBKLinfo->dwPddContext );
               s_Backlight.SetPowerState(pBKLinfo->dwCurState);

                
            }
            break;

            case (WAIT_OBJECT_0 + BKL_EVENT_DISPLAY_MSG):
            {
                union 
                {
                    UCHAR deviceBuf[PNP_QUEUE_SIZE];
                    DEVDETAIL devDetail;
                } u;
                
                DEBUGMSG(ZONE_BACKLIGHT,(TEXT("Display driver interface notification\r\n")));
                if (!ReadMsgQueue(hDisplayNotificationMsgs, u.deviceBuf, PNP_QUEUE_SIZE, &dwSize, 0, &dwFlags)) 
                {
                    DEBUGMSG(ZONE_BACKLIGHT,(TEXT("ReadMsgQueue failed\r\n")));  
                    ASSERT(FALSE);
                }
                else if(dwSize >= sizeof(DEVDETAIL)) 
                {
                    PDEVDETAIL pDevDetail = &u.devDetail;

                    if (pDevDetail->cbName < 0)
                    {
                        DEBUGMSG(ZONE_ERROR,(TEXT("Invalid cbName value\r\n")));                        
                    }
                    else if(( (pDevDetail->cbName + sizeof(TCHAR) < sizeof(pBKLinfo->szDisplayInterface) ) 
                        && ( (int)(pDevDetail->cbName + sizeof(TCHAR)) > pDevDetail->cbName) ))
                    {
                        memcpy(pBKLinfo->szDisplayInterface, pDevDetail->szName, pDevDetail->cbName);
                        // make sure it's null terminated:
                        pBKLinfo->szDisplayInterface[pDevDetail->cbName] = '\0';
                    } 
                }
                else
                {
                    DEBUGMSG(ZONE_BACKLIGHT,(TEXT("insufficient buffer for device message\r\n")));  
                }
            }
            break;
            case (WAIT_OBJECT_0 + BKL_EVENT_LSENS):
            {
				if( !hLsenseNotifyMsgs )//error
					break;

				AMBIENT_LIGHT lsdata = AMBIENT_LIGHT_UNSPEC;
				RETAILMSG(0,(TEXT("BKL: Light sensor notification\r\n")));
                if (!ReadMsgQueue(hLsenseNotifyMsgs, &lsdata, sizeof(AMBIENT_LIGHT), &dwSize, 0, &dwFlags)) 
                {
					RETAILMSG(1,(TEXT("BKL: ReadMsgQueue lsense failed\r\n")));  
                    ASSERT(FALSE);
                }
                else if(dwSize < sizeof(AMBIENT_LIGHT)	||
						AMBIENT_LIGHT_LOW	> lsdata			||
						AMBIENT_LIGHT_HIGH	< lsdata			) 
                {
					RETAILMSG(1,(TEXT("BKL: ReadMsgQueue lsense - wrong data\r\n")));  
				}
				else//all wright
				{
					pBKLinfo->LSLevel = lsdata;
					RETAILMSG(0,(TEXT("BKL: ReadMsgQueue lsdata = %d, fDependOnLight = %d\r\n"), lsdata, pBKLinfo->fDependOnLight ));  
					if( pBKLinfo->fDependOnLight )
					{
						s_Backlight.UpdateDutyCycles(  pBKLinfo->fOnAC, pBKLinfo->LSLevel );
						if( D0 == pBKLinfo->dwCurState )
							s_Backlight.SetPowerState(pBKLinfo->dwCurState);
					}
				}
 			}
			break;
            case(WAIT_OBJECT_0 + BKL_EVENT_EXIT):
            {
                DEBUGMSG(ZONE_BACKLIGHT,(TEXT("Backlight exiting\r\n")));                
            }
            break;
            
            default:
            {
                ASSERT(FALSE);
            }
            
        }
        
    }

    exit:
        
    if (hPwrNotification)
    {
        StopPowerNotifications(hPwrNotification);
    }
    if (hPowerNotificationMsgs)
    {
        CloseMsgQueue(hPowerNotificationMsgs);
    }
    if (hDisplayNotifications)
    {
        StopDeviceNotifications(hDisplayNotifications);
    }    
    if (hDisplayNotificationMsgs)
    {
        CloseMsgQueue(hDisplayNotificationMsgs);
    }
    if(hEventRegistryChange)
    {
		CloseHandle(hEventRegistryChange);
    }
    if(hLsenseNotifyMsgs)
		CloseMsgQueue(hLsenseNotifyMsgs);

	if( LsDevice )
		LightSenseClose(LsDevice);
    
    DEBUGMSG(ZONE_BACKLIGHT,(TEXT("-fnBackLightRegThread\r\n")));

    return 0;
}

