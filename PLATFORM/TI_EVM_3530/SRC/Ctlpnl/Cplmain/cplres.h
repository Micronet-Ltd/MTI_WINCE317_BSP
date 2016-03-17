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
/**
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Abstract: RCIDs of resources used in this DLL
**/

#define RESBASE                     0x8000        // must be a round-number in hex

// Common dialogs
#define IDD_CHECK_PASSWD            50
// Controls for common dialogs
#define IDC_EDIT_OLD_PASS           60
#define IDC_PWDMSG                  61

// Network Dialogs--rebased from 100, 101 to 6666,6667
#define IDD_NETCPL_ADAPTERS         6666

// Comm Dialogs--rebased from 120, 121,122 to 6670,6671, 6672
#define IDD_COMPUTER                6670
#define IDD_RAS                     6671
#define IDD_CHANGE_CONN             6672

// Keybd Dialogs
#define IDD_KEYBD                   130

// Passwd dialogs
#define IDD_PASSWD                  140

// Owner dialogs
#define IDD_PROFILE                 150
#define IDD_NOTES                   151
#define IDD_NETIDENT                152

// Power dialogs
#define IDD_SLEEP                   160
#define IDD_BATTERY                 161
#define IDD_POWER_DEVICE            162
#define IDD_POWER_TIMEOUTS          163

// System dialogs
#define IDD_SYSTEM                  170
#define IDD_MEMORY                  171
#define IDD_COPYRIGHTS              172

// Screen dialogs
#define IDD_BACKGROUND              180
#define IDD_BACKLIGHT               181
#define IDD_COLSCHEME               182
#define IDD_COLSCHEMESAVE           183

// Stylus dialogs
#define IDD_CALIBRATE               190

// Mouse/stylus dialogs
#define IDD_DBLTAP                  200

// Sound dialogs
#define IDD_VOLUME                  210
#define IDD_SNDSCHEME               211
#define IDD_SNDSCHEMESAVE           212

// SIP dialogs
#define IDD_INPUTMETHOD             220

// Remove-Programs dialogs
#define IDD_REMOVE                  230
#define IDD_UNLOADERRORDLG          231

// Accessibility dialogs
#define IDD_ACC_KEYBD               240
#define IDD_ACC_SOUND               241
#define IDD_ACC_DISP                242
#define IDD_ACC_MOUSE               243
#define IDD_ACC_GEN                 244
// Accessibility sub-dialogs
#define IDD_ACCD_STICKYSETTINGS     245
#define IDD_ACCD_TOGGLESETTINGS     246
#define IDD_ACCD_SOUNDSETTINGS      247
#define IDD_ACCD_DISPSETTINGS       248
#define IDD_ACCD_MOUSESETTINGS      249
#define IDD_ACCD_SCHEMESETTINGS     250


// Date/Time Dialogs
#define IDD_DATETIME                260 

// Certificate Dialogs
#define IDD_CERTROOTS_TAB           270
#define IDD_CERTPROP_GENERAL        271
#define IDD_ENTERKEYPASSWORD        272
#define IDD_IMPORTCERT              273
#define IDD_CERTIFICATE_PROMPT		274

// Network Dialog controls
#define IDC_NETCPL0_ADAPTERLIST     400
#define IDC_NETCPL0_PROPERTIES      401
#define IDC_NETCPL1_USERNAME        402
#define IDC_NETCPL1_DOMAIN          403
#define IDC_NETCPL1_PASSWORD        404

// Comm dialog controls
#define IDC_CONNECTION              421
#define IDC_AUTO                    424
#define IDC_STATIC_DIRECT           425
#define IDC_CHANGE                  426
#define IDC_CB                      427
#define IDC_STATIC1                 428
#define IDC_CNCT_BOX                429

// Keybd dialog controls
#define IDC_CHK_BOX                 440
#define IDC_STATIC4                 441
#define IDC_STATIC_L                442
#define IDC_BTN_LONG                443
//#define IDC_STATIC1               444
#define IDC_TRACKBAR_DLY            445
#define IDC_STATIC_H                446
#define IDC_BTN_SHORT               447
#define IDC_STATIC5                 448
#define IDC_STATIC_S                449
#define IDC_BTN_SLOW                450
#define IDC_STATIC2                 451
#define IDC_TRACKBAR_RPT            452
#define IDC_STATIC_F                453
#define IDC_BTN_FAST                454
#define IDC_STATIC3                 455
#define IDC_EDIT                    456

//Passwd dialog controls
#define IDC_EDIT_NEW_PASS           460
#define IDC_EDIT_CONF_PASS          461
#define IDC_CHK_PASSWRD             462
#define IDC_BOX1                    463
#define IDC_BOX2                    464
#define IDC_CHK_PASSWRDSCREENSAVER  465

// Owner dialog controls
#define IDC_EDIT_NAME1              480
#define IDC_EDIT_COMPANY            481
#define IDC_EDIT_ADD                482
#define IDC_EDIT_WAC                483
#define IDC_EDIT_WPHONE             484
#define IDC_EDIT_HAC                485
#define IDC_EDIT_HPHONE             486
#define IDC_EDIT_NOTES              487
#define IDC_CHK_NOTES               488
#define IDC_CHK_ID                  489

