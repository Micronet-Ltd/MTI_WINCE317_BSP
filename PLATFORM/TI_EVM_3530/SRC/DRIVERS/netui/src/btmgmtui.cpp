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
//
//	This code implements user part of activation of Bluetooth peers.
//
//	It creates registry key upon which the boot time activation implemented
//	in public\common\oak\drivers\bluetooth\samples\btsvc is heavily dependent.
//
//	Please maintain synchronization between the two files.
//
#include <windows.h>
#include <commctrl.h>

#include <winsock2.h>

#include "Bluetooth.h"
#include "resource.h"

#include "svsutil.hxx"

#include <bt_api.h>
#include <bthapi.h>
#include <initguid.h>
#include <bt_sdp.h>

#include <bt_buffer.h>
#include <bt_ddi.h>

//#include "../bluetooth/sample/btenum/btenum.hxx"
#include	"..\..\..\..\public\common\oak\drivers\bluetooth\sample\btenum\btenum.hxx"//temp!!!!

#ifdef SDK_BUILD
#include <Sipapi.h>
#include <aygshell.h>
#endif

//////////////////////////////////////////
#define MAX_NAME			248
#define DEVICE_MAX			40

#define MAX_LOC_STRING		256

#define STATUS_AUTHENTICATE			1
#define STATUS_NOAUTHENTICATE		2
#define STATUS_CANCELAUTHENTICATE	3
#define STATUS_AUTHERROR			4
#define STATUS_AUTHERROR_CONNECT	5
#define STATUS_AUTHERROR_NODRIVER	6

const UUID HIDServiceClassID_UUID = {0x00001124, 0x0000, 0x1000, {0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB}};
#define HIDServiceClassID_UUID16 0x1124

const UUID ActiveSyncServiceClassID_UUID = {0x0350278F, 0x3DCA, 0x4e62, {0x83, 0x1D, 0xA4, 0x11, 0x65, 0xFF, 0x90, 0x6C}};

#define HandsFreeServiceClassID_UUID16 0x111e

//
//	Device classification
//

#define IMAGE_TRUSTED		0
#define IMAGE_UNTRUSTED		1
#define IMAGE_QUESTION		2

#define IMAGE_MODEM			3
#define IMAGE_MODEM_A		4
#define IMAGE_PRINTER		5
#define IMAGE_PRINTER_A		6
#define IMAGE_LAP			7
#define IMAGE_LAP_A			8
#define IMAGE_OBEX_FTP		9
#define IMAGE_OBEX_FTP_A	10
#define IMAGE_OBEX_OPP		11
#define IMAGE_OBEX_OPP_A	12
#define IMAGE_HEADSET		13
#define IMAGE_HEADSET_A		14
#define IMAGE_ASYNC			15
#define IMAGE_ASYNC_A		16
#define IMAGE_KEYBOARD		17
#define IMAGE_KEYBOARD_A	18
#define IMAGE_MOUSE			19
#define IMAGE_MOUSE_A		20
#define IMAGE_PAN			21
#define IMAGE_PAN_A			22
#define IMAGE_HF			23
#define IMAGE_HF_A			24


#define WM_BTMGR_POPULATE_LISTS     (WM_USER + 1)

static DWORD gszIconList[] = {IDI_TRUSTED, IDI_UNTRUSTED, IDI_QUESTIONMARK,
		IDI_MODEM,			IDI_ACTIVE_MODEM,
		IDI_PRINT,			IDI_ACTIVE_PRINT,
		IDI_LAN,			IDI_ACTIVE_LAN,
		IDI_FILE_TRANSFER,	IDI_ACTIVE_FILE_TRANSFER,
		IDI_OBJ_TRANSFER,	IDI_ACTIVE_OBJ_TRANSFER,
		IDI_HEADSET,		IDI_ACTIVE_HEADSET,
		IDI_ACTIVESYNC,		IDI_ACTIVE_ACTIVESYNC,
		IDI_KEYBOARD,		IDI_ACTIVE_KEYBOARD,
		IDI_MOUSE,			IDI_ACTIVE_MOUSE,
		IDI_PAN,			IDI_ACTIVE_PAN,
		IDI_HANDSFREE,		IDI_ACTIVE_HANDSFREE

};

////////////////////////////////////////////////////////////////////////////////////////
// all data structures used
////////////////////////////////////////////////////////////////////////////////////////
struct BTDEVICE : public BTDEV {
	int			iDeviceIconA;
	int			iDeviceIconI;

	BTDEVICE(BT_ADDR b, int iclass) : BTDEV(b, iclass) {
		iDeviceIconA = iDeviceIconI = IMAGE_QUESTION;
	}

	BTDEVICE(BTDEV *pbtd) : BTDEV(*pbtd) {
		iDeviceIconA = iDeviceIconI = IMAGE_QUESTION;
	}
};

// COM library functions
typedef DWORD (WINAPI * p_CoInitializeEx)(LPVOID pvReserved, DWORD dwCoInit);
typedef DWORD (WINAPI * p_CoUninitialize)();
typedef DWORD (WINAPI * p_CoCreateInstance)(REFCLSID rclsid, LPUNKNOWN pUnkOuter, DWORD dwClsContext, REFIID riid, LPVOID *ppv);
typedef DWORD (WINAPI * p_CoTaskMemAlloc)(ULONG cb);
typedef DWORD (WINAPI * p_CoTaskMemFree)(LPVOID pv);

// bluetooth library functions
typedef DWORD (WINAPI * p_BthNsLookupServiceEnd)(HANDLE hLookup);
typedef DWORD (WINAPI * p_BthGetLinkKey)(BT_ADDR *pba, unsigned char *pkey);
typedef DWORD (WINAPI * p_BthNsLookupServiceNext)(HANDLE hLookup, DWORD dwFlags, LPDWORD lpdwBufferLength, LPWSAQUERYSET lpqsResults);
typedef DWORD (WINAPI * p_BthNsLookupServiceBegin)(LPWSAQUERYSET pQuerySet, DWORD dwFlags, LPHANDLE lphLookup);
typedef DWORD (WINAPI * p_BthAuthenticate)(BT_ADDR *pba);
typedef DWORD (WINAPI * p_BthSetPIN)(BT_ADDR *pba, int cPinLength, unsigned char *ppin);
typedef DWORD (WINAPI * p_BthRevokeLinkKey)(BT_ADDR *pba);
typedef DWORD (WINAPI * p_BthRevokePIN)(BT_ADDR *pba);
typedef DWORD (WINAPI * p_BthCreateBasebandConnection) (BT_ADDR *pba, unsigned short *phandle);
typedef DWORD (WINAPI * p_BthCloseBasebandConnection) (unsigned short handle);


//commctrl library functions
typedef DWORD (WINAPI * p_PropertySheet)(LPCPROPSHEETHEADER lppsph);

////////////////////////////////////////////////////////////////////////////////////////
// ALL INTERNAL FUNCTION DEFINITIONS
////////////////////////////////////////////////////////////////////////////////////////
static HIMAGELIST LoadIcons();
static BOOL CALLBACK DlgProcScanDC (HWND, UINT, WPARAM, LPARAM);
static BOOL CALLBACK DlgProcScanDCP (HWND, UINT, WPARAM, LPARAM);
static int StopInquiryAndPair (void);
static int CreateLists();
static int PopulateLists();
static int CreateActiveMenu(HWND hwndParent, int fProtrait);
static void CreateDevice (void);
static DWORD WINAPI DoInquiry (LPVOID);
static BTDEVICE *FindTrustedOrEmptyService(BT_ADDR, int iService);
static BTDEVICE *FindTrustedDevice(BT_ADDR);
static void DeleteListItem (HWND, BTDEVICE *);
static int GetBA (WCHAR *, BT_ADDR *);
static int DoSDP (BT_ADDR *, int, unsigned char *, unsigned char **, unsigned int *, DWORD *, GUID *);
static int VerifyHIDRecord (unsigned char *pStream, int cStream, DWORD *pdwDevSubclass);
static HRESULT FindRFCOMMChannel (unsigned char *, int, unsigned char *, GUID* pguidSearch);
static HRESULT FindPANService (unsigned char *pStream, int cStream, GUID *pServiceId);
static HRESULT ServiceAndAttributeSearch(UCHAR*, DWORD, ISdpRecord ***, ULONG *);
static int IsRfcommUuid(NodeData *);
static int GetChannel (NodeData *);
static void DeleteUntrusted();
static void RemovePINLink(BTDEVICE*);
static int ifAuthenticate();
static int CALLBACK PropSheetCallback(HWND hDlg, UINT uMsg, LPARAM lParam);
static void ActivateBthDevice(BTDEVICE* pbtDevice);
static int GetSelectedListItem(HWND hwndList, LV_ITEM* plvi, WCHAR* szTextBuffer, int cbTextBuffer);

////////////////////////////////////////////////////////////////////////////////////////
//ALL GLOBAL DATA USED
////////////////////////////////////////////////////////////////////////////////////////

static p_CoInitializeEx g_pfnCoInitializeEx;
static p_CoUninitialize g_pfnCoUninitialize;
static p_CoCreateInstance g_pfnCoCreateInstance;
static p_CoTaskMemAlloc g_pfnCoTaskMemAlloc;
static p_CoTaskMemFree g_pfnCoTaskMemFree;

static p_BthNsLookupServiceEnd g_pfnBthNsLookupServiceEnd;
static p_BthGetLinkKey g_pfnBthGetLinkKey;
static p_BthNsLookupServiceNext g_pfnBthNsLookupServiceNext;
static p_BthNsLookupServiceBegin g_pfnBthNsLookupServiceBegin;
static p_BthAuthenticate g_pfnBthAuthenticate;
static p_BthSetPIN g_pfnBthSetPIN;
static p_BthRevokeLinkKey g_pfnBthRevokeLinkKey;
static p_BthRevokePIN g_pfnBthRevokePIN;
static p_BthCreateBasebandConnection g_pfnBthBasebandConnect;
static p_BthCloseBasebandConnection g_pfnBthBasebandDisconnect;
 
static HANDLE ghInquiryThread = NULL;

static HINSTANCE ghInst;
static HWND      ghWnd;

static HANDLE ghListsPopulated;

static HANDLE ghInquiryHandle;

static BTDEVICE* gpbtList[DEVICE_MAX];
static DWORD gdwNoOfDevice = 0;
static HWND ghUntrustedList;
static HWND ghTrustedList;
static int gfPortrait = FALSE;
static HIMAGELIST ghIconList = NULL;

static WCHAR gszUnnamed[MAX_LOC_STRING];
static WCHAR gszStop[MAX_LOC_STRING];
static WCHAR gszScan[MAX_LOC_STRING];
static WCHAR gszInquiryTitle[MAX_LOC_STRING];
static WCHAR gszCaption[MAX_LOC_STRING];
static WCHAR gszReqAuth[MAX_LOC_STRING];

static WCHAR gszError[MAX_LOC_STRING];
static WCHAR gszErrFailComm[MAX_LOC_STRING];
static WCHAR gszErrAuthFailed[MAX_LOC_STRING];
static WCHAR gszErrHwError[MAX_LOC_STRING];
static WCHAR gszErrHardwareNotPresent[MAX_LOC_STRING];
static WCHAR gszErrOperationCancelled[MAX_LOC_STRING];
static WCHAR gszErrInvalidOperation[MAX_LOC_STRING];
static WCHAR gszErrOOM[MAX_LOC_STRING];
static WCHAR gszErrConnectionFailed[MAX_LOC_STRING];
static WCHAR gszErrAuthFailedNoDriver[MAX_LOC_STRING];
static WCHAR gszErrAuthFailedCantConnect[MAX_LOC_STRING];

BOOL	g_TotalStop = 0;
static int Reference (BTDEVICE *pbtd, HWND hList) {
	int icount = 0;
	int itotalItems = ListView_GetItemCount(hList);
	for (int i=itotalItems-1; i>=0; --i) {
		LV_ITEM lvi;
		memset (&lvi, 0, sizeof(lvi));
	    lvi.mask = LVIF_PARAM | LVIF_IMAGE;
		lvi.stateMask = -1;
		lvi.iItem = i;
		ListView_GetItem(hList, &lvi);
		BTDEVICE *pbt = (BTDEVICE*) lvi.lParam;
		if (pbt == pbtd)
			++icount;
	}

	return icount;
}

static int Reference (BTDEVICE *pbtd) {
	return Reference (pbtd, ghTrustedList) + ((!gfPortrait) ? Reference (pbtd, ghUntrustedList) : 0);
}

