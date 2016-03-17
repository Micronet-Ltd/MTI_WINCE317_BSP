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

#include <shlobj.h>
#include "shellcmds.h"

#define IDC_STATIC                        -1

// Icons
#define IDI_MULTIPLEFILES             0x1000
#define IDI_DRAGBASE                  0x1001
#define IDI_DRAGCOPY                  0x1001
#define IDI_DRAGMOVE                  0x1002
#define IDI_DRAGLINK                  0x1003
#define IDI_DRAGNONE                  0x1004

#define IDI_BITBUCKET                 0x1100
#define IDI_BITBUCKET_FULL            0x1103
#define IDI_DATABASE                  0x1101
#define IDI_MYCOMPUTER                0x1102

#define IDI_SYSFILE                   0x1200
#define IDI_TTFFILE                   0x1201
#define IDI_WAVFILE                   0x1202

#define IDI_CONFIRMMOVE               0x1300
#define IDI_CONFIRMRENAME             0x1301
#define IDI_CONFIRMREPLACE            0x1302

#define IDI_DELETE_FILE               0x1400
#define IDI_DELETE_FOLDER             0x1401
#define IDI_DELETE_MULTIPLE           0x1402
#define IDI_DELETE_PERMANENTLY        0x1407
#define IDI_MOVE                      0x1403
#define IDI_RENAME                    0x1404
#define IDI_REPLACE_FILE              0x1405
#define IDI_REPLACE_FOLDER            0x1406

// Copy file progress dialog
#define IDD_FILEOP_PROGRESS           0x2000
#define IDC_FILEOP_PROGRESS           0x2001
#define IDC_TIME                      0x2002
#define IDC_TITLE                     0x2003
#define IDC_INFO                      0x2004

// Confirm dialogs
#define IDD_CONFIRM_YESNOALLCANCEL    0x2200
#define IDD_CONFIRM_YESNO             0x2201
#define IDD_CONFIRM_REPLACE           0x2202
#define IDYESTOALL                    0x2203
#define IDC_CONFIRM_ICON              0x2204
#define IDC_CONFIRM_MESSAGE           0x2205
#define IDC_FILEICON_OLD              0x2206
#define IDC_FILEINFO_OLD              0x2207
#define IDC_FILEICON_NEW              0x2208
#define IDC_FILEINFO_NEW              0x2209

// Property and Shortcut dialogs
#define IDD_PROPERTY                  0x2300
#define IDD_SHORTCUT                  0x2301
#define IDC_ARCHIVE                   0x2302
#define IDC_FILENAME                  0x2303
#define IDC_GROUPBOX                  0x2304
#define IDC_HIDDEN                    0x2305
#define IDC_LOCATION                  0x2306
#define IDC_LOCATIONLABEL             0x2307
#define IDC_MODIFIED                  0x2308
#define IDC_MODIFIEDLABEL             0x2309
#define IDC_PICON                     0x230A
#define IDC_PSIZE                     0x230B
#define IDC_READONLY                  0x230C
#define IDC_SIZELABEL                 0x230D
#define IDC_SYSTEM                    0x230E
#define IDC_TARGET                    0x230F
#define IDC_TARGETTYPE                0x2310
#define IDC_TYPE                      0x2311

// Recycle Bin Properties dialog
#define IDD_RECYCLEBIN                0x2312
#define IDD_RECYCLEBINFULL            0x2313
#define IDC_USERECYCLEBIN             0x2314
#define IDC_PERCENTAGE                0x2315
#define IDC_MAXLABEL                  0x2316
#define IDC_PERCENTAGESLIDER          0x2317
#define IDC_TOTALSTORAGE              0x2318
#define IDC_MAXRECYCLE                0x2319
#define IDC_SWITCHTO                  0x231A
#define IDC_EMPTY                     0x231B

// Folder Options dialog
#define IDD_FOLDER_OPTIONS            0x2400
#define IDC_HIDEFILEEXTENSIONS        0x2401
#define IDC_HIDEHIDDENFILES           0x2402
#define IDC_HIDESYSTEMFILES           0x2403

