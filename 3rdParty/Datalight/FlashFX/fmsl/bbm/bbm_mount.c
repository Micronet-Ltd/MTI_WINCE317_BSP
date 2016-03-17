/*---------------------------------------------------------------------------
               ----> DO NOT REMOVE THE FOLLOWING NOTICE <----

                  Copyright (c) 1993 - 2012 Datalight, Inc.
                       All Rights Reserved Worldwide.

  Datalight, Incorporated is a Washington State corporation currently located
  at:
        21520 30th Dr SE, Suite 110,      Tel:  425-951-8086
        Bothell, WA  98021                Fax:  425-951-8094
        USA                               Web:  http://www.datalight.com

  This software, including without limitation all source code and documen-
  tation, is a trade secret and the confidential property of Datalight, 
  Inc., protected under the copyright laws of the United States and other
  jurisdictions.  The software may be subject to one or more US patents, 
  including without limitation US Patents: US#5860082, US#6260156.  Patents
  may be pending.

  U.S. Government Restricted Rights:  Use, duplication, reproduction, or
  transfer of this commercial product and accompanying documentation is
  restricted in accordance with FAR 12.212 and DFARS 227.7202 and by a
  License Agreement.

  IN ADDITION TO COPYRIGHT AND PATENT LAW, THIS SOFTWARE IS PROTECTED UNDER
  CONTRACT(S) BETWEEN DATALIGHT, INC. AND THE LICENSEE ("BINDING AGREEMENTS").
  IF YOU ARE A LICENSEE, YOUR RIGHT, IF ANY, TO COPY, PUBLISH, MODIFY, OR
  OTHERWISE USE THE SOFTWARE, IS SUBJECT TO THE TERMS AND CONDITIONS OF THE
  BINDING AGREEMENTS.  BY USING THE SOFTWARE IN ANY MANNER, IN WHOLE OR IN
  PART, YOU AGREE TO BE BOUND BY THE TERMS OF THE BINDING AGREEMENTS.

  IF YOU ARE NOT A DATALIGHT LICENSEE, ANY USE MAY RESULT IN CIVIL AND
  CRIMINAL ACTION AGAINST YOU.  CONTACT DATALIGHT, INC. AT THE ADDRESS
  SET FORTH ABOVE IF YOU OBTAINED THIS SOFTWARE IN ERROR.
---------------------------------------------------------------------------*/
/* COBF by BB -- 'bbm_mount.c' obfuscated at Tue Apr 24 23:20:51 2012
*/
#include "cobf.h"
#ifndef D_PRODUCTLOCALE
#define D_PRODUCTLOCALE (2)
#endif
#include <dcl.h>
#include <ffxconf.h>
#include <oesl.h>
FooFFXBbmBarb DCLSTATUS FFXSTATUS;
#define FooFFXBbmBarn DCLSTAT_SUCCESS
#define FooFFXBbmBarl285 DCLSTAT_FAILURE
#define FooFFXBbmBarl57( FooFFXBbmBarl125, FooFFXBbmBarl87)  \
DCL_MAKESTATUS( PRODUCTNUM_FLASHFX, DCLSTATUSCLASS_ERROR,   FooFFXBbmBarl125, FooFFXBbmBarl87)
#define FooFFXBbmBarl272 DCLSTAT_BADSTRUCLEN
#define FooFFXBbmBarl140 DCLSTAT_BADPARAMETER
#define FooFFXBbmBarl163 DCLSTAT_BADHANDLE
#define FooFFXBbmBarl284 DCLSTAT_UNSUPPORTEDFUNCTION
#define FooFFXBbmBarl465 DCLSTAT_BADSYNTAX
#define FooFFXBbmBarl620 DCLSTAT_ARGUMENTSTOOLONG
#define FooFFXBbmBarl591 DCLSTAT_BADPARAMETERLEN
#define FooFFXBbmBarl242 DCLSTAT_OUTOFMEMORY
#define FooFFXBbmBarl562 DCLSTAT_UNSUPPORTEDREQUEST
#define FooFFXBbmBarl210 DCLSTAT_OUTOFRANGE
#define FooFFXBbmBarl619 DCLSTAT_CATEGORYDISABLED
#define FooFFXBbmBarl584 DCLSTAT_SUBTYPECOMPLETE
#define FooFFXBbmBarl595 DCLSTAT_SUBTYPEUNUSED
#define FooFFXBbmBarl601 DCLSTAT_UNSUPPORTEDFEATURE
#define FooFFXBbmBarl538 FooFFXBbmBarl456
FooFFXBbmBarb FooFFXBbmBarc FooFFXBbmBarl592* *
FooFFXBbmBarl534;FooFFXBbmBarb DCLOSREQHANDLE FFXREQHANDLE;
FooFFXBbmBarb FooFFXBbmBarc FooFFXBbmBarl585*
FooFFXBbmBarl215;FooFFXBbmBarb FooFFXBbmBarc
FooFFXBbmBarl602*FooFFXBbmBarl476;FooFFXBbmBarb
FooFFXBbmBarc FooFFXBbmBarl571 FooFFXBbmBarl44;
FooFFXBbmBarb FooFFXBbmBarc FooFFXBbmBarl459
FooFFXBbmBarl175;FooFFXBbmBarb FooFFXBbmBarc
FooFFXBbmBarl470 FooFFXBbmBarl166;FooFFXBbmBarb
FooFFXBbmBarc FooFFXBbmBarl550 FooFFXBbmBarl105;
FooFFXBbmBarb FooFFXBbmBarc FooFFXBbmBarl169
FooFFXBbmBarl510;FooFFXBbmBarb FooFFXBbmBarc
FooFFXBbmBarl169* *FooFFXBbmBarl63;FooFFXBbmBarb
FooFFXBbmBarc FooFFXBbmBarl519 FooFFXBbmBarl164;
FooFFXBbmBarb FooFFXBbmBarc FooFFXBbmBarl524
FooFFXBbmBarl179;FooFFXBbmBarb FooFFXBbmBarc
FooFFXBbmBarl561 FooFFXBbmBarl102;FooFFXBbmBarb
FooFFXBbmBarc FooFFXBbmBarl174 FooFFXBbmBarl582;
FooFFXBbmBarb FooFFXBbmBarc FooFFXBbmBarl174* *
FooFFXBbmBarl568;FooFFXBbmBarb FooFFXBbmBarc
FooFFXBbmBarl271 FooFFXBbmBarl43;
#define FFX_FORMAT_NEVER (0)
#define FFX_FORMAT_ONCE (1)
#define FFX_FORMAT_ALWAYS (2)
#define FFX_FORMAT_UNFORMAT (3)
#define FooFFXBbmBarl579 ( D_UINT32_MAX)
#define FooFFXBbmBarl499 ( D_UINT32_MAX)
#define FooFFXBbmBarl617 ( D_UINT32_MAX-2)
#define FooFFXBbmBarl411 (( FooFFXBbmBard  *  )   D_UINT32_MAX)
#define FooFFXBbmBarl394 TRUE, FFX_FORMAT_ONCE,  \
FooFFXBbmBarl464,100
#define FFX_FILESYS_UNKNOWN FooFFXBbmBarl380
#define FFX_FILESYS_FAT FooFFXBbmBarl400
#define FFX_FILESYS_RELIANCE FooFFXBbmBarl357
#define FooFFXBbmBarl616 FooFFXBbmBarl429
#define FooFFXBbmBarl490 FFX_FORMAT_ONCE,  \
FFX_FILESYS_UNKNOWN, FALSE, 0
#define FFX_COMPACT_NONE (0)
#define FFX_COMPACT_SYNCHRONOUS (0)
#define FFX_COMPACT_BACKGROUNDIDLE (1)
#define FFX_COMPACT_BACKGROUNDTHREAD (2)
#define FooFFXBbmBarl588 FALSE
#ifndef FFXCONF_ALLOCATORSUPPORT
#define FFXCONF_ALLOCATORSUPPORT TRUE
#endif
#define FooFFXBbmBarl104 FFXCONF_ALLOCATORSUPPORT
#if ! FFXCONF_ALLOCATORSUPPORT
#if FooFFXBbmBarl104
#error  \
"FFX: FFXCONF_ALLOCATORSUPPORT is FALSE, but FFXCONF_VBFSUPPORT is TRUE"
#endif
#endif
#ifndef FFXCONF_QUICKMOUNTSUPPORT
#define FFXCONF_QUICKMOUNTSUPPORT FALSE
#endif
#if FFXCONF_QUICKMOUNTSUPPORT
#if ! FooFFXBbmBarl104
#error  \
"FFX: FFXCONF_QUICKMOUNTSUPPORT is TRUE, but FFXCONF_VBFSUPPORT is FALSE."
#endif
#endif
#ifndef FFXCONF_DRIVERAUTOFORMAT
#define FFXCONF_DRIVERAUTOFORMAT FALSE
#endif
#ifndef FooFFXBbmBarl69
#define FooFFXBbmBarl69 FALSE
#endif
#ifndef FFXCONF_FATFORMATSUPPORT
#define FFXCONF_FATFORMATSUPPORT FALSE
#endif
#ifndef FFXCONF_BBMFORMAT
#define FFXCONF_BBMFORMAT FALSE
#endif
#define FooFFXBbmBarl613 FFXCONF_NANDSUPPORT
#ifndef FooFFXBbmBarl161
#define FooFFXBbmBarl161 FFXCONF_NANDSUPPORT
#endif
#if ! FFXCONF_FATSUPPORT
#if FFXCONF_FATFORMATSUPPORT
#error  \
"FFX: FFXCONF_FATSUPPORT is FALSE, but FFXCONF_FATFORMATSUPPORT is TRUE"
#endif
#if FFXCONF_FATMONITORSUPPORT
#error  \
"FFX: FFXCONF_FATSUPPORT is FALSE, but FFXCONF_FATMONITORSUPPORT is TRUE"
#endif
#endif
#if ! FFXCONF_MBRSUPPORT
#if FooFFXBbmBarl69
#error  \
"FFX: FFXCONF_MBRSUPPORT is FALSE, but FFXCONF_MBRFORMAT is TRUE"
#endif
#endif
#if ! FFXCONF_FORMATSUPPORT
#if FooFFXBbmBarl69
#error  \
"FFX: FFXCONF_FORMATSUPPORT is FALSE, but FFXCONF_MBRFORMAT is TRUE"
#endif
#if FFXCONF_DRIVERAUTOFORMAT
#error  \
"FFX: FFXCONF_FORMATSUPPORT is FALSE, but FFXCONF_DRIVERAUTOFORMAT is TRUE"
#endif
#if FFXCONF_FATFORMATSUPPORT
#error  \
"FFX: FFXCONF_FORMATSUPPORT is FALSE, but FFXCONF_FATFORMATSUPPORT is TRUE"
#endif
#if FFXCONF_BBMFORMAT
#error  \
"FFX: FFXCONF_FORMATSUPPORT is FALSE, but FFXCONF_BBMFORMAT is TRUE"
#endif
#endif
#if ! FFXCONF_ALLOCATORSUPPORT
#if FFX_COMPACTIONMODEL != FFX_COMPACT_NONE
#error  \
"FFX: FFXCONF_ALLOCATORSUPPORT is FALSE, but FFX_COMPACTIONMODEL is not 'FFX_COMPACT_NONE'"
#endif
#endif
#if FFX_COMPACTIONMODEL == FFX_COMPACT_BACKGROUNDTHREAD
#if DCL_OSFEATURE_THREADS == FALSE
#error  \
"FFX: FFX_COMPACTIONMODEL is FFX_COMPACT_BACKGROUNDTHREAD, but DCL_OSFEATURE_THREADS is FALSE"
#endif
#else
#if FFX_COMPACTIONMODEL != FFX_COMPACT_BACKGROUNDIDLE
#if FFX_COMPACTIONMODEL != FFX_COMPACT_NONE
#error "FFX: FFX_COMPACTIONMODEL is not valid"
#endif
#endif
#endif
#ifndef FFXCONF_NANDSUPPORT
#define FFXCONF_NANDSUPPORT TRUE
#endif
#ifndef FFXCONF_NORSUPPORT
#define FFXCONF_NORSUPPORT TRUE
#endif
#ifndef FFXCONF_ISWFSUPPORT
#define FFXCONF_ISWFSUPPORT FALSE
#endif
#if ((! FFXCONF_NANDSUPPORT) && (! FFXCONF_NORSUPPORT) && (  ! FFXCONF_ISWFSUPPORT))
#error  \
"FFX: FFXCONF_NANDSUPPORT, FFXCONF_NORSUPPORT, and FFXCONF_ISWFSUPPORT are all FALSE"
#endif
#ifndef FFXCONF_BBMSUPPORT
#define FFXCONF_BBMSUPPORT FFXCONF_NANDSUPPORT
#endif
#if ! FFXCONF_NANDSUPPORT
#if FFXCONF_BBMSUPPORT
#error  \
"FFX: FFXCONF_BBMSUPPORT is TRUE but FFXCONF_NANDSUPPORT is FALSE"
#endif
#endif
#if ! FFXCONF_BBMSUPPORT
#if FFXCONF_BBMFORMAT
#error  \
"FFX: FFXCONF_BBMSUPPORT is FALSE but FFXCONF_BBMFORMAT is TRUE"
#endif
#endif
#ifndef FFXCONF_STATS_BBM
#define FFXCONF_STATS_BBM FFXCONF_BBMSUPPORT
#endif
#define FooFFXBbmBarl607 FALSE
#define FooFFXBbmBarl586 FALSE
#ifndef FFXCONF_STATS_COMPACTION
#define FFXCONF_STATS_COMPACTION FALSE
#endif
#ifndef FFXCONF_STATS_VBFREGION
#define FFXCONF_STATS_VBFREGION FALSE
#endif
#if ! FFXCONF_LATENCYREDUCTIONENABLED
#define FFXCONF_ERASESUSPENDSUPPORT FALSE
#define FooFFXBbmBarl417 FALSE
#endif
#if FFXCONF_NANDSUPPORT
#ifndef FFXCONF_MIGRATE_LEGACY_FLASH
#define FFXCONF_MIGRATE_LEGACY_FLASH FALSE
#endif
#endif
#if DCLCONF_COMMAND_SHELL
#ifndef FooFFXBbmBarl73
#define FooFFXBbmBarl73 FooFFXBbmBarl427
#endif
#ifndef FooFFXBbmBarl71
#define FooFFXBbmBarl71 FooFFXBbmBarl327
#endif
#else
#if defined( FooFFXBbmBarl73) && FooFFXBbmBarl73
#error  \
"FFX: FFXCONF_SHELL_TESTS is defined, but DCLCONF_COMMAND_SHELL is FALSE"
#endif
#if defined( FooFFXBbmBarl71) && FooFFXBbmBarl71
#error  \
"FFX: FFXCONF_SHELL_TOOLS is defined, but DCLCONF_COMMAND_SHELL is FALSE"
#endif
#endif
FooFFXBbmBarb FooFFXBbmBarl42{FooFFXBbmBarl526=0x0000,
#if FFXCONF_FATSUPPORT
#if FFXCONF_FATFORMATSUPPORT
FooFFXBbmBarl590=0x0400,FooFFXBbmBarl589,FooFFXBbmBarl518,
#endif
#if FFXCONF_FATMONITORSUPPORT
FooFFXBbmBarl570=0x0600,
#endif
#endif
#if FFX_COMPACTIONMODEL != FFX_COMPACT_SYNCHRONOUS
FooFFXBbmBarl555=0x0A00,FooFFXBbmBarl466,
#if FFX_COMPACTIONMODEL == FFX_COMPACT_BACKGROUNDTHREAD
FooFFXBbmBarl587,FooFFXBbmBarl543,
#endif
#endif
FooFFXBbmBarl558=0x2000,FooFFXBbmBarl572,FooFFXBbmBarl468,
FooFFXBbmBarl517,
#if FFXCONF_LATENCYREDUCTIONENABLED
FooFFXBbmBarl478,FooFFXBbmBarl483,
#endif
#if FFXCONF_BBMSUPPORT
FooFFXBbmBarl514,
#endif
FooFFXBbmBarl594,FooFFXBbmBarl531,FooFFXBbmBarl516,
FooFFXBbmBarl475=0x3000,FooFFXBbmBarl615,FooFFXBbmBarl477,
FooFFXBbmBarl611,FooFFXBbmBarl535,
#if FFXCONF_ALLOCATORSUPPORT
FooFFXBbmBarl481,FooFFXBbmBarl505,
#endif
#if FFXCONF_FORMATSUPPORT
FooFFXBbmBarl511,
#endif
FooFFXBbmBarl532=0x6000,FooFFXBbmBarl485,FooFFXBbmBarl494,
FooFFXBbmBarl603,FooFFXBbmBarl489=0x7000,FooFFXBbmBarl473,
FooFFXBbmBarl495,FooFFXBbmBarl537,FooFFXBbmBarl539,
FooFFXBbmBarl551,FooFFXBbmBarl567,FooFFXBbmBarl469,
FooFFXBbmBarl581=0x8000,FooFFXBbmBarl386=0x9000,
FooFFXBbmBarl501,FooFFXBbmBarl566}FooFFXBbmBarl72;
FooFFXBbmBarm FooFFXBbmBarl593(FooFFXBbmBarl72 FooFFXBbmBarl101,FooFFXBbmBard*FooFFXBbmBarl65, FooFFXBbmBara FooFFXBbmBarl95);FooFFXBbmBarb FooFFXBbmBarl42
{FooFFXBbmBarl512=0x0000,FooFFXBbmBarl472=0x0001,
FooFFXBbmBarl575=0x0100,FooFFXBbmBarl461,FooFFXBbmBarl569,
FooFFXBbmBarl488,FooFFXBbmBarl529,FooFFXBbmBarl545,
FooFFXBbmBarl520,FooFFXBbmBarl460,FooFFXBbmBarl597,
FooFFXBbmBarl523,FooFFXBbmBarl546,FooFFXBbmBarl482,
FooFFXBbmBarl497=0x0200,FooFFXBbmBarl574,FooFFXBbmBarl533,
FooFFXBbmBarl577,FooFFXBbmBarl552,FooFFXBbmBarl463=0x0300,
FooFFXBbmBarl525,FooFFXBbmBarl503=0x0400,FooFFXBbmBarl573,
FooFFXBbmBarl486,FooFFXBbmBarl493,FooFFXBbmBarl559=0x04C0,
FooFFXBbmBarl583=0x04C0,FooFFXBbmBarl618,FooFFXBbmBarl599,
FooFFXBbmBarl506,FooFFXBbmBarl556,FooFFXBbmBarl500=0x0500,
FooFFXBbmBarl608}FooFFXBbmBarl504;FooFFXBbmBard
FooFFXBbmBarl542(FooFFXBbmBara FooFFXBbmBarl467);
FooFFXBbmBara FooFFXBbmBarl598(FooFFXBbmBard);FooFFXBbmBard
FooFFXBbmBarl548(FooFFXBbmBara FooFFXBbmBarl614);
FooFFXBbmBarm FooFFXBbmBarl509(FooFFXBbmBard);FooFFXBbmBarm
FooFFXBbmBarl557(FooFFXBbmBarl44*FooFFXBbmBarl176);
FooFFXBbmBard FooFFXBbmBarl604(FooFFXBbmBarl44* FooFFXBbmBarl176);FooFFXBbmBarl102*FooFFXBbmBarl554( FooFFXBbmBari FooFFXBbmBarl44*FooFFXBbmBarl76,FooFFXBbmBari FooFFXBbmBarl164*FooFFXBbmBarl487,FooFFXBbmBarl179* FooFFXBbmBarl162);FooFFXBbmBard FooFFXBbmBarl600( FooFFXBbmBari FooFFXBbmBarl44*FooFFXBbmBarl76, FooFFXBbmBarl102*FooFFXBbmBarl170);FooFFXBbmBarl105*
FooFFXBbmBarl480(FooFFXBbmBari FooFFXBbmBarl44* FooFFXBbmBarl76,FooFFXBbmBari FooFFXBbmBarl175* FooFFXBbmBarl576,FooFFXBbmBarl166*FooFFXBbmBarl162);
FooFFXBbmBard FooFFXBbmBarl492(FooFFXBbmBari FooFFXBbmBarl44  *FooFFXBbmBarl76,FooFFXBbmBarl105*FooFFXBbmBarl170);
FFXSTATUS FooFFXBbmBarl508(FooFFXBbmBarl63 FooFFXBbmBarl74, FooFFXBbmBarl61 FooFFXBbmBarl187);FFXSTATUS FooFFXBbmBarl564
(FooFFXBbmBarl63 FooFFXBbmBarl74,FooFFXBbmBarl61 FooFFXBbmBarl187);FooFFXBbmBara FooFFXBbmBarl605( FooFFXBbmBarl63 FooFFXBbmBarl74,FooFFXBbmBara FooFFXBbmBarl209,FooFFXBbmBara FooFFXBbmBarl596, FooFFXBbmBarl231 FooFFXBbmBard* *FooFFXBbmBarl521);
FooFFXBbmBara FooFFXBbmBarl578(FooFFXBbmBarl63 FooFFXBbmBarl74,FooFFXBbmBara FooFFXBbmBarr,FooFFXBbmBara FooFFXBbmBarl206);FooFFXBbmBarm FooFFXBbmBarl360( FooFFXBbmBarl72 FooFFXBbmBarl101,FooFFXBbmBard* FooFFXBbmBarl98,FooFFXBbmBard*FooFFXBbmBarl65,FooFFXBbmBara FooFFXBbmBarl95);FooFFXBbmBarm FooFFXBbmBarl606( FooFFXBbmBarl72 FooFFXBbmBarl101,FooFFXBbmBard* FooFFXBbmBarl98,FooFFXBbmBard*FooFFXBbmBarl65,FooFFXBbmBara FooFFXBbmBarl95);FooFFXBbmBarl FooFFXBbmBarl547( FooFFXBbmBari FooFFXBbmBars*FooFFXBbmBarl351);FooFFXBbmBari
FooFFXBbmBars*FooFFXBbmBarl352(FooFFXBbmBarm FooFFXBbmBarl255);
#if D_DEBUG && DCLCONF_OUTPUT_ENABLED
#define FooFFXBbmBarl528 DCLTRACE_ALWAYS
#define FooFFXBbmBarl188 1
#define FFXTRACE_DRIVERFW 2
#define FooFFXBbmBarl181 3
#define FFXTRACE_COMPACTION 4
#define FooFFXBbmBarl178 5
#define FooFFXBbmBarl186 6
#define FFXTRACE_BBM 7
#define FooFFXBbmBarl184 8
#define FooFFXBbmBarl177 9
#define FooFFXBbmBarl165 10
#define FFXTRACE_CONFIG 11
#define FooFFXBbmBarl171 12
#define FooFFXBbmBarl168 13
#define FooFFXBbmBarl182 14
#define FooFFXBbmBarl185 15
#define FooFFXBbmBarl183 16
#define FooFFXBbmBarl311 0
#define FooFFXBbmBarl479 D_UINT32_MAX
#define FooFFXBbmBarl560 (( FooFFXBbmBara)1 << (   FooFFXBbmBarl188-1))
#define FooFFXBbmBarl515 (( FooFFXBbmBara)1 << (   FFXTRACE_DRIVERFW-1))
#define FooFFXBbmBarl540 (( FooFFXBbmBara)1 << (   FooFFXBbmBarl181-1))
#define FooFFXBbmBarl553 (( FooFFXBbmBara)1 << (   FFXTRACE_COMPACTION-1))
#define FooFFXBbmBarl496 (( FooFFXBbmBara)1 << (   FooFFXBbmBarl178-1))
#define FooFFXBbmBarl536 (( FooFFXBbmBara)1 << (   FooFFXBbmBarl186-1))
#define FooFFXBbmBarl541 (( FooFFXBbmBara)1 << (   FFXTRACE_BBM-1))
#define FooFFXBbmBarl498 (( FooFFXBbmBara)1 << (   FooFFXBbmBarl184-1))
#define FooFFXBbmBarl563 (( FooFFXBbmBara)1 << (   FooFFXBbmBarl177-1))
#define FooFFXBbmBarl471 (( FooFFXBbmBara)1 << (   FooFFXBbmBarl165-1))
#define FooFFXBbmBarl612 (( FooFFXBbmBara)1 << (   FFXTRACE_CONFIG-1))
#define FooFFXBbmBarl549 (( FooFFXBbmBara)1 << (   FooFFXBbmBarl171-1))
#define FooFFXBbmBarl491 (( FooFFXBbmBara)1 << (   FooFFXBbmBarl168-1))
#define FooFFXBbmBarl462 (( FooFFXBbmBara)1 << (   FooFFXBbmBarl182-1))
#define FooFFXBbmBarl522 (( FooFFXBbmBara)1 << (   FooFFXBbmBarl185-1))
#define FooFFXBbmBarl530 (( FooFFXBbmBara)1 << (   FooFFXBbmBarl183-1))
FooFFXBbmBarl79 FooFFXBbmBarl173(FooFFXBbmBari FooFFXBbmBars  *FooFFXBbmBarl142,...);FooFFXBbmBarl79 FooFFXBbmBarl180( FooFFXBbmBara FooFFXBbmBarl247,FooFFXBbmBari FooFFXBbmBars* FooFFXBbmBarl142,...);FooFFXBbmBara FooFFXBbmBarl565( FooFFXBbmBard);FooFFXBbmBara FooFFXBbmBarl527(FooFFXBbmBara FooFFXBbmBarl347);
#define FFXPRINTF( FooFFXBbmBarl88, FooFFXBbmBarl97) (((   FooFFXBbmBarl88)<= D_DEBUG) ? ( FooFFXBbmBard)   FooFFXBbmBarl173 FooFFXBbmBarl97 : (( FooFFXBbmBard)0))
#define FFXTRACEPRINTF( FooFFXBbmBarl96) FooFFXBbmBarl180  \
FooFFXBbmBarl96
#else
#define FFXPRINTF( FooFFXBbmBarl88, FooFFXBbmBarl97)
#define FFXTRACEPRINTF( FooFFXBbmBarl96)
#endif
FooFFXBbmBari FooFFXBbmBars*FooFFXBbmBarl474(FooFFXBbmBari FooFFXBbmBarl43*FooFFXBbmBarl326);
#define FooFFXBbmBarl502( FooFFXBbmBarl31) ( FooFFXBbmBard)  \
FooFFXBbmBarl67( FooFFXBbmBarl30, __FILE__, (   FooFFXBbmBarl31))
#define FooFFXBbmBarl260( FooFFXBbmBarl31, FooFFXBbmBarl62)  \
(( FooFFXBbmBarl62) ? (( FooFFXBbmBard)0) : ( FooFFXBbmBard  ) FooFFXBbmBarl67( FooFFXBbmBarl30, __FILE__, (   FooFFXBbmBarl31)))
#if D_DEBUG >= 1
#define FooFFXBbmBarl154( FooFFXBbmBarl31) ( FooFFXBbmBard)  \
FooFFXBbmBarl67( FooFFXBbmBarl30, __FILE__, (   FooFFXBbmBarl31))
#define FooFFXBbmBarl53( FooFFXBbmBarl31, FooFFXBbmBarl62)  \
(( FooFFXBbmBarl62) ? (( FooFFXBbmBard)0) : ( FooFFXBbmBard  ) FooFFXBbmBarl67( FooFFXBbmBarl30, __FILE__, (   FooFFXBbmBarl31)))
#else
#define FooFFXBbmBarl154( FooFFXBbmBarl31)
#define FooFFXBbmBarl53( FooFFXBbmBarl31, FooFFXBbmBarl62)
#endif
FooFFXBbmBarb FooFFXBbmBarl42{FooFFXBbmBarl823=
DCLIO_MAKEFUNCTION(PRODUCTNUM_FLASHFX,0x000),
FooFFXBbmBarl916=DCLIO_MAKEFUNCTION(PRODUCTNUM_FLASHFX, 0x100),FooFFXBbmBarl949,FooFFXBbmBarl937,FooFFXBbmBarl973,
FooFFXBbmBarl997,FooFFXBbmBarl919,FooFFXBbmBarl1057,
FooFFXBbmBarl1035,FooFFXBbmBarl955,FooFFXBbmBarl950,
FooFFXBbmBarl892,FooFFXBbmBarl1047,FooFFXBbmBarl1012,
FooFFXBbmBarl974,FooFFXBbmBarl826,FooFFXBbmBarl925,
FooFFXBbmBarl918,FooFFXBbmBarl833,FooFFXBbmBarl864,
FooFFXBbmBarl796,FooFFXBbmBarl793,FooFFXBbmBarl995,
FooFFXBbmBarl962,FooFFXBbmBarl1045,FooFFXBbmBarl910,
FooFFXBbmBarl1064,FooFFXBbmBarl857,FooFFXBbmBarl844,
FooFFXBbmBarl1063,FooFFXBbmBarl898,FooFFXBbmBarl1009,
FooFFXBbmBarl968,FooFFXBbmBarl1056=DCLIO_MAKEFUNCTION( PRODUCTNUM_FLASHFX,0x200),FooFFXBbmBarl1073,
FooFFXBbmBarl847,FooFFXBbmBarl959,FooFFXBbmBarl829,
FooFFXBbmBarl810,FooFFXBbmBarl801,FooFFXBbmBarl1046,
FooFFXBbmBarl1053,FooFFXBbmBarl965,FooFFXBbmBarl942,
FooFFXBbmBarl872,FooFFXBbmBarl845,FooFFXBbmBarl827,
FooFFXBbmBarl843,FooFFXBbmBarl1031,FooFFXBbmBarl1059,
FooFFXBbmBarl988,FooFFXBbmBarl912,FooFFXBbmBarl870,
FooFFXBbmBarl798=DCLIO_MAKEFUNCTION(PRODUCTNUM_FLASHFX, 0x280),FooFFXBbmBarl821,FooFFXBbmBarl978=DCLIO_MAKEFUNCTION
(PRODUCTNUM_FLASHFX,0x300),FooFFXBbmBarl1058,
FooFFXBbmBarl935,FooFFXBbmBarl1076,FooFFXBbmBarl873,
FooFFXBbmBarl805,FooFFXBbmBarl1041,FooFFXBbmBarl802,
FooFFXBbmBarl969,FooFFXBbmBarl1038,FooFFXBbmBarl814,
FooFFXBbmBarl972,FooFFXBbmBarl1028=DCLIO_MAKEFUNCTION( PRODUCTNUM_FLASHFX,0x380),FooFFXBbmBarl815,
FooFFXBbmBarl1049,FooFFXBbmBarl913,FooFFXBbmBarl1055,
FooFFXBbmBarl989,FooFFXBbmBarl904,FooFFXBbmBarl797,
FooFFXBbmBarl908=DCLIO_MAKEFUNCTION(PRODUCTNUM_FLASHFX, 0x500),FooFFXBbmBarl945,FooFFXBbmBarl897,FooFFXBbmBarl984,
FooFFXBbmBarl1077,FooFFXBbmBarl934,FooFFXBbmBarl1017,
FooFFXBbmBarl885,FooFFXBbmBarl966,FooFFXBbmBarl957,
FooFFXBbmBarl958,FooFFXBbmBarl809,FooFFXBbmBarl1075=
DCLIO_MAKEFUNCTION(PRODUCTNUM_FLASHFX,0x600),
FooFFXBbmBarl903,FooFFXBbmBarl836,FooFFXBbmBarl899,
FooFFXBbmBarl808,FooFFXBbmBarl992,FooFFXBbmBarl1022,
FooFFXBbmBarl971,FooFFXBbmBarl932,FooFFXBbmBarl996,
FooFFXBbmBarl858,FooFFXBbmBarl1019,FooFFXBbmBarl804,
FooFFXBbmBarl893,FooFFXBbmBarl1044,FooFFXBbmBarl924,
FooFFXBbmBarl1004,FooFFXBbmBarl856,FooFFXBbmBarl800,
FooFFXBbmBarl819=DCLIO_MAKEFUNCTION(PRODUCTNUM_FLASHFX, 0x800),FooFFXBbmBarl975,FooFFXBbmBarl820,FooFFXBbmBarl961,
FooFFXBbmBarl1003,FooFFXBbmBarl976,FooFFXBbmBarl923,
FooFFXBbmBarl875,FooFFXBbmBarl939,FooFFXBbmBarl865,
FooFFXBbmBarl1065,FooFFXBbmBarl816,FooFFXBbmBarl1034,
FooFFXBbmBarl1048,FooFFXBbmBarl1008,FooFFXBbmBarl868,
FooFFXBbmBarl799,FooFFXBbmBarl824,FooFFXBbmBarl940,
FooFFXBbmBarl840,FooFFXBbmBarl795}FooFFXBbmBarl954;
FooFFXBbmBarb FooFFXBbmBarl42{FooFFXBbmBarl1062=0x0100,
FooFFXBbmBarl807=0x0200,FooFFXBbmBarl1027,FooFFXBbmBarl931,
FooFFXBbmBarl1021,FooFFXBbmBarl927,FooFFXBbmBarl1066,
FooFFXBbmBarl854,FooFFXBbmBarl944,FooFFXBbmBarl947=0x0300,
FooFFXBbmBarl983,FooFFXBbmBarl1015,FooFFXBbmBarl979,
FooFFXBbmBarl1025,FooFFXBbmBarl1072,FooFFXBbmBarl1069,
FooFFXBbmBarl822,FooFFXBbmBarl930=0x0400,FooFFXBbmBarl895=
0x0500,FooFFXBbmBarl806,FooFFXBbmBarl812,FooFFXBbmBarl1051,
FooFFXBbmBarl794,FooFFXBbmBarl1032,FooFFXBbmBarl929,
FooFFXBbmBarl933,FooFFXBbmBarl1024=0x0600,FooFFXBbmBarl1043
,FooFFXBbmBarl982,FooFFXBbmBarl813,FooFFXBbmBarl881,
FooFFXBbmBarl928,FooFFXBbmBarl848,FooFFXBbmBarl887,
FooFFXBbmBarl1007}FooFFXBbmBarl641;FooFFXBbmBarb
DCLIOREQUEST FooFFXBbmBarl40;FooFFXBbmBarb FooFFXBbmBarc{
FooFFXBbmBarl40 FooFFXBbmBarl36;FooFFXBbmBara
FooFFXBbmBarl75;FooFFXBbmBara FooFFXBbmBarl110;
FooFFXBbmBarl41*FooFFXBbmBarl100;}FooFFXBbmBarl1068;
FooFFXBbmBarb FooFFXBbmBarc{FooFFXBbmBarl40 FooFFXBbmBarl36
;FooFFXBbmBara FooFFXBbmBarl75;FooFFXBbmBara
FooFFXBbmBarl110;FooFFXBbmBari FooFFXBbmBarl41*
FooFFXBbmBarl100;}FooFFXBbmBarl1002;FooFFXBbmBarb
FooFFXBbmBarc{FooFFXBbmBarl40 FooFFXBbmBarl36;FooFFXBbmBara
FooFFXBbmBarl75;FooFFXBbmBara FooFFXBbmBarl110;
FooFFXBbmBarl41*FooFFXBbmBarl100;FooFFXBbmBarl41*
FooFFXBbmBarl228;}FooFFXBbmBarl1039;FooFFXBbmBarb
FooFFXBbmBarc{FooFFXBbmBarl40 FooFFXBbmBarl36;FooFFXBbmBara
FooFFXBbmBarl75;FooFFXBbmBara FooFFXBbmBarl110;
FooFFXBbmBari FooFFXBbmBarl41*FooFFXBbmBarl100;
FooFFXBbmBari FooFFXBbmBarl41*FooFFXBbmBarl228;}
FooFFXBbmBarl920;FooFFXBbmBarb FooFFXBbmBarc{
FooFFXBbmBarl40 FooFFXBbmBarl36;FooFFXBbmBara
FooFFXBbmBarl507;FooFFXBbmBara FooFFXBbmBarl609;}
FooFFXBbmBarl869,FooFFXBbmBarl941,FooFFXBbmBarl889,
FooFFXBbmBarl952;FooFFXBbmBarb FooFFXBbmBarc{
FooFFXBbmBarl40 FooFFXBbmBarl36;FooFFXBbmBara
FooFFXBbmBarl507;FooFFXBbmBara FooFFXBbmBarl609;}
FooFFXBbmBarl1005,FooFFXBbmBarl901,FooFFXBbmBarl1001;
FooFFXBbmBarb FooFFXBbmBarc{FooFFXBbmBarl40 FooFFXBbmBarl36
;FooFFXBbmBara FooFFXBbmBarl507;FooFFXBbmBara
FooFFXBbmBarl609;}FooFFXBbmBarl1016,FooFFXBbmBarl1006,
FooFFXBbmBarl1018;FooFFXBbmBarb FooFFXBbmBarc{
FooFFXBbmBarl40 FooFFXBbmBarl36;FooFFXBbmBara
FooFFXBbmBarl507;FooFFXBbmBara FooFFXBbmBarl609;
FooFFXBbmBarm FooFFXBbmBarl896;}FooFFXBbmBarl917,
FooFFXBbmBarl938,FooFFXBbmBarl853;FooFFXBbmBarb
FooFFXBbmBarc{FooFFXBbmBarl40 FooFFXBbmBarl36;
FooFFXBbmBarl641 FooFFXBbmBarl699;FooFFXBbmBara
FooFFXBbmBarl75;FooFFXBbmBara FooFFXBbmBarl81;
FooFFXBbmBarl41*FooFFXBbmBarl100;FooFFXBbmBarl41*
FooFFXBbmBarl689;FooFFXBbmBarl FooFFXBbmBarl688;}
FooFFXBbmBarl1000,FooFFXBbmBarl1054;FooFFXBbmBarb
FooFFXBbmBarc{FooFFXBbmBarl40 FooFFXBbmBarl36;
FooFFXBbmBarl641 FooFFXBbmBarl699;FooFFXBbmBara
FooFFXBbmBarl75;FooFFXBbmBara FooFFXBbmBarl81;FooFFXBbmBari
FooFFXBbmBarl41*FooFFXBbmBarl100;FooFFXBbmBari
FooFFXBbmBarl41*FooFFXBbmBarl689;FooFFXBbmBarl
FooFFXBbmBarl688;}FooFFXBbmBarl817,FooFFXBbmBarl993;
FooFFXBbmBarb FooFFXBbmBarc{FooFFXBbmBarl40 FooFFXBbmBarl36
;FooFFXBbmBarl FooFFXBbmBarl690;}FooFFXBbmBarl884,
FooFFXBbmBarl963;FooFFXBbmBarb FooFFXBbmBarc{
FooFFXBbmBarl40 FooFFXBbmBarl36;FooFFXBbmBarl
FooFFXBbmBarl690;}FooFFXBbmBarl894,FooFFXBbmBarl850;
#if FFXCONF_NORSUPPORT || FFXCONF_ISWFSUPPORT
FooFFXBbmBarb FooFFXBbmBarc{FooFFXBbmBarl40 FooFFXBbmBarl36
;FooFFXBbmBara FooFFXBbmBarl209;FooFFXBbmBara
FooFFXBbmBarl50;FooFFXBbmBarl41*FooFFXBbmBarl626;}
FooFFXBbmBarl831,FooFFXBbmBarl1079,FooFFXBbmBarl1026;
FooFFXBbmBarb FooFFXBbmBarc{FooFFXBbmBarl40 FooFFXBbmBarl36
;FooFFXBbmBara FooFFXBbmBarl209;FooFFXBbmBara
FooFFXBbmBarl50;FooFFXBbmBari FooFFXBbmBarl41*
FooFFXBbmBarl626;}FooFFXBbmBarl977,FooFFXBbmBarl803,
FooFFXBbmBarl811;FooFFXBbmBarb FooFFXBbmBarc{
FooFFXBbmBarl40 FooFFXBbmBarl36;FooFFXBbmBara
FooFFXBbmBarl694;FooFFXBbmBara FooFFXBbmBarl686;}
FooFFXBbmBarl999,FooFFXBbmBarl915;FooFFXBbmBarb
FooFFXBbmBarc{FooFFXBbmBarl40 FooFFXBbmBarl36;FooFFXBbmBara
FooFFXBbmBarl694;FooFFXBbmBara FooFFXBbmBarl686;}
FooFFXBbmBarl846;
#endif
#if FFXCONF_NANDSUPPORT
FooFFXBbmBarb FooFFXBbmBarc{FooFFXBbmBarl40 FooFFXBbmBarl36
;FooFFXBbmBara FooFFXBbmBarl75;FooFFXBbmBara
FooFFXBbmBarl110;FooFFXBbmBarl41*FooFFXBbmBarl100;
FooFFXBbmBarl41*FooFFXBbmBarl228;}FooFFXBbmBarl906,
FooFFXBbmBarl1014;FooFFXBbmBarb FooFFXBbmBarc{
FooFFXBbmBarl40 FooFFXBbmBarl36;FooFFXBbmBara
FooFFXBbmBarl75;FooFFXBbmBara FooFFXBbmBarl110;
FooFFXBbmBari FooFFXBbmBarl41*FooFFXBbmBarl100;
FooFFXBbmBari FooFFXBbmBarl41*FooFFXBbmBarl228;}
FooFFXBbmBarl849,FooFFXBbmBarl1061;FooFFXBbmBarb
FooFFXBbmBarc{FooFFXBbmBarl40 FooFFXBbmBarl36;FooFFXBbmBara
FooFFXBbmBarl75;FooFFXBbmBara FooFFXBbmBarl110;
FooFFXBbmBarl41*FooFFXBbmBarl100;FooFFXBbmBarl41*
FooFFXBbmBarl544;FooFFXBbmBarl FooFFXBbmBarl610;}
FooFFXBbmBarl1060;FooFFXBbmBarb FooFFXBbmBarc{
FooFFXBbmBarl40 FooFFXBbmBarl36;FooFFXBbmBara
FooFFXBbmBarl75;FooFFXBbmBara FooFFXBbmBarl110;
FooFFXBbmBari FooFFXBbmBarl41*FooFFXBbmBarl100;
FooFFXBbmBari FooFFXBbmBarl41*FooFFXBbmBarl544;
FooFFXBbmBarl FooFFXBbmBarl610;}FooFFXBbmBarl1040;
FooFFXBbmBarb FooFFXBbmBarc{FooFFXBbmBarl40 FooFFXBbmBarl36
;FooFFXBbmBara FooFFXBbmBarl75;FooFFXBbmBara
FooFFXBbmBarl681;FooFFXBbmBarl41*FooFFXBbmBarl228;}
FooFFXBbmBarl832;FooFFXBbmBarb FooFFXBbmBarc{
FooFFXBbmBarl40 FooFFXBbmBarl36;FooFFXBbmBara
FooFFXBbmBarl75;FooFFXBbmBara FooFFXBbmBarl681;
FooFFXBbmBari FooFFXBbmBarl41*FooFFXBbmBarl228;}
FooFFXBbmBarl886;FooFFXBbmBarb FooFFXBbmBarc{
FooFFXBbmBarl40 FooFFXBbmBarl36;FooFFXBbmBara
FooFFXBbmBarl75;FooFFXBbmBara FooFFXBbmBarl692;
FooFFXBbmBarl41*FooFFXBbmBarl544;FooFFXBbmBarl
FooFFXBbmBarl610;}FooFFXBbmBarl878;FooFFXBbmBarb
FooFFXBbmBarc{FooFFXBbmBarl40 FooFFXBbmBarl36;FooFFXBbmBara
FooFFXBbmBarl75;FooFFXBbmBara FooFFXBbmBarl692;
FooFFXBbmBari FooFFXBbmBarl41*FooFFXBbmBarl544;
FooFFXBbmBarl FooFFXBbmBarl610;}FooFFXBbmBarl970;
#if FFXCONF_BBMSUPPORT
FooFFXBbmBarb FooFFXBbmBarc{FooFFXBbmBarl40 FooFFXBbmBarl36
;FooFFXBbmBara FooFFXBbmBarr;FooFFXBbmBara FooFFXBbmBarl874
;FooFFXBbmBara FooFFXBbmBarl902;FooFFXBbmBara
FooFFXBbmBarl206;}FooFFXBbmBarl900,FooFFXBbmBarl891,
FooFFXBbmBarl834,FooFFXBbmBarl861;FooFFXBbmBarb
FooFFXBbmBarc{FooFFXBbmBarl40 FooFFXBbmBarl36;FooFFXBbmBara
FooFFXBbmBarl877;FooFFXBbmBara FooFFXBbmBarl110;
FooFFXBbmBara FooFFXBbmBarl998;}FooFFXBbmBarl867,
FooFFXBbmBarl911;
#endif
FooFFXBbmBarb FooFFXBbmBarc{FooFFXBbmBarl40 FooFFXBbmBarl36
;FooFFXBbmBara FooFFXBbmBarl207;}FooFFXBbmBarl888,
FooFFXBbmBarl880,FooFFXBbmBarl1074;FooFFXBbmBarb
FooFFXBbmBarc{FooFFXBbmBarl40 FooFFXBbmBarl36;FooFFXBbmBara
FooFFXBbmBarr;}FooFFXBbmBarl936,FooFFXBbmBarl1037,
FooFFXBbmBarl960,FooFFXBbmBarl909,FooFFXBbmBarl1070;
FooFFXBbmBarb FooFFXBbmBarc{FooFFXBbmBarl40 FooFFXBbmBarl36
;FooFFXBbmBara FooFFXBbmBarr;FooFFXBbmBara FooFFXBbmBarl206
;}FooFFXBbmBarl1030,FooFFXBbmBarl1052,FooFFXBbmBarl860;
FooFFXBbmBarb FooFFXBbmBarc{FooFFXBbmBarl40 FooFFXBbmBarl36
;FooFFXBbmBara FooFFXBbmBarl75;FooFFXBbmBara
FooFFXBbmBarl110;FooFFXBbmBarl41*FooFFXBbmBarl100;}
FooFFXBbmBarl835,FooFFXBbmBarl1020,FooFFXBbmBarl1042;
FooFFXBbmBarb FooFFXBbmBarc{FooFFXBbmBarl40 FooFFXBbmBarl36
;FooFFXBbmBara FooFFXBbmBarl75;FooFFXBbmBara
FooFFXBbmBarl110;FooFFXBbmBari FooFFXBbmBarl41*
FooFFXBbmBarl100;}FooFFXBbmBarl839,FooFFXBbmBarl871,
FooFFXBbmBarl946;FooFFXBbmBarb FooFFXBbmBarc{
FooFFXBbmBarl40 FooFFXBbmBarl36;}FooFFXBbmBarl981,
FooFFXBbmBarl838,FooFFXBbmBarl987;
#endif
FooFFXBbmBarc FooFFXBbmBarl271{FooFFXBbmBara FooFFXBbmBarl81
;FFXSTATUS FooFFXBbmBarl223;FooFFXBbmBara FooFFXBbmBarl247;
FooFFXBbmBarl714{FooFFXBbmBara FooFFXBbmBarl1080;
FooFFXBbmBara FooFFXBbmBarl206;FooFFXBbmBara
FooFFXBbmBarl953;FooFFXBbmBara FooFFXBbmBarl985;}
FooFFXBbmBarl1078;};
#if FFXCONF_STATS_BBM
#ifndef FooFFXBbmBarl155
#define FooFFXBbmBarl155
FooFFXBbmBarb FooFFXBbmBarc{FooFFXBbmBarl106
FooFFXBbmBarl433;FooFFXBbmBara FooFFXBbmBarl406;
FooFFXBbmBara FooFFXBbmBarl377;FooFFXBbmBara
FooFFXBbmBarl435;FooFFXBbmBara FooFFXBbmBarl339;
FooFFXBbmBara FooFFXBbmBarl447;FooFFXBbmBara
FooFFXBbmBarl365;}FooFFXBbmBarl58;FooFFXBbmBarb
FooFFXBbmBarc{FooFFXBbmBarl58 FooFFXBbmBarl458;
FooFFXBbmBarl58 FooFFXBbmBarl319;FooFFXBbmBarl58
FooFFXBbmBarl335;FooFFXBbmBarl58 FooFFXBbmBarl442;
FooFFXBbmBara FooFFXBbmBarl450;FooFFXBbmBara
FooFFXBbmBarl310;}FooFFXBbmBarl153;FooFFXBbmBarb
FooFFXBbmBarc{FooFFXBbmBars FooFFXBbmBarl436[
DCL_THREADNAMELEN];FooFFXBbmBara FooFFXBbmBarl314;
FooFFXBbmBara FooFFXBbmBarl425;FooFFXBbmBara
FooFFXBbmBarl334;FooFFXBbmBara FooFFXBbmBarl407;
FooFFXBbmBarl61 FooFFXBbmBarl443;}FooFFXBbmBarl151;
FooFFXBbmBarb FooFFXBbmBarc{FooFFXBbmBara FooFFXBbmBarl367;
FooFFXBbmBara FooFFXBbmBarl383;FooFFXBbmBara
FooFFXBbmBarl422;FooFFXBbmBara FooFFXBbmBarl342;
FooFFXBbmBara FooFFXBbmBarl346;FooFFXBbmBara
FooFFXBbmBarl454;FooFFXBbmBara FooFFXBbmBarl434;
FooFFXBbmBara FooFFXBbmBarl452;FooFFXBbmBara
FooFFXBbmBarl350;FooFFXBbmBara FooFFXBbmBarl345;
FooFFXBbmBara FooFFXBbmBarl403;FooFFXBbmBara
FooFFXBbmBarl373;FooFFXBbmBara FooFFXBbmBarl322;
FooFFXBbmBara FooFFXBbmBarl381;FooFFXBbmBara
FooFFXBbmBarl397;FooFFXBbmBara FooFFXBbmBarl428;
FooFFXBbmBara FooFFXBbmBarl343;FooFFXBbmBara
FooFFXBbmBarl328;FooFFXBbmBara FooFFXBbmBarl308;
FooFFXBbmBara FooFFXBbmBarl353;FooFFXBbmBara
FooFFXBbmBarl382;FooFFXBbmBara FooFFXBbmBarl324;
FooFFXBbmBara FooFFXBbmBarl412;FooFFXBbmBara
FooFFXBbmBarl432;FooFFXBbmBara FooFFXBbmBarl309;
FooFFXBbmBara FooFFXBbmBarl445;FooFFXBbmBara
FooFFXBbmBarl368;}FooFFXBbmBarl149;FooFFXBbmBarb
FooFFXBbmBarc{FooFFXBbmBara FooFFXBbmBarl332;FooFFXBbmBara
FooFFXBbmBarl152;FooFFXBbmBara FooFFXBbmBarl455;}
FooFFXBbmBarl158;FooFFXBbmBarb FooFFXBbmBarc{FooFFXBbmBarl
FooFFXBbmBarl318;FooFFXBbmBara FooFFXBbmBarl323;
FooFFXBbmBara FooFFXBbmBarl437;FooFFXBbmBara
FooFFXBbmBarl152;FooFFXBbmBara FooFFXBbmBarl401;
FooFFXBbmBarl FooFFXBbmBarl376;FooFFXBbmBarl
FooFFXBbmBarl375;FooFFXBbmBarl FooFFXBbmBarl341;
FooFFXBbmBarl158*FooFFXBbmBarl430;}FooFFXBbmBarl160;
FooFFXBbmBarb FooFFXBbmBarc{FooFFXBbmBara FooFFXBbmBarl439;
FooFFXBbmBara FooFFXBbmBarl395;FooFFXBbmBara
FooFFXBbmBarl317;}FooFFXBbmBarl159;FooFFXBbmBarb
FooFFXBbmBarc{FooFFXBbmBara FooFFXBbmBarl391;FooFFXBbmBara
FooFFXBbmBarl331;FooFFXBbmBara FooFFXBbmBarl396;
FooFFXBbmBara FooFFXBbmBarl424;FooFFXBbmBara
FooFFXBbmBarl441;FooFFXBbmBara FooFFXBbmBarl316;
FooFFXBbmBara FooFFXBbmBarl405;FooFFXBbmBara
FooFFXBbmBarl333;FooFFXBbmBara FooFFXBbmBarl423;
FooFFXBbmBara FooFFXBbmBarl379;FooFFXBbmBara
FooFFXBbmBarl337;FooFFXBbmBarl159*FooFFXBbmBarl387;
#if FFXCONF_NORSUPPORT || FFXCONF_ISWFSUPPORT
FooFFXBbmBara FooFFXBbmBarl348;FooFFXBbmBara
FooFFXBbmBarl313;FooFFXBbmBara FooFFXBbmBarl336;
FooFFXBbmBara FooFFXBbmBarl361;FooFFXBbmBara
FooFFXBbmBarl329;FooFFXBbmBara FooFFXBbmBarl362;
#endif
#if FFXCONF_NANDSUPPORT
FooFFXBbmBara FooFFXBbmBarl446;FooFFXBbmBara
FooFFXBbmBarl338;FooFFXBbmBara FooFFXBbmBarl356;
FooFFXBbmBara FooFFXBbmBarl408;FooFFXBbmBara
FooFFXBbmBarl369;FooFFXBbmBara FooFFXBbmBarl320;
FooFFXBbmBara FooFFXBbmBarl431;FooFFXBbmBara
FooFFXBbmBarl330;FooFFXBbmBara FooFFXBbmBarl384;
FooFFXBbmBara FooFFXBbmBarl359;FooFFXBbmBara
FooFFXBbmBarl355;FooFFXBbmBara FooFFXBbmBarl364;
FooFFXBbmBara FooFFXBbmBarl421;FooFFXBbmBara
FooFFXBbmBarl321;FooFFXBbmBara FooFFXBbmBarl438;
FooFFXBbmBara FooFFXBbmBarl349;FooFFXBbmBara
FooFFXBbmBarl404;FooFFXBbmBara FooFFXBbmBarl390;
FooFFXBbmBara FooFFXBbmBarl449;FooFFXBbmBara
FooFFXBbmBarl420;FooFFXBbmBara FooFFXBbmBarl363;
FooFFXBbmBara FooFFXBbmBarl409;FooFFXBbmBara
FooFFXBbmBarl340;FooFFXBbmBara FooFFXBbmBarl385;
FooFFXBbmBara FooFFXBbmBarl354;FooFFXBbmBara
FooFFXBbmBarl448;FooFFXBbmBara FooFFXBbmBarl312;
FooFFXBbmBara FooFFXBbmBarl398;FooFFXBbmBara
FooFFXBbmBarl457;FooFFXBbmBara FooFFXBbmBarl410;
FooFFXBbmBara FooFFXBbmBarl419;FooFFXBbmBara
FooFFXBbmBarl393;FooFFXBbmBara FooFFXBbmBarl378;
FooFFXBbmBara FooFFXBbmBarl307;FooFFXBbmBara
FooFFXBbmBarl413;FooFFXBbmBara FooFFXBbmBarl374;
FooFFXBbmBara FooFFXBbmBarl444;FooFFXBbmBara
FooFFXBbmBarl344;FooFFXBbmBara FooFFXBbmBarl389;
FooFFXBbmBara FooFFXBbmBarl392;FooFFXBbmBara
FooFFXBbmBarl388;FooFFXBbmBara FooFFXBbmBarl366;
#endif
}FooFFXBbmBarl156;FooFFXBbmBarb FooFFXBbmBarc{FooFFXBbmBara
FooFFXBbmBarl259;FooFFXBbmBara FooFFXBbmBarl64;
FooFFXBbmBara FooFFXBbmBarl267;FooFFXBbmBara
FooFFXBbmBarl262;FooFFXBbmBara FooFFXBbmBarl265;
FooFFXBbmBara FooFFXBbmBarl240;FooFFXBbmBara
FooFFXBbmBarl243;FooFFXBbmBara FooFFXBbmBarl115;
FooFFXBbmBara FooFFXBbmBarl123;FooFFXBbmBara
FooFFXBbmBarl213;}FooFFXBbmBarl93;FooFFXBbmBard
FooFFXBbmBarl453(FooFFXBbmBari FooFFXBbmBarl151* FooFFXBbmBarl66,FooFFXBbmBarm FooFFXBbmBarl48);
FooFFXBbmBard FooFFXBbmBarl371(FooFFXBbmBari FooFFXBbmBarl153*FooFFXBbmBarl66,FooFFXBbmBarm FooFFXBbmBarl48);FooFFXBbmBard FooFFXBbmBarl370( FooFFXBbmBari FooFFXBbmBarl149*FooFFXBbmBarl402, FooFFXBbmBarm FooFFXBbmBarl48);FooFFXBbmBard
FooFFXBbmBarl315(FooFFXBbmBari FooFFXBbmBarl160* FooFFXBbmBarl325,FooFFXBbmBarm FooFFXBbmBarl48);
FooFFXBbmBard FooFFXBbmBarl414(FooFFXBbmBari FooFFXBbmBarl156*FooFFXBbmBarl66,FooFFXBbmBarm FooFFXBbmBarl48);FooFFXBbmBard FooFFXBbmBarl416( FooFFXBbmBari FooFFXBbmBarl93*FooFFXBbmBarl66,FooFFXBbmBarm FooFFXBbmBarl48);
#endif
#endif
FooFFXBbmBarl42{FooFFXBbmBarl294,FooFFXBbmBarl297,
FooFFXBbmBarl298,FooFFXBbmBarl300,FooFFXBbmBarl305,
FooFFXBbmBarl302};FooFFXBbmBarl42{FooFFXBbmBarl235,
FooFFXBbmBarl211,FooFFXBbmBarl200,FooFFXBbmBarl257};
FooFFXBbmBarb FooFFXBbmBarl42{FooFFXBbmBarl193=0x00,
FooFFXBbmBarl189=0x01,FooFFXBbmBarl85=0x02,FooFFXBbmBarl124
=0x03,FooFFXBbmBarl291=0x08,FooFFXBbmBarl241,
FooFFXBbmBarl258,FooFFXBbmBarl190=0x0F}FooFFXBbmBarl513;
FooFFXBbmBarb FooFFXBbmBarc FooFFXBbmBarl103*FooFFXBbmBaro;
FooFFXBbmBarb FooFFXBbmBarc FooFFXBbmBarl167 FooFFXBbmBarl33
;FooFFXBbmBarb FooFFXBbmBarc FooFFXBbmBarl451
FooFFXBbmBarl51;FooFFXBbmBarb FooFFXBbmBarc{FooFFXBbmBara
FooFFXBbmBarl68;FooFFXBbmBara FooFFXBbmBarl111;
FooFFXBbmBara FooFFXBbmBarl64;FooFFXBbmBarl51*
FooFFXBbmBarl141;FooFFXBbmBarl28*FooFFXBbmBarl26;
FooFFXBbmBarl43( *FooFFXBbmBarl280)(FooFFXBbmBarl51* , FooFFXBbmBara,FooFFXBbmBarl28* );FooFFXBbmBarl43( * FooFFXBbmBarl293)(FooFFXBbmBarl51* ,FooFFXBbmBara, FooFFXBbmBarl28* );FooFFXBbmBarl43( *FooFFXBbmBarl289)( FooFFXBbmBarl51* ,FooFFXBbmBara);}FooFFXBbmBarl70;
FooFFXBbmBarb FooFFXBbmBarc{FooFFXBbmBara FooFFXBbmBarl46;
FooFFXBbmBara FooFFXBbmBarl60;}FooFFXBbmBarl78;
FooFFXBbmBarb FooFFXBbmBarc{FooFFXBbmBarl FooFFXBbmBarl37:4
;FooFFXBbmBarl FooFFXBbmBarl29:28;}FooFFXBbmBarl49;
FooFFXBbmBarb FooFFXBbmBarc{FooFFXBbmBarl49 FooFFXBbmBarl34
;FooFFXBbmBarl49 FooFFXBbmBarl39;}FooFFXBbmBart;
FooFFXBbmBarb FooFFXBbmBarc{FooFFXBbmBarl FooFFXBbmBarl192;
FooFFXBbmBara FooFFXBbmBarr;FooFFXBbmBarl51*
FooFFXBbmBarl301;}FooFFXBbmBarl91;FooFFXBbmBarb
FooFFXBbmBard( *FooFFXBbmBarl86)(FooFFXBbmBarl91* );
FooFFXBbmBarl42{FooFFXBbmBarl244,FooFFXBbmBarl286,
FooFFXBbmBarl288,FooFFXBbmBarl292,FooFFXBbmBarl274,
FooFFXBbmBarl299};FFXSTATUS FooFFXBbmBarl144( FooFFXBbmBarl70*FooFFXBbmBarl238,FooFFXBbmBaro* FooFFXBbmBarl121);FFXSTATUS FooFFXBbmBarl146(FooFFXBbmBaro* FooFFXBbmBarl121);FFXSTATUS FooFFXBbmBarl148(FooFFXBbmBaro FooFFXBbmBarl98);FFXSTATUS FooFFXBbmBarl92(FooFFXBbmBaro FooFFXBbmBarx,FooFFXBbmBarl78*FooFFXBbmBarl230, FooFFXBbmBarm FooFFXBbmBarl281);FFXSTATUS FooFFXBbmBarl138( FooFFXBbmBaro FooFFXBbmBarx);FFXSTATUS FooFFXBbmBarl195( FooFFXBbmBaro FooFFXBbmBarx,FooFFXBbmBara*FooFFXBbmBarl268);
FFXSTATUS FooFFXBbmBarl113(FooFFXBbmBaro FooFFXBbmBarx, FooFFXBbmBara FooFFXBbmBarr,FooFFXBbmBart*FooFFXBbmBarl55);
FFXSTATUS FooFFXBbmBarl128(FooFFXBbmBaro FooFFXBbmBarx, FooFFXBbmBart*FooFFXBbmBarl55);FFXSTATUS FooFFXBbmBarl139( FooFFXBbmBaro FooFFXBbmBarx,FooFFXBbmBart*FooFFXBbmBarl55, FooFFXBbmBarl FooFFXBbmBarl37,FooFFXBbmBara FooFFXBbmBarl84 );FFXSTATUS FooFFXBbmBarl245(FooFFXBbmBaro FooFFXBbmBarx, FooFFXBbmBart*FooFFXBbmBarl55);FFXSTATUS FooFFXBbmBarl225( FooFFXBbmBaro FooFFXBbmBarx,FooFFXBbmBara FooFFXBbmBarr);
FFXSTATUS FooFFXBbmBarl251(FooFFXBbmBaro FooFFXBbmBarx, FooFFXBbmBarl FooFFXBbmBarl37,FooFFXBbmBart*FooFFXBbmBarl47 ,FooFFXBbmBara*FooFFXBbmBarl277,FooFFXBbmBara* FooFFXBbmBarl112);FFXSTATUS FooFFXBbmBarl137(FooFFXBbmBaro FooFFXBbmBarx,FooFFXBbmBara FooFFXBbmBarl253,FooFFXBbmBara FooFFXBbmBarl117,FooFFXBbmBara*FooFFXBbmBarl120);FFXSTATUS
FooFFXBbmBarl237(FooFFXBbmBarl28*FooFFXBbmBarl26, FooFFXBbmBara FooFFXBbmBarl50,FooFFXBbmBarl33*FooFFXBbmBarv );FFXSTATUS FooFFXBbmBarl282(FooFFXBbmBarl28* FooFFXBbmBarl26,FooFFXBbmBara FooFFXBbmBarl50, FooFFXBbmBarl33*FooFFXBbmBarv);FFXSTATUS FooFFXBbmBarl306( FooFFXBbmBarl28*FooFFXBbmBarl26,FooFFXBbmBara FooFFXBbmBarl50,FooFFXBbmBarl49*FooFFXBbmBarl83);FFXSTATUS
FooFFXBbmBarl372(FooFFXBbmBarl28*FooFFXBbmBarl26, FooFFXBbmBara FooFFXBbmBarl50,FooFFXBbmBarl49* FooFFXBbmBarl83);FooFFXBbmBard FooFFXBbmBarl248( FooFFXBbmBarl86*FooFFXBbmBarl287,FooFFXBbmBarl86* FooFFXBbmBarl296);
#if FFXCONF_STATS_BBM
FFXSTATUS FooFFXBbmBarl204(FooFFXBbmBaro FooFFXBbmBarx, FooFFXBbmBarl93*FooFFXBbmBarl208);
#endif
#if DCLCONF_OUTPUT_ENABLED
FooFFXBbmBard FooFFXBbmBarl440(FooFFXBbmBarl33* FooFFXBbmBarv);FooFFXBbmBard FooFFXBbmBarl278( FooFFXBbmBarl49*FooFFXBbmBarl83);FooFFXBbmBard
FooFFXBbmBarl415(FooFFXBbmBart*FooFFXBbmBarl59);
FooFFXBbmBard FooFFXBbmBarl418(FooFFXBbmBarl28* FooFFXBbmBarl26,FooFFXBbmBara FooFFXBbmBarl50);
#endif
FooFFXBbmBarb FooFFXBbmBarl42 FooFFXBbmBarl1301{
FooFFXBbmBarl1191=0,FooFFXBbmBarl1184,FooFFXBbmBarl1283,
FooFFXBbmBarl1295,FooFFXBbmBarl1335,FooFFXBbmBarl1359,
FooFFXBbmBarl1299,FooFFXBbmBarl1231,FooFFXBbmBarl1368}
FooFFXBbmBarl770;FooFFXBbmBarb FooFFXBbmBarl42
FooFFXBbmBarl1465{FooFFXBbmBarl1339=0,FooFFXBbmBarl1407,
FooFFXBbmBarl1292,FooFFXBbmBarl1287}FooFFXBbmBarl709;
FooFFXBbmBarb FooFFXBbmBarl42 FooFFXBbmBarl1417{
FooFFXBbmBarl1276=0,FooFFXBbmBarl1207,FooFFXBbmBarl1213,
FooFFXBbmBarl733,FooFFXBbmBarl1477,FooFFXBbmBarl1320,
FooFFXBbmBarl1357}FooFFXBbmBarl664;FooFFXBbmBarc
FooFFXBbmBarl758;FooFFXBbmBarb FooFFXBbmBarc
FooFFXBbmBarl758 FooFFXBbmBarl665;FooFFXBbmBara
FooFFXBbmBarl1354(FooFFXBbmBarl215 FooFFXBbmBarl661);
FooFFXBbmBara FooFFXBbmBarl1444(FooFFXBbmBarl43* FooFFXBbmBarl250);FooFFXBbmBard FooFFXBbmBarl1227( FooFFXBbmBara FooFFXBbmBarl1401,FooFFXBbmBarl43* FooFFXBbmBarl250);FooFFXBbmBarl664 FooFFXBbmBarl783( FooFFXBbmBarl43*FooFFXBbmBarl250);FooFFXBbmBard
FooFFXBbmBarl1092(FooFFXBbmBarl664 FooFFXBbmBarl1437, FooFFXBbmBarl43*FooFFXBbmBarl250);FooFFXBbmBard
FooFFXBbmBarl1210(FooFFXBbmBarl709 FooFFXBbmBarl1404, FooFFXBbmBarl43*FooFFXBbmBarl250);FooFFXBbmBarl709
FooFFXBbmBarl1448(FooFFXBbmBarl43*FooFFXBbmBarl250);
FooFFXBbmBard FooFFXBbmBarl1358(FooFFXBbmBarl215 FooFFXBbmBarl661,FooFFXBbmBarl665*FooFFXBbmBarl764, FooFFXBbmBarl43*FooFFXBbmBarl250,FooFFXBbmBara FooFFXBbmBarl1293,FooFFXBbmBara FooFFXBbmBarl747);
FooFFXBbmBard FooFFXBbmBarl762(FooFFXBbmBarl43* FooFFXBbmBarl250);FooFFXBbmBarl665*FooFFXBbmBarl1390( FooFFXBbmBarl215 FooFFXBbmBarl661);FooFFXBbmBard
FooFFXBbmBarl1233(FooFFXBbmBarl665*FooFFXBbmBarl764);
FooFFXBbmBarl664 FooFFXBbmBarl1408(FooFFXBbmBarl770 FooFFXBbmBarh,FooFFXBbmBarl43*FooFFXBbmBarl250, FooFFXBbmBarl215 FooFFXBbmBarl661);
#define FooFFXBbmBarl399( FooFFXBbmBarl109) ((((   FooFFXBbmBarl109) >> 7) & 1) == 1 ? FALSE : TRUE)
FooFFXBbmBarb FooFFXBbmBarc{FooFFXBbmBarl FooFFXBbmBarl252;
FooFFXBbmBarl FooFFXBbmBarl54;FooFFXBbmBart*FooFFXBbmBarl47
;FooFFXBbmBarm FooFFXBbmBarl266;}FooFFXBbmBarl38;
#if FFXCONF_STATS_BBM
FooFFXBbmBarb FooFFXBbmBarc{FooFFXBbmBara FooFFXBbmBarl115;
FooFFXBbmBara FooFFXBbmBarl123;}FooFFXBbmBarl172;
#endif
FooFFXBbmBarc FooFFXBbmBarl167{FooFFXBbmBara
FooFFXBbmBarl126;FooFFXBbmBara FooFFXBbmBarl249;
FooFFXBbmBara FooFFXBbmBarl118;FooFFXBbmBara
FooFFXBbmBarl227;FooFFXBbmBara FooFFXBbmBarl202;
FooFFXBbmBara FooFFXBbmBarl218;FooFFXBbmBara FooFFXBbmBarl46
;FooFFXBbmBara FooFFXBbmBarl60;FooFFXBbmBara
FooFFXBbmBarl114;FooFFXBbmBara FooFFXBbmBarl233;
FooFFXBbmBara FooFFXBbmBarl273;FooFFXBbmBara
FooFFXBbmBarl203;};FooFFXBbmBarb FooFFXBbmBarc
FooFFXBbmBarl103{FooFFXBbmBara FooFFXBbmBarl126;PDCLMUTEX
FooFFXBbmBarl132;FooFFXBbmBarl70 FooFFXBbmBarl27;
FooFFXBbmBarm FooFFXBbmBarl205;FooFFXBbmBara
FooFFXBbmBarl118;FooFFXBbmBara FooFFXBbmBarl114;
FooFFXBbmBarl78 FooFFXBbmBarl32;FooFFXBbmBara
FooFFXBbmBarl89;FooFFXBbmBara FooFFXBbmBarl194;
FooFFXBbmBarl33 FooFFXBbmBarl134;FooFFXBbmBarm
FooFFXBbmBarl254;FooFFXBbmBarl38 FooFFXBbmBarl56;
FooFFXBbmBart FooFFXBbmBarl131;
#if FFXCONF_STATS_BBM
FooFFXBbmBarl172 FooFFXBbmBarl108;
#endif
FooFFXBbmBarc FooFFXBbmBarl103*FooFFXBbmBarl221;}
FooFFXBbmBarz;FFXSTATUS FooFFXBbmBarl276(FooFFXBbmBarz* FooFFXBbmBarg,FooFFXBbmBara FooFFXBbmBarr,FooFFXBbmBarl33* FooFFXBbmBarv);FFXSTATUS FooFFXBbmBarl304(FooFFXBbmBarz* FooFFXBbmBarg,FooFFXBbmBarl38*FooFFXBbmBarl35);
FooFFXBbmBarl28*FooFFXBbmBarl295(FooFFXBbmBarz* FooFFXBbmBarg,FooFFXBbmBarl33*FooFFXBbmBarv,FooFFXBbmBarl28  *FooFFXBbmBarl26);FFXSTATUS FooFFXBbmBarl303(FooFFXBbmBarz  *FooFFXBbmBarg,FooFFXBbmBarl33*FooFFXBbmBarv,FooFFXBbmBara FooFFXBbmBarl89);FFXSTATUS FooFFXBbmBarl275(FooFFXBbmBarz* FooFFXBbmBarg,FooFFXBbmBara FooFFXBbmBarr);FooFFXBbmBard
FooFFXBbmBarl290(FooFFXBbmBarz*FooFFXBbmBarg, FooFFXBbmBarl33*FooFFXBbmBarv);FFXSTATUS FooFFXBbmBarl263( FooFFXBbmBarz*FooFFXBbmBarg);FFXSTATUS FooFFXBbmBarl580( FooFFXBbmBarz*FooFFXBbmBarg);FFXSTATUS FooFFXBbmBarl269( FooFFXBbmBarl38*FooFFXBbmBarl35,FooFFXBbmBarl FooFFXBbmBarl54);FFXSTATUS FooFFXBbmBarl256(FooFFXBbmBarl38  *FooFFXBbmBarl35);FFXSTATUS FooFFXBbmBarl191( FooFFXBbmBarl38*FooFFXBbmBarl35,FooFFXBbmBart* FooFFXBbmBarl59);FFXSTATUS FooFFXBbmBarl198(FooFFXBbmBarl38  *FooFFXBbmBarl35,FooFFXBbmBart*FooFFXBbmBarl59);FFXSTATUS
FooFFXBbmBarl116(FooFFXBbmBarl38*FooFFXBbmBarl35, FooFFXBbmBara FooFFXBbmBarl84,FooFFXBbmBarl FooFFXBbmBarl147 ,FooFFXBbmBart*FooFFXBbmBarl59,FooFFXBbmBara* FooFFXBbmBarl112);FooFFXBbmBard FooFFXBbmBarl283( FooFFXBbmBarz*FooFFXBbmBarg);FooFFXBbmBard FooFFXBbmBarl261
(FooFFXBbmBaro FooFFXBbmBarp,FooFFXBbmBarl91* FooFFXBbmBarl212);FooFFXBbmBarl130 FooFFXBbmBard
FooFFXBbmBarl1396(FooFFXBbmBarz*FooFFXBbmBarg,FooFFXBbmBari FooFFXBbmBarl33*FooFFXBbmBarv,FooFFXBbmBara FooFFXBbmBarl89 );FooFFXBbmBarl130 FFXSTATUS FooFFXBbmBarl1290( FooFFXBbmBarz*FooFFXBbmBarg,FooFFXBbmBarl33*FooFFXBbmBarv, FooFFXBbmBara*FooFFXBbmBarl774);FooFFXBbmBarl130 FFXSTATUS
FooFFXBbmBarl1091(FooFFXBbmBarz*FooFFXBbmBarg,FooFFXBbmBara FooFFXBbmBarr,FooFFXBbmBarl33*FooFFXBbmBarv);
FooFFXBbmBarl130 FFXSTATUS FooFFXBbmBarl1217(FooFFXBbmBarz* FooFFXBbmBarg,FooFFXBbmBarl33*FooFFXBbmBarv);FFXSTATUS
FooFFXBbmBarl92(FooFFXBbmBaro FooFFXBbmBarx,FooFFXBbmBarl78  *FooFFXBbmBarl230,FooFFXBbmBarm FooFFXBbmBarl281){
FFXSTATUS FooFFXBbmBarw;FooFFXBbmBarz*FooFFXBbmBarg;
FooFFXBbmBarl33 FooFFXBbmBarl134;FooFFXBbmBara
FooFFXBbmBarl89;FooFFXBbmBarm FooFFXBbmBarl145;
FooFFXBbmBarl91 FooFFXBbmBarl622;FFXTRACEPRINTF(( FooFFXBbmBarl239(FFXTRACE_BBM,1,FooFFXBbmBarl643),"\x46\x66\x78\x42\x62\x6d\x35\x4d\x6f\x75\x6e\x74\x28\x29\n"));
FooFFXBbmBarl634("\x46\x66\x78\x42\x62\x6d\x35\x4d\x6f\x75\x6e\x74",0,0);FooFFXBbmBarg=FooFFXBbmBarl30;
FooFFXBbmBarl145=FALSE;FooFFXBbmBare(FooFFXBbmBarl148( FooFFXBbmBarx)!=FooFFXBbmBarn){FooFFXBbmBarw=
FooFFXBbmBarl163;FooFFXBbmBarj FooFFXBbmBarl677;}
FooFFXBbmBarg=(FooFFXBbmBarz* )FooFFXBbmBarx;FooFFXBbmBare( !FooFFXBbmBarl230){FooFFXBbmBarw=FooFFXBbmBarl140;
FooFFXBbmBarj FooFFXBbmBarl677;}FooFFXBbmBarl645( FooFFXBbmBarg->FooFFXBbmBarl132);FooFFXBbmBarl145=TRUE;
FooFFXBbmBare(FooFFXBbmBarg->FooFFXBbmBarl205){
FooFFXBbmBarw=(FooFFXBbmBarl57(15,12));FooFFXBbmBarj
FooFFXBbmBarl677;}FooFFXBbmBare(FooFFXBbmBarl230-> FooFFXBbmBarl46==0||(FooFFXBbmBarg->FooFFXBbmBarl27. FooFFXBbmBarl64!=(FooFFXBbmBarl230->FooFFXBbmBarl60+ FooFFXBbmBarl230->FooFFXBbmBarl46))){FooFFXBbmBarw=( FooFFXBbmBarl57(15,11));FooFFXBbmBarj FooFFXBbmBarl677;}
FooFFXBbmBarl426(&FooFFXBbmBarg->FooFFXBbmBarl32, FooFFXBbmBarl230,FooFFXBbmBarl82 FooFFXBbmBarg-> FooFFXBbmBarl32);FooFFXBbmBarw=FooFFXBbmBarl1290( FooFFXBbmBarg,&FooFFXBbmBarl134,&FooFFXBbmBarl89);
FooFFXBbmBare(FooFFXBbmBarw!=FooFFXBbmBarn)FooFFXBbmBarj
FooFFXBbmBarl677;FooFFXBbmBare(FooFFXBbmBarl281){
FooFFXBbmBare((FooFFXBbmBarl134.FooFFXBbmBarl60!= FooFFXBbmBarl230->FooFFXBbmBarl60)||(FooFFXBbmBarl134. FooFFXBbmBarl46!=FooFFXBbmBarl230->FooFFXBbmBarl46)){
FooFFXBbmBarw=(FooFFXBbmBarl57(15,11));FooFFXBbmBarj
FooFFXBbmBarl677;}}FooFFXBbmBarl122{FooFFXBbmBare(( FooFFXBbmBarl134.FooFFXBbmBarl60+FooFFXBbmBarl134. FooFFXBbmBarl46)!=FooFFXBbmBarg->FooFFXBbmBarl27. FooFFXBbmBarl64){FooFFXBbmBarw=(FooFFXBbmBarl57(15,11));
FooFFXBbmBarj FooFFXBbmBarl677;}}FooFFXBbmBare(( FooFFXBbmBarg->FooFFXBbmBarl27.FooFFXBbmBarl68>( FooFFXBbmBara)(1<<(((FooFFXBbmBarl134.FooFFXBbmBarl202)>>16 )&0x0000001F)))||(FooFFXBbmBarg->FooFFXBbmBarl27. FooFFXBbmBarl111>(FooFFXBbmBara)((FooFFXBbmBara)(1<<((( FooFFXBbmBarl134.FooFFXBbmBarl202)>>16)&0x0000001F)) * (( FooFFXBbmBarl134.FooFFXBbmBarl202)&0x0000FFFF)))){
FooFFXBbmBarw=(FooFFXBbmBarl57(15,11));FooFFXBbmBarj
FooFFXBbmBarl677;}FooFFXBbmBarl1396(FooFFXBbmBarg,& FooFFXBbmBarl134,FooFFXBbmBarl89);FooFFXBbmBare(! FooFFXBbmBarl281){FooFFXBbmBarl230->FooFFXBbmBarl46=
FooFFXBbmBarl134.FooFFXBbmBarl46;FooFFXBbmBarl230->
FooFFXBbmBarl60=FooFFXBbmBarl134.FooFFXBbmBarl60;}
FooFFXBbmBarw=FooFFXBbmBarl269(&FooFFXBbmBarg-> FooFFXBbmBarl56,FooFFXBbmBarg->FooFFXBbmBarl32. FooFFXBbmBarl60);FooFFXBbmBare(FooFFXBbmBarw!=FooFFXBbmBarn )FooFFXBbmBarj FooFFXBbmBarl677;FooFFXBbmBarg->
FooFFXBbmBarl254=FALSE;FooFFXBbmBarw=FooFFXBbmBarl304( FooFFXBbmBarg,&FooFFXBbmBarg->FooFFXBbmBarl56);
FooFFXBbmBare(FooFFXBbmBarw!=FooFFXBbmBarn)FooFFXBbmBarj
FooFFXBbmBarl677;{(&FooFFXBbmBarg->FooFFXBbmBarl131)->
FooFFXBbmBarl34.FooFFXBbmBarl37=FooFFXBbmBarl190;(& FooFFXBbmBarg->FooFFXBbmBarl131)->FooFFXBbmBarl34.
FooFFXBbmBarl29=(0x0FFF9000);(&FooFFXBbmBarg-> FooFFXBbmBarl131)->FooFFXBbmBarl39.FooFFXBbmBarl37=
FooFFXBbmBarl190;(&FooFFXBbmBarg->FooFFXBbmBarl131)->
FooFFXBbmBarl39.FooFFXBbmBarl29=(0x0FFF9000);};
FooFFXBbmBare(FooFFXBbmBarg->FooFFXBbmBarl254){
FooFFXBbmBarl622.FooFFXBbmBarl192=FooFFXBbmBarl299;
FooFFXBbmBarl622.FooFFXBbmBarr=FooFFXBbmBarl89;
FooFFXBbmBarl261(FooFFXBbmBarg,&FooFFXBbmBarl622);
FooFFXBbmBarg->FooFFXBbmBarl254=FALSE;FooFFXBbmBarl263( FooFFXBbmBarg);}FooFFXBbmBarl677:FooFFXBbmBare( FooFFXBbmBarw!=FooFFXBbmBarn){FooFFXBbmBare(FooFFXBbmBarw!= FooFFXBbmBarl163&&FooFFXBbmBarw!=FooFFXBbmBarl140){
FooFFXBbmBarl283(FooFFXBbmBarg);}}FooFFXBbmBarl122{
FooFFXBbmBarl622.FooFFXBbmBarl192=FooFFXBbmBarl288;
FooFFXBbmBarl622.FooFFXBbmBarr=FooFFXBbmBarg->
FooFFXBbmBarl89;FooFFXBbmBarl261(FooFFXBbmBarg,& FooFFXBbmBarl622);FooFFXBbmBarg->FooFFXBbmBarl205=TRUE;}
FooFFXBbmBare(FooFFXBbmBarl145)FooFFXBbmBarl646( FooFFXBbmBarg->FooFFXBbmBarl132);FooFFXBbmBarl635(0UL);
FFXTRACEPRINTF((FooFFXBbmBarl239(FFXTRACE_BBM,1, FooFFXBbmBarl644),"\x46\x66\x78\x42\x62\x6d\x35\x4d\x6f\x75\x6e\x74\x28\x29\x20\x72\x65\x74\x75\x72\x6e\x69\x6e\x67\x20\x73\x74\x61\x74\x75\x73\x3d\x25\x6c\x58\n", FooFFXBbmBarw));FooFFXBbmBarl52 FooFFXBbmBarw;}FFXSTATUS
FooFFXBbmBarl138(FooFFXBbmBaro FooFFXBbmBarx){FFXSTATUS
FooFFXBbmBarw;FooFFXBbmBarz*FooFFXBbmBarg;FooFFXBbmBarm
FooFFXBbmBarl145;FFXTRACEPRINTF((FooFFXBbmBarl239( FFXTRACE_BBM,1,FooFFXBbmBarl643),"\x46\x66\x78\x42\x62\x6d\x35\x55\x6e\x6d\x6f\x75\x6e\x74\x28\x29\n"));
FooFFXBbmBarl634("\x46\x66\x78\x42\x62\x6d\x35\x55\x6e\x6d\x6f\x75\x6e\x74",0,0);FooFFXBbmBarg=FooFFXBbmBarl30;
FooFFXBbmBarl145=FALSE;FooFFXBbmBare(FooFFXBbmBarl148( FooFFXBbmBarx)!=FooFFXBbmBarn){FooFFXBbmBarw=
FooFFXBbmBarl163;FooFFXBbmBarj FooFFXBbmBarl1199;}
FooFFXBbmBarg=(FooFFXBbmBarz* )FooFFXBbmBarx;
FooFFXBbmBarl645(FooFFXBbmBarg->FooFFXBbmBarl132);
FooFFXBbmBarl145=TRUE;FooFFXBbmBare(!FooFFXBbmBarg-> FooFFXBbmBarl205){FooFFXBbmBarw=(FooFFXBbmBarl57(15,4));
FooFFXBbmBarj FooFFXBbmBarl1199;}FooFFXBbmBarl283( FooFFXBbmBarg);FooFFXBbmBarw=FooFFXBbmBarn;
FooFFXBbmBarl1199:FooFFXBbmBare(FooFFXBbmBarl145)FooFFXBbmBarl646
(FooFFXBbmBarg->FooFFXBbmBarl132);FooFFXBbmBarl635(0UL);
FFXTRACEPRINTF((FooFFXBbmBarl239(FFXTRACE_BBM,1, FooFFXBbmBarl644),"\x46\x66\x78\x42\x62\x6d\x35\x55\x6e\x6d\x6f\x75\x6e\x74\x28\x29\x20\x72\x65\x74\x75\x72\x6e\x69\x6e\x67\x20\x73\x74\x61\x74\x75\x73\x3d\x25\x6c\x58\n", FooFFXBbmBarw));FooFFXBbmBarl52 FooFFXBbmBarw;}
FooFFXBbmBard FooFFXBbmBarl283(FooFFXBbmBarz*FooFFXBbmBarg){
FooFFXBbmBarl53(410,FooFFXBbmBarg);FooFFXBbmBarg->
FooFFXBbmBarl205=FALSE;FooFFXBbmBarg->FooFFXBbmBarl32.
FooFFXBbmBarl46=0;FooFFXBbmBarg->FooFFXBbmBarl32.
FooFFXBbmBarl60=0;FooFFXBbmBarg->FooFFXBbmBarl89=0;
FooFFXBbmBarg->FooFFXBbmBarl194=0;FooFFXBbmBarg->
FooFFXBbmBarl114=0;FooFFXBbmBarg->FooFFXBbmBarl118=0;
FooFFXBbmBarl232(&FooFFXBbmBarg->FooFFXBbmBarl134,0x00, FooFFXBbmBarl82 FooFFXBbmBarg->FooFFXBbmBarl134);
#if FFXCONF_STATS_BBM
FooFFXBbmBarg->FooFFXBbmBarl108.FooFFXBbmBarl123=0;
FooFFXBbmBarg->FooFFXBbmBarl108.FooFFXBbmBarl115=0;
#endif
{(&FooFFXBbmBarg->FooFFXBbmBarl131)->FooFFXBbmBarl34.
FooFFXBbmBarl37=FooFFXBbmBarl190;(&FooFFXBbmBarg-> FooFFXBbmBarl131)->FooFFXBbmBarl34.FooFFXBbmBarl29=( 0x0FFF9000);(&FooFFXBbmBarg->FooFFXBbmBarl131)->
FooFFXBbmBarl39.FooFFXBbmBarl37=FooFFXBbmBarl190;(& FooFFXBbmBarg->FooFFXBbmBarl131)->FooFFXBbmBarl39.
FooFFXBbmBarl29=(0x0FFF9000);};FooFFXBbmBare(FooFFXBbmBarg ->FooFFXBbmBarl56.FooFFXBbmBarl54){FooFFXBbmBarl256(& FooFFXBbmBarg->FooFFXBbmBarl56);FooFFXBbmBarg->
FooFFXBbmBarl56.FooFFXBbmBarl54=0;}FooFFXBbmBarl52;}
FooFFXBbmBard FooFFXBbmBarl290(FooFFXBbmBarz*FooFFXBbmBarg, FooFFXBbmBarl33*FooFFXBbmBarv){FooFFXBbmBarl53(461, FooFFXBbmBarg);FooFFXBbmBarv->FooFFXBbmBarl126=0x4D424264;
FooFFXBbmBarv->FooFFXBbmBarl249=0x30302E35;FooFFXBbmBarv->
FooFFXBbmBarl227=(((0x30)/4)|((0x00)<<7)|((0x00)<<8)|((0x00 )<<10));FooFFXBbmBarv->FooFFXBbmBarl273=0x00000000;
FooFFXBbmBarv->FooFFXBbmBarl202=0;FooFFXBbmBarv->
FooFFXBbmBarl202|=((FooFFXBbmBarl1175(FooFFXBbmBarg-> FooFFXBbmBarl27.FooFFXBbmBarl68)-1)&0x1f)<<16;FooFFXBbmBarv
->FooFFXBbmBarl202|=((FooFFXBbmBarg->FooFFXBbmBarl27. FooFFXBbmBarl111/FooFFXBbmBarg->FooFFXBbmBarl27. FooFFXBbmBarl68)&0xffff);FooFFXBbmBarv->FooFFXBbmBarl218|=( FooFFXBbmBarg->FooFFXBbmBarl56.FooFFXBbmBarl252)&0xffff;
FooFFXBbmBarv->FooFFXBbmBarl46=FooFFXBbmBarg->
FooFFXBbmBarl32.FooFFXBbmBarl46;FooFFXBbmBarv->
FooFFXBbmBarl60=FooFFXBbmBarg->FooFFXBbmBarl32.
FooFFXBbmBarl60;FooFFXBbmBarv->FooFFXBbmBarl114=
FooFFXBbmBarg->FooFFXBbmBarl114;FooFFXBbmBarv->
FooFFXBbmBarl118=FooFFXBbmBarg->FooFFXBbmBarl118;
FooFFXBbmBarv->FooFFXBbmBarl233=FooFFXBbmBarg->
FooFFXBbmBarl89;FooFFXBbmBarv->FooFFXBbmBarl203=0;
FooFFXBbmBarl52;}FooFFXBbmBarl130 FFXSTATUS
FooFFXBbmBarl1290(FooFFXBbmBarz*FooFFXBbmBarg, FooFFXBbmBarl33*FooFFXBbmBarv,FooFFXBbmBara* FooFFXBbmBarl774){FFXSTATUS FooFFXBbmBarw;FooFFXBbmBara
FooFFXBbmBarl1099;FooFFXBbmBara FooFFXBbmBarl89;
FooFFXBbmBara FooFFXBbmBarr;FooFFXBbmBarl53(526, FooFFXBbmBarg);FooFFXBbmBarl53(527,FooFFXBbmBarv);
FooFFXBbmBarl53(528,FooFFXBbmBarl774);FooFFXBbmBarr=
FooFFXBbmBarg->FooFFXBbmBarl27.FooFFXBbmBarl64-1;
FooFFXBbmBarl89=FooFFXBbmBarr;FooFFXBbmBarl484( FooFFXBbmBarr>=FooFFXBbmBarg->FooFFXBbmBarl32. FooFFXBbmBarl46){FooFFXBbmBarw=FooFFXBbmBarl1091( FooFFXBbmBarg,FooFFXBbmBarr,FooFFXBbmBarv);FooFFXBbmBare( FooFFXBbmBarw==FooFFXBbmBarn){FooFFXBbmBarl89=FooFFXBbmBarr
;FooFFXBbmBarr=FooFFXBbmBarv->FooFFXBbmBarl233;
FooFFXBbmBarl1099=FooFFXBbmBarv->FooFFXBbmBarl114;
FooFFXBbmBarl53(546,FooFFXBbmBarv->FooFFXBbmBarl233>= FooFFXBbmBarg->FooFFXBbmBarl32.FooFFXBbmBarl46);
FooFFXBbmBarw=FooFFXBbmBarl1091(FooFFXBbmBarg,FooFFXBbmBarr ,FooFFXBbmBarv);FooFFXBbmBare(FooFFXBbmBarw==FooFFXBbmBarn &&FooFFXBbmBarv->FooFFXBbmBarl233==FooFFXBbmBarl89){
FooFFXBbmBare(FooFFXBbmBarv->FooFFXBbmBarl114> FooFFXBbmBarl1099){FooFFXBbmBarl89=FooFFXBbmBarr;
FooFFXBbmBarl1099=FooFFXBbmBarv->FooFFXBbmBarl114;}
FooFFXBbmBarl127;}FooFFXBbmBarl122{FooFFXBbmBarl127;}}
FooFFXBbmBarr-=1;}FooFFXBbmBare(FooFFXBbmBarr>= FooFFXBbmBarg->FooFFXBbmBarl32.FooFFXBbmBarl46){ *
FooFFXBbmBarl774=FooFFXBbmBarl89;FooFFXBbmBarw=
FooFFXBbmBarl276(FooFFXBbmBarg, *FooFFXBbmBarl774, FooFFXBbmBarv);FooFFXBbmBarl53(591,FooFFXBbmBarw== FooFFXBbmBarn);}FooFFXBbmBarl122{ *FooFFXBbmBarl774=0;
FooFFXBbmBarw=(FooFFXBbmBarl57(15,1));}FooFFXBbmBarl52
FooFFXBbmBarw;}FooFFXBbmBarl130 FooFFXBbmBard
FooFFXBbmBarl1396(FooFFXBbmBarz*FooFFXBbmBarg,FooFFXBbmBari FooFFXBbmBarl33*FooFFXBbmBarv,FooFFXBbmBara FooFFXBbmBarl89 ){FooFFXBbmBarl53(625,FooFFXBbmBarg);FooFFXBbmBarl53(626, FooFFXBbmBarv);FooFFXBbmBarl53(627,FooFFXBbmBarl89< FooFFXBbmBarg->FooFFXBbmBarl27.FooFFXBbmBarl64);
FooFFXBbmBarl53(628,FooFFXBbmBarl89>=FooFFXBbmBarg-> FooFFXBbmBarl32.FooFFXBbmBarl46);FooFFXBbmBarg->
FooFFXBbmBarl32.FooFFXBbmBarl46=FooFFXBbmBarv->
FooFFXBbmBarl46;FooFFXBbmBarg->FooFFXBbmBarl32.
FooFFXBbmBarl60=FooFFXBbmBarv->FooFFXBbmBarl60;
FooFFXBbmBarg->FooFFXBbmBarl118=FooFFXBbmBarv->
FooFFXBbmBarl118;FooFFXBbmBarg->FooFFXBbmBarl194=
FooFFXBbmBarv->FooFFXBbmBarl233;FooFFXBbmBarg->
FooFFXBbmBarl114=FooFFXBbmBarv->FooFFXBbmBarl114;
FooFFXBbmBarg->FooFFXBbmBarl89=FooFFXBbmBarl89;
FooFFXBbmBarl426(&FooFFXBbmBarg->FooFFXBbmBarl134, FooFFXBbmBarv,FooFFXBbmBarl82 FooFFXBbmBarg-> FooFFXBbmBarl134);}FooFFXBbmBarl130 FFXSTATUS
FooFFXBbmBarl1091(FooFFXBbmBarz*FooFFXBbmBarg,FooFFXBbmBara FooFFXBbmBarr,FooFFXBbmBarl33*FooFFXBbmBarv){FFXSTATUS
FooFFXBbmBarw;FooFFXBbmBarl43 FooFFXBbmBarl216;
FooFFXBbmBara FooFFXBbmBarl1214;FooFFXBbmBara
FooFFXBbmBarl841;FooFFXBbmBara FooFFXBbmBarl718;
FooFFXBbmBara FooFFXBbmBarl852;FooFFXBbmBara FooFFXBbmBarl68
;FooFFXBbmBara FooFFXBbmBarl648;FooFFXBbmBarl53(678, FooFFXBbmBarg);FooFFXBbmBare((FooFFXBbmBarr<FooFFXBbmBarg-> FooFFXBbmBarl32.FooFFXBbmBarl46)||(FooFFXBbmBarr>= FooFFXBbmBarg->FooFFXBbmBarl27.FooFFXBbmBarl64))FooFFXBbmBarl52
FooFFXBbmBarl210;FooFFXBbmBare(FooFFXBbmBarn!= FooFFXBbmBarl276(FooFFXBbmBarg,FooFFXBbmBarr,FooFFXBbmBarv))FooFFXBbmBarl52
(FooFFXBbmBarl57(15,16));FooFFXBbmBare(FooFFXBbmBarn!= FooFFXBbmBarl1217(FooFFXBbmBarg,FooFFXBbmBarv))FooFFXBbmBarl52
(FooFFXBbmBarl57(15,3));FooFFXBbmBarl718=0x30;
FooFFXBbmBarl718+=(FooFFXBbmBara)(((FooFFXBbmBarv-> FooFFXBbmBarl218)&0x0000FFFF)>>0) * (4);FooFFXBbmBarl53(697 ,FooFFXBbmBarg->FooFFXBbmBarl27.FooFFXBbmBarl68);
FooFFXBbmBarl68=FooFFXBbmBarg->FooFFXBbmBarl27.
FooFFXBbmBarl68;FooFFXBbmBarl648=FooFFXBbmBarg->
FooFFXBbmBarl27.FooFFXBbmBarl111/FooFFXBbmBarl68;
FooFFXBbmBarl648*=FooFFXBbmBarr;FooFFXBbmBarl1214=
FooFFXBbmBarv->FooFFXBbmBarl203;FooFFXBbmBarl232(& FooFFXBbmBarg->FooFFXBbmBarl27.FooFFXBbmBarl26[(0x2C)],0, FooFFXBbmBarl82(FooFFXBbmBarv->FooFFXBbmBarl203));
FooFFXBbmBarl841=0;FooFFXBbmBarl484(FooFFXBbmBarl718){
FooFFXBbmBare(FooFFXBbmBarl718>FooFFXBbmBarl68)FooFFXBbmBarl852
=FooFFXBbmBarl68;FooFFXBbmBarl122 FooFFXBbmBarl852=
FooFFXBbmBarl718;FooFFXBbmBarl841=FooFFXBbmBarl763( FooFFXBbmBarl841,FooFFXBbmBarg->FooFFXBbmBarl27. FooFFXBbmBarl26,FooFFXBbmBarl852);FooFFXBbmBarl718-=
FooFFXBbmBarl852;FooFFXBbmBare(FooFFXBbmBarl718){++
FooFFXBbmBarl648;FooFFXBbmBarl216=FooFFXBbmBarg->
FooFFXBbmBarl27.FooFFXBbmBarl280(FooFFXBbmBarg-> FooFFXBbmBarl27.FooFFXBbmBarl141,FooFFXBbmBarl648, FooFFXBbmBarg->FooFFXBbmBarl27.FooFFXBbmBarl26);
FooFFXBbmBare(FooFFXBbmBarl733==FooFFXBbmBarl783(& FooFFXBbmBarl216)){FooFFXBbmBarg->FooFFXBbmBarl254=TRUE;
FooFFXBbmBarl762(&FooFFXBbmBarl216);}FooFFXBbmBare(!( FooFFXBbmBarl216.FooFFXBbmBarl81==(1)&&FooFFXBbmBarl216. FooFFXBbmBarl223==FooFFXBbmBarn)){FooFFXBbmBarl52
FooFFXBbmBarl216.FooFFXBbmBarl223;}}}FooFFXBbmBare( FooFFXBbmBarl841!=FooFFXBbmBarl1214)FooFFXBbmBarw=( FooFFXBbmBarl57(15,3));FooFFXBbmBarl122 FooFFXBbmBarw=
FooFFXBbmBarn;FooFFXBbmBarl52 FooFFXBbmBarw;}
FooFFXBbmBarl130 FFXSTATUS FooFFXBbmBarl1217(FooFFXBbmBarz* FooFFXBbmBarg,FooFFXBbmBarl33*FooFFXBbmBarv){
FooFFXBbmBarl53(768,FooFFXBbmBarv);FooFFXBbmBare( FooFFXBbmBarv->FooFFXBbmBarl126!=0x4D424264||FooFFXBbmBarv ->FooFFXBbmBarl249!=0x30302E35||FooFFXBbmBarv-> FooFFXBbmBarl227!=(((0x30)/4)|((0x00)<<7)|((0x00)<<8)|(( 0x00)<<10))||FooFFXBbmBarv->FooFFXBbmBarl273!=0x00000000){
FooFFXBbmBarl52(FooFFXBbmBarl57(15,19));}FooFFXBbmBare( FooFFXBbmBarg->FooFFXBbmBarl27.FooFFXBbmBarl111!=( FooFFXBbmBara)((FooFFXBbmBara)(1<<(((FooFFXBbmBarv-> FooFFXBbmBarl202)>>16)&0x0000001F)) * ((FooFFXBbmBarv-> FooFFXBbmBarl202)&0x0000FFFF))||FooFFXBbmBarg-> FooFFXBbmBarl27.FooFFXBbmBarl68!=(FooFFXBbmBara)(1<<((( FooFFXBbmBarv->FooFFXBbmBarl202)>>16)&0x0000001F))){
FooFFXBbmBarl52(FooFFXBbmBarl57(15,11));}FooFFXBbmBare( FooFFXBbmBarv->FooFFXBbmBarl46+FooFFXBbmBarv-> FooFFXBbmBarl60>FooFFXBbmBarg->FooFFXBbmBarl27. FooFFXBbmBarl64){FooFFXBbmBarl52(FooFFXBbmBarl57(15,11));}
FooFFXBbmBare(FooFFXBbmBarv->FooFFXBbmBarl233>= FooFFXBbmBarg->FooFFXBbmBarl27.FooFFXBbmBarl64|| FooFFXBbmBarv->FooFFXBbmBarl233<FooFFXBbmBarg-> FooFFXBbmBarl32.FooFFXBbmBarl46){FooFFXBbmBarl52( FooFFXBbmBarl57(15,19));}FooFFXBbmBarl52 FooFFXBbmBarn;}
