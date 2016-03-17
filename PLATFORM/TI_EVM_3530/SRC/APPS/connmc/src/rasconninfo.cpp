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
#include "windowsx.h"
#include "connmc.h"
#include "rasconninfo.h"
#include "rnaconninfo.h"
#include "dccconninfo.h"
#include "vpnconninfo.h"
#include "pppoeconninfo.h"
#include ".\ipaddr.h"
#include "wzcsapi.h"
#include "ras.h"
#include "eaputil.h"


#define MAX_EAP_DATA_SIZE 0x20000

LPRASENTRYNAME RasConnInfo::pREN = NULL;
DWORD RasConnInfo::dwRENCount = 0;
DWORD RasConnInfo::dwRENCurrent = 0;
const TCHAR RasConnInfo::szBadRasCharList[] = { _T('\\'), _T('/'), _T(':'),
                                                _T('?'), _T('\"'), _T('<'),
                                                _T('>'), _T('|') };

#define IDM_USER_CHECK_ENTRYNAME (WM_USER + 202)
TCHAR *gszEntryToFind = NULL;

RASOPTIONMAP TCPDlgOptionMap0[] = {
    RASEO_SpecificIpAddr,       IDC_USESERVIP,      OPTION_DIRECTION_REVERSE,
    RASEO_SwCompression,        IDC_SOFTCOMPCHK,    OPTION_DIRECTION_NORMAL,
    RASEO_IpHeaderCompression,  IDC_IPCOMP,         OPTION_DIRECTION_NORMAL,
    0,0,0
};

RASOPTIONMAP SecurityDlgOptionMap[] = {
    RASEO_RequireDataEncryption,IDC_CA_CB_ENCRYPTION,   OPTION_DIRECTION_NORMAL,
    RASEO_ProhibitEAP,          IDC_CA_CB_EAP,          OPTION_DIRECTION_REVERSE,
    RASEO_ProhibitPAP,          IDC_CA_CB_PAP,          OPTION_DIRECTION_REVERSE,
    RASEO_ProhibitCHAP,         IDC_CA_CB_CHAP,         OPTION_DIRECTION_REVERSE,
    RASEO_ProhibitMsCHAP,       IDC_CA_CB_MSCHAP,       OPTION_DIRECTION_REVERSE,
    RASEO_ProhibitMsCHAP2,      IDC_CA_CB_MSCHAP2,      OPTION_DIRECTION_REVERSE,
    RASEO_UseLogonCredentials,  IDC_CA_CB_USEWINDOWSPW, OPTION_DIRECTION_NORMAL,
    RASEO_PreviewUserPw,        IDC_CA_CB_PREVIEWUSERPW,OPTION_DIRECTION_NORMAL,
    0,0,0
};


////////////////////////
// RasConnInfo
////////////////////////

RasConnInfo::RasConnInfo(TCHAR * pszName) :
   pDevConfig(NULL),
   cbDevConfig(0)
{
   if (!pszName)
      return;

    m_pEapExtInfo = NULL;
    m_cEapExtInfo = 0;
   	m_pEapConnData = NULL;
	m_cbEapConnData = 0;

   TCHAR szDefaultRasString[] = _T("");
   this->pszName = new TCHAR[::_tcslen(pszName)+1];
   if (this->pszName)
   {
      DWORD dwResult = 0;

      // Save off the new name
      ::_tcscpy(this->pszName, pszName);

      // Figure out if this is an existing entry or a new one
      DWORD cb = sizeof(RASENTRY);
      RasEntry.dwSize = sizeof(RASENTRY);
      dwResult = ::RasGetEntryProperties(NULL, pszName, &RasEntry, &cb,
                                         pDevConfig, &cbDevConfig);

      // If this is a new entry use the defaults and make note that this
      // ras entry has not been written out yet
      if (ERROR_CANNOT_FIND_PHONEBOOK_ENTRY == dwResult)
      {
         pszName = szDefaultRasString;
         dwResult = ::RasGetEntryProperties(NULL, pszName, &RasEntry, &cb,
                                            pDevConfig, &cbDevConfig);
      }

      // Does CE use extra bytes on the RasEntry?
      // if (ERROR_BUFFER_TOO_SMALL == dwResult)
      // {
      // }

      // Now populate this object with either existing data or default data
      if (0 < cbDevConfig)
      {
         ASSERT(sizeof(RASENTRY) == cb);
         pDevConfig = new BYTE[cbDevConfig];
         if (pDevConfig)
         {
            dwResult = ::RasGetEntryProperties(NULL, pszName, &RasEntry,
                                               &cb, pDevConfig, &cbDevConfig);
         }
         else
         {
            cbDevConfig = 0;
         }
      }

	  GetEapConnData();

      // By the time we get here we should have returned success (0) from RasGetEntryProperties
      ASSERT(0 == dwResult);
   }
}

RasConnInfo::~RasConnInfo()
{
	if (pDevConfig)
		delete [] pDevConfig;

	LocalFree(m_pEapExtInfo);
	LocalFree(m_pEapConnData);
}


RasConnInfo * RasConnInfo::FindFirst()
{
   RASENTRYNAME ren = {0};
   ren.dwSize = sizeof(ren);
   DWORD cb = sizeof(ren);

   // Clean up from previous calls to FindFirst
   if (pREN)
   {
      delete [] pREN;
      pREN = NULL;
      dwRENCurrent = 0;
      dwRENCount = 0;
   }

   // Bail if there are no entries to enumerate
   RasEnumEntries(NULL, NULL, &ren, &cb, NULL);
   if (0 == cb)
      return NULL;

   // Allocate the array of names
   pREN = new RASENTRYNAME[cb/sizeof(RASENTRYNAME)];
   if (!pREN)
      return NULL;

   // Actually grab the entries
   pREN->dwSize = sizeof(RASENTRYNAME);
   if (RasEnumEntries(NULL, NULL, pREN, &cb, &dwRENCount))
   {
      delete [] pREN;
      pREN = NULL;
      dwRENCount = 0;
      return NULL;
   }

   return FindNext();
}

RasConnInfo * RasConnInfo::FindNext()
{
   RasConnInfo * pRasConnInfo = NULL;

   if (!pREN || !dwRENCount)
      return NULL;

   while (dwRENCurrent < dwRENCount)
   {
      // Skip entries that start with a '`'
      if (_T('`') != pREN[dwRENCurrent].szEntryName[0])
      {
         pRasConnInfo = RasFactory(pREN[dwRENCurrent++].szEntryName);
         ASSERT(pRasConnInfo);
         break;
      }

      dwRENCurrent++;
   }

   // That was the last item in the list
   if (dwRENCurrent == dwRENCount)
   {
      delete [] pREN;
      pREN = NULL;
      dwRENCurrent = 0;
      dwRENCount = 0;
   }

   return pRasConnInfo;
}

BOOL RasConnInfo::isValid(UINT * pError) const
{
   if (!pszName || !(*pszName))
   {
      if (pError) *pError = IDS_ERR_NULLNAME;
      return FALSE;
   }

   DWORD dwError = RasValidateEntryName(NULL, pszName);
   switch (dwError)
   {
      case ERROR_ALREADY_EXISTS:
         if (pError) *pError = IDS_ERR_ALREADY_EXISTS;
         return TRUE;

      case ERROR_SUCCESS:
         if (pError) *pError = IDS_ERR_DOES_NOT_EXIST;
         return FALSE;

      case ERROR_INVALID_NAME:
         if (pError) *pError = IDS_ERR_BADNAME;
         return FALSE;

      default:
         if (pError) *pError = IDS_ERROR;
         return FALSE;
   }
}

HRASCONN RasConnInfo::getRasConnHandle() const
{
   HRASCONN hRasConn = NULL;
   RASCONN rasConn = {0};
   LPRASCONN lpRasConn = &rasConn;
   DWORD cb = sizeof(RASCONN);
   DWORD dwConnections = 1;
   DWORD dwRetVal = 0;

   lpRasConn->dwSize = sizeof(RASCONN);
   dwRetVal = ::RasEnumConnections(lpRasConn, &cb, &dwConnections);

   if (0 != dwRetVal)
   {
      if (ERROR_BUFFER_TOO_SMALL == dwRetVal)
      {
         // This means there is more than one active connection
         lpRasConn = reinterpret_cast<LPRASCONN>(new BYTE[cb]);
         if (!lpRasConn)
            return NULL;

         lpRasConn->dwSize = sizeof(RASCONN);
         if (0 != ::RasEnumConnections(lpRasConn, &cb, &dwConnections))
         {
            // An unknown error occured, bail now
            delete [] lpRasConn;
            return NULL;
         }
      }
      else
      {
         // An unknown error occured, bail now
         return NULL;
      }
   }

   for (DWORD i = 0; i < dwConnections; i++)
   {
      if (!::_tcscmp(pszName, lpRasConn[i].szEntryName))
      {
         hRasConn = lpRasConn[i].hrasconn;
         break;
      }
   }

   // If we had to dynamicly allocate memory, free it here
   if (lpRasConn != &rasConn)
      delete [] lpRasConn;

   return hRasConn;
}

