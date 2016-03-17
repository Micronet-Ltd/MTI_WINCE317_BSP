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
/*++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Module Name:  

resource.h

Abstract:  

Functions:


Notes: 


--*/
#define IDS_DIRECT_NAME			1
#define IDS_ERR_TITLE			2
#define IDS_IPBAD_FIELD_VALUE	3
#define	IDS_PORT_SETTINGS		4
#define	IDS_CALL_OPTIONS		5
#define IDS_DEVICE_PROPERTIES	6
#define IDS_PARAMETER_NONE      7
#define IDS_PARAMETER_ODD       8
#define IDS_PARAMETER_EVEN      9
#define IDS_PARAMETER_MARK      10
#define IDS_PARAMETER_SPACE     11
#define IDS_PARAMETER_SOFTWARE  12
#define IDS_PARAMETER_HARDWARE  13
#define IDS_SRVLOGON_TITLE      14
#define IDS_RESFMT_STR          15
#define IDS_SRVLOGON_CHK_STR    16
#define IDS_NETMSGBOX_TITLE     17
#define	IDS_NETWORK_IPADDR		18
#define IDS_NETWORK_NAMESERV	19
#define IDS_NETWORK_IP_TITLE	20
#define IDS_NETWORK_ERR_BADIP	21
#define IDS_PCCARD_TYPE_MFC		22
#define IDS_PCCARD_TYPE_MEMORY	23
#define IDS_PCCARD_TYPE_SERIAL	24
#define IDS_PCCARD_TYPE_PARALLEL 25
#define IDS_PCCARD_TYPE_DISK	26
#define IDS_PCCARD_TYPE_VIDEO	27
#define IDS_PCCARD_TYPE_NETWORK	28
#define IDS_PCCARD_TYPE_AIMS	29
#define IDS_PCCARD_TYPE_SCSI	30
#define IDS_PCCARD_TYPE_UNKNOWN	31
#define IDS_PCCARD_SOCKET		32

#define IDS_NETWORK_WLAN_TITLE  33
#define IDS_SRVLOGON_AUTH_FAIL_TITLE 34
#define IDS_WININET_TITLE		35
#define IDS_WININET_SITE		36
#define IDS_WININET_PROXY		37

