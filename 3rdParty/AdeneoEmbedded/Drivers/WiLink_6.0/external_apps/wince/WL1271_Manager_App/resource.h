/***************************************************************************
**+----------------------------------------------------------------------+**
**|                                ****                                  |**
**|                                ****                                  |**
**|                                ******o***                            |**
**|                          ********_///_****                           |**
**|                           ***** /_//_/ ****                          |**
**|                            ** ** (__/ ****                           |**
**|                                *********                             |**
**|                                 ****                                 |**
**|                                  ***                                 |**
**|                                                                      |**
**|     Copyright (c) 1998-2010 Texas Instruments Incorporated           |**
**|                        ALL RIGHTS RESERVED                           |**
**|                                                                      |**          
**| Permission is hereby granted to licensees of Texas Instruments       |**
**| Incorporated (TI) products to use this computer program for the sole |**
**| purpose of implementing a licensee product based on TI products.     |**
**| No other rights to reproduce, use, or disseminate this computer      |**
**| program, whether in part or in whole, are granted.                   |**
**|                                                                      |**
**| TI makes no representation or warranties with respect to the         |**
**| performance of this computer program, and specifically disclaims     |**
**| any responsibility for any damages, special or consequential,        |**
**| connected with the use of this program.                              |**
**|                                                                      |**     
**+----------------------------------------------------------------------+**
***************************************************************************/

//{{NO_DEPENDENCIES}}
// Microsoft Visual C++ generated include file.
// Used by WL1271_Manager_App.rc
//
#define IDD_MAINDIALOG                  101
#define IDD_WIFIDIALOG                  102
#define IDD_MACDIALOG                   103
#define IDD_SCANDIALOG					104

#define IDC_STATIC						1
#define IDI_WLAPP                       107

//--- Bitmaps
#define IDB_BT_OFF                      108
#define IDB_BT_ON                       109
#define IDB_WL_OFF                      110
#define IDB_WL_ON                       111
#define IDB_FM_OFF                      112
#define IDB_FM_ON                       113

#define IDD_GLOBALCONFDIALOG			120
#define IDD_MISCANDCHANDIALOG			121
#define IDD_ASGLOBALDIALOG				122
#define IDD_ASCHANDIALOG				123
#define IDD_ROAMINGDIALOG				124

// Activate Buttons
#define ID_ACTIVATE_BT	                1004
#define ID_ACTIVATE_WL	                1005
#define ID_ACTIVATE_FM				    1006

// Configure
#define ID_CONF_BT	                	1007
#define ID_CONF_WL	                	1008
#define ID_CONF_FM				    	1009

// #define IDC_BBT                     	1007
// #define IDC_BWL                     	1008
// #define IDC_BFM						1009

// WLan configuration
#define ID_WL_CONF_MAC					1020
#define ID_WL_CONF_BIP					1021
#define ID_WL_CONF_ROAMING				1022
#define ID_WL_CONF_SCAN					1023
#define ID_WL_START_SCAN				1024
#define ID_WL_STOP_SCAN					1025
#define ID_WL_GLOBAL					1026
#define ID_WL_MISC						1027
#define ID_WL_ASG						1028
#define ID_WL_ROAMING					1029

#define IDC_STATIC_MAC					1050
#define IDC_STATIC_CAL					1051
#define IDC_STATIC_BIP					1052
#define IDC_STATIC_SCAN					1053


// MAC Window
#define IDC_STATIC_1					1150
#define IDC_STATIC_2					1151
#define IDC_EDIT_MAC					1152

//--- Displayed bitmaps
#define IDC_BT_OFF						2000
#define IDC_BT_ON						2001
#define IDC_WL_OFF						2002
#define IDC_WL_ON						2003
#define IDC_FM_OFF						2004
#define IDC_FM_ON						2005

// Scan Window
// Mode Selection
#define ID_WL_CONF_IMMSCAN				2100 // Immediate Scan Mode
#define ID_WL_CONF_DISC					2101 // Discovery Mode
#define ID_WL_CONF_TRACK				2102 // Tracking Mode
#define IDC_STATIC_MODESEL				2103 // Mode Selection GroupBox ID

// Band Indexes
#define ID_WL_BANDINDEX0				2110
#define ID_WL_BANDINDEX1				2111
#define IDC_STATIC_BANDINDEX			2112 // Band Index GroupBox ID

