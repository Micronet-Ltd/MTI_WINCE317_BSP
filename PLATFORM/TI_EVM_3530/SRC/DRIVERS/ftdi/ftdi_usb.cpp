/*++

Copyright (c) 2001-2012  Future Technology Devices International Ltd.

Module Name:

    ftdi_ser.c

Abstract:

    Native USB device driver for FTDI FT8U2XX
    Serial port routines

Environment:

    kernel mode only

Revision History:

	24/05/04	st		Adapted from Microsoft sample
	04/08/04	st		Added support to get the VID and PID from the INF file
	13/08/04	st		Dual chip support added. Can register diffent VID and PID devices.
	08/10/04	st		Assigns proper interface number to the devices
	25/11/04	st		Unique registry entries for serialed devices. Prefix code added. Version number to registry added.
	10/12/04	st		Added default Prefix - so we will install if no INF file. Changed version number for registry.
	10/03/05	st		Made static calls to the RegisterClientDriverID etc.
	22/03/05	st		Changes to support 232R variable IN endpoint size.
	01/06/05	st		Changed registry stuff to allow new settings in persistant registry to be copied over.
	07/06/05	st		Added notify signal in DeviceNotify. Removed set Flags.Open to FALSE in DeviceNotify function.
	23/06/05	st		Multiple VIDs and PIDs change. Added support for unique EmulationMode setting.
	27/06/05	st		ActiveSync Monitor thread changes.
	30/06/05	st		Regsitry reads for configurable bulk flags and transfer type (async/sync).
	05/08/05	st		Removed unneccesary stop of Bulk thread in the Notify routine.
	17/08/05	st		Change required to notify the serial driver that an unplug has occurred. Prevents further access to the device.
	19/09/05	st		Support for persistant unplug/replug.
	22/09/05	st		Updates the version number on a file replace as well as an install.
	28/09/05	st		Had to place some code in to allow 0 based com ports to work again.
	13/01/06	st		No longer multiple object wait in DeviceNotify. Delayed deactivate thread added.
	06/07/07	ma		Changes to USBInstallDriver to reflect registry requirements for loading under CE 5.0.
						Minor change to USBAttachDevice to use GetStreamDriverKey instead of GetCurrentStreamDriverKey, 
						now hard-coded to point to USB\Drivers\ClientDrivers\FTDI_DEVICE.
	16/10/07	ma		Added OS version check and if CE 5 or later, support for 255 COM ports (COM 0 to COM 254).
	17/12/07	ma		Fixed issue with installing multiple ports above COM 10 on CE 5 and later.
						Changed GetNextAvailableIndex to use an array instead of a DWORD bitmap to keep track of ports in use - DWORD can only track 32 ports max.
	14/05/08	ma		Modified USBInstallDriver to dynamically load USBD instaed of using RegisterClientSettings and RegisterClientDriverID from our own USBD files.
						Gets around a permissions problem causing an access violation on CE 6.0.
	04/06/08	ma		Added hi-speed chip support. FT4232H and FT2232H.
	19/06/08	ma		Added a 500ms delay in DeviceNotify which prevents crash in device.exe (WinCE 5.2) when resuming.
	16/10/08	ma		Modified interface setup for FT2232H and FT4232H - port B and greater were selecting wrong interface number.
	03/03/11	ma		Added a surprise remove flag to handle surprise disconnects better (in particular to allow WM_DEVICECHANGE messages to propagate).
						Added support for the FT232H.
	10/08/11	ma		New features added: Assign same COM port number (initial index) to devices with the same EEPROM description.
						Additional device information disaplyed in the Drivers\Active key (Serial number, description, chip type, speed, VID, PID).
	16/12/11	ma		Changes to USBDeviceAttach - now only handle 1 interface per call.  If no interface specified, ignore the call. 


--*/
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
/*++


Module Name:

    ftdi_usb.c

Abstract:

    Entry point for the FTDI serial driver.

Notes:
	
	This is the entry point for a plugged in device (USBDeviceAttach)
	Sets up the registry on a new install or retrieves info from the registry on 
	an unplug replug
	
	No Revision history - all in sourcesafe


--*/

#include <windows.h>
#include <notify.h>
#include "ftdi_ioctl.h"
#include "ftdi_debug.h"
#include "BUSBDBG.H"
#include <serdbg.h>
#include <celog.h>
#include "INFParse.h"

#include "serpriv.h"

#include <omap35xx_usbhost.h>
#include "hcdi.h"
#include "usbdinc.hpp"
#include "usbdobj.hpp"

extern "C" BOOL gbAttached;	// set in DLL_ATTACH_XXX - used for a reboot on persistant port
//extern CRITICAL_SECTION gAttachCS;  // use this to make sure we're not accessing Active registry branch during DeactivateDevice

static USB_DRIVER_SETTINGS g_DriverSettings
= { sizeof(USB_DRIVER_SETTINGS),
	FTDI_VENDOR_ID, FTDI_PRODUCT_ID, USB_NO_INFO,
	USB_NO_INFO, USB_NO_INFO, USB_NO_INFO, 
	USB_NO_INFO, USB_NO_INFO, USB_NO_INFO
	};

#define PREFIX_SIZE		4

#define DEVLOAD_DEVFULLNAME_VALNAME TEXT("FullName")
#define DEVLOAD_DEVFULLNAME_VALTYPE REG_SZ

#define DEVLOAD_DEVSERIALNUMBER_VALNAME TEXT("SerialNumber")
#define DEVLOAD_DEVSERIALNUMBER_VALTYPE REG_SZ
#define DEVLOAD_DEVDESCRIPTION_VALNAME TEXT("Description")
#define DEVLOAD_DEVDESCRIPTION_VALTYPE REG_SZ
#define DEVLOAD_DEVCHIPTYPE_VALNAME TEXT("ChipType")
#define DEVLOAD_DEVCHIPTYPE_VALTYPE REG_SZ
#define DEVLOAD_DEVUSBSPEED_VALNAME TEXT("UsbSpeed")
#define DEVLOAD_DEVUSBSPEED_VALTYPE REG_SZ
#define DEVLOAD_DEVVID_VALNAME TEXT("VendorId")
#define DEVLOAD_DEVVID_VALTYPE REG_DWORD
#define DEVLOAD_DEVPID_VALNAME TEXT("ProductId")
#define DEVLOAD_DEVPID_VALTYPE REG_DWORD

#ifdef __cplusplus
extern "C" {
#endif
TCHAR gszStreamDriverKey[MAX_PATH];	// = TEXT("Drivers\\USB\\FTDI_DEVICE");
#ifdef __cplusplus
}
#endif
static TCHAR gszDriverPrefix[PREFIX_SIZE] = TEXT("MOM");	// default value
const TCHAR gszArrayIndex[] = TEXT("DeviceArrayIndex");
const TCHAR gszVersion[] = TEXT("Version");
const TCHAR gszVersionNumber[] = TEXT("1.1.0.12");

extern "C" OSVERSIONINFO osv;

const WCHAR gcszUnRegisterClientDriverId[] = L"UnRegisterClientDriverID";
const WCHAR gcszUnRegisterClientSettings[] = L"UnRegisterClientSettings";

const WCHAR gcszFTDIUSBDriverId[] = L"FTDI_DEVICE";

#if defined(DEBUG) && !defined(LOG_DEBUG)
#define INF_FILE_AND_PATH "\\Release\\FTDIPORT.INF"
#else
#define INF_FILE_AND_PATH "\\Windows\\FTDIPORT.INF"
#endif

DWORD GetMaxIndex()
{
	if(osv.dwMajorVersion >= 5) {
		DEBUGMSG1(ZONE_FUNCTION, (TEXT("OS 5\n")));
		return 255;	// (0..255)
	}
	else {
		DEBUGMSG1(ZONE_FUNCTION, (TEXT("OS l5\n")));
		return 9;	// (0..9)
	}
}

int GetNameIndex(TCHAR * szName)
{
	TCHAR cBuf[6];	// this is the max it should be
	TCHAR * pcBuf;
	int iNum;
	int ret;
		
	wcsncpy(cBuf, szName, 6);

	pcBuf = &cBuf[3];

	if(wcsncmp(cBuf, gszDriverPrefix, 3) != 0)
		return -1;
	
	ret = swscanf(pcBuf, L"%d:", &iNum);
	DEBUGMSG2(ZONE_FUNCTION,TEXT("iNum: %d\n"),iNum);

	if((ret == 0) || (ret == EOF))
		return -1;
	
	return iNum;
}

/*++

Routine Description:
	From a driver point of view we need to get the next available index availible in the COM0 to COM9 range
	This function gets the index the user wants to start at (InitialIndex) then searches for the next available index 
	in Drivers\Active

Arguments:
	pszStreamDriverKey - the string value of the key to look in for InitialIndex value

Return Value:
	-1 if failed to find index
	otherwise the index it found

--*/
DWORD 
GetRegistryInitialIndex(
	TCHAR * pszStreamDriverKey,
	PFTDI_DEVICE pUsbFTDI
	)
{
	DWORD dwInitIndex, dwSize, dwType, dwMaxIndex;
	HKEY hKey, hSubKey;
	LONG lStatus;
	char bDescIndexFound = 0;

	dwMaxIndex = GetMaxIndex();

	DEBUGMSG1(ZONE_FUNCTION, TEXT(">GetRegistryInitialIndex\n"));


	//
	// The InitialIndex setting is to allow customers to start at a different index other than 0
	//

	// First, try to open a key with the device description
	// This feature was added for Mettler Toledo at sales departement request (Sacha.Romier@mt.com)
	// They wanted any device with the same description to be given the same COM index
	// therefore match the description in the registry first.  Otherwise, do as before.
	if (pUsbFTDI)
	{
		DEBUGMSG1(ZONE_FUNCTION, TEXT("Attempting to get description index...\n"));
		// OK, we're going to try to use the description string to try to open a registry key
		lStatus = RegOpenKeyEx(
			HKEY_LOCAL_MACHINE, 
			pszStreamDriverKey, 
			0, 
			0, 
			&hKey
			);
		

		if(lStatus == ERROR_SUCCESS) {

			if (pUsbFTDI->Description[0]) {

				TCHAR tcTemp[128];
				TCHAR szTemp[MAX_PATH + 10];
				mbstowcs(tcTemp, pUsbFTDI->Description, strlen(pUsbFTDI->Description) +1);
				swprintf(szTemp, TEXT("%s"), tcTemp);

				// try to open subkey with description name
				lStatus = RegOpenKeyEx(
						hKey, 
						szTemp, 
						0, 
						0, 
						&hSubKey
						);


				if(lStatus == ERROR_SUCCESS) {

					DEBUGMSG2(ZONE_ERROR, TEXT("Opened registry key for description %s\n"),szTemp);

					dwSize = sizeof(DWORD);
					lStatus = RegQueryValueEx(
								hSubKey, 
								TEXT("InitialIndex"), 
								0, 
								&dwType, 
								(LPBYTE)&dwInitIndex, 
								&dwSize
								);

					if(lStatus == ERROR_SUCCESS) {
						// found an initial index value for this description
						// use this one
						bDescIndexFound = 1;
						DEBUGMSG2(ZONE_ERROR, TEXT("Got description initial index of %d\n"),dwInitIndex);
					}

					// close hSubKey
					if(hSubKey) {
						RegCloseKey(hSubKey);
					}

				}
				else {
					// failed to open a key for this description
					DEBUGMSG2(ZONE_ERROR, TEXT("FAILED to open registry key for description %s\n"),szTemp);
				}
			}

			// close hKey
			if(hKey) {
				RegCloseKey(hKey);
			}
			
			if (bDescIndexFound == 1)
				return dwInitIndex;
		}

	}


	// OK - didn't find the description in the registry, proceed in the legacy way...
	lStatus = RegOpenKeyEx(
				HKEY_LOCAL_MACHINE, 
				pszStreamDriverKey, 
				0, 
				0, 
				&hKey
				);

	if(lStatus == ERROR_SUCCESS) {

		dwSize = sizeof(DWORD);
		lStatus = RegQueryValueEx(
					hKey, 
					TEXT("InitialIndex"), 
					0, 
					&dwType, 
					(LPBYTE)&dwInitIndex, 
					&dwSize
					);

		if (lStatus != ERROR_SUCCESS) {
			DEBUGMSG2(ZONE_ERROR, TEXT("Error reading value: %X\n"),lStatus);	
		}
		else {
			DEBUGMSG2(ZONE_ERROR, TEXT("Initial Index Reg value: %d\n"),dwInitIndex);	
		}
		//
		// account for invalid entry or failed read
		//
		//if((lStatus != ERROR_SUCCESS) || (dwInitIndex > 9)) {
		if((lStatus != ERROR_SUCCESS) || (dwInitIndex > dwMaxIndex)) {
			DEBUGMSG1(ZONE_FUNCTION, TEXT("Setting Initial index to 0\n"));	
			dwInitIndex = 0;
		}
	}

	if(hKey) {
		RegCloseKey(hKey);
	}
	DEBUGMSG1(ZONE_FUNCTION, TEXT("<GetRegistryInitialIndex\n"));

	return dwInitIndex;
}