void RasConnInfo::writeChanges()
{
   ::RasSetEntryProperties(NULL, pszName, &RasEntry, sizeof(RasEntry),
                           pDevConfig, cbDevConfig);

   // Safe to write the Eap Connection Data now that the RAS entry has been created.   
   SetEapConnData();
}

void RasConnInfo::handleDevConfig(HWND hDlg)
{
   static HANDLE hThread = NULL;
   DWORD dwResult = 0;
   MSG msg = {0};

   ASSERT(!hThread);
   if (hThread)
   {
      // TODO - zonify - RETAILMSG (1, (TEXT("Recursing, only one config allowed at a time\r\n")));
      return;
   }

   hThread = ::CreateThread(NULL, 0, DevConfigThread, (LPVOID)this, 0, NULL);
   ASSERT(hThread);
   if (!hThread)
   {
      // TODO - zonify - RETAILMSG (1, (TEXT("Create Thread failed!!!\r\n")));
      return;
   }

   while (TRUE)
   {
      // Wait for completion or a paint notification to be signalled
      dwResult = ::MsgWaitForMultipleObjectsEx(1, &hThread, INFINITE,
                                               QS_PAINT, MWMO_INPUTAVAILABLE);

      if (WAIT_OBJECT_0+1 == dwResult) // Thread still running but need to paint background
      {
         if (::PeekMessage(&msg, NULL, WM_PAINT, WM_PAINT, PM_REMOVE))
            if (!::IsDialogMessage(hDlg, &msg))
               ::DispatchMessage(&msg);
      }
      else // Thread finished or something bad happened, bail either way
      {
         break;
      }
   }

   ::CloseHandle(hThread);
   hThread = NULL;
}

DWORD WINAPI RasConnInfo::DevConfigThread(LPVOID pVoid)
{
   RasConnInfo * pThis = reinterpret_cast<RasConnInfo *>(pVoid);
   ASSERT(pThis);
   if (!pThis)
      return 0;

   LPVARSTRING pVarString = NULL;
   DWORD cbVarString = sizeof(VARSTRING);
   DWORD dwResult = 0;

   while (TRUE)
   {
      pVarString = reinterpret_cast<LPVARSTRING>(new BYTE[cbVarString]);
      if (!pVarString)
         return 0;

      pVarString->dwTotalSize = cbVarString;

      dwResult = ::RasDevConfigDialogEdit(pThis->RasEntry.szDeviceName,
                                          pThis->RasEntry.szDeviceType,
                                          NULL, pThis->pDevConfig,
                                          pThis->cbDevConfig, pVarString);

      // Interpret the resutlts
      if (STATUS_SUCCESS != dwResult)
      {
         // Something bad happened, bail now
         // TODO - zonify - DEBUGMSG (ZONE_ERROR, (TEXT("ConnMC: Error 0x%X(%d) from RasDevConfigDialogEdit\r\n"), RetVal, RetVal));
         break;
      }
      else if (pVarString->dwNeededSize > pVarString->dwTotalSize)
      {
         cbVarString = pVarString->dwNeededSize;

         // Need more memory
         if(pVarString)
               delete [] pVarString;
         pVarString = NULL;
      }
      else
      {
         // Replace our DevConfig with the new one
         if (pThis->pDevConfig)
         {
            delete [] pThis->pDevConfig;
            pThis->pDevConfig = NULL;
            pThis->cbDevConfig = 0;
         }
         ASSERT(0 == pThis->cbDevConfig);

         pThis->pDevConfig = new BYTE[pVarString->dwStringSize];
         if (pThis->pDevConfig)
         {
            memcpy(pThis->pDevConfig,
                   (LPBYTE) pVarString + pVarString->dwStringOffset,
                   pVarString->dwStringSize);
            pThis->cbDevConfig = pVarString->dwStringSize;
         }

         break;
      }
   }

   if (pVarString)
      delete [] pVarString;

   return 0;
}

void RasConnInfo::getTypeAsDisplayString(HINSTANCE hInstance, TCHAR * pszType,
                                         DWORD cchType) const
{
   ASSERT(_tcslen(RasEntry.szDeviceName) < cchType);
   ::_tcsncpy(pszType, RasEntry.szDeviceName, cchType-1);
}

BOOL RasConnInfo::getDefaultActionString(HINSTANCE hInstance, TCHAR * pszDefaultAction, UINT * pError)
{
   if (!pszDefaultAction)
   {
      if (pError)
         *pError = IDS_ERROR;
      return FALSE;
   }

   if (getRasConnHandle())
      ::LoadString(hInstance, IDS_RAS_STATUS, pszDefaultAction, 128);
   else
      ::LoadString(hInstance, IDS_RAS_CONNECT, pszDefaultAction, 128);

   return TRUE;
}


// A Call back function used while enumerating all the top windows to find 
// the handle to the hidden connection status window owned by RNAApp
BOOL CALLBACK RasConnInfo::FindRNAAppWindow(HWND hWnd, LPARAM lParam)
{
    TCHAR   szClassName[32] = {NULL};
	TCHAR	pszRasEntryName[RAS_MaxEntryName+1] = {NULL};


    GetClassName (hWnd, szClassName,
        sizeof(szClassName)/sizeof(TCHAR));

    if (!_tcscmp (szClassName, TEXT("Dialog")) &&
        (RNAAPP_MAGIC_NUM == GetWindowLong (hWnd, DWL_USER)))
	{
		// Have a potential.  Ask it for its entry name
		LRESULT lRes = 0;
		int i = -1;

		COPYDATASTRUCT
			cds;
		cds.dwData = 0;
		cds.cbData = ( RAS_MaxEntryName +1 ) * sizeof(TCHAR);
		cds.lpData = gszEntryToFind;

		lRes = SendMessage(hWnd, WM_COPYDATA, NULL, (LPARAM)&cds);

		// RNAAPP will be doing a strcmp on the just passed string and its internal name.  It will then return the result to us.
		lRes = SendMessage(hWnd, IDM_USER_CHECK_ENTRYNAME, NULL, NULL);

		if (!lRes)
		{
			*((HWND *)lParam) = hWnd;
			return FALSE;
        }
	}
    return TRUE;
}

// When a connectoid is right clicked, the 1st entry in the list view is called as 
// the default action and will the action performed when the connectoid is double 
// clicked. The default action to be performed depends on the status of the connectoid.
// This is the method called to check the status & implement the default action.
BOOL RasConnInfo::defaultAction(HINSTANCE hInstance, HWND hParent, UINT * pError)
{
    HRASCONN hRasConn = getRasConnHandle();

    if (hRasConn)
    {
		// ... when connection is made, a status window pops up showing the status of the 
		// connection and the user gets to hide it or disconnect the connection. This window
		// is owned by a process called RANApp. So we try to ennumerate all the top windows & 
		// look out for the hidden status window & then show it up.
		
        HWND hOldRNAWnd = NULL;
        BOOL fOK = TRUE;
		
		// Enumerate all the top windows to find the hidden connection status window
		// which is owned by RNAApp
		gszEntryToFind = pszName;
        fOK = EnumWindows(FindRNAAppWindow, (LPARAM)&hOldRNAWnd);

		// Found it - so, show it.
        if( fOK && hOldRNAWnd ) 
        {	
            SetForegroundWindow(hOldRNAWnd);
            ShowWindow( hOldRNAWnd, SW_SHOWNORMAL );
        }
        else
        {
	    //Disconnect the connection in case we cant find the hidden window
            ::RasHangUp(hRasConn);
        }
    }
    else
    {
		TCHAR szTemp[RAS_MaxEntryName+4+1];

        StringCchPrintf(szTemp, RAS_MaxEntryName+4+1, _T("-e\"%s\""), pszName);

        if (!::CreateProcess(_T("rnaapp"), szTemp, NULL, NULL,
                   FALSE, 0, NULL, NULL, NULL, NULL))
        {
            if (pError)
            *pError = IDS_ERR_OOM;

            return FALSE;
        }
    }

    return TRUE;
}

BOOL RasConnInfo::remove(UINT * pError)
{
   if (ERROR_SUCCESS != ::RasDeleteEntry(NULL, pszName))
   {
      if (pError)
         *pError = IDS_ERROR;
      return FALSE;
   }

   if (isSetAsDefault())
      setAsDefault(FALSE, NULL);

   return TRUE;
}


