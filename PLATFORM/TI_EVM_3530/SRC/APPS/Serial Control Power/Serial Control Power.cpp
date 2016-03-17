// Serial Control Power.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "Serial Control Power.h"
#include "math.h"
#include "Mmsystem.h"

#define MAX_LOADSTRING 100
#define BUTTONCLASS L"button"



// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name
HANDLE hStartTest, hTTiPortIsUp;
HANDLE ComPortsHandles[2];
TCHAR line[1024] = {0};
TCHAR printline[1024] = {0};
TCHAR fileline[128] = {0};

HWND g_hListBox, g_hEditInitDur, g_hEditStep, g_hEditNumLoops, g_hEditComTTi, g_hEditComFb, 
g_hEditFinalDur, g_hEditOffTime;

HBRUSH hColorBrush;
BOOL TestStarted;
DWORD Written;
MMTIME mmtime;
char multiByte[128];
SYSTEMTIME SystemTime;
FILETIME filename;
LoggerCell_t Logger[LOGGER_SIZE];
int LogWriteLoc;
int LogReadLoc;
int PrintSpecialState, TTiPortNum, FbPortNum;
RTL_CRITICAL_SECTION CriticalSection;

#define ADD_TIME_LABEL(x, XLINE) GetSystemTime(&SystemTime); \
								 wsprintf (XLINE, L"%2d:%2d:%2d.%3d",SystemTime.wHour+3, SystemTime.wMinute, \
											SystemTime.wSecond, SystemTime.wMilliseconds); \
								 lstrcat(XLINE, L" ; " ); \
								 lstrcat(XLINE, (LPCWSTR)x ); \

#define WRITE_TO_LISTBOX(x)		 ADD_TIME_LABEL(x, line) \
								 SendMessage(g_hListBox, LB_ADDSTRING, 0, (LPARAM)line);

#define WRITE_TO_LOGGER(x)		 ADD_TIME_LABEL(x, fileline) \
								 lstrcat(fileline, L"\r\n" ); \
								 WriteToLogger(fileline); 


// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

void InitLogger()
{
	for (int i = 0; i < LOGGER_SIZE ; i++)
	{
		Logger[i].visited = 0;
	}
	LogWriteLoc = 0;
	LogReadLoc = 0;
}

int WriteToLogger(LPWSTR str)
{
	DWORD len;
	if (Logger[LogWriteLoc].visited != 0)
	{
		WRITE_TO_LISTBOX(L"WRITE TO LOGGER FAILED. CELL NOT VISITED BY READ")
		return 0;
	}
	len = lstrlen(str);
	memcpy((void*) Logger[LogWriteLoc].LogLine,str, len * 2);

	Logger[LogWriteLoc].visited = 1;

	LogWriteLoc++; 
	if (LogWriteLoc == LOGGER_SIZE) LogWriteLoc = 0;

	return 1;
}

void LoggerReadAndWriteToFile()
{
	HANDLE LoggerFileHan = INVALID_HANDLE_VALUE;
	int ret;
	DWORD Written = 0;

	GetSystemTime(&SystemTime);
	SystemTimeToFileTime(&SystemTime, &filename);
	wsprintf(line, L"C:\\Temp\\Logger%d.txt", filename.dwLowDateTime);
	LoggerFileHan = CreateFile(line, GENERIC_READ|GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);

	PrintSpecialState = 0;

	while (1)
	{
		if (Logger[LogReadLoc].visited == 1)
		{
			Written = lstrlen((LPCWSTR)Logger[LogReadLoc].LogLine);
			ret = WideCharToMultiByte(CP_ACP,NULL,(LPCWSTR)Logger[LogReadLoc].LogLine,Written,multiByte,Written,NULL,NULL);
			WriteFile(LoggerFileHan,multiByte, Written, &Written, NULL);
			LogReadLoc++;
			if (LogReadLoc == LOGGER_SIZE) LogReadLoc = 0;
		}
		if (PrintSpecialState)
		{
			WRITE_TO_LISTBOX(L"WRITE ENABLE AFTER POWER OFF!!!");
			PrintSpecialState = 0;
		}
	}
}