#define IDC_STATIC_SCANT				2120 // Scan Type
#define IDC_STATIC_ETEVENT				2121 // ET Event
#define IDC_STATIC_ETNBFRAMES			2122 // ET Number of Frames
#define IDC_EDIT_NBFRAMES				2123
#define IDC_STATIC_ETCONF				2124 // ET GroupBox ID

#define IDC_STATIC_TRIGAC				2125 // Triggering AC
#define IDC_EDIT_TRIGAC					2126

#define IDC_STATIC_SCANDUR				2127 // Scan Duration
#define IDC_EDIT_SCANDUR				2128

#define IDC_STATIC_TXPWLVL				2129 // Tx Power Level
#define IDC_EDIT_TXPWLVL				2130

#define IDC_STATIC_MAXDWT				2131 // Max Dwell Time
#define IDC_EDIT_MAXDWT					2132

#define IDC_STATIC_MINDWT				2133 // Min Dwell Time
#define IDC_EDIT_MINDWT					2134

#define IDC_STATIC_PRN					2135 // Probe Request Number
#define IDC_EDIT_PRN					2136
#define IDC_STATIC_PRR					2137 // Probe Request Rate
#define IDC_STATIC_PR					2138 // Probe Request GroupBox ID

#define ID_CBBOX_SCANT					2150 // ScanType ComboBox
#define ID_CBBOX_ETEVT					2151 // ET Event ComboBox
#define ID_CBBOX_PRR					2152 // Probe Request Rate ComboBox


// --------------- Scan Modes : Global Configuration ---------------
#define IDC_STATIC_NSINTERVAL			2200 // Normal Scan Interval
#define IDC_EDIT_NSINTERVAL				2201
#define IDC_STATIC_DSINTERVAL			2202 // Deteriorating Scan Interval
#define IDC_EDIT_DSINTERVAL				2203
#define IDC_STATIC_SCANINTERVAL			2204 // Scan Interval ComboBox

#define IDC_STATIC_MAXTFAILURES			2205 // Max Track Failures
#define IDC_EDIT_MAXTFAILURES			2206
#define IDC_STATIC_BSSLISTSIZE			2207 // BSS List Size
#define IDC_EDIT_BSSLISTSIZE			2208
#define IDC_STATIC_BSSNBSTARTDISC		2209 // BSS Number to start discovery cycle
#define IDC_EDIT_BSSNBSTARTDISC			2210
#define IDC_STATIC_NBBANDS				2211 // Number of bands
#define IDC_EDIT_NBBANDS				2212
// ------------ End of Scan Modes : Global Configuration ------------


// ------------ Scan Modes : Misc & Channel Configuration -----------
// ------ Misc Configuration ------
#define IDC_STATIC_MCBANDINDEX			2300 // Band Index
#define ID_CBBOX_MCBANDINDEX			2301
#define IDC_STATIC_MCBAND				2302 // Band Description
#define IDC_EDIT_MCBAND					2303
#define IDC_STATIC_MCBSSTHRESHOLD		2304 // BSS Threshold
#define IDC_EDIT_MCBSSTHRESHOLD			2305
#define IDC_STATIC_MCCHANNBDISCCYCLE	2306 // Channel Number for discovery cycle
#define IDC_EDIT_MCCHANNBDISCCYCLE		2307
#define IDC_STATIC_MCNBCHANS			2308 // Number of channels
#define IDC_EDIT_MCNBCHANS				2309

// ----- Channel Configuration -----
#define IDC_STATIC_MCCHANINDEX			2310 // Channel Index
#define ID_CBBOX_MCCHANINDEX			2311
#define IDC_STATIC_MCCHANNB				2312 // Channel Number
#define IDC_EDIT_MCCHANNB				2313
#define IDC_STATIC_MCCHANCONF			2314
// ------- End of Scan Modes : Misc & Channel Configuration ---------


// ------------ Application Scan : Global Configuration -------------
#define IDC_STATIC_ASGSSID				2400 // SSID
#define IDC_EDIT_ASGSSID				2401
#define IDC_STATIC_ASGSCANTYPE			2402 // ScanType
#define ID_CBBOX_ASGSCANTYPE			2403 // ScanType ComboBox for Application Scan
#define IDC_STATIC_ASGBANDNB			2404 // Band Index
#define ID_CBBOX_ASGBANDNB				2405 // Band ComboBox for Application Scan

