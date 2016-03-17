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
#ifndef __PARTMAN_H__
#define __PARTMAN_H__

//
// implements a control panel interface to the Windows CE Storage Manager API
//

#include <Windows.H>
#include <TChar.H>
#include <CommDlg.H>
#include <CommCtrl.H>
#include <Resource.H>
#include <StoreMgr.H>
#include <msgqueue.h>
#include <pnp.h>

extern HINSTANCE g_hInstance;

// dialog help macros
//
#define DLG_ITEMISCHECKED(hDlg, idButton)   SendDlgItemMessage(hDlg, idButton, BM_GETCHECK, 0, 0)
#define DLG_GETCOMBOSEL(hDlg, idCombo)      SendDlgItemMessage(hDlg, idCombo, CB_GETCURSEL, 0, 0)
#define DLG_GETLISTSEL(hDlg, idList)        SendDlgItemMessage(hDlg, idList, LB_GETCURSEL, 0, 0)

#define DLG_ENABLEITEM(hDlg, idItem)        EnableWindow(GetDlgItem(hDlg, idItem), TRUE)
#define DLG_DISABLEITEM(hDlg, idItem)       EnableWindow(GetDlgItem(hDlg, idItem), FALSE)
#define DLG_CHECKITEM(hDlg, idButton)       SendDlgItemMessage(hDlg, idButton, BM_SETCHECK, BST_CHECKED, 0)

typedef struct _NEWPART_INFO 
{
    DWORD dwSize;
    TCHAR szName[MAX_PATH];
    
} NEWPART_INFO, *PNEWPART_INFO; 

// utility functions
//
VOID CenterWindow(HWND hWnd);
VOID ErrBox(HWND hWnd, UINT idString, DWORD dwErr);
BOOL Size2String(LONGLONG llBytes, LPTSTR pszSize, DWORD cLen);

// property sheet initalization callback
//
int CALLBACK PropSheetCallback(HWND hDlg, UINT uMsg, LPARAM lParam);

// dialog procs
//
BOOL CALLBACK NewPartDlgProc(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK PartInfoDlgProc(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK MainDlgProc(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam);

// function pointer types
//
typedef void (*PFN_FORMATVOLUMEUI)(HANDLE hVolume, HWND hDlg);
typedef void (*PFN_SCANVOLUMEUI)(HANDLE hVolume, HWND hDlg);
typedef void (*PFN_DEFRAGVOLUMEUI)(HANDLE hVolume, HWND hDlg);

#define	NAND_DEV_NAME	L"NFD"

#endif // __PARTMAN_H__