// Power dlg controls
#define IDC_COMBO1                  500
#define IDC_CHECK2                  501
#define IDC_COMBO2                  502
#define IDC_BAT1                    503
#define IDC_BAT2                    504
#define IDC_BAT3                    505
#define IDC_STATIC_MAIN             506
#define IDC_STATIC_CHARGE           507
#define IDC_STATIC_EXTERNAL         508
#define IDC_STATIC_BACK             509
#define IDC_STATIC_GOOD             510
#define IDC_STATIC_LOW              511
#define IDC_STATIC_VLOW             512
#define IDC_STATIC_GOOD1            513
#define IDC_STATIC_LOW1             514
#define IDC_STATIC_VLOW1            515
#define IDC_BAT1_STATIC             516
#define IDC_BAT2_STATIC             517
#define IDC_BAT3_STATIC             518
#define IDC_STATIC_PWR              519
#define IDC_STATIC_MBS              520

#define IDC_LIST_POWER_DEVICE       521
#define IDC_COMBO_TO_USER_IDLE      522
#define IDC_COMBO_TO_SYSTEM_IDLE    523
#define IDC_COMBO_TO_SUSPEND        524
#define IDC_COMBO_POWER_SOURCE      525
#define IDC_TURNOFF      526

// System dlg controls
#define IDC_IDENTIFY                537
#define IDC_EDIT_NAME               538
#define IDC_EDIT_DESCR              539
#define IDC_STATIC_STORAGE          540
#define IDC_STATIC_PROGRAM          541
#define IDC_TRACKBAR                542
#define IDC_STATIC_LEFT             543
#define IDC_STATIC_RIGHT            544
#define IDC_STATIC_ALLOC_LEFT       545
#define IDC_STATIC_ALLOC_RIGHT      546
#define IDC_STATIC_IN_USE_LEFT      547
#define IDC_STATIC_IN_USE_RIGHT     548
#define IDC_IN_USE_LEFT             549
#define IDC_IN_USE_RIGHT            550
#define IDC_STATIC_OS               551
#define IDC_STATIC_VER_SYS          552
#define IDC_STATIC_PROCESSOR        553
#define IDC_STATIC_PCMCIA_ONE       554
#define IDC_STATIC_PCMCIA_TWO       555
#define IDC_STATIC_RAM              556
#define IDC_STATIC_USER             557
#define IDC_STATIC_USER_COMPANY     558
#define IDC_STATIC_COMPUTER         559
#define IDC_STATIC_SYSTEM           560
#define IDC_STATIC_MEMORY1          561
#define IDC_STATIC_MEMORY2          562
#define IDC_EXPANSION_CMB           563        //Expansion Slots
#define IDC_COPYR                   564

// Screen dialog controls
#define IDC_BOX                     580
#define IDC_WALLLIST                581
#define IDC_BROWSEWALL              582
#define IDC_TILEIMAGE               584
#define IDC_BBATTERY_TIME           585
#define IDC_BEXTERNAL_TIME          586
#define IDC_BADVANCED               587
#define IDC_BBATTERY                588
#define IDC_BEXTERNAL               589
#define IDC_SELECTION_STATIC        590
#define IDC_PREVIEW                 591
#define IDC_SCHEME                  592
#define IDC_SAVEAS                  593
#define IDC_APPLY                   594
#define IDC_ITEM                    595
#define IDC_COLOR                   596
#define IDC_SAVESCHEME              597
#define IDC_PREVIEWDUMMY            598

// Stylus dialog controls
#define IDC_CALIB_TEXT              600
#define IDC_BUTTON_CALIB            601

// Mouse/stylus dialog controls
#define IDC_DBLTAP_SET              620
//#define IDC_STATIC_LINE2          621
#define IDC_DBLTAP_TEST             622
#define IDC_TESTDBLCLK              623
#define IDC_CLAPPER1                624
#define IDC_CLAPPER2                625
#define IDC_GETDBLCLK1              626
#define IDC_GETDBLCLK2              627

// Volume dlg controls
//#define IDC_STATIC_L              640
#define IDC_HIGH                    641
//#define IDC_TRACKBAR              642
#define IDC_STATIC_Q                643
#define IDC_LOW                     644
//#define IDC_STATIC1               645
#define IDC_CHK_EVTS                646
#define IDC_CHK_APPS                647
#define IDC_CHK_NOT                 648
#define IDC_ENABLECLICKTAP          649
#define IDC_CHK_KEY                 650
#define IDC_RADIO1                  651
#define IDC_RADIO2                  652
#define IDC_CHK_TAP                 653
#define IDC_RADIO3                  654
#define IDC_RADIO4                  655

// SndScheme dialog controls
#define IDC_STATIC_EVENTNAME        660
#define IDC_EVENT_TREE              661
#define IDC_STATIC_EVENTSOUND       662
#define IDC_COMBO_SOUND             663
#define IDC_BROWSE                  664
#define IDC_TEST                    665
#define IDC_STOP                    666
#define IDC_COMBO_SCHEME            667
#define IDC_SAVE                    668
#define IDC_DELETE                  669
#define IDC_SCHEMENAME              670

