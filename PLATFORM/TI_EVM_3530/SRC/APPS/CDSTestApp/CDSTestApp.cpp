
#include "stdafx.h"
#include "CDSTestApp.h"
#include <windows.h>
#include <commctrl.h>
#include <devload.h>
#include <cdsInterface.h>
#include "winioctl.h"
#include "MicUserSdk.h"
#include "oal.h"
#include "oalex.h"
#include "args.h"
#include "ceddkex.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE			g_hInst;			// current instance
#ifdef SHELL_AYGSHELL
HWND				g_hWndMenuBar;		// menu bar handle
#else // SHELL_AYGSHELL
HWND				g_hWndCommandBar;	// command bar handle
#endif // SHELL_AYGSHELL
HWND				g_hList;

// Forward declarations of functions included in this code module:
ATOM			MyRegisterClass(HINSTANCE, LPTSTR);
BOOL			InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
#ifndef WIN32_PLATFORM_WFSP
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
#endif // !WIN32_PLATFORM_WFSP

#define BUTTONCLASS L"button"


UINT32 MIC_TemperatureSensorGet(HANDLE hTempSensor, TEMPSENSOR_DATA *data); // Micronet internal
UINT32 MIC_TemperatureSensorForceEnqueue(HANDLE hTempSensor);				// functions

int freq = 0;
uint LoadTestInProgress = 0;
int CDSTestInprogerss = 1;
HANDLE ResultsFileHan = INVALID_HANDLE_VALUE;
HANDLE hCtrlCds = INVALID_HANDLE_VALUE;
HANDLE hTempSensor = INVALID_HANDLE_VALUE;
TCHAR *queueName = NULL;
UINT32 queueNameLen = 0;
TEMPSENSOR_DATA CcTempSensMsg;
HANDLE CchMsgQueue = INVALID_HANDLE_VALUE;

TCHAR line[1024] = {0};
char multiByte[128];
INT16 DeviceTempResult[2];

