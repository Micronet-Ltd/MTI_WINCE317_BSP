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
#ifndef __RESOURCE_H__
#define __RESOURCE_H__

#include <Windows.H>

// dialog boxes
#define IDD_PROPPAGE_MAIN               102
#define IDD_PROPPAGE_NEWPART            103
#define IDD_PROPPAGE_PARTINFO           104
#define IDD_PROPPAGE_FORMAT             105

// controls for main dialog
#define IDC_COMBO_DISK                  1001
#define IDC_LIST_PARTITIONS             1003
#define IDC_BUTTON_NEW                  1004
#define IDC_BUTTON_DELETE               1005
#define IDC_BUTTON_PROPERTIES           1006
#define IDC_BUTTON_MOUNT                1007
#define IDC_BUTTON_UNMOUNT              1008
#define IDC_BUTTON_FORMAT               1009
#define IDC_STATIC_PARTITIONS           1010
#define IDC_STATIC_INFO                 1011
#define IDC_LABEL_CAPACITY              1012
#define IDC_LABEL_UNALLOCATED           1013
#define IDC_LABLE_SECTORSIZE            1014
#define IDC_TEXT_CAPACITY               1015
#define IDC_TEXT_UNALLOCATED            1016
#define IDC_TEXT_SECTORSIZE             1017
#define IDC_BUTTON_FORMATSTORE          1018
#define IDC_BUTTON_DISMOUNTSTORE        1019
#define IDC_BUTTON_SCAN	                1020
#define IDC_BUTTON_DEFRAG	            1021


// controls for new partition dialog
#define IDC_LABEL_SIZE                  2001
#define IDC_LABEL_NAME                  2002
#define IDC_EDIT_SIZE                   2003
#define IDC_EDIT_NAME                   2004
#define IDC_CHECKBOX_SPACE              2005

// controls for partition info dialog
#define IDC_LABEL_PARTNAME              3001
#define IDC_LABLE_PARTSIZE              3002
#define IDC_LABEL_FILESYS               3003
#define IDC_LABEL_PARTTYPE              3004
#define IDC_LABEL_FLAGS                 3005
#define IDC_TEXT_PARTNAME               3006
#define IDC_TEXT_PARTSIZE               3007
#define IDC_TEXT_FILESYS                3008
#define IDC_TEXT_PARTTYPE               3009
#define IDC_TEXT_PARTFLAGS              3010

// strings
#define IDS_STRING_XSECTORS             5001
#define IDS_STRING_XBYTES               5002

// error strings
#define IDS_ERROR_PARTCREATE            6001
#define IDS_ERROR_PARTDELETE            6002
#define IDS_ERROR_PARTMOUNT             6003
#define IDS_ERROR_PARTDISMOUNT          6004
#define IDS_ERROR_PARTFORMAT            6005
#define IDS_ERROR_STOREFORMAT           6006
#define IDS_ERROR_STOREDISMOUNT         6007

// warning strings
#define IDS_CONFIRM_DELPART             6008
#define IDS_CONFIRM_DELPART_TITLE       6009
#define IDS_CONFIRM_FMTSTORE            6010
#define IDS_CONFIRM_FMTSTORE_TITLE      6011

// success strings
#define IDS_SUCCESS_FMTSTORE            6016
#define IDS_SUCCESS_FMTSTORE_TITLE      6017

// icon
#define IDI_PARTMAN                     7000

// cpl strings
#define IDS_TITLE                       8000
#define IDS_INFO                        9001
#define IDS_PROP                        9002

#endif // __RESOURCE_H__
