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
#include "lanconninfo.h"
#include <nuiouser.h>
#include <ras.h>
#include <raserror.h>


void ValidateName(TCHAR * pszNewName, UINT * pError);

////////////////////////
// LanConnInfo
////////////////////////

const DWORD LanConnInfo::dwMaxFriendlyNameLength = 128;
const TCHAR LanConnInfo::szNameMappingRegistryPath[] = _T("Comm\\AdapterNameMappings");
const TCHAR LanConnInfo::szWirelessCacheRegistryPath[] = _T("Comm\\WirelessAdapterCache");

UINT LanConnInfo::enabledLanIconID = (UINT) -1;
UINT LanConnInfo::disabledLanIconID = (UINT) -1;
UINT LanConnInfo::enabledWLanIconID = (UINT) -1;
UINT LanConnInfo::disabledWLanIconID = (UINT) -1;

TCHAR LanConnInfo::szAdapterNames[] = {0};
TCHAR * LanConnInfo::pCurrentAdapter = NULL;

LanConnInfo::LanConnInfo(TCHAR * pszName) :
   dwAddress(0),
   dwGatewayAddress(0),
   fEnabled(FALSE)
{
   if (!pszName)
      return;

   // Determine if we are a valid lan type
   TCHAR szAdapterBindings[512] = {0};
   DWORD cbAdapterBindings = sizeof(szAdapterBindings);
   if (DoNdisIOControl(IOCTL_NDIS_GET_ADAPTER_BINDINGS,
                       pszName, (_tcslen(pszName)+1)*sizeof(TCHAR),
                       szAdapterBindings, &cbAdapterBindings))
   {
      // Loop through all of the bindings for this adapter looking for tcp/ip
      TCHAR * pCurrentBinding = szAdapterBindings;
      while (_T('\0') != *pCurrentBinding)
      {
         if (!::_tcsicmp(pCurrentBinding, _T("TCPIP")) ||
             !::_tcsicmp(pCurrentBinding, _T("MSTCP")))
         {
            fEnabled = TRUE;
            break;
         }
         pCurrentBinding += ::_tcslen(pCurrentBinding)+1;
      }
   }

   // Set the internal name
   pszAdapter = new TCHAR[::_tcslen(pszName)+1];
   if (pszAdapter)
      ::_tcscpy(pszAdapter, pszName);
   else
      return;
  
   // Get the friendly name from the registry
   if (!readFriendlyName())
   {
      this->pszName = new TCHAR[::_tcslen(pszAdapter)+1];
      if (this->pszName)
      {
         ::_tcscpy(this->pszName, pszAdapter);

         // Replace all '\' in the name
         TCHAR * pch = ::_tcschr(this->pszName, _T('\\'));
         while (pch)
         {
            *pch = _T('-'); // Replace the '\'
            pch = ::_tcsrchr(pch, _T('\\'));
         }
      }
   }

   // Determine if we support power managment  
   CEDEVICE_POWER_STATE Dx = PwrDeviceUnspecified;
   TCHAR                szName[MAX_PATH];
   int                  nChars;

   nChars = _sntprintf(
                szName, 
                MAX_PATH-1, 
                _T("%s\\%s"), 
                PMCLASS_NDIS_MINIPORT, 
                pszAdapter);
   
    szName[MAX_PATH-1]=0;
    
    if(nChars != (-1)) 
       ::GetDevicePower(szName, POWER_NAME, &Dx);

   fSupportsPowerManagment = IsValidDx(Dx);

   // Grab the ip address info
   if (fEnabled)
   {
      PIP_ADAPTER_INFO pAdapterInfo = NULL;
      PIP_ADAPTER_INFO pCurrentAdapterInfo = NULL;
      DWORD cbAdapterInfo = 0;
      if (ERROR_BUFFER_OVERFLOW == GetAdaptersInfo(pAdapterInfo, &cbAdapterInfo))
      {
         pAdapterInfo = reinterpret_cast<PIP_ADAPTER_INFO>(new BYTE[cbAdapterInfo]);
         ASSERT(pAdapterInfo);
         if (pAdapterInfo)
         {
            if (ERROR_SUCCESS == GetAdaptersInfo(pAdapterInfo, &cbAdapterInfo) && cbAdapterInfo != 0)
            {
               CHAR szAdapter[256];
#ifdef UNICODE
               ::wcstombs(szAdapter, pszAdapter, ::_tcslen(pszAdapter)+1);
#else
               ::strncpy(szAdapter, pszAdapter, ::_tcslen(pszAdapter)+1);
#endif
               pCurrentAdapterInfo = pAdapterInfo;
               while (pCurrentAdapterInfo)
               {
                  if (!::strcmp(szAdapter, pCurrentAdapterInfo->AdapterName))
                  {
                     dwAddress = ::inet_addr(pCurrentAdapterInfo->IpAddressList.IpAddress.String);
                     dwGatewayAddress = ::inet_addr(pCurrentAdapterInfo->GatewayList.IpAddress.String);
                     break;
                  }
                  pCurrentAdapterInfo = pCurrentAdapterInfo->Next;
               }
            }
            delete [] pAdapterInfo;
         }
      }
   }

   // Determine wireless attribute
   fWireless = isWireless();
}