// SIP dialog controls
#define IDC_COMBOIM                 680
#define IDC_BUTTONOPTIONS           681
#define IDC_CHECKALLOWCHANGE        682
#define IDC_ICONIM                  683

// Remove-programs dialog controls
#define IDC_STATIC_PICT             690
#define IDC_REMOVE                  691
#define IDC_LISTBOX                 692
#define IDC_ABORTTEXT               693
#define IDC_FAILTEXT                694
#define IDC_PROG_LIST               695

// Accessibility Options dialog controls
#define IDC_ACC_USESTICKY           700
#define IDC_ACC_STICKYSETTINGS      701
#define IDC_ACC_STICKYGROUP         702
#define IDC_ACC_USETOGGLE           703
#define IDC_ACC_TOGGLESETTINGS      704
#define IDC_ACC_TOGGLEGROUP         705
#define IDC_ACC_KBDHELP             706
#define IDC_ACC_USESNDSENTRY        707
#define IDC_ACC_SNDSETTINGS         708
#define IDC_ACC_USESHOWSND          709
#define IDC_ACC_SHOWGROUP           710
#define IDC_ACC_USEHIGHCONT         711
#define IDC_ACC_DISPSETTINGS        712
#define IDC_ACC_DISPGROUP           713
#define IDC_ACC_USEMOUSEKEYS        714
#define IDC_ACC_MSETTINGS           715
#define IDC_ACC_MOUSEGROUP          716
#define IDC_ACC_IDLECHK             717
#define IDC_ACC_GENCOMBO            718
#define IDC_ACC_WARNSNDCHK          719        
#define IDC_ACC_SOUNDONOFFCHK       720
#define IDC_ACC_GENGROUP1           721
#define IDC_ACC_GENGROUP2           722
#define IDC_ACC_SCHEMESETTINGS      723



// Accessibility Options dialog sub-dialog controls
#define IDC_ACCD_TOGGLESHORT        724
#define IDC_ACCD_STICKYSHORT        725
#define IDC_ACCD_STKLOCK            726
#define IDC_ACCD_STKSOUNDMOD        727
#define IDC_ACCD_SNDSETWARNING      728
#define IDC_ACCD_MOUSESHORT         729
#define IDC_ACCD_MTOPSPEED          730
#define IDC_ACCD_MACCEL             731
#define IDC_ACCD_MHOLDKEYS          732
#define IDC_ACCD_NUMOFF             733
#define IDC_ACCD_NUMON              734
#define IDC_ACCD_HIGHSHORT          735
#define IDC_ACCD_HIGHGROUP          736
#define IDC_ACCD_HCWHITEBLACK       737
#define IDC_ACCD_HCBLACKWHITE       738
#define IDC_ACCD_HCCUSTOM           739        
#define IDC_ACCD_SCHEME             740
#define IDC_ACCD_SSCOMBO            741        
#define IDC_ACCD_MFAST              742        
#define IDC_ACCD_MSLOW              743

// Date/Time Controls               
#define IDC_TIME_TIME               750
#define IDC_TIME_DATE               751
#define IDC_TIME_ZONE               752
#define IDC_TIME_APPLY              754
#define IDC_TIME_DLST               755

// Certifcate controls
#define IDC_CERTROOTS_LIST          760
#define IDC_CERTROOTS_REMOVE        761
#define IDC_CERTROOTS_VIEW          762
#define IDC_CERTROOTS_IMPORT        763
#define IDC_CERTSTORE_SELECT        764
#define IDC_CERTCPL_STATIC_STORE    765

// Certificate import dialog controls

#define IDC_COMBO_SMARTCARD         770
#define IDC_RADIO_FILECERT          771
#define IDC_RADIO_CARDCERT          772
#define IDC_EDIT_FRIENDLYNAME       773
#define IDC_STATIC_CARD             774
#define IDC_STATIC_CARDNAME         775
#define IDC_STATIC_READER           776
#define IDC_STATIC_IMPORTCERT       777

/*
#define IDC_SUBJECT_EDIT               770
#define IDC_ISSUER_EDIT                771
#define IDC_GOODFOR_EDIT               772
#define IDC_CERT_ISSUER_WARNING_EDIT   773
#define IDC_VALID_FROM                 774
#define IDC_VALID_UNTIL                775
*/

#define IDC_PASSWORD0                  780

// Device Management dialog controls

#define IDC_OPLIST                     800
#define IDC_ADD                        801
#define IDC_VIEWPACKAGE                802
#define IDC_ACTIONLIST                 803
#define IDC_SERVER                     804
#define IDC_RUN                        805
#define IDC_INFOLIST                   806
#define IDC_PKGLIST                    807

// Certificate add dialog controls
#define IDC_CertificateText				900
#define IDC_CertificateMessage			901

//----- NOTE: Every CPL's strings must start on a multiple of 16-----------

