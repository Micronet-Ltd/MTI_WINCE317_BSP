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
/*++

THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Module Name:  

	ndispwr.c

Abstract:  

	This module remembers the powre state of NDIS adapters set by
    ConnMC.  ConnMC calls IOCTL_NPW_SET_POWER to let us know what the
    power state of the adapter is.
    This module receives BIND notification from NDISUIO.
    It will then check the last set power state and issue that state
    to the adapter.
    

Author:


Environment:


Revision History:
    
    November 2001  :: Original version.

   
--*/

#include <windows.h>
#include <types.h>
#include <ntcompat.h>
#include <pm.h>
#include <ndis.h>
#include <windev.h>
#include <nuiouser.h>
#include <msgqueue.h>
#include <ndispwr.h>


//
//  Forward decl
//

BOOL
NpwStartNotification(LPVOID pUnused);

BOOL
SavePowerState(PWCHAR pwcAdapterName, CEDEVICE_POWER_STATE  CePowerState);

BOOL
GetSavedDevicePowerState(PTCHAR ptcDeviceName, PCEDEVICE_POWER_STATE pCePowerState);


//
//	Global vars..
//

CRITICAL_SECTION    g_GlobalCS;
HANDLE              g_hMsgQueue = NULL;
HANDLE              g_hNdisuio  = NULL;



//
//	Debug Zones.
//

#ifdef DEBUG

#define ZONE_INIT           DEBUGZONE(0)
#define ZONE_NDISUIO        DEBUGZONE(1)
#define ZONE_POWERSAVE      DEBUGZONE(2)
#define ZONE_ERROR          DEBUGZONE(15)

DBGPARAM	dpCurSettings = 
{
    TEXT("NdisPower"), 
	{
		TEXT("Init"),		
		TEXT("NdisUio"),
        TEXT("PowerSave"),
        TEXT("Undefined"),
		TEXT("Undefined"),
		TEXT("Undefined"),
		TEXT("Undefined"),
		TEXT("Undefined"),
		TEXT("Undefined"),
		TEXT("Undefined"),
		TEXT("Undefined"),
		TEXT("Undefined"),
		TEXT("Undefined"),		
		TEXT("Undefined"),		
		TEXT("Undefined"),
		TEXT("Error")
	},

    0x00008001
};



#endif


////////////////////////////////////////////////////////////////////////////////
//	DriverEntry()
//
//	Routine Description:
//
//
//	Arguments:
//	
//	
//	Return Value:
//	
//

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT   pDriverObject,
    IN PUNICODE_STRING  pRegistryPath)
{    
    return STATUS_SUCCESS;

}   //  DriverEntry()



////////////////////////////////////////////////////////////////////////////////
//	DllEntry()
//
//	Routine Description:
//
//		Implement dllentry() function of ndisuio driver under CE.
//
//	Arguments:
//	
//		hinstDLL	::	Instance pointer.
//		Op			::	Reason routine is called.
//		lpvReserved	::	System parameter.
//	
//	Return Value:
//	
//		TRUE / FALSE.
//

BOOL
DllEntry (HANDLE hinstDLL, DWORD Op, PVOID lpvReserved)
{
	BOOL Status = TRUE;

	switch (Op) 
	{
		case DLL_PROCESS_ATTACH:			

            // don't need thread attach/detach messages
            DisableThreadLibraryCalls (hinstDLL);

#ifdef DEBUG			            
			DEBUGREGISTER(hinstDLL);
#endif
			break;

		case DLL_PROCESS_DETACH:			
			break;

		case DLL_THREAD_DETACH :
			break;

		case DLL_THREAD_ATTACH :
			break;
			
		default :
			break;
	}
	return Status;

}	// DllEntry()



////////////////////////////////////////////////////////////////////////////////
//	NPW_Init()
//
//	Routine Description:
//
//		Called when we call RegisterDevice()
//
//	Arguments:
//		
//		Index:: Specifies a pointer to a string containing the registry path 
//					to the active key for the stream interface driver.
//	 
//	Return Value:
//
//		Handle to device context created.
//		This handle is passed to the XXX_Open and XXX_Deinit functions.
//

#define		VALID_DEVICE_CONTEXT	0xCE01CE01

DWORD 
NPW_Init (DWORD Index) 
{
    BOOL    bStatus = TRUE;

    do
    {
        HANDLE  hThread;
        
	    //
	    //	Init global vars used by this object..
	    //

	    InitializeCriticalSection (&g_GlobalCS);

        //
        //  Hook into NDISUIO and create thread for NDISUIO notification..
        //  Spin a thread to do that as we don't want to spend too long
        //  in the initialization slowing down the device boot up time.
        //
        
        hThread = CreateThread(
                        NULL,
                        0,
                        NpwStartNotification,
                        NULL,
                        0,
                        NULL);

        if (hThread == NULL)
            break;
            
        CloseHandle(hThread);
        
        return (DWORD)VALID_DEVICE_CONTEXT;
    }
    while (FALSE);


    //
    //  Something has gone wrong..   Bail out..
    //
    
    DeleteCriticalSection(&g_GlobalCS);   

    return (DWORD) -1;

}	//	NPW_Init()