LanConnInfo::~LanConnInfo()
{
   if (pszAdapter)
      delete [] pszAdapter;
}

LanConnInfo * LanConnInfo::FindFirst()
{
   DWORD cbAdapterNames = sizeof(szAdapterNames);
   if (DoNdisIOControl(IOCTL_NDIS_GET_ADAPTER_NAMES, NULL, 0,
                       &szAdapterNames, &cbAdapterNames))
   {
      pCurrentAdapter = szAdapterNames;
      return FindNext();
   }
   return NULL;
}

LanConnInfo * LanConnInfo::FindNext()
{
   LanConnInfo * pLanConnInfo = NULL;
   BOOL Filter = FALSE;

   // Need to call FindFirst first
   if (!pCurrentAdapter)
      return NULL;

   // Find the next LAN adapter
   while ((_T('\0') != *pCurrentAdapter) && !pLanConnInfo)
   {
      pLanConnInfo = new LanConnInfo(pCurrentAdapter);
      if (pLanConnInfo)
      {
		  if (pLanConnInfo->isValid(NULL))
		  {
			  // Filter out RAS connections
			  DWORD RetCode = IsRasVEMAdapter(pLanConnInfo->pszAdapter, &Filter);
		  }else{
			  Filter = TRUE;
		  }

		  if (Filter)
		  {
			  Filter = FALSE;
			  delete pLanConnInfo;
			  pLanConnInfo = NULL;
		  }
      }
	 
	  pCurrentAdapter += ::_tcslen(pCurrentAdapter)+1;
   }

   // Reset pCurrentAdapter on a failure or end of list
   if (!pLanConnInfo)
      pCurrentAdapter = NULL;

   return pLanConnInfo;
}

BOOL LanConnInfo::isValid(UINT * pError) const
{
	if (!pszAdapter)
	{
		if (pError)
			*pError = IDS_ERR_BADNAME;
		return FALSE;
	}
	
	
	// Do error checking here
	ASSERT(isValidConnInfoName(pszName));
	if (!isValidConnInfoName(pszName))
	{
		if (pError)
			*pError = IDS_ERR_BADNAME;
		return FALSE;
	}
	
	
	// If the adapter is not enabled and it is bound that means it is
	// not a valid LAN connectoid
	TCHAR szAdapterBindings[512] = {0};
	DWORD cbAdapterBindings = sizeof(szAdapterBindings);
	if (DoNdisIOControl(IOCTL_NDIS_GET_ADAPTER_BINDINGS,
		pszName, (_tcslen(pszAdapter)+1)*sizeof(TCHAR),
		szAdapterBindings, &cbAdapterBindings))
	{
		if (!fEnabled && *szAdapterBindings)
		{
			if (pError)
				*pError = IDS_ERROR;
			
			return FALSE;
		}
	}
	
	if (fSupportsPowerManagment)
	{     
		CEDEVICE_POWER_STATE Dx = PwrDeviceUnspecified;     
		TCHAR                szName[MAX_PATH];
		int                  nChars;
		
		nChars = _sntprintf(
			szName, 
			MAX_PATH-1, 
			_T("%s\\%s"), 
			PMCLASS_NDIS_MINIPORT, 
			pszAdapter);
		
		szName[MAX_PATH-1]=0;
        
		if(nChars != (-1)) 
			::GetDevicePower(szName, POWER_NAME, &Dx);
		
		
		if (!IsValidDx(Dx))
		{
			if (pError)
				*pError = IDS_ERROR;
			return FALSE;
		}
		
		// if we are in power state D0 (Full Power) and not enabled then we 
		// have been unbound by something other than CONNMC and so we shouldn't
		// display the icon.  If Disabled by CONNMC we should be in state D4 (No power)
		if ((D0 == Dx) && !fEnabled)
		{
			if (pError)
				*pError = IDS_ERROR;
			return FALSE;
		}
		
		
	}
	
	//   When we are disabled isWireless fails so this check is invalid
	//   if (isWireless() && !fWireless)
	//   {
	//      if (pError)
	//         *pError = IDS_ERROR;
	//      return FALSE;
	//   }
	
	return TRUE;
}