/*++

Routine Description:
	From a driver point of view we need to get the next available index availible in the COM0 to COM9 range
	This function gets the index the user wants to start at (InitialIndex) then searches for the next available index 
	in Drivers\Active

Arguments:
	pszStreamDriverKey

Return Value:
	-1 if failed to find index
	otherwise the index it found

--*/
DWORD 
GetNextAvailableIndex(
	TCHAR * pszStreamDriverKey,
	PFTDI_DEVICE pUsbFTDI
	)
{
	DWORD dwType, dwSize, dwMaxIndex;
//	DWORD dwUsedIndexMask = 0;
	DWORD dwInitIndex = 0;
	HKEY hKey, hSubKey;
	int i = 0, j = 0;
	LONG lStatus;
	TCHAR szKey[128], szName[128];

	char UsedIndexArray[255];

	// Initialise array
	memset(UsedIndexArray,0,sizeof(UsedIndexArray));

//  MOM0 is dedicated to the internal modem
	dwInitIndex = 1; //GetRegistryInitialIndex(pszStreamDriverKey, pUsbFTDI);

	dwMaxIndex = GetMaxIndex();

	DEBUGMSG2(ZONE_FUNCTION, TEXT("InitialIndex = %d\n"), dwInitIndex);

	lStatus = RegOpenKeyEx(
				HKEY_LOCAL_MACHINE, 
				TEXT("drivers\\active"), 
				0, 
				0, 
				&hKey
				);

	if(lStatus != ERROR_SUCCESS) {

		//
		// give 0 a try if we have nothing
		//
		return 0;
	}

	while (1) {

		//
		// Enumerate active driver list
		//
		dwSize = sizeof(szKey);
		if(RegEnumKeyEx (hKey, j++, szKey, &dwSize, NULL, NULL, NULL, NULL) != ERROR_SUCCESS)
			break;

		//
		// Open active driver key.
		//
		lStatus = RegOpenKeyEx(
					hKey, 
					szKey, 
					0, 
					0, 
					&hSubKey
					);

		if(lStatus != ERROR_SUCCESS)
			continue;

		//
		// Get full device name (e.g. COM0:)
		//
		dwSize = sizeof(szName);
		lStatus = RegQueryValueEx(
					hSubKey, 
					TEXT("Name"), 
					0, 
					&dwType, 
					(PBYTE)szName, 
					&dwSize
					);
	
		if(lStatus == ERROR_SUCCESS) {
			if((i = GetNameIndex(szName)) != -1) {
				//dwUsedIndexMask |= (1 << i);
				UsedIndexArray[i] = 1;
			}
		}
		
		if (osv.dwMajorVersion >= 5) {
			//
			// Get full device name (e.g. COM15)
			//
			dwSize = sizeof(szName);
			lStatus = RegQueryValueEx(
						hSubKey, 
						TEXT("FullName"), 
						0, 
						&dwType, 
						(PBYTE)szName, 
						&dwSize
						);
		
			if(lStatus == ERROR_SUCCESS) {
				if((i = GetNameIndex(szName)) != -1) {
					//dwUsedIndexMask |= (1 << i);
					UsedIndexArray[i] = 1;
				}
			}
		}

		RegCloseKey(hSubKey);
	}

	RegCloseKey(hKey);

	//
	// we have the name mask - now get the lowest unused number that matches the users request
	//
	//for(i = 0; i < 10; i++) {
	for(i = 0; i < (int)dwMaxIndex; i++) {
//		if((dwUsedIndexMask & (1 << i)) == 0)		// then its a free index
		if(UsedIndexArray[i] == 0)		// then its a free index
			if(i >= (int)dwInitIndex) {				// is it what the user wants
				DEBUGMSG2(ZONE_FUNCTION, TEXT("ComPort Number = %d\n"), i);	
				return i;
			}
	}
	return -1;	// none left or error?
}


BOOL USBInstallDriver(
  LPCWSTR szDriverLibFile 
)
{
	BOOL fRet = FALSE;
	BOOL bGotPID = FALSE;
	DWORD dwIndex = -1;
	HINSTANCE hInst = LoadLibrary(L"USBD.DLL");
	DWORD dwVID;
	DWORD dwPID;
	HKEY hKey = NULL;

	LPREGISTER_CLIENT_DRIVER_ID lpfnRegisterClientDriverID;
	LPREGISTER_CLIENT_SETTINGS lpfnRegisterClientSettings;

	HINSTANCE hUSBD = LoadLibrary (TEXT("usbd.dll"));
	if (hUSBD == 0)
	{
	    DEBUGMSG (ZONE_ERROR, (TEXT("Can't load USBD.DLL\r\n")));
		return FALSE;
	}

	
	DEBUGMSG2( ZONE_FUNCTION, TEXT(">USBInstallDriver(%s)\n"), szDriverLibFile );

	if(GetNextVIDPID(INF_FILE_AND_PATH, &dwVID, &dwPID) == TRUE) {
		bGotPID = TRUE;		// this means we will create a unique PID registry path
		// For example if we have a FAC2 PID it will create a FTDI_FAC2 entry for the registry settings
		// and if we have a 6002 PID it will similarly create a FTDI_6002 registry entry.
		// If we find no PID we create a FTDI entry only
		// For above example we will have 
		//	-FTDI_DEVICE_FAC2
		//		-BulkInTransferSize = 4096			// default for this VID if no serial number
		//		-FT00001
		//			-BulkInTransferSize = 64		// unique to this device serial number
		//	-FTDI_DEVICE_6002
		//		-BulkInTransferSize = 4096			// default for this VID if no serial number
		//		-FT00002
		//			-BulkInTransferSize = 64		// unique to this device serial number
	}



	// Create [HKEY_LOCAL_MACHINE\Drivers\USB\ClientDrivers\FTDI_DEVICE]
	//GetStreamDriverKey(szStreamDriverKey, MAX_PATH, dwPID, bGotPID);
	
	// Explicitly link to USBD DLL.  
	lpfnRegisterClientDriverID = (LPREGISTER_CLIENT_DRIVER_ID) GetProcAddress (hUSBD, TEXT("RegisterClientDriverID"));
	lpfnRegisterClientSettings = (LPREGISTER_CLIENT_SETTINGS) GetProcAddress (hUSBD, TEXT("RegisterClientSettings"));
	
	if ((lpfnRegisterClientDriverID == 0) || (lpfnRegisterClientSettings == 0))
	{
		FreeLibrary (hUSBD);
		return FALSE;
	}

	fRet = (lpfnRegisterClientDriverID) (gcszFTDIUSBDriverId);

	//fRet = RegisterClientDriverID(gcszFTDIUSBDriverId);
	if(!fRet)  {
		DEBUGMSG2( ZONE_FUNCTION, TEXT("RegisterClientDriverID Fail %d\n"), GetLastError());
		//UnRegisterClientDriverID(gcszFTDIUSBDriverId);
			FreeLibrary (hUSBD);
			return FALSE;
	}

	// Create LoadClient registry keys for each VID & PID combination

	do {
		// First create 
		// [HKEY_LOCAL_MACHINE\Drivers\USB\LoadClients\<VID_PID>\Default\Default\<driver class>]
		// [HKEY_LOCAL_MACHINE\Drivers\USB\LoadClients\<VID_PID>\Default\Default\FTDI_DEVICE]

		// When installing by interface, need 
		// [HKEY_LOCAL_MACHINE\Drivers\USB\LoadClients\<VID_PID>\Default\<255_255_255>\<driver class>]
		
//		g_DriverSettings.dwInterfaceClass = FTDI_INTERFACE_CLASS;
//		g_DriverSettings.dwInterfaceSubClass = FTDI_INTERFACE_SUBCLASS;
//		g_DriverSettings.dwInterfaceProtocol = FTDI_INTERFACE_PROTOCOL;
		g_DriverSettings.dwInterfaceClass = USB_NO_INFO;

		if(bGotPID == TRUE)	{
			g_DriverSettings.dwVendorId = dwVID;
			g_DriverSettings.dwProductId = dwPID;
		}

		fRet = (lpfnRegisterClientSettings) (szDriverLibFile, gcszFTDIUSBDriverId, 
												 NULL, &g_DriverSettings);

//		fRet = RegisterClientSettings(szDriverLibFile, gcszFTDIUSBDriverId, NULL, &g_DriverSettings);

		if(!fRet)  {
			DEBUGMSG2( ZONE_FUNCTION, TEXT("RegisterClientSettings1 Fail %d\n"), GetLastError());
			//UnRegisterClientDriverID(gcszFTDIUSBDriverId);
			//UnRegisterClientSettings(gcszFTDIUSBDriverId, NULL, &g_DriverSettings);
			FreeLibrary (hUSBD);
			return FALSE;
		}




	// DON'T DO THIS ANY MORE????
		// Now create 
		// [HKEY_LOCAL_MACHINE\Drivers\USB\LoadClients\Default\Default\<InterfaceClassCode>\<driver class>]
		// [HKEY_LOCAL_MACHINE\Drivers\USB\LoadClients\Default\Default\255\FTDI_DEVICE]

		g_DriverSettings.dwVendorId = USB_NO_INFO;
		g_DriverSettings.dwProductId = USB_NO_INFO;
		g_DriverSettings.dwInterfaceClass = FTDI_INTERFACE_CLASS;

		if(fRet) {
			fRet = (lpfnRegisterClientSettings) (szDriverLibFile, gcszFTDIUSBDriverId, 
												 NULL, &g_DriverSettings);

//		fRet = RegisterClientSettings(szDriverLibFile, gcszFTDIUSBDriverId, NULL, &g_DriverSettings);
			if(!fRet)  {
				DEBUGMSG2( ZONE_FUNCTION, TEXT("RegisterClientSettings2 Fail %d\n"), GetLastError());
				//UnRegisterClientDriverID(gcszFTDIUSBDriverId);
				//UnRegisterClientSettings(gcszFTDIUSBDriverId, NULL, &g_DriverSettings);
				FreeLibrary (hUSBD);
				return FALSE;
			}
		}

	} while (GetNextVIDPID(INF_FILE_AND_PATH, &dwVID, &dwPID) == TRUE);


	// Open client registry key and create registry values
	if (fRet) {
		hKey = OpenClientRegistryKey(gcszFTDIUSBDriverId);
		GetStreamDriverKey(gszStreamDriverKey);
		DEBUGMSG2( ZONE_FUNCTION, TEXT("StreamDriverKey: %s\n"), gszStreamDriverKey);
	}


	// Setup the registry values from the INF file first if there is any
	RegisterINFValues(INF_FILE_AND_PATH, gszDriverPrefix, gszStreamDriverKey);

	//if (dwStatus == ERROR_SUCCESS && dwDisp == REG_CREATED_NEW_KEY) {

	if (hKey != NULL) {			
		DWORD dwStatus;						
		//
		// Dll = ftdi_ser.dll
		//
		dwStatus = RegSetValueEx(
						hKey,
						DEVLOAD_DLLNAME_VALNAME,
						0,
						DEVLOAD_DLLNAME_VALTYPE,
						(const BYTE *)szDriverLibFile,
						(lstrlen(szDriverLibFile)+1)*sizeof(WCHAR)
						);

		if (dwStatus == ERROR_SUCCESS) {
			//
			// Prefix = COM or whatever
			//
			dwStatus = RegSetValueEx(
							hKey, 
							DEVLOAD_PREFIX_VALNAME, 
							0, 
							DEVLOAD_PREFIX_VALTYPE, 
							(const BYTE *) gszDriverPrefix, 
							sizeof(gszDriverPrefix)
							);
		}

		//
		// Version = X.X.X.X
		//
		dwStatus = RegSetValueEx(
						hKey, 
						gszVersion,
						0,
						REG_SZ,
						(const BYTE *)gszVersionNumber,
						sizeof(gszVersionNumber)
						);
	
		if (dwStatus == ERROR_SUCCESS) {
			dwIndex = GetNextAvailableIndex(gszStreamDriverKey, NULL);
			if(dwIndex != -1) {
				// DeviceArrayIndex is required by the mdd.c driver but this value is always 0
				// and only seems to be used when you call GetSerialObject which allocates a 
				// structure every time and is freed on closing

				//
				// DeviceArrayIndex
				//
				dwStatus = RegSetValueEx(
								hKey,
								gszArrayIndex,
								0,
								REG_DWORD,
								(const BYTE *) &dwIndex,
								sizeof(DWORD)
								); 
			}
			else {
				dwStatus = ~ERROR_SUCCESS;
				fRet = FALSE;
			}
		}

//	if (dwStatus != ERROR_SUCCESS)
	if (hKey == NULL)
	{
		DEBUGMSG2( ZONE_FUNCTION, TEXT("GetStreamDriverKey Fail %d\n"), GetLastError());

		if (hKey)
			RegDeleteKey(HKEY_LOCAL_MACHINE, gszStreamDriverKey);
		fRet = FALSE;
	}
	if (hKey)
		RegCloseKey(hKey);
	}
//	} while (GetNextVIDPID(INF_FILE_AND_PATH, &dwVID, &dwPID) == TRUE);

	DEBUGMSG2( ZONE_FUNCTION, TEXT("<USBInstallDriver fRet = %d\n"), fRet);
	FreeLibrary (hUSBD);
    return fRet;
}