static int WaitForSingleObjectMessageLoop (HANDLE hEvent, DWORD dwTimeout)
{
    while (TRUE)
    {
        MSG msg ; 

        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) 
        { 
            // If it's a quit message, we're out of here.
            if (msg.message == WM_QUIT)  
                return -1; 

            DispatchMessage(&msg); 
        }

        // Wait for any message sent or posted to this queue 
        // or for one of the passed handles be set to signaled.
        DWORD result = MsgWaitForMultipleObjects(1, &hEvent, 
                 FALSE, dwTimeout, QS_ALLINPUT); 

        // The result tells us the type of event we have.
        if (result == (WAIT_OBJECT_0 + 1))
        {
            // New messages have arrived. 
            // Continue to the top of the always while loop to 
            // dispatch them and resume waiting.
            continue;
        } 

        return result;
    }
}

////////////////////////////////////////////////////////////////////////////////////////
// enter the program through this function
// it creates the initial dialog depending upon system metrics
////////////////////////////////////////////////////////////////////////////////////////
extern "C" int CreateScanDevice(HINSTANCE hInstance, HWND hWnd){
	
	DLGPROC dlgProc;
	LPWSTR szTemplateId;
	PROPSHEETPAGE  psp[1];
    	PROPSHEETHEADER psh;
	WCHAR szTitle[100];
	WCHAR szCaption[100];

	//ghWnd = NULL;
	RETAILMSG(0,(L"+CreateScanDevice\r\n"));
	//make sure we have only one instance of this running
	HANDLE hLock = CreateSemaphore(NULL, 0, 1, L"bthmgmtui");
	if(hLock == NULL) {
		return -1;
	}
	if(GetLastError() == ERROR_ALREADY_EXISTS)
	{
		RETAILMSG(1, (L"CreateScanDevice ERROR_ALREADY_EXISTS\r\n"));
		ReleaseSemaphore(hLock, 1, NULL);
		CloseHandle(hLock);
		
		LoadString(hInstance, IDS_BT_CAPTION, szTitle, sizeof(szTitle)/sizeof(*szTitle));
		
		// Bring the other instance to front
		HWND hPrevWnd = FindWindow(L"Dialog", szTitle);
		if(IsWindow(hPrevWnd))
		{
			SetForegroundWindow((HWND) ((ULONG) hPrevWnd | 0x00000001));
		}
		
		return -1;
	}
	ghWnd = NULL;

    ghListsPopulated = CreateEvent(NULL, FALSE, FALSE, NULL);
    if(!ghListsPopulated) {
        ReleaseSemaphore(hLock, 1, NULL);
        CloseHandle(hLock);
    }

	//load the bluetooth library	
	HINSTANCE hBthDll = LoadLibrary (L"btdrt.dll");
	if(!hBthDll) {
	    CloseHandle(ghListsPopulated);
	    
		ReleaseSemaphore(hLock, 1, NULL);
		CloseHandle(hLock); 

		return 0;
	}

	g_pfnBthNsLookupServiceEnd = (p_BthNsLookupServiceEnd)GetProcAddress (hBthDll, L"BthNsLookupServiceEnd");
	g_pfnBthGetLinkKey = (p_BthGetLinkKey)GetProcAddress (hBthDll, L"BthGetLinkKey");
	g_pfnBthNsLookupServiceNext = (p_BthNsLookupServiceNext)GetProcAddress (hBthDll, L"BthNsLookupServiceNext");
	g_pfnBthNsLookupServiceBegin = (p_BthNsLookupServiceBegin)GetProcAddress (hBthDll, L"BthNsLookupServiceBegin");	
	g_pfnBthAuthenticate = (p_BthAuthenticate)GetProcAddress (hBthDll, L"BthAuthenticate");
	g_pfnBthSetPIN = (p_BthSetPIN)GetProcAddress (hBthDll, L"BthSetPIN");
	g_pfnBthRevokeLinkKey = (p_BthRevokeLinkKey)GetProcAddress (hBthDll, L"BthRevokeLinkKey");
	g_pfnBthRevokePIN = (p_BthRevokePIN)GetProcAddress (hBthDll, L"BthRevokePIN");
	g_pfnBthBasebandConnect = (p_BthCreateBasebandConnection) GetProcAddress (hBthDll, L"BthCreateACLConnection");
	g_pfnBthBasebandDisconnect = (p_BthCloseBasebandConnection) GetProcAddress (hBthDll, L"BthCloseConnection");

	if(!g_pfnBthAuthenticate || !g_pfnBthSetPIN || !g_pfnBthNsLookupServiceEnd || !g_pfnBthGetLinkKey ||
		!g_pfnBthNsLookupServiceNext || !g_pfnBthNsLookupServiceBegin || !g_pfnBthRevokeLinkKey || !g_pfnBthRevokePIN) {
		FreeLibrary(hBthDll);   

        CloseHandle(ghListsPopulated);

		ReleaseSemaphore(hLock, 1, NULL);
		CloseHandle(hLock); 

		return 0;
	}	

	HINSTANCE hCOMDll = LoadLibrary (L"ole32.dll");
	if(!hCOMDll) {
		FreeLibrary(hCOMDll);   
		FreeLibrary(hBthDll);   

        CloseHandle(ghListsPopulated);

		ReleaseSemaphore(hLock, 1, NULL);
		CloseHandle(hLock); 
		return 0;
	}

	g_pfnCoInitializeEx = (p_CoInitializeEx)GetProcAddress (hCOMDll, L"CoInitializeEx");
	g_pfnCoUninitialize = (p_CoUninitialize)GetProcAddress (hCOMDll, L"CoUninitialize");
	g_pfnCoCreateInstance = (p_CoCreateInstance)GetProcAddress (hCOMDll, L"CoCreateInstance");
	g_pfnCoTaskMemAlloc = (p_CoTaskMemAlloc)GetProcAddress (hCOMDll, L"CoTaskMemAlloc");
	g_pfnCoTaskMemFree = (p_CoTaskMemFree)GetProcAddress (hCOMDll, L"CoTaskMemFree");
	
	
	if(!g_pfnCoInitializeEx || !g_pfnCoUninitialize || !g_pfnCoCreateInstance || !g_pfnCoTaskMemAlloc || !g_pfnCoTaskMemFree) {
		FreeLibrary(hCOMDll);   
		FreeLibrary(hBthDll);   

        CloseHandle(ghListsPopulated);
    
    	ReleaseSemaphore(hLock, 1, NULL);
		CloseHandle(hLock); 

		return 0;
	}		    
	
	ghInst = hInstance;

    LoadString(ghInst, IDS_BT_UNNAMED, gszUnnamed, MAX_LOC_STRING);
    LoadString(ghInst, IDS_BT_STOP, gszStop, MAX_LOC_STRING);
    LoadString(ghInst, IDS_BT_SCAN, gszScan, MAX_LOC_STRING);
    LoadString(ghInst, IDS_BT_IN_INQUIRY, gszInquiryTitle, MAX_LOC_STRING);
    LoadString(ghInst, IDS_BT_CAPTION, gszCaption, MAX_LOC_STRING);
	LoadString(ghInst, IDS_BT_REQ_AUTH, gszReqAuth, MAX_LOC_STRING);
    LoadString(ghInst, IDS_BT_ERROR, gszError, MAX_LOC_STRING);
    LoadString(ghInst, IDS_BT_AUTH_FAILED, gszErrAuthFailed, MAX_LOC_STRING);
    LoadString(ghInst, IDS_BT_HW_ERROR, gszErrHwError, MAX_LOC_STRING);
    LoadString(ghInst, IDS_BT_FAIL_COMM, gszErrFailComm, MAX_LOC_STRING);
	LoadString(ghInst, IDS_BT_NO_HW, gszErrHardwareNotPresent, MAX_LOC_STRING);
	LoadString(ghInst, IDS_BT_CANCELLED, gszErrOperationCancelled, MAX_LOC_STRING);
	LoadString(ghInst, IDS_BT_OP_INVALID, gszErrInvalidOperation, MAX_LOC_STRING);
	LoadString(ghInst, IDS_BT_OOM, gszErrOOM, MAX_LOC_STRING);
	LoadString(ghInst, IDS_BT_CANT_CONNECT, gszErrConnectionFailed, MAX_LOC_STRING);
	LoadString(ghInst, IDS_BT_AUTH_NO_DRV, gszErrAuthFailedNoDriver, MAX_LOC_STRING);
	LoadString(ghInst, IDS_BT_AUTH_CNCT, gszErrAuthFailedCantConnect, MAX_LOC_STRING);

	//initialise the globals
	ghUntrustedList = NULL;
	ghTrustedList = NULL;
	//load any devices from the registry
	CreateDevice();
	// decide which dialog will b using
	if(GetSystemMetrics(SM_CXSCREEN) < GetSystemMetrics(SM_CYSCREEN)) {
		gfPortrait = TRUE;
	}
	
	// Load the appropriate resource
	if(gfPortrait) {
		dlgProc = DlgProcScanDCP;
		szTemplateId = MAKEINTRESOURCE(IDD_SCAN_DC_P);
	} else {
		dlgProc = DlgProcScanDC;
		szTemplateId = MAKEINTRESOURCE(DLG_SCAN_DC);
	}	
    // single tab property sheet
    psp[0].dwSize = sizeof(PROPSHEETPAGE);
    psp[0].dwFlags = PSP_USETITLE | PSP_HASHELP;
    psp[0].hInstance = ghInst;
    psp[0].pszTemplate = szTemplateId;
    psp[0].pszIcon = NULL;
    psp[0].pfnDlgProc = dlgProc;
    LoadString(ghInst, IDS_BT_TITLE, szTitle , 100);
    psp[0].pszTitle = szTitle;
    psp[0].pfnCallback = NULL;
    psp[0].lParam = 0;

    // property sheet header
    psh.dwSize = sizeof(PROPSHEETHEADER);
	// Something odd happens if we set the parent and use the callback (which strips off the WS_POPUP
	// style bit).  The new propertysheet tries to draw as a child control and both dialogs lockup. 
	// So if there is an hwnd we will not use the callback.
	if (hWnd)
	{
		// If there is an hWnd passed in then we want this to parent to it.
	    psh.dwFlags = PSH_PROPSHEETPAGE | PSH_HASHELP | PSH_USEICONID;
	}else{
	    psh.dwFlags = PSH_PROPSHEETPAGE | PSH_HASHELP | PSH_USECALLBACK | PSH_USEICONID;
		psh.pfnCallback = PropSheetCallback;
	}

	psh.hwndParent = hWnd;
    psh.hInstance = ghInst;
    psh.pszIcon = MAKEINTRESOURCE(IDI_BLUETOOTH);
    LoadString(ghInst, IDS_BT_PROP, szCaption,100);
    psh.pszCaption = szCaption;
    psh.nPages = 1;
    psh.nStartPage = 0;
    psh.ppsp = psp;

	HINSTANCE hCctlDll = LoadLibrary (L"commctrl.dll");
	if(hCctlDll) {
		p_PropertySheet pfnPropertySheet = (p_PropertySheet)GetProcAddress (hCctlDll, L"PropertySheetW");
		if(pfnPropertySheet) {
			pfnPropertySheet(&psh);
		}
		FreeLibrary (hCctlDll);  
	}

	///////////////////////////////////////////////////////////////////////////////////////

	//if (ghInquiryHandle) {
	//	int ret = 0;
	//	ret = g_pfnBthNsLookupServiceEnd(ghInquiryHandle);
	//	RETAILMSG(1,(L"CreateScanDevice g_pfnBthNsLookupServiceEnd h %x, ret %d, err %d\r\n", ghInquiryHandle, ret, WSAGetLastError()));
	//	ghInquiryHandle = NULL;
	//}

	//if (ghInquiryThread) {
	//	RETAILMSG(1,(L"CreateScanDevice ghInquiryThread\r\n"));
	//	WaitForSingleObjectMessageLoop (ghInquiryThread, INFINITE);

	//	CloseHandle (ghInquiryThread);
	//}

	FreeLibrary(hCOMDll);   
	FreeLibrary(hBthDll);   

	CloseHandle(ghListsPopulated);

	ReleaseSemaphore(hLock, 1, NULL);
	CloseHandle(hLock); 
	RETAILMSG(0, (L"-CreateScanDevice\r\n"));
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
//Property sheet initalization callback
////////////////////////////////////////////////////////////////////////////////////////
static int CALLBACK PropSheetCallback(HWND hDlg, UINT uMsg, LPARAM lParam)
{
    if (PSCB_PRECREATE == uMsg)
        ((LPDLGTEMPLATE)lParam)->style &= ~WS_POPUP; // Treat this like a top level window
    return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
//Dialog Proc for the landscape dialog
////////////////////////////////////////////////////////////////////////////////////////
static BOOL CALLBACK DlgProcScanDC (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) 
	{
		case WM_INITDIALOG:
		{	
			ghWnd = hWnd;
			SetForegroundWindow (hWnd);
			//setting the list as globals so that GetDlgItem doesn't need to b called again & again
			ghUntrustedList = GetDlgItem(hWnd, IDC_UNTRUSTED);
			ghTrustedList = GetDlgItem(hWnd, IDC_TRUSTED);

			//create icon list & add to both lists
			LoadIcons();
			ListView_SetImageList(ghUntrustedList, ghIconList, LVSIL_SMALL);
			ListView_SetImageList(ghTrustedList, ghIconList, LVSIL_SMALL);			
			//create & populate the too lists
			CreateLists();
		}
		return 0;

        case WM_BTMGR_POPULATE_LISTS:
        {
            PopulateLists();
            SetEvent(ghListsPopulated);
        }

		case WM_COMMAND:
		{
			int iID = LOWORD(wParam);
			switch (iID)
			{
				// move to trusted after verifying the PIN
				case IDC_MOVE_TO_TRUSTED:
				{
				    LVITEM lvi;
				    WCHAR szText[MAX_NAME];
					if (ERROR_SUCCESS == GetSelectedListItem(ghUntrustedList, &lvi, szText, MAX_NAME)) {
    					// if PIN is verified then add to the trusted list & remove from the untrusted list
						DWORD dwStatus = StopInquiryAndPair();
						if(dwStatus == STATUS_AUTHENTICATE || dwStatus == STATUS_NOAUTHENTICATE){
							BTDEVICE* pbtTemp = (BTDEVICE*) lvi.lParam;
							DeleteListItem(ghUntrustedList, pbtTemp);
    						pbtTemp->fTrusted = TRUE;
							lvi.iItem = 0;
							ListView_InsertItem(ghTrustedList, &lvi);
							ListView_SetColumnWidth(ghUntrustedList, 0, LVSCW_AUTOSIZE);
							ListView_SetColumnWidth(ghTrustedList, 0, LVSCW_AUTOSIZE);
							BthEnumUpdate(pbtTemp);							
							ASSERT (Reference (pbtTemp) == 1);
						}
						else if(dwStatus == STATUS_AUTHERROR) {
							MessageBox (hWnd, gszErrAuthFailed, gszError, MB_OK | MB_TOPMOST);
						}
						else if(dwStatus == STATUS_AUTHERROR_CONNECT) {
							MessageBox (hWnd, gszErrAuthFailedCantConnect, gszError, MB_OK | MB_TOPMOST);
						}
						else if(dwStatus == STATUS_AUTHERROR_NODRIVER) {
							MessageBox (hWnd, gszErrAuthFailedNoDriver, gszError, MB_OK | MB_TOPMOST);
						}
					}
				}
				return 0;
				//MOVE THE SELECTED ITEM TO THE UNTRUSTED LIST	
				case IDC_MOVE_TO_UNTRUSTED:
				{
					LVITEM lvi;
					WCHAR szText[MAX_NAME];
					if (ERROR_SUCCESS == GetSelectedListItem(ghTrustedList, &lvi, szText, MAX_NAME)) {
						BTDEVICE* pbtTemp = (BTDEVICE*)lvi.lParam;
						pbtTemp->fTrusted = FALSE;
						//make the device inactive before moving it to the other list
						if(pbtTemp->fActive) {
							BthEnumDeactivate(pbtTemp);
							lvi.iImage = pbtTemp->fActive ? pbtTemp->iDeviceIconA : pbtTemp->iDeviceIconI;
						}

						DeleteListItem(ghTrustedList, pbtTemp);
					    lvi.iItem = 0;
					    ListView_InsertItem(ghUntrustedList, &lvi);
					    ListView_SetColumnWidth(ghUntrustedList, 0, LVSCW_AUTOSIZE);
					    ListView_SetColumnWidth(ghTrustedList, 0, LVSCW_AUTOSIZE);
					    BthEnumRemove(pbtTemp);
					    RemovePINLink(pbtTemp);

					    ASSERT (Reference (pbtTemp) == 1);
					}
				}
				return 0;
				//BEGIN A NEW DISCOVERY SESSION & CREATE REFREASHED LISTS
				case IDC_SCAN_DEVICE:
				{
					if (ghInquiryHandle) {
						HANDLE h = ghInquiryHandle;
						ghInquiryHandle = NULL;
						g_pfnBthNsLookupServiceEnd (h);
					}

					if ((! ghInquiryThread) || (WAIT_OBJECT_0 == WaitForSingleObject (ghInquiryThread, 0))) {
						if (ghInquiryThread)
							CloseHandle (ghInquiryThread);

                        DeleteUntrusted();
						ghInquiryThread = CreateThread (NULL, 0, DoInquiry, NULL, 0, NULL);
					}
				}
				return 0;

				case IDCANCEL:
				{
					EndDialog (hWnd, 0);
				}
				return 0;
				//IF THE USER CLICKS THE MENU TOGGLE THE ACTIVE PROPERTY
				case IDM_ACTIVE:
				{
					LVITEM lvi;
					if (ERROR_SUCCESS != GetSelectedListItem(ghTrustedList, &lvi, NULL, 0))
					    return 0;

					BTDEVICE *pbt = (BTDEVICE*) lvi.lParam;
					
					if(pbt->fActive)
						BthEnumDeactivate(pbt);
					else
						ActivateBthDevice(pbt);

					lvi.iImage = pbt->fActive ? pbt->iDeviceIconA : pbt->iDeviceIconI;
					ListView_SetItem(ghTrustedList, &lvi);

					BthEnumUpdate(pbt);
				}
				return 0;
				case IDM_AUTH:
				{
					LVITEM lvi;
					if (ERROR_SUCCESS != GetSelectedListItem(ghTrustedList, &lvi, NULL, 0))
					    return 0;

					BTDEVICE *pbt = (BTDEVICE*) lvi.lParam;
					
					pbt->fAuth = ! pbt->fAuth;
					BthEnumUpdate(pbt);
				}
				return 0;
				case IDM_ENCRYPT:
				{
					LVITEM lvi;
					if (ERROR_SUCCESS != GetSelectedListItem(ghTrustedList, &lvi, NULL, 0))
					    return 0;

					BTDEVICE *pbt = (BTDEVICE*) lvi.lParam;
					
					pbt->fEncrypt = ! pbt->fEncrypt;
					BthEnumUpdate(pbt);
				}
				return 0;
				case IDM_DELETE:
				{
					LVITEM lvi;
					if (ERROR_SUCCESS == GetSelectedListItem(ghTrustedList, &lvi, NULL, 0)) {
						BTDEVICE* pbt = (BTDEVICE*)lvi.lParam;
						BthEnumDeactivate(pbt);
						if (ListView_DeleteItem(ghTrustedList, lvi.iItem)) {
						    BthEnumRemove(pbt);
						    RemovePINLink (pbt);
						    delete pbt;

						    ASSERT (Reference (pbt) == 0);
						} else {
							DEBUGMSG(1, (L"NETUI: failed deleting item %d, GLE=%d\r\n", lvi.iItem, GetLastError ()));
							ASSERT(0);
						}

						return 0;
					}
				}
				return 0;

			}
		}
		return 0;
		case WM_KEYDOWN:
		{	
			//is supposed to capture the delete key but somehow is not doing that right now 
			LV_KEYDOWN *plvkd = (LV_KEYDOWN *)lParam;
		
			switch (plvkd->wVKey) 
			{
				case VK_DELETE:
				{
					SendMessage (hWnd, WM_COMMAND, IDM_DELETE, 0);
					break;
				}
			}					
			return 0;
		}
		return 0;

		case WM_NOTIFY:
		{
			NM_LISTVIEW *pnm = (NM_LISTVIEW *)lParam;
			switch (pnm->hdr.code)
			{
#if defined (GN_CONTEXTMENU)
				case GN_CONTEXTMENU:
					CreateActiveMenu (hWnd, FALSE);
					return 0;
#endif

				case PSN_HELP:
				{
					PROCESS_INFORMATION pi = {0, 0};
					WCHAR szCmdLine[50];
					wcscpy (szCmdLine, L"file:ctpnl.htm#bluetooth");
					if (CreateProcess(L"peghelp", szCmdLine, 0,0,0,0,0,0,0,&pi))
					{
						CloseHandle(pi.hThread);
				       		CloseHandle(pi.hProcess);
					}
				}
				return 0;
				//GENERATE THE POPUP ACTIVE PROPERTY MENU
				case NM_RCLICK:
				case NM_DBLCLK:
				{
					int iWParam = LOWORD(wParam);
					if (iWParam == IDC_TRUSTED)
						CreateActiveMenu (hWnd, FALSE);
				}		
				return 0;
				case NM_SETFOCUS:
				{
					int iWParam = LOWORD(wParam);
					switch (iWParam)
					{
						case IDC_TRUSTED:
						{
							int i = ListView_GetNextItem(ghUntrustedList, -1, LVNI_SELECTED);	
							if(i != -1)
								ListView_SetItemState(ghUntrustedList, i, 0 , LVIS_SELECTED);
						}
						return 0;
						case IDC_UNTRUSTED:
						{
							int i = ListView_GetNextItem(ghTrustedList, -1, LVNI_SELECTED);	
							if(i != -1)
								ListView_SetItemState(ghTrustedList, i, 0 , LVIS_SELECTED);
						}
					}
				}				
				return 0;				
			}
		}
		return 0;
		//deleted the list views 
		case WM_DESTROY:
		{
			RETAILMSG(0, (L"DlgProcScanDC +WM_DESTROY\r\n"));
			////////////
			g_TotalStop = 1;
			SetEvent(ghListsPopulated);
			if (ghInquiryHandle) 
			{
				int ret = 0;
				HANDLE h = ghInquiryHandle;
				ghInquiryHandle = NULL;
				ret = g_pfnBthNsLookupServiceEnd(h);//((HANDLE)BTHNS_ABORT_CURRENT_INQUIRY);// 
				RETAILMSG(0, (L"CreateScanDevice g_pfnBthNsLookupServiceEnd h %x, ret %d, err %d\r\n", ghInquiryHandle, ret, WSAGetLastError()));
			}

			if (ghInquiryThread) 
			{
				RETAILMSG(0,(L"CreateScanDevice ghInquiryThread\r\n"));
				WaitForSingleObjectMessageLoop (ghInquiryThread, 5000);
				CloseHandle (ghInquiryThread);
			}
			////////////////////////////
			//delete image list
			ImageList_Destroy(ghIconList);
			ghIconList = NULL;
					
			DeleteObject(ListView_GetImageList(ghTrustedList, LVSIL_SMALL));
			DeleteObject(ListView_GetImageList(ghUntrustedList, LVSIL_SMALL));
			//delete trusted list
			int itotalItems = ListView_GetItemCount(ghTrustedList);
			for(int i=0; i<itotalItems; i++) {
				LV_ITEM lvi;
				memset (&lvi, 0, sizeof(lvi));
				lvi.mask = LVIF_PARAM;
				lvi.stateMask = -1;
				lvi.iItem = 0;
				ListView_GetItem(ghTrustedList, &lvi);
				if (ListView_DeleteItem(ghTrustedList, 0)) {
				    BTDEVICE* pbtTemp = (BTDEVICE*) lvi.lParam;				
				    delete pbtTemp;							

				    ASSERT (Reference (pbtTemp) == 0);
				} else {
					DEBUGMSG(1, (L"NETUI: failed deleting item %d, GLE=%d\r\n", 0, GetLastError ()));
					ASSERT(0);
				}
			}		
			//delete untrusted list
			itotalItems = ListView_GetItemCount(ghUntrustedList);
			for( i=0; i<itotalItems; i++) {
				LV_ITEM lvi;
				memset (&lvi, 0, sizeof(lvi));
				lvi.mask = LVIF_PARAM;
				lvi.stateMask = -1;
				lvi.iItem = 0;
				ListView_GetItem(ghUntrustedList, &lvi);
				if (ListView_DeleteItem(ghUntrustedList, 0)) {
				    BTDEVICE* pbtTemp = (BTDEVICE*) lvi.lParam;			
				    delete pbtTemp;							

				    ASSERT (Reference (pbtTemp) == 0);
				} else {
					DEBUGMSG(1, (L"NETUI: failed deleting item %d, GLE=%d\r\n", 0, GetLastError ()));
					ASSERT(0);
				}
			}		
			RETAILMSG(0,(L"DlgProcScanDC -WM_DESTROY\r\n"));
		}
		return 0;
	}
	return 0;		
}
////////////////////////////////////////////////////////////////////////////////////////
//Dialog Proc for the potrait dialog with only one list
////////////////////////////////////////////////////////////////////////////////////////
static BOOL CALLBACK DlgProcScanDCP (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) 
	{
		case WM_INITDIALOG:
		{
			ghWnd = hWnd;
			//initialize the image lists & the list itself
			SetForegroundWindow (hWnd);
			ghTrustedList = GetDlgItem(hWnd, IDC_TRUSTED);
			LoadIcons();
			ListView_SetImageList(ghTrustedList, ghIconList, LVSIL_SMALL);
			ListView_SetExtendedListViewStyle(ghTrustedList, LVS_EX_SUBITEMIMAGES | LVS_EX_FULLROWSELECT);
			CreateLists();
		}
		return 0;

        case WM_BTMGR_POPULATE_LISTS:
        {
            PopulateLists();
            SetEvent(ghListsPopulated);
        }

		case WM_COMMAND:
		{
			int wID = LOWORD(wParam);
			switch (wID)
			{			
				//start a new device discovery session
			case IDC_SCAN_DEVICE:
				{
					if (ghInquiryHandle) {
						HANDLE h = ghInquiryHandle;
						ghInquiryHandle = NULL;
						g_pfnBthNsLookupServiceEnd (h);
					}

					if ((! ghInquiryThread) || (WAIT_OBJECT_0 == WaitForSingleObject (ghInquiryThread, 0))) {
						if (ghInquiryThread)
							CloseHandle (ghInquiryThread);

                        DeleteUntrusted();
						ghInquiryThread = CreateThread (NULL, 0, DoInquiry, NULL, 0, NULL);
					}
				}
				return 0;

				case IDCANCEL:
				{
					EndDialog (hWnd, 0);
				}
				return 0;
				//opens the PIN dialog to verify the PIN & if verified, change its status & the corresponding icon
				case IDM_TRUSTED:
				{
					LVITEM lvi;
					if (ERROR_SUCCESS != GetSelectedListItem(ghTrustedList, &lvi, NULL, 0))
					    return 0;
					BTDEVICE* pbtTemp = (BTDEVICE*) lvi.lParam;
					//if the device was previously trusted make it untrusted
					if(pbtTemp->fTrusted){
						pbtTemp->fTrusted = FALSE;
						//if the device was active first make it inactive
						if(pbtTemp->fActive) {
							//change active icon
							LV_ITEM lvi_active;
							memset (&lvi_active, 0, sizeof(lvi_active));
							lvi_active.mask = LVIF_IMAGE;
							lvi_active.iSubItem = 1;
							lvi.stateMask = -1;
							lvi_active.iItem = lvi.iItem;
							//only if we can deregister the device can we make it inactive
							BthEnumDeactivate(pbtTemp);
							ListView_GetItem(ghTrustedList, &lvi_active);
							lvi_active.iImage = pbtTemp->fActive ? pbtTemp->iDeviceIconA : pbtTemp->iDeviceIconI;
							ListView_SetItem(ghTrustedList, &lvi_active);
						}
						pbtTemp->fActive = FALSE;
						//update in list
						lvi.iImage = IMAGE_UNTRUSTED;
						BthEnumRemove(pbtTemp);
						RemovePINLink(pbtTemp);
					}
					//if the device was untrusted make it trusted
					else {
						DWORD dwStatus = StopInquiryAndPair();
						if(dwStatus == STATUS_AUTHENTICATE || dwStatus == STATUS_NOAUTHENTICATE){
							pbtTemp->fTrusted = TRUE;
							lvi.iImage = IMAGE_TRUSTED;
							BthEnumUpdate(pbtTemp);
						}
						else if(dwStatus == STATUS_AUTHERROR) {
							MessageBox (hWnd, gszErrAuthFailed, gszError, MB_OK | MB_TOPMOST);
						}
						else if(dwStatus == STATUS_AUTHERROR_CONNECT) {
							MessageBox (hWnd, gszErrAuthFailedCantConnect, gszError, MB_OK | MB_TOPMOST);
						}
						else if(dwStatus == STATUS_AUTHERROR_NODRIVER) {
							MessageBox (hWnd, gszErrAuthFailedNoDriver, gszError, MB_OK | MB_TOPMOST);
						}
					}
					ListView_SetItem(ghTrustedList, &lvi);
					ListView_SetColumnWidth(ghTrustedList, 1, LVSCW_AUTOSIZE);
				}
				return 0;
				//makes one of the trusted devices active but this menu will only be enabled if a device is trusted, same rules apply as above dlg
				case IDM_ACTIVE:
				{
					LVITEM lvi;
					if (ERROR_SUCCESS != GetSelectedListItem(ghTrustedList, &lvi, NULL, 0))
					    return 0;
					BTDEVICE *pbt = (BTDEVICE*) lvi.lParam;

					if(pbt->fActive)
						BthEnumDeactivate(pbt);
					else
						ActivateBthDevice(pbt);

					//change active icon
					LV_ITEM lvi_active;
					memset (&lvi_active, 0, sizeof(lvi_active));
					lvi_active.mask = LVIF_IMAGE;
					lvi_active.iSubItem = 1;
					lvi.stateMask = -1;
					lvi_active.iItem = lvi.iItem;
					ListView_GetItem(ghTrustedList, &lvi_active);
					lvi_active.iImage = pbt->fActive ? pbt->iDeviceIconA : pbt->iDeviceIconI;

					ListView_SetItem(ghTrustedList, &lvi_active);

					BthEnumUpdate(pbt);	
				}
				return 0;

				case IDM_AUTH:
				{
					LVITEM lvi;
					if (ERROR_SUCCESS != GetSelectedListItem(ghTrustedList, &lvi, NULL, 0))
					    return 0;

					BTDEVICE *pbt = (BTDEVICE*) lvi.lParam;
					
					pbt->fAuth = ! pbt->fAuth;
					BthEnumUpdate(pbt);
				}
				return 0;
				case IDM_ENCRYPT:
				{
					LVITEM lvi;
					if (ERROR_SUCCESS != GetSelectedListItem(ghTrustedList, &lvi, NULL, 0))
					    return 0;

					BTDEVICE *pbt = (BTDEVICE*) lvi.lParam;
					
					pbt->fEncrypt = ! pbt->fEncrypt;
					BthEnumUpdate(pbt);
				}
				return 0;
				case IDM_DELETE:
				{
					LVITEM lvi;
					if (ERROR_SUCCESS == GetSelectedListItem(ghTrustedList, &lvi, NULL, 0)) {
						BTDEVICE* pbtTemp = (BTDEVICE*) lvi.lParam;
						BthEnumDeactivate(pbtTemp);
						DeleteListItem(ghTrustedList, pbtTemp);
					    BthEnumRemove(pbtTemp);
					    RemovePINLink (pbtTemp);
					    delete pbtTemp;

					    ASSERT (Reference (pbtTemp) == 0);

						return 0;
					}
				}
				return 0;
			}
		}
		return 0;
		case WM_NOTIFY:
		{
			NM_LISTVIEW *pnm = (NM_LISTVIEW *)lParam;
			switch (pnm->hdr.code)
			{
#if defined (GN_CONTEXTMENU)
				case GN_CONTEXTMENU:
					CreateActiveMenu(hWnd, TRUE);
					return 0;
#endif

				case PSN_HELP:
				{
					PROCESS_INFORMATION pi = {0, 0};
					WCHAR szCmdLine[50];
					wcscpy (szCmdLine, L"file:ctpnl.htm#bluetooth");
					if (CreateProcess(L"peghelp", szCmdLine, 0,0,0,0,0,0,0,&pi))
					{
						CloseHandle(pi.hThread);
 				       		CloseHandle(pi.hProcess);
					}
				}
				return 0;
				//right mouse to generate the popup menu which is used for authenticating a device & making it active
				case NM_DBLCLK:
				case NM_RCLICK:
				{
					int iWParam = LOWORD(wParam);
					if (iWParam == IDC_TRUSTED)
						CreateActiveMenu(hWnd, TRUE);
				}
				return 0;	
			}
		}
		return 0;
		//delete the list
		case WM_DESTROY:
		{
			//delete image list
			ImageList_Destroy(ghIconList);
			ghIconList = NULL;

			DeleteObject(ListView_GetImageList(ghTrustedList, LVSIL_SMALL));
			//delete trusted list
			int itotalItems = ListView_GetItemCount(ghTrustedList);
			for(int i=0; i<itotalItems; i++) {
				LV_ITEM lvi;
				memset (&lvi, 0, sizeof(lvi));
				lvi.mask = LVIF_PARAM;
				lvi.stateMask = -1;
				lvi.iItem = 0;
				ListView_GetItem(ghTrustedList, &lvi);
				if (ListView_DeleteItem(ghTrustedList, 0)) {
				    BTDEVICE* pbtTemp = (BTDEVICE*) lvi.lParam;				
				    delete pbtTemp;							

				    ASSERT (Reference (pbtTemp) == 0);
				} else {
					DEBUGMSG(1, (L"NETUI: failed deleting item %d, GLE=%d\r\n", 0, GetLastError ()));
					ASSERT(0);
				}
			}		
		}
		return 0;
	}
	return 0;		
}
////////////////////////////////////////////////////////////////////////////////////////
// dialog proc for the validate PIN dialog
////////////////////////////////////////////////////////////////////////////////////////
static BOOL CALLBACK DlgProcEnterPin (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
	case WM_INITDIALOG:
		{
			SetForegroundWindow (hWnd);		
			SetFocus (GetDlgItem (hWnd, IDC_PIN));
#ifdef SDK_BUILD
			SipShowIM(SIPF_ON);
#endif
		}
		return 0;

	case WM_DESTROY:
		{
#ifdef SDK_BUILD
			SipShowIM(SIPF_OFF);
#endif
		}

	case WM_COMMAND:
		{
			int wID = LOWORD(wParam);
			switch (wID)
			{
				//validate the PIN ... return 0 instead in end dialog
				case IDC_OK:
				{
					WCHAR szPIN[64];
					SetForegroundWindow (hWnd);		
					GetDlgItemText(hWnd, IDC_PIN, szPIN, 64);
					EndDialog (hWnd, (int)_wcsdup (szPIN));
					return 0;
				}				

				case IDC_CANCEL:
					EndDialog (hWnd, 0);
					return 0;

				case IDCANCEL:
					EndDialog (hWnd, 0);
					return 0;
			}
		}
	}

	return 0;
}

