/*---------------------------------------------------------------------------
               ----> DO NOT REMOVE THE FOLLOWING NOTICE <----

                  Copyright (c) 1993 - 2010 Datalight, Inc.
                       All Rights Reserved Worldwide.

  Datalight, Incorporated is a Washington State corporation currently located
  at:
        21520 30th Dr SE, Suite 110,      Tel:  425-951-8086
        Bothell, WA  98021                Fax:  425-951-8094
        USA                               Web:  http://www.datalight.com

  This software, including without limitation all source code and documen-
  tation, is a trade secret and the confidential property of Datalight, 
  Inc., protected under the copyright laws of the United States and other
  jurisdictions.  Patents may be pending.

  U.S. Government Restricted Rights:  Use, duplication, reproduction, or
  transfer of this commercial product and accompanying documentation is
  restricted in accordance with FAR 12.212 and DFARS 227.7202 and by a
  License Agreement.

  IN ADDITION TO COPYRIGHT AND PATENT LAW, THIS SOFTWARE IS PROTECTED UNDER
  CONTRACT(S) BETWEEN DATALIGHT, INC. AND THE LICENSEE ("BINDING AGREEMENTS").
  IF YOU ARE A LICENSEE, YOUR RIGHT, IF ANY, TO COPY, PUBLISH, MODIFY, OR
  OTHERWISE USE THE SOFTWARE, IS SUBJECT TO THE TERMS AND CONDITIONS OF THE
  BINDING AGREEMENTS.  BY USING THE SOFTWARE IN ANY MANNER, IN WHOLE OR IN
  PART, YOU AGREE TO BE BOUND BY THE TERMS OF THE BINDING AGREEMENTS.

  IF YOU ARE NOT A DATALIGHT LICENSEE, ANY USE MAY RESULT IN CIVIL AND
  CRIMINAL ACTION AGAINST YOU.  CONTACT DATALIGHT, INC. AT THE ADDRESS
  SET FORTH ABOVE IF YOU OBTAINED THIS SOFTWARE IN ERROR.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Description

    This module implements a generalized abstraction to allow TTY-style
    character-mode applications to run in a GUI scrollable window.

    This code must be fully compilable and usable under both normal Win32
    user mode, as well as Windows CE.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: winviewappfw.c $
    Revision 1.8  2010/11/01 16:30:54Z  garyp
    Updated to build cleanly with output disabled.
    Revision 1.7  2010/11/01 04:25:20Z  garyp
    Fixed to finally work properly under both Win32 and WinCE at the same
    time.  Fixed font scaling problems.  Modified so logging is handled at
    a higher level.
    Revision 1.6  2010/01/23 16:45:50Z  garyp
    Fixed to avoid "possible loss of data" warnings (no actual loss of data).
    Revision 1.5  2009/07/16 21:44:03Z  garyp
    Minor initialization fixes so things build cleanly with output disabled.
    Revision 1.4  2009/07/14 19:25:32Z  garyp
    Updated to use the standardized Windows settings.  Fixed to preload the
    current font information when the "Choose Font" option is used.
    Revision 1.3  2009/07/01 17:36:15Z  garyp
    Fixed to not error out if the window class is already registered, to allow
    use in environments such as the Control Panel.  Reduced the default font
    size when running on CE.  Updated error handling and debug code.
    Revision 1.2  2009/06/28 02:46:24Z  garyp
    Merged from the v3.0 branch.
    Revision 1.1.1.9  2008/12/12 04:49:58Z  garyp
    Properly conditioned some code.
    Revision 1.1.1.8  2008/12/04 23:01:20Z  garyp
    Updated to redirect input and output using the Service API.
    Revision 1.1.1.7  2008/11/05 03:15:48Z  garyp
    Fixed a cut-and-paste error in the previous rev.
    Revision 1.1.1.6  2008/11/05 02:04:36Z  garyp
    Fixed a stack corruption problem caused by examining a wide-character string
    using sizeof() instead of DCLDIMENSIONOF().  Modified to dynamically link
    with the Software Input Panel (SIP) code, rather than statically linking to
    it since it may not exist in some CE build configurations.  Updated to use
    the new "WinLogPrintf()" functionality to aid debugging.  Updated the
    documentation.
    Revision 1.1.1.5  2008/09/04 23:25:41Z  garyp
    First version which works for both general Windows and Windows CE.  Added
    support for the CE CommandBar.  Updated to dynamically resize when the SIP
    (soft keyboard) is activated or deactivated.
    Revision 1.1.1.4  2008/08/25 17:56:55Z  garyp
    Added backspace support.  Moved the Help->About functionality into a
    separately abstracted module.  Added caret support.
    Revision 1.1.1.3  2008/07/28 08:20:07Z  garyp
    Updated to compile with output disabled.
    Revision 1.1.1.2  2008/07/27 23:23:32Z  garyp
    Commented out CE specific code so we can build again for XP.
    Revision 1.1  2008/07/27 02:48:08Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <windows.h>
#include <string.h>
#include <limits.h>
#include <commctrl.h>
#include <commdlg.h>
#include <stdio.h>
#include <setjmp.h>
#ifdef _WIN32_WCE
#include <sipapi.h>
#endif

#include <dcl.h>
#include <dlver.h>
#include <dlservice.h>
#include <dlwinutil.h>
#include <winviewapi.h>
#include <winviewappfw.h>
#include <winviewappfw.rh>
#include "winutilgui.rh"

#define MAX_COMMAND_LINE            (256)
#define BUFFER_GROWTH              (4096)
#define SMALL_SCREEN_FONT_HEIGHT    (-11)
#define LARGE_SCREEN_FONT_HEIGHT    (-20)
#define DEFAULT_FIXED_FONT          FW_NORMAL, FALSE, DEFAULT_PITCH | FF_DONTCARE, TEXT("Courier New")
#define DEFAULT_VARIABLE_FONT       FW_NORMAL, FALSE, VARIABLE_PITCH | FF_DONTCARE, TEXT("System")

#ifdef _WIN32_WCE
  typedef BOOL (*PFNSIPGETINFO)(SIPINFO *pSipInfo);
  typedef int  (*PFNCMDBARHEIGHT)(HWND hWndCB);
  typedef BOOL (*PFNCMDBARDRAWMENUBAR)(HWND hWndCB, WORD iButton);
  typedef void (*PFNCMDBARALIGNADORNMENTS)(HWND hWndCB);
#endif

typedef struct
{
    HINSTANCE               hInstance;
    HICON                   hIcon;
    HVIEW                   hView;
    HFONT                   hFont;
    HGLOBAL                 hBuffer;
    D_BUFFER               *pBuffer;
    D_UINT32                ulBufferLen;
    D_UINT32                ulBufferPos;
    HWND                    hAppWnd;
    HMENU                   hMenu;
    DCLWINSETTINGS          Settings;
    unsigned                fIsCE : 1;
    unsigned                fSmallScreen : 1;
    unsigned                fQuitRequest : 1;
    unsigned                fPaused : 1;
    jmp_buf                 jbEarlyTermination;
    const D_WCHAR          *pwzAppName;
    const D_WCHAR          *pwzAppDescription;
    LOGFONT                 LogFont;
  #if DCLCONF_OUTPUT_ENABLED
    DCLSERVICEHANDLE        hOutputService;
  #endif
  #ifdef _WIN32_WCE
    HMODULE                 hModCommCtrl;
    HWND                    hWndCmdBar;
    PFNCMDBARHEIGHT         pfnCmdBarHeight;
    PFNCMDBARDRAWMENUBAR    pfnCmdBarDrawMenuBar;
    PFNCMDBARALIGNADORNMENTS pfnCmdBarAlignAdornments;
    PFNSIPGETINFO           pfnSipGetInfo;
  #endif
} VIEWAPPFWDATA;

static LRESULT WINAPI   AppWndProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam);
static BOOL             ChooseSaveFile(HWND hWnd, LPTSTR pBuffer);
static void             ResetFont(HWND hWnd, LPLOGFONT pLF, VIEWAPPFWDATA *pOD);
static void             InitFontMenuItems(const VIEWAPPFWDATA *pOD);
static void             ExecuteMenuCommand(HWND hWnd, WPARAM wParam, VIEWAPPFWDATA *pOD);
static DCLSTATUS        InputChar(void *pContext, D_UCHAR *puChar, unsigned nFlags);
static void             UpdatePauseGoMenu(HWND hWnd, LPWSTR pwzText, VIEWAPPFWDATA *pOD);
static D_BOOL           IsMenuItemChecked(HMENU hMenu, UINT uID);
static HFONT            CreateAppFont(VIEWAPPFWDATA *pOD, LONG lHeight, LONG lWeight, BOOL fItalic, BYTE bPitchAndFamily, const D_WCHAR *pwzFaceName);
#if DCLCONF_OUTPUT_ENABLED
static void             AppOutputString(void *pContext, const char *pszString);
#endif
#ifdef _WIN32_WCE
  static DCLSTATUS      CeLoadControls(VIEWAPPFWDATA *pOD);
  static void           CeUnloadControls(VIEWAPPFWDATA *pOD);
#endif

HWND ghTempWnd;


/*-------------------------------------------------------------------
    Public: DclWinViewAppRun()

    This function runs the specified command in a GUI window.

    Parameters:
        hDclInst      - The DCL instance handle.
        nFlags        - The flags to use, which may be one or more of
                        the following values:
                        - WINVIEWAPPFLAGS_NOARGPROMPT - Don't prompt
                          for a command-line if none were supplied.
                        - WINVIEWAPPFLAGS_EXITWAIT - Upon completion
                          of the command, wait for an explicit
                          request to exit the app (require pressing
                          the Close button).
        hInstance     - The application instance handle.
        hPrevInstance - The previous instance handle.
        pwzCmdLine    - A pointer to the command-line arguments to
                        use.  May be NULL.
        nCmdShow      - The standard Windows "show" flags.
        pwzAppName    - The name of the application to display.
        pwzAppDesc    - The application description string.
        pfnCommand    - A pointer to the function to call to run the
                        application.
        pfnWndProc    - A WndProc to use.

    Return Value:
        Returns a DCLSTATUS code indicating the results.
-------------------------------------------------------------------*/
DCLSTATUS DclWinViewAppRun(
    DCLINSTANCEHANDLE       hDclInst,
    unsigned                nFlags,
    HINSTANCE               hInstance,
    HINSTANCE               hPrevInstance,
    D_WCHAR                *pwzCmdLine,     /* NULL indicates command takes no arguments */
    int                     nCmdShow,
    const D_WCHAR          *pwzAppName,
    const D_WCHAR          *pwzAppDesc,
    PFNWINCOMMAND           pfnCommand,
    WNDPROC                 pfnWndProc)
{
    MSG                     msg;
    VIEWAPPFWDATA           od = {0};
    DWORD                   dwStyle;
    DCLSTATUS               dclStat;
    DCLWINSETTINGS          DefaultSettings = {FALSE, 0, DEFAULT_FIXED_FONT};
  #ifdef _WIN32_WCE
    D_BOOL                  fCommandBarInitialized = FALSE;
  #endif

    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_WINAPPFW, 1, TRACEINDENT),
        "DclWinViewAppRun() hDclInst=%P Flags=%u hInst=%lX hInstPrev=%lX Cmd=\"%W\" Show=%d Name=\"%W\" Desc=\"%W\"\n",
        hDclInst, nFlags, hInstance, hPrevInstance, pwzCmdLine, nCmdShow, pwzAppName, pwzAppDesc));

    DclAssert(hDclInst);
    DclAssert(hInstance);
    DclAssertReadPtr(pwzAppName, 0);
    DclAssertReadPtr(pwzAppDesc, 0);
    DclAssertReadPtr(pfnCommand, 0);
    DclAssertReadPtr(pfnWndProc, 0);

    od.hInstance = hInstance;
    od.hIcon = LoadIcon(hInstance, TEXT("WINVIEWAPPFWICON"));
    od.pwzAppDescription = pwzAppDesc;
    od.pwzAppName = pwzAppName;

    /*  Any screen with total pixels of less than or equal to 640x480 is
        considered to be "small".
    */
    if((GetSystemMetrics(SM_CXSCREEN) * GetSystemMetrics(SM_CYSCREEN)) <= (640 * 480))
        od.fSmallScreen = TRUE;

    if(od.fSmallScreen)
        DefaultSettings.lFontHeight = SMALL_SCREEN_FONT_HEIGHT;
    else
        DefaultSettings.lFontHeight = LARGE_SCREEN_FONT_HEIGHT;

    DclWinSettingsLoad(pwzAppName, &DefaultSettings, &od.Settings);

  #ifdef _WIN32_WCE
    od.fIsCE = TRUE;
  #endif

    if(!hPrevInstance)
    {
        WNDCLASS    wndclass;

        /*  Register the main app window
        */
        wndclass.style          = CS_HREDRAW | CS_VREDRAW;
        wndclass.lpfnWndProc    = pfnWndProc;
        wndclass.cbClsExtra     = 0;
        wndclass.cbWndExtra     = 0;
        wndclass.hInstance      = hInstance;
        wndclass.hIcon          = od.hIcon;
      #ifdef _WIN32_WCE
        wndclass.hCursor        = 0;
        wndclass.lpszMenuName   = 0;
      #else
        wndclass.hCursor        = LoadCursor(0, IDC_ARROW);
        wndclass.lpszMenuName   = TEXT("WINVIEWAPPFWMENU");
      #endif
        wndclass.hbrBackground  = GetStockObject(WHITE_BRUSH);
        wndclass.lpszClassName  = pwzAppName;

        if(!RegisterClass(&wndclass))
        {
            DWORD dwError = GetLastError();

            /*  It is possible that the class will already exist, for
                example, in cases where this functionality is used from
                within the Control Panel.  Therefore an "already exists"
                error is not terminal.
            */
            if(dwError != ERROR_CLASS_ALREADY_EXISTS)
            {
                DCLPRINTF(1, ("DclWinViewAppRun() RegisterClass() failed with error %lX\n", dwError));

                msg.wParam = DCLSTAT_WINGUI_REGISTERCLASSFAILED;
                goto AppRunCleanup;
            }
        }
    }

    od.hFont = CreateAppFont(&od, od.Settings.lFontHeight, od.Settings.lFontWeight,
        od.Settings.fFontItalic, od.Settings.bFontPitchAndFamily, od.Settings.wzFontFaceName);

    DclAssert(od.hFont);

    dwStyle = WS_THICKFRAME;

    if(!od.fIsCE || !od.fSmallScreen)
    {
        /*  We use these explicit window styles rather than WS_OVERLAPPEDWINDOW
            to allow compatibility with Windows CE.
        */
        dwStyle |= WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX;
    }

    od.hAppWnd = CreateWindow(pwzAppName, pwzAppName, dwStyle,
                              CW_USEDEFAULT, 0, CW_USEDEFAULT, 0,
                              0, 0, hInstance, NULL);

    if(!od.hAppWnd)
    {
        DCLPRINTF(1, ("DclWinViewAppRun() CreateWindow() failed with error %lX\n", GetLastError()));

        msg.wParam = DCLSTAT_WINGUI_CREATEWINDOWFAILED;
        goto AppRunCleanup;
    }

    ghTempWnd = od.hAppWnd;

    SetWindowLong(od.hAppWnd, GWL_USERDATA, (LONG)&od);

    SendMessage(od.hAppWnd, WM_WINVIEW_SETFONT, (WPARAM)od.hFont, TRUE);

  #ifdef _WIN32_WCE
    {
        dclStat = CeLoadControls(&od);
        if(dclStat != DCLSTAT_SUCCESS)
        {
            DCLPRINTF(1, ("DclWinViewAppRun() CeLoadControls() failed with error %lX\n", dclStat));

            msg.wParam = dclStat;
            goto AppRunCleanup;
        }
        else
        {
            fCommandBarInitialized = TRUE;
        }
    }
  #else

    od.hMenu = GetMenu(od.hAppWnd);

  #endif

    InitFontMenuItems(&od);

    od.hBuffer = GlobalAlloc(GMEM_MOVEABLE, BUFFER_GROWTH);
    DclAssert(od.hBuffer);

    od.pBuffer = GlobalLock(od.hBuffer);
    DclAssert(od.pBuffer);

    od.ulBufferLen = BUFFER_GROWTH;
    od.ulBufferPos = 0;

    /*  Init to handle the error case
    */
    msg.wParam = 0;

    od.hView = DclWinViewObjectCreate(hInstance, od.hAppWnd, od.hFont,
        VIEWFLAGS_DYNAMIC|VIEWFLAGS_INPUT, 8, 0);


  #ifdef _WIN32_WCE
    if(!od.pfnCmdBarDrawMenuBar(od.hWndCmdBar, 0))
    {
        DclPrintf("LASTERROR=%lX\n", GetLastError());
    }
  #endif

    if(od.hView)
    {
        D_WCHAR             wzCmdLine[MAX_COMMAND_LINE];
        DCLINPUTINFO        ii = {NULL, InputChar};
        DCLDECLARESERVICE   (srvIn, "DLWININPUT", DCLSERVICE_INPUT, NULL, NULL, 0);
      #if DCLCONF_OUTPUT_ENABLED
        DCLOUTPUTINFO       oi = {NULL, AppOutputString};
        DCLDECLARESERVICE   (srvOut, "DLWINOUTPUT", DCLSERVICE_OUTPUT, NULL, NULL, 0);

        oi.pContext = &od;
        srvOut.pPrivateData = &oi;
      #endif

        ii.pContext = &od;
        srvIn.pPrivateData = &ii;

        ShowWindow(od.hAppWnd, nCmdShow);

      #if DCLCONF_OUTPUT_ENABLED
        /*  Create the output service to use
        */
        DclServiceCreate(hDclInst, &srvOut);
        od.hOutputService = &srvOut;
      #endif

        /*  Create the input service to use
        */
        DclServiceCreate(hDclInst, &srvIn);

        /*  If the flag to prompt for arguments if necessary is specified,
            or if a non-NULL command-line pointer is supplied...
        */
        if(pwzCmdLine && !wcslen(pwzCmdLine) && !(nFlags & WINVIEWAPPFLAGS_NOARGPROMPT))
        {
            DCLWINDLGEDITDATA  EditData;

            /*  No arguments were supplied, so put up an edit box
                and prompt the user for them.
            */
            wzCmdLine[0] = 0;

            EditData.pwzCaption = pwzAppName;
            EditData.pwzPrompt = TEXT("Command Options (/? for help)");
            EditData.pwzBuffer = wzCmdLine;
            EditData.nBufferLen = DCLDIMENSIONOF(wzCmdLine);

            if(DclWinDialogEdit(hInstance, &EditData) == 0)
                goto Cleanup;

            /*  pwzCmdLine being non-NULL indicates that there is a command
                line to pass to the client.
            */
            pwzCmdLine = wzCmdLine;
        }

        if(setjmp(od.jbEarlyTermination))
        {
            DCLPRINTF(1, ("Program terminated via setjmp()\n"));
        }
        else
        {
            dclStat = pfnCommand(hDclInst, pwzCmdLine);

            DCLPRINTF(2, ("Normal termination, Status=%lX\n", dclStat));

            UpdateWindow(od.hAppWnd);

            SetWindowText(od.hAppWnd, TEXT("Completed"));
            SendMessage(od.hAppWnd, WM_WINVIEW_SETSTATE, VIEWSTATE_COMPLETE, 0);
            EnableMenuItem(od.hMenu, IDM_PAUSE_GO, MF_BYCOMMAND | MF_GRAYED);

          #ifdef _WIN32_WCE
            od.pfnCmdBarDrawMenuBar(od.hWndCmdBar, 0);
          #else
            DrawMenuBar(od.hAppWnd);
          #endif

            /*  If we are not supposed to wait for an explicit exit
                request, automatically terminate the app.
            */
            if(!(nFlags & WINVIEWAPPFLAGS_EXITWAIT))
                DestroyWindow(od.hAppWnd);

            while(GetMessage(&msg, 0, 0, 0))
            {
                DCLWINMSGTRACEPRINT(MAKETRACEFLAGS(DCLTRACE_WINAPPFW, 1, TRACEINDENT),
                    "ViewApp:GetMessage() %s\n", &msg);

                TranslateMessage(&msg);
                DispatchMessage(&msg);

                DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_WINAPPFW, 1, TRACEUNDENT),
                    "ViewApp:GetMessage() complete\n"));
            }
        }

      Cleanup:

        /*  Restore the original input service
        */
        DclServiceDestroy(&srvIn);

      #if DCLCONF_OUTPUT_ENABLED
        /*  Restore the original output service
        */
        DclServiceDestroy(&srvOut);
      #endif

        DclWinViewObjectDestroy(od.hView);
    }

  AppRunCleanup:

  #ifdef _WIN32_WCE
    if(fCommandBarInitialized)
        CeUnloadControls(&od);
  #endif

    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_WINAPPFW, 1, TRACEUNDENT),
        "DclWinViewAppRun() returning %lX\n", msg.wParam));

    return msg.wParam;
}