BOOL LanConnInfo::getDefaultActionString(HINSTANCE hInstance, TCHAR * pszDefaultAction, UINT * pError)
{
   if (!pszDefaultAction)
   {
      if (pError)
         *pError = IDS_ERROR;
      return FALSE;
   }

   //
   //   If it is currently enabled, show "disable" string, otherwise "enable"
   //

   if (fEnabled)      
      ::LoadString(hInstance, IDS_LAN_DISABLE, pszDefaultAction, 128);
   else
      ::LoadString(hInstance, IDS_LAN_ENABLE, pszDefaultAction, 128);

   return TRUE;
}

BOOL LanConnInfo::defaultAction(HINSTANCE hInstance, HWND hParent, UINT * pError)
{
    HCURSOR     hCursor  = SetCursor(LoadCursor(NULL, IDC_WAIT));
    TCHAR       multiSz[257];
    DWORD       cbBuffer = sizeof(multiSz);

    HANDLE                    hNdisPwr;
    NDISPWR_SAVEPOWERSTATE    SavePowerState;
 
   
	// because we building a multisz we need to leave room for the double null at the end so copy one less than the buffer.
    StringCchCopy(multiSz, (cbBuffer / sizeof(TCHAR))-2, this->pszAdapter);
    multiSz[::_tcslen(multiSz)+1] = _T('\0'); // Multi sz needs an extra null
   

    //
    //   Inform ndispwr.dll on the power state of this adapter..
    //
    
    hNdisPwr = CreateFile(
    		    (PTCHAR)NDISPWR_DEVICE_NAME,					//	Object name.
    		    0x00,											//	Desired access.
    		    0x00,											//	Share Mode.
    		    NULL,											//	Security Attr
    		    OPEN_EXISTING,									//	Creation Disposition.
    		    FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,	//	Flag and Attributes..
    		    (HANDLE)INVALID_HANDLE_VALUE);	

    if (hNdisPwr != INVALID_HANDLE_VALUE)
    {
        SavePowerState.pwcAdapterName = pszAdapter;

        //
        //  If it is currently enabled, set the power level to D4.
        //

        if (fEnabled)
            SavePowerState.CePowerState   = D4;
        else
        {
            //
            //  Don't set to D0 in here because that implies we want the 
            //  device to be always in D0 (even when suspending).
            //  Setting to PwrDeviceUnspecified reenables the normal
            //  power management to the device.
            //
            
            SavePowerState.CePowerState   = PwrDeviceUnspecified;
        }                

        DeviceIoControl(
            hNdisPwr,
            IOCTL_NPW_SAVE_POWER_STATE,
            &SavePowerState,
            sizeof(NDISPWR_SAVEPOWERSTATE),
            NULL,
            0x00,
            NULL,
            NULL);    

        CloseHandle(hNdisPwr);
       
    }	        


   //
   //   Issue the PwrDeviceUnspecified or D4 to the adapter if it supports power management.
   //
   
   if (fSupportsPowerManagment)
   {
        CEDEVICE_POWER_STATE Dx = PwrDeviceUnspecified;
        TCHAR                szName[MAX_PATH];        
        int                  nChars;        

        nChars = _sntprintf(
                    szName, 
                    MAX_PATH-1, 
                    _T("%s\\%s"), 
                    PMCLASS_NDIS_MINIPORT, 
                    pszAdapter);

        szName[MAX_PATH-1]=0;

        if(nChars != (-1)) 
        {
            if (fEnabled)
            {
                //
                //  Currently enabled, power down to D4
                //
                
                if (ERROR_SUCCESS != ::SetDevicePower(szName, POWER_NAME, D4))
                {
                    if (pError)
                        *pError = IDS_ERROR;
                }
            }
            else
            {
                //
                //  Currently disabled, get it back to default.
                //
                
                if (ERROR_SUCCESS != ::SetDevicePower(szName, POWER_NAME, PwrDeviceUnspecified))
                {
                    if (pError)
                       *pError = IDS_ERROR;
                }
            }
        }
    }

    //
    //  If it is currently enabled, unbind it, otherwise rebind it.
    //

    if (fEnabled)
    {
        DoNdisIOControl(
            IOCTL_NDIS_UNBIND_ADAPTER, 
            multiSz,
            (_tcslen(multiSz)+2) * sizeof(TCHAR),
            NULL, 
            NULL);   
    }
    else
    {
        DoNdisIOControl(
            IOCTL_NDIS_BIND_ADAPTER, 
            multiSz,
            (_tcslen(multiSz)+2) * sizeof(TCHAR),
            NULL, 
            NULL);
    }

    if (DoNdisIOControl(
            IOCTL_NDIS_GET_ADAPTER_BINDINGS, 
            pszName,
            (_tcslen(pszName)+1) * sizeof(TCHAR),
            multiSz, 
            &cbBuffer))
    {
        #ifdef DEBUG
            BOOL fEnabledVerify = (_T('\0') != multiSz[0]);
            ASSERT(fEnabled != fEnabledVerify);
        #endif

        fEnabled = (_T('\0') != multiSz[0]);
    }
 

    SetCursor(hCursor);
    return TRUE;

}


