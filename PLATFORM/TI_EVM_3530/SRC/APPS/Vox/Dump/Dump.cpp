// DigVoice.cpp : Defines the entry point for the application.
//

#include "resource.h"
#include <windows.h>
//#include <service.h>
#include <commctrl.h>
#include <commdlg.h>
#include "Mixer.h"

#define MAX_LOADSTRING 100

#define NUM_OF_EDIT_CHARS  32

//==============================================================================================
//     enumerations
//==============================================================================================

HINSTANCE       g_hInst;
HICON           g_hBTIcon = NULL;

HWND            g_hDlg;
TCHAR           myTest[32];
TCHAR           gEditLine[NUM_OF_EDIT_CHARS];



HANDLE g_hThr = NULL;

LRESULT CALLBACK mainDlgProc(HWND hDlg, UINT msg, WPARAM wp, LPARAM lp);


void FAR PASCAL lineCallbackFunc(DWORD hDevice,DWORD dwMsg, DWORD dwCallbackInstance,DWORD dwParam1,DWORD dwParam2, DWORD dwParam3)
{
 // NKDbgPrintfW(L"lineCallbackFunc( %d , %d , %d , %d , %d , %d
 // )\n",hDevice,dwMsg,dwCallbackInstance,dwParam1,dwParam2,dwParam3);
 return;
}



LRESULT CALLBACK mainDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lparam)
{
  
  switch(msg)
  {
    case WM_INITDIALOG:
	{
     //TCHAR         myTest[32]; 
	 //BOOL          bRet;
	 //BOOL          micMute;
	 //LONG          iRet;
	 //DWORD         ret;

	 g_hDlg = hDlg;



    // EnableWindow( GetDlgItem( hDlg, IDC_CHECK_BT_HS ), FALSE );  

      return TRUE; 
	} 


	case WM_COMMAND:
	{
      switch( LOWORD(wParam) )
	   {
		 case IDCANCEL:
            CloseHandle(g_hThr);
			EndDialog(hDlg, LOWORD(wParam));
			// Close Com port too ...
			//CloseHandle( g_hThr );
            //CloseHandle(g_hCom);

		 return TRUE;
     
		 case ID_DUMP_BUTTON: 
		  {
		   LONG  len;
		   MIXER_ERROR_CODES  err;

			  *(WORD *)gEditLine = NUM_OF_EDIT_CHARS - sizeof(TCHAR);
			  len = SendDlgItemMessage(hDlg, IDC_DUMP_FILE_EDIT, EM_GETLINE, 0 , (LPARAM)gEditLine);
			  err = dumpMixer(gEditLine);

           return TRUE;  
		  }


	   }

     return TRUE; 
	}

	case WM_USER:
	{
      return TRUE; 
	}

  } //switch(msg)

  return FALSE;
}


int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPTSTR    lpCmdLine,
                   int       nCmdShow)
{
	int  ret = -1;
	INITCOMMONCONTROLSEX slideBarControl;

    g_hInst = hInstance;

	ret = DialogBox(hInstance, MAKEINTRESOURCE(IDD_DUMP_DIALOG), 0, (DLGPROC)mainDlgProc);

	if( ret == -1 )
	{
     ret =	GetLastError();
	}

	return 0;
}