#define IDC_STATIC_ASGPRN				2406 // Probe Request Number
#define IDC_EDIT_ASGPRN					2407
#define IDC_STATIC_ASGPRR				2408 // Probe Request Rate
#define ID_CBBOX_ASGPRR					2409 // Probe Request Rate ComboBox for Application Scan
#define IDC_STATIC_ASGPREQ				2410 // Probe Request Group ID

#define IDC_STATIC_ASGNBCHAN			2411 // Number of Channels
#define IDC_EDIT_ASGNBCHAN				2412
#define IDC_STATIC_ASGTID				2413 // Triggering ID
#define IDC_EDIT_ASGTID					2414
// --------- End of Application Scan : Global Configuration ---------


// ------------ Application Scan : Channel Configuration ------------
#define IDC_STATIC_ASCCHANINDEX			2500 // Channel Index
#define ID_CBBOX_ASCCHANINDEX			2501 // Channel Index ComboBox
#define IDC_STATIC_ASCBSSID				2502 // BSSID
#define IDC_EDIT_ASCBSSID				2503
#define IDC_STATIC_ASCMAXDWELL			2504 // Max Dwell Time for Application Scan
#define IDC_EDIT_ASCMAXDWELL			2505
#define IDC_STATIC_ASCMINDWELL			2506 // Min Dwell Time for Application Scan
#define IDC_EDIT_ASCMINDWELL			2507

#define IDC_STATIC_ASCETCOND			2508 // earlyTermination Condition / Event
#define ID_CBBOX_ASCETCOND				2509
#define IDC_STATIC_ASCETFRAMENB			2510 // ET Frame Number
#define IDC_EDIT_ASCETFRAMENB			2511
#define IDC_STATIC_ASCETEVENT			2512 // ET Event Group ID

#define IDC_STATIC_ASCTXPOWERLVL		2513 // Tx Power Level
#define IDC_EDIT_ASCTXPOWERLVL			2514
#define IDC_STATIC_ASCCHANNB			2515 // Channel Number
#define IDC_EDIT_ASCCHANNB				2516
// --------- End of Application Scan : Channel Configuration ---------

// ---------------------- Roaming Configuration ----------------------
// ----- Global Roaming Settings -----
#define IDC_STATIC_ROAMINGLPF				2600 // Low-pass Filter
#define IDC_EDIT_ROAMINGLPF					2601
#define IDC_STATIC_ROAMINGQT				2602 // Quality Threshold
#define IDC_EDIT_ROAMINGQT					2603
#define IDC_STATIC_ROAMINGGRC				2604 // Global Roaming Settings Group ID

// ------- Thresholds Settings -------
#define IDC_STATIC_ROAMINGDRT				2605 // Data Retry Threshold
#define IDC_EDIT_ROAMINGDRT					2606
#define IDC_STATIC_ROAMINGBSSLOSS			2607 // Number of Expected Tbtt For BSS Loss
#define IDC_EDIT_ROAMINGBSSLOSS				2608
#define IDC_STATIC_ROAMINGTXRATE			2609 // Tx Rate Threshold
#define IDC_EDIT_ROAMINGTXRATE				2610
#define IDC_STATIC_ROAMINGLOWRSSI			2611 // Low RSSI Threshold
#define IDC_EDIT_ROAMINGLOWRSSI				2612
#define IDC_STATIC_ROAMINGLOWSNR			2613 // Low SNR Threshold
#define IDC_EDIT_ROAMINGLOWSNR				2614

#define IDC_STATIC_ROAMINGLQSCAN			2615 // Low Quality For Background Scan Condition
#define IDC_EDIT_ROAMINGLQSCAN				2616
#define IDC_STATIC_ROAMINGNQSCAN			2617 // Normal Quality For Background Scan Condition
#define IDC_EDIT_ROAMINGNQSCAN				2618
#define IDC_STATIC_ROAMINGQUALITY			2619 // Background Scan Condition Group ID

#define IDC_STATIC_ROAMINGTHRESHOLDCONF		2620 // Thresholds Settings Group ID
// ------------------ End of Roaming Configuration -------------------


// Next default values for new objects
// 
#ifdef APSTUDIO_INVOKED
#ifndef APSTUDIO_READONLY_SYMBOLS
#define _APS_NO_MFC                     1
#define _APS_NEXT_RESOURCE_VALUE        114
#define _APS_NEXT_COMMAND_VALUE         40007
#define _APS_NEXT_CONTROL_VALUE         1022
#define _APS_NEXT_SYMED_VALUE           101
#endif
#endif
