//#include "stdafx.h"
#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include <service.h>
#include <MicUserSdk.h>
//#include "ag_hs_api.h"
#include "bt.h"
#include "Mixer.h"
//#include <initguid.h>
//#include <bt_sdp.h>
//#include <bt_api.h>

HANDLE         ghAgHS = NULL; 

static BOOL stabilizeService(DWORD dwService);


BOOL findLastPaired(BT_ADDR *bt)
{
    DWORD dwRetVal = ERROR_SUCCESS;
    HKEY hk = NULL;
    int dwIdx = 0;
    WCHAR szName[MAX_PATH];
    DWORD cchName = ( sizeof(szName)/sizeof(szName[0]) );

    dwRetVal = RegOpenKeyEx(HKEY_LOCAL_MACHINE, RK_AUDIO_GATEWAY_DEVICES, 0, 0, &hk);
    if (ERROR_SUCCESS != dwRetVal) 
	{
		RETAILMSG(1, (L"DIGVOICE:%S, Could not open registry key for BT Addr: %d.\r\n",__FUNCTION__, dwRetVal));
        return(FALSE);
    }

    // Enumerate devices in registry    
    while (ERROR_SUCCESS == RegEnumKeyEx(hk, dwIdx, szName, &cchName, NULL, NULL, NULL, NULL)) 
	{
        HKEY hkAddr;
        BT_ADDR btAddr = 0;
        GUID service;
        DWORD cdwSize;
        
        dwRetVal = RegOpenKeyEx(hk, szName, 0, 0, &hkAddr);
        if (ERROR_SUCCESS != dwRetVal) {
			RETAILMSG(1, (L"BTAGSVC:%S, Could not open enum registry key for BT Addr: %d.\r\n",__FUNCTION__, dwRetVal));
            break;
        }
        
        cdwSize = sizeof(btAddr);
        dwRetVal = RegQueryValueEx(hkAddr, L"Address", NULL, NULL, (PBYTE)&btAddr, &cdwSize);
        if (dwRetVal != ERROR_SUCCESS) {
			RETAILMSG(1, (L"BTAGSVC:%S, Could not open query registry value for BT Addr: %d.\r\n",__FUNCTION__, dwRetVal));
            RegCloseKey(hkAddr);
            break;
        }

        cdwSize = sizeof(service);
        dwRetVal = RegQueryValueEx(hkAddr, L"Service", NULL, NULL, (PBYTE)&service, &cdwSize);
        if (dwRetVal != ERROR_SUCCESS) {
			RETAILMSG(1, (L"BTAGSVC:%S, Could not open query registry value for service: %d.\r\n",__FUNCTION__, dwRetVal));
            RegCloseKey(hkAddr);
            break;
        }        

        RegCloseKey(hkAddr);

        WCHAR* p;
        int iDeviceIdx = wcstol(szName, &p, 10);
        if (btAddr != 0 && ((iDeviceIdx >= 1) || (iDeviceIdx <= 4))) {
            RETAILMSG(1, (L"BTAGSVC:%S, Setting btAddr at index %d to %04x%08x.\r\n",__FUNCTION__, iDeviceIdx, GET_NAP(btAddr), GET_SAP(btAddr))); 
            *bt = btAddr;
            //memcpy(&pDevices[iDeviceIdx-1].service, &service, sizeof(GUID));
        }

        cchName = ( sizeof(szName)/sizeof(szName[0]) );
        dwIdx++;        
    }

    RegCloseKey(hk);

	return(TRUE);
}


