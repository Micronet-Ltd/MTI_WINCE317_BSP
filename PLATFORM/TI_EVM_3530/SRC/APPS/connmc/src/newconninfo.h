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
#ifndef NEW_CONN_INFO_H__
#define NEW_CONN_INFO_H__

#include "conninfo.h"
#include "resource.h"

class NewConnInfo : public ConnInfo
{
protected:
   static UINT iconID;
   static BOOL CALLBACK NewConnPropSheetProc(HWND hDlg, UINT uMsg,
                                             WPARAM wParam, LPARAM lParam);
   BOOL InitDialog(HWND hDlg, PROPSHEETPAGE & psp);
   LONG SetActive(HWND hDlg, PROPSHEETPAGE & psp);
   LONG WizNext(HWND hDlg, PROPSHEETPAGE & psp);
   BOOL DestroyDialog(HWND hDlg, PROPSHEETPAGE & psp);

public:
   NewConnInfo(TCHAR * pszName = NULL);

   virtual BOOL isType(TCHAR * pszType) const { return !::_tcscmp(pszType, _T("NewConnInfo")); }
   virtual BOOL isValid(UINT * pError) const { return TRUE; }

   static void setIconID(UINT iconID) { NewConnInfo::iconID = iconID; }
   virtual UINT getIconID() const { return iconID; }

   // ConnInfo.h
   virtual BOOL getDefaultActionString(HINSTANCE hInstance, TCHAR * pszDefaultAction, UINT * pError);
   virtual BOOL defaultAction(HINSTANCE hInstance, HWND hParent, UINT * pError);

   virtual BOOL allowRemove(BOOL fInGroup, UINT * pReason) const;

   virtual BOOL allowRename(BOOL fInGroup, UINT * pReason) const;

   virtual BOOL allowShortcut(BOOL fInGroup, UINT * pReason) const;

   virtual BOOL allowProperties(BOOL fInGroup, UINT * pReason) const;
   virtual HPROPSHEETPAGE getPropPage(UINT uPage, HINSTANCE hInstance);
};

#endif // NEW_CONN_INFO_H__