// Shell Strings (Last used: 0x3089)
#define IDS_CONFIRM_DEL               0x3050
#define IDS_CONFIRM_DELCANCEL         0x3087
#define IDS_CONFIRM_DEL_BIG           0x306B
#define IDS_CONFIRM_DEL_EXE           0x304E
#define IDS_CONFIRM_DEL_MUL           0x3052
#define IDS_CONFIRM_DEL_MUL_RECBIN    0x3051
#define IDS_CONFIRM_DEL_READONLY      0x304C
#define IDS_CONFIRM_DEL_RECBIN        0x304F
#define IDS_CONFIRM_DEL_SYSTEM        0x304D
#define IDS_CONFIRM_GEN_PROTECTED     0x3070
#define IDS_CONFIRM_MOV_READONLY      0x3048
#define IDS_CONFIRM_MOV_SYSTEM        0x3049
#define IDS_CONFIRM_MRG_FOLDER        0x3053
#define IDS_CONFIRM_REN_EXTENSION     0x3072
#define IDS_CONFIRM_REN_READONLY      0x304A
#define IDS_CONFIRM_REN_SYSTEM        0x304B
#define IDS_CONFIRM_REP               0x3055
#define IDS_CONFIRM_REP_READONLY      0x3056
#define IDS_CONFIRM_REP_SYSTEM        0x3057
//#define IDS_FILECOPY_COPY             0x3001
//#define IDS_FILECOPY_OF               0x3002
#define IDS_FORMAT_MODIFIED           0x302F
#define IDS_HDR_DEFVIEW_DATE          0x3022
#define IDS_HDR_DEFVIEW_NAME          0x301F
#define IDS_HDR_DEFVIEW_SIZE          0x3020
#define IDS_HDR_DEFVIEW_TYPE          0x3021
#define IDS_HDR_RECBINVIEW_DATE       0x3024
#define IDS_HDR_RECBINVIEW_LOCATION   0x3023
#define IDS_MENU_EDIT_COPY            0x310D
#define IDS_MENU_EDIT_CUT             0x310C
#define IDS_MENU_EDIT_PASTE           0x310E
#define IDS_MENU_EDIT_PASTESHORTCUT   0x310F
#define IDS_MENU_EDIT_SELECTALL       0x3110
#define IDS_MENU_EDIT_UNDO            0x3107
#define IDS_MENU_EDIT_UNDOCPY         0x310A
#define IDS_MENU_EDIT_UNDODEL         0x3109
#define IDS_MENU_EDIT_UNDOMOVE        0x310B
#define IDS_MENU_EDIT_UNDOPASTE       0x3083
#define IDS_MENU_EDIT_UNDOREN         0x3108
#define IDS_MENU_FILE_DELETE          0x3102
#define IDS_MENU_FILE_EDIT            0x3077
#define IDS_MENU_FILE_EMPTYRECBIN     0x301C
#define IDS_MENU_FILE_NEWFOLDER       0x3101
#define IDS_MENU_FILE_OPEN            0x3100
#define IDS_MENU_FILE_PRINT           0x3078
#define IDS_MENU_FILE_PROPERTIES      0x3104
#define IDS_MENU_FILE_RENAME          0x3103
#define IDS_MENU_FILE_RESTORE         0x3106
#define IDS_MENU_FILE_SENDTO          0x3105
#define IDS_MENU_GO_MYDOCUMENTS       0x3116
#define IDS_MENU_VIEW_ARRANGEICONS    0x3114
#define IDS_MENU_VIEW_DETAILS         0x3113
#define IDS_MENU_VIEW_LARGE           0x3111
#define IDS_MENU_VIEW_OPTIONS         0x3115
#define IDS_MENU_VIEW_SMALL           0x3112
#define IDS_NET_ERR_ACCESS_DENIED     0x3079
#define IDS_NET_ERR_ACCESS_GENERIC    0x306D
#define IDS_NET_ERR_BAD_NETPATH       0x306C
#define IDS_NET_ERR_NO_NETWORK        0x306E
#define IDS_NEWFOLDER                 0x3004
#define IDS_NEWFOLDER_ERR             0x300A
#define IDS_NEWFOLDER_ERR_ACCESS      0x3008
#define IDS_NEWFOLDER_ERR_BUFFER      0x300B
#define IDS_NEWFOLDER_ERR_DISKFULL    0x3009
#define IDS_ORDER_BYTES               0x3029
#define IDS_ORDER_KB                  0x302A
#define IDS_ORDER_MB                  0x302B
#define IDS_ORDER_GB                  0x302C
#define IDS_ORDER_TB                  0x302D
#define IDS_PROPERTIES_ALLIN          0x3027
#define IDS_PROPERTIES_ERR_ACCESS     0x303C
#define IDS_PROPERTIES_ERR_NOTARGET   0x3030
#define IDS_PROPERTIES_FREE           0x3058
#define IDS_PROPERTIES_NUMOFFILES     0x3035
#define IDS_PROPERTIES_RECBINMAX      0x305A
#define IDS_PROPERTIES_SYSTEMFOLDER   0x303F
#define IDS_PROPERTIES_TYPEALL        0x3040
#define IDS_PROPERTIES_TYPEMULTI      0x3042
#define IDS_PROPERTIES_USED           0x303B
//#define IDS_RECBININIT_FAILED         0x3086
#define IDS_RENAME_NONAME             0x3043

