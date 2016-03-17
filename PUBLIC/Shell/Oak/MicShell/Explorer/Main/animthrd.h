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
/*++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Module Name:  animthrd.h

Abstract:  handles the animation thread

Functions:

Notes: This class encapsulates the thread that runs the animation


--*/

#ifndef _ANIMTHRD_H_
#define _ANIMTHRD_H_


#define ANIMATIONINTERVAL	120


class CAnimThread
{
public:
	CAnimThread(HWND hwndParent);
	~CAnimThread();

public:
	VOID StartThread();
	VOID StartAnimation();
	VOID StopAnimation();
	void PositionWindow(RECT *prcParent);

	HANDLE		_hEvent;  // 
private:
	DWORD Run();
	VOID Paint();
	static DWORD WINAPI AnimationThreadProc(LPVOID lpParam);
	static LRESULT CALLBACK AnimationWndProc(HWND hwnd, UINT message,
												WPARAM wParam, LPARAM lParam);

private:
	DWORD		_dwThreadID;	//the thread id
        DWORD 		_dwTimerInterval; // animation interval
	LONG 		_nIndex;		//the index of the frame in the bitmap currently being shown
	HBITMAP		_hBitmap;		//handle to the animation bitmap
	HWND		_hwndParent;	//the parent of the animation window
	HWND		_hwndAnimation;	//the animation window
	HANDLE         _hThread; 
	LONG		_bRun;			//is the animation being run
	
	WNDPROC		_lpfnAnimationProc;


};

#endif	//_ANIMTHRD_H_