////////////////////////////////////////////////////////////////////////////////
//	NPW_Deinit()
//
//	Routine Description:
//
//		Called when we call DeRegisterDevice()
//
//	Arguments:
//		
//		Handle to the device context. 
//		The XXX_Init function creates and returns this identifier.
//
//	Return Value:
//
//		TRUE indicates success. FALSE indicates failure.
//

BOOL 
NPW_Deinit(DWORD hDeviceContext) 
{
	DEBUGMSG (ZONE_INIT, (TEXT("UIO:: NPW_Deinit()..\r\n")));

    DeleteCriticalSection(&g_GlobalCS);
    
	return TRUE;

}	//	NPW_Deinit()



////////////////////////////////////////////////////////////////////////////////
//	NPW_Open()
//
//	Routine Description:
//
//		This function opens a device for reading and/or writing. 
//		An application indirectly invokes this function when it calls CreateFile
//		to open special device file names.
//
//	Arguments:
//		
//	hDeviceContext ::	Handle to the device context. The XXX_Init function 
//						creates and returns this handle. 
//	
//	dwAccessCode   ::   Specifies the requested access code of the device. The 
//						access is a combination of read and write. 
//
//	dwShareMode	   ::   Specifies the requested file share mode of the PC Card 
//						device. The share mode is a combination of file read 
//						and write sharing. 
//
//	Return Value:
//
//		This function returns a handle that identifies the open context of 
//		the device to the calling application
//		This identifier is passed into the XXX_Read, XXX_Write, XXX_Seek, and 
//		XXX_IOControl functions. 
//

DWORD 
NPW_Open (DWORD hDeviceContext , DWORD dwAccessCode , DWORD dwShareMode)
{
    //
    //  It does not matter..
    //

    return 0x01;	

}	//	NPW_Open()



////////////////////////////////////////////////////////////////////////////////
//	NPW_Close()
//
//	Routine Description:
//
//		Called when app calls closehandle(), or app being terminated..
//
//	Arguments:
//		
//		hOpenContext  :: Handle returned by the XXX_Open function, used to 
//						 identify the open context of the device. 
//
//	Return Value:
//
//		TRUE indicates success. FALSE indicates failure.
//

BOOL
NPW_Close (DWORD hOpenContext)  
{	
    return TRUE;
	
}	//	NPW_Close()



////////////////////////////////////////////////////////////////////////////////
//	NPW_Write()
//
//	Routine Description:
//
//		This function writes data to the device.
//
//	Arguments:
//		
//		hOpenContext    :: Handle returned by the XXX_Open function, used to 
//								identify the open context of the device. 
//
//		pSourceBytes    :: Pointer to the buffer that contains the data to write. 
//
//		dwNumberOfBytes :: Specifies the number of bytes to write from the 
//								pSourceBytes buffer into the device. 
//
//	Return Value:
//
//		The number of bytes written indicates success. 
//		A value of -1 indicates failure
//

DWORD
NPW_Write (DWORD hOpenContext , LPCVOID pSourceBytes , DWORD dwNumberOfBytes) 
{
    //
    //  Not supported..
    //

    return 0x00;

}	//	NPW_Write()



////////////////////////////////////////////////////////////////////////////////
//	NPW_Read()
//
//	Routine Description:
//
//		This function reads data from the device identified by the open context.
//
//	Arguments:
//		
//		hOpenContext    :: Handle returned by the XXX_Open function, used to 
//								identify the open context of the device. 
//
//		pBuffer         :: Pointer to the buffer which stores the data read 
//								from the device. This buffer should be at least 
//								dwCount bytes long. 
//
//		dwCount			:: Specifies the number of bytes to write from the 
//								pSourceBytes buffer into the device. 
//
//	Return Value:
//
//		The number of bytes written indicates success. 
//		A value of -1 indicates failure, 0 indicates time out.
//
//		GetLastError() will return:
//
//		STATUS_UNSUCCESSFUL			
//			- Another read is already blocking..
//			- Adapter not yet opened via: IOCTL_NDISNPW_OPEN_DEVICE
//			
//		STATUS_INVALID_HANDLE
//			- Invalid hOpenContext
//			- The adapter object has unbinded.
//
//

DWORD 
NPW_Read (DWORD hOpenContext, LPVOID pBuffer, DWORD dwCount) 
{
    //
    //  Not supported..
    //

    return 0x00;


}	//	NPW_Read



