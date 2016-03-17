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

#include "QuickConfigUI.h"


#define REG_WZCLOG_KEY          TEXT("Drivers\\BuiltIn\\Ethman\\Log")
#define REG_WZCLOG_QUEUE_NAME   TEXT("QueueData")
#define REG_WZCLOG_NEWIDX_NAME  TEXT("NewIndex")
#define REG_WZCLOG_OLDIDX_NAME  TEXT("OldIndex")


static HWND             g_hWndLog;
static WZCLogMsgQueue   g_LogMsgQueue;



/*++

Routine Name:

    WzcLoggingDialogProc

Routine Description:

    Dialog Proc for the Advanced Logging dialog.

Arguments:

    hwndDlg:    HWND of the dialog
    uMsg:       Message being processed
    wParam:     Additional data
    lParam:     Additional data

--*/

BOOL CALLBACK WzcLoggingDialogProc(HWND hwndDlg, 
                                UINT    uMsg,
                                WPARAM  wParam,
                                LPARAM  lParam)
{
    BOOL fRetVal = FALSE;

    switch(uMsg)
    {
        case WM_INITDIALOG:

            fRetVal = TRUE;
            break;

        case WM_CLOSE:
            
            DestroyWindow(hwndDlg);
            fRetVal = FALSE;
            break;

        case WM_DESTROY:

            PostQuitMessage(0);
            fRetVal = FALSE;
            break;
    
        default:
            fRetVal = DefWindowProc(hwndDlg, uMsg, wParam, lParam);
    }

    return fRetVal;
    
} // WzcLoggingDialogProc



// Use this spaces array for perfect formatting
const TCHAR cszSpaces[] = TEXT("                                        ");



/*++

Routine Name:

    AddLoggingMsg

Routine Description:

    This function adds an entry to the log.

Arguments:

    ptcDeviceName:  Name of the device
    pucMacAddr:     Adapter address
    ft:             Time stamp
    ptcMessage:     Message to display

Return Value:

    HRESULT to indicate return status

--*/

HRESULT AddLoggingMsg(TCHAR* ptcDeviceName, UCHAR* pucMacAddr, FILETIME ft, TCHAR* ptcMessage)
{
    HRESULT     hr = S_OK;
    SYSTEMTIME  SystemTime;
    FILETIME    LocalTime;

    EnterCriticalSection(&g_LogMsgQueue.Lock);

    // If the list is full
    if(g_LogMsgQueue.dwNewMsgIdx - g_LogMsgQueue.dwOldMsgIdx == -1)
    {
        g_LogMsgQueue.dwNewMsgIdx = g_LogMsgQueue.dwOldMsgIdx;

        // Check if we should wrap around the array
        if(g_LogMsgQueue.dwOldMsgIdx == WZCLOG_ENTRIES - 1)
        {
            g_LogMsgQueue.dwOldMsgIdx = 0;
        }
        else
        {
            g_LogMsgQueue.dwOldMsgIdx++;
        }
    }
    // If the list is empty
    else if(g_LogMsgQueue.dwOldMsgIdx == -1 && g_LogMsgQueue.dwNewMsgIdx == -1)
    {
        g_LogMsgQueue.dwNewMsgIdx = 0;
    }
    // If the list has one entry
    else if(g_LogMsgQueue.dwOldMsgIdx == -1)
    {
        g_LogMsgQueue.dwNewMsgIdx++;
        g_LogMsgQueue.dwOldMsgIdx = 0;
    }
    else
    {
        // Check if we should wrap around the array
        if(g_LogMsgQueue.dwNewMsgIdx == WZCLOG_ENTRIES - 1)
        {
            g_LogMsgQueue.dwOldMsgIdx++;
            g_LogMsgQueue.dwNewMsgIdx = 0;
        }
        else
        {
            g_LogMsgQueue.dwNewMsgIdx++;
        }       
    }

    FileTimeToLocalFileTime(&ft, &LocalTime);
    FileTimeToSystemTime(&LocalTime, &SystemTime);

    PREFAST_ASSERT(g_LogMsgQueue.dwNewMsgIdx < WZCLOG_ENTRIES);

    if(pucMacAddr == NULL)
    {
        _sntprintf(g_LogMsgQueue.pszList[g_LogMsgQueue.dwNewMsgIdx], LOG_STATUS_SIZE, TEXT("[%02d:%02d:%02d - %s] %s"), 
                    SystemTime.wHour,
                    SystemTime.wMinute,
                    SystemTime.wSecond,
                    ptcDeviceName,
                    ptcMessage);
    }
    else
    {
        _sntprintf(g_LogMsgQueue.pszList[g_LogMsgQueue.dwNewMsgIdx], LOG_STATUS_SIZE, TEXT("[%02d:%02d:%02d - %s] %s - AP [%02x-%02x-%02x-%02x-%02x-%02x]"), 
                    SystemTime.wHour,
                    SystemTime.wMinute,
                    SystemTime.wSecond,
                    ptcDeviceName, 
                    ptcMessage,
                    pucMacAddr[0],
                    pucMacAddr[1],
                    pucMacAddr[2],
                    pucMacAddr[3],
                    pucMacAddr[4],
                    pucMacAddr[5]);
    }
    
    // Update dialog if it is visible
    if(g_LogMsgQueue.dwState == WZCLOG_ACTIVE)
    {
        UpdateWZCLogView();
    }

    LeaveCriticalSection(&g_LogMsgQueue.Lock);
    
    return hr;
    
} // AddLoggingMsg



