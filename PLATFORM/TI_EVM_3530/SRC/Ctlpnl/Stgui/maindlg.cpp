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
#include "StgUI.H"

// termination event
static HANDLE g_hTerminateEvent = NULL;
static HANDLE g_hPNPThread = NULL;

#define THREAD_WAIT_TIME        5000

#define MAX_DEVLEN              8
#define STORAGE_PROFILE_FMT     _T("System\\StorageManager\\Profiles\\%s")
#define STORAGE_PROFILE_NAME    _T("Name")

DWORD WINAPI StoreRefreshThread(LPVOID lpParam);

// --------------------------------------------------------
BOOL GetFriendlyName(LPCTSTR pszStorageProfile, LPTSTR pszFriendlyName, UINT cFriendlyNameLen)
// --------------------------------------------------------
{
    TCHAR szRegPath[MAX_PATH];
    BOOL fRet = FALSE;
    HKEY hKey = NULL;
    DWORD dwRetVal, dwKeyType, dwLen;
    
    ASSERT(pszStorageProfile);
    if(pszStorageProfile)
    {
        StringCbPrintf(szRegPath, sizeof(szRegPath), STORAGE_PROFILE_FMT, pszStorageProfile);
        dwRetVal = RegOpenKeyEx(HKEY_LOCAL_MACHINE, szRegPath, 0, KEY_ALL_ACCESS, &hKey);

        if(ERROR_SUCCESS == dwRetVal && NULL != hKey)
        {
            dwLen = cFriendlyNameLen * sizeof(*pszFriendlyName);
            dwRetVal = RegQueryValueEx(hKey, STORAGE_PROFILE_NAME, NULL, &dwKeyType,
                (PBYTE)pszFriendlyName, &dwLen);

            if(ERROR_SUCCESS == dwRetVal)
            {
                fRet = TRUE;
            }
            dwRetVal = RegCloseKey(hKey);
            ASSERT(ERROR_SUCCESS == dwRetVal);
        }
    }
    return fRet;
}

// --------------------------------------------------------
HANDLE OpenSelectedStore(HWND hDlg)
// --------------------------------------------------------
{
    HANDLE hStore = INVALID_HANDLE_VALUE;
    TCHAR szDisk[MAX_PATH];
    LPTSTR pszDev = NULL;
    INT cIndex = DLG_GETCOMBOSEL(hDlg, IDC_COMBO_DISK);

    if(CB_ERR != cIndex)
    {
        if(CB_ERR != SendDlgItemMessage(hDlg, IDC_COMBO_DISK, CB_GETLBTEXT, (WPARAM)cIndex, (LPARAM)szDisk))
        {
            pszDev = _tcstok(szDisk, TEXT(" "));
            if(pszDev)
            {
                HCURSOR hCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
                hStore = OpenStore(pszDev);
                SetCursor(hCursor);
            }
        }
    }
    return hStore;
}

// --------------------------------------------------------
HANDLE OpenSelectedPart(HWND hDlg)
// --------------------------------------------------------
{
    TCHAR szPart[PARTITIONNAMESIZE] = _T("");
    HANDLE hStore = INVALID_HANDLE_VALUE;
    HANDLE hPart = INVALID_HANDLE_VALUE;
    INT cIndex = DLG_GETLISTSEL(hDlg, IDC_LIST_PARTITIONS);

    if(LB_ERR != cIndex)
    {
        hStore = OpenSelectedStore(hDlg);
        if(INVALID_HANDLE_VALUE != hStore)
        {
            if(LB_ERR != SendDlgItemMessage(hDlg, IDC_LIST_PARTITIONS, LB_GETTEXT, (WPARAM)cIndex, (LPARAM)szPart))
            {
                HCURSOR hCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
                // remove trailing spaces or *
                szPart[_tcslen(szPart) - 2] = (TCHAR)NULL;
                hPart = OpenPartition(hStore, szPart);
                SetCursor(hCursor);
            }
            CloseHandle(hStore);
        }
    }

    return hPart;
}