BOOL GetThermalShudownState()
{
	BOOL bPresent = FALSE;
	card_ver cv = {0};
	hw_boot_status_ex HwBootStatusEx;

	HANDLE hProxy = CreateFile(L"PXY1:", GENERIC_WRITE | GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (hProxy != INVALID_HANDLE_VALUE)
	{

		DWORD retsize;
		if (DeviceIoControl(hProxy, IOCTL_HAL_GET_HW_BOOT_STAT_EX, 0, 0, &HwBootStatusEx, sizeof(hw_boot_status_ex), 
							&retsize, 0))
		{
			if (cv.ver != -1)
				bPresent = TRUE;
		}

		CloseHandle(hProxy);
	}

	wsprintf(line, L"Proxy Reading of Thermal Shutdown [%x] Boot Status [0x%x]", 
		HwBootStatusEx.ThermalShutdownStatus, HwBootStatusEx.HwBootStatus.bits);
	SendMessage(g_hList, LB_ADDSTRING, 0, (LPARAM)line);

	CloseHandle(hProxy);

	return bPresent;
}

void WtiteSpeedIntoTestResultFile(int opm)
{
	int ret, iRes, ResultBuffer = 0;
	DWORD Written, NumWordsRet = 0;
	int OpmToFreq[3] = {600,800,1000};	
	int InputOpm = opm;//Initial frequency

	if (hCtrlCds != INVALID_HANDLE_VALUE)
	{
		if (InputOpm == 0)// if value is "0", get current speed from device.
		{
			iRes = ((DeviceIoControl (hCtrlCds, IOCTL_CPUDYNAMICSPEED_GET_SPEED, NULL, NULL, 
				&ResultBuffer, sizeof(ResultBuffer), &NumWordsRet,
				NULL)) ? ERROR_SUCCESS : GetLastError());
		}
		else
		{
			iRes = ((DeviceIoControl (hCtrlCds, IOCTL_CPUDYNAMICSPEED_SET_SPEED, &InputOpm, sizeof(InputOpm), 
				&ResultBuffer, sizeof(ResultBuffer), &NumWordsRet,
				NULL)) ? ERROR_SUCCESS : GetLastError());

			if ((ResultBuffer < OPM_600MHz) || (ResultBuffer > OPM_1GHz)) {
				wsprintf(line, L"ERROR: IOCTL Set Speed Command: IOCTl retruned bad opm value [%d]", ResultBuffer);
				SendMessage(g_hList, LB_ADDSTRING, 0, (LPARAM)line);
				return;
			}

		}

		freq = OpmToFreq[ResultBuffer - OPM_600MHz];

		wsprintf(line, L"Device Current Frequency [%dMHz]", freq);
		SendMessage(g_hList, LB_ADDSTRING, 0, (LPARAM)line);						

		if (ResultsFileHan != INVALID_HANDLE_VALUE) {
			wsprintf(line, L"Current CPU Speed %4dMHz\r\n", freq);
			ret = WideCharToMultiByte(CP_ACP,NULL,line,27,multiByte,128,NULL,NULL);
			WriteFile(ResultsFileHan,multiByte, ret, &Written, NULL);
		}
	}
}

void StartCompanionChipTempRead()
{
	UINT32 res = 0;
	MSGQUEUEOPTIONS msgOptions	= {0};

	hTempSensor = MIC_TemperatureSensorOpen();
	if (hTempSensor == INVALID_HANDLE_VALUE)
	{
		wsprintf(line, L"Can't open temperature sensor device. Error = %d", GetLastError());
		SendMessage(g_hList, LB_ADDSTRING, 0, (LPARAM)line);

		goto clean_up;
	}

	res = MIC_TemperatureSensorRegister(hTempSensor, L"", &queueNameLen);
	if (res == TEMPSENSOR_ERROR_BUFFER_OVERFLOW)
	{
		queueName = (TCHAR *)LocalAlloc(LPTR, queueNameLen);
		if (!queueName)
		{
			wsprintf(line, L"Can't allocate memory for queue. Error = %d", GetLastError());
			SendMessage(g_hList, LB_ADDSTRING, 0, (LPARAM)line);
			goto clean_up;
		}

		res = MIC_TemperatureSensorRegister(hTempSensor, queueName, &queueNameLen);
	}

	if (res != TEMPSENSOR_OK)
	{
		wsprintf(line, L"Can't register temperature sensor notification queue. Error = %d", res);
		SendMessage(g_hList, LB_ADDSTRING, 0, (LPARAM)line);
		goto clean_up;
	}

	msgOptions.dwSize		= sizeof(MSGQUEUEOPTIONS);
	msgOptions.dwFlags		= MSGQUEUE_ALLOW_BROKEN;
	msgOptions.cbMaxMessage = sizeof(CcTempSensMsg);
	msgOptions.bReadAccess	= 1;
	CchMsgQueue = CreateMsgQueue(queueName, &msgOptions);

	if (CchMsgQueue == INVALID_HANDLE_VALUE)
		goto clean_up;
	
	/* All is fine */
	return;

clean_up:
	if (queueName)
		LocalFree(queueName);

	if (hTempSensor != INVALID_HANDLE_VALUE)
	{
		MIC_TemperatureSensorDeregister(hTempSensor);
		MIC_TemperatureSensorClose(hTempSensor);
	}
}

BOOL GetDeviceTemperature()
{
	SYSTEMTIME SystemTime;
	int iRes = 0;
	INT16 ResultBuffer[2];
	DWORD NumWordsRet = 0;

	if (INVALID_HANDLE_VALUE != hCtrlCds)
	{
		iRes = ((DeviceIoControl (hCtrlCds, IOCTL_CPUDYNAMICSPEED_GETSTATUS, 0, 0, 
			ResultBuffer, sizeof(ResultBuffer), &NumWordsRet, 
			NULL)) ? ERROR_SUCCESS : GetLastError());
		GetSystemTime(&SystemTime);
		if (iRes == ERROR_SUCCESS) {
			DeviceTempResult[0] = ResultBuffer[0] & 0xff;//Use only lower 8 bits from BGAPTS reading
			DeviceTempResult[1] = ResultBuffer[1];	     //Companion Chip Temperature
			wsprintf(line, L"%2d:%2d:%2d IOCTL Get Status Command Temperature ADC [%d], CC=%d°C ",
				SystemTime.wHour,SystemTime.wMinute, SystemTime.wSecond,DeviceTempResult[0], DeviceTempResult[1]);
		}
		else {
			wsprintf(line, L"IOCTL Get Status error code[%d]",  iRes);
			return FALSE;
		}
		SendMessage(g_hList, LB_ADDSTRING, 0, (LPARAM)line);
	}
	
	return TRUE;
}

void OpenCdsFile()
{
	if (INVALID_HANDLE_VALUE == hCtrlCds)
	{
		hCtrlCds = CreateFile(L"CDS1:", GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, 0, 0);
		if (INVALID_HANDLE_VALUE == hCtrlCds) {
			wsprintf(line, L"CDS1 Opened. Handle[%x] Last Error[%d]", hCtrlCds,GetLastError());
		}
		else {
			wsprintf(line, L"CDS1 Opened. Handle[%x]", hCtrlCds);
		}
		SendMessage(g_hList, LB_ADDSTRING, 0, (LPARAM)line);
	}
	else
	{
		wsprintf(line, L"CDS1 is already open. Using handle[%x]", hCtrlCds);
		SendMessage(g_hList, LB_ADDSTRING, 0, (LPARAM)line);
	}

}

void OccupyCPUResource()
{
	TCHAR line[1024] = {0};
	wsprintf(line, L"Thread Started: OccupyCPUResource");
	if (g_hList)
	{
		SendMessage(g_hList, LB_ADDSTRING, 0, (LPARAM)line);
	}

	while (1)
	{ 
		int i;
		for (i = 0; i < 100000000 ; i++)
		{
			if (LoadTestInProgress == 0)
			{
				break;
			}
		}
		wsprintf(line, L"Cycle ended");
		if (g_hList)
		{
			SendMessage(g_hList, LB_ADDSTRING, 0, (LPARAM)line);
		}
		if (LoadTestInProgress == 0)
		{
			break;
		}
	}
	wsprintf(line, L"Thread OccupyCPUResource: while loop exited");
	if (g_hList)
	{
		SendMessage(g_hList, LB_ADDSTRING, 0, (LPARAM)line);
	}
}

void CdsTestTemperatureThread()
{
	DWORD Written, ret = 0;
	FILETIME filename;
	SYSTEMTIME SystemTime;
//	int aTodTemperature;
//	DWORD dwSize, dwFlags;

#if 0
	/* Temperature sensor in Companion Chip*/
	if (hTempSensor == INVALID_HANDLE_VALUE) StartCompanionChipTempRead();
#endif

	/* Get handle for CDS driver file */
	OpenCdsFile();

	/* Create file for output of test */
	if (ResultsFileHan == INVALID_HANDLE_VALUE) {
		GetSystemTime(&SystemTime);
		SystemTimeToFileTime(&SystemTime, &filename);
		wsprintf(line, L"\\Temp\\CDSTestResult%d.txt", filename.dwLowDateTime);
		ResultsFileHan = CreateFile(line, GENERIC_READ|GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
		WtiteSpeedIntoTestResultFile(0);//A value of "0", gets current value from device
	}
	
	while (CDSTestInprogerss)
	{
		/* First print time in file */
		Written = GetTimeFormat(LOCALE_SYSTEM_DEFAULT, TIME_FORCE24HOURFORMAT, NULL, NULL, line, sizeof(line));
		ret = WideCharToMultiByte(CP_ACP,NULL,line,Written,multiByte,128,NULL,NULL);
		WriteFile(ResultsFileHan,multiByte, Written, &Written, NULL);

		/* Acquire temperature of device and add to file*/
		if (GetDeviceTemperature())
		{
			if ((DeviceTempResult[0] == 0) || (DeviceTempResult[0] > 128)){
				wsprintf(line, L"|  Tj[invalid]");
			}
			else {
				wsprintf(line, L"|  Tj = %2d°C  ",BgaptsTsAdcToTemp[DeviceTempResult[0]]);
			}
			ret = WideCharToMultiByte(CP_ACP,NULL,line,14,multiByte,128,NULL,NULL);
			WriteFile(ResultsFileHan,multiByte, ret, &Written, NULL);

			/* Add Junction Temperature criteria state to file */
			if (DeviceTempResult[0] < 87) {
				wsprintf(line, L"Pass  |  ");
			}
			else {
				wsprintf(line, L"FAIL  |  ");
			}
			ret = WideCharToMultiByte(CP_ACP,NULL,line,9,multiByte,128,NULL,NULL);
			WriteFile(ResultsFileHan,multiByte, ret, &Written, NULL);

			/* Read Companion Chip Temperature from message queue. Don't block: If there's no new message, use last message*/
			//ReadMsgQueue(CchMsgQueue, &CcTempSensMsg, sizeof(CcTempSensMsg), &dwSize, 0, &dwFlags);

			//wsprintf(line, L"Tcc = %2d°C", CcTempSensMsg.CurrentTemperature);
			wsprintf(line, L"Tcc = %2d°C", DeviceTempResult[1]);
			ret = WideCharToMultiByte(CP_ACP,NULL,line,11,multiByte,128,NULL,NULL);
			WriteFile(ResultsFileHan,multiByte, ret, &Written, NULL);

			/* Add companion chip temperature criteria */
			if (DeviceTempResult[1] <= 105) {
				wsprintf(line, L"Pass");
			}
			else {
				wsprintf(line, L"FAIL");
			}
			ret = WideCharToMultiByte(CP_ACP,NULL,line,4,multiByte,128,NULL,NULL);
			WriteFile(ResultsFileHan,multiByte, ret, &Written, NULL);

			/* Force another message to be sent to me, although temperature was nnot necessarily changed */
			//MIC_TemperatureSensorForceEnqueue(hTempSensor);

			/* Insert CPU Load test status */
			if (LoadTestInProgress) wsprintf(line, L"  | CPU Usage High\r\n");
			else wsprintf(line, L"  | CPU Usage Low \r\n");
			ret = WideCharToMultiByte(CP_ACP,NULL,line,20,multiByte,128,NULL,NULL);
			WriteFile(ResultsFileHan,multiByte, ret, &Written, NULL);
		}
		Sleep(1000);
	}
}

int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPTSTR    lpCmdLine,
                   int       nCmdShow)
{
	MSG msg;

	// Perform application initialization:
	if (!InitInstance(hInstance, nCmdShow)) 
	{
		return FALSE;
	}

#ifndef WIN32_PLATFORM_WFSP
	HACCEL hAccelTable;
	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_CDSTESTAPP));