int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_SERIALCONTROLPOWER, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_SERIALCONTROLPOWER));

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage are only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SERIALCONTROLPOWER));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_SERIALCONTROLPOWER);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // Store instance handle in our global variable

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   hColorBrush = 0;

   InitLogger();

   InitializeCriticalSection(&CriticalSection);


   hStartTest = CreateEvent(0,0,0,L"StartTestEventName");
   hTTiPortIsUp = CreateEvent(0,0,0,L"TTIEVENTDONE");

   return TRUE;
}

int InitComPortControl(int PortNum, HANDLE * retHandle)
{
	DCB           dcb;
	COMMTIMEOUTS  CommTimeOuts;

	DWORD OpenFileMask, fileAtt, DTRCont, RTSCont;
	TCHAR FileName[100];
	int Flow;

	if( PortNum > 9 ) {
		wsprintf( FileName, _T("\\\\.\\COM%d"), PortNum );
	}
	else {
		wsprintf(FileName, L"COM%d",PortNum);
	}

	if (PortNum == TTiPortNum)
	{
		OpenFileMask = GENERIC_READ | GENERIC_WRITE;
		fileAtt = FILE_ATTRIBUTE_NORMAL;
		Flow = 0;
		RTSCont = RTS_CONTROL_ENABLE;
		DTRCont = DTR_CONTROL_ENABLE;
	}
	else if (PortNum == FbPortNum)
	{
		OpenFileMask = GENERIC_READ | GENERIC_WRITE;
		fileAtt = FILE_ATTRIBUTE_NORMAL;
		Flow = 0;//1;
		RTSCont = RTS_CONTROL_ENABLE;//RTS_CONTROL_DISABLE;
		DTRCont = DTR_CONTROL_ENABLE;//DTR_CONTROL_DISABLE;

	}
	else
	{
		return FALSE;
	}
	if ( ( *retHandle = CreateFile ( FileName,    /* Open file name as "COM1"  */
		OpenFileMask,
		0, NULL, OPEN_EXISTING,  FILE_ATTRIBUTE_NORMAL,NULL))//FILE_ATTRIBUTE_NORMAL, NULL ) )FILE_FLAG_OVERLAPPED
		== INVALID_HANDLE_VALUE   ) {
			UINT32 res = GetLastError();
			wsprintf(printline,L"ERROR: failed to open COM%d. Error number = %d",PortNum, res);
			WRITE_TO_LISTBOX(printline);
		return FALSE;
	}

	UINT32 res = GetLastError();

	HANDLE Test = *retHandle;

	CommTimeOuts.ReadIntervalTimeout =  MAXDWORD;
	CommTimeOuts.ReadTotalTimeoutMultiplier = 0;
	CommTimeOuts.ReadTotalTimeoutConstant = 5000;
	CommTimeOuts.WriteTotalTimeoutMultiplier = 0;
	CommTimeOuts.WriteTotalTimeoutConstant = 500;
	if ( !SetCommTimeouts( *retHandle, &CommTimeOuts )){
		CloseHandle(*retHandle);
	}
	dcb.DCBlength = sizeof ( DCB ) ;
	dcb.BaudRate = 9600 ;
	dcb.fBinary = TRUE ;
	dcb.fParity = 0;
	dcb.fOutxCtsFlow = Flow;         // CTS output flow control
	dcb.fOutxDsrFlow = Flow;         // DSR output flow control
	dcb.fDtrControl =  DTRCont;         // DTR flow control type
	dcb.fDsrSensitivity = 0;      // DSR sensitivity
	dcb.fTXContinueOnXoff = 0;    // XOFF continues Tx
	dcb.fOutX = 0;                // XON/XOFF output flow control
	dcb.fInX = 0;                 // XON/XOFF input flow control
	dcb.fErrorChar = 0;           // enable error replacement
	dcb.fNull = 0;                // enable null stripping
	dcb.fRtsControl = RTSCont;          // RTS flow control
	dcb.fAbortOnError = 0;        // abort reads/writes on error
	dcb.XonLim = 200;            // transmit XON threshold
	dcb.XoffLim = 256;            // transmit XOFF threshold
	dcb.ByteSize = 8;             // number of bits/byte, 4-8
	dcb.Parity = NOPARITY;        // 0-4=no,odd,even,mark,space
	dcb.StopBits = ONESTOPBIT;    // 0,1,2 = 1, 1.5, 2
	dcb.XonChar = 17;             // Tx and Rx XON character
	dcb.XoffChar = 19;            // Tx and Rx XOFF character
	dcb.ErrorChar = 0;            // error replacement character
	dcb.EofChar = 0;              // end of input character
	dcb.EvtChar = 0;              // received event character
	if ( !SetCommState( *retHandle, &dcb ) ){
		CloseHandle(*retHandle);
	}

	return TRUE;
}