BOOL LanConnInfo::rename(TCHAR * pszNewName, UINT * pError)
{
	// Param check
	if (!pszNewName || !pError)
		return FALSE;
	
	DWORD dwError = ERROR_SUCCESS;
	*pError = 0;
	
	// Null name check
	if (!pszNewName)
	{
		*pError = IDS_ERR_NULLNAME;
		return FALSE;
	}
	
	// did it actualy change?
	if (_tcsicmp(pszNewName, pszName))
	{
		ValidateName(pszNewName, pError);
		if (*pError)
			return FALSE;
		
		if (pszName)
			delete [] pszName;
		
		pszName = new TCHAR[::_tcslen(pszNewName)+1];
		if (pszName)
		{
			::_tcscpy(pszName, pszNewName);
			writeFriendlyName();
		}
		else
		{
			*pError = IDS_ERR_OOM;
			return FALSE;
		}
	}
	
	return TRUE;
}

BOOL LanConnInfo::showProperties(HINSTANCE hInstance, HWND hParent, UINT * pError)
{
   if (::CallAdapterIPProperties(hParent, pszAdapter))
   { 
      TCHAR multiSz[257];
      HCURSOR hCursor = NULL;


	  // because we building a multisz we need to leave room for the double null at the end so copy one less than the buffer.
	  StringCchCopy(multiSz, (sizeof(multiSz) / sizeof(TCHAR))-1, pszAdapter);
      multiSz[::_tcslen(multiSz)+1] = _T('\0'); // Multi sz needs an extra null

      hCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
      DoNdisIOControl(IOCTL_NDIS_REBIND_ADAPTER, multiSz,
                      (_tcslen(multiSz)+2) * sizeof(TCHAR),
                      NULL, NULL);
      SetCursor(hCursor);
   }
   return TRUE;
}

BOOL LanConnInfo::getDisplayInfo(UINT col, HINSTANCE hInstance, TCHAR * pszInfo,
                                 DWORD cchInfo, UINT * pError) const
{    
   if (pszInfo && pszName)
   {
      switch (col)
      {
         case 0: StringCchCopy(pszInfo, cchInfo, pszName); break;
         case 1: ::LoadString(hInstance, IDS_LAN_LAN, pszInfo, cchInfo); break;
         case 2: ::LoadString(hInstance,
                              fEnabled ? IDS_LAN_ENABLED : IDS_LAN_DISABLED,
                              pszInfo, cchInfo); break;
         case 3: StringCchCopy(pszInfo, cchInfo, pszAdapter); break;
         default: *pszInfo = _T('\0');
      }
      return TRUE;
   }
   else
   {
      if (pError)
         *pError = IDS_ERROR;
      return FALSE;
   }
}