// Network Strings
#define IDS_NETCPL_ADAPTORS             (RESBASE+0x0)
#define IDS_NETCPL_NAME                 (RESBASE+0x1)
#define IDS_NETCPL_DESC                 (RESBASE+0x2)
#define IDS_NETCPL_TITLE                (RESBASE+0x3)
#define IDS_NETCPL_ADAPTERCHG           (RESBASE+0x4)

// Comm Strings
#define IDS_COMM_NAME                   (RESBASE+0x10)
#define IDS_COMM_DESC                   (RESBASE+0x11)
#define IDS_COMM_TITLE                  (RESBASE+0x12)
#define IDS_COMM_CONNECTTAB             (RESBASE+0x16)
#define IDS_COMM_CHG_TOOLTIP            (RESBASE+0x17)
#define IDS_COMM_CONNECTION             (RESBASE+0x18)
#define IDS_LOC_DESKTOPCONNECT          (RESBASE+0x19)

// dialing strings
#define IDS_DIALING_NAME                (RESBASE+0x90)
#define IDS_DIALING_DESC                (RESBASE+0x91)
#define IDS_DIALING_TITLE               (RESBASE+0x92)

// KEYBD strings
#define IDS_KEYBD_NAME                  (RESBASE+0x20)
#define IDS_KEYBD_DESC                  (RESBASE+0x21)
#define IDS_KEYBD_TITLE                 (RESBASE+0x22)
#define IDS_KEYBD_TAB                   (RESBASE+0x23)

// Password strings
#define IDS_PASSWD_NAME                 (RESBASE+0x30)
#define IDS_PASSWD_DESC                 (RESBASE+0x31)
#define IDS_PASSWD_TITLE                (RESBASE+0x32)
#define IDS_PASSWD_TAB                  (RESBASE+0x33)
#define IDS_PASSWDERR_TITLE             (RESBASE+0x34)
#define IDS_PASSWDERR_NOTMATCH          (RESBASE+0x35)

// Owner strings
#define IDS_PROFILE_NAME                (RESBASE+0x40)
#define IDS_PROFILE_DESC                (RESBASE+0x41)
#define IDS_PROFILE_TITLE               (RESBASE+0x42)
#define IDS_PROFILE_IDTAB               (RESBASE+0x43)
#define IDS_PROFILE_NOTESTAB            (RESBASE+0x44)
#define IDS_PROFILE_NETIDENT            (RESBASE+0x45)

// Power strings
#define IDS_POWER_NAME                  (RESBASE+0x50)
#define IDS_POWER_DESC                  (RESBASE+0x51)
#define IDS_POWER_TITLE                 (RESBASE+0x52)
#define IDS_POWER_BATTERYTAB            (RESBASE+0x53)
#define IDS_POWER_SLEEPTAB              (RESBASE+0x54)
#define IDS_POWER_MINUTE                (RESBASE+0x55)
#define IDS_POWER_MINUTES               (RESBASE+0x56)
#define IDS_POWER_VERYLOW               (RESBASE+0x57)
#define IDS_POWER_NOTAVAIL              (RESBASE+0x58)
#define IDS_POWER_HOUR                  (RESBASE+0x59)
#define IDS_POWER_HOURS                 (RESBASE+0x5a)

#define IDS_POWER_DEVICETAB             (RESBASE+0x3c0)
#define IDS_POWER_DEVICE                (RESBASE+0x3c1)
#define IDS_POWER_STATUS                (RESBASE+0x3c2)
#define IDS_POWER_TIMEOUTSTAB           (RESBASE+0x3c3)
#define IDS_POWER_NEVER                 (RESBASE+0x3c4)
#define IDS_POWER_AFTER                 (RESBASE+0x3c5)
#define IDS_POWER_BATTERY               (RESBASE+0x3c6)
#define IDS_POWER_AC                    (RESBASE+0x3c7)
#define IDS_POWER_HIGH_D0               (RESBASE+0x3c8)
#define IDS_POWER_MEDIUM_D1             (RESBASE+0x3c9)
#define IDS_POWER_LOW_D2                (RESBASE+0x3ca)
#define IDS_POWER_OFF_D3                (RESBASE+0x3cb)
#define IDS_POWER_OFF_D4                (RESBASE+0x3cc)
// Power has to RESBASE+0x3ff