// Resource IDs
#define GETNETSTR_BASE         40
#define IDS_NETUI_DIRECT_NAME	 (GETNETSTR_BASE + NETUI_GETNETSTR_DIRECT_NAME)
#define IDS_NETUI_OPEN_FILE_CONF (GETNETSTR_BASE + NETUI_GETNETSTR_OPEN_FILE_CONF)
#define IDS_NETUI_BAD_RES_NAME   (GETNETSTR_BASE + NETUI_GETNETSTR_BAD_RES_NAME)
#define IDS_NETUI_ERR_ADDCONNECT (GETNETSTR_BASE + NETUI_GETNETSTR_ERR_ADDCONNECT)
#define IDS_NETUI_ERR_INV_COMPNAME (GETNETSTR_BASE + NETUI_GETNETSTR_ERR_INV_COMPNAME)
#define IDS_NETUI_ERR_ADDNAME    (GETNETSTR_BASE + NETUI_GETNETSTR_ERR_ADDNAME)
#define IDS_NETUI_DISCON_ERROR   (GETNETSTR_BASE + NETUI_GETNETSTR_DISCON_ERROR)
#define IDS_NETUI_SAVE_PWD       (GETNETSTR_BASE + NETUI_GETNETSTR_SAVE_PWD)
#define IDS_NETUI_DFLT_LOCALNAME (GETNETSTR_BASE + NETUI_GETNETSTR_DFLT_LOCALNAME)
#define IDS_NETUI_NO_IPADDR		 (GETNETSTR_BASE + NETUI_GETNETSTR_NO_IPADDR)
#define IDS_NETUI_LEASE_EXPIRED	 (GETNETSTR_BASE + NETUI_GETNETSTR_LEASE_EXPIRED)
#define IDS_NETUI_CACHED_LEASE	 (GETNETSTR_BASE + NETUI_GETNETSTR_CACHED_LEASE)
#define IDS_NETUI_PRINT_COMPLETE (GETNETSTR_BASE + NETUI_GETNETSTR_PRINT_COMPLETE)
#define IDS_NETUI_PRINT_STATUS   (GETNETSTR_BASE + NETUI_GETNETSTR_PRINT_STATUS)
#define IDS_NETUI_PRINT_STATUS_OFFLINE   (GETNETSTR_BASE + NETUI_GETNETSTR_PRINT_STATUS_OFFLINE)
#define IDS_NETUI_PRINT_STATUS_PAPER     (GETNETSTR_BASE + NETUI_GETNETSTR_PRINT_STATUS_PAPER)
#define IDS_NETUI_PRINT_STATUS_OTHER     (GETNETSTR_BASE + NETUI_GETNETSTR_PRINT_STATUS_OTHER)
#define IDS_NETUI_PRINT_STATUS_NETERR    (GETNETSTR_BASE + NETUI_GETNETSTR_PRINT_STATUS_NETERR)
#define IDS_NETUI_USB_INSTALL_FAILURE    (GETNETSTR_BASE + NETUI_GETNETSTR_USB_INSTALL_FAILURE)
#define IDS_NETUI_ARP_CONFLICT_OLD_DHCP   (GETNETSTR_BASE + NETUI_GETNETSTR_ARP_CONFLICT_OLD_DHCP)
#define IDS_NETUI_ARP_CONFLICT_OLD_STATIC   (GETNETSTR_BASE + NETUI_GETNETSTR_ARP_CONFLICT_OLD_STATIC)
#define IDS_NETUI_ARP_CONFLICT_NETDOWN   (GETNETSTR_BASE + NETUI_GETNETSTR_ARP_CONFLICT_NETDOWN)


// USB string defs                                      
#define IDS_USB_CAPTION       70
#define IDS_USB_CMDTEXT       71

#define IDS_CHANGE_PASSWORD          80
#define IDS_CHANGE_PASSWORD_MISTYPE  81

#define IDD_IPADDR				100


#define IDD_PORT_SETTINGS		102
#define IDD_PORT_SETTINGS2		103
#define IDD_DIALING_SETTINGS	104
#define IDD_DIALING_SETTINGS2	105
#define IDD_ADD_CONNECTION_W    106
#define IDD_REMOVE_CONNECTION_W 107
#define IDD_RESPWD_W            108
#define IDD_USERNAME			109
#define IDD_NETWORK_IP			112
#define IDD_NETWORK_NAMESERV	113
#define IDD_DRIVERNAME			114
#define IDD_NETWORK_IP2			115
#define IDD_NETWORK_NAMESERV2	116
#define IDD_NETWORK_WLAN        117

#define IDD_NTLM_AUTH_W			124
#define IDD_REALM_AUTH_W		126
#define IDD_CHANGE_PASSWORD_W   127

// QVGA versions
#define IDD_NTLM_AUTH_N         200
#define IDD_REALM_AUTH_N        201         
#define IDD_ADD_CONNECTION_N    202
#define IDD_REMOVE_CONNECTION_N 203
#define IDD_RESPWD_N            204
#define IDD_CHANGE_PASSWORD_N   205

#define IDC_REALM_NAME			1217
#define IDC_USERNAME            1218
#define IDC_PASSWORD            1219

#define IDC_NEW_PASSWORD        1233
#define IDC_CONFIRM_PASSWORD    1234