static int GetSelectedListItem(HWND hwndList, LVITEM* plvi, WCHAR* szTextBuffer, int cbTextBuffer)
{
    int iErr = ERROR_SUCCESS;
   
	memset (plvi, 0, sizeof(LVITEM));
	plvi->mask = LVIF_PARAM | LVIF_IMAGE;
	plvi->stateMask = -1;
    if (cbTextBuffer > 0)
    {
        plvi->mask |= LVIF_TEXT;
        plvi->pszText = szTextBuffer;
        plvi->cchTextMax = cbTextBuffer;
    }
	plvi->iItem = ListView_GetNextItem(hwndList, -1, LVNI_SELECTED);
	if(plvi->iItem == -1) 
	{
	    iErr = ERROR_NOT_FOUND;
		goto exit;
	}
	ListView_GetItem(hwndList, plvi);

exit:
	return iErr;
}

////////////////////////////////////////////////////////////////////////////////////////
//creates the list, depending on which dialog is being created
////////////////////////////////////////////////////////////////////////////////////////
static int CreateLists(){
	LV_COLUMN lvc;
	RECT rect;
	int iNum = 0;
	
	lvc.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_WIDTH;
	lvc.iSubItem = 0;
	
       GetWindowRect(ghTrustedList, &rect);
       lvc.cx = rect.right - rect.left + 10;			
	lvc.fmt = LVCFMT_IMAGE ;
	if(gfPortrait) {
		lvc.iSubItem = 0;
		lvc.cx = 20;
		if (ListView_InsertColumn(ghTrustedList, iNum, &lvc) == -1) {
			return -1;
	       }	
		lvc.iSubItem = 1;
		lvc.cx = rect.right - rect.left + 10 - 20;	
		iNum = 1;
	}
	lvc.mask = LVCF_FMT | LVCF_SUBITEM;
	if (ListView_InsertColumn(ghTrustedList, iNum, &lvc) == -1) {
        return -1;
	}
	if(!gfPortrait) {
		if (ListView_InsertColumn(ghUntrustedList, iNum, &lvc) == -1) {
			return -1;
		}			
	}	
	return PopulateLists();	
}
////////////////////////////////////////////////////////////////////////////////////////
//populates the lists keeping in mind which dialog is being used
////////////////////////////////////////////////////////////////////////////////////////
static int PopulateLists(){
	
	LV_ITEM lvi;
	int iuntst =0;
	int itst = 0;
	for(DWORD i=0; i<gdwNoOfDevice; i++) {
		memset (&lvi, 0, sizeof(lvi));
	    lvi.mask = LVIF_TEXT | LVIF_PARAM | LVIF_IMAGE;
		lvi.stateMask = -1;		
		lvi.pszText = gpbtList[i]->szDeviceName;
		lvi.cchTextMax = lstrlen(gpbtList[i]->szDeviceName);
		lvi.iImage = gpbtList[i]->fActive ? gpbtList[i]->iDeviceIconA : gpbtList[i]->iDeviceIconI;
		lvi.lParam = (LPARAM)gpbtList[i];

		if(gpbtList[i]->fTrusted || gfPortrait) {
			lvi.iItem = itst;
			ListView_InsertItem(ghTrustedList, &lvi);
			if(gfPortrait) {
				lvi.iSubItem = 0;
				lvi.mask = LVIF_IMAGE | LVIF_PARAM | LVIF_TEXT;
				if(gpbtList[i]->fTrusted)
					lvi.iImage = IMAGE_TRUSTED;
				else
					lvi.iImage = IMAGE_UNTRUSTED;
				ListView_SetItem(ghTrustedList, &lvi);
				lvi.mask = LVIF_TEXT | LVIF_IMAGE;
				lvi.iSubItem = 1;
				lvi.iImage = gpbtList[i]->fActive ? gpbtList[i]->iDeviceIconA : gpbtList[i]->iDeviceIconI;
			}
			ListView_SetItem(ghTrustedList, &lvi);			
			itst++;
		}
		else {
			lvi.iItem = iuntst;
			ListView_InsertItem(ghUntrustedList, &lvi);
			ListView_SetItem(ghUntrustedList, &lvi);
			iuntst++;
		}
		
	}
	if(!gfPortrait) {
		ListView_SetColumnWidth(ghUntrustedList, 0, LVSCW_AUTOSIZE);
		ListView_SetColumnWidth(ghTrustedList, 0, LVSCW_AUTOSIZE);
	} else {
		ListView_SetColumnWidth(ghTrustedList, 1, LVSCW_AUTOSIZE);
	}
	return 1;
}
////////////////////////////////////////////////////////////////////////////////////////
//loads the icons used. right now i have used dummy placeholder icons... later will replace with actual
////////////////////////////////////////////////////////////////////////////////////////
static HIMAGELIST LoadIcons() {
	if (!ghIconList)
	{
		ghIconList = ImageList_Create(16, 16, ILC_COLOR, 0, 5);

		for (int i = 0 ; i < sizeof(gszIconList)/sizeof(gszIconList[0]) ; ++i) {
			HICON hIcon = (HICON)LoadImage(ghInst, MAKEINTRESOURCE(gszIconList[i]), IMAGE_ICON, 16, 16, 0);	
			ImageList_AddIcon(ghIconList, hIcon);
			DestroyIcon(hIcon);
		}
	}
	
	return ghIconList;
}