////////////////////////////////////////////////////////////////////////////////
//	NPW_Seek()
//
//	Routine Description:
//
//		This function moves the data pointer in the device.
//
//	Arguments:
//		
//		hOpenContext    :: Handle returned by the XXX_Open function, used to 
//								identify the open context of the device. 
//
//		lAmount			:: Specifies the number of bytes to move the data pointer 
//								in the device. A positive value moves the data 
//								pointer toward the end of the file, and a 
//								negative value moves it toward the beginning. 
//
//		wType			:: Specifies the starting point for the data pointer: 
//
//						   FILE_BEGIN 
//								Indicates that the starting point is 0 or the beginning of the file. 
//						   FILE_CURRENT 
//								Indicates that the current value of the file pointer is the starting point. 
//						   FILE_END 
//								Indicates that the current end-of-file position is the starting point. 
//
//	Return Value:
//
//		The device's new data pointer indicates success. A value of -1 indicates 
//		failure.
//

DWORD 
NPW_Seek (DWORD hOpenContext , long lAmount, WORD wType) 
{
    //
    //  Not supported..
    //

	return -1;

}	//	NPW_Seek()



////////////////////////////////////////////////////////////////////////////////
//	NPW_IOControl()
//
//	Routine Description:
//
//		This function sends a command to a device.
//
//	Arguments:
//		
//		None.
//
//	Return Value:
//
//		None.
//

BOOL 
NPW_IOControl(
	DWORD	hOpenContext,
	DWORD	dwCode, 
	PBYTE	pBufIn,
	DWORD	dwLenIn, 
	PBYTE	pBufOut, 
	DWORD	dwLenOut,
	PDWORD	pdwActualOut)
{	
    
    BOOL                     bStatus = FALSE;	
    PWCHAR                   pwcAdapterName = NULL;
    CEDEVICE_POWER_STATE     DevicePowerState;
    PNDISPWR_SAVEPOWERSTATE  pSavePowerState;
   
	switch (dwCode)
	{
		////////////////////////////////////////////////////////////////////////

        case IOCTL_NPW_SAVE_POWER_STATE:
        {
            DEBUGMSG (ZONE_POWERSAVE,
                (TEXT("NdisPower:: IOCTL_NPW_SAVE_POWER_STATE.\r\n")));

            if (dwLenIn != sizeof(NDISPWR_SAVEPOWERSTATE) || pBufIn == NULL)
            {
                SetLastError(ERROR_INVALID_PARAMETER);
                break;
            }

            pSavePowerState = (PNDISPWR_SAVEPOWERSTATE) pBufIn;            
       
            __try
            {
                //  Make a local copy of Adapter Name and power state.               

                DevicePowerState = pSavePowerState->CePowerState;

                if (S_OK != CeOpenCallerBuffer(&pwcAdapterName, (PVOID)pSavePowerState->pwcAdapterName, 0x00, ARG_I_WSTR, TRUE))
                {               
                    DEBUGMSG (ZONE_ERROR,
                        (TEXT("NdisPower:: Invalid argument in IOCTL_NPW_SAVE_POWER_STATE.\r\n")));
                    pwcAdapterName = NULL;
                }
            }
    		__except (EXCEPTION_EXECUTE_HANDLER)
            {
                DEBUGMSG (ZONE_ERROR,
                        (TEXT("NdisPower:: Invalid argument passed in IOCTL_NPW_SAVE_POWER_STATE.\r\n")));            
            }

            if (pwcAdapterName == NULL)
            {
                SetLastError(ERROR_INVALID_PARAMETER);
                break;
            }

            EnterCriticalSection(&g_GlobalCS);

            bStatus = SavePowerState(
                            pwcAdapterName, 
                            DevicePowerState);

            LeaveCriticalSection(&g_GlobalCS);

            if (pwcAdapterName)
                CeCloseCallerBuffer(pwcAdapterName, (PVOID)pSavePowerState->pwcAdapterName, 0x00, ARG_I_WSTR);
            
            break;
        }

        ////////////////////////////////////////////////////////////////////////

        case IOCTL_NPW_QUERY_SAVED_POWER_STATE:
        {
            PNDISPWR_SAVEPOWERSTATE  pSavedPowerState;

            if (dwLenOut != sizeof(NDISPWR_SAVEPOWERSTATE) || pBufOut == NULL)
            {
                SetLastError(ERROR_INVALID_PARAMETER);
                break;
            }

            pSavedPowerState = (PNDISPWR_SAVEPOWERSTATE) pBufOut;

            EnterCriticalSection(&g_GlobalCS);
            
            __try
            {    

                if (S_OK == CeOpenCallerBuffer(&pwcAdapterName, (PVOID)pSavedPowerState->pwcAdapterName, 0x00, ARG_I_WSTR, TRUE))
                {
                    //
                    //  The adapter power state may have never been set, so D0 can be assumed.
                    //                    
                    
                    pSavedPowerState->CePowerState = D0;
                    
                    if (GetSavedDevicePowerState(
                            pwcAdapterName, 
                            &DevicePowerState))
                    {
                        pSavedPowerState->CePowerState = DevicePowerState;
                    }              
                    
                    bStatus = TRUE;
                }
                else
                {
                    DEBUGMSG (ZONE_ERROR,
                        (TEXT("NdisPower:: Invalid argument in IOCTL_NPW_QUERY_SAVED_POWER_STATE.\r\n")));                                        
                }                    

            }
    		__except (EXCEPTION_EXECUTE_HANDLER)
            {   
                DEBUGMSG (ZONE_ERROR,
                        (TEXT("NdisPower:: Invalid argument passed in IOCTL_NPW_QUERY_SAVED_POWER_STATE.\r\n"))); 
            }

            LeaveCriticalSection(&g_GlobalCS);

            if (pwcAdapterName)
                CeCloseCallerBuffer(pwcAdapterName, (PVOID)pSavedPowerState->pwcAdapterName, 0x00, ARG_I_WSTR);

            if (!bStatus)
                SetLastError(ERROR_INVALID_PARAMETER);

            break;
        }     

        ////////////////////////////////////////////////////////////////////////
        

        default:
			break;
	}

    return bStatus;	

}	//	NPW_IOControl()