HANDLE   btAudioOpen(BT_ADDR btAddr)
{
 LONG          iRet;
 //DWORD         ret;
 BOOL          bRet = FALSE;
 DWORD         AgServiceStatus;

 RETAILMSG(1, (L"BT:+%S\r\n",__FUNCTION__));

  iRet = MIC_AGHSControl(AG_SERVICE_STATUS, &AgServiceStatus);
  if( iRet != AG_HS_OK )
  {
	  RETAILMSG(1, (L"BT:%S, Ag Service Control Failure, ret = %d, last err = %d\r\n",__FUNCTION__, iRet, GetLastError() ));
  }
  else
  {
	RETAILMSG(1, (L"BT:%S, Ag Service Status = 0x%X\r\n",__FUNCTION__, AgServiceStatus));
  }

  switch(AgServiceStatus)
  {
    case  SERVICE_STATE_OFF:
	case  SERVICE_STATE_UNINITIALIZED:
		{
          iRet = MIC_AGHSControl(AG_SERVICE_START, &AgServiceStatus);
		} break;

	case  SERVICE_STATE_ON:
		{
         iRet = AG_HS_OK;
		} break;

	case  SERVICE_STATE_STARTING_UP:
	case  SERVICE_STATE_SHUTTING_DOWN:
	case  SERVICE_STATE_UNLOADING:
		{
		  bRet = stabilizeService(AgServiceStatus);
		  iRet = -1;
		} break;
	case  SERVICE_STATE_UNKNOWN:
	default:
		{
         iRet = -1;   
		} break; 

  }

  if( bRet == TRUE )
  {
   iRet = MIC_AGHSControl(AG_SERVICE_START, &AgServiceStatus);
  }
 
  if( iRet == AG_HS_OK )
  {
   iRet = MIC_AGHSOpen(&ghAgHS, btAddr);
  }

 
  if( iRet != AG_HS_OK )
  {
    RETAILMSG(1, (L"BT:-%S, Ag Service ret = %d, Status = 0x%X\r\n",__FUNCTION__, iRet, AgServiceStatus));
    iRet = MIC_AGHSClose(ghAgHS);
	ghAgHS = NULL;
	return(NULL);
  }
  else
  {
    // register events
     iRet = MIC_AGHSEventRegistering(ghAgHS, EV_AG_ALL_MASK);
     RETAILMSG(1, (L"BT:-%S, Ag Service ret = %d, Status = 0x%X\r\n",__FUNCTION__, iRet, AgServiceStatus));
	 //bRet = turnBTPlbOnOff(TRUE);
	 return(ghAgHS);
  }
  
}


BOOL   btAudioClose(void)
{
  //DWORD         AgServiceStatus;
  //BOOL          bRet;
  LONG          iRet;

  RETAILMSG(1, (L"BT:+%S\r\n",__FUNCTION__));

  //bRet = turnBTPlbOnOff(FALSE);

  if( ghAgHS != NULL )
  {
   iRet = MIC_AGHSClose(ghAgHS);
   ghAgHS = NULL; 
  }
  
  RETAILMSG(1, (L"BT:-%S, ret = %d\r\n",__FUNCTION__, iRet));

  return(TRUE);
}


BOOL   btAudioDisconnect(DWORD hsConnect)
{
  LONG          iRet;
  DWORD         AgServiceStatus;

  RETAILMSG(1, (L"BT:+%S\r\n",__FUNCTION__));

  if( ghAgHS == NULL )
  {
   RETAILMSG(1, (L"BT:-%S, AG handle is NULL\r\n",__FUNCTION__));
   return(TRUE);
  }

  RETAILMSG(1, (L"BT:%S, Audio Off\r\n",__FUNCTION__));
  iRet = MIC_AGHSControl(AG_AUDIO_OFF, &AgServiceStatus);

#if 0
  RETAILMSG(1, (L"BT:%S, Control Off\r\n",__FUNCTION__));
  iRet = MIC_AGHSControl(AG_DISCONNECT, &AgServiceStatus);

  RETAILMSG(1, (L"BT:%S, Service Stop:\r\n",__FUNCTION__));
  iRet = MIC_AGHSControl(AG_SERVICE_STOP, &AgServiceStatus);
  RETAILMSG(1, (L"BT:%S, Service Sart:\r\n",__FUNCTION__));
  iRet = MIC_AGHSControl(AG_SERVICE_START, &AgServiceStatus);
#endif

#if 0
  iRet = MIC_AGHSControl(AG_DISCONNECT, &AgServiceStatus);
  //iRet = MIC_AGHSControl(AG_SERVICE_STOP, &AgServiceStatus);
#endif

  // because bug in MS Audio gareway service !
  // In the case of HS profile, if we close Audio Channel,
  // The Control Channel will be closed too.
  // Despite of their docs!
  // For opening it likes Ok.
  // So, re-open Control Channel !
  //Sleep(30);

#if 0  
  iRet = MIC_AGHSControl(AG_SERVICE_STATUS, &AgServiceStatus);
  RETAILMSG(1, (L"BT:%S, Service Status = %d\r\n",__FUNCTION__, AgServiceStatus));
#endif

#if 0  
  RETAILMSG(1, (L"BT:%S, Control On\r\n",__FUNCTION__));
  iRet = MIC_AGHSControl(AG_CONNECT, &AgServiceStatus);
  if( iRet != 0 )
  {
   RETAILMSG(1, (L"BT:%S, Control On Again\r\n",__FUNCTION__));
   Sleep(200);
   iRet = MIC_AGHSControl(AG_CONNECT, &AgServiceStatus);
  }
#endif

  RETAILMSG(1, (L"BT:-%S, ret = %d\r\n",__FUNCTION__, iRet));
  return(TRUE);
}