// --------------------------------------------------------
VOID RefreshStoreList(HWND hDlg)
// --------------------------------------------------------
{
    STOREINFO si = {0};
    HANDLE hFind = INVALID_HANDLE_VALUE;
    DWORD dwCurrentSel = 0;
    TCHAR szFriendlyName[MAX_PATH];
    TCHAR szDisplayName[MAX_PATH];

    // Get currently selected item so item stays selected after refresh
    dwCurrentSel = SendDlgItemMessage(hDlg, IDC_COMBO_DISK, CB_GETCURSEL, 0, 0);
    if (dwCurrentSel == CB_ERR)
        dwCurrentSel = 0;
    
    si.cbSize = sizeof(STOREINFO);

    // remove all entries from dropdown list
    while(CB_ERR != SendDlgItemMessage(hDlg, IDC_COMBO_DISK, CB_DELETESTRING, (WPARAM)0, 0));        

    // enumerate first store
    hFind = FindFirstStore(&si);
    
    if(INVALID_HANDLE_VALUE != hFind)
    {
        do
        {
            // add device name of store to the list
            if(GetFriendlyName(si.sdi.szProfile, szFriendlyName, MAX_PATH))
            {
                _stprintf(szDisplayName, _T("%s %s"), si.szDeviceName, szFriendlyName);
            }  
            else
            {
                _tcscpy(szDisplayName, si.szDeviceName);
            }
            SendDlgItemMessage(hDlg, IDC_COMBO_DISK, CB_ADDSTRING, 0, (LPARAM)szDisplayName);
#if DEBUG
            NKDbgPrintfW(szFriendlyName);
#endif
        }
        while(FindNextStore(hFind, &si));
        FindClose(hFind);

        // set cursor to current selection
        SendDlgItemMessage(hDlg, IDC_COMBO_DISK, CB_SETCURSEL, (WPARAM)dwCurrentSel, 0);
    }
	
    dwCurrentSel = SendDlgItemMessage(hDlg, IDC_COMBO_DISK, CB_GETCURSEL, 0, 0);

    EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_FORMATSTORE), (BOOL)(dwCurrentSel != CB_ERR));
    EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_DISMOUNTSTORE), (BOOL)(dwCurrentSel != CB_ERR));
    EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_NEW), (BOOL)(dwCurrentSel != CB_ERR));
}

// --------------------------------------------------------
VOID RefreshStoreInfo(HWND hDlg)
// --------------------------------------------------------
{
    TCHAR szInfo[MAX_PATH] = _T("");
    TCHAR szFmt[MAX_PATH] = _T("");
    HANDLE hStore = INVALID_HANDLE_VALUE;
    STOREINFO storeInfo = {0};
	BOOL	fEnable = 1;

    storeInfo.cbSize = sizeof(STOREINFO);
    
    hStore = OpenSelectedStore(hDlg);
    if(INVALID_HANDLE_VALUE != hStore)
    {
        GetStoreInfo(hStore, &storeInfo);
        CloseHandle(hStore);
    }

    // update text
    if(Size2String(storeInfo.snNumSectors * storeInfo.dwBytesPerSector, szInfo, MAX_PATH))
        SetDlgItemText(hDlg, IDC_TEXT_CAPACITY, szInfo);

    if(Size2String(storeInfo.snBiggestPartCreatable * storeInfo.dwBytesPerSector, szInfo, MAX_PATH))
        SetDlgItemText(hDlg, IDC_TEXT_UNALLOCATED, szInfo);

    if(Size2String(storeInfo.dwBytesPerSector, szInfo, MAX_PATH))
        SetDlgItemText(hDlg, IDC_TEXT_SECTORSIZE, szInfo);

	RETAILMSG(1,(L"CreateNewPart: 1, %s 2. %s 3. %s\r\n", storeInfo.szDeviceName, storeInfo.szStoreName, storeInfo.sdi.szProfile));  
	if(0 == wcscmp(NAND_DEV_NAME, storeInfo.szDeviceName))
		fEnable = 0;
	EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_FORMATSTORE), fEnable);
	EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_DISMOUNTSTORE), fEnable);
	EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_NEW), fEnable);
//	EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_DELETE), fEnable);
}