BOOL LanConnInfo::DoNdisIOControl(DWORD dwCommand, LPVOID pInBuffer,
                                  DWORD cbInBuffer, LPVOID pOutBuffer,
                                  DWORD * pcbOutBuffer)
{
   HANDLE hNdis;
   BOOL fResult = FALSE;

   hNdis = ::CreateFile(DD_NDIS_DEVICE_NAME, GENERIC_READ | GENERIC_WRITE,
                        FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS,
                        0, NULL);

   if (INVALID_HANDLE_VALUE != hNdis)
   {
      fResult = ::DeviceIoControl(hNdis, dwCommand, pInBuffer, cbInBuffer,
                                  pOutBuffer, (pcbOutBuffer ? *pcbOutBuffer : 0),
                                  pcbOutBuffer, NULL);
      ::CloseHandle(hNdis);
   }

   return fResult;
}

BOOL LanConnInfo::isWireless() const
{
   BOOL fSuccess = FALSE;

   if (!pszAdapter)
      return FALSE;

   if (fEnabled)
   {
      HINSTANCE hWZClib = ::LoadLibraryW(L"wzcsapi.dll");
      if (!hWZClib)
         return FALSE;

      DWORD (*pfnWZCQueryInterface)(LPWSTR,DWORD,PINTF_ENTRY,LPDWORD) =
         (DWORD (*)(LPWSTR,DWORD,PINTF_ENTRY,LPDWORD))
            ::GetProcAddressW(hWZClib, L"WZCQueryInterface");
      if (pfnWZCQueryInterface)
      {
         INTF_ENTRY ie = {0};
         DWORD dwOIDFlags = 0;
         ie.wszGuid = pszAdapter;
         if (ERROR_SUCCESS == pfnWZCQueryInterface(NULL, INTF_ALL,
                                                   &ie, &dwOIDFlags))
         {
            VOID (*pfnWZCDeleteIntfObj)(PINTF_ENTRY) =
               (VOID (*)(PINTF_ENTRY))
                  ::GetProcAddressW(hWZClib, L"WZCDeleteIntfObj");
            if (pfnWZCDeleteIntfObj)
               pfnWZCDeleteIntfObj(&ie);
            fSuccess = TRUE;

            // Write out our wireless property
            HKEY hKey = NULL;
            if (ERROR_SUCCESS == ::RegCreateKeyEx(HKEY_CURRENT_USER,
                                                  szWirelessCacheRegistryPath,
                                                  NULL, NULL, 0, KEY_WRITE,
                                                  NULL, &hKey, NULL))
            {
               DWORD dwSet = 1;
               ::RegSetValueEx(hKey, pszAdapter, NULL, REG_DWORD,
                               reinterpret_cast<LPBYTE>(&dwSet), sizeof(dwSet));
               ::RegCloseKey(hKey);
            }
         }
      }
      FreeLibrary(hWZClib);
   }
   else
   {
      HKEY hKey = NULL;
      if (ERROR_SUCCESS == ::RegOpenKeyEx(HKEY_CURRENT_USER,
                                          szWirelessCacheRegistryPath,
                                          NULL, KEY_READ, &hKey))
      {
         DWORD dwSet = 0;
         DWORD cbSet = sizeof(dwSet);
         if (ERROR_SUCCESS == ::RegQueryValueEx(hKey, pszAdapter, NULL, NULL,
                                                reinterpret_cast<LPBYTE>(&dwSet),
                                                &cbSet))
         {
            fSuccess = static_cast<BOOL>(dwSet);
         }
         ::RegCloseKey(hKey);
      }
   }

   return fSuccess;
}

BOOL LanConnInfo::readFriendlyName()
{
   HKEY hKey = NULL;
   TCHAR szName[dwMaxFriendlyNameLength] = {0};
   DWORD dwName = sizeof(szName)/sizeof(*szName);

   if (!pszAdapter)
      return FALSE;

   if (ERROR_SUCCESS != ::RegOpenKeyEx(HKEY_CURRENT_USER, szNameMappingRegistryPath,
                                       NULL, KEY_READ, &hKey))
   {
      return FALSE;
   }

   if (ERROR_SUCCESS != ::RegQueryValueEx(hKey, pszAdapter, NULL, NULL,
                                          reinterpret_cast<LPBYTE>(szName),
                                          &dwName))
   {
      ::RegCloseKey(hKey);
      return FALSE;
   }
   ::RegCloseKey(hKey);

   if (!isValidConnInfoName(szName))
      return FALSE;

   if (pszName)
      delete [] pszName;

   pszName = new TCHAR[::_tcslen(szName)+1];
   if (!pszName)
      return FALSE;

   ::_tcscpy(pszName, szName);
   return TRUE;
}