#ifdef REMOVED
BOOL 
USBUnInstallDriver(
   VOID
   )
{
    BOOL fRet = FALSE;
    HINSTANCE hInst = LoadLibrary(L"USBD.DLL");

	DEBUGMSG2( ZONE_FUNCTION, TEXT("<USBUnInstallDriver:%d\n"), fRet);


    if(hInst)
    {
        LPUN_REGISTER_CLIENT_DRIVER_ID pUnRegisterId =
                (LPUN_REGISTER_CLIENT_DRIVER_ID)
                GetProcAddress(hInst, gcszUnRegisterClientDriverId);

        LPUN_REGISTER_CLIENT_SETTINGS pUnRegisterSettings =
                (LPUN_REGISTER_CLIENT_SETTINGS) GetProcAddress(hInst,
                gcszUnRegisterClientSettings);
		if(pUnRegisterSettings)
		{
		} else
    		DEBUGMSG1(ZONE_ERROR, (TEXT("[USB] FTDI: USBUnInstallDriver : no procaddr for pUnRegisterSettings\n")));


        if(pUnRegisterId)
        { 
		} else
    		DEBUGMSG1( ZONE_ERROR, (TEXT("[USB] FTDI: USBUnInstallDriver : no procaddr for pUnRegisterId\n")));

        FreeLibrary(hInst);
    } else
    	DEBUGMSG2(ZONE_ERROR, TEXT("[USB] FTDI: USBUnInstallDriver : could not load usbd.dll : %d\n"), GetLastError ());

    DEBUGMSG2(ZONE_FUNCTION, TEXT("[USB] FTDI: -USBUnInstallDriver %d\n"), fRet);

	return fRet;
}
#endif

/*++

Routine Description:


Arguments:

Return Value:

    TRUE if successful,
    FALSE otherwise

--*/
BOOL 
USBUnInstallDriver(
   VOID
   )
{
   BOOL bRc = TRUE;

   DEBUGMSG2(ZONE_FUNCTION, TEXT("<USBUnInstallDriver:%d\n"), bRc);

   return bRc;
}

/*++

Routine Description:
	Helper - cleans up allocated memory/events etc...

Arguments:
	pUsbFTDI	- pointer to our device structure
	bRemovePipes - only required when restoring a device instance

Return Value:

    TRUE if successful,
    FALSE otherwise

--*/
VOID
RemoveDeviceStructure(
   PFTDI_DEVICE pUsbFTDI,
   BOOL bRemovePipes
   )
{
	DEBUGMSG2(ZONE_FUNCTION,TEXT(">RemoveDeviceStructure(%p)\n"), pUsbFTDI);

	if(VALID_CONTEXT(pUsbFTDI)) {

		if(pUsbFTDI->Flags.Open) {
			DEBUGMSG1(ZONE_ERROR,(TEXT("RemoveDeviceStructure on open device!\n")));
			TEST_TRAP();
			return;
		}

		if(bRemovePipes && pUsbFTDI->BulkIn.hPipe && pUsbFTDI->UsbFuncs) {
			pUsbFTDI->UsbFuncs->lpClosePipe(pUsbFTDI->BulkIn.hPipe);
		}  	

		if(pUsbFTDI->BulkIn.hEvent) {
			if(CloseHandle(pUsbFTDI->BulkIn.hEvent) == FALSE) {
				DEBUGMSG2(ZONE_FUNCTION,TEXT("CloseHandle(pUsbFTDI->BulkIn.hEvent) failed - %ld)\n"), GetLastError());
			}
		}
      
		if(bRemovePipes && pUsbFTDI->BulkOut.hPipe && pUsbFTDI->UsbFuncs) {
			pUsbFTDI->UsbFuncs->lpClosePipe(pUsbFTDI->BulkOut.hPipe);
		}

		if(pUsbFTDI->BulkOut.hEvent) {
			if(CloseHandle(pUsbFTDI->BulkOut.hEvent) == FALSE) {
				DEBUGMSG2(ZONE_FUNCTION,TEXT("CloseHandle(pUsbFTDI->BulkOut.hEvent) failed - %ld)\n"), GetLastError());
			}
		}

		if(pUsbFTDI->hEP0Event) {
			if(CloseHandle(pUsbFTDI->hEP0Event) == FALSE) {
				DEBUGMSG2(ZONE_FUNCTION,TEXT("CloseHandle(pUsbFTDI->hEP0Event) failed - %ld)\n"), GetLastError());
			}
		}

		if(pUsbFTDI->hCloseEvent) {
			if(CloseHandle(pUsbFTDI->hCloseEvent) == FALSE) {
				DEBUGMSG2(ZONE_FUNCTION,TEXT("CloseHandle(pUsbFTDI->hCloseEvent) failed - %ld)\n"), GetLastError());
			}
		}

		if(pUsbFTDI->hReconnectEvent) {
			if(CloseHandle(pUsbFTDI->hReconnectEvent) == FALSE) {
				DEBUGMSG2(ZONE_FUNCTION,TEXT("CloseHandle(pUsbFTDI->hReconnectEvent) failed - %ld)\n"), GetLastError());
			}
		}

		if(pUsbFTDI->hRestartEvent) {
			if(CloseHandle(pUsbFTDI->hRestartEvent) == FALSE) {
				DEBUGMSG2(ZONE_FUNCTION,TEXT("CloseHandle(pUsbFTDI->hRestartEvent) failed - %ld)\n"), GetLastError());
			}
		}
      
		if(&pUsbFTDI->Lock) {
			DeleteCriticalSection(&pUsbFTDI->Lock);
		}

		FT_LocalFree(pUsbFTDI);
   
	} else {
		DEBUGMSG1(ZONE_ERROR,(TEXT("Invalid Parameter\n")));
	}

   DEBUGMSG1(ZONE_FUNCTION,(TEXT("<RemoveDeviceStructure\n")));  
   return;
}

/*++

Routine Description:
	This gets called when the device is removed - so it does the cleanup of the device then

Arguments:

Return Value:

    TRUE if successful,
    FALSE otherwise

--*/
BOOL WINAPI 
DeviceNotify(
   LPVOID lpvNotifyParameter,
   DWORD dwCode,
   LPDWORD * dwInfo1,
   LPDWORD * dwInfo2,
   LPDWORD * dwInfo3,
   LPDWORD * dwInfo4
   )
{
	PFTDI_DEVICE pUsbFTDI = (PFTDI_DEVICE)lpvNotifyParameter;
	DWORD dwWaitReturn = 0;
	BOOL bRc = FALSE;
	BOOL bDeactivateDevice = FALSE;
    UNREFERENCED_PARAMETER(dwInfo1);
    UNREFERENCED_PARAMETER(dwInfo2);
    UNREFERENCED_PARAMETER(dwInfo3);
    UNREFERENCED_PARAMETER(dwInfo4);


	DEBUGMSG1(ZONE_FUNCTION, (TEXT(">DeviceNotify\r\n")));
//	RETAILMSG(1, (TEXT("DeviceNotify...\r\n")));

	if(!VALID_CONTEXT(pUsbFTDI)) {
		DEBUGMSG1(ZONE_ERROR, (TEXT("Invalid Context!\n")));
		return FALSE;
	}
   
	// Use the OpenCS to make sure we don't collide with an in-progress open.
	EnterCriticalSection(&((PHW_INDEP_INFO)(pUsbFTDI->pMddHead))->OpenCS);

	switch(dwCode) {

		case USB_CLOSE_DEVICE:
			DEBUGMSG1(ZONE_FUNCTION, (TEXT("USB_CLOSE_DEVICE\n")));

			if(pUsbFTDI->Flags.Open && (pUsbFTDI->ConfigDataFlags & FTDI_PERSISTANT_DEVICE_INSTANCE)) {
//				RETAILMSG(1, (TEXT("DeviceNotify: device is PERSISTENT!\r\n")));
				SetDeviceUnloading(pUsbFTDI);
				// Need this delay here to prevent crash in device.exe on resume (CE 5.2)
				Sleep(500);
				LeaveCriticalSection(&((PHW_INDEP_INFO)(pUsbFTDI->pMddHead))->OpenCS);
				return TRUE;
			}
			else if(pUsbFTDI->Flags.Open) {
//				RETAILMSG(1, (TEXT("DeviceNotify: device is open\r\n")));
				SetDeviceUnloading(pUsbFTDI);

				//
				// Notify the user application if they have requested it
				//
				EnterCriticalSection(&pUsbFTDI->NotifyLock);
				if(pUsbFTDI->Notify.pvParam) {
					
#ifndef WINCE6
					DWORD dwOldPermissions;
					dwOldPermissions = SetProcPermissions(pUsbFTDI->dwNotifyPermissions);
#endif
					pUsbFTDI->dwCurrentNotifyStatus = FT_USB_CLOSE_DEVICE;
					SetEvent((HANDLE)pUsbFTDI->Notify.pvParam);
#ifndef WINCE6
					SetProcPermissions(dwOldPermissions);
#endif
				}
				LeaveCriticalSection(&pUsbFTDI->NotifyLock);

				bDeactivateDevice = TRUE;
			}
			else {
//				RETAILMSG(1, (TEXT("DeviceNotify: device is closed\r\n")));
				//
				// Close the ActiveSync monitoring thread if it is running
				//
				TerminateActiveSyncMonitor(pUsbFTDI);
				bDeactivateDevice = TRUE;
			}
			break;	//USB_CLOSE_DEVICE

		default:
			DEBUGMSG2(ZONE_ERROR, TEXT("Unhandled notify code:%d\n"), dwCode);
			break;
	};


	do {
		if((pUsbFTDI->Flags.UnloadPending == TRUE) && (pUsbFTDI->Flags.SurpriseRemoved == FALSE)) {

			//
			// Something strange happened with the ARM platforms - we couldnt have a WaitForMultiple here
			// so this had to be kludged and use hCloseEvent for multiple thihgs by using the Persistant flag
			//
			DEBUGMSG1(ZONE_FUNCTION, (TEXT("Waiting for CloseEvent or ReconnectEvent...\n")));
			dwWaitReturn = WaitForSingleObject(pUsbFTDI->hCloseEvent, 250/*INFINITE*/);

			if(dwWaitReturn == 0) {
				//
				// Warning: DeactivateDevice forces Device Manager to call our FTD_Deinit, which then
				// causes COREDLL!xxX_CloseHandle to AV when PRNPORT!PrinterSend calls CloseHandle(hPrinter).
				// So, give it a chance to clean up.
				//
				// Took this out as Gotive were concerned with the 1 second delay (4/8/05) but this really messed things up
				// The unplug and notify on open would work most of the time but a subtle timing problem would break the driver
				// so left this in and suggested a few other things. See COM_Close for what I proposed with the signals on closing
				// the device.
				//
				DEBUGMSG2(ZONE_ERROR, TEXT("DeviceNotify close for device %p\n"), pUsbFTDI);

				Sleep(1000);
				bDeactivateDevice = TRUE;
			}
		}
		else if (pUsbFTDI->Flags.SurpriseRemoved == TRUE) {
			DEBUGMSG1(ZONE_FUNCTION, (TEXT("Surprise removed -> deactivate device\n")));
			bDeactivateDevice = TRUE;
		}
		else {
			bDeactivateDevice = TRUE;
		}
	} while (!bDeactivateDevice);

	if(bDeactivateDevice == TRUE) {

		DEBUGMSG1(ZONE_FUNCTION, (TEXT("DeactivateDevice\n")));

		// Deactivate up-layer VSP
		if (pUsbFTDI->hVspDevice)
			DeactivateDevice(pUsbFTDI->hVspDevice);

		// Protect this with Critical Section
		// Try to prevent us from having the Active key open when calling DeactivateDevice
		// deprecated code
//		EnterCriticalSection(&gAttachCS);
		bRc = DeactivateDevice(pUsbFTDI->hStreamDevice);
//		LeaveCriticalSection(&gAttachCS);

		if(!bRc) {
			DEBUGMSG2(ZONE_ERROR, TEXT("DeactivateDevice error: %d\n"), GetLastError());
//			RETAILMSG(1, (TEXT("DeactivateDevice FAILED!!\r\n")));
		}
		else {
//			RETAILMSG(1, (TEXT("DeactivateDevice SUCCEEDED!!\r\n")));
		}
//		Sleep(1000);
		LPCRITICAL_SECTION OpenCS = &((PHW_INDEP_INFO)(pUsbFTDI->pMddHead))->OpenCS;
		RemoveDeviceStructure(pUsbFTDI, TRUE);
		LeaveCriticalSection(OpenCS);

		return TRUE;
	}

	DEBUGMSG1(ZONE_FUNCTION, (TEXT("<DeviceNotify\n")));

	LeaveCriticalSection(&((PHW_INDEP_INFO)(pUsbFTDI->pMddHead))->OpenCS);

	return FALSE;
}