// System strings
#define IDS_SYSTEM_NAME                 (RESBASE+0x60)
#define IDS_SYSTEM_DESC                 (RESBASE+0x61)
#define IDS_SYSTEM_TITLE                (RESBASE+0x62)
#define IDS_SYSTEM_SYSTAB               (RESBASE+0x63)
#define IDS_SYSTEM_MEMTAB               (RESBASE+0x64)
#define IDS_SYSTEM_RAMSIZE              (RESBASE+0x65)
#define IDS_SYSTEM_MEMKBYTE             (RESBASE+0x66)
#define IDS_SYSTEM_MEMFAIL              (RESBASE+0x67)
#define IDS_SYSTEM_MEMERRTITLE          (RESBASE+0x68)
#define IDS_SYSTEM_VER_WITH_BUILD       (RESBASE+0x69)
#define IDS_SYSTEM_VER_WITHOUT_BUILD    (RESBASE+0x6a)
#define IDS_CPU_INTEL_386               (RESBASE+0x6b)
#define IDS_CPU_INTEL_486               (RESBASE+0x6c)
#define IDS_CPU_INTEL_PENTIUM           (RESBASE+0x6d)
#define IDS_CPU_MIPS_R4000              (RESBASE+0x6e)
#define IDS_CPU_HITACHI_SH3             (RESBASE+0x6f)
#define IDS_CPU_HITACHI_SH4             (RESBASE+0x70)
#define IDS_CPU_ALPHA_21064             (RESBASE+0x71)
#define IDS_CPU_PPC_601                 (RESBASE+0x72)
#define IDS_CPU_PPC_603                 (RESBASE+0x73)
#define IDS_CPU_PPC_604                 (RESBASE+0x74)
#define IDS_CPU_PPC_620                 (RESBASE+0x75)
#define IDS_CPU_PPC_821                 (RESBASE+0x76)
#define IDS_CPU_STRONGARM               (RESBASE+0x79)
#define IDS_CPU_ARM720                  (RESBASE+0x7a)
#define IDS_CPU_ARM820                  (RESBASE+0x7b)
#define IDS_CPU_ARM920                  (RESBASE+0x7c)
#define IDS_CPU_UNKNOWN                 (RESBASE+0x7d)
#define IDS_CPU_PPC_403                 (RESBASE+0x7e)
#define IDS_CPU_INTEL_PENTIUMII         (RESBASE+0x7f)
#define IDS_CPU_HITACHI_SH3DSP          (RESBASE+0x80)
#define IDS_SYSTEM_IDTAB                (RESBASE+0x81)
#define IDS_SYSTEM_IDENTIFY             (RESBASE+0x82)
#define IDS_SYSTEM_ID_NONET             (RESBASE+0x83)
#define IDS_SYSTEM_COPYRTAB             (RESBASE+0x84)



// Screen strings
#define IDS_SCREEN_NAME                 (RESBASE+0x100)
#define IDS_SCREEN_DESC                 (RESBASE+0x101)
#define IDS_SCREEN_TITLE                (RESBASE+0x102)
#define IDS_SCREEN_BACKGNDTAB           (RESBASE+0x103)
#define IDS_SCREEN_BACKLIGHTTAB         (RESBASE+0x104)
#define IDS_SCREEN_HPCNAME              (RESBASE+0x105)
#define IDS_SCREEN_FILTER               (RESBASE+0x108)
#define IDS_SCREEN_ERRTITLE             (RESBASE+0x109)
#define IDS_SCREEN_IMAGE_ERROR          (RESBASE+0x10a)
#define IDS_SCREEN_MEMORY_ERROR         (RESBASE+0x10b)
#define IDS_15SECS                      (RESBASE+0x10c)
#define IDS_30SECS                      (RESBASE+0x10d)
#define IDS_1MIN                        (RESBASE+0x10e)
#define IDS_2MIN                        (RESBASE+0x10f)
#define IDS_5MIN                        (RESBASE+0x110)
#define IDS_10MIN                       (RESBASE+0x111)
#define IDS_15MIN                       (RESBASE+0x112)
#define IDS_30MIN                       (RESBASE+0x113)

// Stylus strings
#define IDS_STYLUS_NAME                 (RESBASE+0x130)
#define IDS_STYLUS_DESC                 (RESBASE+0x131)
#define IDS_STYLUS_TITLE                (RESBASE+0x132)
#define IDS_STYLUS_CALIBTAB             (RESBASE+0x133)
#define IDS_STYLUS_DBLTAPTAB            (RESBASE+0x134)
#define IDS_DBLTAP_SET                  (RESBASE+0x135)
#define IDS_DBLTAP_TEST                 (RESBASE+0x136)
#define IDS_CALIBRATE_TEXT              (RESBASE+0x137)

// Mouse strings
#define IDS_MOUSE_NAME                  (RESBASE+0x150)
#define IDS_MOUSE_DESC                  (RESBASE+0x151)
#define IDS_MOUSE_TITLE                 (RESBASE+0x152)
#define IDS_MOUSE_DBLCLICKTAB           (RESBASE+0x153)
#define IDS_DBLCLICK_SET                (RESBASE+0x154)
#define IDS_DBLCLICK_TEST               (RESBASE+0x155)

// Sound strings
#define IDS_SOUND_NAME                  (RESBASE+0x160)
#define IDS_SOUND_DESC                  (RESBASE+0x161)
#define IDS_SOUND_TITLE                 (RESBASE+0x162)
#define IDS_LOC_DEFAULTSOUNDS           (RESBASE+0x163)
#define IDS_LOC_ALLSOUNDS               (RESBASE+0x164)
#define IDS_LOC_NOSOUNDS                (RESBASE+0x167)

// Sound:Volume strings
#define IDS_VOLUMETAB                   (RESBASE+0x165)
#define IDS_HARDWAREBUTTONS             (RESBASE+0x166)

