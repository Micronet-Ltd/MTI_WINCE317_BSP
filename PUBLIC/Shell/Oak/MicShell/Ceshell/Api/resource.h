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

#define IDC_STATIC                        -1

#define IDR_QVGAENABLED               0x0999

// Icons
#define IDI_SYSTEM_ICON_FIRST         0x1500
#define IDI_DOCUMENT                  0x1500 // 0
#define IDI_FOLDER                    0x1501 // 1
#define IDI_APPLICATION               0x1502 // 2
#define IDI_STORAGECARD               0x1503 // 3
#define IDI_NETWORKFOLDER             0x1504 // 4
#define IDI_SHORTCUT                  0x1505 // 5
#define IDI_SYSTEM_ICON_LAST          0x1505
#define IDI_EXPLORER2                 0x1506

// Browse for folder dialog
#define IDD_BROWSEFORFOLDER           0x2100
#define IDC_BROWSETITLE               0x2101
#define IDC_BROWSESTATUS              0x2102
#define IDC_BROWSEEDIT                0x2103
#define IDC_FOLDERLIST                0x2104

// FileOpen dialog
#define IDD_GETOPENFILENAME           0x2500
#define IDD_GETOPENFILENAME_SIP       0x2501
#define IDC_FILEOPENCMDBAR            0x2502
#define IDC_FILEOPENHELP              0x2503
#define IDC_FILEOPENLIST              0x2504
#define IDC_FILEOPENNAME              0x2505
#define IDC_FILEOPENNAMELABEL         0x2506
#define IDC_FILEOPENTYPE              0x2507
#define IDC_FILEOPENTYPELABEL         0x2508


// API Shell strings
#define IDS_FILETYPE_APPLICATION      0x8000
#define IDS_FILETYPE_FOLDER           0x8001
#define IDS_FILETYPE_FILE             0x8002
#define IDS_FILETYPE_FILE_GENERIC     0x8003
#define IDS_FILETYPE_SHORTCUT         0x8004
#define IDS_FILETYPE_SYSTEM           0x8005
#define IDS_FILETYPE_UNKNOWN          0x8006
#define IDS_OOM                       0x8007
#define IDS_OOM_TITLE                 0x8008
#define IDS_SHORTCUT_TO               0x8009

#define IDS_SHGOFN_OPEN               0x8100
#define IDS_SHGOFN_SAVE               0x8101
#define IDS_SHGOFN_TTSTR1             0x8102
#define IDS_SHGOFN_TTSTR2             0x8103
#define IDS_SHGOFN_TTSTR3             0x8104
#define IDS_SHGOFN_ERR_CREATEPROMPT      0x8105
#define IDS_SHGOFN_ERR_FILENOTFOUND      0x8106
#define IDS_SHGOFN_ERR_INVALIDFILENAME   0x8107
#define IDS_SHGOFN_ERR_PATHDOESNOTEXIST  0x8108
#define IDS_SHGOFN_ERR_PATHTOOLONG       0x8109
#define IDS_SHGOFN_ERR_REPLACEPROMPT     0x810A


///////////////////////////////////////////////////////////////////
//// From UI

// TODO: Abstract later
// Copy file progress dialog
#define IDD_FILEOP_PROGRESS           0x2000
#define IDC_FILEOP_PROGRESS           0x2001
#define IDC_TIME                      0x2002
#define IDC_TITLE                     0x2003
#define IDC_INFO                      0x2004

#define IDYESTOALL                    0x2203
#define IDD_RECYCLEBINFULL            0x2313

#define APIOFFSET                     0x9000

#define IDS_FILECOPY_COPY             (APIOFFSET + 0x3001)
#define IDS_FILECOPY_OF               (APIOFFSET + 0x3002)
#define IDS_NEWFOLDER                 (APIOFFSET + 0x3004)

#define IDS_TITLE_COPYERROR           0x3006
#define IDS_TITLE_MOVEERROR           0x301A
#define IDS_TITLE_RENAMEERROR         0x3025
#define IDS_TITLE_DELETEERROR         0x3064

// SHFileOperation
#define IDS_SHFO_ERR_GENERIC          (APIOFFSET + 0x3000)
#define IDS_SHFO_PROGRESS_LT_MINUTE   0x3003
#define IDS_SHFO_PROGRESS_X_MINUTES   0x3007
#define IDS_SHFO_PROMPT_CREATEFOLDER  0x3005
#define IDS_SHFO_ERR_OOM              (APIOFFSET + 0x300D)
#define IDS_SHFO_ERR_INROMCOPY        (APIOFFSET + 0x300E)
#define IDS_SHFO_ERR_ACCESS           (APIOFFSET + 0x300F)

#define IDS_SHFO_ERR_SRCDSTSAME       (APIOFFSET + 0x3010)
#define IDS_SHFO_ERR_DSTISSUBFOLDER   (APIOFFSET + 0x3011)
#define IDS_SHFO_ERR_FILENOTFOUND     (APIOFFSET + 0x3012)
#define IDS_SHFO_ERR_DISKFULL         (APIOFFSET + 0x3013)
#define IDS_SHFO_ERR_INVALID          (APIOFFSET + 0x3014)
#define IDS_SHFO_ERR_SHARING          (APIOFFSET + 0x3015)

#define IDS_SHFO_ERR_BADNAME          (APIOFFSET + 0x3044)

#define IDS_PROPERTIES_RECBINMAX      0x305A
#define IDS_SHFO_ERR_HDR_COPY         (APIOFFSET + 0x305B)
#define IDS_SHFO_ERR_HDR_DELETE       (APIOFFSET + 0x305C)
#define IDS_SHFO_ERR_HDR_MOVE         (APIOFFSET + 0x305E)
#define IDS_SHFO_ERR_HDR_RENAME       (APIOFFSET + 0x305F)

#define IDS_SHFO_ERR_INROMDELETE      (APIOFFSET + 0x3060)
#define IDS_SHFO_ERR_INROMMOVE        (APIOFFSET + 0x3061)
#define IDS_SHFO_PROGRESS_FROM_TO     0x3062
#define IDS_CONFIRM_DEL_BIG           (APIOFFSET + 0x306B)

#define IDS_CONFIRM_GEN_PROTECTED     0x3070
#define IDS_SHFO_ERR_FOLDERLOCKED     (APIOFFSET + 0x3071)
#define IDS_TITLE_PROTECTEDFOLDER     0x3073
#define IDS_SHFO_PROGRESS_FROM        0x3074
#define IDS_SHFO_ERR_ALREADYEXISTS    (APIOFFSET + 0x3075)

#define IDS_SHFO_PROGRESS_PREP_COPY   0x3080
#define IDS_SHFO_PROGRESS_PREP_MOVE   0x3081
#define IDS_SHFO_PROGRESS_PREP_DELE   0x3082
#define IDS_TITLE_FILEOPERATION       0x3085
#define IDS_SHFO_PROMPT_TOOMUCHDATA   0x3084
#define IDS_RECBININIT_FAILED         (APIOFFSET + 0x3086)