/*++

Routine Description:
	This routine will be called as a thread from COM_Close to delay removal of
	the FTDI device and perform a cleanup.

Arguments:

Return Value:

--*/
DWORD WINAPI
DelayedDeviceDeactivate(
    IN PVOID Context
    )
{
	BOOL bRc = FALSE;
	PFTDI_DEVICE pUsbFTDI = NULL;

//	RETAILMSG(1, (TEXT("DelayedDeviceDeactivate\r\n")));
	DEBUGMSG1(ZONE_FUNCTION, (TEXT(">DelayedDeviceDeactivate\n")));

	pUsbFTDI = (PFTDI_DEVICE)Context;
	
	bRc = DeactivateDevice(pUsbFTDI->hStreamDevice);
	if(!bRc) {
		DEBUGMSG2(ZONE_ERROR, TEXT("DeactivateDevice error: %d\n"), GetLastError());
//		RETAILMSG(1, (TEXT("DeactivateDevice FAILED!!\n")));
	}
	else {
//		RETAILMSG(1, (TEXT("DeactivateDevice SUCCEEDED!!\n")));
	}
	RemoveDeviceStructure(pUsbFTDI, TRUE);

	DEBUGMSG1(ZONE_FUNCTION, (TEXT("<DelayedDeviceDeactivate\n")));

	return 0;
}


/*++

Routine Description:
	This was added as if you are configuring for synchronous/asynchronous mode then you will want to 
	set the BulkIn.hEvent to null or not depending on the setup

Arguments:

	pDevice - Handle to the USB device
	
Return Value:

--*/
VOID 
CreateBulkPipeEvents(
		PFTDI_DEVICE pDevice
		)
{
	pDevice->BulkIn.pCompletionRoutine = &FTDIReadTransferComplete;
	pDevice->BulkOut.pCompletionRoutine = &FTDIWriteTransferComplete;

	pDevice->BulkIn.hEvent = CreateEvent(NULL, MANUAL_RESET_EVENT, FALSE, NULL);

	DEBUGMSG2(ZONE_FUNCTION, TEXT("CreateBulkPipeEvents BulkIn.hEvent: 0x%X\n"), pDevice->BulkIn.hEvent);

	if(!pDevice->BulkIn.hEvent) {
		DEBUGMSG2(ZONE_ERROR, TEXT("CreateBulkPipeEvents CreateEvent1 error:%d\n"), GetLastError());
		//
		// Make it synchronous calls
		//		
		pDevice->BulkIn.hEvent = NULL;		
		pDevice->BulkIn.pCompletionRoutine = NULL;
	}

	pDevice->BulkOut.hEvent = CreateEvent(NULL, MANUAL_RESET_EVENT, FALSE, NULL);
	if(!pDevice->BulkOut.hEvent) {
		DEBUGMSG2(ZONE_ERROR, TEXT("CreateBulkPipeEvents CreateEvent2 error:%d\n"), GetLastError());
		//
		// Make it synchronous calls
		//
		pDevice->BulkOut.hEvent = NULL;
		pDevice->BulkOut.pCompletionRoutine = NULL;
	}
}

/*++

Routine Description:
	Setup the BulkIn and BulkOut transfer flags. Originally this was for the Thales Fudge that occurred
	with the Phillips 1760 host controller which allowed the host driver to determine if we were calling
	its lower levels or not. 30/6/05.
	This will be set on a global registry (not on a per device setting) so there is no need to look for the 
	serial numbers etc...
	Also setup for synchronous or asynchronous transfers. This will be set on a global scale too.
	
	These will be undocumented registry settings only to be used by support in extreme cases

Arguments:

	pDevice - Handle to the USB device
	
Return Value:

--*/
VOID 
ConfigureBulkTransfers(
		PFTDI_DEVICE pDevice
		)
{
	TCHAR szDeviceRegistry[MAX_PATH + 10];
	DWORD dwBulkInFlags, dwBulkOutFlags, dwSynchronousDummy;
	DWORD dwDataSize;
	LONG regError;
    HKEY hKey;

	//
	// Get the registry setting - if none then return
	//
//	GetCurrentStreamDriverKey(szDeviceRegistry, pDevice->dwPID);
	GetStreamDriverKey(szDeviceRegistry);

	//
	// Setup the default values
	//
	pDevice->BulkIn.dwBulkFlags = (USB_IN_TRANSFER | USB_SHORT_TRANSFER_OK);
	pDevice->BulkOut.dwBulkFlags = (USB_OUT_TRANSFER /*| USB_SHORT_TRANSFER_OK*/);

	regError = RegOpenKeyEx(
					HKEY_LOCAL_MACHINE, 
					szDeviceRegistry,
					0, 
					KEY_ALL_ACCESS, 
					&hKey
					);

    if (regError == ERROR_SUCCESS) {

		dwDataSize = sizeof(DWORD);
		regError = RegQueryValueEx(
						hKey, 
						TEXT("BulkInFlags"),
						NULL, 
						NULL, 
						(LPBYTE)&dwBulkInFlags,
						&dwDataSize
						);

		if(regError == ERROR_SUCCESS) {
			pDevice->BulkIn.dwBulkFlags = dwBulkInFlags;
		}

		regError = RegQueryValueEx(
						hKey, 
						TEXT("BulkOutFlags"),
						NULL, 
						NULL, 
						(LPBYTE)&dwBulkOutFlags,
						&dwDataSize
						);

		if(regError == ERROR_SUCCESS) {
			pDevice->BulkOut.dwBulkFlags = dwBulkOutFlags;
		}

		dwDataSize = sizeof(DWORD);
		regError = RegQueryValueEx(
						hKey, 
						TEXT("SynchronousBulk"),
						NULL, 
						NULL, 
						(LPBYTE)&dwSynchronousDummy,
						&dwDataSize
						);

		if(regError == ERROR_SUCCESS) {
			pDevice->BulkIn.hEvent = NULL;
			pDevice->BulkIn.pCompletionRoutine = NULL;
		}
		else {
			CreateBulkPipeEvents(pDevice);
		}
	}
	else {
		DEBUGMSG2(ZONE_ERROR, TEXT("GetBulkTransferFlags: Failed to get registy settings for path %s\n"), szDeviceRegistry);
		CreateBulkPipeEvents(pDevice);
	}

	if(hKey)
		RegCloseKey(hKey);
}

/*++

Routine Description:

	If the device has a serial number then create a unique registry entry to 
	store device dependent registry values.
	Copy the registry values from the default version

Arguments:
	pDevice: pointer to the device context
	
Return Value:

    TRUE if created,
    FALSE otherwise

--*/
BOOL CreateUniqueDriverSettings(
		PFTDI_DEVICE pDevice,
		TCHAR * pszDriverKey
		)
{
	BOOL bRet = FALSE;
	DWORD dwDisp, dwDataSize = 0, dwStatus = -1;
	DWORD dwLatency, dwInTransferSize, dwOutTransferSize, dwConfigDataFlags, dwInitialIndex, dwEmMode, dwActiveSetting;
	HKEY hKeyDefaultSettings = NULL;
	HKEY hKeyUniqueSettings = NULL;
	TCHAR szTemp[MAX_PATH + 10];	// +10to account for the serial number
	FTDI_CONFIG_DATA cd;

	wcscpy(szTemp, gszStreamDriverKey);
	if(pDevice->SerialNumber[0]) {

		dwStatus =	RegCreateKeyEx(
			HKEY_LOCAL_MACHINE,
			gszStreamDriverKey,
			0,
			NULL,
			0,
			0,
			NULL,
			&hKeyDefaultSettings,
			&dwDisp
			);

		if(dwStatus == ERROR_SUCCESS) {
			TCHAR tcTemp[20];
			mbstowcs(tcTemp, pDevice->SerialNumber, strlen(pDevice->SerialNumber) +1);
			swprintf(szTemp, TEXT("%s\\%s"), gszStreamDriverKey, tcTemp);

			dwStatus =	RegCreateKeyEx(
				HKEY_LOCAL_MACHINE,
				szTemp,
				0,
				NULL,
				0,
				0,
				NULL,
				&hKeyUniqueSettings,
				&dwDisp
				);

			//
			// If the user had a persistant registry then next time they installed it they wouldnt get the new settings
			// copied over. Therefore rmeoved the REG_CREATED_NEW_KEY part of things.
			// But if I dont then persistant com port indexes dont work - changing back - need uninstaller instead
			//
			if((dwStatus == ERROR_SUCCESS) && (dwDisp & REG_CREATED_NEW_KEY)) {	// DONT overwrite the key
//			if((dwStatus == ERROR_SUCCESS)) {	// DONT overwrite the key
				// now open and copy over the config data for each setting

				dwDataSize = sizeof(DWORD);
				dwStatus = RegQueryValueEx(
							hKeyDefaultSettings, 
							TEXT("LatencyTimer"),
							NULL, 
							NULL, 
							(LPBYTE)&dwLatency,
							&dwDataSize);

				if(dwStatus == ERROR_SUCCESS) {
					dwStatus =	RegSetValueEx(
									hKeyUniqueSettings,
									TEXT("LatencyTimer"),
									0,
									REG_DWORD,
									(LPBYTE)&dwLatency,
									dwDataSize
									);
				}
				dwStatus = RegQueryValueEx(
										  hKeyDefaultSettings,
										  TEXT("InTransferSize"),
										  NULL,
										  NULL,
										  (LPBYTE)&dwInTransferSize,
										  &dwDataSize);
				if(dwStatus == ERROR_SUCCESS) {
					dwStatus =	RegSetValueEx(
								hKeyUniqueSettings,
								TEXT("InTransferSize"),
								0,
								REG_DWORD,
								(LPBYTE)&dwInTransferSize,
								dwDataSize
								);
				}
				dwStatus = RegQueryValueEx(
										  hKeyDefaultSettings,
										  TEXT("OutTransferSize"),
										  NULL,
										  NULL,
										  (LPBYTE)&dwOutTransferSize,
										  &dwDataSize);
				if(dwStatus == ERROR_SUCCESS) {
					dwStatus =	RegSetValueEx(
								hKeyUniqueSettings,
								TEXT("OutTransferSize"),
								0,
								REG_DWORD,
								(LPBYTE)&dwOutTransferSize,
								dwDataSize
								);
				}

				dwStatus = RegQueryValueEx(
										  hKeyDefaultSettings,
										  TEXT("ConfigDataFlags"),
										  NULL,
										  NULL,
										  (LPBYTE)&dwConfigDataFlags,
										  &dwDataSize);
				if(dwStatus == ERROR_SUCCESS) {
					dwStatus =	RegSetValueEx(
								hKeyUniqueSettings,
								TEXT("ConfigDataFlags"),
								0,
								REG_DWORD,
								(LPBYTE)&dwConfigDataFlags,
								dwDataSize
								);
				}

				dwStatus = RegQueryValueEx(
										  hKeyDefaultSettings,
										  TEXT("InitialIndex"),
										  NULL,
										  NULL,
										  (LPBYTE)&dwInitialIndex,
										  &dwDataSize);
				if(dwStatus == ERROR_SUCCESS) {
					dwStatus =	RegSetValueEx(
								hKeyUniqueSettings,
								TEXT("InitialIndex"),
								0,
								REG_DWORD,
								(LPBYTE)&dwInitialIndex,
								dwDataSize
								);
					if(dwStatus == ERROR_SUCCESS)
						bRet = TRUE;
				}
				dwStatus = RegQueryValueEx(
										  hKeyDefaultSettings,
										  TEXT("MinWriteTimeout"),
										  NULL,
										  NULL,
										  (LPBYTE)&dwConfigDataFlags,
										  &dwDataSize);
				if(dwStatus == ERROR_SUCCESS) {
					dwStatus =	RegSetValueEx(
								hKeyUniqueSettings,
								TEXT("MinWriteTimeout"),
								0,
								REG_DWORD,
								(LPBYTE)&dwConfigDataFlags,
								dwDataSize
								);
				}
				dwStatus = RegQueryValueEx(
										  hKeyDefaultSettings,
										  TEXT("MinReadTimeout"),
										  NULL,
										  NULL,
										  (LPBYTE)&dwConfigDataFlags,
										  &dwDataSize);
				if(dwStatus == ERROR_SUCCESS) {
					dwStatus =	RegSetValueEx(
								hKeyUniqueSettings,
								TEXT("MinReadTimeout"),
								0,
								REG_DWORD,
								(LPBYTE)&dwConfigDataFlags,
								dwDataSize
								);
				}

				dwDataSize = sizeof(cd.ConfigData);
				dwStatus = RegQueryValueEx(
										  hKeyDefaultSettings,
										  TEXT("ConfigData"),
										  NULL,
										  NULL,
										  (LPBYTE)&cd.ConfigData,
										  &dwDataSize);
				if(dwStatus == ERROR_SUCCESS) {
					dwStatus = RegSetValueEx(
							 hKeyUniqueSettings,
							 TEXT("ConfigData"),
							 0,
							 REG_BINARY,
							 (LPBYTE)&cd.ConfigData, //pointer to the beginning of array
							 dwDataSize		// size of the array
							 ); 					
				}
				
				dwDataSize = sizeof(DWORD);
				dwStatus = RegQueryValueEx(
										  hKeyDefaultSettings,
										  TEXT("EmulationMode"),
										  NULL,
										  NULL,
										  (LPBYTE)&dwEmMode,
										  &dwDataSize);
				if(dwStatus == ERROR_SUCCESS) {
					dwStatus = RegSetValueEx(
							 hKeyUniqueSettings,
							 TEXT("EmulationMode"),
							 0,
							 REG_DWORD,
							 (LPBYTE)&dwEmMode, 
							 dwDataSize
							 ); 					
				}

				dwDataSize = sizeof(DWORD);
				dwStatus = RegQueryValueEx(
										  hKeyDefaultSettings,
										  TEXT("ActiveSyncSetting"),
										  NULL,
										  NULL,
										  (LPBYTE)&dwActiveSetting,
										  &dwDataSize);
				if(dwStatus == ERROR_SUCCESS) {
					dwStatus = RegSetValueEx(
							 hKeyUniqueSettings,
							 TEXT("ActiveSyncSetting"),
							 0,
							 REG_DWORD,
							 (LPBYTE)&dwActiveSetting, 
							 dwDataSize
							 ); 					
				}

			}
			else if(dwStatus == ERROR_SUCCESS) {
				// there is a registry entry already for this device
				dwDataSize = sizeof(DWORD);
				dwStatus = RegQueryValueEx(
							  hKeyUniqueSettings,
							  TEXT("InitialIndex"),
							  NULL,
							  NULL,
							  (LPBYTE)&dwInitialIndex,
							  &dwDataSize);
				if(dwStatus == ERROR_SUCCESS) {
					bRet = TRUE;
				}
			}
		}
	}

	if(hKeyDefaultSettings)
		RegCloseKey(hKeyDefaultSettings);
	if(hKeyUniqueSettings)
		RegCloseKey(hKeyUniqueSettings);

	wcscpy(pszDriverKey, szTemp);

	return bRet;
}