// Turn On Audio only, Control (ALC) channel should be On now
BOOL   btAudioConnect(DWORD hsConnect)
{
 LONG          iRet;
 DWORD         AgServiceStatus, dwErr;
 BOOL          bRet = TRUE;

  RETAILMSG(1, (L"BT:+%S\r\n",__FUNCTION__));

  if( ghAgHS == NULL )
  {
   RETAILMSG(1, (L"BT:%S, AG handle is NULL\r\n",__FUNCTION__));
   return(FALSE);
  }

  iRet = MIC_AGHSControl(AG_SERVICE_STATUS, &AgServiceStatus);
  if( iRet != AG_HS_OK )
  {
	RETAILMSG(1, (L"BT:%S, Ag Service Control Failure, ret = %d\r\n",__FUNCTION__, iRet));
  }
  else
  {
	RETAILMSG(1, (L"BT:%S, Ag Service Status = 0x%X\r\n",__FUNCTION__, AgServiceStatus));
  }

  if( AgServiceStatus != SERVICE_STATE_ON )
  {
   RETAILMSG(1, (L"BT:%S, AG invalid state\r\n",__FUNCTION__));
   return(FALSE);
  }

#if 0
   iRet = MIC_AGHSControl(AG_CONNECT, &AgServiceStatus);
#endif

   if( iRet == AG_HS_OK )
   {
    
    if( hsConnect & BT_AG_HS_ACL )
	{
     iRet = MIC_AGHSControl(AG_CONNECT, &AgServiceStatus);
	 if( iRet != AG_HS_OK ) // May be already connected
	 {
       dwErr = GetLastError();
	   if( dwErr != ERROR_ALREADY_INITIALIZED )
         bRet = FALSE; 
	 }

	 if( bRet == TRUE )
	 {
	   RETAILMSG(1, (L"BT:%S, ACL connection OK\r\n",__FUNCTION__));
	 }
	}

	if( (bRet == TRUE) && ( hsConnect & BT_AG_HS_SCO ) )
	{
		iRet = MIC_AGHSControl(AG_AUDIO_ON, &AgServiceStatus);
		if( iRet == AG_HS_OK )
		{
		 RETAILMSG(1, (L"BT:-%S\r\n",__FUNCTION__));
		 bRet = TRUE;
		}
		else
		{
         dwErr = GetLastError();
	     if( dwErr != ERROR_ALREADY_INITIALIZED )
          bRet = FALSE; 
		}

	  if( bRet == TRUE )
	  {
	   RETAILMSG(1, (L"BT:%S, SCO connection OK\r\n",__FUNCTION__));
	  }
	}

   }
 
 RETAILMSG(1, (L"BT:-%S, bRet = %d\r\n",__FUNCTION__, bRet));
 return(bRet);

}