/*-------------------------------------------------------------------
    Public: DclWinViewAppWndProc()

    This function is the standard message handler for the "ViewAppFW"
    system.  This function must be called by the WndProc function
    specified in the DclWinViewAppRun() function.

    Parameters:
        hWnd    - The window handle.
        nMsg    - The message number.
        wParam  - The WPARAM parameter.
        lParam  - The LPARAM parameter.

    Return Value:
        Returns a standard message LRESULT value.
-------------------------------------------------------------------*/
LRESULT DclWinViewAppWndProc(
    HWND                hWnd,
    UINT                nMsg,
    WPARAM              wParam,
    LPARAM              lParam)
{
    LRESULT             lResult = 0;
    static D_UINT32     ulPos = 0;
    VIEWAPPFWDATA      *pOD = NULL;

    DCLWINMESSAGETRACEPRINT(MAKETRACEFLAGS(DCLTRACE_WINAPPFW, 2, TRACEINDENT),
        "DclWinViewAppWndProc(%s)\n", hWnd, nMsg, wParam, lParam);

    switch(nMsg)
    {
        case WM_WINVIEW_GETDATALEN:
        {
            pOD = (VIEWAPPFWDATA*)GetWindowLong(hWnd, GWL_USERDATA);
            DclAssert(pOD);

            lResult = pOD->ulBufferPos;
            break;
        }

        case WM_WINVIEW_SETPOSITION:
        {
            pOD = (VIEWAPPFWDATA*)GetWindowLong(hWnd, GWL_USERDATA);
            DclAssert(pOD);

            if(lParam < 0)
                ulPos = 0;
            else if((D_UINT32)lParam > pOD->ulBufferPos)
                ulPos = pOD->ulBufferPos;
            else
                ulPos = lParam;

            lResult = (LPARAM)ulPos;

            break;
        }

        case WM_WINVIEW_RELATIVEPOSITION:
        {
            pOD = (VIEWAPPFWDATA*)GetWindowLong(hWnd, GWL_USERDATA);
            DclAssert(pOD);

            DclAssert(ulPos <= pOD->ulBufferPos);

            if(lParam > 0)
            {
                if(pOD->ulBufferPos - ulPos < (D_UINT32)lParam)
                    ulPos = pOD->ulBufferPos;
                else
                    ulPos += lParam;
            }
            else
            {
                if(ulPos >= (D_UINT32)lParam)
                    ulPos += lParam;
                else
                    ulPos = 0;
            }

            lResult = (LPARAM)ulPos;

            break;
        }

        case WM_WINVIEW_READDATA:
        {
            pOD = (VIEWAPPFWDATA*)GetWindowLong(hWnd, GWL_USERDATA);
            DclAssert(pOD);

            DclAssert(ulPos <= pOD->ulBufferPos);

            if(wParam > pOD->ulBufferPos - ulPos)
                wParam = pOD->ulBufferPos - ulPos;

            DclMemCpy((void*)lParam, pOD->pBuffer+ulPos, wParam);

            ulPos += wParam;

            lResult = wParam;

            break;
        }

        case WM_WINVIEW_RESERVEDY:
        {
          #ifdef _WIN32_WCE
            pOD = (VIEWAPPFWDATA*)GetWindowLong(hWnd, GWL_USERDATA);
            DclAssert(pOD);

            lResult = pOD->pfnCmdBarHeight(pOD->hWndCmdBar);
          #else
            lResult = 0;
          #endif

            break;
        }

      #ifdef _WIN32_WCE
        case WM_SETTINGCHANGE:
        {
            pOD = (VIEWAPPFWDATA*)GetWindowLong(hWnd, GWL_USERDATA);
            DclAssert(pOD);

            if(wParam == SPI_SETSIPINFO && pOD->pfnSipGetInfo)
            {
                SIPINFO si;

                DclMemSet(&si, 0, sizeof(si));
                si.cbSize = sizeof(si);

                if(pOD->pfnSipGetInfo(&si))
                {
                    MoveWindow(hWnd,
                        si.rcVisibleDesktop.left,
                        si.rcVisibleDesktop.top,
                        si.rcVisibleDesktop.right - si.rcVisibleDesktop.left,
                        si.rcVisibleDesktop.bottom - si.rcVisibleDesktop.top,
                        TRUE);
                }
            }
        }
        break;
      #endif

        case WM_HELP:
            wParam = IDM_HELPABOUT;
            /*  Fall through...
                    .
                    .
                    .
            */
        case WM_COMMAND:
        {
            pOD = (VIEWAPPFWDATA*)GetWindowLong(hWnd, GWL_USERDATA);
            DclAssert(pOD);

            ExecuteMenuCommand(hWnd, wParam, pOD);

            break;
        }

        case WM_DESTROY:
        {
            pOD = (VIEWAPPFWDATA*)GetWindowLong(hWnd, GWL_USERDATA);
            DclAssert(pOD);

            pOD->fQuitRequest = TRUE;

            SendMessage(pOD->hAppWnd, WM_WINVIEW_SETSTATE, VIEWSTATE_TERMINATING, 0);

            PostQuitMessage(0);

            break;
        }

        case WM_KEYDOWN:
        {
            pOD = (VIEWAPPFWDATA*)GetWindowLong(hWnd, GWL_USERDATA);
            DclAssert(pOD);

            if(wParam == VK_PAUSE)
            {
                pOD = (VIEWAPPFWDATA*)GetWindowLong(hWnd, GWL_USERDATA);
                DclAssert(pOD);

                ExecuteMenuCommand(hWnd, IDM_PAUSE_GO, pOD);
            }
            else if(wParam == VK_ESCAPE && pOD->Settings.fEscKeyToExit)
            {
                DestroyWindow(hWnd);
            }
            else
            {
                lResult = DclWinViewObjectDispatchMessage(hWnd, nMsg, wParam, lParam);
            }

            break;
        }

        case WM_SIZE:
          #ifdef _WIN32_WCE
            pOD = (VIEWAPPFWDATA*)GetWindowLong(hWnd, GWL_USERDATA);
            DclAssert(pOD);

            SendMessage(pOD->hWndCmdBar, TB_AUTOSIZE, 0L, 0L);
            pOD->pfnCmdBarAlignAdornments(pOD->hWndCmdBar);
          #endif

            /*  Fall through
                    .
                    .
                    .
            */

        case WM_CHAR:
        case WM_MOUSEWHEEL:
        case WM_SETFOCUS:
        case WM_KILLFOCUS:
        {
            lResult = DclWinViewObjectDispatchMessage(hWnd, nMsg, wParam, lParam);

            break;
        }

        default:
        {
            if(nMsg > WM_USER)
                lResult = DclWinViewObjectDispatchMessage(hWnd, nMsg, wParam, lParam);
            else
                lResult = DefWindowProc(hWnd, nMsg, wParam, lParam);

            break;
        }
    }

    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_WINAPPFW, 2, TRACEUNDENT),
        "DclWinViewAppWndProc() %s returning %lX\n", DclWinMessageName(nMsg), lResult));

    return lResult;
}


