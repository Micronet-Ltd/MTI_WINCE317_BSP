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
#define IDC_STATIC                      -1

#include <shlobj.h>

// command bar
#define ID_BAND_CMD             20
#define ID_BAND_TOOL            21
#define ID_BAND_ADDR            22
#define ID_BAND_INVALID         23
#define ID_ADDRESSBAR           24
#define ID_COMMANDBAND          25
#define ID_STATUSBAR            26
// bitmaps
#define IDB_COOLBARIMAGES       50
#define IDB_TBBITMAP            51
#define IDB_ANIMATION           52


////////////////////////////////////////////////////////////////////////////
// Icons
//

#define IDI_FLAG                2

#define IDI_STPROGS             20
#define IDI_STDOCS              21
#define IDI_STSETNGS            22
#define IDI_STHELP              24
#define IDI_STRUN               25
#define IDI_STSUSPD             26
#define IDI_STFAVORITES         29
#define IDI_STCTLPNL            30
#define IDI_STTASKBAR           31
#define IDI_STCONNMC            32

#define IDI_RUNDLG              160      // Icon in the Run dialog
#define IDI_HTML                108

// Zone (and lock) icons need to be in order
#define IDI_ZONE                300
#define IDI_INTRANET            301
#define IDI_TRUSTED             302
#define IDI_INTERNET            303
#define IDI_RESTRICTED          304
#define IDI_LOCK                305

// Icons for power stuff
#define IDI_POWER               310
#define IDI_LOW                 312
#define IDI_VERYLOW             313
#define IDI_LOWBKUP             314
#define IDI_VLOWBKUP            316
#define IDI_CHARGING            317

#define IDI_DESKTOP                     1208
#define IDI_SPINNER                     1207
#define IDI_CAPSLOCK                    1234
#define IDI_RECYCLERFULL                1235
#define IDI_SIP                         1236
#define IDI_SIP2                        1237

// menu
#define IDR_MAINMENU                    100
#define IDR_MAINMENU_PIE                101
#define IDM_CONTEXT_TASKBAR             109
#define IDM_CONTEXT_TASKBAR_BUTTON      110
#define IDM_CONTEXT_TASKBAR_EXTRAS      111
#define IDM_CONTEXT_TASKBAR_SIP         112


// Accelerator
#define IDR_ACCELERATOR                 201

// dialogs
#define IDD_OPEN_DIALOG                 1000
#define DLG_RUN                         1003
#define IDD_TASKBAR_OPTIONS             1004
#define IDD_TASKBAR_DOCUMENTS           1005
#define IDD_TASK_MANAGER                1006
#define IDD_BACKUP_LOW                  1010
#define IDD_BACKUP_VLOW                 1011
#define IDD_MAIN_VLOW                   1012
#define IDD_COMMAND                     1013
#define IDD_SHUTDOWN                    1014
#define IDD_BROWSE                      1015
#define IDD_PAGEPROPERTIES              1016
// Dialog items
#define IDC_URL_EDIT                    3000
#define IDC_BROWSE                      3001
#define IDC_NEWWINDOW                   3002

#define IDC_PROP_HEADER1                3030
#define IDC_PROP_TITLE                  3031
#define IDC_PROP_PROTOCOL_LABEL         3032
#define IDC_PROP_PROTOCOL               3033
#define IDC_PROP_TYPE_LABEL             3034
#define IDC_PROP_TYPE                   3035
#define IDC_PROP_ITEMSIZE_LABEL         3036
#define IDC_PROP_ITEMSIZE               3037
#define IDC_PROP_SECURITY_LABEL         3038
#define IDC_PROP_SECURITY               3039
#define IDC_PROP_ADDRESS_LABEL          3040
#define IDC_PROP_ADDRESS                3041


//Bitmaps
#define IDB_STPEGASUS                   200      // Start menu banner bitmap
#define IDB_STPEGASUSG                  201      // Start menu banner bitmap

// strings
#define IDS_IE                          20000
#define IDS_ADDRESS                     20001
#define IDS_FILESCHEME                  20002
#define IDS_HYPHEN                      20003
#define IDS_EMPTY                       20004
#define IDS_ORGANIZEFAVORITES           20005
#define IDS_ADDTOFAVORITES              20006
#define IDS_ADDFAVTITLE                 20007
#define IDS_BROWSEFILTER                20010
#define IDS_ERR_GENERIC                 20011
#define IDS_UNKNOWN                     20012

