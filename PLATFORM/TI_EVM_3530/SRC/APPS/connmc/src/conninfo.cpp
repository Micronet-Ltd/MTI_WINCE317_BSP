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
#include "conninfo.h"

////////////////////////
// ConnInfo
////////////////////////

const DWORD ConnInfo::MAX_DEFAULT_ACTION_STR_LEN = 25;
const TCHAR ConnInfo::szBadNameCharList[] = { _T('\\'), _T('/'),
                                              _T('*'), _T('?'), _T('\"'),
                                              _T('<'), _T('>'), _T('|') };

ConnInfo::ConnInfo() :
   pszName(NULL)
{
   hCommCtrl = LoadLibrary(_T("commctrl.dll"));
   if (hCommCtrl)
   {
      pfnPropertySheet =
         (PFN_PropertySheetW)
            GetProcAddressW(hCommCtrl,
               _T("PropertySheetW"));

      pfnCreatePropertySheetPage =
         (PFN_CreatePropertySheetPageW)
            GetProcAddressW(hCommCtrl,
               _T("CreatePropertySheetPageW"));
   }
}

ConnInfo::~ConnInfo()
{
   if (pszName)
      delete [] pszName;

   if (hCommCtrl)
      FreeLibrary(hCommCtrl);
}

BOOL ConnInfo::isValidConnInfoName(TCHAR * pszValidate)
{
   if (!pszValidate)
      return FALSE;

   for (int i = 0; i < (sizeof(szBadNameCharList)/sizeof(*szBadNameCharList)); i++)
   {
      if (::_tcschr(pszValidate, szBadNameCharList[i]))
         return FALSE;
   }

   return TRUE;
}

BOOL ConnInfo::allowDefaultAction(BOOL fInGroup, UINT * pReason)
{
   if (fInGroup)
   {
      if (pReason)
         *pReason = IDS_ERROR;
      return FALSE;
   }

   return TRUE;
}


BOOL ConnInfo::getDisplayInfo(UINT col, HINSTANCE hInstance, TCHAR * pszInfo,
                              DWORD cchInfo, UINT * pError) const
{    
   if (pszInfo && pszName)
   {
      switch (col)
      {
         case 0: StringCchCopy(pszInfo, cchInfo, pszName); break;
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

BOOL ConnInfo::showProperties(HINSTANCE hInstance, HWND hParent, UINT * pError)
{
   if (!pfnPropertySheet)
   {
      if (pError) *pError = IDS_ERROR;
      return FALSE;
   }

   PROPSHEETHEADER psh = {0};
   HPROPSHEETPAGE phpsp[128] = {0};

   DWORD dwPropSheetCount = 0;
   while (dwPropSheetCount < 128)
   {
      phpsp[dwPropSheetCount] = getPropPage(dwPropSheetCount, hInstance);
      if (!phpsp[dwPropSheetCount])
         break;
      dwPropSheetCount++;
   }

   psh.dwSize = sizeof(psh);
   psh.dwFlags = PSH_PROPTITLE;
   psh.hwndParent = hParent;
   psh.hInstance = hInstance;
   psh.pszCaption = pszName;
   psh.nPages = dwPropSheetCount;
   psh.phpage = phpsp;

   (*pfnPropertySheet)(&psh);

   return TRUE;
}