#endif // !WIN32_PLATFORM_WFSP

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0)) 
	{
#ifndef WIN32_PLATFORM_WFSP
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) 
#endif // !WIN32_PLATFORM_WFSP
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
ATOM MyRegisterClass(HINSTANCE hInstance, LPTSTR szWindowClass)
{
	WNDCLASS wc;

	wc.style         = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc   = WndProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hInstance     = hInstance;
	wc.hIcon         = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_CDSTESTAPP));
	wc.hCursor       = 0;
	wc.hbrBackground = (HBRUSH) GetStockObject(WHITE_BRUSH);
	wc.lpszMenuName  = 0;
	wc.lpszClassName = szWindowClass;

	return RegisterClass(&wc);
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
    TCHAR szTitle[MAX_LOADSTRING];		// title bar text
    TCHAR szWindowClass[MAX_LOADSTRING];	// main window class name

    g_hInst = hInstance; // Store instance handle in our global variable

#if defined(WIN32_PLATFORM_PSPC) || defined(WIN32_PLATFORM_WFSP)
    // SHInitExtraControls should be called once during your application's initialization to initialize any
    // of the device specific controls such as CAPEDIT and SIPPREF.
    SHInitExtraControls();
#endif // WIN32_PLATFORM_PSPC || WIN32_PLATFORM_WFSP

    LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING); 
    LoadString(hInstance, IDC_CDSTESTAPP, szWindowClass, MAX_LOADSTRING);

