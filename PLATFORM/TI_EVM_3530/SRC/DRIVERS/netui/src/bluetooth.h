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
//
//	This code implements user part of activation of Bluetooth peers.
//
//	It creates registry key upon which the boot time activation implemented
//	in public\common\oak\drivers\bluetooth\samples\btsvc is heavily dependent.
//
//	Please maintain synchronization between the two files.
//

#define IDS_BT_TITLE					2019
#define IDS_BT_PROP						2020
#define IDI_QUESTIONMARK				2021
#define IDS_BT_ERROR					2022
#define IDS_BT_AUTH_FAILED				2023
#define IDS_BT_UNNAMED					2024
#define IDS_BT_STOP						2025
#define IDS_BT_SCAN						2026
#define IDS_BT_IN_INQUIRY				2027
#define IDS_BT_HW_ERROR					2028
#define IDS_BT_FAIL_COMM				2029
#define IDS_BT_CAPTION					2030
#define IDS_BT_NO_HW                                    2031
#define IDS_BT_CANCELLED                                2032
#define IDS_BT_OP_INVALID                               2033
#define IDS_BT_OOM                                      2034
#define IDS_BT_CANT_CONNECT                             2035
#define IDS_BT_AUTH_NO_DRV                              2036
#define IDS_BT_AUTH_CNCT                                2037
#define IDS_BT_REQ_AUTH									2038

//definations added for the bluetooth management UI resources
#define IDD_SCAN_DC_P                   2103
#define IDD_ENTERPIN                    2104
#define IDD_AUTHENTICATE                2105
#define IDM_ACTIVE                      2120
#define IDM_ACTIVEMENU                  2121
#define IDM_ACTIVEMENU_P                2122
#define IDM_TRUSTED                     2123
#define IDM_DELETE                      2124
#define IDM_AUTH                        2125
#define IDM_ENCRYPT                     2126
#define IDC_UNTRUSTED                   2001
#define IDC_TRUSTED                     2002
#define IDC_MOVE_TO_TRUSTED             2003
#define IDC_MOVE_TO_UNTRUSTED           2004
#define IDC_SCAN_DEVICE                 2005
#define IDC_TRUSTED_TEXT                2008
#define IDC_UNTRUSTED_TEXT              2009
#define IDC_PIN                         2015
#define IDC_OK                          2017
#define IDC_CANCEL                      2018
#define DLG_SCAN_DC                     2602

// Include these defines if we are doing an SDK_BUILD (since they won't be taken from resource.h)
#ifdef SDK_BUILD
#define IDI_LAN                         2106
#define IDI_OBJ_TRANSFER                2107
#define IDI_PRINT                       2108
#define IDI_FILE_TRANSFER               2109
#define IDI_MODEM                       2110
#define IDI_ACTIVESYNC                  2111
#define IDI_HEADSET                     2112
#define IDI_ACTIVE_LAN                  2113
#define IDI_ACTIVE_OBJ_TRANSFER         2114
#define IDI_ACTIVE_PRINT                2115
#define IDI_ACTIVE_FILE_TRANSFER        2116
#define IDI_ACTIVE_MODEM                2117
#define IDI_ACTIVE_ACTIVESYNC           2118
#define IDI_ACTIVE_HEADSET              2119
#define IDI_TRUSTED                     2120
#define IDI_UNTRUSTED                   2121
#define IDI_BLUETOOTH                   2122
#define IDI_KEYBOARD                    2123
#define IDI_ACTIVE_KEYBOARD             2124
#define IDI_MOUSE                       2125
#define IDI_ACTIVE_MOUSE                2126
#define IDI_PAN                         2127
#define IDI_ACTIVE_PAN                  2128
#define IDI_HANDSFREE                   2129
#define IDI_ACTIVE_HANDSFREE            2130

#endif

