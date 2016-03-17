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
#ifndef PPPOE_CONN_INFO_H__
#define PPPOE_CONN_INFO_H__

#include "rasconninfo.h"

class PPPOEConnInfo : public RasConnInfo
{
protected:
   static UINT iconID;

   static BOOL CALLBACK PPPOEConnPropSheetProc(HWND hDlg, UINT uMsg,
                                             WPARAM wParam, LPARAM lParam);

public:
   PPPOEConnInfo(TCHAR * pszName = NULL);

   virtual BOOL isType(TCHAR * pszType) const { return !::_tcscmp(pszType, _T("PPPOEConnInfo")); }

   static void setIconID(UINT iconID) { PPPOEConnInfo::iconID = iconID; }
   virtual UINT getIconID() const { return iconID; }

   virtual void getTypeAsDisplayString(HINSTANCE hInstance, TCHAR * pszType, DWORD cchType) const;

   virtual BOOL allowSetAsDefault(BOOL fInGroup, UINT * pReason) const { return methodNotImplemented(pReason); }
   virtual BOOL setAsDefault(BOOL fSet, UINT * pError) { return methodNotImplemented(pError); }
   virtual BOOL isSetAsDefault() { return FALSE; }

   // RasConnInfo.h
   virtual BOOL allowProperties(BOOL fInGroup, UINT * pReason) const { return TRUE; }
   virtual HPROPSHEETPAGE getPropPage(UINT uPage, HINSTANCE hInstance);
};

#endif // PPPOE_CONN_INFO_H__
