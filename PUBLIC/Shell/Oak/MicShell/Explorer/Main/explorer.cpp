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


#include <windows.h>
#include "resource.h"
#include "desktop.h"
#include "utils.h"
#include "taskbar.hxx"
#include "shcore.h"
#include <shellapi.h>
#include <shellsdk.h>
#include "aygutils.h"
#include "curlfile.h"
#include "debugzones.h"
#include "urlmonutils.hpp"
#include "explorerlist.hpp"

#ifndef _PREFAST_
#pragma warning(disable:4068)
#endif // _PREFAST_

LONG      glThreadCount            = 0;
HANDLE    ghExitEvent              = NULL;
HANDLE    ghKillFontThread         = NULL;
HINSTANCE g_hInstance              = NULL;
DWORD     g_dwMainWindowStackSize  = 0x20000;
HINSTANCE g_hCoreDLL               = NULL;
BOOL      g_bBrowseInPlace         = TRUE;
BOOL      g_bQVGAEnabled           = FALSE;

CTaskBar *g_TaskBar = NULL;
CDesktopWnd *g_Desktop = NULL;

WCHAR const c_szSystemStart[] = TEXT("SystemStart");
#define DoSHCreateExplorerInstance \
    PRIV_IMPLICIT_DECL(BOOL, SH_SHELL, 9, (LPCWSTR,UINT))

#define MAX_URL  2060

BOOL WINAPI SHCreateExplorerInstance(LPCWSTR pszPath, UINT uFlags);
extern "C" BOOL IEUsesDCOM();

DWORD WINAPI CreateTaskBar(LPVOID pEvent)
{
    HANDLE hSyncEvent = *((HANDLE *) pEvent);
    CTaskBar *pTaskBar = NULL;
    HWND hwndTB = NULL;
    
    pTaskBar = new CTaskBar;
    if(!pTaskBar)
    {
        SetEvent(hSyncEvent);
        return 0;
    }

    g_TaskBar = pTaskBar;
    if(!pTaskBar->Register(g_hInstance))
    {
        g_TaskBar = NULL;
        delete pTaskBar;
        SetEvent(hSyncEvent);
        return 0;
    }

    RegisterTaskBar(pTaskBar->GetWindow());
    SetEvent(hSyncEvent);

    DWORD dwRet = pTaskBar->MessageLoop();

    delete pTaskBar;

    return dwRet;
}

