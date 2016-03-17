//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this source code is subject to the terms of the Microsoft end-user
// license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
// If you did not accept the terms of the EULA, you are not authorized to use
// this source code. For a copy of the EULA, please see the LICENSE.RTF on your
// install media.
//
/**
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Purpose:    Function prototypes for code in customised 
"Advanced..." button under Display Properties\Backlight tab
**/
//------------------------------------------------------------------------------
//
// Copyright (C) 2006, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT 
//
//------------------------------------------------------------------------------


#ifndef _APPS_ADVBACKLIGHT_ADVBACKLIGHT_H
#define _APPS_ADVBACKLIGHT_ADVBACKLIGHT_H

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <string.h>
#include <Winuser.h>
#include <prsht.h>
#include "..\bl.h"

#include "resource.h"

/*********************************************************************
 MACRO DEFINITIONS 
*********************************************************************/
//#define EVENTNAME_BACKLIGHTLEVELCHANGEEVENT     L"BackLightLevelChangeEvent"

/*********************************************************************
 ENUMERATIONS AND STRUCTURES 
*********************************************************************/
/*********************************************************************
 FUNCTION PROTOTYPES
*********************************************************************/

extern "C" BOOL APIENTRY AdvancedDlgProc (HWND hDlg, UINT message, UINT wParam, LONG lParam);
extern "C" BOOL CALLBACK AdvancedDlgProcLs(HWND hwndPage, UINT message, WPARAM wParam, LPARAM lParam);
extern "C" BOOL BacklightAdvApplet(HWND hDlg);
extern "C" BOOL DllEntry(HANDLE hInstance, DWORD fdwReason, LPVOID lpvReserved);


#define BKL_MIN_SETTING         BACKLIGHT_HIGH
#define BKL_MAX_SETTING         BACKLIGHT_LOW
#define BKL_NUM_TICKS           BACKLIGHT_LEVELS
#define BKL_TICK_INTERVAL       ((BKL_MAX_SETTING - BKL_MIN_SETTING) / BKL_NUM_TICKS)

//void InitSlider(HWND hDlg, DWORD dwBacklightLevel);
void	InitBklSettings(HWND hDlg, BOOL* pfNotify);
void	InitDlgSetting( HWND hDlg );
void	UpdateBklSettings( DWORD LastActive );
LONG	GetMultiSzFromReg( LPCTSTR lpszRegKey, LPCTSTR pName, DWORD* pParam );
void	SetMultiSzToReg(LPCTSTR dwState1, LPCTSTR lpszRegKey, DWORD* RegValues); 
void	UpdateLables(HWND hDlg);

/*********************************************************************
 EXTERN DECLARATIONS
*********************************************************************/
/*********************************************************************
 CLASS DEFINITIONS
*********************************************************************/
#endif  // _APPS_ADVBACKLIGHT_ADVBACKLIGHT_H

/*********************************************************************
 END OF FILE
*********************************************************************/