// keep zones in contiguous order
#define IDS_ZONEBASE                    20100
#define IDS_ZONELOCAL                   20100
#define IDS_ZONEINTRA                   20101
#define IDS_ZONETRUSTED                 20102
#define IDS_ZONEINTER                   20103
#define IDS_ZONERESTRICTED              20104

//mainwnd.cpp
#define IDS_FILE                        22000
#define IDS_FILE_CLOSE                  22001
#define IDS_EDIT                        22050
#define IDS_VIEW                        22100
#define IDS_VIEW_REFRESH                22101
#define IDS_VIEW_HIDETOOLBARS           22102
#define IDS_VIEW_ADDRESSBAR             22103
#define IDS_VIEW_STATUSBAR              22104
#define IDS_GO                          22150
#define IDS_GO_BACK                     22151
#define IDS_GO_FORWARD                  22152
#define IDS_FAVORITES                   22200

#define IDS_IE_TTSTR                    22300    // Tooltips strings
#define IDS_IE_TTSTR1                   (IDS_IE_TTSTR + 1)
#define IDS_IE_TTSTR2                   (IDS_IE_TTSTR + 2)
#define IDS_IE_TTSTR3                   (IDS_IE_TTSTR + 3)
#define IDS_IE_TTSTR4                   (IDS_IE_TTSTR + 4)
#define IDS_IE_TTSTR5                   (IDS_IE_TTSTR + 5)
#define IDS_IE_TTSTR6                   (IDS_IE_TTSTR + 6)

//taskbar.cpp
#define IDS_EXPLORER                    21000
#define IDS_START                       21001
#define IDS_PROGRAMS                    21002
#define IDS_PROGRAMS1                   IDS_PROGRAMS + 1
#define IDS_PROGRAMS2                   IDS_PROGRAMS + 2
#define IDS_PROGRAMS3                   IDS_PROGRAMS + 3
#define IDS_CTLPNLKEYBOARD              21100
#define IDS_CTLPNLSTYLUS                21101
#define IDS_CTLPNLPOWER                 21102

#define IDS_TASKBAR_OPTIONS             21104
#define IDS_STARTMENU                   21105
#define IDS_HELPFILE                    21106

#define IDS_RUN                         21108
#define IDS_BROWSE                      21109
#define IDS_TSBTITLE                    21110
#define IDS_TSB                         21111
#define IDS_CLICK_HERE_TO_BEGIN         21112
#define IDS_SHOW_DESKTOP                21113
#define IDS_POWER_TRAY                  21114
#define IDS_DESKTOP                     21115
#define IDS_SIP_DESC                    21118
#define IDS_CLOSE_SIP                   21119
#define IDS_APP_BUTTON_TT               21120

//vga-qvga strings
#define IDS_TASKBAR_PROPERTIES          21200

//dst.cpp
#define IDS_DSTCHANGED                  25000
#define IDS_DSTTIMEINFO                 25001

//stmenu.cpp
#define IDS_MENUEXPLORE                 0x8a00
#define IDS_MENUPROGRAMS                0x8a01
#define IDS_MENUDOCUMENTS               0x8a02
#define IDS_MENUSETTING                 0x8a03
#define IDS_MENUFIND                    0x8a04
#define IDS_MENUHELP                    0x8a05
#define IDS_MENURUN                     0x8a06
#define IDS_NULL                        0x8a07
#define IDS_MENUSUSPEND                 0x8a08
#define IDS_MENUFAVORITES               0x8a09
#define IDS_MENUCTLPNL                  0x8a0a
#define IDS_MENUTASKBAR                 0x8a0b
#define IDS_MENUCONNMC                  0x8a0c

