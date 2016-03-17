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

#define IDC_STATIC                  -1

/////////////////////////
// Menu IDs
/////////////////////////

#define IDM_LANDSCAPE             1000
#define IDM_PORTRAIT              1001
#define IDM_CONTEXT               1002

/////////////////////////
// Menu Item IDs
/////////////////////////

#define ID_DEFAULT_ACTION         2000
#define ID_STATUS                 2001
#define ID_FILE_CREATESHORTCUT    2002
#define ID_FILE_DELETE            2003
#define ID_FILE_RENAME            2004
#define ID_FILE_PROPERTIES        2005
#define ID_FILE_EXIT              2006
#define ID_EDIT_CUT               2007
#define ID_EDIT_COPY              2008
#define ID_EDIT_CREATECOPY        2009
#define ID_EDIT_PASTE             2010
#define ID_EDIT_SELECTALL         2011
#define ID_VIEW_LARGEICON         2012
#define ID_VIEW_SMALLICON         2013
#define ID_VIEW_DETAILS           2014
#define ID_VIEW_REFRESH           2015
#define ID_CONN_NEW               2016
#define ID_PROPERTIES             2017
#define ID_HELP                   2018
#define ID_ADVANCED_ADVANCED      2019
#define ID_CONN_SET_AS_DEFAULT    2020

/////////////////////////
// Icon IDs
/////////////////////////

#define IDI_REMOTENW              3000
#define IDI_NEWCONN               3001
#define IDI_RNA                   3002
#define IDI_DCC                   3003
#define IDI_VPN                   3004
#define IDI_LAN_ENABLED           3005
#define IDI_LAN_DISABLED          3006
#define IDB_TOOLBAR               3007
#define IDI_WLAN_ENABLED          3008
#define IDI_WLAN_DISABLED         3009
#define IDI_ADAPTER_UP_ENABLED    3010
#define IDI_ADAPTER_UP_DISABLED   3011
#define IDI_ADAPTER_DOWN_ENABLED  3012
#define IDI_ADAPTER_DOWN_DISABLED 3013
#define IDI_DEFAULTOVERLAY        3014
// Sneak in the button icons width and height here
#define UP_DOWN_ICON_WIDTH          10
#define UP_DOWN_ICON_HEIGHT         13

/////////////////////////
// Dialog Template IDs
/////////////////////////

#define IDD_WIZ_NEW               4000
#define IDD_RNA_0                 4001
#define IDD_RNA_1                 4002
#define IDD_DCC_0                 4003
#define IDD_VPN_0                 4004
#define IDD_PPPOE_0               4005
#define IDD_TCPIP_0               4006
#define IDD_TCPIP_1               4007
#define IDD_ADAPTER_ORDERER       4008
#define IDD_SECURITY_SETTINGS     4009
#define IDD_CONFIG_L2TP     4010

// QVGA dialogs
#define IDD_WIZ_NEW_P             4100
#define IDD_RNA_0_P               4101

#define IDD_RNA_1_P               4102
#define IDD_DCC_0_P               4103
#define IDD_VPN_0_P               4104
#define IDD_PPPOE_0_P             4105
#define IDD_TCPIP_0_P             4106
#define IDD_TCPIP_1_P             4107
#define IDD_ADAPTER_ORDERER_P     4108
#define IDD_SECURITY_SETTINGS_P   4109
#define IDD_CONFIG_L2TP_P         4110

/////////////////////////
// Dialog Control IDs
/////////////////////////

#define IDC_REMNAME               5000
#define IDC_NEW_CONN_MIN          5001
#define IDC_NEW_RNA               5001
#define IDC_NEW_DCC               5002
#define IDC_NEW_VPN               5003
#define IDC_NEW_PPPOE             5004
#define IDC_NEW_L2TPVPN             5005
#define IDC_NEW_CONN_MAX          5005
#define IDC_CONNLABEL             5004
#define IDC_CONFIG                5006
#define IDC_TCPSETTINGS           5007
#define IDC_CNTRYRGN              5008
#define IDC_AREA_CODE             5009
#define IDC_PHONE_NUM             5010
#define IDC_FORCELD               5011
#define IDC_FORCELOCAL            5012
#define IDC_TCPIP_ICON            5013
#define IDC_HOSTNAME              5014
#define IDC_USESERVIP             5015
#define IDC_USESLIP               5016
#define IDC_SOFTCOMPCHK           5017
#define IDC_IPCOMP                5018
#define IDC_SERVIPADDR            5019
#define IDC_NAMESERVADDR          5020
#define IDC_DNS_LABEL             5021
#define IDC_DNS_ALTLABEL          5022
#define IDC_WINS_LABEL            5023
#define IDC_WINS_ALTLABEL         5024
#define IDC_DNS_IPADDR            5025
#define IDC_DNS_ALTIPADDR         5026
#define IDC_WINS_IPADDR           5027
#define IDC_WINS_ALTIPADDR        5028
#define IDC_ADAPTER_LIST          5029
#define IDC_ADAPTER_UP            5030
#define IDC_ADAPTER_DOWN          5031
#define IDC_DEVICE                5032