BOOL RasConnInfo::rename(TCHAR * pszNewName, UINT * pError)
{
	DWORD dwError = ERROR_SUCCESS;
	if (pError)
		*pError = 0;
	
	if (!pszNewName)
	{
		if (pError)
			*pError = IDS_ERR_NULLNAME;
		return FALSE;
	}
	
	if (_tcsicmp(pszNewName, pszName))
	{
        if (pError)
        {
		    ValidateName(pszNewName, pError);
		    if (*pError)
			    return FALSE;
        }
		
		// Check for valid address book entry
		dwError = ::RasValidateEntryName(NULL, pszNewName);
		if (ERROR_SUCCESS != dwError)
		{
			if (pError)
			{
				if (ERROR_ALREADY_EXISTS == dwError)
					*pError = IDS_ERR_ALREADY_EXISTS;
				else
					*pError = IDS_ERR_BADNAME;
			}
			return FALSE;
		}
	}
	
	dwError = ::RasRenameEntry(NULL, pszName, pszNewName);
	if (ERROR_SUCCESS != dwError)
	{
		if (pError)
			*pError = IDS_ERROR;
		return FALSE;
	}
	
	BOOL fIsSetDefault = isSetAsDefault();
	TCHAR * pszOldName = pszName;
	pszName = new TCHAR[::_tcslen(pszNewName)+1];
	if (!pszName)
	{
		pszName = pszOldName;
		if (pError)
			*pError = IDS_ERR_OOM;
		return FALSE;
	}
	
	::_tcscpy(pszName, pszNewName);
	
	if (fIsSetDefault)
		setAsDefault(TRUE, NULL);
	
	if (pszOldName)
		delete [] pszOldName;
	
	return TRUE;
}

BOOL RasConnInfo::copy(HINSTANCE hInstance, UINT * pError)
{
   if (!pszName)
   {
      if (pError)
         *pError = IDS_ERR_DOES_NOT_EXIST;
      return FALSE;
   }

   // Find an open name
   TCHAR szCopyName[RAS_MaxEntryName+1] = {0};
   TCHAR szCopyNameFmt[RAS_MaxEntryName+1] = {0};
   LoadString(hInstance, IDS_RAS_COPY_OF, szCopyNameFmt,
              sizeof(szCopyNameFmt)/sizeof(*szCopyNameFmt));

   _sntprintf(szCopyName, RAS_MaxEntryName, _T("%s %s"), szCopyNameFmt, pszName);
   if (ERROR_SUCCESS != RasValidateEntryName(NULL, szCopyName))
   {
      LoadString(hInstance, IDS_RAS_COPY_NUM_OF, szCopyNameFmt,
                 sizeof(szCopyNameFmt)/sizeof(*szCopyNameFmt));
      DWORD dwCounter = 2;
      do
      {
         if (99 < dwCounter)
         {
            if (pError)
               *pError = IDS_ERROR;
            return FALSE;
         }

         StringCchPrintf(szCopyName, RAS_MaxEntryName, szCopyNameFmt, dwCounter, pszName);
         dwCounter++;
      } while (ERROR_SUCCESS != RasValidateEntryName(NULL, szCopyName));
   }

   // Now do the copy
   RASENTRY rasEntryCopy = {0};
   DWORD cb = rasEntryCopy.dwSize = sizeof(RASENTRY);
   DWORD cbCopyDevConfig = 0;
   PBYTE pCopyDevConfig = NULL;
   DWORD dwResult = ::RasGetEntryProperties(NULL, pszName, &rasEntryCopy,
                                            &cb, pCopyDevConfig, &cbCopyDevConfig);

   if (cbCopyDevConfig && (ERROR_BUFFER_TOO_SMALL == dwResult))
   {
      ASSERT(sizeof(RASENTRY) == cb);
      pCopyDevConfig = new BYTE[cbCopyDevConfig];
      if (pCopyDevConfig)
      {
         dwResult = ::RasGetEntryProperties(NULL, pszName, &rasEntryCopy,
                                            &cb, pCopyDevConfig, &cbCopyDevConfig);
         ASSERT(0 == dwResult);
      }
   }

   ::RasSetEntryProperties(NULL, szCopyName, &rasEntryCopy,
                           cb, pCopyDevConfig, cbCopyDevConfig);

   ::RasSetEapConnectionData(NULL, szCopyName, m_pEapConnData, m_cbEapConnData);

   if (pCopyDevConfig)
      delete [] pCopyDevConfig;

   return TRUE;
}

BOOL RasConnInfo::setAsDefault(BOOL fSet, UINT * pError)
{
   BOOL fIsSetAsDefault = isSetAsDefault();

   if (!pszName)
   {
      if (pError)
         *pError = IDS_ERR_DOES_NOT_EXIST;
      return FALSE;
   }

   if (!fIsSetAsDefault && fSet)
   {
      HKEY hKey = NULL;
      if (ERROR_SUCCESS == RegCreateKeyEx(HKEY_LOCAL_MACHINE, REG_AUTORAS_ROOT, 0,
                                          NULL, 0, KEY_WRITE, NULL, &hKey, NULL))
      {
         if (ERROR_SUCCESS == RegSetValueEx(hKey, REG_DEFAULT_RAS_ENTRY, 0, REG_SZ,
                                            reinterpret_cast<const BYTE *>(pszName),
                                            (_tcslen(pszName)+1)*sizeof(TCHAR)))
         {
            fIsSetAsDefault = TRUE;
         }
         RegCloseKey(hKey);
      }
   }
   else if (fIsSetAsDefault && !fSet)
   {
      HKEY hKey = NULL;
      if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, REG_AUTORAS_ROOT,
                                        0, KEY_WRITE, &hKey))
      {
         if (ERROR_SUCCESS == RegDeleteValue(hKey, REG_DEFAULT_RAS_ENTRY))
         {
            fIsSetAsDefault = FALSE;
         }
         RegCloseKey(hKey);
      }
   }

   if ((fIsSetAsDefault && fSet) || (!fIsSetAsDefault && !fSet))
   {
      return TRUE;
   }
   else
   {
      if (pError)
         *pError = IDS_ERROR;
      return FALSE;
   }
}

BOOL RasConnInfo::isSetAsDefault()
{
   BOOL fIsSetAsDefault = FALSE;

   if (!pszName)
      return FALSE;

   HKEY hKey = NULL;
   if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, REG_AUTORAS_ROOT,
                                     0, KEY_READ, &hKey))
   {
      TCHAR szNameBuffer[255];
      DWORD dwNameBuffer = (sizeof(szNameBuffer)/sizeof(*szNameBuffer));

      if (ERROR_SUCCESS == RegQueryValueEx(hKey, REG_DEFAULT_RAS_ENTRY, 0, 0,
                                           reinterpret_cast<LPBYTE>(szNameBuffer),
                                           &dwNameBuffer))
      {
         fIsSetAsDefault = !_tcscmp(szNameBuffer, pszName);
      }
      RegCloseKey(hKey);
   }

   return fIsSetAsDefault;
}