#define APIOFFSET                     0x9000

#define IDS_SHFO_ERR_BADNAME          (APIOFFSET + 0x3044)
#define IDS_SHFO_ERR_HDR_RENAME       (APIOFFSET + 0x305F)
#define IDS_SHFO_ERR_INVALID 			(APIOFFSET + 0x3014)

/*
#define IDS_SHFO_ERR_ACCESS           0x300F
#define IDS_SHFO_ERR_ALREADYEXISTS    0x3075
#define IDS_SHFO_ERR_DISKFULL         0x3013
#define IDS_SHFO_ERR_DSTISSUBFOLDER   0x3011
*/
#define IDS_SHFO_ERR_FILENOTFOUND     0x3012
/*
#define IDS_SHFO_ERR_FOLDERLOCKED     0x3071
#define IDS_SHFO_ERR_GENERIC          0x3000
*/
#define IDS_SHFO_ERR_HDR_COPY         0x305B
#define IDS_SHFO_ERR_HDR_DELETE       0x305C
#define IDS_SHFO_ERR_HDR_MOVE         0x305E

/*
#define IDS_SHFO_ERR_INROMCOPY        0x300E
#define IDS_SHFO_ERR_INROMDELETE      0x3060
#define IDS_SHFO_ERR_INROMMOVE        0x3061
#define IDS_SHFO_ERR_INVALID          0x3014
#define IDS_SHFO_ERR_OOM              0x300D
#define IDS_SHFO_ERR_SHARING          0x3015
*/
#define IDS_SHFO_ERR_SRCDSTSAME       0x3010
#define IDS_SHFO_PROGRESS_PREP_COPY   0x3080
#define IDS_SHFO_PROGRESS_PREP_MOVE   0x3081
#define IDS_SHFO_PROGRESS_PREP_DELE   0x3082
#define IDS_SHFO_PROGRESS_FROM        0x3074
#define IDS_SHFO_PROGRESS_FROM_TO     0x3062
#define IDS_SHFO_PROGRESS_LT_MINUTE   0x3003
#define IDS_SHFO_PROGRESS_X_MINUTES   0x3007
#define IDS_SHFO_PROMPT_CREATEFOLDER  0x3005
#define IDS_SHFO_PROMPT_TOOMUCHDATA   0x3084
#define IDS_SHORTCUT_ERR              0x301E
#define IDS_STATUS_BASE               0x3088
#define IDS_STATUS_SELECTED           0x3089
#define IDS_TITLE_CONFIRMDELETE       0x3047
#define IDS_TITLE_CONFIRMMERGE        0x3076
#define IDS_TITLE_CONFIRMMOVE         0x3045
#define IDS_TITLE_CONFIRMRENAME       0x3046
#define IDS_TITLE_CONFIRMREPLACE      0x3054
#define IDS_TITLE_COPYERROR           0x3006
#define IDS_TITLE_DELETEERROR         0x3064
#define IDS_TITLE_FILEOPERATION       0x3085
#define IDS_TITLE_GENERAL             0x303A
#define IDS_TITLE_INTERNAL            0x3039
#define IDS_TITLE_MOVEERROR           0x301A
#define IDS_TITLE_NETWORKING          0x306F
#define IDS_TITLE_PROPERTIES          0x3036
#define IDS_TITLE_PROTECTEDFOLDER     0x3073
#define IDS_TITLE_RECYCLEBIN          0x303D
#define IDS_TITLE_RENAMEERROR         0x3025
#define IDS_TITLE_SHORTCUT            0x301D
#define IDS_TTSTR1                    0x3065 // Tooltip Strings sequentially hardcoded for easy use 
#define IDS_TTSTR2                    0x3066
#define IDS_TTSTR3                    0x3067
#define IDS_TTSTR4                    0x3068
#define IDS_TTSTR5                    0x3069
#define IDS_TTSTR6                    0x306A

