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
#include <commdlg.h>
#include <shobjidl.h>
#include <shlobj.h>
#include "ceshlobj.h"
#include <sipapi.h>


#ifdef DEBUG
    #define DECLAREWAITCURSOR  HCURSOR hcursor_wait_cursor_save = (HCURSOR)0xDEADBEEF
    #define SetWaitCursor()   { DEBUGCHK(hcursor_wait_cursor_save == (HCURSOR)0xDEADBEEF); hcursor_wait_cursor_save = SetCursor(LoadCursor(NULL, IDC_WAIT)); }
    #define ResetWaitCursor() { DEBUGCHK(hcursor_wait_cursor_save != (HCURSOR)0xDEADBEEF); SetCursor(hcursor_wait_cursor_save); hcursor_wait_cursor_save = (HCURSOR)0xDEADBEEF; }
#else
    #define DECLAREWAITCURSOR  HCURSOR hcursor_wait_cursor_save = NULL
    #define SetWaitCursor()   { hcursor_wait_cursor_save = SetCursor(LoadCursor(NULL, IDC_WAIT)); }
    #define ResetWaitCursor() { SetCursor(hcursor_wait_cursor_save);  hcursor_wait_cursor_save = NULL; }
#endif


// function ptrs into coredll for SIP functionality
typedef BOOL (WINAPI* LPFNSIP)(SIPINFO*);
typedef DWORD (WINAPI* LPFNSIPSTATUS)();

typedef struct tagSIPPTRS
{
    HINSTANCE m_hCoreDll;
    LPFNSIP m_pSipGetInfo;
    //LPFNSIPSTATUS m_pSipStatus;
}
SIPPTRS, *LPSIPPTRS;


class FileOpenDlg_t  :
    public IShellBrowser,
    public IOleCommandTarget
{

protected:
    DWORD m_ObjRefCount;
    FOLDERSETTINGS m_FolderSettings;
    LPCITEMIDLIST m_pidlFQ;
    IShellFolder* m_pIShellDesktopFolder;
    IShellFolder* m_pIShellFolder;
    IShellView* m_pIShellView;
    IShellListView* m_pIShellListView;
    IEnumFilter* m_pIEnumFilter;
    IOleCommandTarget* m_pIOleCommandTarget;

    OPENFILENAME* m_pOpenFileName;
    LPCWSTR m_pwszTitle;
    LPWSTR m_pwszDefExt;
    WCHAR m_wszCurrDir[MAX_PATH];
    WCHAR m_wszCurrentFilter[MAX_PATH];
    WCHAR m_wszDirPath[MAX_PATH];

    bool m_bChangeUpdateDisabled:1;
    bool m_bFileSave:1;
    bool m_bHelpAvailable:1;
    bool m_bInLabelEdit:1;
    bool m_bRetVal:1;
    bool m_bSelected:1;

    HWND m_hwndDialog;
    HWND m_hwndToolbar;

    HWND m_hwndStatic;
    WNDPROC m_pfnStaticWndProc;

    RECT m_rcView;
    HWND m_hwndView;
    WNDPROC m_pfnViewWndProc;

    const short* m_pColumnWidths;
    const TBBUTTON* m_ptbButtons;
    short m_cButtons;

    HFONT m_hFont;
    HBRUSH m_hBGBrush;
    HICON m_hiconFolderSM;

    SIPPTRS m_sip;

    static LPWSTR s_pwszTooltips[];
    static DWORD s_TTRefCount;

    static DWORD TTAddRef();
    static DWORD TTRelease();

    static const WCHAR s_szDesktopCLSID[];
    static const WCHAR s_szBitBucketCLSID[];
    static const WCHAR s_RegGUIDs[];
    static const WCHAR s_RegDisplayName[];

    static BOOL FindNameInClassesRoot(LPCWSTR szNameSpace, LPCWSTR pszDisplayName, size_t cchToCompare);
    static BOOL GetParsingName(LPCWSTR pszDisplayName, LPWSTR pszParsingName, size_t* pcchOutBuf);

    static BOOL CALLBACK DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
    static LRESULT CALLBACK StaticSubclassProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    static LRESULT CALLBACK ViewSubclassProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);


    void ActivatePendingView();
    BOOL CenterWindowSIPAware(HWND hwnd, BOOL fInitial);
    BOOL CreateCommandBar();
    void DestroyActiveView();
    void DrawPathBanner(HDC hdc, RECT* pRect);
    void EnableToolbar(HWND hwndToolbar, BOOL fCanGoUp);
    BOOL HandleCommand(WPARAM wParam, LPARAM lParam);
    BOOL HandleListViewNotify(WPARAM wParam, LPARAM lParam, LRESULT* plResult);
    BOOL HandleOK();
    BOOL HaveSIP();
    BOOL InitDialog(HWND hDlg);
    DWORD InitFilterBox();
    BOOL InitGlobals();
    BOOL IsBrowsableItem(IShellFolder* psfParent, LPCITEMIDLIST pidlRelative, LPITEMIDLIST* ppidlOut);
    HRESULT NavigateToFolder(LPCWSTR pwszPath);
    int NotifyTypeChange();
    BOOL ProcessDots(LPCWSTR pwszCurrentDir, LPWSTR pwszPath, LPCWSTR pwszInput);
    BOOL SetDefaultExtension();
    void UnInitGlobals();
    void UpdateToolbar(HWND hwndToolbar);