#define IDC_SAVE_PASSWORD       1024
#define IDC_PROXY_OR_SERVER		1041
#define IDC_SERVER_OR_PROXY     1041
#define IDC_SITE_OR_FIREWALL    1042
#define IDC_DOMAIN_OR_REALM     1043
#define IDC_DOMAIN_FIELD        1010
#define IDC_DOMAIN              1010
#define IDC_DOMAIN_TAG          1043
#define IDC_REALM_FIELD         1044
#define IDC_REALM               1045
#define IDC_REALM_TAG           1045
#define IDC_USERNAME_TAG        1046
#define IDC_PASSWORD_TAG        1047

#define IDC_STATIC			-1
#define IDC_IPADDRLABEL		1000
#define IDC_IP_ADDR			1001
#define IDC_CMDTEXT         1006
#define IDC_DRIVERNAME      1007

// IDD_PORT_SETTINGS
#define	IDC_CHK_MANUAL		1006
#define	IDC_CHK_PREDIAL		1007
#define IDC_CHK_POSTDIAL	1008
#define	IDC_BAUD_RATE		1009
#define	IDC_DATA_BITS		1010
#define	IDC_PARITY			1011
#define	IDC_STOP_BITS		1012
#define	IDC_FLOW_CONTROL	1013

// IDD_DIAL_SETTINGS
#define	IDC_CHK_TIMEOUT		1014
#define IDC_TEXT_SECONDS	1015
#define	IDC_EDIT_TIMEOUT	1016
#define IDC_CHK_BLIND_DIAL	1017
#define IDC_EDIT_WAITBONG	1018
#define IDC_EXTRA			1019

// IDD_ADD_CONNECTION
#define IDC_LOCALNAME       1000
#define IDC_REMOTENAME      1001


// IDD_REMOVE_CONNECTION
#define IDC_DISCONLIST      1001

// IDD_NETWORK_IP
//#define IDC_IPADDRLABEL		1000 // These two are defined above... ReUse
//#define IDC_IP_ADDR			1001
#define IDC_ADAPTERNAME		1002
#define IDC_USEDHCP			1003
#define IDC_DONTUSEDHCP		1004
#define	IDC_SUBNETLABEL		1005
#define IDC_SUBNET_MASK		1006
#define IDC_GATEWAYLABEL	1007
#define IDC_GATEWAY			1008
#define IDC_DNS_LABEL		1009
#define IDC_DNS_ADDR		1010
#define IDC_DNS_ALTLABEL	1011
#define IDC_DNS_ALTADDR		1012
#define IDC_WINS_LABEL		1013
#define IDC_WINS_ADDR		1014
#define IDC_WINS_ALTLABEL	1015
#define IDC_WINS_ALTADDR	1016



// Translate Dialog
#define IDC_STATIC_LONG                 1030
#define IDC_STATIC_INTL                 1031
#define IDC_STATIC_LCLSET               1032
#define IDC_STATIC_DIALPAT              1033                                                 
#define IDC_STATIC_DIAL                 1034                                                 
#define IDC_STATIC_LOCAL                1035
#define IDC_TAB                         1036

                    
#define IDCB_DL_CHANGE_PATERN                   1043
#define IDCB_DL_NEW_LOCATION                    1044
#define IDCB_DL_REMOVE_LOCATION                 1045

#define IDCC_DL_CALL_WAITING                    1047
#define IDCC_DL_CURR_LOCATION                   1048

#define IDCE_DL_AREA_CODE                       1050
#define IDCE_DL_CNTRYRGN_CODE                   1051
#define IDCE_DL_INTL_CALLS                      1052
#define IDCE_DL_LOCAL_CALLS                     1053
#define IDCE_DL_LONG_DIST_CALLS                 1054
#define IDCE_NL_NEW_LOCATION                    1055

#define IDCK_DL_CALL_WAITING                    1056

#define IDCR_PULSE                              1057
#define IDCR_TONE                               1058

#define IDM_EXIT                                1059
#define IDOK_EXIT                               1060

