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

Module Name:  animthrd.cpp

Abstract:  handles the animation thread

Functions:

Notes: This class encapsulates the thread that runs the animation


--*/
#include "windows.h"
#include "animthrd.h"
#include "resource.h"

#define MAXFRAMES	25		//the number of frames in the bitmap

extern HINSTANCE g_hInstance;


#define MAXLEN MAX_URL
#define ANIMATIONWIDTH		22
#define ANIMATIONHEIGHT		22


CAnimThread::CAnimThread(HWND hwndParent)
{
	_nIndex = 0;
	_hBitmap = LoadBitmap(g_hInstance, MAKEINTRESOURCE(IDB_ANIMATION));
	_bRun = FALSE;
    	_hwndParent = hwndParent;
			
        HKEY hk = NULL;
        DWORD dwSize = sizeof(DWORD);
        if(ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER,L"Software\\Microsoft\\Internet Explorer\\Main",0,0,&hk))
        {
            if(ERROR_SUCCESS != RegQueryValueEx(hk, L"AnimationTicks",0,NULL, (LPBYTE)&_dwTimerInterval, &dwSize))
            {               
                _dwTimerInterval = ANIMATIONINTERVAL;
            }   
            RegCloseKey(hk);
        }

       _hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
       _hThread = NULL;
}

CAnimThread::~CAnimThread()
{
	DeleteObject(_hBitmap);
	if(_hEvent)
            CloseHandle(_hEvent);

	if(_hwndAnimation)
	{
            
            
            PostMessage(_hwndAnimation, WM_CLOSE, 0, 0);

            // don't leave the destructor until we're sure the thread (and window) is gone
            if(_hThread)
            {
                WaitForSingleObject(_hThread, 5000);
                CloseHandle(_hThread);
            }
       }

}

VOID CAnimThread::StartThread()
{
	_hThread = CreateThread(NULL, 0, AnimationThreadProc, this, 0, &_dwThreadID);
}


DWORD WINAPI CAnimThread::AnimationThreadProc(LPVOID lpParam)
{
	CAnimThread *pAnimation = (CAnimThread *)lpParam;
	return pAnimation->Run();
}

DWORD CAnimThread::Run()
{
	_hwndAnimation = CreateWindowEx(0, _T("STATIC"), NULL, WS_CHILD|WS_DISABLED,
					0, 0, ANIMATIONWIDTH, ANIMATIONHEIGHT, _hwndParent, NULL,
					g_hInstance, NULL);

       SetEvent(_hEvent);
	if (!_hwndAnimation)
		return FALSE;

	SetWindowLong(_hwndAnimation, GWL_USERDATA, (DWORD)this);
	
	_lpfnAnimationProc = (WNDPROC )SetWindowLong(_hwndAnimation, GWL_WNDPROC, (LONG )AnimationWndProc);

	SetTimer(_hwndAnimation, ID_ANIMATIONTIMER, _dwTimerInterval, NULL);

	MSG msg;
	while(GetMessage(&msg, _hwndAnimation, 0, 0))
	{
	       if(msg.message == WM_QUIT)
                    break;
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return msg.wParam;
}

void CAnimThread::PositionWindow(RECT *prcParent)
{
                  
	SetWindowPos(_hwndAnimation, NULL, prcParent->right-ANIMATIONWIDTH-20, prcParent->top, 0,0, SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);
}

LRESULT CALLBACK CAnimThread::AnimationWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    CAnimThread *pthrdAnimation = (CAnimThread*)GetWindowLong(hwnd, GWL_USERDATA);
			
	switch(message)
	{

	case WM_TIMER:
	       if(pthrdAnimation)
		    pthrdAnimation->Paint();
		return 0L;
	case WM_CLOSE:
		break;
	case WM_DESTROY:
	       SetWindowLong(hwnd, GWL_USERDATA, (DWORD)0);
		break;
	}	

	if(pthrdAnimation)
	    return CallWindowProc(pthrdAnimation->_lpfnAnimationProc, hwnd, message, wParam, lParam);
	else
	    return 0;

}


VOID CAnimThread::Paint()
{
	if (_bRun)
	{
        	HDC hDC = GetDC(_hwndAnimation);
        	HDC hMemDC = CreateCompatibleDC(hDC);
        	HBITMAP hBmpOld = (HBITMAP )SelectObject(hMemDC, _hBitmap);
        	BitBlt(hDC, 0, 0, ANIMATIONWIDTH, ANIMATIONHEIGHT,
        					hMemDC, _nIndex*ANIMATIONWIDTH, 0, SRCCOPY);
        	SelectObject(hMemDC, hBmpOld);
        	DeleteDC(hMemDC);
        	ReleaseDC(_hwndAnimation, hDC);

		InterlockedIncrement(&_nIndex);
		if(_nIndex == MAXFRAMES)
		{
			LONG lVal = 0;
			InterlockedExchange(&_nIndex, lVal);
		}
	}
}

//StartAnimation and StopAnimation are called from the main thread, to control the animation
VOID CAnimThread::StartAnimation()
{
	LONG dwVal = TRUE;
	SetWindowPos(_hwndAnimation, HWND_TOP, 0,0,0,0, SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE|SWP_SHOWWINDOW);
	InterlockedExchange(&_bRun, dwVal);
}

VOID CAnimThread::StopAnimation()
{
	LONG lVal = 0;
	InterlockedExchange(&_bRun, lVal);
	InterlockedExchange(&_nIndex, lVal);
	SetWindowPos(_hwndAnimation, NULL, 0,0,0,0, SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE|SWP_HIDEWINDOW);
	
}


