#include "stdafx.h"
#include "resource.h"
#include <windows.h>



LRESULT CALLBACK  Test (HWND, UINT, WPARAM, LPARAM);
HWND hMyDlg;
HANDLE  hThread;
UINT uRet, uTime=10;
TCHAR szInfo[64];

ULONG WINAPI WD_ThreadProc(void *arg);
UINT uTextRez;
BOOL bRet;


int WINAPI WinMain(	HINSTANCE hInstance,
					HINSTANCE hPrevInstance,
					LPTSTR    lpCmdLine,
					int       nCmdShow)
{

 	// TODO: Place code here.	

	DialogBox( hInstance,(LPCTSTR)IDD_TEST,0,(DLGPROC)Test);
	return 0;
}


LRESULT CALLBACK Test(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	RECT rt, rt1;
	int DlgWidth, DlgHeight;	// dialog width and height in pixel units
	int NewPosX, NewPosY;
	hMyDlg=hDlg;
	DWORD dwExit=0;

	switch (message)
	{
		case WM_INITDIALOG:
			// trying to center the About dialog
			if (GetWindowRect(hDlg, &rt1))
			{
				GetClientRect(GetParent(hDlg), &rt);
				DlgWidth	= rt1.right - rt1.left;
				DlgHeight	= rt1.bottom - rt1.top ;
				NewPosX		= 0;//(rt.right - rt.left - DlgWidth)/2;
				NewPosY		= 0;//(rt.bottom - rt.top - DlgHeight)/2;
				
				// if the About box is larger than the physical screen 
				if (NewPosX < 0) NewPosX = 0;
				if (NewPosY < 0) NewPosY = 0;
				SetWindowPos(hDlg, 0, 80, 0,
					0, 0, SWP_NOZORDER | SWP_NOSIZE);

				SetTimer(hMyDlg,1,1000,NULL);

			}
			return TRUE;

		case WM_COMMAND:
			switch(LOWORD(wParam))
			{
						
			case IDCANCEL:
				KillTimer(hMyDlg,1);
				EndDialog(hDlg, LOWORD(wParam));
				return TRUE;
				break;
			}		    
		break;

		case WM_TIMER:
			 swprintf(szInfo,L"Reset after: %d sec",uTime);
			 SetDlgItemText(hMyDlg,IDC_INFO,szInfo);
			 if(!uTime)
			 {
				 KillTimer(hMyDlg,1);
				 hThread = CreateThread(0,0,&WD_ThreadProc,0,CREATE_SUSPENDED,0);
				 CeSetThreadPriority(hThread,50);
				 ResumeThread(hThread);
			 }
			 uTime-=1;
		break;

		case WM_GETDLGCODE:
		{
			MSG *msg = (MSG *)lParam;
			if(!msg)
				break;
			if(msg->message == WM_KEYDOWN)
			{
				switch(msg->wParam)
				{
					case VK_UP:
					break;
				}
			}

			break;
		}
		case WM_KEYDOWN:
			switch (wParam)
			{

			case VK_F1:
			
			 return TRUE;
			 break;

			}
             break;
	}
	return FALSE;
}

ULONG WINAPI WD_ThreadProc(void *arg)
{
	
	while(1)
	{
	  Sleep(1000);
      while(1);
	}

	return 0;
}