BOOL RasConnInfo::shortcut(HINSTANCE hInstance, UINT * pError)
{
   if (!pszName)
   {
      if (pError)
         *pError = IDS_ERR_SHORTCUT;
      return FALSE;
   }

   HANDLE hFile = INVALID_HANDLE_VALUE;
   TCHAR szShortcut[MAX_PATH] = {0};
   TCHAR szShortcutFmt[MAX_PATH] = {0};

   LoadString(hInstance, IDS_DESKTOP_SHORTCUT, szShortcutFmt, CCHSIZEOF(szShortcutFmt));

   // Create the file on the desktop
   StringCchPrintf(szShortcut, MAX_PATH, szShortcutFmt, pszName);
   StringCchCat(szShortcut, MAX_PATH, _T(".lnk"));

   hFile = CreateFile(szShortcut, GENERIC_WRITE, 0, NULL,
                      CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
   if (INVALID_HANDLE_VALUE == hFile)
   {
      DWORD dwCounter = 2;
      TCHAR * pszShortcutEnd = _tcsstr(szShortcut, _T(".lnk"));
      ASSERT(pszShortcutEnd);

      do
      {
         if (99 < dwCounter)
         {
            if (pError)
               *pError = IDS_ERR_SHORTCUT;
            return FALSE;
         }

         _stprintf(pszShortcutEnd, _T(" (%u).lnk"), dwCounter);
         hFile = CreateFile(szShortcut, GENERIC_WRITE, 0, NULL,
                            CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);

         dwCounter++;
      } while (INVALID_HANDLE_VALUE == hFile);
   }

   // Create the command line to launch from the shortcut
   LoadString(hInstance, IDS_SHORTCUT_STRING, szShortcutFmt, CCHSIZEOF(szShortcutFmt));
   StringCchPrintf(szShortcut, MAX_PATH, szShortcutFmt, pszName);

   // Put the command line into shortcut format
   DWORD dwLen = _tcslen(szShortcut);
   StringCchPrintf(szShortcutFmt, MAX_PATH, _T("%u#%s"), dwLen, szShortcut);

   // Convert to ascii
   CHAR szAnsiTarget[MAX_PATH] = {0};
   dwLen = WideCharToMultiByte(CP_ACP, 0, szShortcutFmt, -1,
                               szAnsiTarget, MAX_PATH, NULL, NULL);

   // Put it all together
   WriteFile(hFile, szAnsiTarget, dwLen, &dwLen, NULL);
   CloseHandle(hFile);

   return TRUE;
}

BOOL RasConnInfo::getDisplayInfo(UINT col, HINSTANCE hInstance, TCHAR * pszInfo,
                                 DWORD cchInfo, UINT * pError) const
{
	if (pszInfo)
	{
		switch (col)
		{
		case 0:
			StringCchCopy(pszInfo, cchInfo, pszName);
			break;
		case 1:
			getTypeAsDisplayString(hInstance, pszInfo, cchInfo);
			break;
		case 2:
		{
			HRASCONN hRasConn = getRasConnHandle();
			RASCONNSTATUS rasconnstatus;

			if (hRasConn && (0 == RasGetConnectStatus(hRasConn, &rasconnstatus)))
			{
				switch (rasconnstatus.rasconnstate)
				{
				case RASCS_Connected:
					::LoadString(hInstance, IDS_RAS_CONNECTED, pszInfo, cchInfo);
					break;
				case RASCS_Disconnected:
					::LoadString(hInstance, IDS_RAS_DISCONNECTED, pszInfo, cchInfo);
					break;
				default:
					::LoadString(hInstance, IDS_RAS_DIALING, pszInfo, cchInfo);
					break;
				}
			}
			else
			{
				::LoadString(hInstance, IDS_RAS_DISCONNECTED, pszInfo, cchInfo);
			}
		}
		break;
		case 3:
			StringCchCopy(pszInfo, cchInfo, RasEntry.szDeviceName);
			break;
		default:
			*pszInfo = _T('\0');
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


RasConnInfo * RasConnInfo::RasFactory(TCHAR * pszName)
{
   if (pszName)
   {
      RASENTRY re = {0};
      DWORD cb = re.dwSize = sizeof(RASENTRY);
      if (!::RasGetEntryProperties(NULL, pszName, &re, &cb, NULL, NULL))
      {
         if (!_tcscmp(RASDT_Modem, re.szDeviceType))
            return new RnaConnInfo(pszName);
         else if (!_tcscmp(RASDT_Direct, re.szDeviceType))
            return new DccConnInfo(pszName);
         else if (!_tcscmp(RASDT_Vpn, re.szDeviceType))
         {
            BOOL fL2TP = !_tcsncmp(re.szDeviceName, L2TPDeviceNamePrefix, _tcslen(L2TPDeviceNamePrefix));
            return new VpnConnInfo(pszName, fL2TP);
         }
         else if (!_tcscmp(RASDT_PPPoE, re.szDeviceType))
            return new PPPOEConnInfo(pszName);

      }
   }

   return NULL;
}


BOOL RasConnInfo::GetUnusedRasName(HINSTANCE hInstance, TCHAR * pszName)
{
   ASSERT(pszName);
   if (!pszName)
      return FALSE;

   ::LoadString(hInstance, IDS_RAS_NEW_NAME, pszName, RAS_MaxEntryName+1);
   if (ERROR_SUCCESS != RasValidateEntryName(NULL, pszName))
   {
      TCHAR szNewName[RAS_MaxEntryName+1] = {0};
      DWORD dwCounter = 2;
      do
      {
         ::_sntprintf(szNewName, RAS_MaxEntryName,
                      _T("%s %u"), pszName, dwCounter);
         dwCounter++;
      } while (ERROR_SUCCESS != ::RasValidateEntryName(NULL, szNewName));

      ::_tcsncpy(pszName, szNewName, RAS_MaxEntryName);
   }
   return TRUE;
}

DWORD RasConnInfo::GetDevices(LPRASDEVINFO * pRasDevInfo)
{
   if (!pRasDevInfo)
      return 0;

   *pRasDevInfo = NULL;
   DWORD cb = 0;
   DWORD dwDeviceCount = 0;
   DWORD dwRetVal = ::RasEnumDevices(NULL, &cb, &dwDeviceCount);
   if ((0 == dwRetVal) && (0 != cb))
   {
      // Allocate a buffer (CALLER MUST FREE THIS BUFFER!!!)
      *pRasDevInfo = reinterpret_cast<LPRASDEVINFO>(new BYTE[cb]);
      ASSERT(*pRasDevInfo);

      if (*pRasDevInfo)
      {
         (*pRasDevInfo)->dwSize = sizeof(RASDEVINFO);
         dwRetVal = ::RasEnumDevices(*pRasDevInfo, &cb, &dwDeviceCount);
      }
      else
      {
         dwDeviceCount = 0;
      }
   }

#ifdef DEBUG
   DEBUGMSG (1, (TEXT("GetRasDevices: Found %d devices\r\n"), dwDeviceCount));
   for (DWORD i = 0; i < dwDeviceCount; i++)
   {
      DEBUGMSG (1, (TEXT("\tDev[%d] : Name='%s' Type='%s'\r\n"),
                i, (*pRasDevInfo)[i].szDeviceName,
                (*pRasDevInfo)[i].szDeviceType));
   }
#endif

   return dwDeviceCount;
}

BOOL RasConnInfo::HasDevices(TCHAR * pszDeviceType, TCHAR *pszDeviceNamePrefix)
{
   ASSERT(pszDeviceType);
   if (!pszDeviceType)
      return FALSE;

   BOOL fHasDevices = FALSE;
   LPRASDEVINFO pRasDevInfo = NULL;
   DWORD dwRasDevices = GetDevices(&pRasDevInfo);

   if ((!dwRasDevices) || (!pRasDevInfo)) {
       // No devices
       return FALSE;
   }

   for (DWORD i=0; i < dwRasDevices; i++)
   {
      if (!_tcscmp(pRasDevInfo[i].szDeviceType, pszDeviceType))
      {
          if (pszDeviceNamePrefix == NULL
            || !_tcsncmp(pRasDevInfo[i].szDeviceName, pszDeviceNamePrefix, _tcslen(pszDeviceNamePrefix)) )
          {
             fHasDevices = TRUE;
             break;
          }
      }
   }
   if (pRasDevInfo)
      delete [] pRasDevInfo;

   return fHasDevices;
}

void CALLBACK RasConnInfo::NullLineCallbackFunc(DWORD dwDevice, DWORD dwMsg,
                                                DWORD dwCallbackInstance,
                                                DWORD dwParam1, DWORD dwParam2,
                                                DWORD dwParam3)

{
   // NULL function.  I can't do a lineInitialize without this function.
   // Since I never dial I don't actually care about any of the state changes
   return;
}

LPLINETRANSLATECAPS RasConnInfo::GetTranslateCaps(HINSTANCE hInstance)
{
   long lReturn = 0;
   HLINEAPP hLineApp = NULL;
   DWORD dwNumDevs = 0;
   DWORD dwNeededSize = sizeof(LINETRANSLATECAPS);
   LPLINETRANSLATECAPS pLineTranCaps = NULL;
   DWORD dwVersion = TAPI_CURRENT_VERSION;

   // TODO - zonify - DEBUGMSG(ZONE_FUNCTION, (TEXT("+GetTranslateCaps()\r\n")));

   lReturn = ::lineInitialize(&hLineApp, hInstance,
                              RasConnInfo::NullLineCallbackFunc,
                              NULL, &dwNumDevs);

   if (lReturn)
   {
      // TODO - zonify - DEBUGMSG(ZONE_FUNCTION | ZONE_ERROR, (TEXT("-GetTranslateCaps(): Error %d from lineInitialize\r\n"), lReturn));
      return NULL;
   }

   while (TRUE)
   {
      pLineTranCaps = (LPLINETRANSLATECAPS) LocalAlloc(LPTR, dwNeededSize);
      if (NULL == pLineTranCaps)
      {
         // TODO - zonify - DEBUGMSG(ZONE_ERROR|ZONE_ALLOC, (TEXT("!GetTranslateCaps: Alloc failed\r\n")));
         break;
      }
      pLineTranCaps->dwTotalSize = dwNeededSize;
      lReturn = ::lineGetTranslateCaps(hLineApp, dwVersion, pLineTranCaps);

      if (lReturn)
      {
         // TODO - zonify - DEBUGMSG (ZONE_ERROR, (TEXT(" GetTranslateCaps:lineGetTranslateCaps returned 0x%X\r\n"), lReturn));
         LocalFree(pLineTranCaps);
         pLineTranCaps = NULL;
         break;
      }
      if (pLineTranCaps->dwNeededSize > pLineTranCaps->dwTotalSize)
      {
         dwNeededSize = pLineTranCaps->dwNeededSize;
         ::LocalFree(pLineTranCaps);
         pLineTranCaps = NULL;
         continue;
      }
      break;
   }

   ::lineShutdown(hLineApp);

   // TODO - zonify - DEBUGMSG(ZONE_FUNCTION, (TEXT("-TapiLineGetTranslateCaps Returning 0x%X\r\n"), pLineTranCaps));

   return pLineTranCaps;
}

DWORD RasConnInfo::GetDefaultCntryRgnCode(LPLINETRANSLATECAPS pCaps)
{
   DWORD dwCntryRgnCode = 0;

   if (pCaps && pCaps->dwLocationListSize )
   {
      LPLINELOCATIONENTRY pLocEntry = (LPLINELOCATIONENTRY) ((LPBYTE)pCaps + pCaps->dwLocationListOffset);

      for (int i=0; i < (int) pCaps->dwNumLocations; i++)
      {
         if (pLocEntry[i].dwPermanentLocationID == pCaps->dwCurrentLocationID)
         {
            dwCntryRgnCode = pLocEntry[i].dwCountryCode;
         }
      }
   }

   return dwCntryRgnCode;
}

void RasConnInfo::GetDefaultAreaCode(LPLINETRANSLATECAPS pCaps, TCHAR * pszAreaCode, DWORD dwMaxLen)
{
   if (!pszAreaCode)
      return;

   *pszAreaCode = _T('\0');

   if (pCaps && pCaps->dwLocationListSize)
   {
      LPLINELOCATIONENTRY pLocEntry = (LPLINELOCATIONENTRY) ((LPBYTE)pCaps + pCaps->dwLocationListOffset);

      for(int i = 0; i < (int) pCaps->dwNumLocations; i++)
      {
         if(pLocEntry[i].dwPermanentLocationID == pCaps->dwCurrentLocationID)
         {
            _tcsncpy(pszAreaCode,
                     (LPTSTR)((LPBYTE)pCaps + pLocEntry[i].dwCityCodeOffset),
                     dwMaxLen);
         }
      }
   }
}

BOOL RasConnInfo::isValidRasInfoString(TCHAR * pszValidate)
{
   if (!pszValidate)
      return FALSE;

   for (int i = 0; i < (sizeof(szBadRasCharList)/sizeof(*szBadRasCharList)); i++)
   {
      if (::_tcschr(pszValidate, szBadRasCharList[i]))
         return FALSE;
   }

   return TRUE;
}

BOOL RasConnInfo::ConfigureTcpIp(HWND hParent, HINSTANCE hInstance)
{
   PROPSHEETPAGE psp[2] = {0};
   PROPSHEETHEADER psh = {0};
   TCHAR szTitle[30];

   psp[0].dwSize = sizeof(PROPSHEETPAGE);
   psp[0].hInstance = hInstance;
   psp[0].pszTemplate = MAKEINTRESOURCE(g_fPortrait?IDD_TCPIP_0_P:IDD_TCPIP_0);
   psp[0].pfnDlgProc = RasConnInfo::TCPDlgProc0;
   psp[0].lParam = (LPARAM) this;

   psp[1].dwSize = sizeof(PROPSHEETPAGE);
   psp[1].hInstance = hInstance;
   psp[1].pszTemplate = MAKEINTRESOURCE(g_fPortrait?IDD_TCPIP_1_P:IDD_TCPIP_1);
   psp[1].pfnDlgProc = RasConnInfo::TCPDlgProc1;
   psp[1].lParam = (LPARAM) this;

   LoadString(hInstance, IDS_TCPIP_PROP_TITLE, szTitle,
              sizeof(szTitle)/sizeof(*szTitle));
   psh.dwSize = sizeof(PROPSHEETHEADER);
   psh.dwFlags = PSH_PROPSHEETPAGE;
   psh.hwndParent = hParent;
   psh.hInstance = hInstance;
   psh.pszCaption = szTitle;
   psh.nPages = 2;
   psh.ppsp = psp;

   return CallCommCtrlFunc(PropertySheetW)(&psh);
}

BOOL CALLBACK RasConnInfo::TCPDlgProc0(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
   PROPSHEETPAGE * ppsp = reinterpret_cast<PROPSHEETPAGE *>(GetWindowLong(hDlg, GWL_USERDATA));
   RasConnInfo * pRasConn = NULL;
   if (ppsp) pRasConn = reinterpret_cast<RasConnInfo *>(ppsp->lParam);

   switch (msg)
   {
      case WM_INITDIALOG:
      {
         ::SetWindowLong(hDlg, GWL_USERDATA, lParam);
         ppsp = reinterpret_cast<PROPSHEETPAGE *>(lParam);
         ASSERT(ppsp);
         pRasConn = reinterpret_cast<RasConnInfo *>(ppsp->lParam);
         ASSERT(pRasConn);

         ::SetWindowLong(GetParent(hDlg), GWL_EXSTYLE,
                         ::GetWindowLong(GetParent(hDlg), GWL_EXSTYLE) |
                         WS_EX_NODRAG);

         ::SetWindowText(::GetDlgItem(hDlg, IDC_CONNLABEL), pRasConn->pszName);

         SetCheckRasOptions(hDlg, pRasConn, TCPDlgOptionMap0);

         if (pRasConn->RasEntry.dwfOptions & RASEO_SpecificIpAddr)
         {
            ::SendDlgItemMessage(hDlg, IDC_SERVIPADDR, IP_SETADDRESS, 0,
                                 (LPARAM) (*((DWORD *)&(pRasConn->RasEntry.ipaddr))));
         }
         else
         {
            ::EnableWindow(::GetDlgItem(hDlg, IDC_SERVIPADDR), FALSE);
         }

         if (RASFP_Slip == pRasConn->RasEntry.dwFramingProtocol)
            ::SendDlgItemMessage(hDlg, IDC_USESLIP, BM_SETCHECK, (WPARAM) BST_CHECKED, 0);

         //
         // If it's a VPN or PPPoE type then disable the SLIP option
         //

         if (!_tcscmp(RASDT_Vpn,   pRasConn->RasEntry.szDeviceType) ||
             !_tcscmp(RASDT_PPPoE, pRasConn->RasEntry.szDeviceType))
         {
            ::EnableWindow(::GetDlgItem(hDlg, IDC_USESLIP), FALSE);
         }
      }
      break;

      case WM_COMMAND:
      {
         if (IDC_USESERVIP == LOWORD(wParam))
         {
            if (BST_CHECKED == ::SendDlgItemMessage(hDlg, IDC_USESERVIP, BM_GETCHECK, 0, 0))
            {
               ::SendDlgItemMessage(hDlg, IDC_SERVIPADDR, IP_CLEARADDRESS, 0, 0);
               ::EnableWindow(::GetDlgItem(hDlg, IDC_SERVIPADDR), FALSE);
            }
            else
            {
               ::SendDlgItemMessage(hDlg, IDC_SERVIPADDR, IP_SETADDRESS, 0,
                                    (LPARAM) (*((DWORD *)&(pRasConn->RasEntry.ipaddr))));
               ::EnableWindow(::GetDlgItem(hDlg, IDC_SERVIPADDR), TRUE);
            }
         }
      }
      break;

      case WM_NOTIFY:
      {
         if (PSN_APPLY == (reinterpret_cast<NMHDR *>(lParam))->code)
         {
            if (!pRasConn) {
               // pRasConn shouldn't be NULL, but if it is, return FALSE
               return FALSE;
            }

            // Even if they want server assigned save the IP addr...
            ::SendDlgItemMessage(hDlg, IDC_SERVIPADDR, IPM_GETADDRESS,
                                 0, (LPARAM) &(pRasConn->RasEntry.ipaddr));

            UpdateRasOptions(hDlg, pRasConn, TCPDlgOptionMap0);

            // Grab the rest of the data off of this sheet
            if (BST_CHECKED == ::SendDlgItemMessage(hDlg, IDC_USESLIP, BM_GETCHECK, 0, 0))
               pRasConn->RasEntry.dwFramingProtocol = RASFP_Slip;
            else
               pRasConn->RasEntry.dwFramingProtocol = RASFP_Ppp;
         }
         ::SetWindowLong(hDlg, DWL_MSGRESULT, 0);
      }
      break;

      default:
         return FALSE;
   }

   return TRUE;
}

BOOL CALLBACK RasConnInfo::TCPDlgProc1(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
   PROPSHEETPAGE * ppsp = reinterpret_cast<PROPSHEETPAGE *>(GetWindowLong(hDlg, GWL_USERDATA));
   RasConnInfo * pRasConn = NULL;
   if (ppsp) pRasConn = reinterpret_cast<RasConnInfo *>(ppsp->lParam);

   switch (msg)
   {
      case WM_INITDIALOG:
      {
         ::SetWindowLong(hDlg, GWL_USERDATA, lParam);
         ppsp = reinterpret_cast<PROPSHEETPAGE *>(lParam);
         ASSERT(ppsp);
         pRasConn = reinterpret_cast<RasConnInfo *>(ppsp->lParam);
         ASSERT(pRasConn);

         ::SetWindowLong(GetParent(hDlg), GWL_EXSTYLE,
                         ::GetWindowLong(GetParent(hDlg), GWL_EXSTYLE) |
                         WS_EX_NODRAG);

         ::SetWindowText(::GetDlgItem(hDlg, IDC_CONNLABEL), pRasConn->pszName);

         // Set the IPs properly if the SpecificNameServer flag is set
         // otherwise grey everything and check the box
         if (RASEO_SpecificNameServers & pRasConn->RasEntry.dwfOptions)
         {
            ::SendDlgItemMessage(hDlg, IDC_NAMESERVADDR, BM_SETCHECK,
                                 BST_UNCHECKED, 0);
            ::SendDlgItemMessage(hDlg, IDC_DNS_IPADDR, IP_SETADDRESS, 0,
                                 (LPARAM) (*((DWORD *)&(pRasConn->RasEntry.ipaddrDns))));
            ::SendDlgItemMessage(hDlg, IDC_DNS_ALTIPADDR, IP_SETADDRESS, 0,
                                 (LPARAM) (*((DWORD *)&(pRasConn->RasEntry.ipaddrDnsAlt))));
            ::SendDlgItemMessage(hDlg, IDC_WINS_IPADDR, IP_SETADDRESS, 0,
                                 (LPARAM) (*((DWORD *)&(pRasConn->RasEntry.ipaddrWins))));
            ::SendDlgItemMessage(hDlg, IDC_WINS_ALTIPADDR, IP_SETADDRESS, 0,
                                 (LPARAM) (*((DWORD *)&(pRasConn->RasEntry.ipaddrWinsAlt))));
         }
         else
         {
            ::SendDlgItemMessage(hDlg, IDC_NAMESERVADDR, BM_SETCHECK,
                                 BST_CHECKED, 0);
            ::EnableWindow(::GetDlgItem(hDlg, IDC_DNS_LABEL), FALSE);
            ::EnableWindow(::GetDlgItem(hDlg, IDC_DNS_IPADDR), FALSE);
            ::EnableWindow(::GetDlgItem(hDlg, IDC_DNS_ALTLABEL), FALSE);
            ::EnableWindow(::GetDlgItem(hDlg, IDC_DNS_ALTIPADDR), FALSE);
            ::EnableWindow(::GetDlgItem(hDlg, IDC_WINS_LABEL), FALSE);
            ::EnableWindow(::GetDlgItem(hDlg, IDC_WINS_IPADDR), FALSE);
            ::EnableWindow(::GetDlgItem(hDlg, IDC_WINS_ALTLABEL), FALSE);
            ::EnableWindow(::GetDlgItem(hDlg, IDC_WINS_ALTIPADDR), FALSE);
         }
      }
      break;

      case WM_COMMAND:
      {
         if (IDC_NAMESERVADDR != LOWORD(wParam))
            break;

         BOOL fUseSpecificNameServers =
            (BST_CHECKED != SendMessage(::GetDlgItem(hDlg, IDC_NAMESERVADDR),
                                        BM_GETCHECK, 0, 0));

         ::EnableWindow(::GetDlgItem(hDlg, IDC_DNS_LABEL), fUseSpecificNameServers);
         ::EnableWindow(::GetDlgItem(hDlg, IDC_DNS_IPADDR), fUseSpecificNameServers);
         ::EnableWindow(::GetDlgItem(hDlg, IDC_DNS_ALTLABEL), fUseSpecificNameServers);
         ::EnableWindow(::GetDlgItem(hDlg, IDC_DNS_ALTIPADDR), fUseSpecificNameServers);
         ::EnableWindow(::GetDlgItem(hDlg, IDC_WINS_LABEL), fUseSpecificNameServers);
         ::EnableWindow(::GetDlgItem(hDlg, IDC_WINS_IPADDR), fUseSpecificNameServers);
         ::EnableWindow(::GetDlgItem(hDlg, IDC_WINS_ALTLABEL), fUseSpecificNameServers);
         ::EnableWindow(::GetDlgItem(hDlg, IDC_WINS_ALTIPADDR), fUseSpecificNameServers);

         if (fUseSpecificNameServers)
         {
            ::SendDlgItemMessage(hDlg, IDC_DNS_IPADDR, IP_SETADDRESS, 0,
                                 (LPARAM) (*((DWORD *)&(pRasConn->RasEntry.ipaddrDns))));
            ::SendDlgItemMessage(hDlg, IDC_DNS_ALTIPADDR, IP_SETADDRESS, 0,
                                 (LPARAM) (*((DWORD *)&(pRasConn->RasEntry.ipaddrDnsAlt))));
            ::SendDlgItemMessage(hDlg, IDC_WINS_IPADDR, IP_SETADDRESS, 0,
                                 (LPARAM) (*((DWORD *)&(pRasConn->RasEntry.ipaddrWins))));
            ::SendDlgItemMessage(hDlg, IDC_WINS_ALTIPADDR, IP_SETADDRESS, 0,
                                 (LPARAM) (*((DWORD *)&(pRasConn->RasEntry.ipaddrWinsAlt))));
         }
         else
         {
            ::SendDlgItemMessage(hDlg, IDC_DNS_IPADDR, IP_CLEARADDRESS, 0, 0);
            ::SendDlgItemMessage(hDlg, IDC_DNS_ALTIPADDR, IP_CLEARADDRESS, 0, 0);
            ::SendDlgItemMessage(hDlg, IDC_WINS_IPADDR, IP_CLEARADDRESS, 0, 0);
            ::SendDlgItemMessage(hDlg, IDC_WINS_ALTIPADDR, IP_CLEARADDRESS, 0, 0);
         }
      }
      break;

      case WM_NOTIFY:
      {
         if (PSN_APPLY == (reinterpret_cast<NMHDR *>(lParam))->code)
         {
            if (!pRasConn) {
               // pRasConn shouldn't be NULL, but if it is, return FALSE
               return FALSE;
            }

            ::SendDlgItemMessage(hDlg, IDC_DNS_IPADDR, IP_GETADDRESS,
                                 0, (LPARAM) &(pRasConn->RasEntry.ipaddrDns));
            ::SendDlgItemMessage(hDlg, IDC_DNS_ALTIPADDR, IP_GETADDRESS,
                                 0, (LPARAM) &(pRasConn->RasEntry.ipaddrDnsAlt));
            ::SendDlgItemMessage(hDlg, IDC_WINS_IPADDR, IP_GETADDRESS,
                                 0, (LPARAM) &(pRasConn->RasEntry.ipaddrWins));
            ::SendDlgItemMessage(hDlg, IDC_WINS_ALTIPADDR, IP_GETADDRESS,
                                 0, (LPARAM) &(pRasConn->RasEntry.ipaddrWinsAlt));

            if (BST_CHECKED == ::SendDlgItemMessage(hDlg, IDC_NAMESERVADDR,
                                                    BM_GETCHECK, 0, 0))
            {
                pRasConn->RasEntry.dwfOptions &= ~RASEO_SpecificNameServers;
            }
            else
            {
                pRasConn->RasEntry.dwfOptions |= RASEO_SpecificNameServers;
            }
         }
         ::SetWindowLong(hDlg, DWL_MSGRESULT, 0);
      }
      break;

      default:
         return FALSE;
   }

   return TRUE;
}

typedef struct _EAP_CB_ITEMDATA {
    int index;
    PEAP_EXTENSION_INFO pExtInfo;
} EAP_CB_ITEMDATA, * PEAP_CB_ITEMDATA;

void
RasConnInfo::ClearEapConnData()
//
//  Clear current Eap conn data, if any.
//
{
    LocalFree(m_pEapConnData);
    m_pEapConnData = NULL;
    m_cbEapConnData = 0;
}

DWORD
RasConnInfo::ChangeEapConnData(
	IN PVOID pNewConnData,
	IN DWORD cbNewConnData)
//
//  Change current Eap conn data into the specified new conn data.
//
{
	DWORD dwResult = NO_ERROR;
    if (cbNewConnData <0 || cbNewConnData > MAX_EAP_DATA_SIZE)
    {
        dwResult = ERROR_INVALID_PARAMETER;
    }else{

	    ClearEapConnData();

        if (cbNewConnData)
	    {
		    m_pEapConnData = (PBYTE)LocalAlloc(LPTR, cbNewConnData);
		    if (!m_pEapConnData)
		    {
			    dwResult = ERROR_OUTOFMEMORY;
		    }
		    else
		    {
			    m_cbEapConnData = cbNewConnData;
			    memcpy(m_pEapConnData, pNewConnData, cbNewConnData);
		    }
	    }
    }

	return dwResult;
}

DWORD
RasConnInfo::GetEapConnData()
//
//  Retrieve current EAP Connection Settings from the system for the connectoid.
//
{
	DWORD dwResult;
	DWORD cbData;

	// Free existing conn data
	ClearEapConnData();

	// Get system setting for eap conn data
	cbData = 0;
	dwResult = ::RasGetEapConnectionData(NULL, pszName, NULL, &cbData);
    if ((dwResult == ERROR_SUCCESS) && cbData)
	{
		m_pEapConnData = (PBYTE)LocalAlloc(LPTR, cbData);
		if (!m_pEapConnData)
		{
			dwResult = ERROR_OUTOFMEMORY;
		}
		else
		{
			m_cbEapConnData = cbData;
			dwResult = ::RasGetEapConnectionData(NULL, pszName, m_pEapConnData, &cbData);
		}
	}

	return dwResult;
}

DWORD
RasConnInfo::SetEapConnData()
//
//  Store current EAP Connection Settings to the system for the connectoid.
//
{
	DWORD dwResult;

	dwResult = ::RasSetEapConnectionData(NULL, pszName, m_pEapConnData, m_cbEapConnData);

	return dwResult;
}

DWORD
RasConnInfo::EapEnumExtensions()
//
//  Build an array containining information about all the EAP extensions
//  available in the system.
//
{
	DWORD                 dwResult = NO_ERROR;
	DWORD                 cbEapExtInfo = 0;

	// Don't reenumerate if we have already enumerated the info
	if (m_pEapExtInfo == NULL)
	{
		m_cEapExtInfo = 0;

		while (TRUE)
		{
			dwResult = ::EapEnumExtensions(cbEapExtInfo, m_pEapExtInfo, &m_cEapExtInfo, &cbEapExtInfo);
			if (dwResult != ERROR_INSUFFICIENT_BUFFER)
				break;

			// Free old buffer
			LocalFree(m_pEapExtInfo);

			// Allocate new bigger buffer
			m_pEapExtInfo = (PEAP_EXTENSION_INFO)LocalAlloc(LPTR, cbEapExtInfo);
			if (m_pEapExtInfo == NULL)
			{
				DEBUGMSG(1, (L"CONNMC: EapEnumExtensions LocalAlloc(%u) failed\n", cbEapExtInfo));
				m_cEapExtInfo = 0;
				dwResult = ERROR_OUTOFMEMORY;
				break;
			}
		}
	}
	DEBUGMSG(dwResult, (L"CONNMC: EapEnumExtensions failed %u\n", dwResult));
	return dwResult;
}

BOOL RasConnInfo::InitEAPControls(HWND hDlg, RasConnInfo * pRasConn)
{
    PEAP_EXTENSION_INFO pCurExt;
    PEAP_CB_ITEMDATA    pCbData;
    PEAP_CB_ITEMDATA    pCurCbData;
    int                 i;
    int                 iListIndex;
    HWND                hEAPList;
    BOOL                bCurSelSet;

    bCurSelSet = FALSE;
    pCbData = NULL;

	EapEnumExtensions();

    pCbData = (PEAP_CB_ITEMDATA)LocalAlloc(LPTR, m_cEapExtInfo * sizeof(EAP_CB_ITEMDATA));
    if (NULL == pCbData) {
        DEBUGMSG(1, (L"CONNMC:InitEAPControls LocalAlloc(ITEMDATA) failed\n"));
        goto iec_fail;
    }

    hEAPList = GetDlgItem(hDlg, IDC_CA_LB_EAPPACKAGES);

    for (i = 0, pCurExt = m_pEapExtInfo, pCurCbData = pCbData;
         i < (int)m_cEapExtInfo;
         i++, pCurExt++, pCurCbData++) {
        iListIndex = ComboBox_AddString(hEAPList, pCurExt->wszFriendlyName);
        pCurCbData->index = i;
        pCurCbData->pExtInfo = pCurExt;
        ComboBox_SetItemData(hEAPList, iListIndex, pCurCbData);
        if (pRasConn->RasEntry.dwCustomAuthKey == pCurExt->dwExtensionType) {
            ComboBox_SetCurSel(hEAPList, iListIndex);
            bCurSelSet = TRUE;
        }
    }
    if (FALSE == bCurSelSet) {
        ComboBox_SetCurSel(hEAPList, 0);
    }
    return TRUE;

iec_fail:
     LocalFree(pCbData);
    return FALSE;
}   // InitEAPControls

PEAP_CB_ITEMDATA GetCurrentEAPItemData(HWND hDlg)
{
    PEAP_CB_ITEMDATA pCbData;
    int iListIndex;
    HWND hEAPList;

    hEAPList = GetDlgItem(hDlg, IDC_CA_LB_EAPPACKAGES);
    iListIndex = ComboBox_GetCurSel(hEAPList);
    if (iListIndex != CB_ERR) {
        pCbData = (PEAP_CB_ITEMDATA)ComboBox_GetItemData(hEAPList, iListIndex);
        if (pCbData && ((DWORD)pCbData != CB_ERR)) {
            return pCbData;
        }
    }
    return NULL;
}   // GetCurrentEAPItemData

static void  EnableNonEAPRasOptions(HWND hDlg, BOOL bEnable)
{

    ::EnableWindow(::GetDlgItem(hDlg,IDC_CA_CB_PAP), bEnable);
    ::EnableWindow(::GetDlgItem(hDlg,IDC_CA_CB_CHAP), bEnable);
    ::EnableWindow(::GetDlgItem(hDlg,IDC_CA_CB_MSCHAP), bEnable);
    ::EnableWindow(::GetDlgItem(hDlg,IDC_CA_CB_MSCHAP2), bEnable);
}


BOOL  EnableEAPControls(HWND hDlg)
{
    PEAP_CB_ITEMDATA pCbData;
    BOOL bEnable, bEnableProperties;
    bEnableProperties = FALSE;

    bEnable = (BST_CHECKED == ::SendDlgItemMessage(hDlg, IDC_CA_CB_EAP, BM_GETCHECK, 0, 0));
    if (bEnable) {
        pCbData = GetCurrentEAPItemData(hDlg);
        if (pCbData) {
            bEnableProperties = (pCbData->pExtInfo->wszConfigUIPath && pCbData->pExtInfo->wszConfigUIPath[0]);
        }
    }
    EnableWindow(GetDlgItem(hDlg, IDC_CA_PB_PROPERTIES), bEnableProperties);
    return bEnable;
}   // EnableEAPControls

void CleanupEAPControls(HWND hDlg)
{
    PEAP_CB_ITEMDATA pCbData;
    HWND hEAPList;
    int i, j;
    PEAP_CB_ITEMDATA pCbDataList;

    pCbDataList = NULL;
    hEAPList = GetDlgItem(hDlg, IDC_CA_LB_EAPPACKAGES);
    j = ComboBox_GetCount(hEAPList);

    for (i=0;i<j;i++) {
        pCbData = (PEAP_CB_ITEMDATA)ComboBox_GetItemData(hEAPList,i);
        if (pCbData && ((int)pCbData != CB_ERR)) {
            if (0 == pCbData->index) {
                //
                // The zero-th item has the list pointers
                //
                pCbDataList = pCbData;
            }
        }
    }

    LocalFree(pCbDataList);
}   // CleanupEAPControls

void RasConnInfo::SaveEAPConnectionData(HWND hDlg, RasConnInfo * pRasConn)
{
    PEAP_CB_ITEMDATA pCbData;

    if (BST_CHECKED == ::SendDlgItemMessage(hDlg, IDC_CA_CB_EAP, BM_GETCHECK, 0, 0)) {
        pCbData = GetCurrentEAPItemData(hDlg);
        if (pCbData) {
            pRasConn->RasEntry.dwCustomAuthKey = pCbData->pExtInfo->dwExtensionType;

			// Save the EAP Conn data to be saved via RasSetEapConnectionData later,
			// after the RAS Connectoid is created.
        }
    }
}   // SaveEAPConnectionData

BOOL RasConnInfo::ConfigureSecurity(HWND hParent, HINSTANCE hInstance)
{
    PROPSHEETPAGE psp[1] = {0};
    PROPSHEETHEADER psh = {0};
    TCHAR szTitle[30];

    psp[0].dwSize = sizeof(PROPSHEETPAGE);
    psp[0].hInstance = hInstance;
    psp[0].pszTemplate = MAKEINTRESOURCE(g_fPortrait?IDD_SECURITY_SETTINGS_P:IDD_SECURITY_SETTINGS);
    psp[0].pfnDlgProc = RasConnInfo::SecurityDlgProc;
    psp[0].lParam = (LPARAM) this;

    LoadString(hInstance, IDS_SECURITY_TITLE, szTitle,
              sizeof(szTitle)/sizeof(*szTitle));
    psh.dwSize = sizeof(PROPSHEETHEADER);
    psh.dwFlags = PSH_PROPSHEETPAGE;
    psh.hwndParent = hParent;
    psh.hInstance = hInstance;
    psh.pszCaption = szTitle;
    psh.nPages = 1;
    psh.ppsp = psp;

    return CallCommCtrlFunc(PropertySheetW)(&psh);
}

BOOL CALLBACK RasConnInfo::SecurityDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    PROPSHEETPAGE * ppsp = reinterpret_cast<PROPSHEETPAGE *>(GetWindowLong(hDlg, GWL_USERDATA));
    RasConnInfo * pRasConn = NULL;
    WPARAM wCmdId;
    WPARAM wCmdCbn;

    if (ppsp) pRasConn = reinterpret_cast<RasConnInfo *>(ppsp->lParam);

    switch (msg)
    {
    case WM_INITDIALOG:
    {
        ::SetWindowLong(hDlg, GWL_USERDATA, lParam);
        ppsp = reinterpret_cast<PROPSHEETPAGE *>(lParam);
        ASSERT(ppsp);
        pRasConn = reinterpret_cast<RasConnInfo *>(ppsp->lParam);
        ASSERT(pRasConn);

        ::SetWindowLong(GetParent(hDlg), GWL_EXSTYLE,
                ::GetWindowLong(GetParent(hDlg), GWL_EXSTYLE) | WS_EX_NODRAG);

        if (!pRasConn->InitEAPControls(hDlg, pRasConn)) {
            pRasConn->RasEntry.dwfOptions |= RASEO_ProhibitEAP;
        }

	if(!_tcscmp(pRasConn->RasEntry.szDeviceType, RASDT_Modem))
	{
	    ::SendDlgItemMessage(hDlg, IDC_CA_CB_PREVIEWUSERPW, BM_SETCHECK, (WPARAM) BST_CHECKED, 0);
	    EnableWindow(::GetDlgItem(hDlg, IDC_CA_CB_PREVIEWUSERPW), FALSE);
	}

        // Hide check box that allows RAS to use Windows default credentials. This feature
        // is not available
        ::ShowWindow(::GetDlgItem(hDlg, IDC_CA_CB_USEWINDOWSPW), SW_HIDE);
        SetCheckRasOptions(hDlg, pRasConn, SecurityDlgOptionMap);
        // Enable CHAP, MSCHAP etc iff EAP is not checked
        EnableNonEAPRasOptions(hDlg, !EnableEAPControls(hDlg));
    }
    break;

    case WM_COMMAND:
    {
        wCmdId = LOWORD(wParam);
        wCmdCbn = HIWORD(wParam);

        switch (wCmdId) {
        case IDC_CA_CB_EAP:
            EnableNonEAPRasOptions(hDlg, !EnableEAPControls(hDlg));
                
            break;

        case IDC_CA_LB_EAPPACKAGES:
            if (wCmdCbn == CBN_SELCHANGE) {
				if (pRasConn)
					pRasConn->ClearEapConnData();
                EnableEAPControls(hDlg);
            }
            break;

        case IDC_CA_PB_PROPERTIES:
            // respond to the "Properties" button by calling RasGetEapConnectionData and RasEapInvokeConfigUI
            {
                HINSTANCE hinstEAPDll;
                PEAP_CB_ITEMDATA pEapCbItemData;
                PEAP_EXTENSION_INFO pCurExt=NULL;
                DWORD dwError;
                UCHAR *pConnectionDataOut=NULL;
                DWORD dwSizeOfConnectionDataOut=0;

                pEapCbItemData = GetCurrentEAPItemData(hDlg);
                if (pEapCbItemData)
                    pCurExt = pEapCbItemData->pExtInfo;

                if (pCurExt && pCurExt->wszConfigUIPath) {
                    RASEAPINVOKECONFIGUI pfnEapConfig;
                    RASEAPFREE pfnEapFreeMem;

                    hinstEAPDll = LoadLibrary(pCurExt->wszConfigUIPath);
                    if (hinstEAPDll == NULL) {
                        DEBUGMSG(1,(TEXT("CONNMC: Couldn't load EAP extension DLL %s\r\n"),pCurExt->wszConfigUIPath));
                        break;
                    }
                    pfnEapConfig = (RASEAPINVOKECONFIGUI)GetProcAddress(hinstEAPDll,TEXT("RasEapInvokeConfigUI"));
                    pfnEapFreeMem= (RASEAPFREE)GetProcAddress(hinstEAPDll,TEXT("RasEapFreeMemory"));
                    if ((pfnEapConfig) == NULL || (pfnEapFreeMem == NULL)) {
                        DEBUGMSG(1,(TEXT("CONNMC: Couldn't get entry points from EAP extension DLL %s\r\n"),pCurExt->wszConfigUIPath));
                        FreeLibrary(hinstEAPDll);
                        break;
                    }

                    // Call into config UI
                    __try {
                        dwError = pfnEapConfig(pCurExt->dwExtensionType,
                                               hDlg,
                                               0, // flags TBD
                                               pRasConn->m_pEapConnData,
                                               pRasConn->m_cbEapConnData,
                                               &pConnectionDataOut,
                                               &dwSizeOfConnectionDataOut);
                    } __except(EXCEPTION_EXECUTE_HANDLER) {
                          FreeLibrary(hinstEAPDll);
                          break;
                    }

					DEBUGMSG(ZONE_ERROR, (TEXT("CONNMC: Error 0x%X returned from EAP extension DLL %s\r\n"),
                                             dwError, pCurExt->wszConfigUIPath));

                    if (dwError == ERROR_SUCCESS)
                    {
                        if (dwSizeOfConnectionDataOut < 0 || dwSizeOfConnectionDataOut > MAX_EAP_DATA_SIZE)
                        {
                            // Out of bounds for the size of the structure
                            ASSERT(0); // control shouldn't be enabled in this case
					        DEBUGMSG(ZONE_ERROR, (TEXT("CONNMC: Error dwSizeOfConnectionDataOut out of bounds (>=0 && <= MAX_EAP_DATA_SIZE) returned from EAP extension DLL %s\r\n"),
                                                 pCurExt->wszConfigUIPath));
                        }else{
						    pRasConn->ChangeEapConnData(pConnectionDataOut, dwSizeOfConnectionDataOut);
						    pfnEapFreeMem(pConnectionDataOut);
                        }
                    }

                    FreeLibrary(hinstEAPDll);
                } else {
                    ASSERT(0); // control shouldn't be enabled in this case
                }
            }
            break;
        }
    }
    break;  // WM_COMMAND

    case WM_NOTIFY:
    {
        switch((reinterpret_cast<NMHDR *>(lParam))->code) {
        case PSN_APPLY:
          if (!pRasConn) {
             // pRasConn shouldn't be NULL, but if it is, return FALSE
             return FALSE;
          }

          UpdateRasOptions(hDlg, pRasConn, SecurityDlgOptionMap);
          pRasConn->SaveEAPConnectionData(hDlg, pRasConn);
          // FALLTHROUGH

        case PSN_QUERYCANCEL:
          CleanupEAPControls(hDlg);
          break;
        }
    }
    break;

    default:
        return FALSE;
    }

    return TRUE;
}

void RasConnInfo::SetCheckRasOptions(HWND hDlg,  RasConnInfo * pRasConn, PRASOPTIONMAP pMap)
{
    DWORD dwTmp;

    while (pMap->dwfOptionBit) {
        dwTmp = (pMap->dwfOptionBit & pRasConn->RasEntry.dwfOptions) ? 1 : 0;
        if (dwTmp ^ pMap->dwDirection) {
            ::SendDlgItemMessage(hDlg, pMap->dwControlId, BM_SETCHECK, (WPARAM) BST_CHECKED, 0);
        }
        pMap++;
    }
}

void RasConnInfo::UpdateRasOptions(HWND hDlg,  RasConnInfo * pRasConn, PRASOPTIONMAP pMap)
{
    DWORD dwDlg;

	while (pMap->dwfOptionBit) {
        dwDlg = (BST_CHECKED == ::SendDlgItemMessage(hDlg, pMap->dwControlId, BM_GETCHECK, 0, 0)) ? 1 : 0;
		if (dwDlg ^ pMap->dwDirection) {
            pRasConn->RasEntry.dwfOptions |= pMap->dwfOptionBit;
        } else {
            pRasConn->RasEntry.dwfOptions &= ~(pMap->dwfOptionBit);
        }
        pMap++;
    }

    if(pRasConn->RasEntry.dwfOptions & RASEO_RequireDataEncryption)
	{
	    pRasConn->RasEntry.dwfOptions |= RASEO_RequireMsEncryptedPw;
	}
    else
	{
	    pRasConn->RasEntry.dwfOptions &= ~(RASEO_RequireMsEncryptedPw);
	}
}