BOOL LanConnInfo::writeFriendlyName() const
{
   HKEY hKey = NULL;

   ASSERT(isValidConnInfoName(pszName));
   if (!pszAdapter || !pszName || !isValidConnInfoName(pszName))
      return FALSE;

   if (ERROR_SUCCESS != ::RegCreateKeyEx(HKEY_CURRENT_USER, szNameMappingRegistryPath,
                                         NULL, NULL, 0, KEY_WRITE, NULL,
                                         &hKey, NULL))
   {
      return FALSE;
   }

   if (ERROR_SUCCESS != ::RegSetValueEx(hKey, pszAdapter, NULL, REG_SZ,
                                        reinterpret_cast<LPBYTE>(pszName),
                                        (_tcslen(pszName)+1)*sizeof(*pszName)))
   {
      ::RegCloseKey(hKey);
      return FALSE;
   }

   // Update all top-level windows of a settings change (netui in particular)
   SendMessage(HWND_BROADCAST, WM_SETTINGCHANGE, NULL, (LPARAM)L"netui"); 

   ::RegCloseKey(hKey);

   return TRUE;
}

DWORD LanConnInfo::GetIfAddressFromIfIndex(DWORD dwIndex)
{
   DWORD dwError = NO_ERROR;
   PMIB_IPADDRTABLE pTables = NULL;
   DWORD cbTables = 0;
   DWORD dwIfAddress = static_cast<DWORD>(-1);

   dwError = ::GetIpAddrTable(pTables, &cbTables, FALSE);
   if ((ERROR_INSUFFICIENT_BUFFER == dwError) || (ERROR_BUFFER_OVERFLOW == dwError))
   {
      pTables = reinterpret_cast<PMIB_IPADDRTABLE>(new BYTE[cbTables]);
      ASSERT(pTables);
      if (pTables)
      {
         if (NO_ERROR == ::GetIpAddrTable(pTables, &cbTables, FALSE))
         {
            for (DWORD i = 0; i < pTables->dwNumEntries; i++)
            {
               if (pTables->table[i].dwIndex == dwIndex)
               {
                  dwIfAddress = pTables->table[i].dwAddr;
                  break;
               }
            }
         }
         delete [] pTables;
      }
   }

   return dwIfAddress;
}

BOOL LanConnInfo::getDefaultGatewayRoute(MIB_IPFORWARDROW & ipRoute) const
{
   DWORD dwError = NO_ERROR;
   PMIB_IPFORWARDTABLE pTables = NULL;
   DWORD cbTables = 0;
   BOOL fSuccess = FALSE;

   dwError = ::GetIpForwardTable(pTables, &cbTables, FALSE);
   if ((ERROR_INSUFFICIENT_BUFFER == dwError) || (ERROR_BUFFER_OVERFLOW == dwError))
   {
      pTables = reinterpret_cast<PMIB_IPFORWARDTABLE>(new BYTE[cbTables]);
      ASSERT(pTables);
      if (pTables)
      {
         if (NO_ERROR == ::GetIpForwardTable(pTables, &cbTables, FALSE))
         {
            in_addr addr = {0};
            for (DWORD i = 0; i < pTables->dwNumEntries; i++)
            {
               if ((GetIfAddressFromIfIndex(pTables->table[i].dwForwardIfIndex) == dwAddress) &&
                   (pTables->table[i].dwForwardNextHop == dwGatewayAddress))
               {
                  ipRoute = pTables->table[i];
                  fSuccess = TRUE;
                  break;
               }
            }
         }
         delete [] pTables;
      }
   }

   return fSuccess;
}