////////////////////////////////////////////////////////////////////////////////
//	SavePowerState()
//
//	Routine Description:
//
//		This function looks into the registry entry:
//      [HKLM\Comm\NdisPower\<Adapter_Name>].
//      If power state is PwrDeviceUnspecified then it will erase this entry 
//      (if entry exists) otherwise it will set the registry entry with this 
//      power state..
//      
//	Arguments:
//		
//		ptcDeviceName   :: The adapter instance name.
//      CePowerState    :: The power state of the adapter.
//
//	Return Value:
//
//		TRUE if successful, FALSE otherwise..
//

BOOL
SavePowerState(PWCHAR pwcAdapterName, CEDEVICE_POWER_STATE  CePowerState)
{
    HKEY    hkRoot  = NULL;
    BOOL    bStatus = FALSE;
    DWORD   dwErr;    
    
    do
    {
        //
        //  Open or Create the key if it is not already there..
        //

        dwErr = RegCreateKeyEx(
                    HKEY_LOCAL_MACHINE,
                    REG_NDISPOWER_ROOT,
                    0x00,
                    NULL,
                    0x00,
                    KEY_WRITE,
                    NULL,
                    &hkRoot,
                    NULL);

        if (dwErr != ERROR_SUCCESS)
        {
            DEBUGMSG (ZONE_ERROR,
                (TEXT("NdisPower:: Unable to open/create key [%s]\r\n"),
                REG_NDISPOWER_ROOT));

            break;        
        }


        //
        //  Then set to the non PwrDeviceUnspecified state..
        //

        if (CePowerState == PwrDeviceUnspecified)    
        {
            DEBUGMSG (ZONE_POWERSAVE,
                (TEXT("NdisPower:: Adapter was set to PwrDeviceUnspecified, deleting entry for [%s].\r\n"),
                pwcAdapterName));

            RegDeleteValue(
                hkRoot,
                pwcAdapterName);
        }
        else
        {
            DEBUGMSG (ZONE_POWERSAVE,
                (TEXT("NdisPower:: Saving power state [%s] for adapter [%s].\r\n"),
                (CePowerState == D0) ? TEXT("D0") :
                (CePowerState == D1) ? TEXT("D1") :
                (CePowerState == D2) ? TEXT("D2") :
                (CePowerState == D3) ? TEXT("D3") :
                (CePowerState == D4) ? TEXT("D3") :
                (CePowerState == PwrDeviceUnspecified) ? TEXT("PwrDeviceUnspecified") : TEXT("Unknown!"),
                pwcAdapterName));

            RegSetValueEx(
                hkRoot,
                pwcAdapterName,
                0x00,
                REG_DWORD,
                (PBYTE)&CePowerState,
                sizeof(DWORD));
        }


        bStatus = TRUE;        
    }

    while (FALSE);

    if (hkRoot)
        RegCloseKey(hkRoot);

    return bStatus;

}   //  SavePowerState()



////////////////////////////////////////////////////////////////////////////////
//	GetSavedDevicePowerState()
//
//	Routine Description:
//
//		This function looks into the registry entry:
//      [HKLM\Comm\NdisPower\<Adapter_Name>] for the remembered power state of 
//      the device.   
//      
//	Arguments:
//		
//		ptcDeviceName   :: The adapter instance name.
//      CePowerState    :: The power state of the adapter.
//
//	Return Value:
//
//		TRUE if entry is found, FALSE otherwise..
//