/*++

Routine Name:

    UpdateWZCLogView

Routine Description:

    Updates the dialog display with the current contents of the queue.

--*/

void UpdateWZCLogView()
{
    DWORD   dwCurr = 0;
    HANDLE  hTextBox;
    TCHAR*  ptcLog;

    // List is empty
    if(g_LogMsgQueue.dwNewMsgIdx == -1)
    {
        goto exit;
    }

    // Get dialog item
    hTextBox = GetDlgItem(g_hWndLog, IDC_QC_WZCLOG);
    if(hTextBox == NULL)
    {
        goto exit;
    }

    // Allocate buffer for log
    ptcLog = LocalAlloc(0, (WZCLOG_ENTRIES * LOG_STATUS_SIZE * sizeof(TCHAR)));
    if(! ptcLog)
    {
        DEBUGMSG(ZONE_ERROR, (_T("NETUIQC: Out of memory")));
        goto exit;
    }

    _tcscpy(ptcLog, TEXT(""));
    dwCurr = g_LogMsgQueue.dwOldMsgIdx;

    // If the queue only has one message then advance to element 0
    if(dwCurr == -1) dwCurr = 0;

    // Create window text
    while(dwCurr != g_LogMsgQueue.dwNewMsgIdx)
    {
        // If we hit the end of the queue then restart
        if(dwCurr == WZCLOG_ENTRIES)
        {
            dwCurr = 0;
            if(dwCurr == g_LogMsgQueue.dwNewMsgIdx)
            {
                break;
            }
        }
        
        _tcscat(ptcLog, g_LogMsgQueue.pszList[dwCurr]);
        _tcscat(ptcLog, TEXT("\r\n"));
        dwCurr++;
    }

    _tcscat(ptcLog, g_LogMsgQueue.pszList[g_LogMsgQueue.dwNewMsgIdx]);

    // Set new text
    SetWindowText(hTextBox, ptcLog);

    LocalFree(ptcLog);
    
exit:
   
    return;

} // UpdateWZCLogView



/*++

Routine Name:

    GetWZCLogState

Routine Description:

    Returns the state of the Logging Message Queue

Return Value:

    DWORD representing the state of the message queue

--*/

DWORD GetWZCLogState()
{
    return g_LogMsgQueue.dwState;
}

// The following two functions are called to init/deinit the logging CS
void InitLoggingCS(void)
{
    InitializeCriticalSection(&g_LogMsgQueue.Lock);
}
void DeleteLoggingCS(void)
{
    DeleteCriticalSection(&g_LogMsgQueue.Lock); 
}