// Supported keyboard layouts
// New Input Locales should be appended to this list.
// NOTE: It is OEM responsibility to keep this in sync with inputlang.cpp.
#define IDS_BASE_HKL__                  0x9000
#define IDS_HKL_00000401                (IDS_BASE_HKL__+0X0000)
#define IDS_HKL_00000409                (IDS_BASE_HKL__+0X0001)
#define IDS_HKL_00010409                (IDS_BASE_HKL__+0X0002)
#define IDS_HKL_0000040D                (IDS_BASE_HKL__+0X0003)
#define IDS_HKL_00000411                (IDS_BASE_HKL__+0X0004)
#define IDS_HKL_00000412                (IDS_BASE_HKL__+0X0005)
#define IDS_HKL_0000041E                (IDS_BASE_HKL__+0X0006)
#define IDS_HKL_00010439                (IDS_BASE_HKL__+0X0007)
#define IDS_HKL_00000446                (IDS_BASE_HKL__+0X0008)
#define IDS_HKL_00000447                (IDS_BASE_HKL__+0X0009)
#define IDS_HKL_00000449                (IDS_BASE_HKL__+0X000a)
#define IDS_HKL_0000044A                (IDS_BASE_HKL__+0X000b)
#define IDS_HKL_0000044B                (IDS_BASE_HKL__+0X000c)
#define IDS_HKL_0000044E                (IDS_BASE_HKL__+0X000d)


// commands

// These Command IDs are reserved by MSHTML for MIME encoding menu IDs
// But passed to this WM_COMMAND handler
// Listed here for reference, the actual definition is in mshtmcid.h 
//#define IDM_MIMECSET__FIRST__           3609
//#define IDM_MIMECSET__LAST__            3699


// Common command ranges
#define ID_COMMON_FIRST__               FCIDM_GLOBALFIRST
#define ID_COMMON_LAST__                FCIDM_GLOBALLAST
    

// All IE specific commands should lie in this range
#define ID_IE_FIRST__                   FCIDM_BROWSERFIRST
#define ID_IE_LAST__                    FCIDM_BROWSERLAST


// Shell command ranges
#define ID_SHELL_FIRST__                FCIDM_SHVIEWFIRST
#define ID_SHELL_LAST__                 FCIDM_SHVIEWLAST

// favorites reserve the following range
#define IDM_FAVORITES_FIRST__           (ID_IE_FIRST__+0x0100)
#define IDM_FAVORITES_LAST__            (ID_IE_FIRST__+0x01C8)

// Extras menu reserves the following range
#define IDM_DESKTOP                     (ID_IE_FIRST__+0x01FF)
#define IDM_EXTRAS_MENU_FIRST__         (ID_IE_FIRST__+0x0200)
#define IDM_EXTRAS_MENU_LAST__          (ID_IE_FIRST__+0x02C8)

#define IDM_SIP_MENU_FIRST__            (ID_IE_FIRST__+0x0300)
#define IDM_SIP_MENU_LAST__             (ID_IE_FIRST__+0x03C8)

// BEGIN: IE Commands
#define ID_VIEW_STOP                    (ID_IE_FIRST__+0x0000)
#define ID_VIEW_REFRESH                 (ID_IE_FIRST__+0x0001)
#define ID_VIEW_FONTSLARGEST            (ID_IE_FIRST__+0x0006)
#define ID_VIEW_FONTSLARGER             (ID_IE_FIRST__+0x0005)
#define ID_VIEW_FONTSMEDIUM             (ID_IE_FIRST__+0x0004)
#define ID_VIEW_FONTSSMALLER            (ID_IE_FIRST__+0x0003)
#define ID_VIEW_FONTSSMALLEST           (ID_IE_FIRST__+0x0002)
#define ID_VIEW_ENCODING                (ID_IE_FIRST__+0x0007)

#define ID_FILE_OPEN                    (ID_IE_FIRST__+0x0008)
#define ID_FILE_FIND                    (ID_IE_FIRST__+0x0009)
#define ID_FILE_SAVEAS                  (ID_IE_FIRST__+0x000A)
#define ID_EDIT_CUT                     (ID_IE_FIRST__+0x000B)
#define ID_EDIT_COPY                    (ID_IE_FIRST__+0x000C)
#define ID_EDIT_PASTE                   (ID_IE_FIRST__+0x000D)

