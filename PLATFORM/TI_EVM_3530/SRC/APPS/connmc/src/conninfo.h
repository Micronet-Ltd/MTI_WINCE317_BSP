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
#ifndef CONN_INFO_H__
#define CONN_INFO_H__

#pragma warning (disable:4100) // 'identifier' : unreferenced formal parameter
#pragma warning (disable:4127) // conditional expresion is constant
#pragma warning (disable:4514) // unreferenced inline/local function has been removed
#pragma warning (disable:4710) // 'function' : function not inlined
#pragma warning (push, 3)
#include <windows.h>
#include <windowsx.h>
#pragma warning (pop)
#include <dbt.h> // For WM_DEVICECHANGE
#include <commctrl.h>
#include <wcommctl.h>
#include "resource.h"

typedef int (WINAPI *PFN_PropertySheetW)(LPCPROPSHEETHEADERW);
typedef HPROPSHEETPAGE (WINAPI *PFN_CreatePropertySheetPageW)(LPCPROPSHEETPAGEW);

class ConnInfo
{
protected:
   TCHAR * pszName;
   HINSTANCE hCommCtrl;
   PFN_PropertySheetW pfnPropertySheet;
   PFN_CreatePropertySheetPageW pfnCreatePropertySheetPage;
   static const TCHAR szBadNameCharList[];

   ConnInfo();

   ////////////////////////////////////////
   // isValidConnInfoName
   //   params: [IN] TCHAR * pszValidate - string to validate
   //   return: BOOL - Does this string contain invalid characters
   //  descrip: Invalid characters are \/:*?<>|" (see szBadNameCharList[])
   ////////////////////////////////////////
   static BOOL isValidConnInfoName(TCHAR * pszValidate);

   inline BOOL methodNotImplemented(UINT * pError) const
   {
      if (pError)
         *pError = IDS_ERROR;
      return FALSE;
   }

public:

   static const DWORD MAX_DEFAULT_ACTION_STR_LEN;

   virtual ~ConnInfo();

   ////////////////////////////////////////
   // unsafeRename
   //   params: [IN] TCHAR * pszNewName - new name of this connection
   //   return: void
   //  descrip: Used to rename connectioids without notifing the underlying
   //           driver, can be unsafe because your ui can become out of sync
   //           with what it is representing  
   ////////////////////////////////////////
   inline void unsafeRename(TCHAR * pszNewName)
   {
      ASSERT(pszNewName);
      if (!pszNewName)
         return;

      if (pszName)
      {
         if (!_tcscmp(pszNewName, pszName))
            return;
         delete [] pszName;
      }

      pszName = new TCHAR[_tcslen(pszNewName)+1];
      if (pszName) _tcscpy(pszName, pszNewName);
   }

   ////////////////////////////////////////
   // unsafeRename
   //   params: [OUT] TCHAR * pszType - the exact name of this class
   //   return: BOOL - TRUE if this function succeeded, FALSE othewise
   //  descrip: Used for RTTI should return the exact name of the this class
   ////////////////////////////////////////
   virtual BOOL isType(TCHAR * pszType) const { return !::_tcscmp(pszType, _T("ConnInfo")); }

   ////////////////////////////////////////
   // virtual isValid const
   //   params: [OUT] UINT * pError - an error id to a resource string
   //   return: BOOL - TRUE if this function succeeded, FALSE othewise
   //  descrip: Tests the data in this object to make sure it matches with
   //           its underlying representation. pError can be NULL if no
   //           extended information is required, othewise on a return of
   //           FALSE pError point to a resource id which is a string
   //           containing extended error information.
   ////////////////////////////////////////
   virtual BOOL isValid(UINT * pError) const = 0;

   virtual UINT getIconID() const { return ((UINT) -1); }

   virtual BOOL allowDefaultAction(BOOL fInGroup, UINT * pReason);
   virtual BOOL getDefaultActionString(HINSTANCE hInstance, TCHAR * pszDefaultAction, UINT * pError) = 0;
   virtual BOOL defaultAction(HINSTANCE hInstance, HWND hParent, UINT * pError) = 0;

   virtual BOOL allowRemove(BOOL fInGroup, UINT * pReason) const { return methodNotImplemented(pReason); }
   virtual BOOL remove(UINT * pError) { return methodNotImplemented(pError); }

   virtual BOOL allowRename(BOOL fInGroup, UINT * pReason) const { return methodNotImplemented(pReason); }
   virtual BOOL rename(TCHAR * pszNewName, UINT * pError) { return methodNotImplemented(pError); }

   virtual BOOL allowCopy(BOOL fInGroup, UINT * pReason) const { return methodNotImplemented(pReason); }
   virtual BOOL copy(HINSTANCE hInstance, UINT * pError) { return methodNotImplemented(pError); }

   virtual BOOL allowShortcut(BOOL fInGroup, UINT * pReason) const { return methodNotImplemented(pReason); }
   virtual BOOL shortcut(HINSTANCE hInstance, UINT * pError) { return methodNotImplemented(pError); }

   virtual BOOL allowSetAsDefault(BOOL fInGroup, UINT * pReason) const { return methodNotImplemented(pReason); }
   virtual BOOL setAsDefault(BOOL fSet, UINT * pError) { return methodNotImplemented(pError); }
   virtual BOOL isSetAsDefault() { return FALSE; }

   ////////////////////////////////////////
   // virtual getDisplayInfo const
   //   params: [IN ] UINT col - the column to display
   //           [IN ] HINSTANCE hInstance - the HINSTANCE for this application
   //           [OUT] TCHAR * pszInfo - the information string
   //           [IN ] DWORD cchInfo - the length of pszInfo in TCHARs
   //           [OUT] UINT * pError - an error id to a resource string
   //   return: BOOL - TRUE if this function succeeded, FALSE othewise
   //  descrip: The columns are as follows:
   //              1: Friendly name of the connectiod
   //              2: Type of the connectiod
   //              3: Status of the connectoid
   //              4: Long name of the connectoid
   //           pError can be NULL if no extended information is required,
   //           othewise on a return of FALSE pError point to a resource id
   //           which is a string containing extended error information.
   ////////////////////////////////////////
   virtual BOOL getDisplayInfo(UINT col, HINSTANCE hInstance, TCHAR * pszInfo,
                               DWORD cchInfo, UINT * pError) const;

   virtual BOOL allowProperties(BOOL fInGroup, UINT * pReason) const { return methodNotImplemented(pReason); }
   virtual HPROPSHEETPAGE getPropPage(UINT uPage, HINSTANCE hInstance) { return NULL; }
   virtual BOOL showProperties(HINSTANCE hInstance, HWND hParent, UINT * pError);
};

#endif // CONN_INFO_H__
