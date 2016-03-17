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
/* COBF by BB -- 'bbm_commit.c' obfuscated at Tue Apr 24 23:20:51 2012
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
(FooFFXBbmBaro FooFFXBbmBarp,FooFFXBbmBarl91* FooFFXBbmBarl212);FFXSTATUS FooFFXBbmBarl139(FooFFXBbmBaro FooFFXBbmBarx,FooFFXBbmBart*FooFFXBbmBarl55,FooFFXBbmBarl FooFFXBbmBarl37,FooFFXBbmBara FooFFXBbmBarr){FFXSTATUS
FooFFXBbmBarw;FooFFXBbmBarz*FooFFXBbmBarg;FooFFXBbmBart
FooFFXBbmBarq;FooFFXBbmBarm FooFFXBbmBarl145;FFXTRACEPRINTF
((FooFFXBbmBarl239(FFXTRACE_BBM,1,FooFFXBbmBarl643),"\x46\x66\x78\x42\x62\x6d\x35\x43\x6f\x6d\x6d\x69\x74\x52\x65\x6d\x61\x70\x28\x29\n"));FooFFXBbmBarl634("\x46\x66\x78\x42\x62\x6d\x35\x43\x6f\x6d\x6d\x69\x74\x52\x65\x6d\x61\x70",0,0);FooFFXBbmBarg=FooFFXBbmBarl30;FooFFXBbmBarl145=
FALSE;FooFFXBbmBare(FooFFXBbmBarl55==FooFFXBbmBarl30){
FooFFXBbmBarw=FooFFXBbmBarl140;FooFFXBbmBarj
FooFFXBbmBarl672;}FooFFXBbmBare(FooFFXBbmBarl148( FooFFXBbmBarx)!=FooFFXBbmBarn){FooFFXBbmBarw=
FooFFXBbmBarl163;FooFFXBbmBarj FooFFXBbmBarl672;}
FooFFXBbmBarg=(FooFFXBbmBarz* )FooFFXBbmBarx;
FooFFXBbmBarl645(FooFFXBbmBarg->FooFFXBbmBarl132);
FooFFXBbmBarl145=TRUE;FooFFXBbmBare(!FooFFXBbmBarg-> FooFFXBbmBarl205){FooFFXBbmBarw=(FooFFXBbmBarl57(15,4));
FooFFXBbmBarj FooFFXBbmBarl672;}FooFFXBbmBare(!(( FooFFXBbmBarl37)==FooFFXBbmBarl193||(FooFFXBbmBarl37)== FooFFXBbmBarl189||(FooFFXBbmBarl37)==FooFFXBbmBarl85||( FooFFXBbmBarl37)==FooFFXBbmBarl124)&&FooFFXBbmBarl37!= FooFFXBbmBarl189){FooFFXBbmBarw=FooFFXBbmBarl140;
FooFFXBbmBarj FooFFXBbmBarl672;}FooFFXBbmBare(FooFFXBbmBarr >=FooFFXBbmBarg->FooFFXBbmBarl27.FooFFXBbmBarl64){
FooFFXBbmBarw=FooFFXBbmBarl210;FooFFXBbmBarj
FooFFXBbmBarl672;}FooFFXBbmBare(!(((FooFFXBbmBarl55)-> FooFFXBbmBarl34.FooFFXBbmBarl29^(0x004a6f34))==( FooFFXBbmBarl55)->FooFFXBbmBarl39.FooFFXBbmBarl29)){
FooFFXBbmBarw=(FooFFXBbmBarl57(15,21));FooFFXBbmBarj
FooFFXBbmBarl672;}FooFFXBbmBare(FooFFXBbmBarr== FooFFXBbmBarg->FooFFXBbmBarl131.FooFFXBbmBarl39. FooFFXBbmBarl29||FooFFXBbmBarr==FooFFXBbmBarg-> FooFFXBbmBarl131.FooFFXBbmBarl34.FooFFXBbmBarl29){{(& FooFFXBbmBarg->FooFFXBbmBarl131)->FooFFXBbmBarl34.
FooFFXBbmBarl37=FooFFXBbmBarl190;(&FooFFXBbmBarg-> FooFFXBbmBarl131)->FooFFXBbmBarl34.FooFFXBbmBarl29=( 0x0FFF9000);(&FooFFXBbmBarg->FooFFXBbmBarl131)->
FooFFXBbmBarl39.FooFFXBbmBarl37=FooFFXBbmBarl190;(& FooFFXBbmBarg->FooFFXBbmBarl131)->FooFFXBbmBarl39.
FooFFXBbmBarl29=(0x0FFF9000);};}FooFFXBbmBarw=( FooFFXBbmBarl57(15,14));FooFFXBbmBare(FooFFXBbmBarr< FooFFXBbmBarg->FooFFXBbmBarl32.FooFFXBbmBarl46){
FooFFXBbmBarw=FooFFXBbmBarl116(&FooFFXBbmBarg-> FooFFXBbmBarl56,FooFFXBbmBarr,FooFFXBbmBarl211,& FooFFXBbmBarq,FooFFXBbmBarl30);}FooFFXBbmBarl122{
FooFFXBbmBare(FooFFXBbmBarl55->FooFFXBbmBarl39. FooFFXBbmBarl29!=FooFFXBbmBarr){FFXPRINTF(2,("\x57\x61\x72\x6e\x69\x6e\x67\x20\x61\x74\x74\x65\x6d\x70\x74\x20\x74\x6f\x20\x72\x65\x6d\x61\x70\x20\x61\x20\x42\x42\x4d\x20\x73\x79\x73\x74\x65\x6d\x20\x62\x6c\x6f\x63\x6b\x20\x74\x6f\x20\x61\x6e\x6f\x74\x68\x65\x72\x20\x73\x79\x73\x74\x65\x6d\x20\x62\x6c\x6f\x63\x6b\x2e\x20\x28\x52\x65\x6d\x61\x70\x2e\x64\x61\x74\x61\x2e\x6b\x65\x79\x20\x3d\x20\x25\x6c\x58\x2c\x20\x52\x65\x6d\x61\x70\x2c\x70\x68\x79\x73\x2e\x6b\x65\x79\x20\x3d\x20\x25\x6c\x58\x29", FooFFXBbmBarl55->FooFFXBbmBarl34.FooFFXBbmBarl29, FooFFXBbmBarl55->FooFFXBbmBarl39.FooFFXBbmBarl29));
FooFFXBbmBarw=(FooFFXBbmBarl57(15,6));FooFFXBbmBarj
FooFFXBbmBarl672;}}FooFFXBbmBare(FooFFXBbmBarw== FooFFXBbmBarn&&FooFFXBbmBarl37==FooFFXBbmBarl124){
FooFFXBbmBarw=(FooFFXBbmBarl57(15,22));FooFFXBbmBarj
FooFFXBbmBarl672;}FooFFXBbmBarl122 FooFFXBbmBare( FooFFXBbmBarw==FooFFXBbmBarn){FooFFXBbmBarw=
FooFFXBbmBarl198(&FooFFXBbmBarg->FooFFXBbmBarl56,& FooFFXBbmBarq);FooFFXBbmBarl53(222,FooFFXBbmBarw== FooFFXBbmBarn);FooFFXBbmBarq.FooFFXBbmBarl34.
FooFFXBbmBarl29=FooFFXBbmBarq.FooFFXBbmBarl39.
FooFFXBbmBarl29;FooFFXBbmBarq.FooFFXBbmBarl34.
FooFFXBbmBarl37=FooFFXBbmBarq.FooFFXBbmBarl39.
FooFFXBbmBarl37=FooFFXBbmBarl85;FooFFXBbmBarw=
FooFFXBbmBarl191(&FooFFXBbmBarg->FooFFXBbmBarl56,& FooFFXBbmBarq);FooFFXBbmBarl53(226,FooFFXBbmBarw== FooFFXBbmBarn);}FooFFXBbmBarw=FooFFXBbmBarl198(& FooFFXBbmBarg->FooFFXBbmBarl56,FooFFXBbmBarl55);
FooFFXBbmBare(FooFFXBbmBarw!=FooFFXBbmBarn){FooFFXBbmBarj
FooFFXBbmBarl672;}FooFFXBbmBarl55->FooFFXBbmBarl34.
FooFFXBbmBarl29=FooFFXBbmBarr;FooFFXBbmBarl55->
FooFFXBbmBarl34.FooFFXBbmBarl37=FooFFXBbmBarl37;
FooFFXBbmBarw=FooFFXBbmBarl191(&FooFFXBbmBarg-> FooFFXBbmBarl56,FooFFXBbmBarl55);FooFFXBbmBare( FooFFXBbmBarw!=FooFFXBbmBarn){FooFFXBbmBarj FooFFXBbmBarl672
;}FooFFXBbmBarw=FooFFXBbmBarl263(FooFFXBbmBarg);
FooFFXBbmBarl672:FooFFXBbmBare(FooFFXBbmBarl145)FooFFXBbmBarl646
(FooFFXBbmBarg->FooFFXBbmBarl132);FooFFXBbmBarl635(0UL);
FFXTRACEPRINTF((FooFFXBbmBarl239(FFXTRACE_BBM,1, FooFFXBbmBarl644),"\x46\x66\x78\x42\x62\x6d\x35\x43\x6f\x6d\x6d\x69\x74\x52\x65\x6d\x61\x70\x28\x29\x20\x72\x65\x74\x75\x72\x6e\x69\x6e\x67\x20\x25\x55\n",FooFFXBbmBarw));
FooFFXBbmBarl52 FooFFXBbmBarw;}