BOOL
GetSavedDevicePowerState(PTCHAR ptcDeviceName, PCEDEVICE_POWER_STATE pCePowerState)
{
    HKEY    hkRoot = NULL;
    BOOL    bStatus = FALSE;
    DWORD   dwBufferSize, dwType, dwErr;

    do
    {
        dwErr = RegOpenKeyEx(
                    HKEY_LOCAL_MACHINE,
                    REG_NDISPOWER_ROOT,
                    0,
                    KEY_READ,
                    &hkRoot);

        if (dwErr == ERROR_FILE_NOT_FOUND)
        {
            //
            //  No go..
            //

            DEBUGMSG(ZONE_NDISUIO,
                (TEXT("NdisPower:: Reg does not contain [%s]\r\n"),
                REG_NDISPOWER_ROOT));

            break;
        }


        dwBufferSize = sizeof(CEDEVICE_POWER_STATE);

        dwErr = RegQueryValueEx(
                    hkRoot,
                    ptcDeviceName,
                    0,
                    &dwType,
                    (LPBYTE)pCePowerState,
                    &dwBufferSize);

        if (dwErr != ERROR_SUCCESS || dwType != REG_DWORD)
        {
            DEBUGMSG (ZONE_NDISUIO,
                (TEXT("NdisPower:: Reg does not contain entry for [%s]..\r\n"),
                ptcDeviceName));

            break;
        }

        //
        //  Everything is OKAY..
        //

        DEBUGMSG (ZONE_NDISUIO,
            (TEXT("NdisPower:: Adapter [%s] - Saved Power State [%s].\r\n"),
            ptcDeviceName,
            (*pCePowerState == D0) ? TEXT("D0") :
            (*pCePowerState == D1) ? TEXT("D1") :
            (*pCePowerState == D2) ? TEXT("D2") :
            (*pCePowerState == D3) ? TEXT("D3") :
            (*pCePowerState == D4) ? TEXT("D4") :
            (*pCePowerState == PwrDeviceUnspecified) ? TEXT("PwrDeviceUnspecified") :            
            TEXT("UNKNOWN!")));

        bStatus = TRUE;

    }
    while (FALSE);

    if (hkRoot)
        RegCloseKey(hkRoot);

    return bStatus;

}   //  GetSavedDevicePowerState()



////////////////////////////////////////////////////////////////////////////////
//  DoNdisIOControl()
//
//  Routine Description:
//
//      This function sends the dwCommand IOCTL to NDIS.
//
//
//  Return Value:
//
//      TRUE if successful, FALSE otherwise..
//

BOOL 
DoNdisIOControl(
        DWORD   dwCommand, 
        LPVOID  pInBuffer,
        DWORD   cbInBuffer, 
        LPVOID  pOutBuffer,
        DWORD   *pcbOutBuffer)
{
   HANDLE   hNdis;
   BOOL     bResult = FALSE;

   hNdis =  CreateFile(
                DD_NDIS_DEVICE_NAME, 
                GENERIC_READ | GENERIC_WRITE,
                FILE_SHARE_READ | FILE_SHARE_WRITE, 
                NULL, 
                OPEN_ALWAYS,
                0, 
                NULL);

   if (INVALID_HANDLE_VALUE != hNdis)
   {
      bResult = DeviceIoControl(
                    hNdis, 
                    dwCommand, 
                    pInBuffer, 
                    cbInBuffer,
                    pOutBuffer, 
                    (pcbOutBuffer ? *pcbOutBuffer : 0x00),
                    pcbOutBuffer, 
                    NULL);
                    
        CloseHandle(hNdis);
   }

   return bResult;
   
}



////////////////////////////////////////////////////////////////////////////////
//	SetAdapterPowerState()
//
//	Routine Description:
//
//		This function looks into the registry entry:
//      [HKLM\Comm\NdisPower\<Adapter_Name>] for the remembered power state of 
//      the device.   If entry is not found, do nothing.
//  
//      Up to CE 4.0, the PCMCIA power on for network adapters emulates 
//      card ejection and re-insertion.
//      Thus the power state of the device is not remembered by Power Manager.
//      Hence this module reinstate the state..
//      
//	Arguments:
//		
//		ptcDeviceName   :: The adapter instance name.
//
//	Return Value:
//
//		TRUE if the adapter is disabled, FALSE otherwise.
//