#if defined(WIN32_PLATFORM_PSPC) || defined(WIN32_PLATFORM_WFSP)
    //If it is already running, then focus on the window, and exit
    hWnd = FindWindow(szWindowClass, szTitle);	
    if (hWnd) 
    {
        // set focus to foremost child window
        // The "| 0x00000001" is used to bring any owned windows to the foreground and
        // activate them.
        SetForegroundWindow((HWND)((ULONG) hWnd | 0x00000001));
        return 0;
    } 
#endif // WIN32_PLATFORM_PSPC || WIN32_PLATFORM_WFSP

    if (!MyRegisterClass(hInstance, szWindowClass))
    {
    	return FALSE;
    }

    hWnd = CreateWindow(szWindowClass, szTitle, WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, NULL);

    if (!hWnd)
    {
        return FALSE;
    }

#ifdef WIN32_PLATFORM_PSPC
    // When the main window is created using CW_USEDEFAULT the height of the menubar (if one
    // is created is not taken into account). So we resize the window after creating it
    // if a menubar is present
    if (g_hWndMenuBar)
    {
        RECT rc;
        RECT rcMenuBar;

        GetWindowRect(hWnd, &rc);
        GetWindowRect(g_hWndMenuBar, &rcMenuBar);
        rc.bottom -= (rcMenuBar.bottom - rcMenuBar.top);
		
        MoveWindow(hWnd, rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top, FALSE);
    }