#define IDS_ERR_ALREADY_IN_LIST                 1061
#define IDS_ERR_APPLICATION                     1062
#define IDS_ERR_INVALID_INI                     1063
#define IDS_ERR_MEMORY                          1064
#define IDS_LOCATION_REMOVE_OR_STUPID           1065
#define IDS_REMOVE_TITLE                        1066
#define IDS_WRN_LOCATION_NAME_REQUIRED          1067
#define IDS_WRN_TITLE_NAMEINUSE                 1068
#define IDS_WRN_TITLE_REQUIRED                  1069
#define IDS_WRN_TITLE_WARNING                   1070
#define IDS_CALL_WAITING_STR1                   1071
#define IDS_CALL_WAITING_STR2                   1072
#define IDS_CALL_WAITING_STR3                   1073
#define INTLMSG_RES_LOCATION                    1074

#define DIAL_DLG_POPUP_W                        1075
#define DIAL_DLG_POPUP_N                        1076
#define IDI_DIALINGICON							1077
#define IDI_SECURITY4							128

#define IDS_SUBJECT                     		1080
#define IDS_ISSUER                              1081
#define IDS_FRIENDLY_NAME						1082
#define IDS_KEY_USAGE				1083
#define IDS_NOT_BEFORE				1085
#define IDS_NOT_AFTER				1086
#define IDS_PRIVATE_KEY				1087
#define IDS_ALL_PURPOSES			1088
#define IDS_NOT_PRESENT			1089
#define IDS_PRESENT					1090
#define IDS_PUBLIC_KEY				1091
#define IDS_CERT_SERIALNUMBER		1092

#define IDB_PKEY                                  1099

#define IDD_EDIT_PATTERN_W                      1100
#define IDD_EDIT_PATTERN_N                      1101
#define IDD_NEW_LOCATION_W                      1102
#define IDD_NEW_LOCATION_N                      1103