BOOL
SetAdapterPowerState(PTCHAR ptcDeviceName, HANDLE hEthManQueue)
{   
    
    CEDEVICE_POWER_STATE    CePowerState;
    BOOL                    bSetDisabled = FALSE;
	NDISUIO_DEVICE_NOTIFICATION		sDeviceNotification;
	BOOL							bOk;
    //
    //  See if there is a saved device power state, if so set it.
    //

    if (GetSavedDevicePowerState(ptcDeviceName, &CePowerState))
    {
        //
        //  If it is not PwrDeviceUnspecified then issue the power state through PM..
        //  We do this regardless of whether the actual adapter is power manageable.
        //  Failure is okay, we'll unbind it just like connmc.exe does.
        //  

        if (CePowerState != PwrDeviceUnspecified)
        {
            TCHAR szName[MAX_PATH];
            int   nChars;

            RETAILMSG (1, (TEXT("NdisPower::: Disabling adapter [%s]\r\n"),
                ptcDeviceName));

            nChars = _sntprintf(
                        szName, 
                        MAX_PATH-1, 
                        _T("%s\\%s"), 
                        PMCLASS_NDIS_MINIPORT, 
                        ptcDeviceName);


            if (nChars != -1)
            {
                DEBUGMSG (ZONE_NDISUIO,
                    (TEXT("NdisPower:: Setting adapter [%s] to Power State [%s].\r\n"),
                    ptcDeviceName,
                    (CePowerState == D0) ? TEXT("D0") :
                    (CePowerState == D1) ? TEXT("D1") :
                    (CePowerState == D2) ? TEXT("D2") :
                    (CePowerState == D3) ? TEXT("D3") :
                    (CePowerState == D4) ? TEXT("D4") :
                    (CePowerState == PwrDeviceUnspecified) ? TEXT("PwrDeviceUnspecified") :                    
                    TEXT("UNKNOWN!")));

                SetDevicePower(szName, POWER_NAME, CePowerState);

            }

            //
            //  IOCTL_NDIS_UNBIND_ADAPTER takes multisz (protocol driver names
            //  follow the adapter name) hence we need extra NULL.
            //
            
            memset (szName, 0x00, sizeof(szName));
            StringCchCopyW(szName, MAX_PATH-1, ptcDeviceName);
            DoNdisIOControl(
                IOCTL_NDIS_UNBIND_ADAPTER, 
                szName,
                (_tcslen(ptcDeviceName)+2) * sizeof(TCHAR),
                NULL, 
                NULL); 

            bSetDisabled = TRUE;
            
			sDeviceNotification.dwNotificationType = NDISUIO_NOTIFICATION_UNBIND;
			_tcsncpy(sDeviceNotification.ptcDeviceName, ptcDeviceName, MAX_PATH - 1);
			bOk = WriteMsgQueue(
					hEthManQueue, 
					&sDeviceNotification,
    				sizeof(NDISUIO_DEVICE_NOTIFICATION),
					0,
					0);

			RETAILMSG(1, (TEXT("NdisPower:: %S, WriteMsgQueue (UNBIND) for %s = %d\r\n"), __FUNCTION__, sDeviceNotification.ptcDeviceName, bOk));
        }
    }

	if (!bSetDisabled)
	{
		sDeviceNotification.dwNotificationType = NDISUIO_NOTIFICATION_BIND;
		_tcsncpy(sDeviceNotification.ptcDeviceName, ptcDeviceName, MAX_PATH - 1);
		bOk = WriteMsgQueue(
				hEthManQueue, 
				&sDeviceNotification,
				sizeof(NDISUIO_DEVICE_NOTIFICATION),
				0,
				0);

		RETAILMSG(1, (TEXT("NdisPower:: %S, WriteMsgQueue (BIND) for %s = %d\r\n"), __FUNCTION__, sDeviceNotification.ptcDeviceName, bOk));
	}

    return bSetDisabled;
    
}   //  SetAdapterPowerState()



////////////////////////////////////////////////////////////////////////////////
//	NpwNotificationThread()
//
//	Routine Description:
//
//		The thread that handles notification from NDISUIO..
//
//	Arguments:
//		
//		None.
//
//	Return Value:
//
//		None.
//