#endif // WIN32_PLATFORM_PSPC

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

#ifndef SHELL_AYGSHELL
    if (g_hWndCommandBar)
    {
        CommandBar_Show(g_hWndCommandBar, TRUE);
    }
#endif // !SHELL_AYGSHELL

    return TRUE;
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

	// Create a push button.
	static HWND hwndG1B1;
	static HWND hlist = 0;
	static HANDLE ThHan = INVALID_HANDLE_VALUE;
	static HANDLE CdsThHan = INVALID_HANDLE_VALUE;
	static HANDLE CDSTestThHan = INVALID_HANDLE_VALUE;

#if defined(SHELL_AYGSHELL) && !defined(WIN32_PLATFORM_WFSP)
    static SHACTIVATEINFO s_sai;
#endif // SHELL_AYGSHELL && !WIN32_PLATFORM_WFSP
	
    switch (message) 
    {
        case WM_COMMAND:
			if(wParam ==  MAKELONG(ID_BUTTON1, BN_CLICKED)){
						SendMessage(g_hList, LB_RESETCONTENT, 0, 0);

			}

            wmId    = LOWORD(wParam); 
            wmEvent = HIWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
#ifndef WIN32_PLATFORM_WFSP
                case IDM_HELP_ABOUT:
                    DialogBox(g_hInst, (LPCTSTR)IDD_ABOUTBOX, hWnd, About);
                    break;
#endif // !WIN32_PLATFORM_WFSP
#ifndef SHELL_AYGSHELL
                case IDM_FILE_EXIT:
                    DestroyWindow(hWnd);
                    break;
#endif // !SHELL_AYGSHELL
#ifdef WIN32_PLATFORM_WFSP
                case IDM_OK:
                    DestroyWindow(hWnd);
                    break;
#endif // WIN32_PLATFORM_WFSP
#ifdef WIN32_PLATFORM_PSPC
                case IDM_OK:
                    SendMessage (hWnd, WM_CLOSE, 0, 0);				
                    break;
#endif // WIN32_PLATFORM_PSPC
				case ID_LOADTEST_START:
					{
						wsprintf(line, L"Test started");
						if (g_hList)
						{
							SendMessage(g_hList, LB_ADDSTRING, 0, (LPARAM)line);
						}
						LoadTestInProgress = 1;
						ThHan = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)OccupyCPUResource,NULL,0,0);
					}
					break;

				case ID_LOADTEST_END:
					LoadTestInProgress = 0;
					if (ThHan != INVALID_HANDLE_VALUE)
					{
						CloseHandle (ThHan);
					}
					break;

				case ID_FILE_LOADCDSDRIVER:
					if (CdsThHan == INVALID_HANDLE_VALUE)
					{
						CdsThHan = ActivateDevice(L"Drivers\\BuiltIn\\CDS",0);
						if (CdsThHan == INVALID_HANDLE_VALUE)
						{
							wsprintf(line, L"CDS driver load failure. Last Error[%d]", GetLastError());
							SendMessage(g_hList, LB_ADDSTRING, 0, (LPARAM)line);
						}
						else
						{
							wsprintf(line, L"CDS driver load success [%x]", CdsThHan);
							SendMessage(g_hList, LB_ADDSTRING, 0, (LPARAM)line);
							hCtrlCds = CreateFile(L"CDS1:", GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, 0, 0);
							wsprintf(line, L"CDS1 Opened. Handle[%x] Last Error[%d]", hCtrlCds,GetLastError());
							SendMessage(g_hList, LB_ADDSTRING, 0, (LPARAM)line);
						}
					}
					else
					{
						wsprintf(line, L"CDS driver already loaded [%x]", CdsThHan);
						SendMessage(g_hList, LB_ADDSTRING, 0, (LPARAM)line);
					}
					break;

				case ID_FILE_UNLOADCDSDRIVER:
					if (CdsThHan != INVALID_HANDLE_VALUE)
					{
						if((void *)-1 != hCtrlCds)	{
							CloseHandle(hCtrlCds);
							DWORD res =  GetLastError();
							wsprintf(line, L"CDS File Handle closed [%x] Error code[%d]", hCtrlCds, res);
							SendMessage(g_hList, LB_ADDSTRING, 0, (LPARAM)line);
							if (res == ERROR_SUCCESS)hCtrlCds = 0;
						}
						DeactivateDevice(CdsThHan);
						wsprintf(line, L"CDS driver unloaded [%x]", CdsThHan);
						SendMessage(g_hList, LB_ADDSTRING, 0, (LPARAM)line);
						CdsThHan = 0;
					}
					else
					{
						wsprintf(line, L"CDS driver not loaded");
						SendMessage(g_hList, LB_ADDSTRING, 0, (LPARAM)line);
					}
					break;
				case ID_FILE_OPENCDSFILE:
					{
						OpenCdsFile();
					}
					break;
				case ID_FILE_GETIOCTLSTATUS:
					{
						GetDeviceTemperature();
					}
					break;
				case ID_CPUFREQUENCY_1GHZ:
					if ((void *)-1 != hCtrlCds)
					{
						WtiteSpeedIntoTestResultFile(OPM_1GHz);
					}
					break;
				case ID_CPUFREQUENCY_800MHZ:
					if ((void *)-1 != hCtrlCds)
					{
						WtiteSpeedIntoTestResultFile(OPM_800MHz);
					}
					break;
				case ID_CPUFREQUENCY_600MHZ:
					if ((void *)-1 != hCtrlCds)
					{
						WtiteSpeedIntoTestResultFile(OPM_600MHz);
					}
					break;
				case ID_TEMPERATURETEST_STARTTEST:
					{
						CDSTestInprogerss = 1;
						if (CDSTestThHan == INVALID_HANDLE_VALUE) {
							CDSTestThHan = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)CdsTestTemperatureThread,NULL,0,0);
						}
					}
					break;
				case ID_TEMPERATURETEST_ENDTEST:
					{
						CDSTestInprogerss = 0;
						if (CDSTestThHan != INVALID_HANDLE_VALUE)
						{
							CloseHandle (CDSTestThHan);
							CDSTestThHan = INVALID_HANDLE_VALUE;
						}

						if (ResultsFileHan != INVALID_HANDLE_VALUE) {
							CloseHandle (ResultsFileHan);
							ResultsFileHan = INVALID_HANDLE_VALUE;
						}
					}
					break;
				case ID_FILE_GETTHERMALSHUTDOWN:
					{
						GetThermalShudownState();
					}
					break;


                default:
                    return DefWindowProc(hWnd, message, wParam, lParam);
            }
            break;
        case WM_CREATE:
#ifndef SHELL_AYGSHELL
            g_hWndCommandBar = CommandBar_Create(g_hInst, hWnd, 1);
            CommandBar_InsertMenubar(g_hWndCommandBar, g_hInst, IDR_MENU, 0);
            CommandBar_AddAdornments(g_hWndCommandBar, 0, 0);
#endif // !SHELL_AYGSHELL
#ifdef SHELL_AYGSHELL
            SHMENUBARINFO mbi;

            memset(&mbi, 0, sizeof(SHMENUBARINFO));
            mbi.cbSize     = sizeof(SHMENUBARINFO);
            mbi.hwndParent = hWnd;
            mbi.nToolBarId = IDR_MENU;
            mbi.hInstRes   = g_hInst;

            if (!SHCreateMenuBar(&mbi)) 
            {
                g_hWndMenuBar = NULL;
            }
            else
            {
                g_hWndMenuBar = mbi.hwndMB; 
            }

#ifndef WIN32_PLATFORM_WFSP
            // Initialize the shell activate info structure
            memset(&s_sai, 0, sizeof (s_sai));
            s_sai.cbSize = sizeof (s_sai);
#endif // !WIN32_PLATFORM_WFSP
#endif // SHELL_AYGSHELL

			RECT r;
			GetClientRect(hWnd, &r);
			r.top +=  GetSystemMetrics(SM_CYMENU) + GetSystemMetrics(SM_CYBORDER); 
			//r.bottom +=  GetSystemMetrics(SM_CXVSCROLL);

			g_hList = hlist = CreateWindow(L"Listbox", 0, (LBS_STANDARD & ~LBS_SORT) | WS_CHILD | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL | WS_TABSTOP,
								 r.left, r.top + 100, r.right - r.left,
								 r.bottom - r.top - 100,
								 hWnd,(HMENU)0, g_hInst, 0);

			hwndG1B1 = CreateWindow( BUTTONCLASS, L"Clear Box", 
				WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 
				r.left + 20, r.top + 20 , 100, 
				60, hWnd, 
				(HMENU) ID_BUTTON1, // 0x8801
				g_hInst, NULL );

            break;
        case WM_PAINT:
            hdc = BeginPaint(hWnd, &ps);

			// TODO: Add any drawing code here...
            
            EndPaint(hWnd, &ps);
            break;
        case WM_DESTROY:
#ifndef SHELL_AYGSHELL
            CommandBar_Destroy(g_hWndCommandBar);
#endif // !SHELL_AYGSHELL
#ifdef SHELL_AYGSHELL
            CommandBar_Destroy(g_hWndMenuBar);
#endif // SHELL_AYGSHELL
			CDSTestInprogerss = 0;

			if((void *)-1 != hCtrlCds)	{
				CloseHandle(hCtrlCds);
			}
			if (CdsThHan != INVALID_HANDLE_VALUE)	{
				DeactivateDevice(CdsThHan);
			}
			if (ThHan != INVALID_HANDLE_VALUE)	{
				CloseHandle (ThHan);
			}
			if (ResultsFileHan != INVALID_HANDLE_VALUE) {
				CloseHandle (ResultsFileHan);
			}
			if (CDSTestThHan != INVALID_HANDLE_VALUE)
			{
				CloseHandle (CDSTestThHan);
			}
			if (hTempSensor != INVALID_HANDLE_VALUE)
			{
				MIC_TemperatureSensorDeregister(hTempSensor);
				MIC_TemperatureSensorClose(hTempSensor);
			}
			if (queueName)
				LocalFree(queueName);
			if (CchMsgQueue != INVALID_HANDLE_VALUE)
			{
				CloseMsgQueue(CchMsgQueue);
			}
			PostQuitMessage(0);
            break;