// Wireless Zero Config defs
#define IDC_WZC_CHK_ENABLEWZC                   1200
#define IDC_WZC_LVW_BSSIDLIST                   1201
#define IDC_WZC_LVW_STSSIDLIST                  1202
#define IDC_WZC_LBL_VISNET                      1203
#define IDC_WZC_LBL_PREFNET                     1204
#define IDC_WZC_LBL_NETTYPE                     1205
#define IDC_WZC_BTN_UP                          1206
#define IDC_WZC_BTN_DOWN                        1207
#define IDC_WZC_BTN_COPY                        1208
#define IDC_WZC_BTN_RFSH                        1209
#define IDC_WZC_BTN_ADD                         1210
#define IDC_WZC_BTN_REM                         1211
#define IDC_WZC_BTN_NETTYPE                     1212
#define IDS_WZC_AUTH_OPEN                       1215  
#define IDS_WZC_AUTH_SHARED                     1216
#define IDS_WZC_AUTH_AUTO                       1217
#define IDS_WZC_INFRA_IBSS                      1218
#define IDS_WZC_INFRA_INFRA                     1219
#define IDS_WZC_INFRA_AUTO                      1220
#define IDS_WZC_ERR_DUPSSID                     1221
#define IDS_WZC_PRIV_ABS                        1222
#define IDS_WZC_PRIV_REQ                        1223
#define IDS_WZC_WEP_DISABLED                    1224
#define IDS_WZC_WEP_ENABLED                     1225
#define IDS_WZC_KLEN_40X                        1226
#define IDS_WZC_KLEN_104X                       1227
#define IDS_WZC_KLEN_40A                        1228
#define IDS_WZC_KLEN_104A                       1229
#define IDS_WZC_KFMT_X                          1230
#define IDS_WZC_KFMT_A                          1231
#define IDS_WZC_KERR_SHORT                      1232
#define IDS_WZC_KERR_LARGE                      1233
#define IDS_WZC_KERR_NONX                       1234
#define IDS_WZC_KERR_IDX                        1235
#define IDS_WZC_KFMT_LEN						1236
#define IDS_WZC_KFMT_DGTS						1237
#define IDS_WZC_INFRA_IBSS2                     1238
#define IDS_WZC_INFRA_INFRA2                    1239
#define IDS_WZC_ERROR_CAPTION                   1240
#define IDS_WZC_SSID_LEN                        1241
#define IDC_WZC_DLG_PROPS_W                     1242
#define IDC_WZC_DLG_PROPS_N                     1243
#define IDC_WZC_EDIT_SSID                       1244
#define IDC_WZC_LBL_KLEN                        1249
#define IDC_WZC_LBL_KFMT                        1251
#define IDC_WZC_LBL_KMAT                        1253
#define IDC_WZC_EDIT_KMAT                       1254
#define IDC_WZC_LBL_KIDX                        1255
#define IDC_WZC_EDIT_KIDX                       1256
#define IDC_AVAILLABEL                          1258
#define IDC_PREFERLABEL                         1259
#define IDC_ADVANCEDLABEL                       1260
#define IDC_ADVANCED                            1261
#define IDC_WZC_PREF_LIST						1262
#define IDC_WZC_PREF_UP							1263
#define IDC_WZC_PREF_DOWN						1264
#define IDC_PROPERTIES                          1265
#define IDC_WZC_ENCRYPT                         1266
#define IDC_USEHARDWAREPW                       1267
#define IDC_WZC_AUTHEN                          1268
#define IDC_WZC_GRP_WEP                         1269
#define IDC_WZC_PREF_DELETE						1270
#define IDC_WZC_CHK_FALLBACK                    1271
#define IDD_LAN_WZC_ADVANCED                    1272
#define IDB_WZCSTATE                            1273
#define IDC_WZC_CHK_ENABLE8021X                 1275
#define IDC_WZC_CB_EAPTYPE                      1276
#define IDB_WZC_EAPPROPS                        1277
#define IDC_WZC_SHOWCFG							1278
#define IDD_PICKCERT                            1279
#define IDC_CERTLISTVIEW                        1280
#define IDC_BTN_CERTVIEW						1281
#define IDC_WZC_NETACCESS_COMBO					1282
#define IDC_ADHOC								1283
#define IDD_EAPTLSCONFIG_W              1284
#define IDCB_VALIDATESERVER             1285
#define IDB_PICKCERT                    1286
#define IDC_ISSUEDTO_VALUE              1287
#define IDC_ISSUEDBY_VALUE              1288
#define IDD_EAPTLSCONFIG_N              1289

#define IDC_STATIC                      -1

#define IDD_CERTDETAILS_W               1300
#define IDC_CERT_FRIENDLYNAME           1301
#define IDC_COMBO_CERTFIELD             1302
#define IDC_CERTFIELDVALUE              1303
#define IDD_CERTDETAILS_N               1310

//{{NO_DEPENDENCIES}}
// Microsoft Developer Studio generated include file.
// Used by wzcdlg.rc
//
#define IDB_WZC_LISTICONS				2010
#define IDD_WZCQCFG_W                   2001
#define IDC_WZCQCFG_LBL_INFO            2002
#define IDC_WZCQCFG_LBL_NETWORKS        2003
#define IDC_WZCQCFG_NETWORKS            2004
#define IDC_WZCQCFG_LBL_WKINFO          2005
#define IDC_WZCQCFG_LBL_WEPK            2006
#define IDC_WZCQCFG_WEPK                2007
#define IDC_WZCQCFG_ADVANCED            2008
#define IDC_WZCQCFG_LBL_WARN            2009
#define IDS_WZCDLG_FAILED               5001
#define IDS_WZCERR_INVALID_WEPK         5002
#define IDS_WZCERR_CAPTION              5003
#define IDC_STATIC                      -1