// Display Strings (Last used: 0x500C)
#define IDS_DISPLAY_APPDATA           0x5003 // 20483
#define IDS_DISPLAY_DESKTOPDIRECTORY  0x5000 // 20480
#define IDS_DISPLAY_FAVORITES         0x5004 // 20484
#define IDS_DISPLAY_FONTS             0x5005 // 20485
#define IDS_DISPLAY_FSFOLDER          0x500C // 20492
#define IDS_DISPLAY_DRIVES            0x5002 // 20482
#define IDS_DISPLAY_MYDOCUMENTS       0x5006 // 20486
#define IDS_DISPLAY_PROGRAMFILES      0x5007 // 20487
#define IDS_DISPLAY_PROGRAMS          0x5008 // 20488
#define IDS_DISPLAY_RECENT            0x5009 // 20489
#define IDS_DISPLAY_RECYCLEBIN        0x5001 // 20481
#define IDS_DISPLAY_STARTUP           0x500A // 20490
#define IDS_DISPLAY_WINDOWS           0x500B // 20491

// Application Type Strings
#define IDS_APPTYPE_APPLICATION       0x6000 // 24576
#define IDS_APPTYPE_CPLFILE           0x6001 // 24577
#define IDS_APPTYPE_DLLFILE           0x6002 // 24578
#define IDS_APPTYPE_HELPFILE          0x6003 // 24579
#define IDS_APPTYPE_TTFFILE           0x6004 // 24580
#define IDS_APPTYPE_WAVFILE           0x6005 // 24581

// Menu & Accelerator items
#define FILE_MENU_OFFSET                   0
#define EDIT_MENU_OFFSET                   1
#define VIEW_MENU_OFFSET                   2
#define GO_MENU_OFFSET                     3
#define FOLDER_MENU_OFFSET                 0
#define FOLDER_VIEW_MENU_OFFSET            0
#define FOLDER_ARRANGEBY_MENU_OFFSET       2
#define ITEM_MENU_OFFSET                   1
#define USER_INSERT_OFFSET                 1
#define IDA_DEFVIEW                   0x4000
#define IDM_DEF_POPUP_MENUS           0x4001
#define IDM_FILE_SENDTO_MENU          0x4002
#define IDM_VIEW_ARRANGE_MENU         0x4003
#define IDM_VIEW_TYPE_MENU            0x4004
#define IDM_RECBIN_POPUP_MENUS        0x4005
#define IDC_HELP_TOPICS               0x4014
#define IDC_REFRESH                   0x4015
#define ID_ESCAPE                     0x4016
#define ID_CONTEXTMENU                0x4017