// --------------------------------------------------------
VOID RefreshPartitionList(HWND hDlg)
// --------------------------------------------------------
{
    INT cIndex = 0;
    HANDLE hFind = INVALID_HANDLE_VALUE;
    HANDLE hStore = INVALID_HANDLE_VALUE;
    PARTINFO partInfo = {0};
    TCHAR szPartName[MAX_PATH];
	STOREINFO storeInfo = {0};

	storeInfo.cbSize = sizeof(STOREINFO);
    partInfo.cbSize = sizeof(PARTINFO);

    // remove all entries from the list
    while(LB_ERR != SendDlgItemMessage(hDlg, IDC_LIST_PARTITIONS, LB_DELETESTRING, (WPARAM)0, 0));

    if(CB_ERR != cIndex)
    {
        hStore = OpenSelectedStore(hDlg);
        if(INVALID_HANDLE_VALUE != hStore)
        {
	        GetStoreInfo(hStore, &storeInfo);

			hFind = FindFirstPartition(hStore, &partInfo);
            if(INVALID_HANDLE_VALUE != hFind)
            {
                do
                {
                    // append a * to the name if the partition is mounted
                    if(PARTITION_ATTRIBUTE_MOUNTED & partInfo.dwAttributes)
                    {
                        StringCbPrintf(szPartName, sizeof(szPartName), _T("%s *"), partInfo.szPartitionName);
                    }
                    else
                    {
                        StringCbPrintf(szPartName, sizeof(szPartName), _T("%s  "), partInfo.szPartitionName);
                    }
                    szPartName[MAX_PATH-1] = 0;
                    SendDlgItemMessage(hDlg, IDC_LIST_PARTITIONS, LB_ADDSTRING, 0, (LPARAM)szPartName);
                }
                while(FindNextPartition(hFind, &partInfo));
            }

            // set cursor to top item
            SendDlgItemMessage(hDlg, IDC_LIST_PARTITIONS, LB_SETCURSEL, (WPARAM)0, 0);
            CloseHandle(hStore);
        }
    }

    cIndex = SendDlgItemMessage(hDlg, IDC_LIST_PARTITIONS, LB_GETCURSEL, (WPARAM)0, 0);

	if(0 == wcscmp(NAND_DEV_NAME, storeInfo.szDeviceName))
		EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_DELETE), 0);
	else
		EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_DELETE), (BOOL)(cIndex != LB_ERR));
    EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_PROPERTIES), (BOOL)(cIndex != LB_ERR));
}

// --------------------------------------------------------
VOID InitMainDlg(HWND hDlg)
// --------------------------------------------------------
{   
    // some init functions
    
    g_hTerminateEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    g_hPNPThread = CreateThread(NULL, 0, StoreRefreshThread, (LPVOID)hDlg, 0, NULL);
    CenterWindow(hDlg);
    RefreshStoreList(hDlg);
    RefreshStoreInfo(hDlg);
    RefreshPartitionList(hDlg);
}

// --------------------------------------------------------
VOID DeinitMainDlg(HWND hDlg)
// --------------------------------------------------------
{
    SetEvent(g_hTerminateEvent);
    WaitForSingleObject(g_hPNPThread, THREAD_WAIT_TIME);
    CloseHandle(g_hTerminateEvent);
    CloseHandle(g_hPNPThread);    
    EndDialog(hDlg, TRUE);
}

// --------------------------------------------------------
VOID CreateNewPart(HWND hDlg)
// --------------------------------------------------------
{
    NEWPART_INFO param = {0};
    STOREINFO storeInfo = {0};
    HANDLE hStore = INVALID_HANDLE_VALUE;

    storeInfo.cbSize = sizeof(STOREINFO);
    
    hStore = OpenSelectedStore(hDlg);
	if(INVALID_HANDLE_VALUE != hStore)
    {
        // get store info
        HCURSOR hCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
        BOOL fRet = GetStoreInfo(hStore, &storeInfo);
        SetCursor(hCursor);
        if(fRet)
        {
            // default size is the maximum size
            param.dwSize = (DWORD)storeInfo.snBiggestPartCreatable;

            // start new dialog for user input of name/size
            if(DialogBoxParam(g_hInstance, MAKEINTRESOURCE(IDD_PROPPAGE_NEWPART), 
                hDlg, (DLGPROC) NewPartDlgProc, (DWORD)&param))
            {
                hCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
                fRet = CreatePartition(hStore, param.szName, param.dwSize);
                SetCursor(hCursor);
                if(!fRet)
                {
                    ErrBox(hDlg, IDS_ERROR_PARTCREATE, GetLastError());
                }
                RefreshStoreInfo(hDlg);
                RefreshPartitionList(hDlg);
            }
        }
        CloseHandle(hStore);
    }
}