#define IDD_IPQCFG_W                    6001
#define IDC_IP_FRAME					6006
#define IDC_ADDRTYPE_LBL                6007
#define IDC_IPADDR_LBL                  6008
#define IDC_SUBNETMSK_LBL               6009
#define IDC_GATEWAY_LBL                 6010
#define IDC_ADDRTYPE_LBL_INF            6011
#define IDC_IPADDR_LBL_INF              6012
#define IDC_SUBNETMSK_LBL_INF           6013
#define IDC_GATEWAY_LBL_INF             6014
#define IDC_WZCQCFG_SIGNAL_LBL			6015
#define IDC_WZCQCFG_SIGNAL_LBL_INF		6016
#define IDC_WZCQCFG_CONNECT				6017

#define IDC_WZCQCFG_RENEW               6019
#define IDC_WZCQCFG_DETAILS             6020

#define IDS_QCIP_TITLE					6021
#define IDS_QCWZC_TITLE					6022
#define IDS_QC_IPRENEWERR				6023
#define IDS_QC_STATIC_IP				6028
#define IDS_QC_DHCP						6029

#define IDD_NETDETAILS_W			    6030

#define IDS_IPCFG_PH_ADDR               6031
#define IDS_IPCFG_IPADDR                6032
#define IDS_IPCFG_SUBNET                6033
#define IDS_IPCFG_DEFGW                 6034
#define IDS_IPCFG_DHCP                  6035
#define IDS_IPCFG_LEASE_OBT             6036
#define IDS_IPCFG_LEASE_EXP             6037
#define IDS_IPCFG_DNSSERVERS            6038
#define IDS_IPCFG_WINSSERVERS           6039
#define IDS_IPCFG_PRAMETER              6040
#define IDS_IPCFG_VALUE                 6041

#define IDD_NETDETAILS_N			    6048
#define IDC_LIST_IPCFG                  6049

#define IDI_LAN	                    	2106
#define IDI_OBJ_TRANSFER                2107
#define IDI_PRINT                       2108
#define IDI_FILE_TRANSFER               2109
#define IDI_MODEM                       2110
#define IDI_ACTIVESYNC                  2111
#define IDI_HEADSET						2112
#define IDI_ACTIVE_LAN	               	2113
#define IDI_ACTIVE_OBJ_TRANSFER         2114
#define IDI_ACTIVE_PRINT                2115
#define IDI_ACTIVE_FILE_TRANSFER        2116
#define IDI_ACTIVE_MODEM                2117
#define IDI_ACTIVE_ACTIVESYNC           2118
#define IDI_ACTIVE_HEADSET              2119
#define IDI_TRUSTED                     2120
#define IDI_UNTRUSTED                   2121
#define IDI_BLUETOOTH                   2122
#define IDI_KEYBOARD					2123
#define IDI_ACTIVE_KEYBOARD				2124
#define IDI_MOUSE						2125
#define IDI_ACTIVE_MOUSE				2126
#define IDI_PAN						2127
#define IDI_ACTIVE_PAN				2128
#define IDI_HANDSFREE                   2129
#define IDI_ACTIVE_HANDSFREE            2130

#define IDD_QCNOTIF					6050
#define IDI_CONNECTED				6051
#define IDI_DISCONNECTED			6052
#define IDI_LANCONNECT				6054
#define IDI_LANDISCONNECT			6055

#define IDC_WZCQCFG_STATUS_LBL_INF	6056
#define IDC_WZCQCFG_STATUS_LBL 		6057

#define IDC_ND_DNSSERVERS1_LBL_INF	6058
#define IDC_ND_DNSSERVERS2_LBL_INF	6059
#define IDC_ND_WINSSERVERS1_LBL_INF	6060
#define IDC_ND_WINSSERVERS2_LBL_INF	6061

#define IDS_QC_SIGNAL0			6062
#define IDS_QC_SIGNAL1			6063
#define IDS_QC_SIGNAL2			6064
#define IDS_QC_SIGNAL3			6065
#define IDS_QC_SIGNAL4			6066
#define IDS_QC_SIGNAL5			6067