BOOL SetIndexKeyValue(
	DWORD * pComPortNumber, 
	TCHAR * pszStreamDriverKey,
	PFTDI_DEVICE pUsbFTDI
	)
{
	DWORD dwIndex = 0;
	HKEY hKey = NULL;
	HKEY hKey1 = NULL;
	DWORD dwDisp;
	DWORD dwStatus;
	BOOL bRet = FALSE;

	//
	// +++ DONT CHANGE THE WAY THIS OPERATES. YOU NEED TO SET THE REGISTRY THAT ActivateDeviceEx
	// POINTS TO to have the Index value of the unique device. That way you can specify which device 
	// has which COM port. BELIEVE ME - you dont want to mess with this funciton.
	//
	dwStatus =	RegCreateKeyEx(
		HKEY_LOCAL_MACHINE,
		pszStreamDriverKey,
		0,
		NULL,
		0,
		0,
		NULL,
		&hKey,
		&dwDisp
		);
	//
	// This is how you set up the index to be zero based
	//
	if (dwStatus == ERROR_SUCCESS) {
		if (pUsbFTDI->usbDeviceInfo.DeviceType != USB_SERIAL_DEVICE_TYPE_INTERNAL_MODEM)
			dwIndex = GetNextAvailableIndex(pszStreamDriverKey, pUsbFTDI);
		if(dwIndex != -1) {

			dwStatus =	RegCreateKeyEx(
				HKEY_LOCAL_MACHINE,
				gszStreamDriverKey,
				0,
				NULL,
				0,
				0,
				NULL,
				&hKey1,
				&dwDisp
				);
			if(dwStatus == ERROR_SUCCESS) {
				dwStatus =	RegSetValueEx(
					hKey1,
					TEXT("Index"),
					0,
					REG_DWORD,
					(const BYTE *)&dwIndex,
					sizeof(DWORD)
					);
			}
			else {
				dwStatus =	RegSetValueEx(
					hKey,
					TEXT("Index"),
					0,
					REG_DWORD,
					(const BYTE *)&dwIndex,
					sizeof(DWORD)
					);
			}
			bRet = TRUE;
		}
	}
	*pComPortNumber = dwIndex;
	DEBUGMSG2(ZONE_FUNCTION, TEXT("SetIndexKeyValue: %d\n"),dwIndex);
	if (hKey)
		RegCloseKey(hKey);
	if (hKey1)
		RegCloseKey(hKey1);
	return bRet;
}

/*++

Routine Description:
	Helper function to get the bulk endpoints ofthe device

Arguments:
	pUsbFTDI		- device structure to alter
	UsbInterface	- Pointer to the USB_INTERFACE structure

Return Value:

    TRUE if successful,
    FALSE otherwise

--*/
BOOL
SetUsbInterface(
   PFTDI_DEVICE pUsbFTDI,
   LPCUSB_INTERFACE UsbInterface
   )
{
	BOOL bRc = FALSE;
	DWORD dwIndex;

	DEBUGMSG1(ZONE_FUNCTION, (TEXT(">SetUsbInterface\n")));

	if(!VALID_CONTEXT(pUsbFTDI) || !UsbInterface) {
		DEBUGMSG1(ZONE_ERROR, (TEXT("Invalid parameter\n")));
		return FALSE;
	}

	//
	// now parse the endpoints
	//
	for(dwIndex = 0; dwIndex < UsbInterface->Descriptor.bNumEndpoints; dwIndex++) {
    
		LPCUSB_ENDPOINT pEndpoint;
      
		pEndpoint = UsbInterface->lpEndpoints + dwIndex;

		DUMP_USB_ENDPOINT_DESCRIPTOR(pEndpoint->Descriptor);

		if(USB_ENDPOINT_DIRECTION_OUT(pEndpoint->Descriptor.bEndpointAddress)) {
			if(NULL == pUsbFTDI->BulkOut.hPipe  &&
				(pEndpoint->Descriptor.bmAttributes & USB_ENDPOINT_TYPE_MASK) == USB_ENDPOINT_TYPE_BULK) {

				//
				// create the Bulk OUT pipe
				//
				pUsbFTDI->BulkOut.hPipe = pUsbFTDI->UsbFuncs->lpOpenPipe( 
																	pUsbFTDI->hUsbDevice,
																	&pEndpoint->Descriptor 
																	);
				if(!pUsbFTDI->BulkOut.hPipe) {
					DEBUGMSG2(ZONE_ERROR, TEXT("OpenPipe out error:%d\n"), GetLastError());
					DEBUGMSG2(ZONE_ERROR, TEXT("hUsbDevice: %p\n"), pUsbFTDI->hUsbDevice );
					DEBUGMSG2(ZONE_ERROR, TEXT("Descriptor: %p\n"), &pEndpoint->Descriptor);
					DEBUGMSG2(ZONE_ERROR, TEXT("OpenPipe: %p\n"), pUsbFTDI->UsbFuncs->lpOpenPipe);
					DEBUGMSG2(ZONE_ERROR, TEXT("hPipe: %p\n"), pUsbFTDI->BulkOut.hPipe);
					bRc = FALSE;
					break;
				}

				DEBUGMSG1(ZONE_ERROR, (TEXT("Out Pipe OK\n")));

				//
				// setup any endpoint specific timers, buffers, context, etc.
				//
				pUsbFTDI->BulkOut.EndpointAddress = pEndpoint->Descriptor.bEndpointAddress;
			}
		} 
		else if(USB_ENDPOINT_DIRECTION_IN(pEndpoint->Descriptor.bEndpointAddress)) {
			if(NULL == pUsbFTDI->BulkIn.hPipe && 
				(pEndpoint->Descriptor.bmAttributes & USB_ENDPOINT_TYPE_MASK) == USB_ENDPOINT_TYPE_BULK) {

				//
				// create the Bulk IN pipe
				//
				pUsbFTDI->BulkIn.hPipe = pUsbFTDI->UsbFuncs->lpOpenPipe( 
																pUsbFTDI->hUsbDevice,
                                                                &pEndpoint->Descriptor 
																);
				if(!pUsbFTDI->BulkIn.hPipe) {
					DEBUGMSG2(ZONE_ERROR, TEXT("OpenPipe in error: %d\n"), GetLastError());
					DEBUGMSG2(ZONE_ERROR, TEXT("hUsbDevice: %p\n"), pUsbFTDI->hUsbDevice);
					DEBUGMSG2(ZONE_ERROR, TEXT("Descriptor: %p\n"), &pEndpoint->Descriptor);
					bRc = FALSE;
					break;
				}
				DEBUGMSG1(ZONE_ERROR, (TEXT("In Pipe OK\n")));

				//
				// setup any endpoint specific timers, buffers, context, etc.
				//
				pUsbFTDI->BulkIn.EndpointAddress = pEndpoint->Descriptor.bEndpointAddress;
				pUsbFTDI->BulkIn.wMaxPacketSize = pEndpoint->Descriptor.wMaxPacketSize;
			}
		} 
		else {
			DEBUGMSG2(ZONE_WARN, TEXT("Unsupported Endpoint:0x%x\n"), pEndpoint->Descriptor.bEndpointAddress);
		}
	}

	//
	// Fix/Fudge for Phillips 176x - have configurable bulk transfer settings
	//
	ConfigureBulkTransfers(pUsbFTDI);

	//
	// did we find our endpoints?
	//
	bRc = (pUsbFTDI->BulkOut.hPipe && pUsbFTDI->BulkIn.hPipe) ? TRUE : FALSE;
	
	//
	// if we failed to find all of our endpoints then cleanup will occur later
	//
	DEBUGMSG2(ZONE_FUNCTION, TEXT("<SetUsbInterface:%d\n"), bRc);
	return bRc;
}

