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

#include "filemenu.h"
#include "idlist.h"
#include "resource.h"

//////////////////////////////////////////////////
// CFileMenu

CFileMenu::CFileMenu() :
   m_ObjRefCount(0),
   m_pShellFolder(NULL),
   m_cidl(0),
   m_apidl(NULL),
   m_dwVerbLookups(0),
   m_pVerbLookups(NULL)
{
   m_ObjRefCount = 1;
}
 
CFileMenu::~CFileMenu()
{
   Destroy();
}

void CFileMenu::Destroy()
{
   if (m_pShellFolder)
   {
      m_pShellFolder->Release();
      m_pShellFolder = NULL;
   }

   if (m_apidl)
   {
      for (UINT i = 0; i < m_cidl; i++)
      {
         if (m_apidl[i])
            ILFree(m_apidl[i]);
      }
      g_pShellMalloc->Free(m_apidl);
      m_apidl = NULL;
   }
   m_cidl = 0;

   if (m_pVerbLookups)
   {
      for (DWORD i = 0; i < m_dwVerbLookups; i++)
      {
         if (m_pVerbLookups[i].m_pwszVerb)
            delete [] m_pVerbLookups[i].m_pwszVerb;
      }
      delete [] m_pVerbLookups;
      m_pVerbLookups = NULL;
   }
   m_dwVerbLookups = 0;
}

BOOL CFileMenu::Initialize(IShellFolder * pShellFolder, UINT cidl, LPCITEMIDLIST * apidl)
{
   if (!pShellFolder || (0 == cidl) || !apidl)
      return FALSE;

   BOOL fSuccess = FALSE;

   m_pShellFolder = pShellFolder;
   m_pShellFolder->AddRef();

   m_cidl = cidl;

   m_apidl = (LPCITEMIDLIST *) g_pShellMalloc->Alloc(m_cidl * sizeof(LPITEMIDLIST));
   ASSERT(m_apidl);

   if (m_apidl)
   {
      UINT i = 0;

      while (i < m_cidl)
      {
         ASSERT(apidl[i]);
         if (apidl[i])
         {
            m_apidl[i] = ILCopy(apidl[i], IL_ALL);

            ASSERT(m_apidl[i]);
            if (!m_apidl[i])
               break;
         }
         else
         {
            break;
         }

         i++;
      }

      if (i == m_cidl)
         fSuccess = TRUE;
   }

   if (!fSuccess)
      Destroy();

   return fSuccess;
}


//////////////////////////////////////////////////
// IUnknown

STDMETHODIMP CFileMenu::QueryInterface(REFIID riid,
                                       LPVOID FAR * ppobj)
{
   HRESULT hr = E_NOINTERFACE;

   if (!ppobj)
   {
      ASSERT(0);
      hr = E_INVALIDARG;
      goto leave;
   }

   *ppobj = NULL;

   if(IsEqualIID(riid, IID_IUnknown)) // IUnknown
   {
      *ppobj = this;
   }
   else if(IsEqualIID(riid, IID_IContextMenu)) // IContextMenu
   {
      *ppobj = (IContextMenu *) this;
   }

   if(*ppobj)
   {
      (*(LPUNKNOWN*)ppobj)->AddRef();
      hr = S_OK;
   }

leave:
   return hr;
}

STDMETHODIMP_(ULONG) CFileMenu::AddRef(VOID)
{
   return ++m_ObjRefCount;
}

STDMETHODIMP_(ULONG) CFileMenu::Release(VOID)
{
   if (--m_ObjRefCount == 0)
   {
      delete this;
      return 0;
   }
   
   return m_ObjRefCount;
}


//////////////////////////////////////////////////
// IContextMenu 

STDMETHODIMP CFileMenu::GetCommandString(UINT_PTR idCmd,
                                         UINT uFlags,
                                         UINT * pwReserved,
                                         LPSTR pszName,
                                         UINT cchMax)
{
   return E_NOTIMPL;
}