////////////////////////////////////////////////////////////////////////////////////////
//create popup menu for landscape dialog
////////////////////////////////////////////////////////////////////////////////////////
static int CreateActiveMenu(HWND hwndParent, int fPortrait) {
   	HMENU hMenu, hMenuTrack = NULL;
	LV_ITEM lvi;
	memset (&lvi, 0, sizeof(lvi));
       lvi.mask = LVIF_PARAM;
	lvi.stateMask = -1;
	// Get first item selected.
	lvi.iItem = ListView_GetNextItem(ghTrustedList, -1, LVIS_FOCUSED);
	if(lvi.iItem == -1) 
		return 0;
	ListView_GetItem(ghTrustedList, &lvi);
	BTDEVICE* pbtTemp = (BTDEVICE*) lvi.lParam;

	// Background menu
	hMenu = LoadMenu(ghInst, MAKEINTRESOURCE(fPortrait ? IDM_ACTIVEMENU_P : IDM_ACTIVEMENU));
	//bug fix 30285
	if(hMenu) {
		hMenuTrack = GetSubMenu(hMenu, 0);
		if(hMenuTrack) {
			if (pbtTemp->iDeviceClass == -1) {
				EnableMenuItem(hMenu, IDM_ACTIVE, MF_GRAYED);
			}

			if (! pbtTemp->fTrusted) {
				EnableMenuItem (hMenu, IDM_ACTIVE, MF_GRAYED);
				EnableMenuItem (hMenu, IDM_AUTH, MF_GRAYED);
				EnableMenuItem (hMenu, IDM_ENCRYPT, MF_GRAYED);
			} else {
				if(pbtTemp->fActive)
					CheckMenuItem( hMenu, IDM_ACTIVE , MF_CHECKED);

				if(pbtTemp->fAuth)
					CheckMenuItem( hMenu, IDM_AUTH , MF_CHECKED);

				if(pbtTemp->fEncrypt)
					CheckMenuItem( hMenu, IDM_ENCRYPT , MF_CHECKED);

				if (fPortrait)
					CheckMenuItem( hMenu, IDM_TRUSTED , MF_CHECKED);

				// these can only be changed when device is not active
				// because the device needs to be recreated with new parameters
				if (pbtTemp->fActive && pbtTemp->hDevHandle) {
					EnableMenuItem (hMenu, IDM_AUTH, MF_GRAYED);
					EnableMenuItem (hMenu, IDM_ENCRYPT, MF_GRAYED);
				}
			}

			// get click-location
			DWORD dwPoint = GetMessagePos(); 
  
			TrackPopupMenu(hMenuTrack, TPM_LEFTALIGN|TPM_TOPALIGN, LOWORD(dwPoint), HIWORD(dwPoint), 0, hwndParent, NULL);

		}
		DestroyMenu(hMenu);
	}
	return 0;

}