DWORD
NpwNotificationThread(LPVOID param)
{
	NDISUIO_DEVICE_NOTIFICATION		sDeviceNotification;
	DWORD							dwBytesReturned;
	DWORD							dwFlags;
	HANDLE							hEthManQueue = (HANDLE)param;
	BOOL							bOk;

    //
    //  Wait till PM is ready..  or NDIS has registered its 
    //  adapters..
    //

    while (WAIT_OBJECT_0 != WaitForAPIReady(SH_SHELL, 500))
            ; 

    while (WaitForSingleObject(g_hMsgQueue, INFINITE) == WAIT_OBJECT_0)
    {
    	while (ReadMsgQueue(
    				g_hMsgQueue,
    				&sDeviceNotification,
    				sizeof(NDISUIO_DEVICE_NOTIFICATION),
    				&dwBytesReturned,
    				1,
    				&dwFlags))
    	{
    		//
    		//	Okay, we have notification..  
    		//
    		
    		DEBUGMSG(ZONE_NDISUIO,
    			(TEXT("NdisPower:: Notification:: Event [%s] \t Adapter [%s]\r\n"),				
    			(sDeviceNotification.dwNotificationType & NDISUIO_NOTIFICATION_RESET_START)
    				?	TEXT("RESET_START")			:
    			(sDeviceNotification.dwNotificationType & NDISUIO_NOTIFICATION_RESET_END)
    				? 	TEXT("RESET_END")			:
    			(sDeviceNotification.dwNotificationType & NDISUIO_NOTIFICATION_MEDIA_CONNECT)
    				?	TEXT("MEDIA_CONNECT")		:
    			(sDeviceNotification.dwNotificationType & NDISUIO_NOTIFICATION_MEDIA_DISCONNECT)
    				?	TEXT("MEDIA_DISCONNECT")	:
    			(sDeviceNotification.dwNotificationType & NDISUIO_NOTIFICATION_BIND)
    				?	TEXT("BIND")					:
    			(sDeviceNotification.dwNotificationType & NDISUIO_NOTIFICATION_UNBIND)
    				?	TEXT("UNBIND")                  :
    			TEXT("Unknown!"),
    			sDeviceNotification.ptcDeviceName));


			//				
			//	This is what we do to the notification:
			//

            if (sDeviceNotification.dwNotificationType & NDISUIO_NOTIFICATION_BIND)
            {
                //
                //  Give NDIS some times to bind all the protocols to this adapter
                //  before we issue unbind if necessary...
                //

                RETAILMSG (1, 
                    (TEXT("NdisPower:: BIND notification for adapter [%s]\r\n"),
                    sDeviceNotification.ptcDeviceName));
                
                //Sleep(2000);
			    SetAdapterPowerState(sDeviceNotification.ptcDeviceName, hEthManQueue);

			}
			else if (sDeviceNotification.dwNotificationType & NDISUIO_NOTIFICATION_UNBIND)
			{
				bOk = WriteMsgQueue(
					hEthManQueue, 
					&sDeviceNotification,
    				sizeof(NDISUIO_DEVICE_NOTIFICATION),
					0,
					0);

				RETAILMSG(!bOk, (TEXT("NdisPower:: %S, WriteMsgQueue (UNBIND) for %s = %d\r\n"), __FUNCTION__, sDeviceNotification.ptcDeviceName, bOk));
			}

		}	
	
	}	

	return 0x00;

}	//	NpwNotificationThread



////////////////////////////////////////////////////////////////////////////////
//	NpwStartNotification()
//
//	Routine Description:
//
//		This function register with NDISUIO and request for notification..
//
//	Arguments:
//		
//		None.
//
//	Return Value:
//
//		TRUE if successful, FALSE otherwise..
//