#if defined(SHELL_AYGSHELL) && !defined(WIN32_PLATFORM_WFSP)
        case WM_ACTIVATE:
            // Notify shell of our activate message
            SHHandleWMActivate(hWnd, wParam, lParam, &s_sai, FALSE);
            break;
        case WM_SETTINGCHANGE:
            SHHandleWMSettingChange(hWnd, wParam, lParam, &s_sai);
            break;
#endif // SHELL_AYGSHELL && !WIN32_PLATFORM_WFSP
			if(hlist)
				DestroyWindow(hlist);
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

#ifndef WIN32_PLATFORM_WFSP
// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_INITDIALOG:
#ifndef SHELL_AYGSHELL
            RECT rectChild, rectParent;
            int DlgWidth, DlgHeight;	// dialog width and height in pixel units
            int NewPosX, NewPosY;

            // trying to center the About dialog
            if (GetWindowRect(hDlg, &rectChild)) 
            {
                GetClientRect(GetParent(hDlg), &rectParent);
                DlgWidth	= rectChild.right - rectChild.left;
                DlgHeight	= rectChild.bottom - rectChild.top ;
                NewPosX		= (rectParent.right - rectParent.left - DlgWidth) / 2;
                NewPosY		= (rectParent.bottom - rectParent.top - DlgHeight) / 2;
				
                // if the About box is larger than the physical screen 
                if (NewPosX < 0) NewPosX = 0;
                if (NewPosY < 0) NewPosY = 0;
                SetWindowPos(hDlg, 0, NewPosX, NewPosY,
                    0, 0, SWP_NOZORDER | SWP_NOSIZE);
            }
#endif // !SHELL_AYGSHELL
#ifdef SHELL_AYGSHELL
            {
                // Create a Done button and size it.  
                SHINITDLGINFO shidi;
                shidi.dwMask = SHIDIM_FLAGS;
                shidi.dwFlags = SHIDIF_DONEBUTTON | SHIDIF_SIPDOWN | SHIDIF_SIZEDLGFULLSCREEN | SHIDIF_EMPTYMENU;
                shidi.hDlg = hDlg;
                SHInitDialog(&shidi);
            }
#endif // SHELL_AYGSHELL

            return (INT_PTR)TRUE;

        case WM_COMMAND:
#ifndef SHELL_AYGSHELL
            if ((LOWORD(wParam) == IDOK) || (LOWORD(wParam) == IDCANCEL))
#endif // !SHELL_AYGSHELL
#ifdef SHELL_AYGSHELL
            if (LOWORD(wParam) == IDOK)
#endif
            {
                EndDialog(hDlg, LOWORD(wParam));
                return (INT_PTR)TRUE;
            }
            break;

        case WM_CLOSE:
            EndDialog(hDlg, message);
            return (INT_PTR)TRUE;

    }
    return (INT_PTR)FALSE;
}
#endif // !WIN32_PLATFORM_WFSP
