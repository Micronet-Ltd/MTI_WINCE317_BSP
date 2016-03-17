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

HINSTANCE g_hUtilDll = NULL;

// --------------------------------------------------------
VOID MountUnmountPart(HWND hDlg, HANDLE hPart, BOOL fMount)
// --------------------------------------------------------
{
    BOOL fRet = FALSE;
    HCURSOR hCursor = NULL;
    
    if(INVALID_HANDLE_VALUE != hPart)
    {
        if(fMount)
        {
            hCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
            fRet = MountPartition(hPart);
            SetCursor(hCursor);
            if(!fRet)
                ErrBox(hDlg, IDS_ERROR_PARTMOUNT, GetLastError());
        }
        else
        {
            hCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
            fRet = DismountPartition(hPart);
            SetCursor(hCursor);
            if(!fRet)
                ErrBox(hDlg, IDS_ERROR_PARTDISMOUNT, GetLastError());    
        }
    }
}

// --------------------------------------------------------
VOID FormatSelectedPart(HWND hDlg, HANDLE hPart)
// --------------------------------------------------------
{       
    PFN_FORMATVOLUMEUI pfnFormatVolume;
    PARTINFO partInfo = {0};
    partInfo.cbSize = sizeof(PARTINFO);

    if(INVALID_HANDLE_VALUE != hPart)
    {
        if(GetPartitionInfo(hPart, &partInfo))
        {
            pfnFormatVolume = (PFN_FORMATVOLUMEUI)GetProcAddress(g_hUtilDll, TEXT("FormatVolumeUI"));
            if (!pfnFormatVolume) {
                NKDbgPrintfW( L"GetProcAddress failed \r\n");
                return;
            }
            else {
                pfnFormatVolume(hPart, hDlg);
            }

        }
    }   
}

// --------------------------------------------------------
VOID ScanSelectedPart(HWND hDlg, HANDLE hPart)
// --------------------------------------------------------
{       
    PFN_SCANVOLUMEUI pfnScanVolume;
    PARTINFO partInfo = {0};
    partInfo.cbSize = sizeof(PARTINFO);

    if(INVALID_HANDLE_VALUE != hPart)
    {
        if(GetPartitionInfo(hPart, &partInfo))
        {
            pfnScanVolume = (PFN_SCANVOLUMEUI)GetProcAddress(g_hUtilDll, TEXT("ScanVolumeUI"));
            if (!pfnScanVolume) {
                NKDbgPrintfW( L"GetProcAddress failed \r\n");
                return;
            }
            else {
                pfnScanVolume(hPart, hDlg);
            }

        }
    }   
}

// --------------------------------------------------------
VOID DefragSelectedPart(HWND hDlg, HANDLE hPart)
// --------------------------------------------------------
{       
    PFN_DEFRAGVOLUMEUI pfnDefragVolume;
    PARTINFO partInfo = {0};
    partInfo.cbSize = sizeof(PARTINFO);

    if(INVALID_HANDLE_VALUE != hPart)
    {
        if(GetPartitionInfo(hPart, &partInfo))
        {
            pfnDefragVolume = (PFN_DEFRAGVOLUMEUI)GetProcAddress(g_hUtilDll, TEXT("DefragVolumeUI"));
            if (!pfnDefragVolume) {
                NKDbgPrintfW( L"GetProcAddress failed \r\n");
                return;
            }
            else {
                pfnDefragVolume(hPart, hDlg);
            }
        }
    }   
}