////////////////////////////////////////////////////////////////////////////////////////
//read in from registry any devices that were previously trusted
////////////////////////////////////////////////////////////////////////////////////////
static void SetDeviceIcons (BTDEVICE *pbt) {
	switch (pbt->iDeviceClass) {
	case BTENUM_DEVICE_MODEM:
		pbt->iDeviceIconI = IMAGE_MODEM;
		pbt->iDeviceIconA = IMAGE_MODEM_A;
		break;
	case BTENUM_DEVICE_PRINTER:
		pbt->iDeviceIconI = IMAGE_PRINTER;
		pbt->iDeviceIconA = IMAGE_PRINTER_A;
		break;
	case BTENUM_DEVICE_LAP:
		pbt->iDeviceIconI = IMAGE_LAP;
		pbt->iDeviceIconA = IMAGE_LAP_A;
		break;
	case BTENUM_DEVICE_OBEX_FTP:
		pbt->iDeviceIconI = IMAGE_OBEX_FTP;
		pbt->iDeviceIconA = IMAGE_OBEX_FTP_A;
		break;
	case BTENUM_DEVICE_OBEX_OPP:
		pbt->iDeviceIconI = IMAGE_OBEX_OPP;
		pbt->iDeviceIconA = IMAGE_OBEX_OPP_A;
		break;
	case BTENUM_DEVICE_HEADSET:
		pbt->iDeviceIconI = IMAGE_HEADSET;
		pbt->iDeviceIconA = IMAGE_HEADSET_A;
		break;
	case BTENUM_DEVICE_HANDSFREE:
		pbt->iDeviceIconI = IMAGE_HF;
		pbt->iDeviceIconA = IMAGE_HF_A;
		break;
	case BTENUM_DEVICE_ASYNC:
		pbt->iDeviceIconI = IMAGE_ASYNC;
		pbt->iDeviceIconA = IMAGE_ASYNC_A;
		break;
	case BTENUM_DEVICE_PAN:
		pbt->iDeviceIconI = IMAGE_PAN;
		pbt->iDeviceIconA = IMAGE_PAN_A;
		break;
	case BTENUM_DEVICE_HID:
		if (pbt->dwHidDevClass & 0x40) {
			pbt->iDeviceIconI = IMAGE_KEYBOARD;
			pbt->iDeviceIconA = IMAGE_KEYBOARD_A;
		} else {
			pbt->iDeviceIconI = IMAGE_MOUSE;
			pbt->iDeviceIconA = IMAGE_MOUSE_A;
		}
		break;

	default:
		pbt->iDeviceIconI = IMAGE_QUESTION;
		pbt->iDeviceIconA = IMAGE_QUESTION;
	}
}

static int EnumCallback (void *pContext, BTDEV *pb) {
	gpbtList[gdwNoOfDevice] = new BTDEVICE (pb);
	if (gpbtList[gdwNoOfDevice]) {
		SetDeviceIcons (gpbtList[gdwNoOfDevice]);
		gdwNoOfDevice++;
	}

	return gdwNoOfDevice < DEVICE_MAX;
}