#define ID_FOCUS_URL                    (ID_IE_FIRST__+0x000E)
#define ID_OPEN                         (ID_IE_FIRST__+0x000F)
#define ID_GO_HOME                      (ID_IE_FIRST__+0x0010)
#define ID_GO_SEARCH                    (ID_IE_FIRST__+0x0011)
#define ID_FULLSCREEN                   (ID_IE_FIRST__+0x0012)
#define ID_INTERNET_OPTIONS             (ID_IE_FIRST__+0x0013)
#define ID_ZOOMUP                       (ID_IE_FIRST__+0x0014)
#define ID_ZOOMDOWN                     (ID_IE_FIRST__+0x0015)
#define ID_POPUP_URL                    (ID_IE_FIRST__+0x0016)
#define ID_HELP_TOPICS                  (ID_IE_FIRST__+0x0017)

#define IDM_FITTOSCREEN                 (ID_IE_FIRST__+0x0020)
#define IDM_SHOWPICTURES                (ID_IE_FIRST__+0x0021)
#define IDM_STATUSBAR_TOGGLE            (ID_IE_FIRST__+0x0022)
#define IDM_FILE_PROPERTIES             (ID_IE_FIRST__+0x0023)


// END: IE Commands

// Common Commands
#define ID_CLOSE                        (ID_COMMON_FIRST__+0x0000)
#define ID_TOOLBARS_TOGGLE              (ID_COMMON_FIRST__+0x0080)
#define ID_ADDRESSBAR_TOGGLE            (ID_COMMON_FIRST__+0x0081)

#define ID_GO_BACK                      (ID_COMMON_FIRST__+0x0150)
#define ID_GO_FORWARD                   (ID_COMMON_FIRST__+0x0151)

#define ID_FAV_ADD                      (ID_COMMON_FIRST__+0x0170)
#define ID_FAV_ORG                      (ID_COMMON_FIRST__+0x0171)
#define ID_FAV_PLACEHOLDER              (ID_COMMON_FIRST__+0x0172)

#define ID_ANIMATIONWINDOW              (ID_COMMON_FIRST__+0x0175)
#define ID_ANIMATIONTIMER               (ID_COMMON_FIRST__+0x0176)


// IDs for the Taskbar Options
#define IDC_TASKBAR_ONTOP               3000
#define IDC_TASKBAR_AUTOHIDE            3001
#define IDC_TASKBAR_SHOWCLOCK           3002
#define IDC_EXPAND                      3010

// Constants for Task Manager Dialog
#define IDC_LISTBOX                     3020
#define IDC_SHUTDOWN                    3021
#define IDC_ENDTASK                     3022

////////////////////////////////////////////////////////////////////////////
// Taskbar Command IDs
//
#define IDM_TASKBAR_FULL                100
#define IDM_TASKBAR_WARNING             103

#define IDM_TASKBAR_MIN                 3200
#define IDM_TASKBAR_MAX                 3201
#define IDM_TASKBAR_CLOSE               3202

#define IDM_TASKBAR_PROPERTIES          3300
#define IDM_TASKBAR_DESTROY_STARTMENU   3301
#define IDM_TASKBAR_TASKMAN             3302
#define ID_TASKBARCLOCK                 3303
#define IDM_TASKBAR_EXTRAS_PLACEHOLDER  3304
#define IDM_TASKBAR_SIP_PLACEHOLDER     3305

#define IDD_DESKTOPLISTVIEW             4242

#define IDD_ICON                        4000
#define IDD_PROMPT                      4001
#define IDD_RUNDLGOPENPROMPT            4011

////////////////////////////////////////////////////////////////////////////
// Start Menu Command IDs
//
// NOTE: start menu commands must be < 64k (an invalid pointer)
#define IDM_START_EXPLORE               12
#define IDM_START_PROGRAMS              13
#define IDM_START_DOCUMENTS             14
#define IDM_START_SETTINGS              15
#define IDM_START_FIND                  16
#define IDM_START_HELP                  17
#define IDM_START_RUN                   18
#define IDM_START_SUSPEND               19
#define IDM_START_FAVORITES             20
#define IDM_START_LAST_BANNER           20
// >= 30 and < 64k don't use the start banner
#define IDM_START_CTLPNL                30
#define IDM_START_TASKBAR               31
#define IDM_START_CONNMC                32
#define IDM_START_LAST_NO_BANNER        32