int ConvertToInt(TCHAR* line, DWORD length)
{
	int ret = 0, pos = 0;
	while (length)
	{
		int powerof =  (int)pow((double)10, (double)(length - 1));
		ret += (line[pos] - 48) * powerof;
		length--;
		pos++;
	}
	return ret;
}

// Thread continuously sending commands to power supply according to test settings.
//
void SendVoltageCommandsThread()
{
	int initDuration, stepDuration, finalDuration, NumLoops, infinite, offTime, currentDur, CurreLoops;
	DWORD length;
	HANDLE lComPort = INVALID_HANDLE_VALUE;
	
	length = (int)SendMessage(g_hEditComTTi, EM_GETLINE, 0, (LPARAM)line);
	TTiPortNum = ConvertToInt(line, length);

	if (!InitComPortControl(TTiPortNum, &lComPort)) return;
	ComPortsHandles[0] = lComPort;
	wsprintf(printline,L"COM%d Opened. TTi Power Supply Control.",TTiPortNum);
	WRITE_TO_LISTBOX(printline);

	SetEvent(hTTiPortIsUp);

	while (1)
	{
		WaitForSingleObject(hStartTest, INFINITE);

		WriteFile(lComPort, "V1 12\n", 6, &length, NULL);
		WriteFile(lComPort, "OP1 1\n", 6, &length, NULL);

		length = (int)SendMessage(g_hEditInitDur, EM_GETLINE, 0, (LPARAM)line);
		initDuration = ConvertToInt(line, length);
		currentDur = initDuration;

		length = (int)SendMessage(g_hEditStep, EM_GETLINE, 0, (LPARAM)line);
		stepDuration = ConvertToInt(line, length);

		length = (int)SendMessage(g_hEditFinalDur, EM_GETLINE, 0, (LPARAM)line);
		finalDuration = ConvertToInt(line, length);

		length = (int)SendMessage(g_hEditOffTime, EM_GETLINE, 0, (LPARAM)line);
		offTime = ConvertToInt(line, length);

		length = (int)SendMessage(g_hEditNumLoops, EM_GETLINE, 0, (LPARAM)line);
		NumLoops = ConvertToInt(line, length);
		CurreLoops = NumLoops;
		if (NumLoops == 0){infinite = 1;NumLoops = 1;} else {infinite = 0;}

		while ((NumLoops) && (TestStarted))
		{
			while ((currentDur <= finalDuration) && (TestStarted))
			{
				WriteFile(lComPort, "OP1 1\n", 6, &length, NULL);
				Sleep(currentDur);
				if (!TestStarted) break;
				WriteFile(lComPort, "OP1 0\n", 6, &length, NULL);
				Sleep(offTime);
				if (!TestStarted) break;
				currentDur += stepDuration;
			}

			currentDur = initDuration;

			if (!infinite) --NumLoops;
		}

		wsprintf(printline,L"Test Completed %d Loops",CurreLoops);
		WRITE_TO_LISTBOX(printline);

	}

}

