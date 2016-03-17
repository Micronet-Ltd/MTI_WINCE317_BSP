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
/* COBF by BB -- 'fxoption.c' obfuscated at Tue Apr 24 23:21:13 2012
*/
#include "cobf.h"
#ifndef D_PRODUCTLOCALE
#define D_PRODUCTLOCALE (2)
#endif
#include <dcl.h>
#include <ffxconf.h>
#include <oesl.h>
FooFFXVbfBarb DCLSTATUS FFXSTATUS;
#define FooFFXVbfBarl44 DCLSTAT_SUCCESS
#define FooFFXVbfBarl241 DCLSTAT_FAILURE
#define FFXLOCALE_VBF 3
#define FooFFXVbfBarq( FooFFXVbfBarl194, FooFFXVbfBarl127)  \
DCL_MAKESTATUS( PRODUCTNUM_FLASHFX, DCLSTATUSCLASS_ERROR,   FooFFXVbfBarl194, FooFFXVbfBarl127)
#define FooFFXVbfBarl133 DCLSTAT_BADSTRUCLEN
#define FFXSTAT_BADPARAMETER DCLSTAT_BADPARAMETER
#define FooFFXVbfBarl156 DCLSTAT_BADHANDLE
#define FooFFXVbfBarl284 DCLSTAT_UNSUPPORTEDFUNCTION
#define FooFFXVbfBarl949 DCLSTAT_BADSYNTAX
#define FooFFXVbfBarl874 DCLSTAT_ARGUMENTSTOOLONG
#define FooFFXVbfBarl578 DCLSTAT_BADPARAMETERLEN
#define FooFFXVbfBarl373 DCLSTAT_OUTOFMEMORY
#define FooFFXVbfBarl885 DCLSTAT_UNSUPPORTEDREQUEST
#define FooFFXVbfBarl766 DCLSTAT_OUTOFRANGE
#define FooFFXVbfBarl616 DCLSTAT_CATEGORYDISABLED
#define FooFFXVbfBarl991 DCLSTAT_SUBTYPECOMPLETE
#define FooFFXVbfBarl926 DCLSTAT_SUBTYPEUNUSED
#define FooFFXVbfBarl775 DCLSTAT_UNSUPPORTEDFEATURE
#define FooFFXVbfBarl823 FooFFXVbfBarl704
#define FooFFXVbfBarl720 ( FooFFXVbfBarq( FFXLOCALE_VBF,   100))
#define FooFFXVbfBarl791 ( FooFFXVbfBarq( FFXLOCALE_VBF,   101))
#define FooFFXVbfBarl569 ( FooFFXVbfBarq( FFXLOCALE_VBF,   102))
#define FooFFXVbfBarl973 ( FooFFXVbfBarq( FFXLOCALE_VBF,   103))
#define FooFFXVbfBarl714 ( FooFFXVbfBarq( FFXLOCALE_VBF,   104))
#define FooFFXVbfBarl694 ( FooFFXVbfBarq( FFXLOCALE_VBF,   105))
#define FooFFXVbfBarl692 ( FooFFXVbfBarq( FFXLOCALE_VBF,   106))
#define FooFFXVbfBarl638 ( FooFFXVbfBarq( FFXLOCALE_VBF,   107))
#define FooFFXVbfBarl605 ( FooFFXVbfBarq( FFXLOCALE_VBF,   108))
#define FooFFXVbfBarl966 ( FooFFXVbfBarq( FFXLOCALE_VBF,   109))
#define FooFFXVbfBarl737 ( FooFFXVbfBarq( FFXLOCALE_VBF,   110))
#define FooFFXVbfBarl539 ( FooFFXVbfBarq( FFXLOCALE_VBF,   111))
#define FooFFXVbfBarl677 ( FooFFXVbfBarq( FFXLOCALE_VBF,   112))
#define FooFFXVbfBarl772 ( FooFFXVbfBarq( FFXLOCALE_VBF,   113))
#define FooFFXVbfBarl529 ( FooFFXVbfBarq( FFXLOCALE_VBF,   120))
#define FooFFXVbfBarl787 ( FooFFXVbfBarq( FFXLOCALE_VBF,   122))
#define FooFFXVbfBarl538 ( FooFFXVbfBarq( FFXLOCALE_VBF,   123))
#define FooFFXVbfBarl518 ( FooFFXVbfBarq( FFXLOCALE_VBF,   125))
#define FooFFXVbfBarl456 ( FooFFXVbfBarq( FFXLOCALE_VBF,   126))
#define FooFFXVbfBarl733 ( FooFFXVbfBarq( FFXLOCALE_VBF,   127))
#define FooFFXVbfBarl577 ( FooFFXVbfBarq( FFXLOCALE_VBF,   128))
#define FooFFXVbfBarl761 ( FooFFXVbfBarq( FFXLOCALE_VBF,   129))
#define FooFFXVbfBarl933 ( FooFFXVbfBarq( FFXLOCALE_VBF,   130))
#define FooFFXVbfBarl740 ( FooFFXVbfBarq( FFXLOCALE_VBF,   131))
#define FooFFXVbfBarl674 ( FooFFXVbfBarq( FFXLOCALE_VBF,   132))
#define FooFFXVbfBarl738 ( FooFFXVbfBarq( FFXLOCALE_VBF,   133))
#define FooFFXVbfBarl424 ( FooFFXVbfBarq( FFXLOCALE_VBF,   134))
#define FooFFXVbfBarl748 ( FooFFXVbfBarq( FFXLOCALE_VBF,   135))
#define FooFFXVbfBarl764 ( FooFFXVbfBarq( FFXLOCALE_VBF,   136))
#define FooFFXVbfBarl756 ( FooFFXVbfBarq( FFXLOCALE_VBF,   137))
FooFFXVbfBarb FooFFXVbfBarf FooFFXVbfBarl101* *
FooFFXVbfBarx;FooFFXVbfBarb DCLOSREQHANDLE FFXREQHANDLE;
FooFFXVbfBarb FooFFXVbfBarf FooFFXVbfBarl922*
FooFFXVbfBarl124;FooFFXVbfBarb FooFFXVbfBarf
FooFFXVbfBarl895*FooFFXVbfBarl45;FooFFXVbfBarb FooFFXVbfBarf
FooFFXVbfBarl533 FooFFXVbfBarl54;FooFFXVbfBarb FooFFXVbfBarf
FooFFXVbfBarl904 FooFFXVbfBarl235;FooFFXVbfBarb
FooFFXVbfBarf FooFFXVbfBarl520 FooFFXVbfBarl173;
FooFFXVbfBarb FooFFXVbfBarf FooFFXVbfBarl965
FooFFXVbfBarl157;FooFFXVbfBarb FooFFXVbfBarf
FooFFXVbfBarl234 FooFFXVbfBarl354;FooFFXVbfBarb
FooFFXVbfBarf FooFFXVbfBarl234* *FooFFXVbfBarl71;
FooFFXVbfBarb FooFFXVbfBarf FooFFXVbfBarl905
FooFFXVbfBarl229;FooFFXVbfBarb FooFFXVbfBarf
FooFFXVbfBarl525 FooFFXVbfBarl167;FooFFXVbfBarb
FooFFXVbfBarf FooFFXVbfBarl838 FooFFXVbfBarl154;
FooFFXVbfBarb FooFFXVbfBarf FooFFXVbfBarl238
FooFFXVbfBarl236;FooFFXVbfBarb FooFFXVbfBarf
FooFFXVbfBarl238* *FooFFXVbfBarl70;FooFFXVbfBarb
FooFFXVbfBarf FooFFXVbfBarl282 FooFFXVbfBarl35;
#define FFX_FORMAT_NEVER (0)
#define FFX_FORMAT_ONCE (1)
#define FFX_FORMAT_ALWAYS (2)
#define FFX_FORMAT_UNFORMAT (3)
#define FooFFXVbfBarl900 ( D_UINT32_MAX)
#define FooFFXVbfBarl987 ( D_UINT32_MAX)
#define FooFFXVbfBarl817 ( D_UINT32_MAX-2)
#define FooFFXVbfBarl685 (( FooFFXVbfBarh  *  )   D_UINT32_MAX)
#define FooFFXVbfBarl705 TRUE, FFX_FORMAT_ONCE,  \
FooFFXVbfBarl862,100
#define FFX_FILESYS_UNKNOWN FooFFXVbfBarl731
#define FFX_FILESYS_FAT FooFFXVbfBarl646
#define FFX_FILESYS_RELIANCE FooFFXVbfBarl680
#define FooFFXVbfBarl916 FooFFXVbfBarl681
#define FooFFXVbfBarl827 FFX_FORMAT_ONCE,  \
FFX_FILESYS_UNKNOWN, FALSE, 0
#define FFX_COMPACT_NONE (0)
#define FFX_COMPACT_SYNCHRONOUS (0)
#define FFX_COMPACT_BACKGROUNDIDLE (1)
#define FFX_COMPACT_BACKGROUNDTHREAD (2)
#define FooFFXVbfBarl902 FALSE
#ifndef FooFFXVbfBarl74
#define FooFFXVbfBarl74 TRUE
#endif
#ifndef FFXCONF_VBFSUPPORT
#define FFXCONF_VBFSUPPORT FooFFXVbfBarl74
#endif
#if ! FooFFXVbfBarl74
#if FFXCONF_VBFSUPPORT
#error  \
"FFX: FFXCONF_ALLOCATORSUPPORT is FALSE, but FFXCONF_VBFSUPPORT is TRUE"
#endif
#endif
#ifndef FFXCONF_QUICKMOUNTSUPPORT
#define FFXCONF_QUICKMOUNTSUPPORT FALSE
#endif
#if FFXCONF_QUICKMOUNTSUPPORT
#if ! FFXCONF_VBFSUPPORT
#error  \
"FFX: FFXCONF_QUICKMOUNTSUPPORT is TRUE, but FFXCONF_VBFSUPPORT is FALSE."
#endif
#endif
#ifndef FFXCONF_DRIVERAUTOFORMAT
#define FFXCONF_DRIVERAUTOFORMAT FALSE
#endif
#ifndef FooFFXVbfBarl109
#define FooFFXVbfBarl109 FALSE
#endif
#ifndef FFXCONF_FATFORMATSUPPORT
#define FFXCONF_FATFORMATSUPPORT FALSE
#endif
#ifndef FFXCONF_BBMFORMAT
#define FFXCONF_BBMFORMAT FALSE
#endif
#define FooFFXVbfBarl971 FFXCONF_NANDSUPPORT
#ifndef FooFFXVbfBarl228
#define FooFFXVbfBarl228 FFXCONF_NANDSUPPORT
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
#if FooFFXVbfBarl109
#error  \
"FFX: FFXCONF_MBRSUPPORT is FALSE, but FFXCONF_MBRFORMAT is TRUE"
#endif
#endif
#if ! FFXCONF_FORMATSUPPORT
#if FooFFXVbfBarl109
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
#if ! FooFFXVbfBarl74
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
#define FooFFXVbfBarl961 FALSE
#define FooFFXVbfBarl530 FALSE
#ifndef FFXCONF_STATS_COMPACTION
#define FFXCONF_STATS_COMPACTION FALSE
#endif
#ifndef FFXCONF_STATS_VBFREGION
#define FFXCONF_STATS_VBFREGION FALSE
#endif
#if ! FFXCONF_LATENCYREDUCTIONENABLED
#define FFXCONF_ERASESUSPENDSUPPORT FALSE
#define FooFFXVbfBarl673 FALSE
#endif
#if FFXCONF_NANDSUPPORT
#ifndef FFXCONF_MIGRATE_LEGACY_FLASH
#define FFXCONF_MIGRATE_LEGACY_FLASH FALSE
#endif
#endif
#if DCLCONF_COMMAND_SHELL
#ifndef FooFFXVbfBarl122
#define FooFFXVbfBarl122 FooFFXVbfBarl670
#endif
#ifndef FooFFXVbfBarl121
#define FooFFXVbfBarl121 FooFFXVbfBarl783
#endif
#else
#if defined( FooFFXVbfBarl122) && FooFFXVbfBarl122
#error  \
"FFX: FFXCONF_SHELL_TESTS is defined, but DCLCONF_COMMAND_SHELL is FALSE"
#endif
#if defined( FooFFXVbfBarl121) && FooFFXVbfBarl121
#error  \
"FFX: FFXCONF_SHELL_TOOLS is defined, but DCLCONF_COMMAND_SHELL is FALSE"
#endif
#endif
FooFFXVbfBarb FooFFXVbfBarl61{FooFFXVbfBarl990=0x0000,
#if FFXCONF_FATSUPPORT
#if FFXCONF_FATFORMATSUPPORT
FooFFXVbfBarl868=0x0400,FooFFXVbfBarl945,FooFFXVbfBarl1004,
#endif
#if FFXCONF_FATMONITORSUPPORT
FooFFXVbfBarl998=0x0600,
#endif
#endif
#if FFX_COMPACTIONMODEL != FFX_COMPACT_SYNCHRONOUS
FooFFXVbfBarl755=0x0A00,FooFFXVbfBarl804,
#if FFX_COMPACTIONMODEL == FFX_COMPACT_BACKGROUNDTHREAD
FooFFXVbfBarl897,FooFFXVbfBarl832,
#endif
#endif
FooFFXVbfBarl842=0x2000,FooFFXVbfBarl968,FooFFXVbfBarl906,
FooFFXVbfBarl941,
#if FFXCONF_LATENCYREDUCTIONENABLED
FooFFXVbfBarl876,FooFFXVbfBarl955,
#endif
#if FFXCONF_BBMSUPPORT
FooFFXVbfBarl851,
#endif
FooFFXVbfBarl860,FooFFXVbfBarl881,FooFFXVbfBarl997,
FooFFXVbfBarl815=0x3000,FooFFXVbfBarl852,FooFFXVbfBarl893,
FooFFXVbfBarl869,FooFFXVbfBarl841,
#if FooFFXVbfBarl74
FooFFXVbfBarl870,FooFFXVbfBarl613,
#endif
#if FFXCONF_FORMATSUPPORT
FooFFXVbfBarl994,
#endif
FooFFXVbfBarl821=0x6000,FooFFXVbfBarl826,FooFFXVbfBarl910,
FooFFXVbfBarl892,FooFFXVbfBarl939=0x7000,FooFFXVbfBarl1001,
FooFFXVbfBarl840,FooFFXVbfBarl825,FooFFXVbfBarl932,
FooFFXVbfBarl849,FooFFXVbfBarl908,FooFFXVbfBarl883,
FooFFXVbfBarl999=0x8000,FooFFXVbfBarl839=0x9000,
FooFFXVbfBarl790,FooFFXVbfBarl867}FooFFXVbfBarl103;
FooFFXVbfBarj FooFFXVbfBarl762(FooFFXVbfBarl103 FooFFXVbfBarl128,FooFFXVbfBarh*FooFFXVbfBarz,FooFFXVbfBara FooFFXVbfBarl57);
#ifndef FXPARAM_H_INCLUDED
#define FXPARAM_H_INCLUDED
#define FooFFXVbfBarl928 (8)
#define FooFFXVbfBarl974 (16)
FooFFXVbfBarb FooFFXVbfBarl61{FooFFXVbfBarl915=0x0000,
FooFFXVbfBarl927=0x0001,FooFFXVbfBarl912=0x0100,
FooFFXVbfBarl891,FooFFXVbfBarl820,FooFFXVbfBarl992,
FooFFXVbfBarl889,FooFFXVbfBarl911,FooFFXVbfBarl631,
FooFFXVbfBarl540,FooFFXVbfBarl711,FooFFXVbfBarl547,
FooFFXVbfBarl829,FooFFXVbfBarl630,FooFFXVbfBarl989=0x0200,
FooFFXVbfBarl913,FooFFXVbfBarl858,FooFFXVbfBarl909,
FooFFXVbfBarl925,FooFFXVbfBarl898=0x0300,FooFFXVbfBarl914,
FooFFXVbfBarl980=0x0400,FooFFXVbfBarl929,FooFFXVbfBarl967,
FooFFXVbfBarl830,FooFFXVbfBarl836=0x04C0,FooFFXVbfBarl938=
0x04C0,FooFFXVbfBarl875,FooFFXVbfBarl947,FooFFXVbfBarl985,
FooFFXVbfBarl953,FooFFXVbfBarl1000=0x0500,FooFFXVbfBarl866}
FooFFXVbfBarl87;
#endif
FooFFXVbfBarh FooFFXVbfBarl901(FooFFXVbfBara FooFFXVbfBarl843);FooFFXVbfBara FooFFXVbfBarl850( FooFFXVbfBarh);FooFFXVbfBarh FooFFXVbfBarl824(FooFFXVbfBara FooFFXVbfBarl970);FooFFXVbfBarj FooFFXVbfBarl907( FooFFXVbfBarh);FooFFXVbfBarj FooFFXVbfBarl887( FooFFXVbfBarl54*FooFFXVbfBarl114);FooFFXVbfBarh
FooFFXVbfBarl899(FooFFXVbfBarl54*FooFFXVbfBarl114);
FooFFXVbfBarl154*FooFFXVbfBarl835(FooFFXVbfBarc FooFFXVbfBarl54*FooFFXVbfBarl118,FooFFXVbfBarc FooFFXVbfBarl229*FooFFXVbfBarl871,FooFFXVbfBarl167* FooFFXVbfBarl146);FooFFXVbfBarh FooFFXVbfBarl995( FooFFXVbfBarc FooFFXVbfBarl54*FooFFXVbfBarl118, FooFFXVbfBarl154*FooFFXVbfBarl214);FooFFXVbfBarl157*
FooFFXVbfBarl936(FooFFXVbfBarc FooFFXVbfBarl54* FooFFXVbfBarl118,FooFFXVbfBarc FooFFXVbfBarl235* FooFFXVbfBarl831,FooFFXVbfBarl173*FooFFXVbfBarl146);
FooFFXVbfBarh FooFFXVbfBarl963(FooFFXVbfBarc FooFFXVbfBarl54  *FooFFXVbfBarl118,FooFFXVbfBarl157*FooFFXVbfBarl214);
FFXSTATUS FooFFXVbfBarl865(FooFFXVbfBarl71 FooFFXVbfBarl90, FooFFXVbfBari FooFFXVbfBarl271);FFXSTATUS FooFFXVbfBarl882( FooFFXVbfBarl71 FooFFXVbfBarl90,FooFFXVbfBari FooFFXVbfBarl271);FooFFXVbfBara FooFFXVbfBarl878( FooFFXVbfBarl71 FooFFXVbfBarl90,FooFFXVbfBara FooFFXVbfBarl138,FooFFXVbfBara FooFFXVbfBarl946, FooFFXVbfBarl111 FooFFXVbfBarh* *FooFFXVbfBarl811);
FooFFXVbfBara FooFFXVbfBarl814(FooFFXVbfBarl71 FooFFXVbfBarl90,FooFFXVbfBara FooFFXVbfBarl84,FooFFXVbfBara FooFFXVbfBarl119);FooFFXVbfBarj FooFFXVbfBarl417( FooFFXVbfBarl103 FooFFXVbfBarl128,FooFFXVbfBarh* FooFFXVbfBarl125,FooFFXVbfBarh*FooFFXVbfBarz,FooFFXVbfBara FooFFXVbfBarl57);FooFFXVbfBarj FooFFXVbfBarl979( FooFFXVbfBarl103 FooFFXVbfBarl128,FooFFXVbfBarh* FooFFXVbfBarl125,FooFFXVbfBarh*FooFFXVbfBarz,FooFFXVbfBara FooFFXVbfBarl57);FooFFXVbfBare FooFFXVbfBarl812( FooFFXVbfBarc FooFFXVbfBarl37*FooFFXVbfBarl753);
FooFFXVbfBarc FooFFXVbfBarl37*FooFFXVbfBarl847( FooFFXVbfBarj FooFFXVbfBarl394);
#if D_DEBUG && DCLCONF_OUTPUT_ENABLED
#define FooFFXVbfBarl494 DCLTRACE_ALWAYS
#define FooFFXVbfBarl260 1
#define FFXTRACE_DRIVERFW 2
#define FFXTRACE_VBF 3
#define FFXTRACE_COMPACTION 4
#define FooFFXVbfBarl248 5
#define FooFFXVbfBarl261 6
#define FooFFXVbfBarl246 7
#define FooFFXVbfBarl244 8
#define FooFFXVbfBarl247 9
#define FooFFXVbfBarl249 10
#define FFXTRACE_CONFIG 11
#define FooFFXVbfBarl162 12
#define FooFFXVbfBarl254 13
#define FooFFXVbfBarl200 14
#define FooFFXVbfBarl150 15
#define FooFFXVbfBarl253 16
#define FooFFXVbfBarl561 0
#define FooFFXVbfBarl654 D_UINT32_MAX
#define FooFFXVbfBarl648 (( FooFFXVbfBara)1 << (   FooFFXVbfBarl260-1))
#define FooFFXVbfBarl801 (( FooFFXVbfBara)1 << (   FFXTRACE_DRIVERFW-1))
#define FooFFXVbfBarl651 (( FooFFXVbfBara)1 << (   FFXTRACE_VBF-1))
#define FooFFXVbfBarl729 (( FooFFXVbfBara)1 << (   FFXTRACE_COMPACTION-1))
#define FooFFXVbfBarl693 (( FooFFXVbfBara)1 << (   FooFFXVbfBarl248-1))
#define FooFFXVbfBarl796 (( FooFFXVbfBara)1 << (   FooFFXVbfBarl261-1))
#define FooFFXVbfBarl709 (( FooFFXVbfBara)1 << (   FooFFXVbfBarl246-1))
#define FooFFXVbfBarl671 (( FooFFXVbfBara)1 << (   FooFFXVbfBarl244-1))
#define FooFFXVbfBarl621 (( FooFFXVbfBara)1 << (   FooFFXVbfBarl247-1))
#define FooFFXVbfBarl687 (( FooFFXVbfBara)1 << (   FooFFXVbfBarl249-1))
#define FooFFXVbfBarl675 (( FooFFXVbfBara)1 << (   FFXTRACE_CONFIG-1))
#define FooFFXVbfBarl647 (( FooFFXVbfBara)1 << (   FooFFXVbfBarl162-1))
#define FooFFXVbfBarl676 (( FooFFXVbfBara)1 << (   FooFFXVbfBarl254-1))
#define FooFFXVbfBarl622 (( FooFFXVbfBara)1 << (   FooFFXVbfBarl200-1))
#define FooFFXVbfBarl767 (( FooFFXVbfBara)1 << (   FooFFXVbfBarl150-1))
#define FooFFXVbfBarl634 (( FooFFXVbfBara)1 << (   FooFFXVbfBarl253-1))
FooFFXVbfBarl55 FooFFXVbfBarl240(FooFFXVbfBarc FooFFXVbfBarl37*FooFFXVbfBarl212,...);FooFFXVbfBarl55
FooFFXVbfBarl256(FooFFXVbfBara FooFFXVbfBarl65, FooFFXVbfBarc FooFFXVbfBarl37*FooFFXVbfBarl212,...);
FooFFXVbfBara FooFFXVbfBarl758(FooFFXVbfBarh);FooFFXVbfBara
FooFFXVbfBarl724(FooFFXVbfBara FooFFXVbfBarl599);
#define FFXPRINTF( FooFFXVbfBarl136, FooFFXVbfBarl152) (((   FooFFXVbfBarl136)<= D_DEBUG) ? ( FooFFXVbfBarh)   FooFFXVbfBarl240 FooFFXVbfBarl152 : (( FooFFXVbfBarh)0))
#define FFXTRACEPRINTF( FooFFXVbfBarl155) FooFFXVbfBarl256  \
FooFFXVbfBarl155
#else
#define FFXPRINTF( FooFFXVbfBarl136, FooFFXVbfBarl152)
#define FFXTRACEPRINTF( FooFFXVbfBarl155)
#endif
FooFFXVbfBarc FooFFXVbfBarl37*FooFFXVbfBarl313( FooFFXVbfBarc FooFFXVbfBarl35*FooFFXVbfBarl617);
#define FooFFXVbfBarl338( FooFFXVbfBarl52) ( FooFFXVbfBarh)  \
FooFFXVbfBarl110( FooFFXVbfBarl79, __FILE__, (   FooFFXVbfBarl52))
#define FooFFXVbfBarl372( FooFFXVbfBarl52, FooFFXVbfBarl94)  \
(( FooFFXVbfBarl94) ? (( FooFFXVbfBarh)0) : ( FooFFXVbfBarh  ) FooFFXVbfBarl110( FooFFXVbfBarl79, __FILE__, (   FooFFXVbfBarl52)))
#if D_DEBUG >= 1
#define FooFFXVbfBarl112( FooFFXVbfBarl52) ( FooFFXVbfBarh)  \
FooFFXVbfBarl110( FooFFXVbfBarl79, __FILE__, (   FooFFXVbfBarl52))
#define FooFFXVbfBarl27( FooFFXVbfBarl52, FooFFXVbfBarl94)  \
(( FooFFXVbfBarl94) ? (( FooFFXVbfBarh)0) : ( FooFFXVbfBarh  ) FooFFXVbfBarl110( FooFFXVbfBarl79, __FILE__, (   FooFFXVbfBarl52)))
#else
#define FooFFXVbfBarl112( FooFFXVbfBarl52)
#define FooFFXVbfBarl27( FooFFXVbfBarl52, FooFFXVbfBarl94)
#endif
FooFFXVbfBarj FooFFXVbfBarl762(FooFFXVbfBarl103 FooFFXVbfBarl128,FooFFXVbfBarh*FooFFXVbfBarz,FooFFXVbfBara FooFFXVbfBarl57){FooFFXVbfBarj FooFFXVbfBarl1785=TRUE;( FooFFXVbfBarh)FooFFXVbfBarl57;FFXTRACEPRINTF(( FooFFXVbfBarl66(FFXTRACE_CONFIG,2,FooFFXVbfBarl283),"\x46\x66\x78\x4f\x70\x74\x69\x6f\x6e\x47\x65\x74\x28\x29\x20\x54\x79\x70\x65\x3d\x25\x78\x20\x70\x42\x75\x66\x66\x3d\x25\x50\x20\x4c\x65\x6e\x3d\x25\x6c\x55\n", FooFFXVbfBarl128,FooFFXVbfBarz,FooFFXVbfBarl57));
FooFFXVbfBarl1152(FooFFXVbfBarl128){FooFFXVbfBarl1227:
FFXPRINTF(2,("\x46\x66\x78\x4f\x70\x74\x69\x6f\x6e\x47\x65\x74\x28\x29\x20\x55\x6e\x73\x75\x70\x70\x6f\x72\x74\x65\x64\x20\x63\x6f\x6e\x66\x69\x67\x75\x72\x61\x74\x69\x6f\x6e\x20\x6f\x70\x74\x69\x6f\x6e\x20\x25\x78\n", FooFFXVbfBarl128));FooFFXVbfBarl1785=FALSE;FooFFXVbfBarl100
;}FFXTRACEPRINTF((FooFFXVbfBarl66(FFXTRACE_CONFIG,2, FooFFXVbfBarl267),"\x46\x66\x78\x4f\x70\x74\x69\x6f\x6e\x47\x65\x74\x28\x29\x20\x72\x65\x74\x75\x72\x6e\x69\x6e\x67\x20\x25\x55\n",FooFFXVbfBarl1785));FooFFXVbfBarl46
FooFFXVbfBarl1785;}