DWORD WINAPI FontThread(LPVOID)
{
    int FontFolders[] = { CSIDL_FONTS, CSIDL_WINDOWS };
    WCHAR szPath[MAX_PATH];
    LPWSTR pszFileName = NULL;
    DWORD dwWaitObjects = 0;
    HANDLE waitObjects[(sizeof(FontFolders)/sizeof(*FontFolders))+1];
    HANDLE hFind = NULL;
    WIN32_FIND_DATA fd = {0}; 
    DWORD dwWait;

    waitObjects[dwWaitObjects++] = ghKillFontThread;

    for (int i = 0; i < sizeof(FontFolders)/sizeof(*FontFolders); i++)
    {
        if (SHGetSpecialFolderPath(NULL, szPath, FontFolders[i], TRUE))
        {
            waitObjects[dwWaitObjects] = FindFirstChangeNotification(szPath, FALSE,
                                                                  FILE_NOTIFY_CHANGE_CEGETINFO |
                                                                  FILE_NOTIFY_CHANGE_FILE_NAME |
                                                                  FILE_NOTIFY_CHANGE_SIZE);

            if (INVALID_HANDLE_VALUE != waitObjects[dwWaitObjects])
            {
                dwWaitObjects++;
            }

         if (SUCCEEDED(StringCchCatEx(szPath, sizeof(szPath)/sizeof(*szPath),
                                      TEXT("\\*.*"), &pszFileName, NULL, 0)))
         {
            pszFileName -= 3; // Just before the *.*
            hFind = FindFirstFile(szPath, &fd);
            if (INVALID_HANDLE_VALUE != hFind)
            {
               do
               {
                  if (PathIsExtension(fd.cFileName, TEXT(".ttf")) ||
                      PathIsExtension(fd.cFileName, TEXT(".ttc")) ||
                      PathIsExtension(fd.cFileName, TEXT(".fon")) ||
                      PathIsExtension(fd.cFileName, TEXT(".fnt")))
                  {
                     _tcscpy(pszFileName, fd.cFileName);
                     if (AddFontResource(szPath))
                        PostMessage(HWND_BROADCAST, WM_FONTCHANGE, 0, 0);
                  }
               } while (FindNextFile(hFind, &fd));
               FindClose(hFind);
            }
         }
      }
   }

    if (dwWaitObjects < 2)
    {
        return 0;
    }

    while (TRUE)
    {
        dwWait = WaitForMultipleObjects(dwWaitObjects, waitObjects, FALSE, INFINITE);
        if (WAIT_OBJECT_0 == dwWait)
        {
            break;
        }
        else if (WAIT_OBJECT_0+dwWaitObjects > dwWait)
        {
            DWORD cbBuffer = 0;
            LPBYTE pBuffer = NULL;

            if (!::CeGetFileNotificationInfo(waitObjects[dwWait-WAIT_OBJECT_0],
                            0, NULL, 0, NULL, &cbBuffer))
            {
                continue;
            }

            ASSERT(0 < cbBuffer);

            pBuffer = (LPBYTE) LocalAlloc(LMEM_FIXED, cbBuffer);
            if (!pBuffer)
            {
                continue;
            }

            if (!::CeGetFileNotificationInfo(waitObjects[dwWait-WAIT_OBJECT_0],
                            0, pBuffer, cbBuffer, NULL, NULL))
            {
                LocalFree(pBuffer);
                continue;
            }

            int csidl = FontFolders[dwWait-WAIT_OBJECT_0-1];
            FILE_NOTIFY_INFORMATION * pNotifyInfo = NULL;
            DWORD cbOffset = 0;
            do
            {
                pNotifyInfo = (FILE_NOTIFY_INFORMATION *) (pBuffer+cbOffset);
                ASSERT(0 != ::wcscmp(L"\\", pNotifyInfo->FileName));
                switch (pNotifyInfo->Action)
                {
                case FILE_ACTION_CHANGE_COMPLETED:
                case FILE_ACTION_RENAMED_NEW_NAME:
                    if ( ( PathIsExtension(pNotifyInfo->FileName, TEXT(".ttf")) ||
                        PathIsExtension(pNotifyInfo->FileName, TEXT(".ttc")) ||
                        PathIsExtension(pNotifyInfo->FileName, TEXT(".fon")) ||
                        PathIsExtension(pNotifyInfo->FileName, TEXT(".fnt")) ) &&
                        SHGetSpecialFolderPath(NULL, szPath, csidl, FALSE) &&
                        ( StringCchCat(szPath, sizeof(szPath)/sizeof(*szPath), TEXT("\\")) == S_OK ) && 
                        ( StringCchCat(szPath, sizeof(szPath)/sizeof(*szPath), pNotifyInfo->FileName) == S_OK ) &&
                        AddFontResource(szPath) )
                    {
                        PostMessage(HWND_BROADCAST, WM_FONTCHANGE, 0, 0);
                    }
                break;

                case FILE_ACTION_REMOVED:
                case FILE_ACTION_RENAMED_OLD_NAME:
                    if ( (PathIsExtension(pNotifyInfo->FileName, TEXT(".ttf")) ||
                        PathIsExtension(pNotifyInfo->FileName, TEXT(".ttc")) ||
                        PathIsExtension(pNotifyInfo->FileName, TEXT(".fon")) ||
                        PathIsExtension(pNotifyInfo->FileName, TEXT(".fnt")) ) &&
                        SHGetSpecialFolderPath(NULL, szPath, csidl, FALSE) && 
                        ( StringCchCat(szPath, sizeof(szPath)/sizeof(*szPath), TEXT("\\")) == S_OK ) &&
                        ( StringCchCat(szPath, sizeof(szPath)/sizeof(*szPath), pNotifyInfo->FileName) == S_OK ) &&
                        RemoveFontResource(szPath) )
                    {
                        PostMessage(HWND_BROADCAST, WM_FONTCHANGE, 0, 0);
                    }
                    break;
                }

                cbOffset += pNotifyInfo->NextEntryOffset;
            } while(pNotifyInfo->NextEntryOffset);

            LocalFree(pBuffer);
        }
        else
        {
            ASSERT(WAIT_OBJECT_0+dwWaitObjects > dwWait);
            break;
        }
    }

    return 0;
}

