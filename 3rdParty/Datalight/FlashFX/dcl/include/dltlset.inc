;----------------------------------------------------------------------------
;              ----> DO NOT REMOVE THE FOLLOWING NOTICE <----
;
;                 Copyright (c) 1993 - 2010 Datalight, Inc.
;                      All Rights Reserved Worldwide.
;
; Datalight, Incorporated is a Washington State corporation currently located
; at:
;       21520 30th Dr SE, Suite 110,      Tel:  425-951-8086
;       Bothell, WA  98021                Fax:  425-951-8094
;       USA                               Web:  http://www.datalight.com
;
; This software, including without limitation all source code and documen-
; tation, is a trade secret and the confidential property of Datalight, 
; Inc., protected under the copyright laws of the United States and other
; jurisdictions.  Patents may be pending.
;
; U.S. Government Restricted Rights:  Use, duplication, reproduction, or
; transfer of this commercial product and accompanying documentation is
; restricted in accordance with FAR 12.212 and DFARS 227.7202 and by a
; License Agreement.
;
; IN ADDITION TO COPYRIGHT AND PATENT LAW, THIS SOFTWARE IS PROTECTED UNDER
; CONTRACT(S) BETWEEN DATALIGHT, INC. AND THE LICENSEE ("BINDING AGREEMENTS").
; IF YOU ARE A LICENSEE, YOUR RIGHT, IF ANY, TO COPY, PUBLISH, MODIFY, OR
; OTHERWISE USE THE SOFTWARE, IS SUBJECT TO THE TERMS AND CONDITIONS OF THE
; BINDING AGREEMENTS.  BY USING THE SOFTWARE IN ANY MANNER, IN WHOLE OR IN
; PART, YOU AGREE TO BE BOUND BY THE TERMS OF THE BINDING AGREEMENTS.
;
; IF YOU ARE NOT A DATALIGHT LICENSEE, ANY USE MAY RESULT IN CIVIL AND
; CRIMINAL ACTION AGAINST YOU.  CONTACT DATALIGHT, INC. AT THE ADDRESS
; SET FORTH ABOVE IF YOU OBTAINED THIS SOFTWARE IN ERROR.
;----------------------------------------------------------------------------

;----------------------------------------------------------------------------
;                               Description
;
;       This header defines the legitimate values for D_TOOLSETNUM.
;
;       This information MUST match that found in dltlset.h and dltlinit.h,
;       as well as the settings found in all the various toolset.mak/
;       toolinit.bat files.
;
;       The numbering scheme used is as follows:
;
;       1) The 1000's digit indicates the vendor as well as the major
;          product category:
;               1000 - Borland
;               2000 - Microsoft
;               3000 - ARM
;               4000 - Diab
;               5000 - GNU
;               6000 - Green Hills
;               7000 - Altium Tasking
;               8000 - Mentor
;               9000 - Marvell
;              10000 - Texas Instruments
;       2) The 100's and 10's digits indicate the version number.
;       3) The 1's digit is used for further differentiation as needed.
;----------------------------------------------------------------------------

;----------------------------------------------------------------------------
;                               Revision History
;   $Log: dltlset.inc $
;   Revision 1.12  2010/06/24 14:19:17Z  jimmb
;   Name police - Bad programmer - no donut - renamed to reflect GNU
;   Revision 1.11  2010/06/18 15:12:56Z  jimmb
;   Completed adding the Intel compiler descriptions for DCL
;   Revision 1.10  2010/04/28 23:30:36Z  garyp
;   Added support for the TICCS ToolSet abstraction.
;   Revision 1.9  2009/02/08 01:14:24Z  garyp
;   Merged from the v4.0 branch.  Added support for the MARVELLSDT and
;   MSWCE ToolSet abstractions.
;   Revision 1.8  2008/03/11 21:56:02Z  jeremys
;   Added WDK support.
;   Revision 1.7  2007/11/03 23:31:22Z  Garyp
;   Added the standard module header.
;   Revision 1.6  2007/06/15 00:59:55Z  pauli
;   Replaced the poorly named gnunu toolset with gnucs.
;   Revision 1.5  2007/03/18 18:05:54Z  Garyp
;   Added the "msvs" ToolSet.
;   Revision 1.4  2006/12/06 23:55:37Z  Garyp
;   Added Microtec ToolSet support.
;   Revision 1.3  2006/11/02 19:54:07Z  Garyp
;   Renamed the Green Hills toolset from ghsarm36 to ghs.
;   Revision 1.2  2006/10/07 01:03:26Z  Garyp
;   Added support for the RVDS30 ToolSet abstraction.
;   Revision 1.1  2005/10/02 00:29:48Z  Pauli
;   Initial revision
;   Revision 1.5  2005/06/20 20:55:24Z  Pauli
;   Added toolset for Nucleus GNU tools.
;   Revision 1.4  2005/06/10 16:55:30Z  PaulI
;   XP Merge
;   Revision 1.3  2005/04/24 07:13:56Z  GaryP
;   Added support for new and renamed ToolSets.
;   Revision 1.2.1.2  2005/03/31 21:06:50Z  garyp
;   Added the MSWINDDK ToolSet.
;   Revision 1.2  2005/02/14 01:02:40Z  GaryP
;   Prefixed the toolset names with "TS_".
;   Revision 1.1  2005/01/25 23:47:28Z  GaryP
;   Initial revision
;----------------------------------------------------------------------------

TS_BC4X         equ  <1400>     ; Borland C 4.x
TS_DEVTOOLS     equ  <1520>     ; DL DevTools (BC 5.2)
TS_MSWCE4       equ  <2400>     ; Microsoft Windows CE.net 4.2 Command Line (deprecated use "mswce")
TS_MSWCEPB      equ  <2401>     ; Microsoft Windows CE Platform Builder 3 & 4
TS_MSWCE        equ  <2402>     ; Microsoft Windows CE 4, 5 and 6, WinMobile, Command Line
TS_MSVC6        equ  <2600>     ; Microsoft Visual C 6
TS_MSVS         equ  <2620>     ; Microsoft Visual Studio 6, 7, and 8
TS_MSWINDDK     equ  <2700>     ; Microsoft Windows XP DDK
TS_MSWDK        equ  <2701>     ; Microsoft Windows XP WDK
TS_ADS12        equ  <3120>     ; ARM Developer's Suite 1.2
TS_RVDS30       equ  <3300>     ; ARM RealView Developer's Suite 3.0
TS_DIABVX5X     equ  <4500>     ; WindRiver Diab tools for VxWorks 5.x
TS_DIABVX6X     equ  <4505>     ; WindRiver Diab tools for VxWorks 6.x
TS_GNUVX5X      equ  <5290>     ; GNU Tools for VxWorks 5.x
TS_GNUVX6X      equ  <5295>     ; GNU Tools for VxWorks 6.x
TS_ICCVX6X      equ  <5296>     ; Intel Tools for VxWorks 6.x
TS_GNURTOS	equ  <5297>	; GNU Tools for RTOS
TS_GNUCS        equ  <5400>     ; Code Sourcery ARM GNU Tools
TS_GHS          equ  <6360>     ; Green Hills ARM tools (v4.2.3 tested)
TS_TASKING      equ  <7152>     ; Altium Tasking v1.5r2
TS_MICROTEC     equ  <8500>     ; Mentor Graphics Microtec tools
TS_MARVELLSDT   equ  <9210>	; Marvell SDT v2.1
TS_TICCS        equ <10000>     ; Texas Instruments Code Composer Studio  