STDMETHODIMP CFileMenu::InvokeCommand(LPCMINVOKECOMMANDINFO pici)
{
   if (!pici)
      return E_INVALIDARG;

   if (!m_pShellFolder || !m_apidl || !m_pVerbLookups)
      return E_UNEXPECTED;

   SHELLEXECUTEINFO sei = {0};
   STRRET str = {0};
   WCHAR sz[MAX_PATH];

   ASSERT(sizeof(CMINVOKECOMMANDINFO) == pici->cbSize);
   ASSERT(0 == HIWORD(pici->fMask));
   ASSERT(0 == HIWORD(pici->lpVerb));

   for (DWORD i = 0; i < m_dwVerbLookups; i++)
   {
      if (LOWORD(pici->lpVerb) == m_pVerbLookups[i].m_id)
      {
         sei.cbSize = sizeof(sei);
         sei.hwnd = pici->hwnd;
         sei.lpVerb = m_pVerbLookups[i].m_pwszVerb;
         sei.nShow = SW_SHOWNORMAL;

         break;
      }
   }

   if (sei.lpVerb)
   {
      for (UINT i = 0; i < m_cidl; i++)
      {
         m_pShellFolder->GetDisplayNameOf(m_apidl[i],
                                          SHGDN_NORMAL | SHGDN_FORPARSING,
                                          &str);
         if (SUCCEEDED(StrRetToBuf(&str, m_apidl[i], sz, lengthof(sz))))
         {
            sei.lpFile = sz;
            ::ShellExecuteEx(&sei);
         }
      }
   }

   return ((sei.lpVerb) ? NOERROR : E_FAIL);
}