void ProcessStartupFolder()
{
    WCHAR szPath[MAX_PATH];
    HANDLE hFind = NULL;
    WIN32_FIND_DATA fd = {0};
    LPWSTR pszFileName = NULL;

    if (!SHGetSpecialFolderPath(NULL, szPath, CSIDL_STARTUP, FALSE) ||
        FAILED(StringCchCatEx(szPath, sizeof(szPath)/sizeof(*szPath),
                              TEXT("\\*.*"), &pszFileName, NULL, 0)))
    {
       return;
    }

    hFind = FindFirstFile(szPath, &fd);
    if (INVALID_HANDLE_VALUE != hFind)
    {
        SHELLEXECUTEINFO sei = {0};
        sei.cbSize = sizeof(sei);
        sei.nShow = SW_SHOWNORMAL;
        sei.lpFile = szPath;
        pszFileName -= 3; // Just before the *.*

        do
        {
            if ((!(FILE_ATTRIBUTE_DIRECTORY & fd.dwFileAttributes)) &&
                (0 != _tcsicmp(TEXT("desktop.ini"), fd.cFileName)))
            {
                _tcscpy(pszFileName, fd.cFileName);
                ShellExecuteEx(&sei);
            }
        } while (FindNextFile(hFind, &fd));
        FindClose(hFind);
    }
}

void CopyDesktopIniFiles()
{
    LPCWSTR wszFileNames[] = { L"appdata.ini", L"desktopdirectory.ini", L"favorites.ini",
                               L"fonts.ini", L"mydocuments.ini", L"programfiles.ini",
                               L"programs.ini", L"recent.ini", L"startup.ini", NULL };
    UINT csidlFolders[] = { CSIDL_APPDATA, CSIDL_DESKTOPDIRECTORY, CSIDL_FAVORITES, 
                            CSIDL_FONTS, CSIDL_PERSONAL, CSIDL_PROGRAM_FILES, 
                            CSIDL_PROGRAMS, CSIDL_RECENT, CSIDL_STARTUP, 0};
    WCHAR wszFrom[MAX_PATH];
    WCHAR wszTo[MAX_PATH];

    for (int i = 0; wszFileNames[i]; i ++)
    {
        if (SHGetSpecialFolderPath(NULL, wszFrom, CSIDL_WINDOWS, TRUE) &&
            SHGetSpecialFolderPath(NULL, wszTo, csidlFolders[i], TRUE))
        {
           if (SUCCEEDED(StringCbCatW(wszFrom, sizeof(wszFrom), L"\\")) &&
               SUCCEEDED(StringCbCatW(wszFrom, sizeof(wszFrom), wszFileNames[i])) &&
               SUCCEEDED(StringCbCatW(wszTo, sizeof(wszTo), L"\\desktop.ini")))
           {
              CopyFile(wszFrom, wszTo, TRUE);
           }
        }
    }
}

void DoStartupTasks()
{
    HANDLE hThread = NULL;

    // Spin off the thread which registers and watches the font dirs
    hThread = CreateThread(NULL, NULL, FontThread, NULL, 0, NULL);
    if (hThread)
    {
        CloseHandle(hThread);
    }

    // Launch all applications in the startup folder
    ProcessStartupFolder();
}

// If we're running on an image that has DCOM included, we cannot call
// CoCreateInstance() until after the DCOM services have been started.
// Block here until they are ready.  This does not affect images with base COM.
void WaitForCOMToBeReady() {
    HANDLE hCOMReadyEvent = OpenEvent(EVENT_ALL_ACCESS,FALSE,DCOM_EVENTNAME_API_SET_READY);
    if (!hCOMReadyEvent) {
        // If we were unable to open the event, indicates DCOM is not present
        // on the system and that there is no need to wait.
        return;
    }

    DEBUGMSG(ZONE_INFO,(L"Explorer: Blocking for DCOM services to become available before proceeding\r\n"));
    WaitForSingleObject(hCOMReadyEvent,INFINITE);
    DEBUGMSG(ZONE_INFO,(L"Explorer: DCOM services available\r\n"));
    CloseHandle(hCOMReadyEvent);
}