// The following function is called to open the logging dialog
void OpenLoggingWindow(void)
{
    EnterCriticalSection(&g_LogMsgQueue.Lock);
    
    if (! g_LogMsgQueue.hThread) 
    {
        g_LogMsgQueue.hThread = CreateThread(NULL, 0, WzcLoggingThread, NULL, 0, NULL);
    }

    LeaveCriticalSection(&g_LogMsgQueue.Lock);
}


/*++

Routine Name:

    InitializeLoggingQueue

Routine Description:

    Initializes the logging queue.

Return Value:

    HRESULT to indicate return status

--*/

HRESULT InitializeLoggingQueue()
{
    HRESULT hr = S_OK;  

    EnterCriticalSection(&g_LogMsgQueue.Lock);

    g_LogMsgQueue.dwNewMsgIdx   = -1;
    g_LogMsgQueue.dwOldMsgIdx   = -1;

    LoadStateFromRegistry();
    
    g_LogMsgQueue.dwState       = WZCLOG_INACTIVE;

    LeaveCriticalSection(&g_LogMsgQueue.Lock);

    return hr;
    
} // InitializeLoggingQueue


/*++

Routine Name:

    UnloadLoggingQueue

Routine Description:

    Uninitializes the logging queue.

--*/

void UnloadLoggingQueue()
{
    HANDLE hThread;

    EnterCriticalSection(&g_LogMsgQueue.Lock);
    SaveStateToRegistry();
    PostMessage(g_hWndLog, WM_CLOSE, 0, 0); 
    hThread = g_LogMsgQueue.hThread;
    LeaveCriticalSection(&g_LogMsgQueue.Lock);

    if (hThread) {
        WaitForSingleObject(hThread, INFINITE);
    }
} // UnloadLoggingQueue


/*++

Routine Name:

    WzcLoggingThread

Routine Description:

    This thread opens the dialog and enters a message pump.

--*/

DWORD WINAPI WzcLoggingThread(LPVOID pv)
{
    MSG     msg;
    BOOL    fPortrait = (GetSystemMetrics(SM_CXSCREEN) < 480)? TRUE:FALSE;
    int     idd = fPortrait? IDD_QC_WZCLOG_N:IDD_QC_WZCLOG_W;

    EnterCriticalSection(&g_LogMsgQueue.Lock);

    // If Window is active then do not open a second window
    if(g_LogMsgQueue.dwState == WZCLOG_INACTIVE)
    {
        g_LogMsgQueue.dwState = WZCLOG_ACTIVE;

        // Create Dialog
        g_hWndLog = CreateDialog(v_hInst, MAKEINTRESOURCE(idd), NULL, WzcLoggingDialogProc);
        if(g_hWndLog == NULL)
        {
            DEBUGMSG(ZONE_ERROR, (TEXT("NETUIQC: Error creating logging dialog")));
        }

        UpdateWZCLogView();

        LeaveCriticalSection(&g_LogMsgQueue.Lock);

        // Message Pump
        while(GetMessage(&msg, NULL, 0, 0))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        EnterCriticalSection(&g_LogMsgQueue.Lock);

        g_LogMsgQueue.dwState = WZCLOG_INACTIVE;
    }
    else if(g_LogMsgQueue.dwState == WZCLOG_ACTIVE)
    {
        SetForegroundWindow(g_hWndLog);
    }

    g_LogMsgQueue.hThread = NULL;

    LeaveCriticalSection(&g_LogMsgQueue.Lock);
    
    return 0;
} // WzcLoggingThread




/*++

Routine Name:

    LoadStateFromRegistry

Routine Description:

    This function gets the state of the 

--*/