public:
    FileOpenDlg_t(BOOL bFileSave);
    ~FileOpenDlg_t();

    BOOL Run(LPOPENFILENAME lpofn);

    // IUnknown methods
    STDMETHOD (QueryInterface)(REFIID, void**);
    STDMETHOD_ (DWORD, AddRef)();
    STDMETHOD_ (DWORD, Release)();

    // IOleWindow
    STDMETHOD (ContextSensitiveHelp)(BOOL){return E_NOTIMPL;};
    STDMETHOD (GetWindow)(HWND*);

    // IShellBrowser methods
    STDMETHOD(BrowseObject)(LPCITEMIDLIST pidl, UINT wFlags);
    STDMETHOD(EnableModelessSB)(BOOL fEnable){return E_NOTIMPL;}
    STDMETHOD(GetControlWindow)(UINT id, HWND* lphwnd){return E_NOTIMPL;}
    STDMETHOD(GetViewStateStream)(DWORD grfMode, IStream** ppStrm){return E_NOTIMPL;}
    STDMETHOD(InsertMenusSB)(HMENU hmenuShared, LPOLEMENUGROUPWIDTHS lpMenuWidths){return E_NOTIMPL;}
    STDMETHOD(OnViewWindowActive)(IShellView* ppshv){return E_NOTIMPL;}
    STDMETHOD(QueryActiveShellView)(IShellView** ppshv){return E_NOTIMPL;}
    STDMETHOD(RemoveMenusSB)(HMENU hmenuShared){return E_NOTIMPL;}
    STDMETHOD(SendControlMsg)(UINT id, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT* pret){return E_NOTIMPL;}
    STDMETHOD(SetMenuSB)(HMENU hmenuShared, HOLEMENU holemenuRes, HWND hwndActiveObject){return E_NOTIMPL;}
    STDMETHOD(SetStatusTextSB)(LPCOLESTR lpszStatusText){return E_NOTIMPL;}
    STDMETHOD(SetToolbarItems)(LPTBBUTTONSB lpButtons, UINT nButtons, UINT uFlags){return E_NOTIMPL;}
    STDMETHOD(TranslateAcceleratorSB)(LPMSG lpmsg, WORD wID){return E_NOTIMPL;}

    // IOleCommandTarget
    STDMETHOD(QueryStatus)(const GUID *pguidCmdGroup, ULONG cCmds, OLECMD rgCmds[], OLECMDTEXT *pcmdtext){return E_NOTIMPL;}
    STDMETHOD(Exec)(const GUID *pguidCmdGroup, DWORD nCmdID, DWORD nCmdexecopt, VARIANTARG *pvarargIn, VARIANTARG *pvarargOut);

};