int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPWSTR lpCmdLine, int nCmdShow)
{
    INITCOMMONCONTROLSEX iccsex;
    HKEY hKey;
    DWORD dwSize = sizeof(DWORD);
    MSG msg;
    HANDLE hThread = INVALID_HANDLE_VALUE;
    HANDLE hTaskbarSyncEvent = NULL;
    HANDLE hShellAPIs = NULL;
    g_hInstance = hInst;
    BOOL bInitialized = FALSE;

    HRESULT hr;

    // HKCU is where IE\Main settings are
    if(ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER, TEXT("SOFTWARE\\Microsoft\\Internet Explorer\\Main"), 0, 0, &hKey))
    {
        RegQueryValueEx(hKey, TEXT("StackRes"), NULL, NULL, (LPBYTE)&g_dwMainWindowStackSize, &dwSize);
        RegCloseKey(hKey);
    }

    // provide a default stack size if the one given is too small or too large. 
    if(g_dwMainWindowStackSize < 0x10000 || g_dwMainWindowStackSize > 0x80000)
    {
            g_dwMainWindowStackSize = 0x20000; // default to 128k
    }
    g_bBrowseInPlace = BrowseInPlace();

    DWORD dwQVGAEnabled = 0;
    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("Explorer"), 0, 0, &hKey))
    {
        dwSize = sizeof(DWORD);
        RegQueryValueEx(hKey, TEXT("QVGA"), NULL, NULL, (LPBYTE)&dwQVGAEnabled, &dwSize);
        RegCloseKey(hKey);
    }
    g_bQVGAEnabled = (dwQVGAEnabled != 0);

    ghExitEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (!ghExitEvent)
    {
        return FALSE;
    }

    // shell cannot run multi-instanced
    if(hPrevInst || FindWindow(L"DesktopExplorerWindow", NULL)) 
    {
        LPWSTR pszUrl = NULL;
        WCHAR sz[] = TEXT("\\");
        if (!lpCmdLine || (TEXT('\0') == *lpCmdLine))
        {
            lpCmdLine = sz;
        }

        // special handling for URL files
        if(!_wcsnicmp(lpCmdLine, TEXT("-u"), 2))
        {
            CUrlFile theURL;
            pszUrl= (LPWSTR)LocalAlloc(LMEM_FIXED, sizeof(WCHAR)*MAX_URL);

            if(pszUrl && S_OK == theURL.OpenFile(lpCmdLine+2, FALSE))
            {
                theURL.ReadURL(pszUrl, MAX_URL);
                lpCmdLine = pszUrl;
            }
        }

        LPWSTR pszParsingName = (LPWSTR)LocalAlloc(LPTR, sizeof(WCHAR)*MAX_PATH);
        if (pszParsingName)
        {
            lstrcpy(pszParsingName, TEXT("::"));
            UINT cchBuf = MAX_PATH-2;
            if(GetParsingName(lpCmdLine, pszParsingName+2, &cchBuf))
            {
                lpCmdLine = pszParsingName;
            }
        }

        if (!g_bBrowseInPlace && !PathCanBrowseInPlace(lpCmdLine))
        {
            // init common controls
            iccsex.dwSize = sizeof(INITCOMMONCONTROLSEX);
            iccsex.dwICC = ICC_COOL_CLASSES;
            InitCommonControlsEx(&iccsex);

            //Reset browse in place flag
            g_bBrowseInPlace = TRUE;

            SHCreateExplorerInstance(lpCmdLine, 0 );
            while (glThreadCount > 0)
            {
                WaitForSingleObject(ghExitEvent, INFINITE);
            }
        }
        else
        {
            DoSHCreateExplorerInstance(lpCmdLine, 0 );
        }

        if (pszParsingName)
        {
            LocalFree(pszParsingName);
        }

        if (pszUrl)
        {
            LocalFree(pszUrl);
        }

        return 0;
    }

    //set the color scheme
    SetSchemeColors();

    WaitForCOMToBeReady();

    hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if (FAILED(hr))
    {
        return FALSE;
    }

    ghKillFontThread = CreateEvent(NULL, TRUE, FALSE, NULL); 
    if (!ghKillFontThread)
    {
        return FALSE;
    }

    // init common controls
    iccsex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    iccsex.dwICC = ICC_COOL_CLASSES;
    InitCommonControlsEx(&iccsex);

    // Load AYGShell and load extra common controls
    InitializeAygUtils();
    if (LoadAygshellLibrary())
    {
        AygInitExtraControls();
    }