#define IDS_QC_ASSOCIATING				6068
#define IDS_QC_ASSOCIATED				6069
#define IDS_QC_FAILED_ASSOCIATE			6070
#define IDS_QC_AUTHENTICATING			6071
#define IDS_QC_SCANNING					6072
#define IDS_QC_FAILED_AUTHENTICATION	6073
#define IDS_QC_AUTHENTICATED			6074
#define IDS_QC_CONNECTED				6075
#define IDS_QC_REAUTHENTICATING			6076
#define IDS_QC_CANCEL_AUTHENTICATION	6077
#define IDS_QC_NOT_CONNECTED			6078

#define IDS_QC_WZCSTARTED				6079
#define IDS_QC_CLOSE_EAPOL_SESSION		6080
#define IDS_QC_PREF_LIST_EXHAUSTED		6081
#define IDS_QC_PREFERRED 				6082
#define IDS_QC_ADDNEW					6083

#define IDC_WZCQCFG_LOGGING				6084
#define IDD_QC_WZCLOG_W					6085
#define IDC_QC_WZCLOG					6086

#define IDS_QC_CONNECT					6088
#define IDS_QC_CONFIGURE				6089
#define IDS_QC_DELETE					6090


#define IDD_IPV6DETAILS_W               6100 
#define IDC_V6PREFGLOBADDR_LBL          6101 
#define IDC_V6PREFSITEADDR_LBL          6102 
#define IDC_V6LINKADDR_LBL              6103 
#define IDC_V6GATEWAY_LBL               6104 
#define IDC_V66TO4ADDR_LBL              6105 
#define IDC_V6TUNNELADDR_LBL            6106 
#define IDC_V6PREFGLOBADDR_LBL_INF      6107 
#define IDC_V6PREFSITEADDR_LBL_INF      6108 
#define IDC_V6PREFLINKADDR_LBL_INF      6109 
#define IDC_V6GATEWAY_LBL_INF           6110 
#define IDC_V66TO4ADDR_LBL_INF          6111 
#define IDC_V6TUNNELADDR_LBL_INF1       6112 
#define IDC_V6TUNNELADDR_LBL_INF2       6113 
#define IDC_V6TUNNELADDR_LBL_INF3       6114 
#define IDC_IPV6_DETAILS                6115 
#define IDC_IPV6_FRAME                  6116 

#define IDS_QC_V6_TITLE                 6117
#define IDS_QC_V6_ADDR_UNAVAIL          6118

#define IDS_WZCADV_ALLAVAIL				6119
#define IDS_WZCADV_ACCESSPTS			6120
#define IDS_WZCADV_ADHOC				6121

#define IDM_WZC_CONFIGURE				6125
#define IDM_WZC_CONNECT					6126
#define IDM_WZC_DELETE					6127

#define IDS_WZC_OPEN					6130
#define IDS_WZC_SHARED					6131
#define IDS_WZC_SSN						6132
#define IDS_WZC_SSNPSK					6133
#define IDS_WZC_DISABLED				6134
#define IDS_WZC_WEP						6135
#define IDS_WZC_TKIP					6136
#define IDS_WZC_AES						6137
#define IDS_WZC_WPA2PSK                 6138
#define IDS_WZC_WPA2                    6139

#define IDC_USEPW						6140

#define IDS_SERVER_VALIDATION_ERROR     6150
#define IDS_UNKNOWN_ROOT_ERROR          6151
#define IDS_CERT_EXPIRED_ERROR          6152
#define IDS_UNKNOWN_CERT_ERROR          6153
#define IDS_GENERIC_CERT_ERROR          6154


#define IDD_IPQCFG_N                    6200
#define IDD_IPV6DETAILS_N               6201
#define IDD_WZCQCFG_N                   6202
#define IDD_QC_WZCLOG_N					6203


#define	IDS_IPV6_COL_PROPERTY			6300
#define	IDS_IPV6_COL_VALUE				6301
										