static void CreateDevice (void) {
	gdwNoOfDevice = 0;
	BthEnumDevices (NULL, EnumCallback);

	HKEY hk;
	// Now get the list of authenticated devices which we paired with but did not discover...

	if (ERROR_SUCCESS == RegOpenKeyEx (HKEY_LOCAL_MACHINE, L"software\\microsoft\\bluetooth\\security", 0, KEY_READ, &hk)) {
		DWORD dwIndex = 0;
		for ( ; ; ) {
			WCHAR szValName[3 + 4 + 8 + 1];
			DWORD cValName = sizeof(szValName)/sizeof(szValName[0]);
			unsigned char byLinkorpin[16];
			DWORD dwLinkOrPin = sizeof(byLinkorpin);
			DWORD dwValType = 0;

			int iRes = RegEnumValue (hk, dwIndex, szValName, &cValName, NULL, &dwValType, byLinkorpin, &dwLinkOrPin);

			if (iRes == ERROR_NO_MORE_ITEMS)
				break;

			BT_ADDR b = NULL;
			int fLink = FALSE;
			if ((iRes == ERROR_SUCCESS) && (dwValType == REG_BINARY) && (cValName > 3) && GetBA(szValName + 3, &b) &&
				((dwLinkOrPin == sizeof(byLinkorpin)) && (fLink = (wcsnicmp (szValName, L"key", 3) == 0)))
				|| ((dwLinkOrPin <= sizeof(byLinkorpin)) && (dwLinkOrPin > 0) && (wcsnicmp (szValName, L"pin", 3) == 0)))
            {
				for (DWORD i = 0 ; i < gdwNoOfDevice ; ++i)
                {
					if (gpbtList[i]->b == b)
						break;
				}

				if (i == (DWORD)gdwNoOfDevice) {
					gpbtList[gdwNoOfDevice] = new BTDEVICE (b, -1);
					if(!gpbtList[gdwNoOfDevice])
						continue;
					gpbtList[gdwNoOfDevice]->fTrusted = TRUE;
					wsprintf (gpbtList[gdwNoOfDevice]->szDeviceName, L"%s (%04x%08x)", gszUnnamed, GET_NAP(gpbtList[gdwNoOfDevice]->b), GET_SAP(gpbtList[gdwNoOfDevice]->b));

					BthEnumUpdate(gpbtList[gdwNoOfDevice]);

					gdwNoOfDevice++;
				}
			}

			++dwIndex;
		}

		RegCloseKey (hk);
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//go & inquire for bluetooth devices
////////////////////////////////////////////////////////////////////////////////////////
static DWORD WINAPI DoInquiry (LPVOID lpUnused) {

	HWND hWnd        = ghWnd;
	HWND hWndDevList;

	GUID NULL_GUID;
	memset (&NULL_GUID, 0, sizeof(NULL_GUID));

	/////////////////////////////////////////////
	if(gfPortrait)
		hWndDevList = ghTrustedList;
	else
		hWndDevList = ghUntrustedList;
	HWND hWndButton = GetDlgItem (hWnd, IDC_SCAN_DEVICE);

	SetWindowText (hWndButton, gszStop);
	SetWindowText (GetParent(hWnd), gszInquiryTitle);

	WSAQUERYSET		wsaq;
	memset (&wsaq, 0, sizeof(wsaq));
	wsaq.dwSize      = sizeof(wsaq);
	wsaq.dwNameSpace = NS_BTH;
	wsaq.lpcsaBuffer = NULL;

	HANDLE hLookup = NULL;

	int iErr = g_pfnBthNsLookupServiceBegin (&wsaq, LUP_CONTAINERS , &hLookup);
	RETAILMSG(0,(L"DoInquiry: g_pfnBthNsLookupServiceBegin\r\n"));
	
	ghInquiryHandle = hLookup;

	while ((iErr == ERROR_SUCCESS) && ghInquiryHandle && !g_TotalStop) {
		union {
			CHAR buf[5000];
			double __unused;	// ensure proper alignment
		};

		LPWSAQUERYSET pwsaResults = (LPWSAQUERYSET) buf;
		DWORD dwSize  = sizeof(buf);

		memset(pwsaResults,0,sizeof(WSAQUERYSET));
		pwsaResults->dwSize      = sizeof(WSAQUERYSET);
		pwsaResults->dwNameSpace = NS_BTH;
		pwsaResults->lpBlob      = NULL;

		RETAILMSG(0,(L"DoInquiry: g_pfnBthNsLookupServiceNext\r\n"));
		iErr = g_pfnBthNsLookupServiceNext (hLookup, LUP_RETURN_ADDR | LUP_RETURN_NAME, &dwSize, pwsaResults);
		RETAILMSG(0,(L"DoInquiry: g_pfnBthNsLookupServiceNext h %x, ret %x\r\n", hLookup, iErr));

		if(g_TotalStop)
			break;
		if ((iErr == ERROR_SUCCESS) && (pwsaResults->dwNumberOfCsAddrs == 1)) {
			//if the device we found is not already trusted only then do we query it further
			BT_ADDR b = ((SOCKADDR_BTH *)pwsaResults->lpcsaBuffer->RemoteAddr.lpSockaddr)->btAddr;
			int fHaveTrust = FindTrustedDevice (b) ? TRUE : FALSE;
			int fFoundSomething = FALSE;

			//query the device for all services
			for(int i=0; (i < BTENUM_CLASSES) && ghInquiryHandle && !g_TotalStop; i++) {
				BTDEVICE *pTrusted = FindTrustedOrEmptyService (b, i);
				if (pTrusted && (pTrusted->iDeviceClass != -1))
					continue;

				DWORD subclass = 0;
				unsigned char *psdp = NULL;
				unsigned int   csdp = 0;

				unsigned char channel = 0;
				GUID ServiceId = NULL_GUID;
				if (ERROR_SUCCESS != DoSDP(&b, i, &channel, &psdp, &csdp, &subclass, &ServiceId))
					break;

				if (channel || psdp || (ServiceId != NULL_GUID)) {
					BTDEVICE * pbt;
					if (pTrusted && (pTrusted->iDeviceClass == -1)) {
						pbt = pTrusted;
						DeleteListItem (ghTrustedList, pTrusted);
						BthEnumRemove (pTrusted);
						pTrusted->iDeviceClass = i;
					} else
						pbt = new BTDEVICE (b, i);

					if (pbt) {
						pbt->ucChannel = channel;
						pbt->service_id = ServiceId;
						wsprintf (pbt->szDeviceName, L"%s (%04x%08x)", pwsaResults->lpszServiceInstanceName ? pwsaResults->lpszServiceInstanceName : gszUnnamed, GET_NAP(pbt->b), GET_SAP(pbt->b));
				
						pbt->fTrusted = fHaveTrust;

						pbt->fActive = FALSE;

						pbt->psdp = psdp;
						pbt->csdp = csdp;

						pbt->dwHidDevClass = subclass;

						if (pbt->dwHidDevClass == 0x80) { // Override the default for mice
							pbt->fAuth = FALSE;
							pbt->fEncrypt = FALSE;
						}

						SetDeviceIcons (pbt);

						if (pbt->fTrusted)
							BthEnumUpdate(pbt);

						fFoundSomething = TRUE;

						gpbtList[0] = pbt;
						gdwNoOfDevice = 1;
						PostMessage(ghWnd, WM_BTMGR_POPULATE_LISTS, NULL, NULL);
						WaitForSingleObject(ghListsPopulated, INFINITE);

						ASSERT (Reference (pbt) == 1);
					}
				}
			}

			if ((! (fFoundSomething || fHaveTrust)) && !g_TotalStop ) {	// Do not yet have the device
				BTDEVICE *pbt = new BTDEVICE(b, -1);
				if (pbt) {
					wsprintf (pbt->szDeviceName, L"%s (%04x%08x)", pwsaResults->lpszServiceInstanceName ? pwsaResults->lpszServiceInstanceName : gszUnnamed, GET_NAP(pbt->b), GET_SAP(pbt->b));
					gpbtList[0] = pbt;
					gdwNoOfDevice = 1;
					PostMessage(ghWnd, WM_BTMGR_POPULATE_LISTS, NULL, NULL);
					WaitForSingleObject(ghListsPopulated, INFINITE);
				}
			}
		}
	}

	if ((iErr == SOCKET_ERROR) && ((iErr = GetLastError ()) != WSA_E_NO_MORE) && ghInquiryHandle && !g_TotalStop) {
		SetWindowText (hWndButton, gszScan);
		SetWindowText (GetParent(hWnd), gszCaption);
		WCHAR szString[MAX_LOC_STRING * 2 + 7];
		WCHAR *szErr = L"";
		switch (iErr) {
		case ERROR_SERVICE_NOT_ACTIVE:
		case WSASERVICE_NOT_FOUND:
		case WSAENETDOWN:
			szErr = gszErrHardwareNotPresent;
			break;
		case WSA_E_CANCELLED:
			szErr = gszErrOperationCancelled;
			break;
		case WSAEINVAL:
			szErr = gszErrInvalidOperation;
			break;
		case WSA_NOT_ENOUGH_MEMORY:
			szErr = gszErrOOM;
			break;
		case WSAENOTCONN:
			szErr = gszErrConnectionFailed;
			break;
		}

		wsprintf (szString, L"%s %d %s\n", gszErrHwError, iErr & 0x0000FFFF, szErr);
		MessageBox (hWnd, szString, gszError, MB_OK | MB_TOPMOST);

		RETAILMSG(0,(L"-DoInquiry: err\r\n"));
		return 0;
	}

	if (ghInquiryHandle) {
		ghInquiryHandle = NULL;
		g_pfnBthNsLookupServiceEnd(hLookup);
	}

	if(!g_TotalStop)
	{
		SetWindowText (hWndButton, gszScan);
		SetWindowText (GetParent(hWnd), gszCaption);
	}

	RETAILMSG(0,(L"-DoInquiry:\r\n"));
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////
//authenticated the device but only if we want to authenticate the device
////////////////////////////////////////////////////////////////////////////////////////
static int StopInquiryAndPair (void) {
	HWND hWnd = ghWnd;
	HINSTANCE hInst = ghInst;
	WCHAR *pszPIN = NULL;

	if (ghInquiryHandle) {
		HANDLE h = ghInquiryHandle;
		ghInquiryHandle = NULL;
		g_pfnBthNsLookupServiceEnd (h);
	}

	HWND hWndDevList = gfPortrait ? ghTrustedList : ghUntrustedList;

	/////////////////////////////////////////////

	LVITEM lvi;
	if (ERROR_SUCCESS != GetSelectedListItem(hWndDevList, &lvi, NULL, 0))
	    return 0;

	BT_ADDR b = ((BTDEVICE*)lvi.lParam)->b;

	//if we don't need to authenticate then just leave otherwise continue
	int auth = ifAuthenticate();
	if(auth == STATUS_NOAUTHENTICATE)
		return auth;

	if(auth == STATUS_CANCELAUTHENTICATE)
		return auth;

	//get pin from user
	//bug fix 30191
	pszPIN = (WCHAR *) DialogBox (hInst, MAKEINTRESOURCE(IDD_ENTERPIN), GetParent(hWnd),  DlgProcEnterPin);	

	if (! pszPIN)
		return STATUS_AUTHERROR;

	unsigned char pin[16];
	int cPin = 0;

	//bug ... was trying to delete buffer when at the end of it
	WCHAR *pszTemp = pszPIN;

	while ((*pszTemp) && (cPin < 16))
		pin[cPin++] = (unsigned char)*(pszTemp++);

	free (pszPIN);
	pszTemp = NULL;

	// Set PIN first
	g_pfnBthSetPIN (&b, cPin, pin);

	if (ghInquiryThread) {
		WaitForSingleObjectMessageLoop (ghInquiryThread, INFINITE);
		CloseHandle (ghInquiryThread);
		ghInquiryThread = NULL;
	}

	int iRes = STATUS_AUTHERROR;
	unsigned short h = 0;
	
	if (ERROR_SUCCESS == g_pfnBthBasebandConnect (&b, &h)) {
		iRes = g_pfnBthAuthenticate (&b);

		g_pfnBthBasebandDisconnect (h);
		
		if (iRes != ERROR_SUCCESS) {
			iRes = STATUS_AUTHERROR;
		} else
			iRes = STATUS_AUTHENTICATE;		
	}
	else {
		iRes = STATUS_AUTHERROR_CONNECT;
	}

	return iRes;
}
////////////////////////////////////////////////////////////////////////////////////////
//return the appropriate UUID
////////////////////////////////////////////////////////////////////////////////////////
static USHORT getUuid(int i){
	if (i==BTENUM_DEVICE_MODEM)
		return DialupNetworkingServiceClassID_UUID16;
	else if ((i==BTENUM_DEVICE_PRINTER) || (i==BTENUM_DEVICE_ASYNC))
		return SerialPortServiceClassID_UUID16;
	else if (i==BTENUM_DEVICE_LAP)
		return LANAccessUsingPPPServiceClassID_UUID16;
	else if (i==BTENUM_DEVICE_OBEX_FTP)
		return OBEXFileTransferServiceClassID_UUID16;
	else if (i==BTENUM_DEVICE_OBEX_OPP)
		return OBEXObjectPushServiceClassID_UUID16;
	else if (i==BTENUM_DEVICE_HEADSET)
		return HeadsetServiceClassID_UUID16;
	else if (i==BTENUM_DEVICE_PAN)
		return BNEP_PROTOCOL_UUID16;
    else if (i==BTENUM_DEVICE_HANDSFREE)
        return HandsFreeServiceClassID_UUID16;
//	else if (i==BTENUM_DEVICE_HID)	// HID is not used by DoSDP - it has its own function
//		return HIDServiceClassID_UUID16;

	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////
//get the bluetooth address into pba from the string passed
////////////////////////////////////////////////////////////////////////////////////////
static int GetBA (WCHAR *pp, BT_ADDR *pba) {
	*pba = 0;

	while (*pp == ' ')
		++pp;

	for (int i = 0 ; i < 4 ; ++i, ++pp) {
		if (! iswxdigit (*pp))
			return FALSE;

		int c = *pp;
		if (c >= 'a')
			c = c - 'a' + 0xa;
		else if (c >= 'A')
			c = c - 'A' + 0xa;
		else c = c - '0';

		if ((c < 0) || (c > 16))
			return FALSE;

		*pba = *pba * 16 + c;
	}

	for (i = 0 ; i < 8 ; ++i, ++pp) {
		if (! iswxdigit (*pp))
			return FALSE;

		int c = *pp;
		if (c >= 'a')
			c = c - 'a' + 0xa;
		else if (c >= 'A')
			c = c - 'A' + 0xa;
		else c = c - '0';

		if ((c < 0) || (c > 16))
			return FALSE;

		*pba = *pba * 16 + c;
	}

	if ((*pp != ' ') && (*pp != '\0'))
		return FALSE;

	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////
//query the device for a particular passed service
////////////////////////////////////////////////////////////////////////////////////////
static int DoSDP (BT_ADDR *pb, int id, unsigned char *pcChannel, unsigned char **ppsdp, unsigned int *pcsdp, DWORD *pdwDeviceSubclass, GUID *pServiceId) {
	*pcChannel = 0;
	*ppsdp = NULL;
	*pcsdp = 0;
	*pdwDeviceSubclass = 0;

	g_pfnCoInitializeEx(0, COINIT_MULTITHREADED);

	int iRet = 0;

	for (int i = 0 ; i < (((id == BTENUM_DEVICE_ASYNC) && (*pcChannel == 0)) ? 2 : 1) ; ++i) {
		BTHNS_RESTRICTIONBLOB RBlob;

		memset (&RBlob, 0, sizeof(RBlob));

		RBlob.type = SDP_SERVICE_SEARCH_ATTRIBUTE_REQUEST;
		RBlob.numRange = 1;

        if (id == BTENUM_DEVICE_HID) {
			RBlob.pRange[0].minAttribute = 0;
			RBlob.pRange[0].maxAttribute = 0xffff;

			RBlob.uuids[0].uuidType = SDP_ST_UUID128;
			RBlob.uuids[0].u.uuid128 = HIDServiceClassID_UUID;
		} else if ((id == BTENUM_DEVICE_ASYNC) && (i == 0)) {
			RBlob.pRange[0].minAttribute = 0;
			RBlob.pRange[0].maxAttribute = 0xffff;

			RBlob.uuids[0].uuidType = SDP_ST_UUID128;
			RBlob.uuids[0].u.uuid128 = ActiveSyncServiceClassID_UUID;
		} else if (id == BTENUM_DEVICE_PAN) {
			RBlob.pRange[0].minAttribute = SDP_ATTRIB_CLASS_ID_LIST;
			RBlob.pRange[0].maxAttribute = SDP_ATTRIB_CLASS_ID_LIST;

			RBlob.uuids[0].uuidType = SDP_ST_UUID16;
			RBlob.uuids[0].u.uuid16 = getUuid(id);
		} else if ((id == BTENUM_DEVICE_HEADSET) || (id == BTENUM_DEVICE_HANDSFREE)) {
			RBlob.pRange[0].minAttribute = SDP_ATTRIB_CLASS_ID_LIST;
			RBlob.pRange[0].maxAttribute = SDP_ATTRIB_PROTOCOL_DESCRIPTOR_LIST;

			RBlob.uuids[0].uuidType = SDP_ST_UUID16;
			RBlob.uuids[0].u.uuid16 = getUuid(id);	
		} else {
			RBlob.pRange[0].minAttribute = SDP_ATTRIB_PROTOCOL_DESCRIPTOR_LIST;
			RBlob.pRange[0].maxAttribute = SDP_ATTRIB_PROTOCOL_DESCRIPTOR_LIST;

			RBlob.uuids[0].uuidType = SDP_ST_UUID16;
			RBlob.uuids[0].u.uuid16 = getUuid(id);
		}

		BLOB blob;
		blob.cbSize = sizeof(RBlob);
		blob.pBlobData = (BYTE *)&RBlob;

		SOCKADDR_BTH	sa;

		memset (&sa, 0, sizeof(sa));

		*(BT_ADDR *)(&sa.btAddr) = *pb;
		sa.addressFamily = AF_BT;

		CSADDR_INFO		csai;

		memset (&csai, 0, sizeof(csai));
		csai.RemoteAddr.lpSockaddr = (sockaddr *)&sa;
		csai.RemoteAddr.iSockaddrLength = sizeof(sa);

		WSAQUERYSET		wsaq;

		memset (&wsaq, 0, sizeof(wsaq));
		wsaq.dwSize      = sizeof(wsaq);
		wsaq.dwNameSpace = NS_BTH;
		wsaq.lpBlob      = &blob;
		wsaq.lpcsaBuffer = &csai;

		HANDLE hLookup;
		iRet = g_pfnBthNsLookupServiceBegin (&wsaq, 0, &hLookup);

		if (ERROR_SUCCESS == iRet) {
			union {
				CHAR buf[5000];
				double __unused;
			};

			LPWSAQUERYSET pwsaResults = (LPWSAQUERYSET) buf;
			DWORD dwSize  = sizeof(buf);

			memset(pwsaResults,0,sizeof(WSAQUERYSET));
			pwsaResults->dwSize      = sizeof(WSAQUERYSET);
			pwsaResults->dwNameSpace = NS_BTH;
			pwsaResults->lpBlob      = NULL;

			iRet = g_pfnBthNsLookupServiceNext (hLookup, 0, &dwSize, pwsaResults);
            if (iRet == ERROR_SUCCESS)
            {	// Success - got the stream
                if (!pwsaResults->lpBlob)
                {
                    // safety check, if the blob data is null this is not really valid
                    continue;
                }

                switch (id)
                {
                case BTENUM_DEVICE_HID:
                    {
                        if (VerifyHIDRecord (pwsaResults->lpBlob->pBlobData, pwsaResults->lpBlob->cbSize, pdwDeviceSubclass)) {
                            *ppsdp = (unsigned char *)LocalAlloc (LMEM_FIXED, pwsaResults->lpBlob->cbSize);
                            if (*ppsdp) {
                                memcpy (*ppsdp, pwsaResults->lpBlob->pBlobData, pwsaResults->lpBlob->cbSize);
                                *pcsdp = pwsaResults->lpBlob->cbSize;
                            }
                        }
                    }break;
                    
                case BTENUM_DEVICE_PAN:
                    {
                        FindPANService (pwsaResults->lpBlob->pBlobData, pwsaResults->lpBlob->cbSize, pServiceId);
                    }break;
                    
                case BTENUM_DEVICE_HEADSET:
                case BTENUM_DEVICE_HANDSFREE:
                    {
                        GUID guidSearch;
                        
                        if (id == BTENUM_DEVICE_HEADSET) {
                            memcpy(&guidSearch, &HeadsetServiceClass_UUID, sizeof(GUID));
                        } else if (id == BTENUM_DEVICE_HANDSFREE) {
                            memcpy(&guidSearch, &HandsfreeServiceClass_UUID, sizeof(GUID));
                        }
                        
                        if (ERROR_SUCCESS != FindRFCOMMChannel (pwsaResults->lpBlob->pBlobData,
                            pwsaResults->lpBlob->cbSize, pcChannel, &guidSearch))
                            *pcChannel = 0;
                        else {
                            // Need service class for HS & HF
                            if (id == BTENUM_DEVICE_HEADSET) {
                                memcpy(pServiceId, &HeadsetServiceClass_UUID, sizeof(GUID));
                            } else if (id == BTENUM_DEVICE_HANDSFREE) {
                                memcpy(pServiceId, &HandsfreeServiceClass_UUID, sizeof(GUID));
                            }
                        }
                    }break;
                    
                default:
                    {
                        if (ERROR_SUCCESS != FindRFCOMMChannel (pwsaResults->lpBlob->pBlobData, pwsaResults->lpBlob->cbSize, pcChannel, NULL))
                            *pcChannel = 0;
                    }
                }
            }

			g_pfnBthNsLookupServiceEnd(hLookup);
		}
	}
	g_pfnCoUninitialize();
	return iRet;
}

////////////////////////////////////////////////////////////////////////////////////////
static int VerifyHIDRecord (unsigned char *pStream, int cStream, DWORD *pdwDevSubclass) {
	ISdpRecord **pRecordArg;
	int cRecordArg = 0;

	*pdwDevSubclass = 0;

	if (ERROR_SUCCESS != ServiceAndAttributeSearch (pStream, cStream, &pRecordArg, (ULONG *)&cRecordArg))
		return FALSE;

	int fFound = FALSE;

	for (int i = 0; i < cRecordArg; i++) {
	    ISdpRecord *pRecord = pRecordArg[i];
	    NodeData  nodeAttrib;

        if (ERROR_SUCCESS != pRecord->GetAttribute(SDP_ATTRIB_HID_DEVICE_SUBCLASS, &nodeAttrib))
			continue;

		fFound = TRUE;
        *pdwDevSubclass = nodeAttrib.u.uint8;
		break;
	}

    for (i = 0; i < cRecordArg; i++) 
		pRecordArg[i]->Release();

	g_pfnCoTaskMemFree(pRecordArg);

	return fFound;
}

static HRESULT FindRFCOMMChannel (unsigned char *pStream, int cStream, unsigned char *pChann, GUID* pguidSearch) {
	ISdpRecord **pRecordArg;
	int cRecordArg = 0;
	BOOL bServiceFound = FALSE;

	*pChann = 0;

	HRESULT hr = ServiceAndAttributeSearch (pStream, cStream, &pRecordArg, (ULONG *)&cRecordArg);

	if (FAILED(hr))
		return hr;

	for (int i = 0; (! *pChann) && (i < cRecordArg); i++) {
	    ISdpRecord *pRecord = pRecordArg[i];    // particular record to examine in this loop
	    NodeData  protocolList;     // contains SDP_ATTRIB_PROTOCOL_DESCRIPTOR_LIST data, if available
	    NodeData  ServiceClassList; // contains SDP_ATTRIB_CLASS_ID_LIST data, if available

		// Do we need to compare GUID against service class ID list

		if (pguidSearch) {
	        if (ERROR_SUCCESS != pRecord->GetAttribute(SDP_ATTRIB_CLASS_ID_LIST, &ServiceClassList))
	        {
	            continue;
	        }

	        if (ServiceClassList.type != SDP_TYPE_CONTAINER)
	        {
	            continue;
	        }

	        ISdpNodeContainer *pRecordContainer = ServiceClassList.u.container;
	        int cServices = 0;
	        NodeData ServiceClass;

	        pRecordContainer->GetNodeCount((DWORD *) &cServices);
	        for (int j = 0; j < cServices && !bServiceFound; j++)
	        {
	            pRecordContainer->GetNode(j, &ServiceClass);

	            if (ServiceClass.type != SDP_TYPE_UUID)
	            {
	                continue;
	            }

	            const GUID *    pguidCompare;
	            GUID            guidCompare;

	            if (SDP_ST_UUID128 != ServiceClass.specificType)
	            {
	               // We need to convert this into a 128 bit GUID

	               memcpy(&guidCompare, &Bluetooth_Base_UUID, sizeof(Bluetooth_Base_UUID));

	               if (SDP_ST_UUID16 == ServiceClass.specificType)
	               {
	                   guidCompare.Data1 += ServiceClass.u.uuid16;
	               }
	               else if (SDP_ST_UUID32 == ServiceClass.specificType)
	               {
	                   guidCompare.Data1 += ServiceClass.u.uuid32;
	               }

	               pguidCompare = &guidCompare;
	            }
	            else
	            {
	               pguidCompare = &ServiceClass.u.uuid128;
	            }

	            // Compare the UUID of the ServiceClass to the one we are
	            // looking for. If we find it we are done looking through
	            // Service Classes and move on to trying to figure out
	            // what RFCOMM channel we need to use.

	            if (0 == memcmp(pguidSearch, pguidCompare, sizeof(GUID)))
	            {
	                bServiceFound = TRUE;
	            }
	        }

	        // If we did not find the service we are looking for in
	        // this record don't bother going into the Protocol List.

	        if (!bServiceFound)
	        {
	            continue;
	        }
		}

		if (ERROR_SUCCESS != pRecord->GetAttribute(SDP_ATTRIB_PROTOCOL_DESCRIPTOR_LIST,&protocolList) ||
		    (protocolList.type != SDP_TYPE_CONTAINER))
			continue;
		
		ISdpNodeContainer *pRecordContainer = protocolList.u.container;
		int cProtocols = 0;
		NodeData protocolDescriptor; // information about a specific protocol (i.e. L2CAP, RFCOMM, ...)

		pRecordContainer->GetNodeCount((DWORD *)&cProtocols);
		for (int j = 0; (! *pChann) && (j < cProtocols); j++) {
			pRecordContainer->GetNode(j,&protocolDescriptor);

			if (protocolDescriptor.type != SDP_TYPE_CONTAINER)
				continue;

			ISdpNodeContainer *pProtocolContainer = protocolDescriptor.u.container;
			int cProtocolAtoms = 0;
			pProtocolContainer->GetNodeCount((DWORD *)&cProtocolAtoms);

			for (int k = 0; (! *pChann) && (k < cProtocolAtoms); k++) {
				NodeData nodeAtom;  // individual data element, such as what protocol this is or RFCOMM channel id.

				pProtocolContainer->GetNode(k,&nodeAtom);

				if (IsRfcommUuid(&nodeAtom))  {
					if (k+1 == cProtocolAtoms) {
						// misformatted response.  Channel ID should follow RFCOMM uuid
						break;
					}

					NodeData channelID;
					pProtocolContainer->GetNode(k+1,&channelID);

					*pChann = (unsigned char)GetChannel(&channelID);
					break; // formatting error
				}
			}
		}
	}

    for (i = 0; i < cRecordArg; i++) 
		pRecordArg[i]->Release();

	g_pfnCoTaskMemFree(pRecordArg);

	return (*pChann != 0) ? NO_ERROR : E_FAIL;
}


static HRESULT FindPANService (unsigned char *pStream, int cStream, GUID *pServiceId) {
	ISdpRecord **pRecordArg;
	int cRecordArg = 0;

	int fFound = FALSE;

	HRESULT hr = ServiceAndAttributeSearch (pStream, cStream, &pRecordArg, (ULONG *)&cRecordArg);

	if (FAILED(hr))
		return hr;

	for (int i = 0; (! fFound) && (i < cRecordArg); i++) {
		ISdpRecord *pRecord = pRecordArg[i];    // particular record to examine in this loop
		NodeData  classIdList;     // contains SDP_ATTRIB_CLASS_ID_LIST data, if available

		if (ERROR_SUCCESS != pRecord->GetAttribute(SDP_ATTRIB_CLASS_ID_LIST,&classIdList) ||
			(classIdList.type != SDP_TYPE_CONTAINER))
			continue;
		
		ISdpNodeContainer *pRecordContainer = classIdList.u.container;
		int cClasses = 0;

		pRecordContainer->GetNodeCount((DWORD *)&cClasses);
		for (int j = 0; (! fFound) && (j < cClasses); j++) {
			NodeData classIdDescriptor; // information about a specific protocol (i.e. L2CAP, RFCOMM, ...)
			pRecordContainer->GetNode(j,&classIdDescriptor);

			if (classIdDescriptor.type == SDP_TYPE_UUID) {
				if (classIdDescriptor.specificType == SDP_ST_UUID16) {
					memcpy (pServiceId, &Bluetooth_Base_UUID, sizeof(GUID));
					pServiceId->Data1 += classIdDescriptor.u.uuid16;
					fFound = TRUE;
				} else if (classIdDescriptor.specificType == SDP_ST_UUID32) {
					memcpy (pServiceId, &Bluetooth_Base_UUID, sizeof(GUID));
					pServiceId->Data1 += classIdDescriptor.u.uuid32;
					fFound = TRUE;
				} else if (classIdDescriptor.specificType == SDP_ST_UUID128) {
					memcpy(pServiceId, &classIdDescriptor.u.uuid128, sizeof(GUID));
					fFound = TRUE;
				}
			}
		}
	}

	for (i = 0; i < cRecordArg; i++) 
		pRecordArg[i]->Release();

	g_pfnCoTaskMemFree(pRecordArg);

	return fFound ? NO_ERROR : E_FAIL;
}

////////////////////////////////////////////////////////////////////////////////////////
//	SDP parsing 
////////////////////////////////////////////////////////////////////////////////////////
static HRESULT ServiceAndAttributeSearch(
    UCHAR *szResponse,             // in - response returned from SDP ServiceAttribute query
    DWORD cbResponse,            // in - length of response
    ISdpRecord ***pppSdpRecords, // out - array of pSdpRecords
    ULONG *pNumRecords           // out - number of elements in pSdpRecords
    )
{
    *pppSdpRecords = NULL;
    *pNumRecords = 0;

    ISdpStream *pIStream = NULL;
	
	HRESULT hres = g_pfnCoCreateInstance(__uuidof(SdpStream),NULL,CLSCTX_INPROC_SERVER,
                            __uuidof(ISdpStream),(LPVOID *) &pIStream);

    if (FAILED(hres))
        return hres;  

	ULONG ulError;

    hres = pIStream->Validate (szResponse,cbResponse,&ulError);

    if (SUCCEEDED(hres)) {
        hres = pIStream->VerifySequenceOf(szResponse,cbResponse,
                                          SDP_TYPE_SEQUENCE,NULL,pNumRecords);

        if (SUCCEEDED(hres) && *pNumRecords > 0) {
			*pppSdpRecords = (ISdpRecord **) g_pfnCoTaskMemAlloc(sizeof(ISdpRecord*) * (*pNumRecords));

            if (pppSdpRecords != NULL) {
                hres = pIStream->RetrieveRecords(szResponse,cbResponse,*pppSdpRecords,pNumRecords);

                if (!SUCCEEDED(hres)) {
					g_pfnCoTaskMemFree(*pppSdpRecords);
                    *pppSdpRecords = NULL;
                    *pNumRecords = 0;
                }
            }
            else {
                hres = E_OUTOFMEMORY;
            }
        }
    }

    if (pIStream != NULL) {
        pIStream->Release();
        pIStream = NULL;
    }

	return hres;
}
////////////////////////////////////////////////////////////////////////////////////////
static int IsRfcommUuid(NodeData *pNode)  {
	if (pNode->type != SDP_TYPE_UUID)
		return FALSE;

	if (pNode->specificType == SDP_ST_UUID16)
		return (pNode->u.uuid16 == RFCOMM_PROTOCOL_UUID16);
	else if (pNode->specificType == SDP_ST_UUID32)
		return (pNode->u.uuid32 == RFCOMM_PROTOCOL_UUID16);
	else if (pNode->specificType == SDP_ST_UUID128)
		return (0 == memcmp(&RFCOMM_PROTOCOL_UUID,&pNode->u.uuid128,sizeof(GUID)));

	return FALSE;
}
////////////////////////////////////////////////////////////////////////////////////////
static int GetChannel (NodeData *pChannelNode) {
	if (pChannelNode->specificType == SDP_ST_UINT8)
		return pChannelNode->u.uint8;
	else if (pChannelNode->specificType == SDP_ST_INT8)
		return pChannelNode->u.int8;
	else if (pChannelNode->specificType == SDP_ST_UINT16)
		return pChannelNode->u.uint16;
	else if (pChannelNode->specificType == SDP_ST_INT16)
		return pChannelNode->u.int16;
	else if (pChannelNode->specificType == SDP_ST_UINT32)
		return pChannelNode->u.uint32;
	else if (pChannelNode->specificType == SDP_ST_INT32)
		return pChannelNode->u.int32;

	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////
//returns device if the passed address is already trusted
////////////////////////////////////////////////////////////////////////////////////////
static BTDEVICE *FindTrustedDevice(BT_ADDR b){
	int itotalItems = ListView_GetItemCount(ghTrustedList);
	for(int i=0; i<itotalItems; i++) {
		LV_ITEM lvi;
		memset (&lvi, 0, sizeof(lvi));
	    lvi.mask = LVIF_PARAM;
		lvi.stateMask = -1;
		lvi.iItem = i;
		ListView_GetItem(ghTrustedList, &lvi);
		BTDEVICE* pbtTemp = (BTDEVICE*) lvi.lParam;
		if ((b == pbtTemp->b) && pbtTemp->fTrusted)
			return pbtTemp;
	}

	return NULL;
}

static BTDEVICE *FindTrustedOrEmptyService(BT_ADDR b, int iService) {
	int itotalItems = ListView_GetItemCount(ghTrustedList);
	for(int i=0; i<itotalItems; i++) {
		LV_ITEM lvi;
		memset (&lvi, 0, sizeof(lvi));
	    lvi.mask = LVIF_PARAM;
		lvi.stateMask = -1;
		lvi.iItem = i;
		ListView_GetItem(ghTrustedList, &lvi);
		BTDEVICE* pbtTemp = (BTDEVICE*) lvi.lParam;
		if ((b == pbtTemp->b) && (iService == (int)pbtTemp->iDeviceClass))
			return pbtTemp;
	}

	return (iService == -1) ? NULL : FindTrustedOrEmptyService (b, -1);
}

////////////////////////////////////////////////////////////////////////////////////////
//deletes trusted device but does not free the memory
////////////////////////////////////////////////////////////////////////////////////////
static void DeregisterDeviceClass (int iDeviceClass) {
	int itotalItems = ListView_GetItemCount(ghTrustedList);
	for(int i=itotalItems-1; i>=0; --i) {
		LV_ITEM lvi;
		memset (&lvi, 0, sizeof(lvi));
	    lvi.mask = LVIF_PARAM | LVIF_IMAGE;
		lvi.stateMask = -1;
		lvi.iItem = i;
		ListView_GetItem(ghTrustedList, &lvi);
		BTDEVICE *pbt = (BTDEVICE*) lvi.lParam;
		if (pbt->iDeviceClass == iDeviceClass) {
			if(pbt->fActive)
				BthEnumDeactivate(pbt);

			lvi.iImage = pbt->fActive ? pbt->iDeviceIconA : pbt->iDeviceIconI;
			ListView_SetItem(ghTrustedList, &lvi);

			BthEnumUpdate(pbt);
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//deletes device but does not free the memory
////////////////////////////////////////////////////////////////////////////////////////
static void DeleteListItem (HWND hwndList, BTDEVICE *pbtd) {
	int itotalItems = ListView_GetItemCount(hwndList);
	for(int i=itotalItems-1; i>=0; --i) {
		LV_ITEM lvi;
		memset (&lvi, 0, sizeof(lvi));
	    lvi.mask = LVIF_PARAM;
		lvi.stateMask = -1;
		lvi.iItem = i;
		ListView_GetItem(hwndList, &lvi);
		if ((BTDEVICE*) lvi.lParam == pbtd) {
			ListView_DeleteItem(hwndList, i);
			break;
		}
	}
}



////////////////////////////////////////////////////////////////////////////////////////
//deletes all untrusted devices
////////////////////////////////////////////////////////////////////////////////////////
static void DeleteUntrusted() {
	if(!gfPortrait) {
		int itotalItems = ListView_GetItemCount(ghUntrustedList);
		for(int i=0; i<itotalItems; i++) {
			LV_ITEM lvi;
			memset (&lvi, 0, sizeof(lvi));
			lvi.mask = LVIF_PARAM;
			lvi.stateMask = -1;
			lvi.iItem = 0;
			ListView_GetItem(ghUntrustedList, &lvi);
			BTDEVICE* pbtTemp = (BTDEVICE*) lvi.lParam;
			if (ListView_DeleteItem(ghUntrustedList, 0)) {
			    delete pbtTemp;							

			    ASSERT (Reference (pbtTemp) == 0);
			} else {
				DEBUGMSG(1, (L"NETUI: failed deleting item %d, GLE=%d\r\n", 0, GetLastError ()));
				ASSERT(0);
			}
		}		
		ListView_SetColumnWidth(ghUntrustedList, 0, LVSCW_AUTOSIZE);
	} else {
		int fModified;
		do {
			fModified = FALSE;
			int itotalItems = ListView_GetItemCount(ghTrustedList);

			for(int i=itotalItems-1; i>=0; i--) {	// start from the end so deletion does not shift the targets
				LV_ITEM lvi;
				memset (&lvi, 0, sizeof(lvi));
				lvi.mask = LVIF_PARAM;
				lvi.stateMask = -1;
				lvi.iItem = i;
				ListView_GetItem(ghTrustedList, &lvi);
				BTDEVICE* pbtTemp = (BTDEVICE*) lvi.lParam;
				if (! pbtTemp->fTrusted) {
					if (ListView_DeleteItem(ghTrustedList, i)) {
					    delete pbtTemp;
					    fModified = TRUE;

					    ASSERT (Reference (pbtTemp) == 0);
					} else {
						DEBUGMSG(1, (L"NETUI: failed deleting item %d, GLE=%d\r\n", i, GetLastError ()));
						ASSERT(0);
					}
					break;
				}				
			}
		} while (fModified);

		ListView_SetColumnWidth(ghTrustedList, 1, LVSCW_AUTOSIZE);
	}
}
////////////////////////////////////////////////////////////////////////////////////////
//if device is untrusted then also make its PIN invalid
////////////////////////////////////////////////////////////////////////////////////////
static void RemovePINLink(BTDEVICE* pbtDevice){
	if (! FindTrustedDevice (pbtDevice->b)) {
		g_pfnBthRevokeLinkKey(&(pbtDevice->b));
		g_pfnBthRevokePIN(&(pbtDevice->b));
	}
}
////////////////////////////////////////////////////////////////////////////////////////
//find out from user whether he wants to authenticate a particular device
////////////////////////////////////////////////////////////////////////////////////////
static int ifAuthenticate() {
	DWORD dwResult = 2;
	DWORD dwRet = MessageBox(GetParent(ghWnd), gszReqAuth, gszCaption, MB_ICONQUESTION | MB_YESNOCANCEL);

	if(dwRet == IDYES)
	{
		dwResult = STATUS_AUTHENTICATE;
	}
	else if(dwRet == IDNO)
	{
		dwResult = STATUS_NOAUTHENTICATE;
	}
	else if(dwRet == IDCANCEL)
	{
		dwResult = STATUS_CANCELAUTHENTICATE;
	}

	return dwResult;
}


////////////////////////////////////////////////////////////////////////////////////////
//register the bluetooth device so that other applications can use it
////////////////////////////////////////////////////////////////////////////////////////
static void ActivateBthDevice(BTDEVICE* pbtDevice) {
	if (pbtDevice->iDeviceClass == BTENUM_DEVICE_ASYNC)
		DeregisterDeviceClass (BTENUM_DEVICE_ASYNC);

	if (! BthEnumActivate (pbtDevice, TRUE)) {
		WCHAR szString[MAX_LOC_STRING + 6];
		wsprintf (szString, L"%s %d", gszErrFailComm, GetLastError () & 0x0000FFFF);
		MessageBox (NULL, szString, gszError, MB_OK | MB_TOPMOST);
	}
}
