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
#ifndef DCC_CONN_INFO_H__
#define DCC_CONN_INFO_H__

#include "rasconninfo.h"

class DccConnInfo : public RasConnInfo
{
protected:
   static UINT iconID;
   static BOOL CALLBACK DccConnPropSheetProc(HWND hDlg, UINT uMsg,
                                             WPARAM wParam, LPARAM lParam);

public:
   DccConnInfo(TCHAR * pszName = NULL);

   virtual BOOL isType(TCHAR * pszType) const { return !::_tcscmp(pszType, _T("DccConnInfo")); }

   static void setIconID(UINT iconID) { DccConnInfo::iconID = iconID; }
   virtual UINT getIconID() const { return iconID; }

   virtual void getTypeAsDisplayString(HINSTANCE hInstance, TCHAR * pszType, DWORD cchType) const;

   // RasConnInfo.h
   virtual BOOL allowProperties(BOOL fInGroup, UINT * pReason) const { return TRUE; }
   virtual HPROPSHEETPAGE getPropPage(UINT uPage, HINSTANCE hInstance);
};

#endif // DCC_CONN_INFO_H__