// Sound:Scheme strings
#define IDS_SNDSCHEMETAB                (RESBASE+0x170)
#define IDS_SAVESCHEME                  (RESBASE+0x171)
#define IDS_SCHEMENOTSAVED              (RESBASE+0x172)
#define IDS_NOOVERWRITEDEFAULT          (RESBASE+0x173)
#define IDS_OVERWRITESCHEME             (RESBASE+0x174)
#define IDS_CONFIRMREMOVE               (RESBASE+0x175)
#define IDS_NOSNDFILE                   (RESBASE+0x176)
#define IDS_ERRORPLAY1                  (RESBASE+0x177)
#define IDS_ERRORPLAY2                  (RESBASE+0x178)
#define IDS_SOUND_ERROR                 (RESBASE+0x179)
#define IDS_WAVFILES                    (RESBASE+0x180)
#define IDS_REMOVESCHEME                (RESBASE+0x181)
#define IDS_CHANGESCHEME                (RESBASE+0x182)
#define IDS_SND_ASTERISK                (RESBASE+0x183)
#define IDS_SND_CLOSEAPPS               (RESBASE+0x184)
#define IDS_SND_CRITICAL                (RESBASE+0x185)
#define IDS_SND_DEFAULTSND              (RESBASE+0x186)
#define IDS_SND_EMPTYRECYC              (RESBASE+0x187)
#define IDS_SND_EXCLAMATION             (RESBASE+0x188)
#define IDS_SND_IRDABEGIN               (RESBASE+0x189)
#define IDS_SND_IRDAEND                 (RESBASE+0x190)
#define IDS_SND_IRDAINTERRUPT           (RESBASE+0x191)
#define IDS_SND_MENUPOPUP               (RESBASE+0x192)
#define IDS_SND_MENUSELECT              (RESBASE+0x193)
#define IDS_SND_OPENAPPS                (RESBASE+0x194)
#define IDS_SND_QUESTION                (RESBASE+0x195)
#define IDS_SND_REMNETSTART             (RESBASE+0x196)
#define IDS_SND_REMNETEND               (RESBASE+0x197)
#define IDS_SND_REMNETINT               (RESBASE+0x198)
#define IDS_SND_STARTUP                 (RESBASE+0x199)
#define IDS_SND_MAXWIN                  (RESBASE+0x200)
#define IDS_SND_MINWIN                  (RESBASE+0x201)
#define IDS_SND_RECSTART                (RESBASE+0x202)
#define IDS_SND_RECEND                  (RESBASE+0x203)
#define IDS_CURRENTSETTINGS             (RESBASE+0x204)
#define IDS_WINDOWSCE                   (RESBASE+0x205)

// SIP strings
#define IDS_SIP_NAME                    (RESBASE+0x210)
#define IDS_SIP_DESC                    (RESBASE+0x211)
#define IDS_SIP_TITLE                   (RESBASE+0x212)
#define IDS_SIP_TABTITLE                (RESBASE+0x213)
#define IDS_CANT_LOAD_IM                (RESBASE+0x214)
#define IDS_NO_IM_OPTIONS               (RESBASE+0x215)

// Remove-programs strings
#define IDS_REMOVE_NAME                 (RESBASE+0x250)
#define IDS_REMOVE_DESC                 (RESBASE+0x251)
//#define IDS_REMOVE_TITLE              (RESBASE+0x252)
#define IDS_REMOVE_TITLE                IDS_REMOVE_NAME
#define IDS_REMOVECONFIRM               (RESBASE+0x253)
#define IDS_REMOVETITLEERR              (RESBASE+0x254)
#define IDS_REMOVEERR_REG               (RESBASE+0x255)
#define IDS_REMOVEERR_SCRIPT            (RESBASE+0x256)
#define IDS_REMOVEERRDEFAULT            (RESBASE+0x257)
#define IDS_REMOVEERRCOMMON             (RESBASE+0x258)

// Screen--color schemes tab
#define IDS_SCREEN_COLSCHEMETAB         (RESBASE+0x270)
#define IDS_NORMAL                      (RESBASE+0x271)
#define IDS_DISABLED                    (RESBASE+0x272)
#define IDS_SELECTED                    (RESBASE+0x273)
#define IDS_BUTTON                      (RESBASE+0x274)
#define IDS_ACTIVEWINDOW                (RESBASE+0x275)
#define IDS_INACTIVEWINDOW              (RESBASE+0x276)
#define IDS_WINDOWTEXT                  (RESBASE+0x277)
#define IDS_MESSAGETEXT                 (RESBASE+0x278)
#define IDS_BLANKNAME                   (RESBASE+0x279)
#define IDS_NOSCHEME2DEL                (RESBASE+0x27a)
#define IDS_BLESSEDSCHEME               (RESBASE+0x27b)
#define IDS_READONLYSCHEME              (RESBASE+0x27c)
#define IDS_WINDOWSDEFAULT              (RESBASE+0x27d)