/*++

Routine Description:
	Helper Function to restore the broken handles in a persistant unplug/replug

Arguments:
	pUsbFTDIWrite - where we are writing to
	pUsbFTDIRead - reading the data from

Return Value:


--*/
VOID
RestoreUSBHandles(
	PFTDI_DEVICE pUsbFTDIWrite,
	PFTDI_DEVICE pUsbFTDIRead
	)
{
	pUsbFTDIWrite->pUsbInterface = pUsbFTDIRead->pUsbInterface;
	pUsbFTDIWrite->pUsbDevice = pUsbFTDIRead->pUsbDevice;
	pUsbFTDIWrite->UsbFuncs = pUsbFTDIRead->UsbFuncs;
	pUsbFTDIWrite->hUsbDevice = pUsbFTDIRead->hUsbDevice;
	pUsbFTDIWrite->BulkIn.hPipe = pUsbFTDIRead->BulkIn.hPipe;
	pUsbFTDIWrite->BulkOut.hPipe = pUsbFTDIRead->BulkOut.hPipe;
}

/*++

Routine Description:
	Helper Function to allow the initialisation of the device structure. 
	Keeps it separate from the Activate device part.

Arguments:
	hDevice			- Handle to USB device
	pDevice			- ** to the device structure to add
	UsbFuncs		- Pointer to USB device interface function table
	UsbInterface	- Pointer to the USB_INTERFACE structure
	UsbDriverSettings - pointer to driver settings - used to set the PID

Return Value:

    TRUE if successful,
    FALSE otherwise

--*/
BOOL 
InitialiseDeviceStructure(
	USB_HANDLE			hDevice,
	PFTDI_DEVICE		*pDevice,
	LPCUSB_FUNCS		UsbFuncs,      
	LPCUSB_INTERFACE	UsbInterface,
	LPCUSB_DRIVER_SETTINGS UsbDriverSettings
	)
{
	BOOL bRc = FALSE;
	PFTDI_DEVICE pDev;

	pDev = (PFTDI_DEVICE)FT_LocalAlloc(LPTR, sizeof(FTDI_DEVICE));
	if(!pDev) {
		DEBUGMSG2(ZONE_ERROR, TEXT("LocalAlloc error:%d\n"), GetLastError());
		return FALSE;
	}

	pDev->Sig = USB_COM_SIG;		// Our unique identifier - not really neccesary
	pDev->usbDeviceInfo.DeviceType = USB_SERIAL_DEVICE_TYPE_UNKNOWN;
	pDev->hVspDevice = NULL;

	pDev->pUsbInterface	= UsbInterface;
	pDev->pUsbDevice = UsbFuncs->lpGetDeviceInfo(hDevice);
	pDev->UsbFuncs = UsbFuncs;

	pDev->dwPID = UsbDriverSettings->dwProductId;

	if (IS_MULTI_IF_DEVICE(pDev)) {
		if ((UsbInterface->Descriptor.bInterfaceNumber + 1) == FT2232_INTERFACE_A) {
			pDev->bInterfaceNumber= FT2232_INTERFACE_A;
		}
		else if ((UsbInterface->Descriptor.bInterfaceNumber + 1)  == FT2232_INTERFACE_B){
			pDev->bInterfaceNumber= FT2232_INTERFACE_B;
		}
		if (IS_FT4232H(pDev)) {
			if ((UsbInterface->Descriptor.bInterfaceNumber + 1)  == FT2232_INTERFACE_C) {
				pDev->bInterfaceNumber= FT2232_INTERFACE_C;
			}
			else if ((UsbInterface->Descriptor.bInterfaceNumber + 1)  == FT2232_INTERFACE_D) {
				pDev->bInterfaceNumber= FT2232_INTERFACE_D;
			}
		}
	}
/*	if(IS_FT2232(pDev)) {
		if(UsbInterface->Descriptor.bInterfaceNumber == 0)
			pDev->bInterfaceNumber = FT2232_INTERFACE_A;
		else if(UsbInterface->Descriptor.bInterfaceNumber)
			pDev->bInterfaceNumber = FT2232_INTERFACE_B;
	}
*/
	else
		pDev->bInterfaceNumber = UsbInterface->Descriptor.bInterfaceNumber;

	InitializeCriticalSection(&pDev->Lock);

	pDev->hUsbDevice = hDevice;

	pDev->ConfigIndex = (USHORT)0;
	pDev->Flags.Open = FALSE;
	pDev->Flags.UnloadPending = FALSE;
	pDev->Flags.SurpriseRemoved = FALSE;

	//
	// create endpoint 0 event
	//
	pDev->hEP0Event = CreateEvent(NULL, TRUE, FALSE, NULL);		// manual reset event
	if(!pDev->hEP0Event) {
		DEBUGMSG2(ZONE_ERROR, TEXT("CreateEvent error:%d\n"), GetLastError());
		return FALSE;
	}

	pDev->hCloseEvent = CreateEvent(NULL, FALSE, FALSE, NULL);	// auto reset event
	if(!pDev->hCloseEvent) {
		DEBUGMSG2(ZONE_ERROR, TEXT("CreateEvent error:%d\n"), GetLastError());
		return FALSE;
	}
	else {
		ResetEvent(pDev->hCloseEvent); // non-signaled
	}

	pDev->hReconnectEvent = CreateEvent(NULL, FALSE, FALSE, NULL);	// auto reset event
	if(!pDev->hReconnectEvent) {
		DEBUGMSG2(ZONE_ERROR, TEXT("CreateEvent hReconnectEvent error:%d\n"), GetLastError());
		return FALSE;
	}
	else {
		ResetEvent(pDev->hReconnectEvent); // non-signaled
	}

	//
	// For the bulk in thread
	//
	pDev->hRestartEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if(!pDev->hRestartEvent) {
		DEBUGMSG2(ZONE_ERROR, TEXT("CreateEvent error:%d\n"), GetLastError());
		return FALSE;
	}
	else {
		ResetEvent(pDev->hRestartEvent); // non-signaled
	}

	/* +++ Should have the timeouts in the registry !*/
	pDev->UsbTimeouts.DeviceIdTimeout = GET_PORT_STATUS_TIMEOUT;
	pDev->UsbTimeouts.PortStatusTimeout = GET_DEVICE_ID_TIMEOUT;
	pDev->UsbTimeouts.SoftResetTimeout = SOFT_RESET_TIMEOUT;

	pDev->BreakOnParam = FTDI_BREAK_PARAM_DEFAULT;

	/* What if these fail +++*/
	FT_GetDeviceSerialNumber(pDev);				
	FT_GetDeviceDescription(pDev);

	*pDevice = pDev;

	//
	// setup USB parameters 
	//
	pDev->InParams.MaximumTransferSize = FT_USBD_MAXIMUM_TRANSFER_SIZE;
	pDev->InParams.CurrentTransferSize = FT_USBD_DEFAULT_TRANSFER_SIZE;

	pDev->OutParams.MaximumTransferSize = FT_USBD_MAXIMUM_TRANSFER_SIZE;
	pDev->OutParams.CurrentTransferSize = FT_USBD_DEFAULT_TRANSFER_SIZE;

	// set the USB interface/pipes
	bRc = SetUsbInterface(
			pDev, 
			UsbInterface 
			);

	if(!bRc) {
		DEBUGMSG1(ZONE_ERROR, (TEXT("SetUsbInterface failed!\n")));
		return FALSE;
	}

	pDev->ResetPipeRetries = FTDI_RESET_PIPE_RETRIES;

	return TRUE;
}

/*++

Routine Description:
	Some people wnat the device to return to is current state on and unplug/replug, suspend/resume.
	This function checksthe registry for a valid context and returns a true or false value.

Arguments:
	pFTDICtxt - the device context of the currently connected device - this can change


Return Value:

    TRUE if successful,
    FALSE otherwise

--*/
BOOL
RestoreDeviceInstance(
	PFTDI_DEVICE *pFTDICtxt
	)
{
	BOOL bRet;
	DWORD dwStatus, dwDataSize, dwDevContext;
	HKEY hKeyUniqueSettings = NULL;
	PFTDI_DEVICE pTempCtxt;
	TCHAR szTemp[MAX_PATH + 10];
	TCHAR tcTemp[20];

	//
	// Have we just attached or rebooted?
	//
	if(gbAttached == TRUE) {
		return FALSE;
	}

	//
	// Get the unique registry setting for this driver/device
	//
	GetStreamDriverKey(gszStreamDriverKey);
	mbstowcs(tcTemp, (*pFTDICtxt)->SerialNumber, strlen((*pFTDICtxt)->SerialNumber) +1);
	swprintf(szTemp, TEXT("%s\\%s"), gszStreamDriverKey, tcTemp);

	dwStatus =	RegOpenKeyEx(
					HKEY_LOCAL_MACHINE,
					szTemp,
					0,
					0,
					&hKeyUniqueSettings
					);

	if((dwStatus == ERROR_SUCCESS)) {
		
		//
		// Check the registry for the device pointer value
		//
		dwDataSize = sizeof(DWORD);
		dwStatus = RegQueryValueEx(
					  hKeyUniqueSettings, 
					  TEXT("DeviceContext"),
					  NULL, 
					  NULL, 
					  (LPBYTE)&dwDevContext,
					  &dwDataSize
					  );

		if(dwStatus == ERROR_SUCCESS) {

			//
			// test the valididty of this setting
			//
			pTempCtxt = (PFTDI_DEVICE)dwDevContext;
			
			if(pTempCtxt != NULL) {

				if(VALID_CONTEXT(pTempCtxt)) {

					RestoreUSBHandles(pTempCtxt, *pFTDICtxt);
					RemoveDeviceStructure(*pFTDICtxt, FALSE);	// The original one (on attach)
					*pFTDICtxt = pTempCtxt;						// update the structure to the saved one
					FT_RestoreDeviceSettings(*pFTDICtxt);		// Baud, Flow etc...
//					SetEvent((*pFTDICtxt)->hReconnectEvent);	// for the Notify routine
//					SetEvent((*pFTDICtxt)->hCloseEvent);	// for the Notify routine
//				RETAILMSG(1, (TEXT("SETTING RESTART EVENT!!!\r\n")));
					SetEvent((*pFTDICtxt)->hRestartEvent);		// restart the in task for this device
//					SetEvent((*pFTDICtxt)->hCloseReaderEvent);		// restart the in task for this device
					bRet = TRUE;

					// re-register for USB callbacks
					(*pFTDICtxt)->UsbFuncs->lpRegisterNotificationRoutine(	
												(*pFTDICtxt)->hUsbDevice,
												DeviceNotify,
												*pFTDICtxt
												);

//					MessageBox(NULL,L"Got persistent device",NULL,MB_OK);

				}
				else {

					//
					// We had a DeviceContext registry setting but it was invalid
					//
					bRet = FALSE;
					// Since it is invalid, set it to 0
					dwDevContext = 0;
					dwStatus = RegSetValueEx(
								hKeyUniqueSettings,
								TEXT("DeviceContext"),
								0,
								REG_DWORD,
								(LPBYTE)&dwDevContext,
								sizeof(dwDevContext)
								);
				}
			}
			else {

				//
				// DeviceContext setting was NULL
				//
				bRet = FALSE;
			}
		}
		else {

			//
			// no DeviceContext registry setting
			//
			bRet = FALSE;
		}
	}
	else {

		//
		// no unique registy setting 
		//
		bRet = FALSE;
	}

	if(hKeyUniqueSettings)
		RegCloseKey(hKeyUniqueSettings);

	return bRet;

}

/*++

Routine Description:
	The driver may have been replaced therefore update the version. This will only work from version 1.0.1.4 onwards and
	1.0.1.3s1 (Thales special).

Arguments:

	
Return Value:

    TRUE if successful,
    FALSE otherwise

--*/
BOOL UpdateDriverVersion(
		TCHAR * pszStreamDriverKey
		)
{
	HKEY hKey = NULL;
	DWORD dwDisp;
	DWORD dwStatus;
	BOOL bRet = FALSE;

	dwStatus =	RegCreateKeyEx(
		HKEY_LOCAL_MACHINE,
		pszStreamDriverKey,
		0,
		NULL,
		0,
		0,
		NULL,
		&hKey,
		&dwDisp
		);
	// This is how you set up the index to be zero based
	if (dwStatus == ERROR_SUCCESS) {
		dwStatus = RegSetValueEx(
						hKey, 
						gszVersion,
						0,
						REG_SZ,
						(const BYTE *)gszVersionNumber,
						sizeof(gszVersionNumber)
						);

		if (dwStatus == ERROR_SUCCESS) {
			bRet = TRUE;
		}
	}
	if (hKey)
		RegCloseKey(hKey);

	return bRet;
}