DWORD LanConnInfo::getDefaultGatewayRouteMetric() const
{
   MIB_IPFORWARDROW ipRoute = {0};
   DWORD dwMetric = static_cast<DWORD>(-1);

   if (getDefaultGatewayRoute(ipRoute))
   {
      dwMetric = ipRoute.dwForwardMetric1;
   }
   else
   {
      HKEY hKey = NULL;
      TCHAR szMetricRegistryPath[MAX_PATH];
      StringCchPrintf(szMetricRegistryPath, MAX_PATH, _T("Comm\\%s\\Parms\\TCPIP"), pszAdapter);
      if (ERROR_SUCCESS == ::RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                          szMetricRegistryPath,
                                          NULL, KEY_READ, &hKey))
      {
         DWORD cbMetric = sizeof(dwMetric);
         if (ERROR_SUCCESS != ::RegQueryValueEx(hKey, _T("InterfaceMetric"), NULL, NULL,
                                                reinterpret_cast<LPBYTE>(&dwMetric),
                                                &cbMetric))
         {
            dwMetric = static_cast<DWORD>(-1);
         }
         ::RegCloseKey(hKey);
      }
   }

   return dwMetric;
}

BOOL LanConnInfo::setDefaultGatewayRouteMetric(DWORD dwMetric)
{
   MIB_IPFORWARDROW ipRoute = {0};
   DWORD dwSuccess = NO_ERROR;

   if (getDefaultGatewayRoute(ipRoute))
   {
      ipRoute.dwForwardMetric1 = dwMetric;
      dwSuccess = ::SetIpForwardEntry(&ipRoute);
   }

   HKEY hKey = NULL;
   TCHAR szMetricRegistryPath[MAX_PATH];
   StringCchPrintf(szMetricRegistryPath, MAX_PATH, _T("Comm\\%s\\Parms\\TCPIP"), pszAdapter);
   if (ERROR_SUCCESS == ::RegCreateKeyEx(HKEY_LOCAL_MACHINE, szMetricRegistryPath,
                                         NULL, NULL, 0, KEY_WRITE, NULL, &hKey, NULL))
   {
      ::RegSetValueEx(hKey, _T("InterfaceMetric"), NULL, REG_DWORD,
                      reinterpret_cast<LPBYTE>(&dwMetric),  sizeof(dwMetric));
      ::RegCloseKey(hKey);
   }

   return (NO_ERROR == dwSuccess);
}



UINT LanConnInfo::getIconID() const
{ 
    NDISPWR_SAVEPOWERSTATE  SavedPowerState;
    HANDLE                  hNdisPwr;
    DWORD                   dwScrap;
    UINT                    uiReturnValue;

    //
    //   Query ndispwr.dll on the saved power state of this adapter..
    //
    
    hNdisPwr = CreateFile(
        		    (PTCHAR)NDISPWR_DEVICE_NAME,					//	Object name.
        		    0x00,											//	Desired access.
        		    0x00,											//	Share Mode.
        		    NULL,											//	Security Attr
        		    OPEN_EXISTING,									//	Creation Disposition.
        		    FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,	//	Flag and Attributes..
        		    (HANDLE)INVALID_HANDLE_VALUE);	

    if (hNdisPwr != INVALID_HANDLE_VALUE)
    {
        SavedPowerState.pwcAdapterName = pszAdapter;
        
        if (DeviceIoControl(
                hNdisPwr,
                IOCTL_NPW_QUERY_SAVED_POWER_STATE,
                NULL,
                0x00,
                &SavedPowerState,
                sizeof(NDISPWR_SAVEPOWERSTATE),
                &dwScrap,
                NULL) == FALSE)
        {
            //
            //  Assume it is powered on if somehow we fail.
            //
            
            SavedPowerState.CePowerState = D0;           
        }

        CloseHandle(hNdisPwr);
       
    }	   
    else
        SavedPowerState.CePowerState = D0;

    if (fEnabled && (D0 == SavedPowerState.CePowerState))
	{
		// enabled and powered
		uiReturnValue = fWireless ? enabledWLanIconID  : enabledLanIconID;
	}else{
		// either disabled or unpowered, display disabled (red X) icon
		uiReturnValue = fWireless ? disabledWLanIconID : disabledLanIconID;
	}

    return uiReturnValue;

}   //  getIconID()



//  This function returns the physical medium as defined in ntddndis.h
//  NDIS_PHYSICAL_MEDIUM.  
//  It uses NDISUIO's IOCTL_NDISUIO_NIC_STATISTICS to get this information.