// #define IDC_OK                    5032
// #define IDC_CANCEL                5033
#define IDC_SECSETTINGS           5034
#define IDC_CA_GB_LOGONSECURITY   5035
#define IDC_CA_CB_ENCRYPTION      5036
#define IDC_CA_CB_EAP             5038
#define IDC_CA_LB_EAPPACKAGES     5039
#define IDC_CA_PB_PROPERTIES      5040
#define IDC_CA_CB_PAP             5042
#define IDC_CA_CB_SPAP            5043
#define IDC_CA_CB_CHAP            5044
#define IDC_CA_CB_MSCHAP          5045
#define IDC_CA_CB_MSCHAP2         5046
#define IDC_CA_CB_USEWINDOWSPW    5047
#define IDC_CA_CB_PREVIEWUSERPW	  5048
#define IDC_RB_CERTAUTH                 5049
#define IDC_RB_PRESHAREDKEY         5050
#define IDC_PRESHAREDKEY                5051
#define IDC_BLUETOOTH			5052

/////////////////////////
// String IDs
/////////////////////////

// Used by the main ConnMC window
#define IDS_TITLE_LANDSCAPE       6000
#define IDS_TITLE_PORTRAIT        6001
#define IDS_HELPABOUT             6002
#define IDS_COPYRIGHT             6003
#define IDS_MAKE_NEW              6004
#define IDS_CONFDEL               6005
#define IDS_DELCON                6006
#define IDS_DELCONS               6007
#define IDS_DESKTOP_SHORTCUT      6008
#define IDS_SHORTCUT_STRING       6009
#define IDS_LV_COLUMN_0           6010
#define IDS_LV_COLUMN_1           6011
#define IDS_LV_COLUMN_2           6012
#define IDS_LV_COLUMN_3           6013

// Used by NewConnInfo
#define IDS_NEW_NEW               6100

// Used by RasConnInfo
#define	IDS_RAS_DIALING           6200
#define IDS_RAS_CONNECT           6201
#define IDS_RAS_DISCONNECT        6202
#define IDS_RAS_NEW_NAME          6203
#define IDS_RAS_COPY_OF           6204
#define IDS_RAS_COPY_NUM_OF       6205
#define IDS_TCPIP_PROP_TITLE      6206
#define IDS_RAS_DIRECT            6207
#define IDS_RAS_MODEM             6208
#define IDS_RAS_VPN               6209
#define IDS_RAS_PPPOE             6210
#define IDS_SECURITY_TITLE        6211
#define IDS_RAS_CONNECTED         6212
#define IDS_RAS_DISCONNECTED      6213
#define IDS_L2TP_TITLE            6214
#define IDS_RAS_STATUS            6215

// Used by LanConnInfo
#define IDS_LAN_ENABLE            6300
#define IDS_LAN_DISABLE           6301
#define IDS_LAN_ENABLED           6302
#define IDS_LAN_DISABLED          6303
#define IDS_LAN_LAN               6304

// Error Strings (used by everybody)
#define IDS_ERROR                 6400
#define IDS_ERR_OOM               6401
#define IDS_ERR_TOOMANYENT        6402
#define IDS_ERR_SHORTCUT          6403
#define IDS_ERR_DEL_NEW           6404
#define IDS_ERR_RENAME_NEW        6405
#define IDS_ERR_SHORTCUT_NEW      6406
#define IDS_ERR_PHONEREQ          6407
#define IDS_ERR_AREAREQ           6408
#define IDS_ERR_NULLNAME          6409
#define IDS_ERR_ALREADY_EXISTS    6410
#define IDS_ERR_BADNAME           6411
#define IDS_ERR_NOT_ALL_DEL       6412
#define IDS_ERR_NOT_ALL_COPY      6413
#define IDS_ERR_DOES_NOT_EXIST    6414
#define IDS_ERR_HOSTNAMEREQ       6415
#define IDS_ERR_BADHOSTNAME       6416
#define IDS_ERR_ALL_SPACES        6417
#define IDS_ERR_DUPLICATE         6418

// Used by ipaddr
#define IDS_ERR_TITLE             7000
#define IDS_IPBAD_FIELD_VALUE     7001

