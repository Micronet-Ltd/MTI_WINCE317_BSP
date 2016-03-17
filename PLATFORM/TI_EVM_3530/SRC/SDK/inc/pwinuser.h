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
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES OR INDEMNITIES.
//
/*++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Module Name:

ppwinuser.h

Abstract:

Private portion of winuser.h

Notes:


--*/


#ifndef _PRIV_WINUSER_
#define _PRIV_WINUSER_

#ifdef __cplusplus
extern "C"  {
#endif

#include <keybd.h>

// Non-Standard CE internal messages.
#define WM_QM_ACTIVATE          0x60
#define WM_HOOK_DO_CALLBACK     0x61
#define WM_SYSCOPYDATA          0x62


// wparam values for WM_SYSCOPYDATA
#define WMSCD_CHAR_RANKED_ALTS  0x01
#define WMSCD_DIALOGSCROLLBAR   0x02  



/*
 * Extended Window Styles
 */
#if(WINVER >= 0x0400)

#endif /* WINVER >= 0x0400 */

// Marker for end of accelerator tables.
#define FLASTKEY  0x80

// SetWindowPos internal flags
#define SWP_SYSTEMCAPTURE           0x80000000
#define SWP_FINDNEWFOREGROUND       0x40000000
#define SWP_MAXMIN                  0x20000000


// Internal values for hwnd for use in broadcasting messages
// Must be odd numbered.
#define HWND_TOPLEVELVISIBLEORPRE30     ((HWND)0xfffd)    // top level windows that are visible on screen or ver < 3.0
#define HWND_TOPLEVELCHILD              ((HWND)0xfffb)    // top level windows with the WS_CHILD bit set

/*
 * Predefined Clipboard Formats
 */
#define CF_METAFILEPICT     3
#define CF_ENHMETAFILE      14
#define CF_HDROP            15
#define IMAGE_ENHMETAFILE   3

void WINAPI SystemIdleTimerReset( void );

HWND WINAPI SetKeyboardTarget( HWND hWnd );
HWND WINAPI GetKeyboardTarget( VOID );
KEY_STATE_FLAGS WINAPI GetAsyncShiftFlags(UINT VKey);
WINUSERAPI void WINAPI AccessibilitySoundSentryEvent(void);
BOOL WINAPI ImmEnableIME(DWORD);
HKL WINAPI ImmGetKeyboardLayout(DWORD dwThreadId);

/*
 * ImmAssociateValueWithGwesMessageQueue flags
 * 
 * These is not hard & fast rule for the #s given below - just 
 * that they are ordered in pairs of get/set operations
 */
#define IMM_GETTHREADHKL                    1
#define IMM_SETTHREADHKL                    2
#define IMM_GETTHREADPREVHKL                3
#define IMM_SETTHREADPREVHKL                4

#define IMM_SETPROCESSHKL                   6

#define IMM_SET_THREAD_IME_DEFAULT_HWND     8
#define IMM_GET_LAYMGR_STATUS               9



void AudioUpdateFromRegistry(void);

/* Values for NotifyWinUserSystem */
#define NWUS_DOUBLE_TAP_CHANGED     1
#define NWUS_KEYBD_REPEAT_CHANGED   2
#define NWUS_MAX_IDLE_TIME_CHANGED  3

#define SPI_SETNOSEHFORVALIDATION   250

#define MOD_KEYUP_ALWAYS 0x2000

typedef struct tagPLATFORMVERSION {
    DWORD dwMajor;
    DWORD dwMinor;
} PLATFORMVERSION, *PPLATFORMVERSION;


#define SPI_PROJECT1    224
#define SPI_PROJECT2    225
#define SPI_PROJECT3    226
#define SPI_PROJECT4    227
#define SPI_PROJECT5    228
#define SPI_PROJECT6    229
#define SPI_PROJECT7    230
#define SPI_PROJECT8    231
#define SPI_PROJECT9    232
#define SPI_PROJECT10   233
#define SPI_PROJECT11   234
#define SPI_PROJECT12   235
#define SPI_PROJECT13   236
#define SPI_PROJECT14   237
#define SPI_PROJECT15   238
#define SPI_PROJECT16   239

#define SPI_GETPLATFORMVERSION        SPI_PROJECT1    

#define VK_APP_LAUNCH1      0xC1
#define VK_APP_LAUNCH2      0xC2
#define VK_APP_LAUNCH3      0xC3
#define VK_APP_LAUNCH4      0xC4
#define VK_APP_LAUNCH5      0xC5
#define VK_APP_LAUNCH6      0xC6
#define VK_APP_LAUNCH7      0xC7
#define VK_APP_LAUNCH8      0xC8
#define VK_APP_LAUNCH9      0xC9
#define VK_APP_LAUNCH10     0xCA
#define VK_APP_LAUNCH11     0xCB
#define VK_APP_LAUNCH12     0xCC
#define VK_APP_LAUNCH13     0xCD
#define VK_APP_LAUNCH14     0xCE
#define VK_APP_LAUNCH15     0xCF




void WINAPI NotifyWinUserSystem( UINT    uEvent );

BOOL WINAPI RegisterTaskBar (HWND hwndTaskbar);
BOOL WINAPI RegisterTaskBarEx (HWND hwndTaskbar, BOOL bTaskBarOnTop);
BOOL WINAPI RegisterDesktop (HWND hwndDesktop);

WINUSERAPI BOOL WINAPI RectangleAnimation ( HWND hWnd, LPCRECT prc, BOOL fMaximize ) ;
void WINAPI ShellModalEnd( void );

//#define ExitWindows(dwReserved, Code) ExitWindowsEx(EWX_LOGOFF, 0xFFFFFFFF)
//WINUSERAPI BOOL WINAPI ExitWindowsEx( UINT uFlags,DWORD dwReserved);
void GwesPowerOffSystem (void);
BOOL GwesPowerDown (void);
void GwesPowerUp (BOOL);

void WINAPI BatteryGetLifeTimeInfo ( LPSYSTEMTIME pstLastChange, DWORD * pcmsCpuUsage, DWORD * pcmsPreviousCpuUsage );
LONG WINAPI BatteryDrvrGetLevels ( void );
BOOL WINAPI BatteryDrvrSupportsChangeNotification ( void );
void WINAPI BatteryNotifyOfTimeChange ( BOOL fForward, FILETIME * pftDelta );

// A system class built in to GWE for use by everyone
#define CS_SYSTEMCLASS 0x80000000

/* flags for DrawFrameControl */
//#define DFC_CAPTION             1
#define DFC_MENU                2
//#define DFC_SCROLL              3
//#define DFC_BUTTON              4
#define DFC_SPINNER             6

//#define DFCS_CAPTIONCLOSE       0x0000
//#define DFCS_CAPTIONMIN         0x0001
//#define DFCS_CAPTIONMAX         0x0002
//#define DFCS_CAPTIONRESTORE     0x0003
//#define DFCS_CAPTIONHELP        0x0004
//#define DFCS_CAPTIONOKBTN       0x0080
#define DFCS_CAPTIONROUNDOKBTN_WINCE    0x0010 
#define DFCS_CAPTIONROUNDCLOSE_WINCE    0x0020
#define DFCS_CAPTIONALL_WINCE           0x00F0

#define DFCS_MENUARROW          0x0000
#define DFCS_MENUCHECK          0x0001
#define DFCS_MENUBULLET         0x0002
#define DFCS_MENUARROWRIGHT     0x0004

//#define DFCS_SCROLLUP           0x0000
//#define DFCS_SCROLLDOWN         0x0001
//#define DFCS_SCROLLLEFT         0x0002
//#define DFCS_SCROLLRIGHT        0x0003
//#define DFCS_SCROLLCOMBOBOX     0x0005
#define DFCS_SCROLLSIZEGRIP     0x0008
#define DFCS_SCROLLSIZEGRIPRIGHT 0x0010

//#define DFCS_BUTTONCHECK        0x0000
#define DFCS_BUTTONRADIOIMAGE   0x0001
#define DFCS_BUTTONRADIOMASK    0x0002
//#define DFCS_BUTTONRADIO        0x0004
//#define DFCS_BUTTON3STATE       0x0008
//#define DFCS_BUTTONPUSH         0x0010

#define DFCS_SPINNERUP           0x0000
#define DFCS_SPINNERDOWN         0x0001
#define DFCS_SPINNERLEFT         0x0002
#define DFCS_SPINNERRIGHT        0x0003
#define DFCS_SPINNERBUTTON       0x0010
#define DFCS_SPINNERNOBORDER     0x0020

//#define DFCS_INACTIVE           0x0100
//#define DFCS_PUSHED             0x0200
//#define DFCS_CHECKED            0x0400
#define DFCS_ADJUSTRECT         0x2000
#define DFCS_FLAT               0x4000
#define DFCS_MONO               0x8000

#define MFT_NONOWNERSTRING      0x80000000L

//WINUSERAPI BOOL    WINAPI DrawFrameControl(HDC, LPRECT, UINT, UINT);

// NC messages
#define WM_NCCREATE                     0x0081
#define WM_NCDESTROY                    0x0082
#define WM_NCCALCSIZE                   0x0083
#define WM_NCHITTEST                    0x0084
#define WM_NCPAINT                      0x0085
#define WM_NCACTIVATE                   0x0086

#define WM_NCMOUSEFIRST                 0x00A0
#define WM_NCMOUSEMOVE                  0x00A0
#define WM_NCLBUTTONDOWN                0x00A1
#define WM_NCLBUTTONUP                  0x00A2
#define WM_NCLBUTTONDBLCLK              0x00A3
#define WM_NCRBUTTONDOWN                0x00A4
#define WM_NCRBUTTONUP                  0x00A5
#define WM_NCRBUTTONDBLCLK              0x00A6
#define WM_NCMBUTTONDOWN                0x00A7
#define WM_NCMBUTTONUP                  0x00A8
#define WM_NCMBUTTONDBLCLK              0x00A9


#define WM_NCXBUTTONDOWN                0x00AB
#define WM_NCXBUTTONUP                  0x00AC
#define WM_NCXBUTTONDBLCLK              0x00AD
#define WM_NCMOUSELAST                  0x00AD

#ifdef FAREAST
/* Class style for IME UI window */
#define CS_IME      0x00010000
#endif // FAREAST

#ifdef _WIN32_WCE_EMULATION
#ifdef CS_IME
#undef CS_IME
#endif
#define CS_IME      0x00010000
#endif

/* For internal APIs "exposed" to other internal pieces. */
#ifdef __cplusplus
    #define EXTERN_C    extern "C"
#else
    #define EXTERN_C    extern
#endif /*__cplusplus*/
#define INTAPI  EXTERN_C LONG __stdcall
#define INTAPI_(type) EXTERN_C type __stdcall

BOOL KeybdGetDeviceInfo( INT iIndex, LPVOID  lpOutput );
BOOL KeybdInitStates( KEY_STATE KeyState, void  *pKeybdDriverState );

UINT32 KeybdVKeyToUnicode(
    UINT32          VKey,
    KEY_STATE_FLAGS KeyEventFlags,
    KEY_STATE       KeyState,
    void            *KeybdDriverState,
    UINT32          cBufferSize,
    UINT32          *pcCharacters,
    KEY_STATE_FLAGS *pShiftStateBuffer,
    UINT32          *pCharacterBuffer
    );


// Maximum # of slots available in preload reg key.
#define MAX_HKL_PRELOAD_COUNT	15


#define WH_JOURNALRECORD    0
#define WH_JOURNALPLAYBACK  1
#define WH_KEYBOARD_LL      20
#define WH_MOUSE_LL         21


/*
 * Hook Codes
 */
#define HC_ACTION           0
#define HC_GETNEXT          1
#define HC_SKIP             2
#define HC_NOREMOVE         3
#define HC_NOREM            HC_NOREMOVE
#define HC_SYSMODALON       4
#define HC_SYSMODALOFF      5

typedef LRESULT (CALLBACK* HOOKPROC)(int code, WPARAM wParam, LPARAM lParam);

/*
 * Message Structure used in Journaling
 */
typedef struct tagEVENTMSG {
    UINT    message;
    UINT    paramL;
    UINT    paramH;
    DWORD    time;
    HWND    hwnd;
} EVENTMSG, *PEVENTMSGMSG, NEAR *NPEVENTMSGMSG, FAR *LPEVENTMSGMSG;

typedef struct tagEVENTMSG *PEVENTMSG, NEAR *NPEVENTMSG, FAR *LPEVENTMSG;


HHOOK
WINAPI
QASetWindowsJournalHook(
    int         nFilterType,
    HOOKPROC    pfnFilterProc,
    EVENTMSG    *pfnEventMsg
    );


BOOL
WINAPI
QAUnhookWindowsJournalHook(
    int            nFilterType
    );


BOOL
WINAPI
GetMessageWNoWait(
    PMSG    pMsg,
    HWND    hWnd,
    UINT    wMsgFilterMin,
    UINT    wMsgFilterMax
    );

BOOL
WINAPI
TouchCalibrate(
    void
    );

int
WINAPI
DisableCaretSystemWide(
    void
    );


int
WINAPI
EnableCaretSystemWide(
    void
    );


BOOL
WINAPI
EnableHardwareKeyboard(
    BOOL fEnable
    );

BOOL RegisterSIPanel (HWND hwndSIPanel);


void WINAPI SetAssociatedMenu (
    HWND hwnd,
    HMENU hmenu
    );

HMENU WINAPI GetAssociatedMenu (
    HWND hwnd
    );


BOOL
WINAPI
UnregisterFunc1(
    UINT    fsModifiers,
    UINT    vk
    );







typedef struct tagKBDLLHOOKSTRUCT {
    DWORD vkCode;        // virtual key code
    DWORD scanCode;        // scan code    DWORD flags;       // flags
    DWORD flags;        // unused
    DWORD time;            // time stamp for this message
    DWORD dwExtraInfo;    // extra info from the driver or keybd_event
} KBDLLHOOKSTRUCT, *PKBDLLHOOKSTRUCT;

typedef struct {
    POINT pt;           // point of the cursor in screen coordinate
    DWORD mouseData;    // if WM_MOUSEWHEEL, the high-order word of this member is the wheel delta.
    DWORD flags;        // unused
    DWORD time;         // time stamp for this message
    DWORD dwExtraInfo;  // extra info
} MSLLHOOKSTRUCT, *PMSLLHOOKSTRUCT;

HHOOK
WINAPI
SetWindowsHookExW(
    int idHook,
    HOOKPROC lpfn,
    HINSTANCE hmod,
    DWORD dwThreadId);
#define SetWindowsHookEx  SetWindowsHookExW


BOOL
WINAPI
UnhookWindowsHookEx(
    HHOOK hhk);

LRESULT
WINAPI
CallNextHookEx(
    HHOOK hhk,
    int nCode,
    WPARAM wParam,
    LPARAM lParam);

typedef struct tagGetForegroundInfo
    {
    HWND    hwndActive;
    HWND    hwndFocus;
    HWND    hwndMenu;
    HWND    hwndKeyboardDest;
    DWORD    fdwConversion;
    DWORD    fdwSentence;
    BOOL    fOpen;
    DWORD    dwCompStrLen;
    HKL        KeyboardLayoutHandle;
    } GET_FOREGROUND_INFO;

BOOL
WINAPI
GetForegroundInfo(
    GET_FOREGROUND_INFO    *pgfi
    );

HWND GetForegroundKeyboardTarget(
    void
    );

HKL
GetForegroundKeyboardLayoutHandle(
    void
    );


BOOL
WINAPI
ImmGetConversionStatusForeground(
    DWORD    *pfdwConversion,
    DWORD    *pfdwSentence
    );

BOOL
ShowStartupWindow(
    void
    );


// Dialog Styles
#define DS_USEPIXELS        0x8000L // specifies use of pixels, not dialog coordinates

BOOL
WINAPI
SetDialogAutoScrollBar(
    HWND    hDlg
    );

BOOL
WINAPI
SetWindowPosOnRotate(
    HWND hWnd,
    HWND hWndInsertAfter,
    int X,
    int Y,
    int cx,
    int cy,
    UINT uFlags,
    BOOL fNoScroll
    );


#define ARF_IMAGEFILE         0x00001
#define ARF_IMAGERES          0x00002
#define ARF_CLIENTSTARTRECT   0x00004 // specifies that the rcStart is in in client coordinates
#define ARF_CLIENTFINISHRECT  0x00008 // specifies that the rcFinish is in in client coordinates
#define ARF_SYNCHRONOUS       0x00010 // specifies that the animation happens synchronously
#define ARF_BITMAP            0x00020 // specifies that the hdc field contains a bitmap for the animation
#define ARF_BMPGROW           0x00040 // Bitmap animation "grows"
#define ARF_BMPUNCOVER        0x00080 // Bitmap animation "uncovers"
#define ARF_INCLUDELAST       0x00100 // include the last frame in the animation
#define ARF_ANIMATETIME       0x00200 // dwAnimateTime contains valid data
#define ARF_USEPARENT         0x00400 // hwnd should be used as the parent window for the animation

typedef struct
{
    HWND hwnd;
    RECT rcStart;
    RECT rcFinish;
    DWORD dwFlags;
    DWORD dwAnimateTime; // duration of animation in MS

    LPCTSTR pszResID;
    HINSTANCE hinst;
    LPTSTR pszFile;
    HBITMAP hbmAnimate;
} ANIMATERECTSINFO;

HRESULT AnimateRects(ANIMATERECTSINFO *pari);

//    This is taken directly from pbt.h.  Not all values are used or supported.

#ifndef WM_POWERBROADCAST
#define WM_POWERBROADCAST               0x218
#endif

#define PBT_APMQUERYSUSPEND             0x0000
#define PBT_APMQUERYSTANDBY             0x0001

#define PBT_APMQUERYSUSPENDFAILED       0x0002
#define PBT_APMQUERYSTANDBYFAILED       0x0003

#define PBT_APMSUSPEND                  0x0004
#define PBT_APMSTANDBY                  0x0005

#define PBT_APMRESUMECRITICAL           0x0006
#define PBT_APMRESUMESUSPEND            0x0007
#define PBT_APMRESUMESTANDBY            0x0008

#define PBTF_APMRESUMEFROMFAILURE       0x00000001

#define PBT_APMBATTERYLOW               0x0009
#define PBT_APMPOWERSTATUSCHANGE        0x000A

#define PBT_APMOEMEVENT                 0x000B

#define PBT_APMUSERIDLE                 0x000C
#define PBT_APMUSERACTIVE               0x000D


// @CESYSGEN IF GWES_WINMGR

// Notifications sent by GWES to the shell

// Codes used in WM_SHELLNOTIFY
#define SHELLNOTIFY_WINDOWCREATED        0x0006
#define SHELLNOTIFY_WINDOWDESTROYED      0x0007
#define SHELLNOTIFY_WINDOWACTIVATED      0x0008
#define SHELLNOTIFY_REDRAW               0x0009
#define SHELLNOTIFY_LANGUAGE             0x000A
#define SHELLNOTIFY_CAPSLOCK             0x000B
#define SHELLNOTIFY_RESERVED0x000C       0x000C
#define SHELLNOTIFY_RESERVED0x000D       0x000D

// @CESYSGEN ENDIF


// @CESYSGEN IF GWES_UIBASE

#define CURSORPREFERENCE_SYSTEM              0x0000 /* follow system preference */
#define CURSORPREFERENCE_TOUCH               0x0001 /* mouse cursor prefered   */
#define CURSORPREFERENCE_MOUSE               0x0002 /* touch cursor prefered   */

WINUSERAPI
DWORD
WINAPI
SetWindowCursorPreference(
	HWND hwnd,
	DWORD dwFlags
	);


WINUSERAPI
DWORD
WINAPI
GetWindowCursorPreference(
	HWND hwnd
	);

// @CESYSGEN ENDIF


// @CESYSGEN IF GWES_CURSOR || GWES_MCURSOR

WINUSERAPI
DWORD
WINAPI
SetSystemCursorPreference(
	DWORD dwFlags
	);


WINUSERAPI
DWORD
WINAPI
GetSystemCursorPreference(
	void
	);

// @CESYSGEN ENDIF


// Touch gesture command
#define WM_GESTURE                      0x0119

/*
 * Gesture flags - GESTUREINFO.dwFlags
 */
#define GF_BEGIN                        0x00000001
#define GF_INERTIA                      0x00000002
#define GF_END                          0x00000004
#define GF_SYNC                         0x00000008

/*
 * Gesture IDs
 */
#define GID_BEGIN                       1
#define GID_END                         2
#define GID_PAN                         4
#define GID_ROTATE                      5
#define GID_SCROLL                      8
#define GID_HOLD                        9
#define GID_SELECT                      10
#define GID_DOUBLESELECT                11
#define GID_LAST                        11

// The maximum gesture ID value that can be assigned
// to registered gestures
#define GID_MAX                         63

/* Macro to determine if the Gesture ID is for a builtin gesture, excluding
 * GID_BEGIN and GID_END
 */
#define GID_IS_BUILTIN_GESTURE(x)   (((x)>GID_END)&&((x)<=GID_LAST))

/*
 * Gesture argument helpers
 *   - Angle should be a double in the range of -2pi to +2pi
 *   - Argument should be an unsigned 16-bit value
 */
#define GID_ROTATE_ANGLE_TO_ARGUMENT(_arg_)     ((USHORT)((((_arg_) + 2.0 * 3.14159265) / (4.0 * 3.14159265)) * 65536.0))
#define GID_ROTATE_ANGLE_FROM_ARGUMENT(_arg_)   ((((double)(_arg_) / 65536.0) * 4.0 * 3.14159265) - 2.0 * 3.14159265)

// Scroll directions
#define ARG_SCROLL_NONE                 0
#define ARG_SCROLL_DOWN                 1
#define ARG_SCROLL_LEFT                 2
#define ARG_SCROLL_UP                   3
#define ARG_SCROLL_RIGHT                4

// Macros to extract scroll angles and direction
#define GID_SCROLL_ANGLE(x)              ((LONG)(HIWORD(((ULONGLONG)(x)) >> 32) & 0xFFF0))
#define GID_SCROLL_DIRECTION(x)          ((LONG)(HIWORD(((ULONGLONG)(x)) >> 32) & 0x000F))
#define GID_SCROLL_VELOCITY(x)           ((LONG)((short)(LOWORD(((ULONGLONG)(x)) >> 32))))

#define GID_TO_TGF(x)                   (1ULL << (x))

// Flags for use with enabling and disabling gestures
#define TGF_GID_PAN                     GID_TO_TGF(GID_PAN)
#define TGF_GID_SCROLL                  GID_TO_TGF(GID_SCROLL)
#define TGF_GID_HOLD                    GID_TO_TGF(GID_HOLD)
#define TGF_GID_SELECT                  GID_TO_TGF(GID_SELECT)
#define TGF_GID_DOUBLESELECT            GID_TO_TGF(GID_DOUBLESELECT)
#define TGF_GID_ALL                     (0xFFFFFFFFFFFFFFFFULL)

#define TGF_SCOPE_WINDOW                0
#define TGF_SCOPE_PROCESS               1

// @CESYSGEN IF GWES_MSGQUE

/* Gesture info structure.
 */
typedef struct tagGESTUREINFO
{
    UINT cbSize;                /* Initialised to structure size */
    DWORD dwFlags;              /* Gesture Flags */
    DWORD dwID;                 /* Gesture ID */
    HWND hwndTarget;            /* HWND of target window */
    POINTS ptsLocation;         /* Coordinates of start of gesture */
    DWORD dwInstanceID;         /* Gesture Instance ID */
    DWORD dwSequenceID;         /* Gesture Sequence ID */
    ULONGLONG ullArguments;     /* Arguments specific to gesture */
    UINT cbExtraArguments;      /* Size of extra arguments in bytes */
} GESTUREINFO, * PGESTUREINFO;

/* Gesture Information Handle
*/
DECLARE_HANDLE(HGESTUREINFO);

WINUSERAPI
BOOL
WINAPI
CloseGestureInfoHandle(
    __in HGESTUREINFO hGestureInfo);

WINUSERAPI
BOOL
WINAPI
GetGestureInfo(
    __in HGESTUREINFO hGestureInfo,
    __out PGESTUREINFO pGestureInfo);

WINUSERAPI
BOOL
WINAPI
GetGestureExtraArguments(
    __in HGESTUREINFO hGestureInfo,
    __in UINT cbExtraArguments,
    __out_bcount(cbExtraArguments) PBYTE pExtraArguments);

WINUSERAPI
BOOL
WINAPI
Gesture(
    __in_opt HWND hwnd,
    __in const GESTUREINFO* pGestureInfo,
    __in_bcount_opt(pGestureInfo->cbExtraArguments) const BYTE* pExtraArguments);

WINUSERAPI
BOOL
WINAPI
EnableGestures(
   HWND hwnd,
   ULONGLONG ullFlags,
   UINT uScope);

WINUSERAPI
BOOL
WINAPI
DisableGestures(
   HWND hwnd,
   ULONGLONG ullFlags,
   UINT uScope);

WINUSERAPI
BOOL
WINAPI
QueryGestures(
    HWND hwnd,
    UINT uScope,
    __out PULONGLONG pullFlags);

WINUSERAPI
BOOL
WINAPI
RegisterGesture(
   __in LPCWSTR pszName,
   __out PDWORD_PTR pdwID);


WINUSERAPI
BOOL
WINAPI
RegisterDefaultGestureHandler(
   HWND hwnd,
   BOOL fRegister);

// @CESYSGEN ENDIF

// @CESYSGEN IF GWES_GESTUREANIMATION
//
//  WindowAutoGesture flags
//

// General flags
#define WAGIF_OWNERANIMATE                  0x0001
#define WAGIF_VSCROLLABLE                   0x0002
#define WAGIF_HSCROLLABLE                   0x0004
#define WAGIF_LOCKAXES                      0x0008
#define WAGIF_IGNOREPAN                     0x0010
#define WAGIF_IGNORESCROLL                  0x0020

// Used for validation
#define WAGIF_LASTFLAG                      WAGIF_IGNORESCROLL

// Auto gestures animation types
// Only one auto scroll animation is available now
#define AMI_ANIMATION_SCROLL                0

// Status messages
#define WAG_STATUS_ANIMATION_START          0
#define WAG_STATUS_ANIMATION_END            1
#define WAG_STATUS_ANIMATION_CHANGE         2
// Status extra info
#define WAG_STATUS_EXTRAINFO_FINISHED       0
#define WAG_STATUS_EXTRAINFO_CONTINUOUS     1         
#define WAG_STATUS_EXTRAINFO_DISCRETE       2
#define WAG_STATUS_EXTRAINFO_INTERRUPTED    3

//
//  WindowAutoGesture structures
//

typedef struct tagWAGI {
    size_t cbSize;
    DWORD dwFlags;
    UINT nOwnerAnimateMessage;
    UINT nAnimateStatusMessage;
    HBRUSH hExtentBrush;
    UINT nItemHeight;
    UINT nItemWidth;
    BYTE bHorizontalExtent;
    BYTE bVerticalExtent;
} WAGINFO, *LPWAGINFO;

typedef struct tagANIMATEMESSAGEINFO {
    size_t cbSize;
    DWORD dwAnimationID;
    int nVPixelPosition;
    int nHPixelPosition;
} ANIMATEMESSAGEINFO, *LPANIMATEMESSAGEINFO;

typedef const WAGINFO *   LPCWAGINFO;
typedef const ANIMATEMESSAGEINFO * LPCANIMATEMESSAGEINFO;

//
//  APIs
//
WINUSERAPI
BOOL 
WINAPI 
GetWindowAutoGesture(
    __in HWND hWnd, 
    __out LPWAGINFO lpAutoGestureInfo
    );

WINUSERAPI
BOOL 
WINAPI 
SetWindowAutoGesture(
    __in HWND hWnd, 
    __in LPCWAGINFO lpAutoGestureInfo);

WINUSERAPI
BOOL
WINAPI 
GetAnimateMessageInfo(
    __in HWND hWnd, 
    __in WPARAM wParam, 
    __in LPARAM lParam, 
    __out LPANIMATEMESSAGEINFO pAnimateMessageInfo
    );

// GWES_GESTUREANIMATION
// @CESYSGEN ENDIF

#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /* _PRIV_WINUSER_ */
