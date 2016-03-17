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
#ifndef _H_TASKBAR_
#define _H_TASKBAR_

#ifndef __LIST_H__
#include "list.hpp"
#endif

#include "desktop.h"
#include <shsdkstc.h>

extern TCHAR const c_szEllipses[];
extern TCHAR const c_szExplore[];
extern TCHAR const c_szExplorer[] ;
extern TCHAR const c_szClock[];
extern TCHAR const c_szShellKeys[];
extern TCHAR const c_szShellAutoHide[];
extern TCHAR const c_szShellOnTop[];
extern TCHAR const c_szShowClock[];
extern TCHAR const c_szExpandControlPanel[];

class CHtmlBubble;
class TaskbarItem;
class TaskbarApplicationButton;
class TaskbarBubble;
class PowerManagerUI;
class TaskManDlg;
class ExtrasMenuButton;

#define HIBERNATE_BELOW             0x20000 //  128KB
#define HIBERNATE_BELOW_PAGES       40
#define HIBERNATE_EXEC_ABOVE        0x06000 //  24KB
#define HIBERNATE_BROWSE_ABOVE      0x08000 //  32KB
#define HIBERNATE_FREEZE_CACHE      0x10000 //  64KB

#define HHTBS_UP             0x00000001
#define HHTBS_DOWN           0x00000002
#define HHTBS_BUTTON         0x00000004
#define HHTBS_STATE          0x00000008
#define HHTBS_STATUS         0x00000010
#define HHTBS_CHECKED        0x00000020
#define HHTBS_NOTIFY         0x00000040
#define HHTBS_BUBBLE_NOTIFY  0x00000080
#define HHTBS_INPUTLANG_NOTIFY  0x00000100

#define HHTBF_DESTROYICON    0x10000000
#define HHTBF_ACTIVATE       0x20000000

#define HHTB_START           (HWND)0
#define HHTB_TRAY            (HWND)(UINT)-1
#define HHTB_FIRSTNOTIFY     (HWND)(UINT)-2
#define HHTB_DESKTOP         (HWND)(UINT)-3
#define HHTB_SIP             (HWND)(UINT)-4
#define HHTB_EXTRA           (HWND)(UINT)-5
#define TRAY_SPINNER_DOWN    -6
#define TRAY_SPINNER_UP      -7

#define CCHMAXTBLABEL        MAX_PATH

#define NIM_BUBBLE_ADD      0x00000004
#define NIM_BUBBLE_MODIFY   0x00000008
#define NIM_BUBBLE_DELETE   0x00000010

#ifndef GetTextExtentPoint
#define GetTextExtentPoint(hdc, psz, cb, psize)\
        GetTextExtentExPoint(hdc, psz, cb, 1000, NULL, NULL, psize)
#endif

/*
 *
  Notification Codes
 */
#define HHTBN_CLICKED          0

#define HHTASKBARDATA          8
#define HHTASKBARCLASSNAME     TEXT("HHTaskBar")
#define HHTASKBARID            0
#define HHTASKBARSTATE         4

#define HHTRAYCLASSNAME        TEXT("HHTaskBarTray")


/////////////////////////////////////////////////////////////////////////////

typedef struct tagTBOBJ {
    LISTOBJ lpObj;
    TaskbarItem *ptbi;
} TBOBJ, FAR* LPTBOBJ;

const int IDT_AUTOUNHIDE = 5;
const int IDT_AUTOHIDE = 6;
const int IDT_ADMIN = 42;
const int IDT_NOTIFICATION_ICON_UPDATE = 43;

#define MK_LWIN    0x40


#define _SEPERATOR             2
#define _NUMITEMS              3
#define TOPLINE               2
#define BOTTOMLINE            25
#define TASKBAR_HEIGHT        26
#define TASKBAR_HEIGHT_AUTOHIDE 5
#define TRAY_SPINNERWIDTH  5
#define EXTRAS_BUTTON_WIDTH 23
#define DESKTOP_BUTTON_WIDTH 23
#define SIP_BUTTON_WIDTH 23

typedef struct tagNOTIFYCOPYSTRUCT
{
    SHNOTIFICATIONDATA *pData;
} NOTIFYCOPYSTRUCT;


class CTaskbarDropTarget;

class CTaskBar 
{
public:

    inline void * __cdecl operator new(size_t cb) { return LocalAlloc(LPTR, cb); } 
    inline void * __cdecl operator new[](size_t cb) { return LocalAlloc(LPTR, cb); } 
    inline void __cdecl operator delete(void * pv) { LocalFree(pv); }

    CTaskBar();
    ~CTaskBar();

    friend CTaskbarDropTarget;
	friend TaskManDlg;
	friend ExtrasMenuButton;

    BOOL  Register(HINSTANCE hInstance);
    DWORD MessageLoop();
    HWND    GetWindow() { return m_hwndTaskBar; }

    // used by start menu
    void Properties(HWND hwndTB);
    BOOL ShowRunDlg(VOID);
    HWND GetDesktopTaskBar();
    BOOL GetExpandControlPanel(BOOL *pbExpand);

    // used by SHSetNavBarText
    BOOL ChangeItemText( HWND hwndApp, LPCTSTR pszNewText );

    // Bubble Support
    BOOL RemoveBubble( const CLSID *pclsid, DWORD dwID );

    // used by SHNotificationGetData
    BOOL GetNotificationData( const CLSID *pclsid, DWORD dwID, SHNOTIFICATIONDATA *pndBuffer,
                              LPTSTR pszTitle, LPTSTR pszHTML, DWORD *pdwHTMLLength );

    // Used by Shell_NotifyIcon
    int InsertItem(HWND hwndApp);
    BOOL DeleteItem(HWND hwnd, int iItem, BOOL bFree = TRUE);
    UINT FindNotify(HWND hwndTB, PNOTIFYICONDATA pNID);
    BOOL ModifyNotify(HWND hwndTB, PNOTIFYICONDATA pnid, int iItem);
    BOOL InsertNotify(HWND hwndTB, PNOTIFYICONDATA pnid);

    BOOL RecalcItems(HWND hwndTB);

private:    
    HWND GetRunDlg();

    LPTBOBJ FindItem(HWND hwndItem);
    BOOL  GetExistingApplications(HWND hwndTB);