void LoadStateFromRegistry()
{
    DWORD   dwErr   = ERROR_SUCCESS;
    DWORD   dwSize  = 0;
    DWORD   dwType  = 0;
    HKEY    hkRoot  = NULL;

    // Open key
    dwErr = RegOpenKeyEx(
                    HKEY_LOCAL_MACHINE,
                    REG_WZCLOG_KEY,
                    0,
                    0,
                    &hkRoot);
    if(dwErr != ERROR_SUCCESS)
    {
        DEBUGMSG(ZONE_ERROR, 
            (TEXT("NETUIQC: Unable to open key [%s]"),
            REG_WZCLOG_KEY));
        goto exit1;
    }

    // Get MsgQueue
    dwSize = sizeof(g_LogMsgQueue.pszList);
    dwErr = RegQueryValueEx(
                    hkRoot,
                    REG_WZCLOG_QUEUE_NAME,
                    0,
                    &dwType,
                    (LPBYTE)g_LogMsgQueue.pszList,
                    &dwSize);
    if(dwErr != ERROR_SUCCESS)
    {
        goto exit1;
    }

    // Get New Index
    dwSize = sizeof(DWORD);
    dwErr = RegQueryValueEx(
                    hkRoot,
                    REG_WZCLOG_NEWIDX_NAME,
                    0,
                    &dwType,
                    (LPBYTE)&g_LogMsgQueue.dwNewMsgIdx,
                    &dwSize);
    if(dwErr != ERROR_SUCCESS)
    {
        goto exit2;
    }

    // Get Old Index
    dwSize = sizeof(DWORD);
    dwErr = RegQueryValueEx(
                    hkRoot,
                    REG_WZCLOG_OLDIDX_NAME,
                    0,
                    &dwType,
                    (LPBYTE)&g_LogMsgQueue.dwOldMsgIdx,
                    &dwSize);
    if(dwErr != ERROR_SUCCESS)
    {
        goto exit2;
    }

    goto exit1;

exit2:

    g_LogMsgQueue.dwNewMsgIdx = -1;
    g_LogMsgQueue.dwOldMsgIdx = -1;
    
exit1:
        if (hkRoot != NULL)
            RegCloseKey(hkRoot);

    return;
} // LoadStateFromRegistry



/*++

Routine Name:

    SaveStateToRegistry

Routine Description:

    This function saves the state of the message queue to the registry.

--*/

void SaveStateToRegistry()
{
    DWORD   dwErr;
    HKEY    hkRoot = NULL;

    //  Open or Create the key if it is not already there.
    dwErr = RegCreateKeyEx(
                HKEY_LOCAL_MACHINE,
                REG_WZCLOG_KEY,
                0x00,
                NULL,
                0x00,
                KEY_WRITE,
                NULL,
                &hkRoot,
                NULL);
    if (dwErr != ERROR_SUCCESS)
    {
        DEBUGMSG (ZONE_ERROR,
            (TEXT("NETUIQC: Unable to open/create key [%s]\r\n"),
            REG_WZCLOG_KEY));
        goto exit;
    }

    // Try to set the value.  If it fails it is not the end of the world, our data
    // will just not be saved.
    dwErr = RegSetValueEx(
                    hkRoot,
                    REG_WZCLOG_QUEUE_NAME,
                    0,
                    REG_BINARY,
                    (LPBYTE)g_LogMsgQueue.pszList,
                    sizeof(g_LogMsgQueue.pszList));
    if(dwErr != ERROR_SUCCESS)
    {
        goto exit;
    }

    // Try to set DWORD Value for new index
    dwErr = RegSetValueEx(
                    hkRoot,
                    REG_WZCLOG_NEWIDX_NAME,
                    0,
                    REG_DWORD,
                    (LPBYTE) &g_LogMsgQueue.dwNewMsgIdx,
                    sizeof(DWORD));
    if(dwErr != ERROR_SUCCESS)
    {
        goto exit;
    }

    // Try to set DWORD Value for old index
    dwErr = RegSetValueEx(
                    hkRoot,
                    REG_WZCLOG_OLDIDX_NAME,
                    0,
                    REG_DWORD,
                    (LPBYTE) &g_LogMsgQueue.dwOldMsgIdx,
                    sizeof(DWORD));
    if(dwErr != ERROR_SUCCESS)
    {
        goto exit;
    }   
    
exit:
        if (hkRoot != NULL)
            RegCloseKey(hkRoot);
    return;

} // SaveStateToRegistry