/*-------------------------------------------------------------------
-------------------------------------------------------------------*/
static void ExecuteMenuCommand(
    HWND            hWnd,
    WPARAM          wParam,
    VIEWAPPFWDATA  *pOD)
{
    DclAssert(hWnd);
    DclAssert(wParam);
    DclAssertWritePtr(pOD, sizeof(*pOD));

    switch(wParam)
    {
        case IDM_RUN_SAVEAS:
        {
            D_WCHAR awcTempBuff[MAX_PATH];

            if(ChooseSaveFile(hWnd, awcTempBuff))
            {
                FILE   *hFile;

                hFile = _wfopen(awcTempBuff, TEXT("a+b"));
                if(hFile)
                {
                    D_UINT32    ulRemaining = pOD->ulBufferPos;
                    D_BUFFER   *pData = pOD->pBuffer;

                    while(TRUE)
                    {
                        unsigned    nLen = 0;

                        while(ulRemaining && (pData[nLen] != '\n'))
                        {
                            ulRemaining--;
                            nLen++;
                        }

                        if(nLen)
                        {
                            fwrite(pData, 1, nLen, hFile);

                            pData += nLen;
                        }

                        if(!ulRemaining)
                            break;

                        pData++;
                        ulRemaining--;

                        if(!ulRemaining)
                            break;

                        fwrite("\r\n", 1, 2, hFile);
                    }

                    fclose(hFile);
                }
                else
                {
                    wsprintf(awcTempBuff, TEXT("Unable to open the file '%s'"), awcTempBuff);

                    MessageBox(hWnd, awcTempBuff, TEXT("Error"), MB_OK|MB_ICONEXCLAMATION);
                }
            }

            break;
        }

        case IDM_RUN_EXIT:
        {
            DestroyWindow(hWnd);

            break;
        }

        case IDM_VIEW_KEEPONTOP:
        {
            if(IsMenuItemChecked(pOD->hMenu, IDM_VIEW_KEEPONTOP))
            {
                SetWindowPos(hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

                CheckMenuItem(pOD->hMenu, IDM_VIEW_KEEPONTOP, MF_BYCOMMAND | MF_UNCHECKED);
            }
            else
            {
                SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

                CheckMenuItem(pOD->hMenu, IDM_VIEW_KEEPONTOP, MF_BYCOMMAND | MF_CHECKED);
            }

            break;
        }

        case IDM_VIEW_FONTCHOOSE:
        {
            CHOOSEFONT      cf;

            /*  Set all structure fields to zero.
            */
            DclMemSet(&cf, 0, sizeof(cf));

            cf.lStructSize = sizeof(cf);
            cf.hwndOwner   = hWnd;
            cf.lpLogFont   = &pOD->LogFont;
            cf.Flags       = CF_SCREENFONTS | CF_EFFECTS | CF_INITTOLOGFONTSTRUCT;

            if(ChooseFont(&cf))
            {
                ResetFont(hWnd, &pOD->LogFont, pOD);

                SendMessage(hWnd, WM_WINVIEW_SETFONT, (WPARAM)pOD->hFont, TRUE);
            }
            break;
        }

        case IDM_VIEW_FONTFIXED:
        {
            if(IsMenuItemChecked(pOD->hMenu, IDM_VIEW_FONTFIXED))
                break;

            DeleteObject(pOD->hFont);

            pOD->hFont = CreateAppFont(pOD,
                pOD->fSmallScreen ? SMALL_SCREEN_FONT_HEIGHT : LARGE_SCREEN_FONT_HEIGHT,
                DEFAULT_FIXED_FONT);

            DclAssert(pOD->hFont);

            SendMessage(hWnd, WM_WINVIEW_SETFONT, (WPARAM)pOD->hFont, TRUE);

            CheckMenuItem(pOD->hMenu, IDM_VIEW_FONTFIXED,    MF_BYCOMMAND | MF_CHECKED);
            CheckMenuItem(pOD->hMenu, IDM_VIEW_FONTVARIABLE, MF_BYCOMMAND | MF_UNCHECKED);

            break;
        }

        case IDM_VIEW_FONTVARIABLE:
        {
            if(IsMenuItemChecked(pOD->hMenu, IDM_VIEW_FONTVARIABLE))
                break;

            DeleteObject(pOD->hFont);

            pOD->hFont = CreateAppFont(pOD,
                pOD->fSmallScreen ? SMALL_SCREEN_FONT_HEIGHT : LARGE_SCREEN_FONT_HEIGHT,
                DEFAULT_VARIABLE_FONT);

            DclAssert(pOD->hFont);

            SendMessage(hWnd, WM_WINVIEW_SETFONT, (WPARAM)pOD->hFont, TRUE);

            CheckMenuItem(pOD->hMenu, IDM_VIEW_FONTFIXED,    MF_BYCOMMAND | MF_UNCHECKED);
            CheckMenuItem(pOD->hMenu, IDM_VIEW_FONTVARIABLE, MF_BYCOMMAND | MF_CHECKED);

            break;
        }

        case IDM_VIEW_SETTINGS:
        {
            DclWinSettingsEdit(pOD->hInstance, pOD->hAppWnd, &pOD->Settings);

            break;
        }

        case IDM_VIEW_SAVESETTINGS:
        {
            pOD->Settings.lFontHeight = pOD->LogFont.lfHeight;
            pOD->Settings.lFontWeight = pOD->LogFont.lfWeight;
            pOD->Settings.fFontItalic = pOD->LogFont.lfItalic;
            pOD->Settings.bFontPitchAndFamily = pOD->LogFont.lfPitchAndFamily;
            wcsncpy(pOD->Settings.wzFontFaceName, pOD->LogFont.lfFaceName, DCLDIMENSIONOF(pOD->Settings.wzFontFaceName));

            DclWinSettingsSave(pOD->pwzAppName, &pOD->Settings);

            break;
        }

        case IDM_VIEW_DELETESETTINGS:
        {
            DclWinSettingsDelete(pOD->pwzAppName);

            MessageBox(hWnd, TEXT("Restart the application to start using the default settings."), 
                             TEXT("Custom Registry Settings Deleted"), 
                             MB_OK|MB_ICONINFORMATION);

            break;
        }

        case IDM_PAUSE_GO:
        {
            LRESULT         lResult;

            lResult = SendMessage(hWnd, WM_WINVIEW_PAUSEGO, 0, 0);

            if(lResult == VIEWSTATE_RUNNING)
            {
                pOD->fPaused = TRUE;

                UpdatePauseGoMenu(hWnd, TEXT("  &Go!  "), pOD);
            }
            else if(lResult == VIEWSTATE_PAUSED)
            {
                pOD->fPaused = FALSE;

                UpdatePauseGoMenu(hWnd, TEXT("&Pause!"), pOD);
            }

            break;
        }

        case IDM_HELPABOUT:
        {
            DclWinHelpAbout(hWnd, pOD->pwzAppDescription, DclSignOn(TRUE), NULL);

            break;
        }

        default:
        {
            DCLPRINTF(1, ("ViewApp:ExecuteMenuCommand() unhandled command %lX\n", wParam));
            DclError();
        }
    }

    return;
}


/*-------------------------------------------------------------------
-------------------------------------------------------------------*/
static BOOL ChooseSaveFile(
    HWND            hWnd,
    LPTSTR          pBuffer)
{
    D_WCHAR         wzFile[MAX_PATH];
    OPENFILENAME    ofn;

    DclAssertWritePtr(pBuffer, 0);

    /*  clear and fill our file name structure
    */
    DclMemSet(&ofn, 0, sizeof(ofn));

    ofn.hwndOwner   = hWnd;
    ofn.lStructSize = sizeof(ofn);
    ofn.Flags       = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;
    ofn.lpstrFile   = wzFile;
    ofn.lpstrFilter = TEXT("Text Files\0*.txt\0All Files\0*.*\0\0");
    ofn.nMaxFile    = DCLDIMENSIONOF(wzFile);

    /*  set a default file name
    */
    wcscpy(wzFile, TEXT(""));

    /*  get the user's input
    */
    if(GetSaveFileName(&ofn))
    {
        wcscpy(pBuffer, wzFile);

        return TRUE;
    }

    return FALSE;
}


/*-------------------------------------------------------------------
-------------------------------------------------------------------*/
static void ResetFont(
    HWND            hWnd,
    LPLOGFONT       pLF,
    VIEWAPPFWDATA  *pOD)
{
    DclAssertReadPtr(pLF, sizeof(*pLF));
    DclAssertWritePtr(pOD, sizeof(*pOD));

    if(pOD->hFont)
        DeleteObject(pOD->hFont);

    /*  create the font
    */
    pOD->hFont = CreateFontIndirect(pLF);
    DclAssert(pOD->hFont);

    CheckMenuItem(pOD->hMenu, IDM_VIEW_FONTFIXED,    MF_BYCOMMAND | MF_UNCHECKED);
    CheckMenuItem(pOD->hMenu, IDM_VIEW_FONTVARIABLE, MF_BYCOMMAND | MF_UNCHECKED);

    return;
}


/*-------------------------------------------------------------------
    This function determines whether the font information in the
    settings data matches that for the default FIXED or VARIABLE
    fonts, and if so, checks the appropriate menu item.
-------------------------------------------------------------------*/
static void InitFontMenuItems(
    const VIEWAPPFWDATA  *pOD)
{
    DCLWINSETTINGS  Fixed    = {0, 0, DEFAULT_FIXED_FONT};
    DCLWINSETTINGS  Variable = {0, 0, DEFAULT_VARIABLE_FONT};
    LONG            lHeight;

    DclAssertReadPtr(pOD, sizeof(*pOD));

    if(pOD->fSmallScreen)
        lHeight = SMALL_SCREEN_FONT_HEIGHT;
    else
        lHeight = LARGE_SCREEN_FONT_HEIGHT;

    if( pOD->Settings.lFontHeight == lHeight &&
        pOD->Settings.lFontWeight == Fixed.lFontWeight &&
        pOD->Settings.fFontItalic == Fixed.fFontItalic &&
        pOD->Settings.bFontPitchAndFamily == Fixed.bFontPitchAndFamily &&
        wcscmp(pOD->Settings.wzFontFaceName, Fixed.wzFontFaceName) == 0)
    {
        CheckMenuItem(pOD->hMenu, IDM_VIEW_FONTFIXED, MF_BYCOMMAND | MF_CHECKED);
        return;
    }

    if( pOD->Settings.lFontHeight == lHeight &&
        pOD->Settings.lFontWeight == Variable.lFontWeight &&
        pOD->Settings.fFontItalic == Variable.fFontItalic &&
        pOD->Settings.bFontPitchAndFamily == Variable.bFontPitchAndFamily &&
        wcscmp(pOD->Settings.wzFontFaceName, Variable.wzFontFaceName) == 0)
    {
        CheckMenuItem(pOD->hMenu, IDM_VIEW_FONTVARIABLE, MF_BYCOMMAND | MF_CHECKED);
        return;
    }

    /*  Doesn't match either one
    */
    return;
}


/*-------------------------------------------------------------------
-------------------------------------------------------------------*/
static void UpdatePauseGoMenu(
    HWND            hWnd,
    LPWSTR          pwzText,
    VIEWAPPFWDATA  *pOD)
{
    MENUITEMINFO    mii;

    DclAssert(hWnd);
    DclAssertWritePtr(pwzText, 0);
    DclAssertWritePtr(pOD, sizeof(*pOD));

    DclMemSet(&mii, 0, sizeof(mii));

    mii.cbSize = sizeof(mii);
    mii.fMask = MIIM_TYPE;
    mii.fType = MFT_STRING;
    mii.dwTypeData = pwzText;

    SetMenuItemInfo(pOD->hMenu, IDM_PAUSE_GO, FALSE, &mii);

  #ifdef _WIN32_WCE
    pOD->pfnCmdBarDrawMenuBar(pOD->hWndCmdBar, 0);
  #else
    DrawMenuBar(hWnd);
  #endif

    return;
}


/*-------------------------------------------------------------------
-------------------------------------------------------------------*/
static D_BOOL IsMenuItemChecked(
    HMENU           hMenu,
    UINT            uID)
{
    MENUITEMINFO    mii;

    DclAssert(hMenu);

    DclMemSet(&mii, 0, sizeof(mii));

    mii.cbSize = sizeof(mii);
    mii.fMask = MIIM_STATE;

    if(GetMenuItemInfo(hMenu, uID, FALSE, &mii))
        return (D_BOOL)(mii.fState & MF_CHECKED);
    else
        return FALSE;
}


/*-------------------------------------------------------------------
-------------------------------------------------------------------*/
static HFONT CreateAppFont(
    VIEWAPPFWDATA  *pOD,
    LONG            lHeight,
    LONG            lWeight,
    BOOL            fItalic,
    BYTE            bPitchAndFamily,
    const D_WCHAR  *pwzFaceName)
{
    DclAssertWritePtr(pOD, sizeof(*pOD));
    DclAssertReadPtr(pwzFaceName, 0);

    /*  Use CreateFontIndirect() rather than CreateFont() for
        compatibility with Windows CE.
    */

    DclMemSet(&pOD->LogFont, 0, sizeof(pOD->LogFont));

    pOD->LogFont.lfHeight = lHeight;
    pOD->LogFont.lfWeight = lWeight;
    pOD->LogFont.lfItalic = fItalic ? TRUE : FALSE;
    pOD->LogFont.lfPitchAndFamily = bPitchAndFamily;
    wcsncpy(pOD->LogFont.lfFaceName, pwzFaceName, DCLDIMENSIONOF(pOD->LogFont.lfFaceName));

    return CreateFontIndirect(&pOD->LogFont);
}


/*-------------------------------------------------------------------
    Public: DclOsInputChar()

    This is the workhorse function for obtaining character input.

    When this function is used by common, OS independent code, it
    <must> be assumed that the OS interface may wait for a character,
    or even the [Enter] key before returning from this function.

    When this function is used from OS specific code, where the OS
    characteristics are known, it may return immediately with a
    DCLSTAT_INPUT_IDLE status.

    Parameters:
        pContext - A pointer to any context data which may have been
                   specified when the input service was registered.
        puChar   - A pointer to a buffer to receive the character.
                   The contents of this field will only be modified
                   if DCLSTAT_SUCCESS is returned.
        nFlags   - This parameter is reserved for future use and must
                   be 0.

    Return Value:
        Returns one of the following DCLSTATUS values:

        DCLSTAT_SUCCESS           - A character was returned
        DCLSTAT_INPUT_DISABLED    - The input interface is disabled
        DCLSTAT_INPUT_IDLE        - There is no input ready
        DCLSTAT_INPUT_TERMINATE   - The OS is terminating the program
-------------------------------------------------------------------*/
static DCLSTATUS InputChar(
    void           *pContext,
    D_UCHAR        *puChar,
    unsigned        nFlags)
{
    VIEWAPPFWDATA  *pOD = pContext;
    LRESULT         lResult = 0;

    DclAssertWritePtr(puChar, 0);
    DclAssertWritePtr(pOD, sizeof(*pOD));

    while(!lResult)
    {
        MSG msg;

        if(pOD->fQuitRequest)
            return DCLSTAT_INPUT_TERMINATE;

/*        SendMessage(pOD->hAppWnd, WM_PAINT, 0, 0);*/

        if(PeekMessage(&msg, pOD->hAppWnd, 0, 0, PM_REMOVE))
        {
            DCLWINMSGTRACEPRINT(MAKETRACEFLAGS(DCLTRACE_WINAPPFW, 2, TRACEINDENT),
                "ViewApp:InputChar() %s\n", &msg);

            TranslateMessage(&msg);
            DispatchMessage(&msg);

            DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_WINAPPFW, 2, TRACEUNDENT),
                "ViewApp:InputChar() complete\n"));
        }

        lResult = SendMessage(pOD->hAppWnd, WM_WINVIEW_GETKEY, 0, 0);

        if(!lResult)
            return DCLSTAT_INPUT_IDLE;
    }

    if(lResult == '\r')
        lResult = '\n';

    DCLPRINTF(2, ("ViewApp:InputChar() Got key %lX\n", lResult));

    *puChar = (D_UCHAR)lResult;

    return DCLSTAT_SUCCESS;
}