STDMETHODIMP CFileMenu::QueryContextMenu(HMENU hmenu,
                                         UINT indexMenu,
                                         UINT idCmdFirst,
                                         UINT idCmdLast,
                                         UINT uFlags)
{
   ASSERT(hmenu);
   if (!hmenu)
      return E_INVALIDARG;

   ASSERT(m_pShellFolder);
   ASSERT(m_apidl);
   if (!m_pShellFolder || !m_apidl)
      return E_UNEXPECTED;

   HRESULT hr = NOERROR;
   STRRET str = {0};
   WCHAR wszBuffer[MAX_PATH];
   DWORD dwBuffer;
   LPWSTR pwszExtension = NULL;
   HKEY hKey = NULL;
   LONG lRet = 0;
   DWORD dwSubKeys = 0;
   DWORD dwEnumKeys = 0;
   MENUITEMINFO mii = {0};
   mii.cbSize = sizeof(mii);
   mii.fMask = MIIM_TYPE;

   // Get the name of the clicked file
   hr = m_pShellFolder->GetDisplayNameOf(*m_apidl,
                                         SHGDN_INFOLDER | SHGDN_FORPARSING,
                                         &str);
   if (FAILED(hr))
      goto QueryContextMenu_done;

   hr = StrRetToBuf(&str, *m_apidl, wszBuffer, lengthof(wszBuffer));
   if (FAILED(hr))
      goto QueryContextMenu_done;

   // Get the extension of the file in order to build the registry path
   pwszExtension = PathFindExtension(wszBuffer);
   if (!pwszExtension || !(*pwszExtension))
      goto QueryContextMenu_done;

   // Open the .xxx registry key
   lRet = ::RegOpenKeyEx(HKEY_CLASSES_ROOT, pwszExtension, 0, 0, &hKey);
   if (ERROR_SUCCESS != lRet)
   {
      hr = HRESULT_FROM_WIN32_I(GetLastError());
      goto QueryContextMenu_done;
   }

   // Query to get from .xxx to xxxfile
   dwBuffer = sizeof(wszBuffer);
   lRet = ::RegQueryValueEx(hKey, NULL, NULL, NULL, (LPBYTE) wszBuffer, &dwBuffer);
   if (ERROR_SUCCESS != lRet)
   {
      hr = HRESULT_FROM_WIN32_I(GetLastError());
      goto QueryContextMenu_done;
   }

   // Append the proper shell string to get xxxfile\Shell
   hr = StringCbCat(wszBuffer, sizeof(wszBuffer), TEXT("\\Shell"));
   if (FAILED(hr))
      goto QueryContextMenu_done;

   // Open xxxfile\Shell so we can enum the verb
   ::RegCloseKey(hKey); hKey = NULL;
   lRet = ::RegOpenKeyEx(HKEY_CLASSES_ROOT, wszBuffer, 0, 0, &hKey);
   if (ERROR_SUCCESS != lRet)
   {
      hr = HRESULT_FROM_WIN32_I(GetLastError());
      goto QueryContextMenu_done;
   }

   // Figure out how many verbs will need to be added to the menu
   lRet = ::RegQueryInfoKey(hKey, NULL, NULL, NULL, &dwSubKeys, NULL,
                            NULL, NULL, NULL, NULL, NULL, NULL);
   if (ERROR_SUCCESS != lRet)
   {
      hr = HRESULT_FROM_WIN32_I(GetLastError());
      goto QueryContextMenu_done;
   }

   // No verbs to add to the menu
   if (0 == dwSubKeys)
      goto QueryContextMenu_done;

   // Build the array that will store the verb lookups
   m_dwVerbLookups = 0;
   if (m_pVerbLookups) delete [] m_pVerbLookups;
   m_pVerbLookups = new VerbLookup[dwSubKeys];
   if (m_pVerbLookups)
   {
      for (DWORD i = 0; i < dwSubKeys; i++)
      {
         m_pVerbLookups[i].m_id = 0;
         m_pVerbLookups[i].m_pwszVerb = NULL;
      }
   }
   else
   {
      hr = E_OUTOFMEMORY;
      goto QueryContextMenu_done;
   }

   // Enum the verbs and put them into (or remove them from) the menu
   dwBuffer = lengthof(wszBuffer);
   while (ERROR_SUCCESS == ::RegEnumKeyEx(hKey, dwEnumKeys, wszBuffer,
                                          &dwBuffer, NULL, NULL, NULL, NULL))
   {
      if (CSTR_EQUAL != CompareString(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
                                      NORM_IGNORECASE, wszBuffer, -1, TEXT("open"), -1))
      {
         m_pVerbLookups[m_dwVerbLookups].m_pwszVerb = new TCHAR[dwBuffer+1];
         if (!m_pVerbLookups[m_dwVerbLookups].m_pwszVerb)
         {
            hr = E_OUTOFMEMORY;
            goto QueryContextMenu_done;
         }

         _tcscpy(m_pVerbLookups[m_dwVerbLookups].m_pwszVerb, wszBuffer);

         if (CSTR_EQUAL == CompareString(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
                                         NORM_IGNORECASE, wszBuffer, -1, TEXT("edit"), -1))
         {
            ::StringCchCopyEx(wszBuffer, lengthof(wszBuffer),
                              LOAD_STRING(IDS_MENU_FILE_EDIT),
                              NULL, NULL, STRSAFE_NULL_ON_FAILURE);
         }
         if (CSTR_EQUAL == CompareString(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
                                         NORM_IGNORECASE, wszBuffer, -1, TEXT("print"), -1))
         {
            ::StringCchCopyEx(wszBuffer, lengthof(wszBuffer),
                              LOAD_STRING(IDS_MENU_FILE_PRINT),
                              NULL, NULL, STRSAFE_NULL_ON_FAILURE);
         }
         else
         {
            dwBuffer = sizeof(wszBuffer);
            lRet = ::RegQueryValueEx(hKey, NULL, (LPDWORD) wszBuffer, NULL,
                                     (LPBYTE) wszBuffer, &dwBuffer);
            if (ERROR_SUCCESS != lRet)
               _tcscpy(wszBuffer, m_pVerbLookups[m_dwVerbLookups].m_pwszVerb);
         }

         if (::InsertMenu(hmenu, 1, MF_BYPOSITION | MF_STRING,
                          idCmdFirst+m_dwVerbLookups, wszBuffer))
         {
            m_pVerbLookups[m_dwVerbLookups].m_id = idCmdFirst+m_dwVerbLookups;
            m_dwVerbLookups++;
         }
         else
         {
            delete [] m_pVerbLookups[m_dwVerbLookups].m_pwszVerb;
            m_pVerbLookups[m_dwVerbLookups].m_pwszVerb = NULL;
         }
      }
      else
      {
         // This item is handled by the standard menu (IDC_FILE_OPEN)
         dwSubKeys--;
      }

      dwBuffer = lengthof(wszBuffer);
      dwEnumKeys++;
   }
   ASSERT(m_dwVerbLookups == dwSubKeys); // Make sure we entered every verb

QueryContextMenu_done:

   if (hKey)
      ::RegCloseKey(hKey);

   if (SUCCEEDED(hr))
      hr = MAKE_HRESULT(SEVERITY_SUCCESS, 0, m_dwVerbLookups+1);

   return hr;
}