#pragma prefast(suppress:321, "Not loading dll via relative path") 
    g_hCoreDLL = LoadLibrary(TEXT("coredll"));

    // Initialize Urlmon utils to use the Internet Security Manager
    UrlmonUtils_t::InitializeUrlmonUtils(g_hInstance);

    ExplorerList_t::InitializeExplorerList();

    // Put the desktop.ini files in their correct locations
    CopyDesktopIniFiles();

    // Create the desktop window
    CDesktopWnd *pDesktop = new CDesktopWnd;
    if(!pDesktop ||
        !pDesktop->Create())
    {
        goto Cleanup;
    }
    g_Desktop = pDesktop;

    // fork off a thread for the taskbar
    hTaskbarSyncEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (!hTaskbarSyncEvent)
    {
        goto Cleanup;
    }
    hThread = CreateThread(NULL, NULL, CreateTaskBar, &hTaskbarSyncEvent, 0, NULL);
    WaitForSingleObject(hTaskbarSyncEvent, INFINITE);
    // GetExitCodeThread(); // Test for failure?
    CloseHandle(hThread);
    CloseHandle(hTaskbarSyncEvent);

    // Force the workarea change on the desktop since the posted message
    // won't get there until the desktop is already visible
    SendMessage(pDesktop->GetWindow(), WM_SETTINGCHANGE, (WPARAM) SPI_SETWORKAREA, NULL);

    // Tell the system that we are ready
    RegisterShellAPIs();
    SignalStarted(_wtol(lpCmdLine));
    hShellAPIs = OpenEvent(EVENT_ALL_ACCESS, FALSE, TEXT("SYSTEM/ShellAPIReady"));
    ASSERT(hShellAPIs);
    if (hShellAPIs)
    {
        SetEvent(hShellAPIs);
        CloseHandle(hShellAPIs);
    }

    // Process the items in the start up folder
    if (!GetAsyncKeyState(VK_SHIFT))
    {
        DoStartupTasks();
    }
    else
    {
        DEBUGMSG(ZONE_WARNING, (TEXT("Bypassing startup files.\r\n")));

        DWORD dw = 0;
        if (ERROR_SUCCESS != RegQueryValueEx(HKEY_LOCAL_MACHINE,
                                             TEXT("CalibrationData"),
                                             (LPDWORD) TEXT("HARDWARE\\DEVICEMAP\\TOUCH"),
                                             NULL, NULL, &dw))
        {
            TouchCalibrate();
        }

    }

    sndPlaySound(c_szSystemStart, SND_ALIAS|SND_ASYNC|SND_NODEFAULT);
    bInitialized = TRUE;

    while (GetMessage( &msg, NULL, 0, 0 ))
    {
         if (msg.message == WM_QUIT)
         {
             break;
         }

         if (S_OK != pDesktop->TranslateAcceleratorSB(&msg, 0))
         {
            TranslateMessage(&msg);
            DispatchMessage(&msg);  
         }
    }

    SetEvent(ghKillFontThread);

    while (glThreadCount > 0)
    {
        WaitForSingleObject(ghExitEvent, INFINITE);
    }
    
Cleanup:
    if(pDesktop)
        pDesktop->Release();

    FreeAygshellLibrary();
    UnInitializeAygUtils();

    if (g_hCoreDLL)
    {
        FreeLibrary(g_hCoreDLL);
    }

    UrlmonUtils_t::UnInitializeUrlmonUtils();
    ExplorerList_t::UnInitializeExplorerList();

    CoUninitialize();

    UnRegisterShellAPIs();

    // only restart explorer if we initialized correctly, avoids
    // endless loops of explorer restarting when something is wrong
    if ( bInitialized )
    {
        // REVIEW: this is the way it worked in the old explorer winmain, is this still correct?
        ShellNotifyCallback(DLL_PROCESS_DETACH, GetCurrentProcessId(), 0);
    }

    return TRUE;
}