#if DCLCONF_OUTPUT_ENABLED

/*-------------------------------------------------------------------
-------------------------------------------------------------------*/
static void AppOutputString(
    void               *pContext,
    const char         *pszString)
{
    VIEWAPPFWDATA      *pOD = pContext;
    static D_BOOL       fBusy = 0;

    DclAssertReadPtr(pszString, 0);
    DclAssertWritePtr(pOD, sizeof(*pOD));

    {
        DCLSTATUS       dclStat;
        DCLOUTPUTINFO  *pOut;

        dclStat = DclServiceDataPrevious(pOD->hOutputService, (void**)&pOut);
        if(dclStat == DCLSTAT_SUCCESS && pOut)
        {
            (*pOut->pfnOutputString)(pOut->pContext, pszString);
        }
    }

    if(pOD->fQuitRequest)
    {
        static unsigned nQuitCount = 0;

        if(nQuitCount++ == 10)
            longjmp(pOD->jbEarlyTermination, 1);
    }

    if(++fBusy == 1)
    {
        MSG         msg;
        unsigned    nGetMessageLoops = 1;   /* default to 1 GetMessage() call */
        unsigned    nLen;

        DclAssert(pszString);
        DclAssert(pOD);
        DclAssert(pOD->pBuffer);
        DclAssert(pOD->ulBufferLen >= pOD->ulBufferPos);

        nLen = DclStrLen(pszString);

        if(pOD->ulBufferLen - pOD->ulBufferPos < nLen)
        {
            pOD->ulBufferLen += BUFFER_GROWTH;

            GlobalUnlock(pOD->hBuffer);

            pOD->hBuffer = GlobalReAlloc(pOD->hBuffer, pOD->ulBufferLen, GMEM_MOVEABLE);
            DclAssert(pOD->hBuffer);

            pOD->pBuffer = GlobalLock(pOD->hBuffer);
            DclAssert(pOD->pBuffer);
        }

        while(nLen)
        {
            if(*pszString == '\b')
            {
                DclAssert(pOD->ulBufferPos);

                pOD->ulBufferPos--;
            }
            else
            {
                pOD->pBuffer[pOD->ulBufferPos] = *pszString;
                pOD->ulBufferPos++;
            }

            pszString++;
            nLen--;
        }

/*      SendMessage(pOD->hAppWnd, WM_WINVIEW_UPDATEWINDOW, 0, 0);   */

        InvalidateRect(pOD->hAppWnd, NULL, TRUE);
        UpdateWindow(pOD->hAppWnd);

        /*  So long has we have not received a quit request, and either
            the count of "GetMessageLoops" is non-zero, or we are in
            "pause" mode, keep looping.
        */
        while(!pOD->fQuitRequest && (nGetMessageLoops || pOD->fPaused))
        {
            SendMessage(pOD->hAppWnd, WM_WINVIEW_UPDATEWINDOW, 0, 0); 

            if(PeekMessage(&msg, pOD->hAppWnd, 0, 0, PM_REMOVE))
/*          if(GetMessage(&msg, 0, 0, 0)) */
            {
                DCLWINMSGTRACEPRINT(MAKETRACEFLAGS(DCLTRACE_WINAPPFW, 2, TRACEINDENT),
                    "ViewApp:AppOutputString() %s\n", &msg);

                TranslateMessage(&msg);
                DispatchMessage(&msg);

                DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_WINAPPFW, 2, TRACEUNDENT),
                    "ViewApp:AppOutputString() complete\n"));
            }
            else if(!pOD->fQuitRequest && pOD->fPaused)
            {
                /*  Don't eat the CPU if we are in "pause" mode...
                */
                DclOsSleep(1);
            }

            if(nGetMessageLoops)
                nGetMessageLoops--;
        }
