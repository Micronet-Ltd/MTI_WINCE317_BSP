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
#ifndef RNA_CONN_INFO_H__
#define RNA_CONN_INFO_H__

#include "rasconninfo.h"

#define MAX_FORMAT_LEN 128
class CFormattedEdit
{
	TCHAR	m_Format[MAX_FORMAT_LEN+1];

	HWND	m_Window;

	WNDPROC m_OriginalWndProc;

public:

	CFormattedEdit();
	~CFormattedEdit();

	HRESULT Attach(TCHAR *pFormat, HWND Window);
	HRESULT Attach(TCHAR *pFormat, HWND ParentWindow, DWORD ControlID);
	void	Detach();

	static LRESULT CALLBACK SubclassWndProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam);
};



class RnaConnInfo : public RasConnInfo
{
protected:
	static CFormattedEdit FormattedEdit;
	static UINT iconID;
	static const UINT CNTRYRGN_CODE_SIZE;

	static BOOL CALLBACK RnaConnPropSheetProc0(HWND hDlg, UINT uMsg,
		WPARAM wParam, LPARAM lParam);
	static BOOL CALLBACK RnaConnPropSheetProc1(HWND hDlg, UINT uMsg,
		WPARAM wParam, LPARAM lParam);

public:
	RnaConnInfo(TCHAR * pszName = NULL);

	virtual BOOL isType(TCHAR * pszType) const { return !::_tcscmp(pszType, _T("RnaConnInfo")); }

	static void setIconID(UINT iconID) { RnaConnInfo::iconID = iconID; }
	virtual UINT getIconID() const { return iconID; }

	virtual void getTypeAsDisplayString(HINSTANCE hInstance, TCHAR * pszType, DWORD cchType) const;

	// RasConnInfo.h
	virtual BOOL allowProperties(BOOL fInGroup, UINT * pReason) const { return TRUE; }
	virtual HPROPSHEETPAGE getPropPage(UINT uPage, HINSTANCE hInstance);
};

#endif // RNA_CONN_INFO_H__