DWORD LanConnInfo::getPhysicalMedium() const
{
    // the NDISUIO_QUERY_OID includes data for 1 dword, sufficient for getting
    // an enumerative value from the driver. This spares us of an additional
    // allocation.

	NDISUIO_QUERY_OID	NicGetOid;
    HANDLE          	hNdisUio			= NULL;
    DWORD           	dwBytesWritten		= 0;
    DWORD           	dwPhysicalMedium	= 0;   //  NdisPhysicalMediumUnspecified.

    do {
        hNdisUio = CreateFile(
                        (PTCHAR)NDISUIO_DEVICE_NAME,					
            			0x00,											
            			0x00,											
            			NULL,											
            			OPEN_EXISTING,									
            			FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
            			(HANDLE)INVALID_HANDLE_VALUE);	

        if (hNdisUio == INVALID_HANDLE_VALUE) {

//            DEBUGMSG (ZONE_WARNING,
  //              (TEXT("LanConnInfo::getPhysicalMedium(): error opening NDISUIO_DEVICE_NAME\r\n")));

            break;            
        }

		ZeroMemory(&NicGetOid, sizeof(NDISUIO_QUERY_OID));
        NicGetOid.Oid = OID_GEN_PHYSICAL_MEDIUM;
        NicGetOid.ptcDeviceName = pszAdapter;

    	if (DeviceIoControl(
                hNdisUio,
				IOCTL_NDISUIO_QUERY_OID_VALUE,
                &NicGetOid,
                sizeof(NDISUIO_QUERY_OID),
                &NicGetOid,
                sizeof(NDISUIO_QUERY_OID),
                &dwBytesWritten,
                NULL))
		{
    	    dwPhysicalMedium = *(LPDWORD)NicGetOid.Data;
    	}else{
		    DWORD retCode = ERROR_SUCCESS;
			retCode = GetLastError();

    	    DEBUGMSG (0, (TEXT("LanConnInfo::getPhysicalMedium(): error IOCTL_NDISUIO_NIC_STATISTICS.\r\n")));    		
    	}

    	CloseHandle(hNdisUio);
    }
    while (0);

    return dwPhysicalMedium;
}


DWORD
LanConnInfo::IsRasVEMAdapter(
	IN const WCHAR *wszAdapterName,
	OUT BOOL       *bIsVEMAdapter)
//
//  Set *bIsVEMAdapter to TRUE if the adapter name is that of a VEM interface created
//  to represent a RAS connection.
//
//  Return ERROR_SUCCESS if the function completes successfully, otherwise an error code.
{
	BOOL bFoundMatchingRasDeviceName	= FALSE;
	LPRASDEVINFO pRasDevInfo			= NULL;
	DWORD        cbRasDevInfo			= 0;
	DWORD        cRasDevices			= 0;
	DWORD        dwResult				= ERROR_SUCCESS;

	//
	// Get a list of RAS device names
	//
	cbRasDevInfo = sizeof(RASDEVINFO);
	do
	{
		LocalFree(pRasDevInfo);
		pRasDevInfo = (RASDEVINFO*) LocalAlloc(LPTR, cbRasDevInfo);
		if (NULL == pRasDevInfo)
		{
			dwResult = ERROR_NOT_ENOUGH_MEMORY;
			break;
		}

		pRasDevInfo->dwSize = sizeof(RASDEVINFO);
		dwResult = RasEnumDevices(pRasDevInfo, &cbRasDevInfo, &cRasDevices);
	} while (ERROR_BUFFER_TOO_SMALL == dwResult);

	if (ERROR_SUCCESS == dwResult)
	{
		DWORD i;

		//
		// Search the list for a matching name.
		//
		for (i = 0; i < cRasDevices; i++)
		{
			if (0 == wcsicmp(pRasDevInfo[i].szDeviceName, wszAdapterName))
			{
				bFoundMatchingRasDeviceName = TRUE;
				break;
			}
		}
	}
	LocalFree(pRasDevInfo);

	// If the name of the NDIS adapter matches that of a RAS device, then it
	// is a VEM interface representing a RAS connection.
	*bIsVEMAdapter = bFoundMatchingRasDeviceName;

	return dwResult;
}