// --------------------------------------------------------
VOID DeleteSelectedPart(HWND hDlg)
// --------------------------------------------------------
{       
    HANDLE hStore = INVALID_HANDLE_VALUE;
    TCHAR szPart[PARTITIONNAMESIZE] = _T("");
    INT cIndex = DLG_GETLISTSEL(hDlg, IDC_LIST_PARTITIONS);
    if(LB_ERR != SendDlgItemMessage(hDlg, IDC_LIST_PARTITIONS, LB_GETTEXT, (WPARAM)cIndex, (LPARAM)szPart))
    {
        // remove trailing spaces or *
        szPart[_tcslen(szPart) - 2] = (TCHAR)NULL;
        hStore = OpenSelectedStore(hDlg);
        if(INVALID_HANDLE_VALUE != hStore)
        {
            // confirm partition delete dialog
            TCHAR szTitle[MAX_PATH], szMsg[MAX_PATH];
            LoadString(g_hInstance, IDS_CONFIRM_DELPART_TITLE, szTitle, MAX_PATH);
            LoadString(g_hInstance, IDS_CONFIRM_DELPART, szMsg, MAX_PATH); 
            if(IDYES == MessageBox(hDlg, szMsg, szTitle, MB_YESNO))
            {
                HCURSOR hCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
                BOOL fRet = DeletePartition(hStore, szPart);
                SetCursor(hCursor);
                if(!fRet)
                {
                    ErrBox(hDlg, IDS_ERROR_PARTDELETE, GetLastError());
                }
            }

            CloseHandle(hStore);
            
            RefreshStoreInfo(hDlg);
            RefreshPartitionList(hDlg);
        }
    }
    
}

// --------------------------------------------------------
VOID GetSelectedPartProps(HWND hDlg)
// --------------------------------------------------------
{
    HANDLE hPart = INVALID_HANDLE_VALUE;
    PARTINFO partInfo = {0};
    partInfo.cbSize = sizeof(PARTINFO);

    hPart = OpenSelectedPart(hDlg);
    if(INVALID_HANDLE_VALUE != hPart)
    {
        
        DialogBoxParam(g_hInstance, MAKEINTRESOURCE(IDD_PROPPAGE_PARTINFO), 
            hDlg, (DLGPROC) PartInfoDlgProc, (DWORD)hPart);
        CloseHandle(hPart);
        RefreshPartitionList(hDlg);
    }
}

// --------------------------------------------------------
VOID FormatSelectedStore(HWND hDlg)
// --------------------------------------------------------
{
    HANDLE hStore = OpenSelectedStore(hDlg);

    if(INVALID_HANDLE_VALUE != hStore)
    {
        // confirm format dialog
        TCHAR szTitle[MAX_PATH], szMsg[MAX_PATH];
        LoadString(g_hInstance, IDS_CONFIRM_FMTSTORE_TITLE, szTitle, MAX_PATH);
        LoadString(g_hInstance, IDS_CONFIRM_FMTSTORE, szMsg, MAX_PATH);
        if(IDYES == MessageBox(hDlg, szMsg, szTitle, MB_YESNO))
        {
            HCURSOR hCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
            BOOL fRet = FormatStore(hStore);
            SetCursor(hCursor);
            if(fRet)
            {
                // format succeeded dialog
                LoadString(g_hInstance, IDS_SUCCESS_FMTSTORE_TITLE, szTitle, MAX_PATH);
                LoadString(g_hInstance, IDS_SUCCESS_FMTSTORE, szMsg, MAX_PATH);
                MessageBox(hDlg, szMsg, szTitle, MB_OK);
            }
            else
            {
                // error dialog
                ErrBox(hDlg, IDS_ERROR_STOREFORMAT, GetLastError());   
            }
        }

        CloseHandle(hStore);
        
        RefreshStoreInfo(hDlg);
        RefreshPartitionList(hDlg);
    }
    
}

// --------------------------------------------------------
VOID DismountSelectedStore(HWND hDlg)
// --------------------------------------------------------
{
    HANDLE hStore = OpenSelectedStore(hDlg);

    if(INVALID_HANDLE_VALUE != hStore)
    {
        HCURSOR hCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
        BOOL fRet = DismountStore(hStore);
        SetCursor(hCursor);
        if(!fRet)
            ErrBox(hDlg, IDS_ERROR_STOREDISMOUNT, GetLastError());   

        CloseHandle(hStore);
        
        RefreshStoreInfo(hDlg);
        RefreshPartitionList(hDlg);
    }
    
}