//Thread function monitoring state of CTS in COM port
void ComMonitoringThread()
{
	static HANDLE lComPort = INVALID_HANDLE_VALUE;
	int powerup = -1, wrEnabled = -1, length;

	WaitForSingleObject(hTTiPortIsUp,INFINITE);

	length = (int)SendMessage(g_hEditComFb, EM_GETLINE, 0, (LPARAM)line);
	FbPortNum = ConvertToInt(line, length);

	if (!InitComPortControl (FbPortNum, &lComPort)) return;

	ComPortsHandles[1] = lComPort;
	wsprintf(printline,L"COM%d Opened. Feedback Port. DCD used for Power. CTS used for Write Enable.",FbPortNum);
	WRITE_TO_LISTBOX(printline)

	SetCommMask(lComPort, EV_RXCHAR | EV_CTS | EV_DSR | EV_RLSD | EV_RING);

	while (1)
	{
		while (TestStarted)
		{
			DWORD Mask = 0;
			DWORD State = 0;
			if (WaitCommEvent(lComPort, &Mask, NULL))
			{
				SetCommMask(lComPort, EV_RXCHAR | EV_CTS | EV_DSR | EV_RLSD | EV_RING);

				EnterCriticalSection(&CriticalSection);
				
				GetCommModemStatus(lComPort,&State);

				if (Mask & EV_CTS) {
					if (MS_CTS_ON & State)  { WRITE_TO_LOGGER(L"_________________POWER_DOWN_"); powerup = 0;}
					else                    { WRITE_TO_LOGGER(L"^^^^^^^^^^^^^^^^^POWER^UP^^^"); powerup = 1;}
				}
				if (Mask & EV_DSR) {
					if (MS_DSR_ON & State)  { WRITE_TO_LOGGER(L"^DSR^UP^^^^^^^^^^^"); }
					else                    { WRITE_TO_LOGGER(L"_DSR_DOWN_________"); }
				}
				if (Mask & EV_RLSD) {
					if (MS_RLSD_ON & State) { WRITE_TO_LOGGER(L"^WR_PROTCETED^^"); wrEnabled = 0;}
					else                    { WRITE_TO_LOGGER(L"_WR_ENABLED____"); wrEnabled = 1;}
				}
				if (Mask & EV_RING) {
					if (MS_RING_ON & State) { WRITE_TO_LOGGER(L"^RING^UP^^"); }
					else                    { WRITE_TO_LOGGER(L"_RING_DOWN"); }
				}

				if ((wrEnabled == 1) && (powerup == 0))
				{
					PrintSpecialState = 1;
				}
				LeaveCriticalSection(&CriticalSection);
			}
			else
			{
				DWORD dwRet = GetLastError();
				if( ERROR_IO_PENDING == dwRet)
				{
					WRITE_TO_LISTBOX(L"I/O is pending...");

					// To do.
				}
				else 
					WRITE_TO_LISTBOX(L"Wait failed with error ");
			}

/*
			DWORD Err;
			COMSTAT cs;
			ClearCommError(lComPort, &Err, &cs);


			DWORD ms;
			GetCommModemStatus(lComPort, &ms);

			int i = 0;
			i++;
*/
		}
	}
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	HWND hStaticInit, hStaticStep,  hStaticFinal, hStaticNumLoops, hStaticComTTi, hStaticComFb, hStaticOffTime;
	// Create a push button.
	static HWND hwndBStartTest, hwndBEndTest;
	static HANDLE ThHanTTiControl = INVALID_HANDLE_VALUE, ThHanComFbMon = INVALID_HANDLE_VALUE,
		hLoggerFile = INVALID_HANDLE_VALUE;

	switch (message)
	{
	case WM_COMMAND:
		/* Start test button pushed */
		if(wParam ==  MAKELONG(ID_BUTTON_START_TEST, BN_CLICKED)){

			if (ThHanTTiControl == INVALID_HANDLE_VALUE)
			{
				ThHanTTiControl = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)SendVoltageCommandsThread,NULL,0,0);
			}

			if (ThHanComFbMon == INVALID_HANDLE_VALUE)
			{
				ThHanComFbMon = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)ComMonitoringThread,NULL,0,0);
				SetThreadPriority(ThHanComFbMon,THREAD_PRIORITY_TIME_CRITICAL);
			}
			TestStarted = 1;
			SetEvent(hStartTest);
			wsprintf(printline,L"Test Started");
			WRITE_TO_LISTBOX(printline);
		}
		/* End test button pushed */
		if(wParam ==  MAKELONG(ID_BUTTON_START_END, BN_CLICKED)){
			TestStarted = 0;
			ResetEvent(hStartTest);
			wsprintf(printline,L"Test Stopped");
			WRITE_TO_LISTBOX(printline);
		}

		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		case ID_FILE_SAVELOG:
			{
				int ret, i, numItems = 0;
				HANDLE ResultsFileHan = INVALID_HANDLE_VALUE;

				GetSystemTime(&SystemTime);
				SystemTimeToFileTime(&SystemTime, &filename);
				wsprintf(line, L"C:\\Temp\\PSLog%d.txt", filename.dwLowDateTime);
				ResultsFileHan = CreateFile(line, GENERIC_READ|GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
				
				numItems = (int)SendMessage(g_hListBox, LB_GETCOUNT, NULL, NULL);
				if (LB_ERR == numItems)
				{
					WRITE_TO_LISTBOX(L"Illegal number of items in List Box")
					break;
				}
				
				for (i = 0 ; i < numItems ; i++)
				{
					SendMessage(g_hListBox, LB_GETTEXT, i, (LPARAM) line);
					lstrcat(line, L"\r\n\0" );
					Written = lstrlen(line);
					ret = WideCharToMultiByte(CP_ACP,NULL,line,Written,multiByte,Written,NULL,NULL);
					WriteFile(ResultsFileHan,multiByte, Written, &Written, NULL);
				}
				
				if (ResultsFileHan != INVALID_HANDLE_VALUE)
				{
					CloseHandle(ResultsFileHan);
				}
			}
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code here...
		EndPaint(hWnd, &ps);
		break;
	case WM_CREATE:
		{
			RECT r;
			GetClientRect(hWnd, &r);
			r.top +=  GetSystemMetrics(SM_CYMENU) + GetSystemMetrics(SM_CYBORDER); 

			hStaticInit = CreateWindowEx(0, L"static", L"Init (mSec)", (LBS_STANDARD & ~LBS_SORT) |
				WS_VISIBLE | WS_CHILD | SS_LEFT, //SS_WHITEFRAME
				r.left + 200, r.top + 10, 100, 
				20, hWnd,(HMENU)0, hInst, 0);

			ShowScrollBar(hStaticInit,SB_BOTH,0);

			g_hEditInitDur = CreateWindowEx(WS_EX_CLIENTEDGE,L"edit", L"10000", (LBS_STANDARD & ~LBS_SORT) |
				WS_VISIBLE | WS_CHILD | ES_NUMBER,//CBS_DROPDOWN,
				r.left + 200, r.top + 40, 100, 
				20, hWnd,(HMENU)0, hInst, 0);

			ShowScrollBar(g_hEditInitDur,SB_BOTH,0);

			hStaticStep = CreateWindowEx(0, L"static", L"Step (mSec)", (LBS_STANDARD & ~LBS_SORT) |
				WS_VISIBLE | WS_CHILD | SS_LEFT, //SS_WHITEFRAME
				r.left + 350, r.top + 10, 100, 
				20, hWnd,(HMENU)0, hInst, 0);

			ShowScrollBar(hStaticStep,SB_BOTH,0);

			g_hEditStep = CreateWindowEx(WS_EX_CLIENTEDGE,L"edit", L"100", (LBS_STANDARD & ~LBS_SORT) |
				WS_VISIBLE | WS_CHILD | ES_NUMBER,//CBS_DROPDOWN,
				r.left + 350, r.top + 40, 100, 
				20, hWnd,(HMENU)0, hInst, 0);

			ShowScrollBar(g_hEditStep,SB_BOTH,0);

			hStaticFinal = CreateWindowEx(0, L"static", L"Final (mSec)", (LBS_STANDARD & ~LBS_SORT) |
				WS_VISIBLE | WS_CHILD | SS_LEFT, //SS_WHITEFRAME
				r.left + 500, r.top + 10, 100, 
				20, hWnd,(HMENU)0, hInst, 0);

			ShowScrollBar(hStaticFinal,SB_BOTH,0);

			g_hEditFinalDur = CreateWindowEx(WS_EX_CLIENTEDGE,L"edit", L"15000", (LBS_STANDARD & ~LBS_SORT) |
				WS_VISIBLE | WS_CHILD | ES_NUMBER,//CBS_DROPDOWN,
				r.left + 500, r.top + 40, 100, 
				20, hWnd,(HMENU)0, hInst, 0);

			ShowScrollBar(g_hEditFinalDur,SB_BOTH,0);

			hStaticNumLoops = CreateWindowEx(0, L"static", L"Num loops (0 infinite)", (LBS_STANDARD & ~LBS_SORT) |
				WS_VISIBLE | WS_CHILD | SS_LEFT, //SS_WHITEFRAME
				r.left + 650, r.top + 10, 180, 
				20, hWnd,(HMENU)0, hInst, 0);

			ShowScrollBar(hStaticNumLoops,SB_BOTH,0);

			g_hEditNumLoops = CreateWindowEx(WS_EX_CLIENTEDGE,L"edit", L"0", (LBS_STANDARD & ~LBS_SORT) |
				WS_VISIBLE | WS_CHILD | ES_NUMBER,//CBS_DROPDOWN,
				r.left + 650, r.top + 40, 100, 
				20, hWnd,(HMENU)0, hInst, 0);

			ShowScrollBar(g_hEditNumLoops,SB_BOTH,0);

			hStaticOffTime = CreateWindowEx(0, L"static", L"OFF (mSec)", (LBS_STANDARD & ~LBS_SORT) |
				WS_VISIBLE | WS_CHILD | SS_LEFT, //SS_WHITEFRAME
				r.left + 850, r.top + 10, 100, 
				20, hWnd,(HMENU)0, hInst, 0);

			ShowScrollBar(hStaticOffTime,SB_BOTH,0);

			g_hEditOffTime = CreateWindowEx(WS_EX_CLIENTEDGE,L"edit", L"3000", (LBS_STANDARD & ~LBS_SORT) |
				WS_VISIBLE | WS_CHILD | ES_NUMBER,//CBS_DROPDOWN,
				r.left + 850, r.top + 40, 100, 
				20, hWnd,(HMENU)0, hInst, 0);

			ShowScrollBar(g_hEditOffTime,SB_BOTH,0);

			hwndBStartTest = CreateWindow( BUTTONCLASS, L"Start Test", 
				WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 
				r.left + 20, r.top + 10 , 100, 
				20, hWnd, 
				(HMENU) ID_BUTTON_START_TEST, // 0x8001
				hInst, NULL );

			hwndBEndTest = CreateWindow( BUTTONCLASS, L"End Test", 
				WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 
				r.left + 20, r.top + 40 , 100, 
				20, hWnd, 
				(HMENU) ID_BUTTON_START_END, // 0x8002
				hInst, NULL );

			hStaticComTTi = CreateWindowEx(0, L"static", L"TTi Port", (LBS_STANDARD & ~LBS_SORT) |
				WS_VISIBLE | WS_CHILD | SS_LEFT, //SS_WHITEFRAME
				r.left + 20, r.top + 80 , 60, 
				20, hWnd,(HMENU)0, hInst, 0);

			ShowScrollBar(hStaticComTTi,SB_BOTH,0);

			g_hEditComTTi = CreateWindowEx(WS_EX_CLIENTEDGE,L"edit", L"4", (LBS_STANDARD & ~LBS_SORT) |
				WS_VISIBLE | WS_CHILD | ES_NUMBER,//CBS_DROPDOWN,
				r.left + 80, r.top + 80 , 40, 
				20, hWnd,(HMENU)0, hInst, 0);

			ShowScrollBar(g_hEditComTTi,SB_BOTH,0);

			hStaticComFb = CreateWindowEx(0, L"static", L"FB Port", (LBS_STANDARD & ~LBS_SORT) |
				WS_VISIBLE | WS_CHILD | SS_LEFT, //SS_WHITEFRAME
				r.left + 20, r.top + 110 , 60, 
				20, hWnd,(HMENU)0, hInst, 0);

			ShowScrollBar(hStaticComFb,SB_BOTH,0);

			g_hEditComFb = CreateWindowEx(WS_EX_CLIENTEDGE,L"edit", L"3", (LBS_STANDARD & ~LBS_SORT) |
				WS_VISIBLE | WS_CHILD | ES_NUMBER,//CBS_DROPDOWN,
				r.left + 80, r.top + 110 , 40, 
				20, hWnd,(HMENU)0, hInst, 0);

			ShowScrollBar(g_hEditComFb,SB_BOTH,0);

			g_hListBox = CreateWindow(L"Listbox", 0, (LBS_STANDARD & ~LBS_SORT) |
				WS_CHILD | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL | WS_TABSTOP,
				r.left, r.top + 160, r.right - r.left, r.bottom - r.top - 100,
				hWnd,(HMENU)0, hInst, 0);
			

			if (hLoggerFile == INVALID_HANDLE_VALUE)
			{
				hLoggerFile = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)LoggerReadAndWriteToFile,NULL,0,0);
			}

			for (int i = 0; i < 2; i++ ){ComPortsHandles[i] = INVALID_HANDLE_VALUE;}
		}
		break;

	case WM_CTLCOLORBTN: //doesn't work. Requires owner maybe
		{
			if(!hColorBrush) hColorBrush = CreateSolidBrush(RGB(0, 0, 255));  
			return (LRESULT)hColorBrush;  
		}
		break;

	case WM_DESTROY:

		if (ThHanTTiControl != INVALID_HANDLE_VALUE)	{
			CloseHandle (ThHanTTiControl);
		}
		if (ThHanComFbMon != INVALID_HANDLE_VALUE)	{
			CloseHandle (ThHanComFbMon);
		}
		if (!hColorBrush) {
			DeleteObject(hColorBrush);
		}
		if (hStartTest != INVALID_HANDLE_VALUE) {
			CloseHandle(hStartTest);
		}
		if (hLoggerFile != INVALID_HANDLE_VALUE) {
			CloseHandle(hLoggerFile);
		}
		if (hTTiPortIsUp != INVALID_HANDLE_VALUE) {
			CloseHandle(hTTiPortIsUp);
		}
		for (int i = 0; i < 2; i++ ){
			if (ComPortsHandles[i] == INVALID_HANDLE_VALUE) {
				CloseHandle(ComPortsHandles[i]);
			}
		}

		DeleteCriticalSection(&CriticalSection);

		if(g_hListBox != INVALID_HANDLE_VALUE)
			DestroyWindow(g_hListBox);

		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}