BOOL  btSetSpkVol(USHORT vol)
{
 LONG          iRet;
 DWORD         AgServiceStatus;
 USHORT        vol2;

  if( vol > 15 )
   return(FALSE);

  iRet = MIC_AGHSControl(AG_SERVICE_STATUS, &AgServiceStatus);
  if( iRet != AG_HS_OK )
  {
	RETAILMSG(1, (L"BT:%S, Ag Service Control Failure, ret = %d\r\n",__FUNCTION__, iRet));
	return(FALSE);
  }
  else
  {
	RETAILMSG(1, (L"BT:%S, Ag Service Status = 0x%X, vol = %d\r\n",__FUNCTION__, AgServiceStatus, vol));
  }

  if( AgServiceStatus != SERVICE_STATE_ON )
  {
   RETAILMSG(1, (L"BT:%S, AG invalid state\r\n",__FUNCTION__));
   return(FALSE);
  }

  iRet = MIC_AGHSControl(AG_SET_SPEAKER_VOL, &vol);
  if( iRet != AG_HS_OK )
  {
	RETAILMSG(1, (L"BT:%S, 'AG_SET_SPEAKER_VOL' Failure, ret = %d\r\n",__FUNCTION__, iRet));
	return(FALSE);
  }

  // Verify
  iRet = MIC_AGHSControl(AG_GET_SPEAKER_VOL, &vol2);
  if( iRet != AG_HS_OK )
  {
	RETAILMSG(1, (L"BT:%S, verify 'AG_GET_SPEAKER_VOL' Failure, ret = %d\r\n",__FUNCTION__, iRet));
	return(FALSE);
  }

  if( vol != vol2 )
  {
	RETAILMSG(1, (L"BT:%S, 'AG_GET_SPEAKER_VOL' Failure, gain was not set, vol2 = %d\r\n",__FUNCTION__, vol2));
	return(FALSE);
  }

  return(TRUE);

}


BOOL  btSetMicVol(USHORT aVol)
{
 LONG          iRet;
 DWORD         AgServiceStatus;
 USHORT        vol2;
 USHORT        vol;

  vol = aVol;

  if( vol > 15 )
   return(FALSE);

  iRet = MIC_AGHSControl(AG_SERVICE_STATUS, &AgServiceStatus);
  if( iRet != AG_HS_OK )
  {
	RETAILMSG(1, (L"BT:%S, Ag Service Control Failure, ret = %d\r\n",__FUNCTION__, iRet));
	return(FALSE);
  }
  else
  {
	RETAILMSG(1, (L"BT:%S, Ag Service Status = 0x%X, vol = %d\r\n",__FUNCTION__, AgServiceStatus, vol));
  }

  if( AgServiceStatus != SERVICE_STATE_ON )
  {
   RETAILMSG(1, (L"BT:%S, AG invalid state\r\n",__FUNCTION__));
   return(FALSE);
  }

  iRet = MIC_AGHSControl(AG_SET_MIC_VOL, &vol);
  if( iRet != AG_HS_OK )
  {
	RETAILMSG(1, (L"BT:%S, 'AG_GET_MIC_VOL' Failure, ret = %d\r\n",__FUNCTION__, iRet));
	return(FALSE);
  }

  // Verify
  iRet = MIC_AGHSControl(AG_GET_MIC_VOL, &vol2);
  if( iRet != AG_HS_OK )
  {
	RETAILMSG(1, (L"BT:%S, verify 'AG_GET_MIC_VOL' Failure, ret = %d\r\n",__FUNCTION__, iRet));
	return(FALSE);
  }

  if( vol != vol2 )
  {
	RETAILMSG(1, (L"BT:%S, 'AG_GET_MIC_VOL' Failure, gain was not set, vol2 = %d\r\n",__FUNCTION__, vol2));
	return(FALSE);
  }


  return(TRUE);

}


BOOL  btGetSpkVol(USHORT *pVol)
{
 LONG          iRet;
 DWORD         AgServiceStatus;
 
  if( pVol == NULL )
   return(FALSE);

  iRet = MIC_AGHSControl(AG_SERVICE_STATUS, &AgServiceStatus);
  if( iRet != AG_HS_OK )
  {
	RETAILMSG(1, (L"BT:%S, Ag Service Control Failure, ret = %d\r\n",__FUNCTION__, iRet));
	return(FALSE);
  }
  else
  {
	RETAILMSG(1, (L"BT:%S, Ag Service Status = 0x%X\r\n",__FUNCTION__, AgServiceStatus));
  }

  if( AgServiceStatus != SERVICE_STATE_ON )
  {
   RETAILMSG(1, (L"BT:%S, AG invalid state\r\n",__FUNCTION__));
   return(FALSE);
  }

  iRet = MIC_AGHSControl(AG_GET_SPEAKER_VOL, pVol);
  if( iRet != AG_HS_OK )
  {
	RETAILMSG(1, (L"BT:%S, 'AG_GET_SPEAKER_VOL' Failure, ret = %d\r\n",__FUNCTION__, iRet));
	return(FALSE);
  }
  else
  {
   RETAILMSG(1, (L"BT:%S, 'AG_GET_SPEAKER_VOL' Ok, vol = %d\r\n",__FUNCTION__, *pVol));
   return(TRUE);
  }

}


