// boot_i.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "boot_i.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
/*
class CNetworkAdapter {
    public:
        CNetworkAdapter();
        ~CNetworkAdapter();
        BOOL SetupAdapterInfo( IP_ADAPTER_INFO* pAdaptInfo );

            // information about the adapters name for the users
            // and its name to the system
        tstring GetAdapterName() const;
        tstring GetAdapterDescription() const;
        
            // dhcp lease access functions
        time_t    GetLeaseObtained() const;
        time_t    GetLeaseExpired() const;
        
            // access to lists of various server's ip address
        SIZE_T    GetNumIpAddrs() const;        
        SIZE_T    GetNumDnsAddrs() const;
        tstring    GetIpAddr( int nIp = 0 ) const;    
        tstring GetSubnetForIpAddr( int nIp = 0 ) const;
        tstring    GetDnsAddr( int nDns = 0 ) const;        
        tstring GetCurrentIpAddress() const;
        
            // dhcp function
        BOOL    IsDhcpUsed() const;        
        tstring    GetDchpAddr() const;

            // wins function
        BOOL    IsWinsUsed() const;
        tstring GetPrimaryWinsServer() const;
        tstring GetSecondaryWinsServer() const;

        tstring   GetGatewayAddr( int nGateway = DEFAULT_GATEWAY_ADDR) const;
        SIZE_T    GetNumGatewayAddrs() const;

        static    tstring GetAdapterTypeString( UINT nType );
        UINT    GetAdapterType() const;

        DWORD    GetAdapterIndex() const;
        BOOL    ReleaseAddress();
        BOOL    RenewAddress();
        
    protected:
        :
    
    private:
        :
};

DWORD EnumNetworkAdapters( CNetworkAdapter* pAdapters, 
                         ULONG ulSzBuf,LPDWORD lpdwOutSzBuf ) {    
    IP_ADAPTER_INFO* pAdptInfo    = NULL;
    IP_ADAPTER_INFO* pNextAd    = NULL;    
    ULONG ulLen                    = 0;
    int nCnt                    = 0;
    CWinErr erradapt;
    

    erradapt = ::GetAdaptersInfo( pAdptInfo, &ulLen );
    if( erradapt == ERROR_BUFFER_OVERFLOW ) {
        pAdptInfo = ( IP_ADAPTER_INFO* )ALLOCATE_FROM_PROCESS_HEAP( ulLen );
        erradapt = ::GetAdaptersInfo( pAdptInfo, &ulLen );        
    }

    pNextAd = pAdptInfo;
    while( pNextAd ) {
        nCnt++;
        pNextAd = pNextAd->Next;
    }

    *lpdwOutSzBuf = nCnt * sizeof( CNetworkAdapter );
    if( ulSzBuf < *lpdwOutSzBuf ) {        
        DEALLOCATE_FROM_PROCESS_HEAP( pAdptInfo );
        return ERROR_INSUFFICIENT_BUFFER;
    }

        // this needs to be reset for future use.
    nCnt = 0;

    if( erradapt == ERROR_SUCCESS ) {
            // initialize the pointer we use the move through
            // the list.
        pNextAd = pAdptInfo;

            // loop through for all available interfaces and setup an 
            // associated CNetworkAdapter class.
        while( pNextAd ) {
            pAdapters[ nCnt ].SetupAdapterInfo( pNextAd );    
                // move forward to the next adapter in the list so
                // that we can collect its information.
            pNextAd = pNextAd->Next;
            nCnt++;
        }
    }

        // free any memory we allocated from the heap before
        // exit.  we wouldn't wanna leave memory leaks now would we? ;p
    DEALLOCATE_FROM_PROCESS_HEAP( pAdptInfo );        

    return ERROR_SUCCESS;
}
BOOL CNetworkAdapter::SetupAdapterInfo( IP_ADAPTER_INFO* pAdaptInfo ) {
    BOOL bSetupPassed                = FALSE;
    IP_ADDR_STRING* pNext            = NULL;
    IP_PER_ADAPTER_INFO* pPerAdapt    = NULL;
    ULONG ulLen                        = 0;
    CWinErr err;

    _IPINFO iphold;
    
    if( pAdaptInfo ) {        
#ifndef _UNICODE 
        m_sName            = pAdaptInfo->AdapterName;        
        m_sDesc            = pAdaptInfo->Description;
#else
        USES_CONVERSION;
        m_sName            = A2W( pAdaptInfo->AdapterName );
        m_sDesc            = A2W( pAdaptInfo->Description );
#endif

        m_sPriWins        = pAdaptInfo->PrimaryWinsServer.IpAddress.String;
        m_sSecWins        = pAdaptInfo->SecondaryWinsServer.IpAddress.String;
        m_dwIndex        = pAdaptInfo->Index;        
        m_nAdapterType    = pAdaptInfo->Type;    
        m_bDhcpUsed        = pAdaptInfo->DhcpEnabled;
        m_bWinsUsed        = pAdaptInfo->HaveWins;    
        m_tLeaseObtained= pAdaptInfo->LeaseObtained;
        m_tLeaseExpires    = pAdaptInfo->LeaseExpires;
        m_sDhcpAddr        = pAdaptInfo->DhcpServer.IpAddress.String;
        
        if( pAdaptInfo->CurrentIpAddress ) {
           m_sCurIpAddr.sIp = pAdaptInfo->CurrentIpAddress->IpAddress.String;
           m_sCurIpAddr.sSubnet = pAdaptInfo->CurrentIpAddress->IpMask.String;
        }else{
            m_sCurIpAddr.sIp        = _T("0.0.0.0");
            m_sCurIpAddr.sSubnet    = _T("0.0.0.0");
        }


            // since an adapter may have more than one ip address we need
            // to populate the array we have setup with all available
            // ip addresses.
        pNext = &( pAdaptInfo->IpAddressList );
        while( pNext ) {
            iphold.sIp        = pNext->IpAddress.String;
            iphold.sSubnet    = pNext->IpMask.String;
            m_IpAddresses.push_back( iphold );
            pNext = pNext->Next;
        }

         // an adapter usually has just one gateway however the provision 
         // exists for more than one so to "play" as nice as possible 
         // we allow for it here // as well. 
pNext = &( pAdaptInfo->GatewayList ); 
while( pNext ) 
{ m_GatewayList.push_back(pNext->IpAddress.String); 
  pNext = pNext->Next; 
} 
  // we need to generate a IP_PER_ADAPTER_INFO structure in order 
  // to get the list of dns addresses used by this adapter. 
  err = ::GetPerAdapterInfo( m_dwIndex, pPerAdapt, &ulLen ); 
  if( err == ERROR_BUFFER_OVERFLOW ) 
  { 
    pPerAdapt = (IP_PER_ADAPTER_INFO*) 
    ALLOCATE_FROM_PROCESS_HEAP( ulLen ); 
    err = ::GetPerAdapterInfo( m_dwIndex, pPerAdapt, &ulLen ); 
    // if we succeed than we need to drop into our loop 
    // and fill the dns array will all available IP 
    // addresses. 
    if( err == ERROR_SUCCESS ) 
    { 
      pNext = &(pPerAdapt->DnsServerList); 
      while(pNext) 
      { 
       m_DnsAddresses.push_back( pNext->IpAddress.String ); 
       pNext = pNext->Next; 
      } 
      bSetupPassed = TRUE; } 
      // this is done outside the err == ERROR_SUCCES just in case.
      // the macro uses NULL pointer checking so it is ok if 
      // pPerAdapt was never allocated. 
DEALLOCATE_FROM_PROCESS_HEAP(pPerAdapt); 
    } 
  } 
return bSetupPassed; 
}

BOOL CNetworkAdapter::DoRenewRelease(DWORD ( __stdcall *func)
                                ( PIP_ADAPTER_INDEX_MAP AdapterInfo ) ) {    
    IP_INTERFACE_INFO*    pInfo    = NULL;
    BOOL bDidIt                    = FALSE;
    ULONG ulLen                    = 0;
    int nNumInterfaces            = 0;
    int nCnt                    = 0;
    CWinErr err;

    
    err = ::GetInterfaceInfo( pInfo, &ulLen );
    if( err == ERROR_INSUFFICIENT_BUFFER ) {
        pInfo = ( IP_INTERFACE_INFO* ) ALLOCATE_FROM_PROCESS_HEAP( ulLen );
        err = ::GetInterfaceInfo( pInfo, &ulLen );

        if( err != NO_ERROR ) {
            return FALSE;            
        }
    }

        // we can assume from here out that we have a valid array
        // of IP_INTERFACE_INFO structures due to the error 
        // checking one above.
    nNumInterfaces = ulLen / sizeof( IP_INTERFACE_INFO );
    for( nCnt = 0; nCnt < nNumInterfaces; nCnt++ ) {
        if( pInfo[ nCnt ].Adapter[ 0 ].Index == m_dwIndex ) {
            err = func( &pInfo[ nCnt ].Adapter[ 0 ] );
            
                // free all used memory since we don't need it any more.
            DEALLOCATE_FROM_PROCESS_HEAP( pInfo );    
            
            bDidIt = ( err == NO_ERROR );            
            if( ! bDidIt ) {                
                return FALSE;
            }

            break;
        }
    }            

    return bDidIt;
}
*/
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
	LoadString(hInstance, IDC_BOOT_I, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);
	CNetworkAdapter *m_pAdapters = new CNetworkAdapter;
	ULONG ulNeeded = 0;
	EnumNetworkAdapters( m_pAdapters, 0, &ulNeeded );
	delete m_pAdapters;

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_BOOT_I));

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
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_BOOT_I));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_BOOT_I);
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
	static void *srv = (void *)-1;
	static void *quit = (void *)-1;

	switch (message)
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
			case ID_FILE_CONNECT:
			{
				if(srv == (void *)-1)
				{
					quit = CreateEvent(0, 0, 0, 0);
					srv = CreateThread(0, 0, socket_srv, (void *)&sp, 0, 0);
				}
				else
				{
					SetEvent(quit);
					WaitForSingleObject(srv, INFINITE);
					CloseHandle(quit);
					CloseHandle(srv);
					srv = (void *)-1;
				}
				break;
			}
			case IDM_ABOUT:
				DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
				break;
			case IDM_EXIT:
				DestroyWindow(hWnd);
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
	case WM_DESTROY:
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