// Color scheme names
// Need to update registry if these values are changed
#define IDS_LOC_SCHEME_STANDARD         (RESBASE+0x27e)
#define IDS_LOC_SCHEME_WHEAT            (RESBASE+0x27f)
#define IDS_LOC_SCHEME_MARINE           (RESBASE+0x280)
#define IDS_LOC_SCHEME_ROSE             (RESBASE+0x281)
#define IDS_LOC_SCHEME_BRICK            (RESBASE+0x282)
#define IDS_LOC_SCHEME_DESERT           (RESBASE+0x283)
#define IDS_LOC_SCHEME_EGGPLANT         (RESBASE+0x284)
#define IDS_LOC_SCHEME_HCBLACK          (RESBASE+0x285)
#define IDS_LOC_SCHEME_HCWHITE          (RESBASE+0x286)
#define IDS_LOC_SCHEME_LILAC            (RESBASE+0x287)
#define IDS_LOC_SCHEME_MAPLE            (RESBASE+0x288)
#define IDS_LOC_SCHEME_SPRUCE           (RESBASE+0x289)
#define IDS_LOC_SCHEME_STORM            (RESBASE+0x28a)

// These must be contiguous and in the same order as colors in winuser.h
#define IDS_CLR_SCROLLBAR               (RESBASE+0x300)
#define IDS_CLR_BACKGROUND              (RESBASE+0x301)
#define IDS_CLR_ACTIVECAPTION           (RESBASE+0x302)
#define IDS_CLR_INACTIVECAPTION         (RESBASE+0x303)
#define IDS_CLR_MENU                    (RESBASE+0x304)
#define IDS_CLR_WINDOW                  (RESBASE+0x305)
#define IDS_CLR_WINDOWFRAME             (RESBASE+0x306)
#define IDS_CLR_MENUTEXT                (RESBASE+0x307)
#define IDS_CLR_WINDOWTEXT              (RESBASE+0x308)
#define IDS_CLR_CAPTIONTEXT             (RESBASE+0x309)
#define IDS_CLR_ACTIVEBORDER            (RESBASE+0x30a)
#define IDS_CLR_INACTIVEBORDER          (RESBASE+0x30b)
#define IDS_CLR_APPWORKSPACE            (RESBASE+0x30c)
#define IDS_CLR_HIGHLIGHT               (RESBASE+0x30d)
#define IDS_CLR_HIGHLIGHTTEXT           (RESBASE+0x30e)
#define IDS_CLR_BTNFACE                 (RESBASE+0x30f)
#define IDS_CLR_BTNSHADOW               (RESBASE+0x310)
#define IDS_CLR_GRAYTEXT                (RESBASE+0x311)
#define IDS_CLR_BTNTEXT                 (RESBASE+0x312)
#define IDS_CLR_INACTIVECAPTIONTEXT     (RESBASE+0x313)
#define IDS_CLR_BTNHIGHLIGHT            (RESBASE+0x314)
#define IDS_CLR_3DDKSHADOW              (RESBASE+0x315)
#define IDS_CLR_3DLIGHT                 (RESBASE+0x316)
#define IDS_CLR_INFOTEXT                (RESBASE+0x317)
#define IDS_CLR_INFOBK                  (RESBASE+0x318)
#define IDS_CLR_STATIC                  (RESBASE+0x319)
#define IDS_CLR_STATICTEXT              (RESBASE+0x31a)
#define IDS_CLR_FIRST                   IDS_CLR_SCROLLBAR
#define IDS_CLR_LAST                    IDS_CLR_STATICTEXT

// Date/Time Strings
#define IDS_DATETIME_NAME               (RESBASE+0x340)
#define IDS_DATETIME_DESC               (RESBASE+0x341)
#define IDS_DATETIME_TITLE              (RESBASE+0x342)
#define IDS_DATETIME_TAB                IDS_DATETIME_NAME

// Certificate Strings
#define IDS_CERTCPL_NAME                (RESBASE+0x350)
#define IDS_CERTCPL_DESC                (RESBASE+0x351)
#define IDS_CERTCPL_TITLE               (RESBASE+0x352)
#define IDS_CERTSTORES_TAB              (RESBASE+0x353)
#define IDS_CERTCPL_CERTIFICATE         (RESBASE+0x354)
#define IDS_CERT_SELECT                 (RESBASE+0x355)
#define IDS_CERTCPL_FILTER              (RESBASE+0x356)
#define IDS_CERTCPL_ROOTSTORE           (RESBASE+0x357)
#define IDS_CERTCPL_MYSTORE             (RESBASE+0x358)
#define IDS_CERTCPL_ROOT_DESCRIPTION    (RESBASE+0x359)
#define IDS_CERTCPL_MY_DESCRIPTION      (RESBASE+0x35a)
#define IDS_CERTCPL_CASTORE             (RESBASE+0x35b)
#define IDS_CERTCPL_CA_DESCRIPTION      (RESBASE+0x35c)
#define IDS_CARD_ABSENT                 (RESBASE+0x35d)
#define IDS_CARD_UNKNOWN                (RESBASE+0x35e)