    void SetAdminTimer();
    void SetHideTimer();
    void SetUnhideTimer(LONG x, LONG y);
    void SetNotificationIconUpdateTimer();
    BOOL AutoHideTimer(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
    BOOL AutoUnhideTimer(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
    BOOL UpdateTimedNotificationIcons();
    
    void  SlideWindow(HWND hwnd, const RECT &rcNew, BOOL fShow);
    void Hide(HWND hwnd);
    void Show(HWND hwnd);
    BOOL IsActive();
    void HandleFullScreenOrTopmostApp(TaskbarApplicationButton *pApp);
    void SetTaskBarZOrder(HWND hwndZorder);

    void SaveAutoHide(BOOL bAutoHide);
    BOOL GetAutoHide(BOOL *pbAutoHide);
    void SaveExpandControlPanel(BOOL bExpand);
    BOOL GetSlide(BOOL *pbSlide);
    BOOL GetTameStartMenu(BOOL *pbTameStartMenu);
    void SaveOnTop(BOOL bOnTop);
    BOOL GetOnTop(BOOL *pbOnTop);

    void ShowClock(HWND hwnd, BOOL bShow);
    void SetWorkArea(void);
    void UpdateDesktopArea(HWND hwndDT);
    void ResizeAppWindows(BOOL bTaskBarVisible, BOOL bTaskBarOnTop);
    void ApplyProperties(HWND hwnd, BOOL bGeneral);
    void ReloadSettings();
    DWORD GetRecentDocsCount();
    LPTBOBJ  GetItem(HWND hwndTB, int iItem);
    BOOL CloseAllWindows(HWND hwndTB);
    BOOL  GetTrayRect(HWND hwndTB, LPRECT lprc);
    BOOL IsClockVisible(HWND hwndTB);
    BOOL CalcSizeClock(LPRECT lprc);
    BOOL CheckShowClock();
    BOOL RecalcShowClock();

    void WinIniChange(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
    void Cleanup();

    BOOL SetupStandardButtons(HINSTANCE hInstance);
    HWND Create(HWND hwndParent);

    BOOL TimerAdmin(HWND hwndTB, UINT msg, WPARAM wp, LPARAM lp);

    HWND FindCheckedItem(HWND hwndTB);
    int FindItemIndex(HWND hwndTB, HWND hwndItem);

    BOOL NotifyTagDestroyIcon(HWND hwndTB, PNOTIFYICONDATA pNID);

    // bubble notification support
    LRESULT HandleNimBubbleMessage( NOTIFYCOPYSTRUCT *pNCS, DWORD dwMsg );
    UINT FindNotify(HWND hwndTB, PSHNOTIFICATIONDATA pND);
    BOOL NotifyTagDestroyIcon(HWND hwndTB, PSHNOTIFICATIONDATA pND);
    BOOL InsertNotify(HWND hwndTB, PSHNOTIFICATIONDATA pND);
    UINT FindBubble( CLSID clsid, DWORD dwID );
    BOOL AddBubble( PSHNOTIFICATIONDATA pND );
    BOOL AddBubble(LPTBOBJ ptbobj);
    BOOL AddBubbleToTray(HWND hwndTB, LPTBOBJ ptbobj);
    BOOL UpdateBubble( DWORD grnumUpdateMask, SHNOTIFICATIONDATA *pndNew );
    BOOL LaunchNotifyBubble(TaskbarBubble *ptbb, BOOL bFromTray = FALSE);
    BOOL FireNextBubble();
    
    // Input Language support
    BOOL UpdateInputLangIcon(HKL hklIdentifier);

	// iterators for walking the applications
	TaskbarApplicationButton *GetNextAppButton(TaskbarApplicationButton *lastButton);
	TaskbarApplicationButton *GetPrevAppButton(TaskbarApplicationButton *nextButton);
	int GetApplicationCount() { return m_currentNumberOfApps; }
	int GetFirstVisibleApplicationIndex() { return m_indexFirstApp; }

    int ItemCount(HWND hwnd);
    BOOL RecalcButtons(); // recalculate the desktop and extras button
    
    BOOL Draw(HWND hwnd, HDC hdc, LPCRECT lprcUpdate);
    BOOL DrawItem(HWND hwnd, HDC hdc, UINT nItem, BOOL fPressed);
    BOOL UpdateItem(HWND hwnd, UINT nItem, TaskbarItem *ptbi);
    UINT HitTest(HWND hwnd, POINT pt, UINT FAR *lpgrfFlags);
    TaskbarItem * GetItemInfo(HWND hwnd, UINT nItem);

    BOOL Destroy(HWND hwndTB);
    BOOL NotifyMouseEvent(HWND hwndTB, UINT iItem, UINT uMsg, WPARAM wParam, LPARAM lParam);
    BOOL NotifyMouseEventBubble(HWND hwndTB, UINT iItem, UINT uMsg, WPARAM wParam, LPARAM lParam);
    BOOL GetRunDlgHistory(HWND hwndCB);
    BOOL UpdateRunDlgHistory(HWND hwndCB, LPCTSTR lpszLastCmd);

    BOOL ExecuteKey(DWORD key, DWORD modifiers);

    void ForceButtonUp(HWND hwnd, int index);
    void StartMenu(HWND hwnd);
    
    LPTBOBJ GetActive(HWND hwnd);
    BOOL ActivateNextItem(HWND hwnd);
    
    void OnWmShellNotify(
        HWND    hwndTaskBar,
        int     NotifyCode,
        LPARAM  lp,
        int*    pItem);

    BOOL CheckApp(HWND hwnd, UINT nItem, BOOL fShowItem, BOOL *pbRedraw = NULL);
    BOOL DrawTray(HDC hdc, TaskbarItem *ptbi, UINT edge, HBRUSH hbr);
        
LRESULT  RunDlg(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
LRESULT CALLBACK WarningDlgProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
int CALLBACK PropertiesDlgProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
int CALLBACK DocumentMenuDlgProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
LRESULT HandleShellHook(int iCode, LPARAM lParam);
LRESULT CALLBACK TaskBarWndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
LRESULT CALLBACK DisabledTaskBarWndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
LRESULT CALLBACK TrayWndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);

//void Properties(HWND hwndTB);
static BOOL PropertiesThread(HANDLE hEvent);
static DWORD s_BrowseForRunFile(LPVOID lpParameter);

static int CALLBACK s_PropSheetProc(HWND hwndDlg, UINT uMsg, LPARAM lParam);
static int CALLBACK s_PropertiesSubclass(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);

static LRESULT CALLBACK s_RunDlg(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
static BOOL CALLBACK s_ResizeCallBack(HWND hwnd, LPARAM lparam);
static LRESULT CALLBACK s_WarningDlgProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
static int CALLBACK s_PropertiesDlgProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
static int CALLBACK s_DocumentMenuDlgProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
static LRESULT CALLBACK s_TaskBarWndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
static LRESULT CALLBACK s_TrayWndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);

void SipMenu(POINT pt, int index);
BOOL SipMenu_ProcessCommand(HWND hwnd, WPARAM wp, LPARAM lp);
BOOL SipMenu_InitPopup(HMENU hmenu);
BOOL UpdateSipButton( HICON hIcon = NULL );
    
private:
        CList *m_plistTaskBar;
        CList *m_plistBubbles;
        HWND  m_hwndTaskBar;
        HWND m_hwndTaskBarTT;
        HWND m_hwndTray;
        HWND m_hwndRunDlg;
//        HWND m_hwndTaskMan;

        HWND   m_hwndTaskbarProperties;

        CHtmlBubble *m_pBubbleMgr;
        PowerManagerUI *m_pPowerUI;
		TaskManDlg *m_pTaskManDlg;
		ExtrasMenuButton *m_pExtrasMenu;

        HICON  m_hiconDefault;
        HFONT  m_hfont;
        
        BOOL   m_bInStartMenu;
        BOOL   m_fLocaleChange;
        BOOL   m_fShowClock;

        // power warnings and management
        BOOL   m_fShowPower;
        BOOL   m_fShowPowerWarnings;

        BOOL   m_bExpandControlPanel; // Default to DISabled
        BOOL   m_bTaskBarHidden;
        BOOL   m_bTaskBarAutoHide;
        BOOL   m_bTaskBarOnTop;
        BOOL   m_bTaskBarAbortHide;
        POINT  m_ptLastMouseMove;
        POINT  m_ptLastTrayMouseMove;
        BOOL   m_bHasMouse;
        BOOL   m_bTaskBarSlide;
        BOOL   m_bTaskBarTameStartMenu;
        BOOL   m_bInPopupMenu;
        BOOL   m_bInSipMenu;

        BOOL   m_fIsSmallScreen;  // QVGA mode and other thin displays
        BOOL   m_fShowDesktopButton;
        BOOL   m_fShowSipButton;
        HICON  m_hiSipUpIcon;
        HICON  m_hiSipDownIcon;
        HICON  m_hiSipCurrentIcon;
        BOOL   m_fExtraAppsButton;         // display a button for other apps

// ITaskbarList support
        UINT   WM_ShellHook;
        HWND   m_hwndRudeFullScreen;

// Disabling Taskbar (for SHFullScreen)
        BOOL   m_fTaskbarDisabled;
        HWND   m_hwndDisablerWindow;
        
// for a compressed window list
        int      m_indexFirstApp;           // the first item in the list
        int      m_maxAppsToDisplay;        // can be zero
        int      m_currentNumberOfApps;     // maintained at insertsion/deletion number of apps we are keeping track of
        
// for a compressed notify list
        int  m_maxNotifysToDisplay;
        int  m_firstNotifyToDisplay;            // 0-based relative to the 
        int  m_currentNumberOfNotifys;           // maintained count of notifications

// metrics
        int  SEPERATOR;
        int  NUMITEMS;

        
};


class CStringRes {
public:
    CStringRes(HINSTANCE hInst) : m_hInst(hInst) { m_sz = NULL; }
    ~CStringRes() { /*if (m_sz) { LocalFree(m_sz);*/ m_sz = NULL; /*}*/ }

    LPCTSTR LoadString(UINT uID)
    {
#ifdef UNDER_CE
        m_sz = (LPCTSTR)(::LoadString(m_hInst, uID, 0, 0));
        /* On CE, (lpBuffer == 0) means that the return value is a pointer to the string.
           All strings in the string area of the resource section have been manually null-
           terminated in order to take advantage of this feature. */
#else
        ASSERT(FALSE);
        // Cool LoadString(...) behavior is supported only on CE - you need to allocate a buffer...
#endif
        return m_sz;
    }
    
    operator LPCTSTR() { return m_sz; }
    
public:
    HINSTANCE m_hInst;
    LPCTSTR m_sz;

}; /* CStringRes */

HWND Desktop_GetWindow();
HWND Taskbar_GetWindow();


#endif