#define	IDS_IPV6_GLOBAL					6302
#define	IDS_IPV6_SITE_LOCAL				6303
#define	IDS_IPV6_LINK_LOCAL				6304
#define	IDS_IPV6_6TO4					6305
#define	IDS_IPV6_AUTO_TUNNEL			6306
#define	IDS_IPV6_AUTO_TUNNEL2			6307
#define	IDS_IPV6_AUTO_TUNNEL3			6308
#define	IDS_IPV6_DEFAULT_GATEWAY		6309
#define IDC_IPV6_LISTBOX				6310

//	wininet/netui ShowCertitifacate
#define IDD_VIEW_CERT_W					1301
#define IDD_VIEW_CERT_N					1310

#define IDS_ALG_MD2                     1800
#define IDS_ALG_MD4                     1801
#define IDS_ALG_MD5                     1802
#define IDS_ALG_SHA                     1803
#define IDS_ALG_MAC                     1804
#define IDS_ALG_HMAC                    1805
#define IDS_ALG_RSA_SIGN                1806
#define IDS_ALG_DSS_SIGN                1807
#define IDS_ALG_RSA_KEYX                1808
#define IDS_ALG_DES                     1809
#define IDS_ALG_3DES_112                1810
#define IDS_ALG_3DES                    1811
#define IDS_ALG_RC2                     1812
#define IDS_ALG_RC4                     1813
#define IDS_ALG_RC5                     1814
#define IDS_ALG_SEAL                    1815
#define IDS_ALG_DH_SF                   1816
#define IDS_ALG_DH_EPHEM                1817
#define IDS_ALG_KEA_KEYX                1818
#define IDS_ALG_SKIPJACK                1819
#define IDS_ALG_TEK                     1820

#define IDS_LW95_ENTERAUTHINFO          1821
#define IDS_SECERT_CERTINFO             1822
#define IDS_SECERT_STRENGTH_HIGH        1823
#define IDS_SECERT_STRENGTH_MEDIUM      1824
#define IDS_SECERT_STRENGTH_LOW         1825
#define IDS_CERT_SUBJECT                1826
#define IDS_CERT_ISSUER                 1827
#define IDS_CERT_EFFECTIVE_DATE         1828
#define IDS_CERT_EXPIRATION_DATE        1829
#define IDS_CERT_PROTOCOL               1830
#define IDS_CERT_USAGE                  1831

#define IDS_COMMENT_EXPIRES             1832
#define IDS_COMMENT_NOT_VALID           1833
#define IDS_COMMENT_BAD_CN              1834
#define IDS_COMMENT_BAD_CA              1835
#define IDS_COMMENT_BAD_SIGNATURE       1836
#define IDS_COMMENT_REVOKED             1837

#define IDS_STRING_CIPHMSG              1838
#define IDS_STRING_HASHMSG              1839
#define IDS_STRING_EXCHMSG              1840
#define IDS_CERT_FINGERPRINT            1841
#define IDS_DOMAIN                      1842
#define IDS_REALM                       1843
#define IDS_SITE                        1844
#define IDS_FIREWALL                    1845

#define IDS_PROTOCOL_UNKNOWN            1846
#define IDS_PROTOCOL_SSL2               1847
#define IDS_PROTOCOL_SSL3               1848
#define IDS_PROTOCOL_PCT1               1849
#define IDS_PROTOCOL_TLS1               1850

#define IDS_CERT_HASH_ALG               1851
#define IDS_CERT_EXCH_ALG               1852
#define IDS_CERT_COMMENT                1853
#define IDS_CERT_ENCRYPT_ALG            1854
#define IDS_CIPHER_UNKNOWN              1855
#define IDS_HASH_UNKNOWN                1856
#define IDS_EXCH_UNKNOWN                1857
#define IDS_CERT_FRIENDLY_ERROR		1858
#define IDS_CERT_FRIENDLY_ERR_TITLE	1859

#define IDC_FIELDLIST                   1005
#define IDC_CERT_STATUS                 1009
#define IDC_DETAILSLIST                 1011