/*++

Routine Description:

	The USB driver module calls this function when a device is attached and a matching registry key 
	is found in the LoadClients registry key (this is setup in USBInstallDriver

// see C:\WINCE420\PRIVATE\WINCEOS\COREOS\DEVICE\LIB\device.c for ActivateDeviceEx source
// see C:\WINCE420\PRIVATE\WINCEOS\COREOS\DEVICE\LIB\devload.c for StartOneDriver source


Arguments:

	hDevice				- Handle to USB device
	UsbFuncs			- Pointer to USB device interface function table
	UsbInterface		- Pointer to the USB_INTERFACE structure (Could be NULL)
	UniqueDriverId		- Client driver identifier string (the name of the dll minus the .dll extension)
	AcceptControl		- We set this pointer to TRUE
	UsbDriverSettings	- Pointer to the USB_DRIVER_SETTINGS structure that indicates how the driver is loaded
						  Looks like a copy of the registry settings	
	Unused				- Mmmm

Return Value:

    TRUE if successful,
    FALSE otherwise

--*/
BOOL
USBDeviceAttach(
	USB_HANDLE       hDevice,
	LPCUSB_FUNCS     UsbFuncs,      
	LPCUSB_INTERFACE UsbInterface,
	LPCWSTR          UniqueDriverId,   
	LPBOOL           AcceptControl,      
	LPCUSB_DRIVER_SETTINGS UsbDriverSettings,
	DWORD Unused
	)
{
	BOOL bPersistentDevice = FALSE;
	BOOL bRc = TRUE;
	DWORD err = 0;
	DWORD dwComPortNumber = -1;
    LPCUSB_DEVICE pUsbDevice = NULL;
	PFTDI_DEVICE pFTDICtxt = NULL;
	REGINI rega[8];
	REGINI regb[7];

	TCHAR szLegacyDevName[7];
	TCHAR szDevName[30];

	TCHAR tcTemp[128];
	TCHAR szSerialNumber[20];
	TCHAR szDescription[64];
	TCHAR szChipType[20];
	TCHAR szUsbSpeed[20];

	DWORD dwVID;
	DWORD dwPID;

	TCHAR szTemp[MAX_PATH + 10];
	ULONG ulConfigIndex = 0;

	// Called once per interface!  

	DEBUGMSG3(ZONE_FUNCTION, TEXT(">USBDeviceAttach(0x%x, %s)\n"), hDevice, UniqueDriverId);

	//GetCurrentStreamDriverKey(gszStreamDriverKey, UsbDriverSettings->dwProductId);
	GetStreamDriverKey(gszStreamDriverKey);
	GetDevicePrefix(gszStreamDriverKey, gszDriverPrefix, PREFIX_SIZE);

	*AcceptControl = FALSE;			// Dont Accept yet
    pUsbDevice = UsbFuncs->lpGetDeviceInfo(hDevice);

//	RETAILMSG(1, (TEXT("USBDeviceAttach - %d interfaces found\r\n"), (int)pUsbDevice->lpConfigs->dwNumInterfaces));

	if (UsbInterface)
	{
//		RETAILMSG(1, (TEXT("USBDeviceAttach - information for interface %d\r\n"), UsbInterface->Descriptor.bInterfaceNumber));
	}
	else
	{
//		RETAILMSG(1, (TEXT("USBDeviceAttach - no interface information\r\n")));
		return FALSE;
	}

// Only perform actions on one interface at a time... therefore do not loop on number of interfaces
//	for(i = 0; i < (int)pUsbDevice->lpConfigs->dwNumInterfaces ; i++) {
		
	do {
//		RETAILMSG(1, (TEXT("USBDeviceAttach - interface %d\r\n"), UsbInterface->Descriptor.bInterfaceNumber));

//		UsbInterface = UsbFuncs->lpFindInterface(pUsbDevice, (UCHAR)i, 0);
//		UsbInterface = UsbFuncs->lpFindInterface(pUsbDevice, UsbInterface->Descriptor.bInterfaceNumber, 0);
		bRc = InitialiseDeviceStructure(
				hDevice, 
				&pFTDICtxt, 
				UsbFuncs, 
				UsbInterface,
				UsbDriverSettings
				);

		// Could set up notify callback here?.....


		//
		// Now check if persistant instance is required
		//
		bPersistentDevice = RestoreDeviceInstance(&pFTDICtxt);
		//*AcceptControl = RestoreDeviceInstance(&pFTDICtxt);

		if(bPersistentDevice == TRUE) {
			DEBUGMSG1(ZONE_FUNCTION, TEXT("Persistent device - resuming context\n"));
			SetDeviceLoading(pFTDICtxt);
			continue;
		}

//		RETAILMSG(1, (TEXT("USBDeviceAttach - not persistent\r\n")));
		DEBUGMSG2(ZONE_FUNCTION, TEXT("gszStreamDriverKey = %s\n"), gszStreamDriverKey);

//		EnterCriticalSection(&gAttachCS);

		HANDLE hHCD = CreateFile(TEXT("EHC1:"), 0, 0, NULL, 0, 0, NULL);
		if (hHCD != INVALID_HANDLE_VALUE)
		{
			UCHAR address = ((SDevice *)hDevice)->iDevice;
			USB_DEVICE_EXT_INFO deviceInfo = {0};
			DWORD dwActualOut = 0;
			if (DeviceIoControl(hHCD, IOCTL_GET_USB_DEVICE_EXT_INFO, &address, sizeof(address), &deviceInfo, sizeof(deviceInfo), &dwActualOut, NULL))
			{
				if (dwActualOut >= sizeof(deviceInfo) && deviceInfo.Tier == 2 && deviceInfo.Port == 4)
					pFTDICtxt->usbDeviceInfo.DeviceType = USB_SERIAL_DEVICE_TYPE_INTERNAL_MODEM;

				pFTDICtxt->usbDeviceInfo.UsbPort = deviceInfo.Port;
				pFTDICtxt->usbDeviceInfo.UsbTier = deviceInfo.Tier;
				RETAILMSG(1, (TEXT(">USBDeviceAttach, tier = %d, port = %d, type = %d\n"), deviceInfo.Tier, deviceInfo.Port, pFTDICtxt->usbDeviceInfo.DeviceType));
			}
			CloseHandle(hHCD);
		}

		//
		// This is required to set the index value in the root registry - required for 0 based COM port start - DO NOT MOVE
		//
		if(SetIndexKeyValue(&dwComPortNumber, gszStreamDriverKey, pFTDICtxt) != TRUE) {
			MessageBox(NULL, TEXT("Device not installed - free index not found"), TEXT("Error"), MB_OK);
//			RETAILMSG(1, (TEXT("Free index not found 1\r\n")));
			bRc = FALSE;
		}	

		//
		// Now create the serial numbered registry version
		//
//		RETAILMSG(1, (TEXT("CreateUniqueDriverSettings...\r\n")));
		if(CreateUniqueDriverSettings(pFTDICtxt, szTemp) == TRUE) {
//			RETAILMSG(1, (TEXT("CreateUniqueDriverSettings true\r\n")));
			//
			// szTemp now holds the full registry key (Drivers\USB\FTDI_6001\FT000001)
			//
			if(SetIndexKeyValue(&dwComPortNumber, szTemp, pFTDICtxt) != TRUE) {
				
//				RETAILMSG(1, (TEXT("Free index not found 2\r\n")));
				MessageBox(NULL, TEXT("Device not installed - free index not found"), TEXT("Error"), MB_OK);
				bRc = FALSE;
			}
		}
		
		if (pFTDICtxt->usbDeviceInfo.DeviceType == USB_SERIAL_DEVICE_TYPE_INTERNAL_MODEM)
			dwComPortNumber = 0; // MOM0 is reserved for internal modem

//		LeaveCriticalSection(&gAttachCS);

//		RETAILMSG(1, (TEXT("bRc = %d\r\n"), bRc));
				
		if(bRc) {

			//
			// Update our version
			//
			UpdateDriverVersion(gszStreamDriverKey);

			dwVID = (DWORD)pFTDICtxt->pUsbDevice->Descriptor.idVendor;
			dwPID = (DWORD)pFTDICtxt->pUsbDevice->Descriptor.idProduct;

			if ((osv.dwMajorVersion >= 5) && (dwComPortNumber < 10)) {
				// If we are on CE 5.0 or later, we need to support up to 255 COM ports.
				// Need to use the FullName key under Active\Drivers to do this.
				// CreateFile must ask to open \$device\COMXXX for these ports
				wsprintf(szDevName, TEXT("%s%d"), gszDriverPrefix, dwComPortNumber);		// Give ourselves a unique name 
				rega[0].lpszVal	= DEVLOAD_DEVFULLNAME_VALNAME;
				rega[0].dwType	= DEVLOAD_DEVFULLNAME_VALTYPE;
				rega[0].pData	= (LPBYTE)szDevName;
				rega[0].dwLen	= (wcslen(szDevName) + 1) * sizeof(TCHAR);

				// On all version of CE we want to support the legacy naming convention
				// of COMX:  - Need to use the Name key under Active\Drivers to do this.
				wsprintf(szLegacyDevName, TEXT("%s%d:"), gszDriverPrefix, dwComPortNumber);		// Give ourselves a unique name 
				rega[1].lpszVal	= DEVLOAD_DEVNAME_VALNAME;
				rega[1].dwType	= DEVLOAD_DEVNAME_VALTYPE;
				rega[1].pData	= (LPBYTE)szLegacyDevName;
				rega[1].dwLen	= (wcslen(szLegacyDevName) + 1) * sizeof(TCHAR);

				// Also want to include device information (serial number, description)
				mbstowcs(tcTemp, pFTDICtxt->SerialNumber, strlen(pFTDICtxt->SerialNumber) +1);
				swprintf(szSerialNumber, TEXT("%s"), tcTemp);
				rega[2].lpszVal	= DEVLOAD_DEVSERIALNUMBER_VALNAME;
				rega[2].dwType	= DEVLOAD_DEVSERIALNUMBER_VALTYPE;
				rega[2].pData	= (LPBYTE)szSerialNumber;
				rega[2].dwLen	= (wcslen(szSerialNumber) + 1) * sizeof(TCHAR);

				mbstowcs(tcTemp, pFTDICtxt->Description, strlen(pFTDICtxt->Description) +1);
				swprintf(szDescription, TEXT("%s"), tcTemp);
				rega[3].lpszVal	= DEVLOAD_DEVDESCRIPTION_VALNAME;
				rega[3].dwType	= DEVLOAD_DEVDESCRIPTION_VALTYPE;
				rega[3].pData	= (LPBYTE)szDescription;
				rega[3].dwLen	= (wcslen(szDescription) + 1) * sizeof(TCHAR);

				// ...and chip type!
				rega[4].lpszVal	= DEVLOAD_DEVCHIPTYPE_VALNAME;
				rega[4].dwType	= DEVLOAD_DEVCHIPTYPE_VALTYPE;
				switch (pFTDICtxt->pUsbDevice->Descriptor.bcdDevice) {
					case BCD_FT232EX:
						swprintf(szChipType, TEXT("FT X Series"));
						break;
					case BCD_FT232H:
						swprintf(szChipType, TEXT("FT232H"));
						break;
					case BCD_FT4232H:
						swprintf(szChipType, TEXT("FT4232H"));
						break;
					case BCD_FT2232H:
						swprintf(szChipType, TEXT("FT2232H"));
						break;
					case BCD_FT232R:
						swprintf(szChipType, TEXT("FT232R"));
						break;
					case BCD_FT2232:
						swprintf(szChipType, TEXT("FT2232"));
						break;
					case BCD_FT232B:
						swprintf(szChipType, TEXT("FT232B"));
						break;
					case BCD_FT8U232AM:
						swprintf(szChipType, TEXT("FT8U232AM"));
						break;
					default:
						swprintf(szChipType, TEXT("Unknown"));
						break;
				}
				rega[4].pData	= (LPBYTE)szChipType;
				rega[4].dwLen	= (wcslen(szChipType) + 1) * sizeof(TCHAR);

				// ...and speed (as enumerated!)
				rega[5].lpszVal	= DEVLOAD_DEVUSBSPEED_VALNAME;
				rega[5].dwType	= DEVLOAD_DEVUSBSPEED_VALTYPE;
				switch (pFTDICtxt->BulkIn.wMaxPacketSize) {
					case FT_USBD_PACKET_SIZE_HI:
						swprintf(szUsbSpeed, TEXT("Hi-Speed"));
						break;
					default:
						swprintf(szUsbSpeed, TEXT("Full-Speed"));
						break;
				}
				rega[5].pData	= (LPBYTE)szUsbSpeed;
				rega[5].dwLen	= (wcslen(szUsbSpeed) + 1) * sizeof(TCHAR);

				// ...and VID
				rega[6].lpszVal	= DEVLOAD_DEVVID_VALNAME;
				rega[6].dwType	= DEVLOAD_DEVVID_VALTYPE;
				rega[6].pData	= (LPBYTE)&dwVID;
				rega[6].dwLen	= sizeof(DWORD);

				// ...and PID!
				rega[7].lpszVal	= DEVLOAD_DEVPID_VALNAME;
				rega[7].dwType	= DEVLOAD_DEVPID_VALTYPE;
				rega[7].pData	= (LPBYTE)&dwPID;
				rega[7].dwLen	= sizeof(DWORD);

				pFTDICtxt->hStreamDevice = ActivateDeviceEx(
											gszStreamDriverKey,
											&rega, 
											sizeof(rega)/sizeof(rega[0]), 
											(PVOID)pFTDICtxt
											);
			}
			else if ((osv.dwMajorVersion >= 5) && (dwComPortNumber >= 10)) {
				// If we are on CE 5.0 or later, we need to support up to 255 COM ports.
				// Need to use the FullName key under Active\Drivers to do this.
				// CreateFile must ask to open \$device\COMXXX for these ports
				wsprintf(szDevName, TEXT("%s%d"), gszDriverPrefix, dwComPortNumber);		// Give ourselves a unique name 
				regb[0].lpszVal	= DEVLOAD_DEVFULLNAME_VALNAME;
				regb[0].dwType	= DEVLOAD_DEVFULLNAME_VALTYPE;
				regb[0].pData	= (LPBYTE)szDevName;
				regb[0].dwLen	= (wcslen(szDevName) + 1) * sizeof(TCHAR);

				// Also want to include device information (serial number, description)
				wsprintf(szSerialNumber, TEXT("%s"), pFTDICtxt->SerialNumber); 
				regb[1].lpszVal	= DEVLOAD_DEVSERIALNUMBER_VALNAME;
				regb[1].dwType	= DEVLOAD_DEVSERIALNUMBER_VALTYPE;
				regb[1].pData	= (LPBYTE)szSerialNumber;
				regb[1].dwLen	= (wcslen(szSerialNumber) + 1) * sizeof(TCHAR);

				wsprintf(szDescription, TEXT("%s"), pFTDICtxt->Description);
				regb[2].lpszVal	= DEVLOAD_DEVDESCRIPTION_VALNAME;
				regb[2].dwType	= DEVLOAD_DEVDESCRIPTION_VALTYPE;
				regb[2].pData	= (LPBYTE)szDescription;
				regb[2].dwLen	= (wcslen(szDescription) + 1) * sizeof(TCHAR);

				// ...and chip type!
				regb[3].lpszVal	= DEVLOAD_DEVCHIPTYPE_VALNAME;
				regb[3].dwType	= DEVLOAD_DEVCHIPTYPE_VALTYPE;
				switch (pFTDICtxt->pUsbDevice->Descriptor.bcdDevice) {
					case BCD_FT232EX:
						swprintf(szChipType, TEXT("FT X Series"));
						break;
					case BCD_FT232H:
						swprintf(szChipType, TEXT("FT232H"));
						break;
					case BCD_FT4232H:
						swprintf(szChipType, TEXT("FT4232H"));
						break;
					case BCD_FT2232H:
						swprintf(szChipType, TEXT("FT2232H"));
						break;
					case BCD_FT232R:
						swprintf(szChipType, TEXT("FT232R"));
						break;
					case BCD_FT2232:
						swprintf(szChipType, TEXT("FT2232"));
						break;
					case BCD_FT232B:
						swprintf(szChipType, TEXT("FT232B"));
						break;
					case BCD_FT8U232AM:
						swprintf(szChipType, TEXT("FT8U232AM"));
						break;
					default:
						swprintf(szChipType, TEXT("Unknown"));
						break;
				}
				regb[3].pData	= (LPBYTE)szChipType;
				regb[3].dwLen	= (wcslen(szChipType) + 1) * sizeof(TCHAR);

				// ...and speed (as enumerated!)
				regb[4].lpszVal	= DEVLOAD_DEVUSBSPEED_VALNAME;
				regb[4].dwType	= DEVLOAD_DEVUSBSPEED_VALTYPE;
				switch (pFTDICtxt->BulkIn.wMaxPacketSize) {
					case FT_USBD_PACKET_SIZE_HI:
						swprintf(szUsbSpeed, TEXT("Hi-Speed"));
						break;
					default:
						swprintf(szUsbSpeed, TEXT("Full-Speed"));
						break;
				}
				regb[4].pData	= (LPBYTE)szUsbSpeed;
				regb[4].dwLen	= (wcslen(szUsbSpeed) + 1) * sizeof(TCHAR);

				// ...and VID
				regb[5].lpszVal	= DEVLOAD_DEVVID_VALNAME;
				regb[5].dwType	= DEVLOAD_DEVVID_VALTYPE;
				regb[5].pData	= (LPBYTE)&dwVID;
				regb[5].dwLen	= sizeof(DWORD);

				// ...and PID!
				regb[6].lpszVal	= DEVLOAD_DEVPID_VALNAME;
				regb[6].dwType	= DEVLOAD_DEVPID_VALTYPE;
				regb[6].pData	= (LPBYTE)&dwPID;
				regb[6].dwLen	= sizeof(DWORD);

				pFTDICtxt->hStreamDevice = ActivateDeviceEx(
											gszStreamDriverKey,
											&regb, 
											sizeof(regb)/sizeof(regb[0]), 
											(PVOID)pFTDICtxt
											);
			}	
			/*else if (osv.dwMajorVersion < 5) {
				// On all version of CE we want to support the legacy naming convention
				// of COMX:  - Need to use the Name key under Active\Drivers to do this.
				wsprintf(szLegacyDevName, TEXT("%s%d:"), gszDriverPrefix, dwComPortNumber);		// Give ourselves a unique name 
				regb[0].lpszVal	= DEVLOAD_DEVNAME_VALNAME;
				regb[0].dwType	= DEVLOAD_DEVNAME_VALTYPE;
				regb[0].pData	= (LPBYTE)szLegacyDevName;
				regb[0].dwLen	= sizeof(szLegacyDevName)/sizeof(TCHAR);

				// Also want to include device information (serial number, description)
				wsprintf(szSerialNumber, TEXT("%s"), pFTDICtxt->SerialNumber); 
				regb[1].lpszVal	= DEVLOAD_DEVSERIALNUMBER_VALNAME;
				regb[1].dwType	= DEVLOAD_DEVSERIALNUMBER_VALTYPE;
				regb[1].pData	= (LPBYTE)szSerialNumber;
				regb[1].dwLen	= sizeof(szSerialNumber)/sizeof(TCHAR);

				wsprintf(szDescription, TEXT("%s"), pFTDICtxt->Description);
				regb[2].lpszVal	= DEVLOAD_DEVDESCRIPTION_VALNAME;
				regb[2].dwType	= DEVLOAD_DEVDESCRIPTION_VALTYPE;
				regb[2].pData	= (LPBYTE)szDescription;
				regb[2].dwLen	= sizeof(szDescription)/sizeof(TCHAR);

				// ...and chip type!
				regb[3].lpszVal	= DEVLOAD_DEVCHIPTYPE_VALNAME;
				regb[3].dwType	= DEVLOAD_DEVCHIPTYPE_VALTYPE;
				switch (pFTDICtxt->pUsbDevice->Descriptor.bcdDevice) {
					case BCD_FT232EX:
						swprintf(szChipType, TEXT("FT X Series"));
						break;
					case BCD_FT232H:
						swprintf(szChipType, TEXT("FT232H"));
						break;
					case BCD_FT4232H:
						swprintf(szChipType, TEXT("FT4232H"));
						break;
					case BCD_FT2232H:
						swprintf(szChipType, TEXT("FT2232H"));
						break;
					case BCD_FT232R:
						swprintf(szChipType, TEXT("FT232R"));
						break;
					case BCD_FT2232:
						swprintf(szChipType, TEXT("FT2232"));
						break;
					case BCD_FT232B:
						swprintf(szChipType, TEXT("FT232B"));
						break;
					case BCD_FT8U232AM:
						swprintf(szChipType, TEXT("FT8U232AM"));
						break;
					default:
						swprintf(szChipType, TEXT("Unknown"));
						break;
				}
				regb[3].pData	= (LPBYTE)szChipType;
				regb[3].dwLen	= sizeof(szChipType)/sizeof(TCHAR);

				// ...and speed (as enumerated!)
				regb[4].lpszVal	= DEVLOAD_DEVUSBSPEED_VALNAME;
				regb[4].dwType	= DEVLOAD_DEVUSBSPEED_VALTYPE;
				switch (pFTDICtxt->BulkIn.wMaxPacketSize) {
					case FT_USBD_PACKET_SIZE_HI:
						swprintf(szUsbSpeed, TEXT("Hi-Speed"));
						break;
					default:
						swprintf(szUsbSpeed, TEXT("Full-Speed"));
						break;
				}
				regb[4].pData	= (LPBYTE)szUsbSpeed;
				regb[4].dwLen	= sizeof(szUsbSpeed)/sizeof(TCHAR);

				// ...and VID
				regb[5].lpszVal	= DEVLOAD_DEVVID_VALNAME;
				regb[5].dwType	= DEVLOAD_DEVVID_VALTYPE;
				regb[5].pData	= (LPBYTE)&dwVID;
				regb[5].dwLen	= sizeof(DWORD);

				// ...and PID!
				regb[6].lpszVal	= DEVLOAD_DEVPID_VALNAME;
				regb[6].dwType	= DEVLOAD_DEVPID_VALTYPE;
				regb[6].pData	= (LPBYTE)&dwPID;
				regb[6].dwLen	= sizeof(DWORD);

				pFTDICtxt->hStreamDevice = ActivateDeviceEx(
											gszStreamDriverKey,
											&regb, 
											sizeof(regb)/sizeof(regb[0]), 
											(PVOID)pFTDICtxt
											);
			}*/


			DEBUGMSG2(ZONE_FUNCTION, TEXT("ComPort Name = %s\r\n"), szLegacyDevName);

			if(pFTDICtxt->hStreamDevice) {
				RETAILMSG(1, (TEXT("ActivateDevice SUCCEEDED - %s\r\n"), szDevName));
				// register for USB callbacks
				bRc = UsbFuncs->lpRegisterNotificationRoutine(	
									hDevice,
									DeviceNotify,
									pFTDICtxt
									);

				REGINI reg[] = 
				{
					{ TEXT("PortName"), (LPBYTE)szLegacyDevName, (wcslen(szLegacyDevName) + 1) * sizeof(TCHAR), REG_SZ }
				};

				if (pFTDICtxt->usbDeviceInfo.DeviceType != USB_SERIAL_DEVICE_TYPE_INTERNAL_MODEM)
				{
					pFTDICtxt->hVspDevice = ActivateDeviceEx(TEXT("Drivers\\VSP"), &reg, sizeof(reg)/sizeof(reg[0]), (PVOID)pFTDICtxt);
					RETAILMSG(1, (TEXT("ActivateDeviceEx for %s returned %p, error = %d\r\n"), szLegacyDevName, pFTDICtxt->hVspDevice, GetLastError()));
				}
			} 
			else {
				//
				// the streams interface failed to init, no use starting.
				//
//				RETAILMSG(1, (TEXT("ActivateDevice FAILED!! %d\r\n"), GetLastError()));
				DEBUGMSG2(ZONE_ERROR, TEXT("ActivateDevice error:%d\n"), GetLastError());
				bRc = FALSE;
			}
		}

		if(bRc) {
//			*AcceptControl = TRUE;
			ConfigureActiveSyncMonitor(pFTDICtxt);
		} else {
//			RETAILMSG(1, (TEXT("USBDeviceAttach Falied to initialise\r\n")));
			DEBUGMSG2(ZONE_ERROR, TEXT("Falied to initialise error:%d\n"), GetLastError());
			RemoveDeviceStructure(pFTDICtxt, TRUE);	// If not our device, or error, then clean up
		}  

		// These are in FT_Create
		pFTDICtxt->DCB.DCBlength = sizeof(FTDCB);
//	} // end for
	} while (0);

	// OK, AcceptControl for this interface
	if (bRc) {
//		RETAILMSG(1, (TEXT("USBDeviceAttach SUCCEEDED - accepting control\r\n")));
		*AcceptControl = TRUE;
	}

	gbAttached = FALSE;

//	RETAILMSG(1, (TEXT("USBDeviceAttach - end\r\n")));
 	return bRc;
}