/*
        if(!pOD->fQuitRequest)
            SendMessage(pOD->hAppWnd, WM_WINVIEW_SETSTATE, VIEWSTATE_RUNNING, 0);
*/
    }

    fBusy--;

    return;
}

#endif


#ifdef _WIN32_WCE

typedef HWND (*PFNCMDBARCREATE)(HINSTANCE hInst, HWND hWndParent, int idCmdBar);
typedef void (*PFNCMDBARDESTROY)(HWND hWndCB);
typedef BOOL (*PFNCMDBARINSERTMENUBAREX)(HWND hWndCB, HINSTANCE hInst, LPTSTR pszMenu, WORD iButton);
typedef BOOL (*PFNCMDBARADDADORNMENTS)(HWND hWndCB, DWORD dwFlags, DWORD dwReserved);

/*-------------------------------------------------------------------
-------------------------------------------------------------------*/
static DCLSTATUS CeLoadControls(
    VIEWAPPFWDATA              *pOD)
{
    PFNCMDBARCREATE             pfnCmdBarCreate;
    PFNCMDBARADDADORNMENTS      pfnCmdBarAddAdornments;
    PFNCMDBARINSERTMENUBAREX    pfnCmdBarInsertMenuBarEx;
    DCLSTATUS                   dclStat = DCLSTAT_SUCCESS;
    HMODULE                     hModCoreDLL;

    DclAssertWritePtr(pOD, sizeof(*pOD));
    DclAssert(pOD->hModCommCtrl == NULL);
    DclAssert(pOD->hWndCmdBar == NULL);
    DclAssert(pOD->hMenu == NULL);

    /*  Load coredll.dll so we have the module handle for GetProcAddress()
    */
    hModCoreDLL = LoadLibrary(TEXT("coredll.dll"));
    if(!hModCoreDLL)
    {
        /*  Really not expected to fail, but not a critical error in
            any case...
        */
        DCLPRINTF(1, ("CeLoadControls() failed to load coredll.dll!\n"));
    }
    else
    {
        pOD->pfnSipGetInfo = (PFNSIPGETINFO)GetProcAddress(hModCoreDLL, TEXT("SipGetInfo"));
        if(!pOD->pfnSipGetInfo)
            DCLPRINTF(1, ("ViewApp:CeLoadControls() SIP support is disabled\n"));

        /*  Since it is safe to assume that coredll.dll is <always> going
            to be loaded, just free the library now so that we don't have
            to do it at cleanup time.
        */
        FreeLibrary(hModCoreDLL);
    }

    pOD->hModCommCtrl = DclWinCommonControlsLoad(ICC_BAR_CLASSES);
    if(!pOD->hModCommCtrl)
    {
        DCLPRINTF(1, ("ViewApp:CeLoadControls() failed to load common controls\n"));

        dclStat = DCLSTAT_WINGUI_LOADCMNCONTROLSAILED;
        goto CBCreateCleanup;
    }

    pfnCmdBarCreate = (PFNCMDBARCREATE)GetProcAddress(pOD->hModCommCtrl, TEXT("CommandBar_Create"));
    if(!pfnCmdBarCreate)
    {
        DCLPRINTF(1, ("ViewApp:CeLoadControls() failed to find proc address for CommandBar_Create()\n"));

        dclStat = DCLSTAT_WINGUI_GETPROCADDRESSFAILED;
        goto CBCreateCleanup;
    }

    pfnCmdBarAddAdornments = (PFNCMDBARADDADORNMENTS)GetProcAddress(pOD->hModCommCtrl, TEXT("CommandBar_AddAdornments"));
    if(!pfnCmdBarAddAdornments)
    {
        DCLPRINTF(1, ("ViewApp:CeLoadControls() failed to find proc address for CommandBar_AddAdornments()\n"));

        dclStat = DCLSTAT_WINGUI_GETPROCADDRESSFAILED;
        goto CBCreateCleanup;
    }

    pfnCmdBarInsertMenuBarEx = (PFNCMDBARINSERTMENUBAREX)GetProcAddress(pOD->hModCommCtrl, TEXT("CommandBar_InsertMenubarEx"));
    if(!pfnCmdBarInsertMenuBarEx)
    {
        DCLPRINTF(1, ("ViewApp:CeLoadControls() failed to find proc address for CommandBar_InsertMenubarEx()\n"));

        dclStat = DCLSTAT_WINGUI_GETPROCADDRESSFAILED;
        goto CBCreateCleanup;
    }

    pOD->pfnCmdBarDrawMenuBar = (PFNCMDBARDRAWMENUBAR)GetProcAddress(pOD->hModCommCtrl, TEXT("CommandBar_DrawMenuBar"));
    if(!pOD->pfnCmdBarDrawMenuBar)
    {
        DCLPRINTF(1, ("ViewApp:CeLoadControls() failed to find proc address for CommandBar_DrawMenuBar()\n"));

        dclStat = DCLSTAT_WINGUI_GETPROCADDRESSFAILED;
        goto CBCreateCleanup;
    }

    pOD->pfnCmdBarHeight = (PFNCMDBARHEIGHT)GetProcAddress(pOD->hModCommCtrl, TEXT("CommandBar_Height"));
    if(!pOD->pfnCmdBarHeight)
    {
        DCLPRINTF(1, ("ViewApp:CeLoadControls() failed to find proc address for CommandBar_Height()\n"));

        dclStat = DCLSTAT_WINGUI_GETPROCADDRESSFAILED;
        goto CBCreateCleanup;
    }

    pOD->pfnCmdBarAlignAdornments = (PFNCMDBARALIGNADORNMENTS)GetProcAddress(pOD->hModCommCtrl, TEXT("CommandBar_AlignAdornments"));
    if(!pOD->pfnCmdBarAlignAdornments)
    {
        DCLPRINTF(1, ("ViewApp:CeLoadControls() failed to find proc address for CommandBar_AlignAdornments()\n"));

        dclStat = DCLSTAT_WINGUI_GETPROCADDRESSFAILED;
        goto CBCreateCleanup;
    }

    pOD->hMenu = LoadMenu(pOD->hInstance, TEXT("WINVIEWAPPFWMENU"));
    if(!pOD->hMenu)
    {
        DCLPRINTF(1, ("ViewApp:CeLoadControls() failed to load the menu\n"));

        dclStat = DCLSTAT_WINGUI_LOADMENUFAILED;
        goto CBCreateCleanup;
    }

    if(pOD->fSmallScreen)
    {
        /*  For CE we have a help button, so remove the menu item if this
            is considered a small screen.
        */
        RemoveMenu(pOD->hMenu, IDM_POS_HELP, MF_BYPOSITION);
    }

    pOD->hWndCmdBar = pfnCmdBarCreate(pOD->hInstance, pOD->hAppWnd, 7777);
    if(!pOD->hWndCmdBar)
    {
        DCLPRINTF(1, ("ViewApp:CeLoadControls() error creating the command bar\n"));

        dclStat = DCLSTAT_WINGUI_CMDBARCREATEFAILED;
        goto CBCreateCleanup;
    }

    if(!pfnCmdBarInsertMenuBarEx(pOD->hWndCmdBar, NULL, (LPTSTR)pOD->hMenu, 0))
    {
        DCLPRINTF(1, ("ViewApp:CeLoadControls() insert menu bar failed\n"));

        dclStat = DCLSTAT_WINGUI_CMDBARMENUINSERTFAILED;
        goto CBCreateCleanup;
    }

    if(!pfnCmdBarAddAdornments(pOD->hWndCmdBar, CMDBAR_HELP, 0))
    {
        DCLPRINTF(1, ("ViewApp:CeLoadControls() add adornments failed\n"));

        dclStat = DCLSTAT_WINGUI_CMDBARADORNMENTSFAILED;
        goto CBCreateCleanup;
    }

/*    pOD->nReservedHeight = pOD->pfnCmdBarHeight(pOD->hWndCmdBar); */

    if(!pOD->pfnCmdBarDrawMenuBar(pOD->hWndCmdBar, 0))
        DclError();

  CBCreateCleanup:
    if(dclStat != DCLSTAT_SUCCESS)
    {
        DCLPRINTF(1, ("ViewApp:CeLoadControls() failed with status %lX\n", dclStat));

        if(pOD->hWndCmdBar)
        {
            PFNCMDBARDESTROY    pfnCmdBarDestroy;

            pfnCmdBarDestroy = (PFNCMDBARDESTROY)GetProcAddress(pOD->hModCommCtrl, TEXT("CommandBar_Destroy"));

            pfnCmdBarDestroy(pOD->hWndCmdBar);

            pOD->hWndCmdBar = NULL;
        }

        if(pOD->hMenu)
        {
            DestroyMenu(pOD->hMenu);

            pOD->hMenu = NULL;
        }

        if(pOD->hModCommCtrl)
        {
            DclWinCommonControlsUnload(pOD->hModCommCtrl);

            pOD->hModCommCtrl = NULL;
        }
    }

    return dclStat;
}


/*-------------------------------------------------------------------
-------------------------------------------------------------------*/
static void CeUnloadControls(
    VIEWAPPFWDATA  *pOD)
{
    DclAssertWritePtr(pOD, sizeof(*pOD));
    DclAssert(pOD->hModCommCtrl);

    DclWinCommonControlsUnload(pOD->hModCommCtrl);

    pOD->hModCommCtrl = NULL;
}


#endif


