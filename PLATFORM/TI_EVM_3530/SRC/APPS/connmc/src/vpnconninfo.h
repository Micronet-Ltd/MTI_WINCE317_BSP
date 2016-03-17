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
#ifndef VPN_CONN_INFO_H__
#define VPN_CONN_INFO_H__

#include "rasconninfo.h"

#define PPTPDeviceNamePrefix TEXT("RAS VPN Line")
#define L2TPDeviceNamePrefix TEXT("L2TP Line")

class VpnConnInfo : public RasConnInfo
{
protected:
    BOOL fL2TP;
   static UINT iconID;

   static BOOL CALLBACK VpnConnPropSheetProc(HWND hDlg, UINT uMsg,
                                             WPARAM wParam, LPARAM lParam);
   static BOOL CALLBACK L2TPDlgProc(HWND hDlg, UINT uMsg,
                                                WPARAM wParam, LPARAM lParam);

public:
   VpnConnInfo(TCHAR * pszName = NULL, BOOL fL2TP = FALSE);

   virtual BOOL isType(TCHAR * pszType) const { return !::_tcscmp(pszType, _T("VpnConnInfo")); }

   static void setIconID(UINT iconID) { VpnConnInfo::iconID = iconID; }
   virtual UINT getIconID() const { return iconID; }

   virtual void getTypeAsDisplayString(HINSTANCE hInstance, TCHAR * pszType, DWORD cchType) const;

   virtual BOOL allowSetAsDefault(BOOL fInGroup, UINT * pReason) const { return methodNotImplemented(pReason); }
   virtual BOOL setAsDefault(BOOL fSet, UINT * pError) { return methodNotImplemented(pError); }
   virtual BOOL isSetAsDefault() { return FALSE; }

   static BOOL isValidVPNInfoString(WCHAR * wszValidate);

   // RasConnInfo.h
   virtual BOOL allowProperties(BOOL fInGroup, UINT * pReason) const { return TRUE; }
   virtual HPROPSHEETPAGE getPropPage(UINT uPage, HINSTANCE hInstance);

private:
    BOOL ConfigureL2TP(HWND hParent, HINSTANCE hInstance);

};

#endif // VPN_CONN_INFO_H__