BOOL  btGetMicVol(USHORT *pVol)
{
 LONG          iRet;
 DWORD         AgServiceStatus;
 
  if( pVol == NULL )
   return(FALSE);

  iRet = MIC_AGHSControl(AG_SERVICE_STATUS, &AgServiceStatus);
  if( iRet != AG_HS_OK )
  {
	RETAILMSG(1, (L"BT:%S, Ag Service Control Failure, ret = %d\r\n",__FUNCTION__, iRet));
	return(FALSE);
  }
  else
  {
	RETAILMSG(1, (L"BT:%S, Ag Service Status = 0x%X\r\n",__FUNCTION__, AgServiceStatus));
  }

  if( AgServiceStatus != SERVICE_STATE_ON )
  {
   RETAILMSG(1, (L"BT:%S, AG invalid state\r\n",__FUNCTION__));
   return(FALSE);
  }

  iRet = MIC_AGHSControl(AG_GET_MIC_VOL, pVol);
  if( iRet != AG_HS_OK )
  {
	RETAILMSG(1, (L"BT:%S, 'AG_GET_MIC_VOL' Failure, ret = %d\r\n",__FUNCTION__, iRet));
	return(FALSE);
  }
  else
  {
    RETAILMSG(1, (L"BT:%S, 'AG_GET_MIC_VOL' Ok, vol = %d\r\n",__FUNCTION__, iRet, *pVol));
    return(TRUE);
  }

}


BOOL  isBTAbsent(void)
{
 DWORD         AgServiceStatus;

 RETAILMSG(1, (L"BT:+%S\r\n",__FUNCTION__));

  LONG  iRet = MIC_AGHSControl(AG_SERVICE_STATUS, &AgServiceStatus);

  RETAILMSG(1, (L"BT:+%S, ret = %d, status = %d\r\n",__FUNCTION__, iRet, AgServiceStatus));

  if( ( iRet == AG_HS_MODULE_ABSENT )|| (iRet != AG_HS_OK) )
   return(TRUE);
  else
   return(FALSE);
}


static BOOL stabilizeService(DWORD dwServiceState)
{
  DWORD    tempState;
  DWORD    AgServiceStatus;
  INT32    iRet;

  RETAILMSG(1, (L"BT:+%S\r\n",__FUNCTION__));

  if( ( dwServiceState == SERVICE_STATE_STARTING_UP )||
      ( dwServiceState == SERVICE_STATE_SHUTTING_DOWN )||
      ( dwServiceState == SERVICE_STATE_UNLOADING )
	)
  {
   tempState = dwServiceState;
   Sleep(3000);

	  iRet = MIC_AGHSControl(AG_SERVICE_STATUS, &AgServiceStatus);
	  if( iRet != AG_HS_OK )
	  {
		RETAILMSG(1, (L"BT:%S, Ag Service Control Failure, ret = %d\r\n",__FUNCTION__, iRet));
		return(FALSE);
	  }
	  else
	  {
		RETAILMSG(1, (L"BT:%S, Ag Service Status = 0x%X\r\n",__FUNCTION__, AgServiceStatus));
	  }

    if( AgServiceStatus == tempState )
	{
     iRet = MIC_AGHSControl(AG_SERVICE_STOP, &AgServiceStatus);
	 RETAILMSG(1, (L"BT:%S, Ag Service is stucking, stop it\r\n",__FUNCTION__));
	 return(FALSE);
	}
  }

  RETAILMSG(1, (L"BT:-%S\r\n",__FUNCTION__));
  return(TRUE);
}