// resources strings used by the Root Store warning message box
#define IDS_ROOT_MSG_BOX_TITLE          (RESBASE+0x360)
#define IDS_ROOT_MSG_BOX_SUBJECT        (RESBASE+0x361)
#define IDS_ROOT_MSG_BOX_ISSUER         (RESBASE+0x362)
#define IDS_ROOT_MSG_BOX_SELF_ISSUED    (RESBASE+0x363)
#define IDS_ROOT_MSG_BOX_SERIAL_NUMBER  (RESBASE+0x364)
#define IDS_ROOT_MSG_BOX_TIME_VALIDITY  (RESBASE+0x365)
#define IDS_ROOT_MSG_BOX_ADD_ACTION     (RESBASE+0x366)
#define IDS_ROOT_MSG_BOX_DELETE_ACTION  (RESBASE+0x367)

// Accessibility Options strings
#define IDS_ACCESS_NAME                 (RESBASE+0x370)        
#define IDS_ACCESS_DESC                 (RESBASE+0x371)        
#define IDS_ACCESS_TITLE                IDS_ACCESS_NAME        
#define IDS_ACC_KEYTAB                  (RESBASE+0x372)        
#define IDS_ACC_MOUSETAB                (RESBASE+0x373)        
#define IDS_ACC_SOUNDTAB                (RESBASE+0x374)        
#define IDS_ACC_DISPTAB                 (RESBASE+0x375)        
#define IDS_ACC_GENTAB                  (RESBASE+0x376)        
#define IDS_ACCWRN_NONE                 (RESBASE+0x377)
#define IDS_ACCWRN_CAPBAR               (RESBASE+0x378)
#define IDS_ACCWRN_ACTWIND              (RESBASE+0x379)
#define IDS_ACCWRN_DESKTOP              (RESBASE+0x37a)
#define IDS_BLACKWHITE_SCHEME           (RESBASE+0x37b)        
#define IDS_WHITEBLACK_SCHEME           (RESBASE+0x37c)

// Power has (RESBASE+0x3c0)-(RESBASE+0x3ff) above

// Common strings
#define IDS_DEFAULT_LONGNAME            (RESBASE+0x500)
#define IDS_CPLERR_DEVICENAME           (RESBASE+0x503)
#define IDS_CPLERR_DEVICETITLE          (RESBASE+0x504)
#define IDS_PWDCHK_TITLE                (RESBASE+0x505)
#define IDS_PWDCHKERR_INCORRECT         (RESBASE+0x506)
#define IDS_BROWSE                      (RESBASE+0x507)
#define IDS_NONE                        (RESBASE+0x508)
#define IDS_FILENOTFOUND                (RESBASE+0x509)
#define IDS_CPLERR_MEMORY_ERROR         (RESBASE+0x50a)
#define IDS_CPLERR_LOWMEM_TITLE         (RESBASE+0x50b)
//

// Network Icons
#define IDI_NETCPL                  5000

// Comm icons
#define IDI_COMM                    5011
#define IDI_RAS                     5012
#define IDI_DIALING                 5013

// Keybd icons
#define IDI_KEYBD                   5020
#define IDI_DELAY                   5021
#define IDI_RATE                    5022
#define IDB_LEFT                    5023
#define IDB_RIGHT                   5024

// Owner icon
#define IDI_PROFILE                 5040

// Power icons
#define IDI_POWER                   5050
#define IDI_MAIN_BAT                5051
#define IDI_BACK_BAT                5052
#define IDI_SLEEP                   5053
#define IDI_POWER_AC                5054
#define IDI_POWER_BAT               5055

// System icons
#define IDI_SYSTEM                  5060

// Screen icons
#define IDI_SCREEN                  5070
#define IDI_BACKLIGHT               5071
//#define IDI_MYHPC                 5072
#define IDB_SCHEME                  5073
#define IDB_OK                      5074
#define IDB_CLOSE                   5075


// Stylus icons
#define IDI_STYLUS                  5080

// Mouse icons
#define IDI_MOUSE                   5090

// Shared by stylus & mouse
#define IDI_CLAPPER                 5100
#define IDI_CLAPDOWN                5101
#define IDB_CLAPGRID1               5102
#define IDB_CLAPGRID2               5103

// Sound icons
#define IDI_SOUNDS                  5110
// sound volume bmps
#define IDB_UP                      5120
#define IDB_DOWN                    5121

// Sound scheme icons
#define IDI_AUDIO                   5130
#define IDI_PROGRAM                 5131
#define IDI_BLANK                   5132
#define IDB_STOP                    5133

// SIP icons
#define IDI_SIP                     5140

// Remove-programs icons
#define IDI_REMOVE                  5150
#define IDI_ALERT                   5151
#define IDI_TRASH                   5152

// Accessibility Icons
#define IDI_ACCESS                  5160
#define IDI_ACC_STICKY              5161
#define IDI_ACC_TOGGLE              5162
#define IDI_ACC_SNDSENTRY           5163
#define IDI_ACC_SNDSHOW             5164
#define IDI_ACC_DISP                5165
#define IDI_ACC_MKEYS               5166
#define IDI_ACC_SETTINGS            5167
#define IDI_ACC_SHORTCUT            5168

// Date/Time icons
#define IDI_TIME                    5170

// Certificate icons
#define IDI_CERT                    5180    // feature

// Common icons
#define IDI_PASSWD                  6000