BOOL
NpwStartNotification(LPVOID pUnused)
{
    #define	NOTIFICATION_REQUIRED           \
    			NDISUIO_NOTIFICATION_BIND | NDISUIO_NOTIFICATION_UNBIND

    //
    //	CE does not have WMI, so NDISUIO's IOCTL_NDISUIO_REQUEST_NOTIFICATION
    //	handles it for us..
    //	Once created, we never shut down, since we don't have the SCM 
    //	equivalent..
    //	

    MSGQUEUEOPTIONS					sOptions;			
    NDISUIO_REQUEST_NOTIFICATION	sRequestNotification;
    HANDLE							hThread;
    BOOL							bStatus = FALSE;
	HANDLE							hEthManQueue = NULL;

	sOptions.dwSize           = sizeof(MSGQUEUEOPTIONS);
    sOptions.dwFlags          = MSGQUEUE_ALLOW_BROKEN;
    sOptions.dwMaxMessages    = 0;
    sOptions.cbMaxMessage     = sizeof(NDISUIO_DEVICE_NOTIFICATION);
    sOptions.bReadAccess      = FALSE;

    hEthManQueue = CreateMsgQueue(_T("NDISPWR_QUEUE"), &sOptions);
    RETAILMSG(hEthManQueue == NULL, (TEXT("NdisPower:: Could not create message queue for notifying Ethman\r\n")));

    //
    //  Wait till PM is ready..  or NDIS has registered its 
    //  adapters..
    //

    while (WAIT_OBJECT_0 != WaitForAPIReady(SH_SHELL, 500))
            ; 

    do
    {
        //
        //  First stop is to get a handle to NDISUIO..
        //

        DWORD   dwErr = ERROR_SUCCESS;

        char                    Buffer[1024];
        PNDISUIO_QUERY_BINDING  pQueryBinding;
        DWORD                   dwOutSize, i = 0x00;

        //
        //	Note the Desired Access is zero.. 
        //	This handle can only do Write/Query.
        //

        g_hNdisuio = CreateFile(
                        (PTCHAR)NDISUIO_DEVICE_NAME,					//	Object name.
                        0x00,											//	Desired access.
                        0x00,											//	Share Mode.
                        NULL,											//	Security Attr
                        OPEN_EXISTING,									//	Creation Disposition.
                        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,	//	Flag and Attributes..
                        (HANDLE)INVALID_HANDLE_VALUE);	

        if (g_hNdisuio == INVALID_HANDLE_VALUE)
        {
            dwErr = GetLastError();
            DEBUGMSG (ZONE_ERROR,
                (TEXT("NdisPower:: Unable to open [%s]\r\n"),
                NDISUIO_DEVICE_NAME));

            break;
        }


        //
        //  Check out the interfaces that have been instantiated before
        //  we get launched... (IOCTL_NDISUIO_REQUEST_NOTIFICATION will only
        //  give notification for devices arriving in the future).
        //

        do
        {
            PTCHAR  pAdapterName;
            
            pQueryBinding = (PNDISUIO_QUERY_BINDING)&Buffer[0];
            
            memset(pQueryBinding, 0x00, sizeof(NDISUIO_QUERY_BINDING));
                        
            pQueryBinding->BindingIndex = i;
            
            if (!DeviceIoControl(
                    g_hNdisuio,
                    IOCTL_NDISUIO_QUERY_BINDING,
                    pQueryBinding,
                    sizeof(NDISUIO_QUERY_BINDING),
                    pQueryBinding,
                    1024,
                    &dwOutSize,
                    NULL))
            {
                break;
            }
            
            //
            //  Make sure it's terminated..
            //

            pAdapterName = (PTCHAR)&Buffer[pQueryBinding->DeviceNameOffset];
            pAdapterName[(pQueryBinding->DeviceNameLength / sizeof(TCHAR)) - 1] = 0x00;

            RETAILMSG (1,
                (TEXT("NdisPower:: Found adapter [%s]\r\n"),
                pAdapterName));

            //
            //  If the adapter was unbind, then this will change the 
            //  NDISUIO internal queue, and we may hence miss potentially miss 
            //  out the next adapter.
            //  So if SetAdapterPowerState() decides to disable the adapter
            //  then we will reset our count.
            //
            
            if (SetAdapterPowerState(pAdapterName, hEthManQueue))
            {
                i = 0x00;                 
            }            
            else
            {
                i++;
            } 
        } 
        while(TRUE);

		//
		//	Then create the msg queue..
		//

		sOptions.dwSize					= sizeof(MSGQUEUEOPTIONS);
		sOptions.dwFlags				= 0;
		sOptions.dwMaxMessages			= 4;
		sOptions.cbMaxMessage			= sizeof(NDISUIO_DEVICE_NOTIFICATION);
		sOptions.bReadAccess			= TRUE;

		g_hMsgQueue = CreateMsgQueue(NULL,&sOptions);

		if (g_hMsgQueue == NULL)				
		{
			DEBUGMSG (ZONE_ERROR, 
				(TEXT("NdisPower:: Error CreateMsgQueue()..\r\n")));
			break;
		}
		

		//
		//	Queue created successfully, tell NDISUIO about it..			
		//

		sRequestNotification.hMsgQueue				= g_hMsgQueue;
		sRequestNotification.dwNotificationTypes	= NOTIFICATION_REQUIRED;

		if (!DeviceIoControl(
				g_hNdisuio,
				IOCTL_NDISUIO_REQUEST_NOTIFICATION,
				&sRequestNotification,
				sizeof(NDISUIO_REQUEST_NOTIFICATION),
				NULL,
				0x00,
				NULL,
				NULL))
		{
			DEBUGMSG (ZONE_ERROR,
				(TEXT("NdisPower:: Err IOCTL_NDISUIO_REQUEST_NOTIFICATION\r\n")));
			break;
		}

		//
		//	NDISUIO takes it well, now party on it..
		//

		hThread = CreateThread(
						NULL,
						0,
						NpwNotificationThread,
						hEthManQueue,
						0,
						NULL);

        CloseHandle(hThread);

		//
		//	Everything is cool..
		//

		DEBUGMSG (ZONE_INIT,
			(TEXT("NdisPower:: Successfully register for notification!\r\n")));

        bStatus = TRUE;

	}
	while (FALSE);


    if (!bStatus)
    {
        if (g_hMsgQueue)
            CloseMsgQueue(g_hMsgQueue);

        if (g_hNdisuio)
            CloseHandle(g_hNdisuio);

		if (hEthManQueue)
			CloseMsgQueue(hEthManQueue);
    }

	return bStatus;

}	//	NpwStartNotification