// --------------------------------------------------------
DWORD WINAPI StoreRefreshThread(LPVOID lpParam)
// --------------------------------------------------------
{
    HWND hDlg = (HWND)lpParam;
    DWORD dwSize, dwFlags;
    HANDLE hReg = NULL;
    MSGQUEUEOPTIONS msgopts = {0};
    BYTE pPNPBuf[sizeof(DEVDETAIL) + 200] = {0};
    DEVDETAIL *pd = (DEVDETAIL*)pPNPBuf;
    GUID guid = {0};
    HANDLE pHandles[2] = {0};

    msgopts.dwSize = sizeof(MSGQUEUEOPTIONS);
    msgopts.dwFlags = 0;
    msgopts.dwMaxMessages = 0;
    msgopts.cbMaxMessage = sizeof(pPNPBuf);
    msgopts.bReadAccess = TRUE;
    
    pHandles[0] = CreateMsgQueue(NULL, &msgopts);
    pHandles[1] = g_hTerminateEvent;
    
    hReg = RequestDeviceNotifications(&guid, pHandles[0], TRUE);

    while(TRUE)
    {
        DWORD dwWaitCode = WaitForMultipleObjects(2, pHandles, FALSE, INFINITE);
        if(WAIT_OBJECT_0 == dwWaitCode)
        {
            if(ReadMsgQueue(pHandles[0], pd, sizeof(pPNPBuf), &dwSize, INFINITE, &dwFlags))
            {
                if(0 == memcmp(&pd->guidDevClass, &BLOCK_DRIVER_GUID, sizeof(GUID)))
                {
                    RefreshStoreList(hDlg);
                    RefreshStoreInfo(hDlg);
                    RefreshPartitionList(hDlg);
                }
            }
        }
        else
        {
            // got terminate signal
            break;
        }
    }
    StopDeviceNotifications(hReg);
    CloseHandle(pHandles[0]);

    ExitThread(0);
    return 0;
}

// --------------------------------------------------------
BOOL CALLBACK MainDlgProc(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam)
// --------------------------------------------------------
{
	TCHAR	*pszHelpPath[] = {TEXT("file:ctpnl.htm#storage_manager\0")};

    switch(wMsg)
    {
        case WM_NOTIFY:
            switch(((NMHDR*)lParam)->code)
            {
                case PSN_HELP:
                    // start peghelp with storage manager help
					CreateProcess(TEXT("peghelp"), pszHelpPath[0], 0,0,0,0,0,0,0,0);                    
                    break;

                case PSN_APPLY:
                    EndDialog (hDlg, TRUE);
                    break;

                case PSN_RESET:
                    EndDialog (hDlg, FALSE);
                    break;

                case PSN_SETACTIVE:
                    break;
            }
            return TRUE;
            
        case WM_COMMAND:
            switch(LOWORD(wParam))
            {
                case IDC_BUTTON_DELETE:
                    DeleteSelectedPart(hDlg);
                    break;

                case IDC_BUTTON_NEW:
                    CreateNewPart(hDlg);                    
                    break;

                case IDC_BUTTON_PROPERTIES:
                    GetSelectedPartProps(hDlg);
                    break;
                    
                case IDC_COMBO_DISK:
                    RefreshStoreList(hDlg);
                    RefreshStoreInfo(hDlg);
                    RefreshPartitionList(hDlg);
                    break;

                case IDC_BUTTON_FORMATSTORE:
                    FormatSelectedStore(hDlg);
                    break;

                case IDC_BUTTON_DISMOUNTSTORE:
                    DismountSelectedStore(hDlg);
                    break;
        
                default:
                    break;
            }
            return TRUE;

        case WM_INITDIALOG:
            InitMainDlg(hDlg);
            return TRUE;
            
        case WM_DESTROY:
        case WM_CLOSE:
            DeinitMainDlg(hDlg);
            return TRUE;
    }
    return FALSE;
}

// --------------------------------------------------------
int CALLBACK PropSheetCallback(HWND hDlg, UINT uMsg, LPARAM lParam)
// --------------------------------------------------------
{
    if (PSCB_PRECREATE == uMsg)
        ((LPDLGTEMPLATE)lParam)->style &= ~WS_POPUP; // Treat this like a top level window
    return 0;
}