// --------------------------------------------------------
VOID SetDlgPartInfo(HWND hDlg, HANDLE hPart)
// --------------------------------------------------------
{
    TCHAR szInfo[MAX_PATH] = _T("");
    TCHAR szFmt[MAX_PATH] = _T("");

    PARTINFO partInfo;
    partInfo.cbSize = sizeof(PARTINFO);
        
    GetPartitionInfo(hPart, &partInfo);
       
    // partition name
    SetDlgItemText(hDlg, IDC_TEXT_PARTNAME, partInfo.szPartitionName);

    // fsd name
    SetDlgItemText(hDlg, IDC_TEXT_FILESYS, partInfo.szFileSys);

    // partition size
    LoadString(g_hInstance, IDS_STRING_XSECTORS, szFmt, MAX_PATH);
    _stprintf(szInfo, szFmt, partInfo.snNumSectors);
    SetDlgItemText(hDlg, IDC_TEXT_PARTSIZE, szInfo);

    // partition type
    _stprintf(szInfo, _T("0x%02X"), partInfo.bPartType);
    SetDlgItemText(hDlg, IDC_TEXT_PARTTYPE, szInfo);

    // partition type
    _stprintf(szInfo, _T("0x%08X"), partInfo.dwAttributes);
    SetDlgItemText(hDlg, IDC_TEXT_PARTFLAGS, szInfo);   

    if (g_hUtilDll == NULL)
    {
        NKDbgPrintfW( L"LoadLibrary failed \r\n");
    }
   
    BOOL bMounted = (BOOL)(partInfo.dwAttributes & PARTITION_ATTRIBUTE_MOUNTED);

    EnableWindow (GetDlgItem (hDlg, IDC_BUTTON_UNMOUNT), bMounted);
    EnableWindow (GetDlgItem (hDlg, IDC_BUTTON_MOUNT), !bMounted);
    EnableWindow (GetDlgItem (hDlg, IDC_BUTTON_FORMAT), (BOOL)(g_hUtilDll != NULL && !bMounted));
    EnableWindow (GetDlgItem (hDlg, IDC_BUTTON_SCAN), (BOOL)(g_hUtilDll != NULL && !bMounted));
    EnableWindow (GetDlgItem (hDlg, IDC_BUTTON_DEFRAG), (BOOL)(g_hUtilDll != NULL && !bMounted));
}

// --------------------------------------------------------
HINSTANCE LoadUtilityLibrary(HANDLE hPart)
// --------------------------------------------------------
{
    HINSTANCE hRet = NULL;
    PARTINFO partInfo;
    partInfo.cbSize = sizeof(PARTINFO);
        
    GetPartitionInfo(hPart, &partInfo);

    // Every file system can have an associated utility dll which
    // can contin any or all scan/format/defrag exports.
    //
    // Eventually, the name of the utility dll should be available
    // in the STOREINFO structure.
    if(0 == _tcsicmp(partInfo.szFileSys, _T("FATFSD.DLL")))
    {
        hRet = LoadLibrary (TEXT("FATUTIL.DLL"));    
    }
    else if(0 == _tcsicmp(partInfo.szFileSys, _T("EXFAT.DLL")))
    {
        hRet = LoadLibrary (TEXT("FATUTIL.DLL"));    
    }
    // add other filesystem dll -> utility dll mappings here
    else
    {
        hRet = NULL;
    }
    return hRet;
}

// --------------------------------------------------------
BOOL CALLBACK PartInfoDlgProc(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam)
// --------------------------------------------------------
{
    static HANDLE hPart = INVALID_HANDLE_VALUE;
	TCHAR	*pszHelpPath[] = {TEXT("file:ctpnl.htm#advanced_partition\0")};
    
    switch(wMsg)
    {
        case WM_INITDIALOG:

            CenterWindow(hDlg);

            // lparam is hPart
            if(!lParam)
                EndDialog(hDlg, FALSE);
            else
            {
                hPart = (HANDLE)lParam;
                g_hUtilDll = LoadUtilityLibrary(hPart);
                SetDlgPartInfo(hDlg, hPart);
            }
            
            return TRUE;

        case WM_HELP:
			CreateProcess(TEXT("peghelp"), pszHelpPath[0], 0,0,0,0,0,0,0,0);
            return TRUE;

        case WM_COMMAND:
            
            switch(LOWORD(wParam))
            {
                case IDC_BUTTON_MOUNT:
                    MountUnmountPart(hDlg, hPart, TRUE);
                    SetDlgPartInfo(hDlg, hPart);
                    break;

                case IDC_BUTTON_UNMOUNT:
                    MountUnmountPart(hDlg, hPart, FALSE);
                    SetDlgPartInfo(hDlg, hPart);
                    break;

                case IDC_BUTTON_FORMAT:
                    FormatSelectedPart(hDlg, hPart);
                    SetDlgPartInfo(hDlg, hPart);
                    break;
                    
                case IDC_BUTTON_SCAN:
                    ScanSelectedPart(hDlg, hPart);
                    SetDlgPartInfo(hDlg, hPart);
                    break;

                case IDC_BUTTON_DEFRAG:
                    DefragSelectedPart(hDlg, hPart);
                    SetDlgPartInfo(hDlg, hPart);
                    break;
                    
                case IDOK:
                    EndDialog(hDlg, TRUE);
                    return TRUE;

                default:
                    return FALSE;
            }            
            return TRUE;

        case WM_DESTROY:
        case WM_CLOSE:
            if (g_hUtilDll)
                FreeLibrary(g_hUtilDll);
            
            EndDialog(hDlg, TRUE);
            return TRUE;

    }
    return FALSE;
}
