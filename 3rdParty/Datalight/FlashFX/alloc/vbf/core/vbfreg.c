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
/* COBF by BB -- 'vbfreg.c' obfuscated at Tue Apr 24 23:21:14 2012
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
#define FooFFXVbfBarl1283 FooFFXVbfBarl333,  \
FooFFXVbfBarl1298
#define FooFFXVbfBarl1295 FooFFXVbfBarl333,  \
FooFFXVbfBarl1294
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
#if FFXCONF_VBFSUPPORT
#if FFXCONF_VBFSUPPORT
#ifndef FooFFXVbfBarl285
#define FooFFXVbfBarl285
#include <vbfconf.h>
#define FooFFXVbfBarl126 ( VBF_MAX_UNIT_SIZE /   VBF_MIN_BLOCK_SIZE)
#define FooFFXVbfBarl274 (1)
#define FooFFXVbfBarl276 (250)
#if VBFCONF_DEFAULTCUSHION < FooFFXVbfBarl274
#error "FFX: VBF cushion is too small"
#endif
#if VBFCONF_DEFAULTCUSHION > FooFFXVbfBarl276
#error "FFX: VBF cushion is too large"
#endif
#define VBF_DEFAULT_CUSHION VBFCONF_DEFAULTCUSHION
#define FooFFXVbfBarl1009 (5)
#define FooFFXVbfBarl942 (1)
#define FooFFXVbfBarl950 (1)
FooFFXVbfBarb FooFFXVbfBarf{FooFFXVbfBarl45 FooFFXVbfBarm;
FooFFXVbfBara FooFFXVbfBarl400;FooFFXVbfBara
FooFFXVbfBarl216;FooFFXVbfBare FooFFXVbfBarl64;
FooFFXVbfBare FooFFXVbfBarl954:1;FooFFXVbfBare
FooFFXVbfBarl245:1;
#if FFXCONF_QUICKMOUNTSUPPORT
FooFFXVbfBare FooFFXVbfBarl845:1;
#endif
FooFFXVbfBari FooFFXVbfBarl97;}FooFFXVbfBarl251;
FooFFXVbfBarb FooFFXVbfBarf{FooFFXVbfBara FooFFXVbfBarl216;
FooFFXVbfBara FooFFXVbfBarl859;FooFFXVbfBara
FooFFXVbfBarl959;FooFFXVbfBara FooFFXVbfBarl958;
FooFFXVbfBara FooFFXVbfBarl828;FooFFXVbfBara
FooFFXVbfBarl1005;FooFFXVbfBari FooFFXVbfBarl97;
FooFFXVbfBari FooFFXVbfBarl816;FooFFXVbfBari
FooFFXVbfBarl1002;FooFFXVbfBari FooFFXVbfBarl861;
FooFFXVbfBari FooFFXVbfBarl943;FooFFXVbfBare
FooFFXVbfBarl818;FooFFXVbfBare FooFFXVbfBarl846;
FooFFXVbfBare FooFFXVbfBarl894;FooFFXVbfBare
FooFFXVbfBarl810;FooFFXVbfBare FooFFXVbfBarl986;}
FooFFXVbfBarl266;FooFFXVbfBarb FooFFXVbfBarf{FooFFXVbfBara
FooFFXVbfBarl171;FooFFXVbfBare FooFFXVbfBarl886;
FooFFXVbfBare FooFFXVbfBarl962;FooFFXVbfBare
FooFFXVbfBarl837;FooFFXVbfBarj FooFFXVbfBarl896;
FooFFXVbfBare FooFFXVbfBarl924;FooFFXVbfBare
FooFFXVbfBarl977;FooFFXVbfBare FooFFXVbfBarl944;
FooFFXVbfBare FooFFXVbfBarl880;}FooFFXVbfBarl268;
#define FooFFXVbfBarl948 (( FooFFXVbfBarl69) 0x10)
#define FooFFXVbfBarl877 (( FooFFXVbfBarl69) 0x11)
#define FooFFXVbfBarl993 (( FooFFXVbfBarl69) 0x12)
#define FooFFXVbfBarl857 (( FooFFXVbfBarl69) 0x13)
FooFFXVbfBarb FooFFXVbfBarf{FooFFXVbfBara FooFFXVbfBarl655;
FooFFXVbfBara FooFFXVbfBarl522;FooFFXVbfBara
FooFFXVbfBarl551;FooFFXVbfBara FooFFXVbfBarl409;
FooFFXVbfBara FooFFXVbfBarl457;FooFFXVbfBara
FooFFXVbfBarl735;FooFFXVbfBara FooFFXVbfBarl43;
FooFFXVbfBara FooFFXVbfBarl232;FooFFXVbfBarj
FooFFXVbfBarl439;FooFFXVbfBarj FooFFXVbfBarl597;
FooFFXVbfBarl69 FooFFXVbfBarl367[FooFFXVbfBarl126];}
FooFFXVbfBarl270;
#if FFXCONF_QUICKMOUNTSUPPORT
FFXSTATUS FooFFXVbfBarl420(FooFFXVbfBarl45 FooFFXVbfBarm, FooFFXVbfBara FooFFXVbfBarl237,FooFFXVbfBara* FooFFXVbfBarl554,FooFFXVbfBara*FooFFXVbfBarl618);
FooFFXVbfBarh FooFFXVbfBarl664(FooFFXVbfBarx FooFFXVbfBarp);
#endif
FooFFXVbfBarb FooFFXVbfBarl61{FooFFXVbfBarl879=0,
FooFFXVbfBarl404,FooFFXVbfBarl707,FooFFXVbfBarl743,
FooFFXVbfBarl535,FooFFXVbfBarl833}FooFFXVbfBarl281;
#if 0
FooFFXVbfBarb FooFFXVbfBarf FooFFXVbfBarl101* *
FooFFXVbfBarx;
#endif
FooFFXVbfBarx FooFFXVbfBarl531(FooFFXVbfBare FooFFXVbfBarl64 );FFXSTATUS FooFFXVbfBarl592(FooFFXVbfBarl45 FooFFXVbfBarm, FooFFXVbfBara FooFFXVbfBarl322,FooFFXVbfBara FooFFXVbfBarl1031,FooFFXVbfBarj FooFFXVbfBarl277);
FooFFXVbfBarx FooFFXVbfBarl523(FooFFXVbfBarl45 FooFFXVbfBarm );FFXSTATUS FooFFXVbfBarl516(FooFFXVbfBarx FooFFXVbfBarp, FooFFXVbfBara FooFFXVbfBarl65);FFXSTATUS FooFFXVbfBarl776( FooFFXVbfBarx FooFFXVbfBarp);FooFFXVbfBarl35
FooFFXVbfBarl536(FooFFXVbfBarx FooFFXVbfBarp,FooFFXVbfBara FooFFXVbfBary,FooFFXVbfBara FooFFXVbfBarl31,FooFFXVbfBaro* FooFFXVbfBarz);FooFFXVbfBarl35 FooFFXVbfBarl609( FooFFXVbfBarx FooFFXVbfBarp,FooFFXVbfBara FooFFXVbfBary, FooFFXVbfBara FooFFXVbfBarl31,FooFFXVbfBarc FooFFXVbfBaro* FooFFXVbfBarz);FooFFXVbfBarl35 FooFFXVbfBarl585( FooFFXVbfBarx FooFFXVbfBarp,FooFFXVbfBara FooFFXVbfBary, FooFFXVbfBara FooFFXVbfBarl31);FFXSTATUS FooFFXVbfBarl421( FooFFXVbfBarx FooFFXVbfBarp,FooFFXVbfBarl251* FooFFXVbfBarl543);FFXSTATUS FooFFXVbfBarl789(FooFFXVbfBarx FooFFXVbfBarp,FooFFXVbfBarl266*FooFFXVbfBarl224);FFXSTATUS
FooFFXVbfBarl765(FooFFXVbfBarx FooFFXVbfBarp,FooFFXVbfBara FooFFXVbfBarl380,FooFFXVbfBarl268*FooFFXVbfBarl292);
FFXSTATUS FooFFXVbfBarl721(FooFFXVbfBarx FooFFXVbfBarp, FooFFXVbfBarl270*FooFFXVbfBarl550);FooFFXVbfBarl35
FooFFXVbfBarl410(FooFFXVbfBarx FooFFXVbfBarp,FooFFXVbfBara FooFFXVbfBarl65);FFXSTATUS FooFFXVbfBarl888(FooFFXVbfBarx FooFFXVbfBarp,FooFFXVbfBarl87 FooFFXVbfBarl98,FooFFXVbfBarh  *FooFFXVbfBarz,FooFFXVbfBara FooFFXVbfBarl57);FFXSTATUS
FooFFXVbfBarl918(FooFFXVbfBarx FooFFXVbfBarp, FooFFXVbfBarl87 FooFFXVbfBarl98,FooFFXVbfBarc FooFFXVbfBarh  *FooFFXVbfBarz,FooFFXVbfBara FooFFXVbfBarl57);
FooFFXVbfBarl388 FooFFXVbfBarl635(FooFFXVbfBarl111 FooFFXVbfBarx FooFFXVbfBarp,FooFFXVbfBara FooFFXVbfBarl340, FooFFXVbfBare FooFFXVbfBarl222);FooFFXVbfBara
FooFFXVbfBarl699(FooFFXVbfBarx FooFFXVbfBarp);
#if FFX_COMPACTIONMODEL != FFX_COMPACT_NONE
FooFFXVbfBara FooFFXVbfBarl923(FooFFXVbfBarx FooFFXVbfBarp, FooFFXVbfBara FooFFXVbfBarl65);FooFFXVbfBara
FooFFXVbfBarl602(FooFFXVbfBarx FooFFXVbfBarp);FooFFXVbfBara
FooFFXVbfBarl749(FooFFXVbfBarx FooFFXVbfBarp);
#endif
#if FFXCONF_POWERSUSPENDRESUME
FFXSTATUS FooFFXVbfBarl975(FooFFXVbfBarx FooFFXVbfBarp, FooFFXVbfBare FooFFXVbfBarl96);FFXSTATUS FooFFXVbfBarl988( FooFFXVbfBarx FooFFXVbfBarp,FooFFXVbfBare FooFFXVbfBarl96);
#endif
#endif
#endif
#ifndef FooFFXVbfBarl315
#define FooFFXVbfBarl315
#ifndef FooFFXVbfBarl311
#define FooFFXVbfBarl311
#define FooFFXVbfBarl1252 0x10
FooFFXVbfBarb FooFFXVbfBarf{FooFFXVbfBara FooFFXVbfBarl477;
FooFFXVbfBarl205 FooFFXVbfBarl324;FooFFXVbfBara
FooFFXVbfBarl218;FooFFXVbfBara FooFFXVbfBarl169;
FooFFXVbfBara FooFFXVbfBarl131;FooFFXVbfBara
FooFFXVbfBarl794;FooFFXVbfBari FooFFXVbfBarl180;
FooFFXVbfBari FooFFXVbfBarl329;FooFFXVbfBari
FooFFXVbfBarl165;FooFFXVbfBari FooFFXVbfBarl598;
FooFFXVbfBari FooFFXVbfBarl341;FooFFXVbfBari
FooFFXVbfBarl418;FooFFXVbfBari FooFFXVbfBarl498;}
FooFFXVbfBarl1236,FooFFXVbfBarl68;
#define FooFFXVbfBarl1207 (38)
#define FooFFXVbfBarl1213 (64)
#endif
#ifndef FooFFXVbfBarl239
#define FooFFXVbfBarl239
FooFFXVbfBarb FooFFXVbfBarf{FooFFXVbfBarl158
FooFFXVbfBarl653;FooFFXVbfBara FooFFXVbfBarl667;
FooFFXVbfBara FooFFXVbfBarl773;FooFFXVbfBara
FooFFXVbfBarl489;FooFFXVbfBara FooFFXVbfBarl799;
FooFFXVbfBara FooFFXVbfBarl712;FooFFXVbfBara
FooFFXVbfBarl628;}FooFFXVbfBarl85;FooFFXVbfBarb
FooFFXVbfBarf{FooFFXVbfBarl85 FooFFXVbfBarl691;
FooFFXVbfBarl85 FooFFXVbfBarl659;FooFFXVbfBarl85
FooFFXVbfBarl742;FooFFXVbfBarl85 FooFFXVbfBarl746;
FooFFXVbfBara FooFFXVbfBarl652;FooFFXVbfBara
FooFFXVbfBarl632;}FooFFXVbfBarl223;FooFFXVbfBarb
FooFFXVbfBarf{FooFFXVbfBarl37 FooFFXVbfBarl728[
DCL_THREADNAMELEN];FooFFXVbfBara FooFFXVbfBarl797;
FooFFXVbfBara FooFFXVbfBarl770;FooFFXVbfBara
FooFFXVbfBarl726;FooFFXVbfBara FooFFXVbfBarl497;
FooFFXVbfBari FooFFXVbfBarl465;}FooFFXVbfBarl220;
FooFFXVbfBarb FooFFXVbfBarf{FooFFXVbfBara FooFFXVbfBarl570;
FooFFXVbfBara FooFFXVbfBarl607;FooFFXVbfBara
FooFFXVbfBarl612;FooFFXVbfBara FooFFXVbfBarl588;
FooFFXVbfBara FooFFXVbfBarl555;FooFFXVbfBara
FooFFXVbfBarl580;FooFFXVbfBara FooFFXVbfBarl573;
FooFFXVbfBara FooFFXVbfBarl596;FooFFXVbfBara
FooFFXVbfBarl590;FooFFXVbfBara FooFFXVbfBarl564;
FooFFXVbfBara FooFFXVbfBarl581;FooFFXVbfBara
FooFFXVbfBarl586;FooFFXVbfBara FooFFXVbfBarl567;
FooFFXVbfBara FooFFXVbfBarl544;FooFFXVbfBara
FooFFXVbfBarl556;FooFFXVbfBara FooFFXVbfBarl606;
FooFFXVbfBara FooFFXVbfBarl587;FooFFXVbfBara
FooFFXVbfBarl563;FooFFXVbfBara FooFFXVbfBarl576;
FooFFXVbfBara FooFFXVbfBarl591;FooFFXVbfBara
FooFFXVbfBarl604;FooFFXVbfBara FooFFXVbfBarl565;
FooFFXVbfBara FooFFXVbfBarl572;FooFFXVbfBara
FooFFXVbfBarl615;FooFFXVbfBara FooFFXVbfBarl557;
FooFFXVbfBara FooFFXVbfBarl575;FooFFXVbfBara
FooFFXVbfBarl593;}FooFFXVbfBarl163;FooFFXVbfBarb
FooFFXVbfBarf{FooFFXVbfBara FooFFXVbfBarl583;FooFFXVbfBara
FooFFXVbfBarl207;FooFFXVbfBara FooFFXVbfBarl610;}
FooFFXVbfBarl230;FooFFXVbfBarb FooFFXVbfBarf{FooFFXVbfBare
FooFFXVbfBarl603;FooFFXVbfBara FooFFXVbfBarl403;
FooFFXVbfBara FooFFXVbfBarl390;FooFFXVbfBara
FooFFXVbfBarl207;FooFFXVbfBara FooFFXVbfBarl406;
FooFFXVbfBare FooFFXVbfBarl601;FooFFXVbfBare
FooFFXVbfBarl449;FooFFXVbfBare FooFFXVbfBarl374;
FooFFXVbfBarl230*FooFFXVbfBarl227;}FooFFXVbfBarl160;
FooFFXVbfBarb FooFFXVbfBarf{FooFFXVbfBara FooFFXVbfBarl734;
FooFFXVbfBara FooFFXVbfBarl639;FooFFXVbfBara
FooFFXVbfBarl641;}FooFFXVbfBarl259;FooFFXVbfBarb
FooFFXVbfBarf{FooFFXVbfBara FooFFXVbfBarl782;FooFFXVbfBara
FooFFXVbfBarl678;FooFFXVbfBara FooFFXVbfBarl725;
FooFFXVbfBara FooFFXVbfBarl719;FooFFXVbfBara
FooFFXVbfBarl715;FooFFXVbfBara FooFFXVbfBarl778;
FooFFXVbfBara FooFFXVbfBarl780;FooFFXVbfBara
FooFFXVbfBarl650;FooFFXVbfBara FooFFXVbfBarl768;
FooFFXVbfBara FooFFXVbfBarl754;FooFFXVbfBara
FooFFXVbfBarl771;FooFFXVbfBarl259*FooFFXVbfBarl723;
#if FFXCONF_NORSUPPORT || FFXCONF_ISWFSUPPORT
FooFFXVbfBara FooFFXVbfBarl661;FooFFXVbfBara
FooFFXVbfBarl625;FooFFXVbfBara FooFFXVbfBarl760;
FooFFXVbfBara FooFFXVbfBarl663;FooFFXVbfBara
FooFFXVbfBarl658;FooFFXVbfBara FooFFXVbfBarl698;
#endif
#if FFXCONF_NANDSUPPORT
FooFFXVbfBara FooFFXVbfBarl669;FooFFXVbfBara
FooFFXVbfBarl656;FooFFXVbfBara FooFFXVbfBarl636;
FooFFXVbfBara FooFFXVbfBarl690;FooFFXVbfBara
FooFFXVbfBarl784;FooFFXVbfBara FooFFXVbfBarl700;
FooFFXVbfBara FooFFXVbfBarl703;FooFFXVbfBara
FooFFXVbfBarl710;FooFFXVbfBara FooFFXVbfBarl713;
FooFFXVbfBara FooFFXVbfBarl769;FooFFXVbfBara
FooFFXVbfBarl689;FooFFXVbfBara FooFFXVbfBarl626;
FooFFXVbfBara FooFFXVbfBarl747;FooFFXVbfBara
FooFFXVbfBarl695;FooFFXVbfBara FooFFXVbfBarl649;
FooFFXVbfBara FooFFXVbfBarl662;FooFFXVbfBara
FooFFXVbfBarl644;FooFFXVbfBara FooFFXVbfBarl643;
FooFFXVbfBara FooFFXVbfBarl763;FooFFXVbfBara
FooFFXVbfBarl665;FooFFXVbfBara FooFFXVbfBarl686;
FooFFXVbfBara FooFFXVbfBarl624;FooFFXVbfBara
FooFFXVbfBarl745;FooFFXVbfBara FooFFXVbfBarl732;
FooFFXVbfBara FooFFXVbfBarl683;FooFFXVbfBara
FooFFXVbfBarl751;FooFFXVbfBara FooFFXVbfBarl727;
FooFFXVbfBara FooFFXVbfBarl642;FooFFXVbfBara
FooFFXVbfBarl759;FooFFXVbfBara FooFFXVbfBarl795;
FooFFXVbfBara FooFFXVbfBarl779;FooFFXVbfBara
FooFFXVbfBarl620;FooFFXVbfBara FooFFXVbfBarl666;
FooFFXVbfBara FooFFXVbfBarl802;FooFFXVbfBara
FooFFXVbfBarl785;FooFFXVbfBara FooFFXVbfBarl730;
FooFFXVbfBara FooFFXVbfBarl637;FooFFXVbfBara
FooFFXVbfBarl718;FooFFXVbfBara FooFFXVbfBarl688;
FooFFXVbfBara FooFFXVbfBarl697;FooFFXVbfBara
FooFFXVbfBarl750;FooFFXVbfBara FooFFXVbfBarl741;
#endif
}FooFFXVbfBarl257;FooFFXVbfBarb FooFFXVbfBarf{FooFFXVbfBara
FooFFXVbfBarl793;FooFFXVbfBara FooFFXVbfBarl164;
FooFFXVbfBara FooFFXVbfBarl788;FooFFXVbfBara
FooFFXVbfBarl792;FooFFXVbfBara FooFFXVbfBarl657;
FooFFXVbfBara FooFFXVbfBarl623;FooFFXVbfBara
FooFFXVbfBarl800;FooFFXVbfBara FooFFXVbfBarl777;
FooFFXVbfBara FooFFXVbfBarl708;FooFFXVbfBara
FooFFXVbfBarl347;}FooFFXVbfBarl255;FooFFXVbfBarh
FooFFXVbfBarl629(FooFFXVbfBarc FooFFXVbfBarl220* FooFFXVbfBarl72,FooFFXVbfBarj FooFFXVbfBarl77);
FooFFXVbfBarh FooFFXVbfBarl781(FooFFXVbfBarc FooFFXVbfBarl223*FooFFXVbfBarl72,FooFFXVbfBarj FooFFXVbfBarl77);FooFFXVbfBarh FooFFXVbfBarl640( FooFFXVbfBarc FooFFXVbfBarl163*FooFFXVbfBarl786, FooFFXVbfBarj FooFFXVbfBarl77);FooFFXVbfBarh
FooFFXVbfBarl672(FooFFXVbfBarc FooFFXVbfBarl160* FooFFXVbfBarl739,FooFFXVbfBarj FooFFXVbfBarl77);
FooFFXVbfBarh FooFFXVbfBarl696(FooFFXVbfBarc FooFFXVbfBarl257*FooFFXVbfBarl72,FooFFXVbfBarj FooFFXVbfBarl77);FooFFXVbfBarh FooFFXVbfBarl684( FooFFXVbfBarc FooFFXVbfBarl255*FooFFXVbfBarl72, FooFFXVbfBarj FooFFXVbfBarl77);
#endif
#define FooFFXVbfBarl884 (0x7FFFFFFFL)
#define FooFFXVbfBarl706 (16L  *  1024L)
#define FooFFXVbfBarl289 (2)
#define FooFFXVbfBarl93 ( FooFFXVbfBarl126  *    FooFFXVbfBarl289)
#define FooFFXVbfBarl179 (5)
#define FooFFXVbfBarl182 (16)
#define FooFFXVbfBarl952 (0x00200000UL)
#define FooFFXVbfBarl1247 (0x80000000UL)
#define FooFFXVbfBarl1038 (65)
#define FooFFXVbfBarl1025 (50)
FooFFXVbfBarb FooFFXVbfBarl69 FooFFXVbfBarl38;
#define FooFFXVbfBarl210 ( FooFFXVbfBarl182 +   FooFFXVbfBarl179)
#define FooFFXVbfBarl132 (255)
FooFFXVbfBarb FooFFXVbfBara FooFFXVbfBarl50;
#define FooFFXVbfBarl188 D_UINT32_MAX
#if VBF_PARTITION_MAX_UNITS > D_UINT16_MAX
FooFFXVbfBarb FooFFXVbfBara FooFFXVbfBarl32;
#else
FooFFXVbfBarb FooFFXVbfBari FooFFXVbfBarl32;
#endif
FooFFXVbfBarb FooFFXVbfBarl61{FooFFXVbfBarl545=0,
FooFFXVbfBarl231,FooFFXVbfBarl415,FooFFXVbfBarl368,
FooFFXVbfBarl809=7}FooFFXVbfBarl1190;FooFFXVbfBarb
FooFFXVbfBarf{FooFFXVbfBare FooFFXVbfBarl175:3;
FooFFXVbfBare FooFFXVbfBarl33:5;FooFFXVbfBare
FooFFXVbfBarl275:13;FooFFXVbfBare FooFFXVbfBarl219:11;}
FooFFXVbfBarl91;FooFFXVbfBarb FooFFXVbfBarf{FooFFXVbfBara
FooFFXVbfBarl169;FooFFXVbfBarl32 FooFFXVbfBarl43;
FooFFXVbfBarl38 FooFFXVbfBarl288;FooFFXVbfBarl38
FooFFXVbfBarl336;FooFFXVbfBari FooFFXVbfBarl202;
FooFFXVbfBari FooFFXVbfBarl144;FooFFXVbfBari
FooFFXVbfBarl147;FooFFXVbfBara FooFFXVbfBarl278;}
FooFFXVbfBarl198;
#if FFXCONF_NANDSUPPORT
FooFFXVbfBarb FooFFXVbfBare FooFFXVbfBarl55 FooFFXVbfBarl63
;
#define FooFFXVbfBarl305( FooFFXVbfBarl201) (((   FooFFXVbfBarl201) + FooFFXVbfBarl30 ( FooFFXVbfBarl63)  *    FooFFXVbfBarl137 - 1) / ( FooFFXVbfBarl30 ( FooFFXVbfBarl63  )  *  FooFFXVbfBarl137))
#if ( FooFFXVbfBarl93 > VBF_MAX_BLOCK_SIZE  *    FooFFXVbfBarl137)
#error  \
"FFX: A complete discard set must fit in one allocation block."
#endif
FooFFXVbfBarb FooFFXVbfBarl63 FooFFXVbfBarl317[DCLMAX( VBF_MAX_BLOCK_SIZE/FooFFXVbfBarl30(FooFFXVbfBarl63), FooFFXVbfBarl305(FooFFXVbfBarl93+FooFFXVbfBarl179* FooFFXVbfBarl126))];
#define FooFFXVbfBarl619 (0x1FF0)
#define FooFFXVbfBarl526 1
#endif
FooFFXVbfBarb FooFFXVbfBarf{FooFFXVbfBarj FooFFXVbfBarl130;
FooFFXVbfBarl134 FooFFXVbfBarl171;FooFFXVbfBarl50
FooFFXVbfBarl59;FooFFXVbfBari FooFFXVbfBarl215;PDCLMUTEX
FooFFXVbfBarl401;DCLTIMESTAMP FooFFXVbfBarl702;
FooFFXVbfBarl198 FooFFXVbfBarl53[FooFFXVbfBarl210];
FooFFXVbfBarl38 FooFFXVbfBarl226;FooFFXVbfBarl38
FooFFXVbfBarl353;FooFFXVbfBarl38 FooFFXVbfBarl159;
FooFFXVbfBarf FooFFXVbfBarl101*FooFFXVbfBarl183;
#if FFXCONF_NANDSUPPORT
FooFFXVbfBarl317 FooFFXVbfBarl242;FooFFXVbfBari
FooFFXVbfBarl269;FooFFXVbfBari FooFFXVbfBarl375;
FooFFXVbfBarj FooFFXVbfBarl203;
#endif
FooFFXVbfBari FooFFXVbfBarl328;FooFFXVbfBari
FooFFXVbfBarl307;FooFFXVbfBara FooFFXVbfBarl178;
FooFFXVbfBarl91 FooFFXVbfBarl208[FooFFXVbfBarl93];}
FooFFXVbfBaru;
#define FooFFXVbfBarl386( FooFFXVbfBarl108, FooFFXVbfBarl33  ) FooFFXVbfBarl290 { ( FooFFXVbfBarl108)-> FooFFXVbfBarl53[ \
( FooFFXVbfBarl33)]. FooFFXVbfBarl278 = TRUE; ++(   FooFFXVbfBarl108)-> FooFFXVbfBarl178; } FooFFXVbfBarl153 (0  )
#define FooFFXVbfBarl524( FooFFXVbfBarl108) ((   FooFFXVbfBarl108)-> FooFFXVbfBarl178 != 0)
FooFFXVbfBarb FooFFXVbfBarf FooFFXVbfBarl1012
FooFFXVbfBarl225;FooFFXVbfBarb FooFFXVbfBarf
FooFFXVbfBarl1149*FooFFXVbfBarl141;FooFFXVbfBarb
FooFFXVbfBarf FooFFXVbfBarl1011*FooFFXVbfBarl168;
FooFFXVbfBarb FooFFXVbfBarf{FooFFXVbfBarl134
FooFFXVbfBarl344;}FooFFXVbfBarl319;FooFFXVbfBarb
FooFFXVbfBarf{FooFFXVbfBare FooFFXVbfBarl864;FooFFXVbfBare
FooFFXVbfBarl844;FooFFXVbfBare FooFFXVbfBarl822;
FooFFXVbfBare FooFFXVbfBarl834;}FooFFXVbfBarl280;
FooFFXVbfBarb FFXSTATUS( *FooFFXVbfBarl318)(FooFFXVbfBarc FooFFXVbfBarf FooFFXVbfBarl101*FooFFXVbfBark,FooFFXVbfBaru* FooFFXVbfBarr,FooFFXVbfBara FooFFXVbfBarl511,FooFFXVbfBara FooFFXVbfBarl411);FooFFXVbfBarb FooFFXVbfBarf
FooFFXVbfBarl101{FooFFXVbfBari FooFFXVbfBarl359;
FooFFXVbfBari FooFFXVbfBarl362;FooFFXVbfBara
FooFFXVbfBarl412;FooFFXVbfBara FooFFXVbfBarl218;
FooFFXVbfBarl141 FooFFXVbfBarl192;FooFFXVbfBari
FooFFXVbfBarl180;FooFFXVbfBari FooFFXVbfBarl537;
FooFFXVbfBara FooFFXVbfBarl233;FooFFXVbfBarl205
FooFFXVbfBarl356;FooFFXVbfBarl32 FooFFXVbfBarl131;
FooFFXVbfBarl32 FooFFXVbfBarl798;FooFFXVbfBarl32
FooFFXVbfBarl213;FooFFXVbfBarl32 FooFFXVbfBarl165;
FooFFXVbfBari FooFFXVbfBarl129;FooFFXVbfBari
FooFFXVbfBarl937;FooFFXVbfBarl50 FooFFXVbfBarl211;
FooFFXVbfBarl50 FooFFXVbfBarl627[DCL_PAD( VBF_PARTITION_MAX_UNITS,FooFFXVbfBarl50)];FooFFXVbfBare
FooFFXVbfBarl64;FooFFXVbfBare FooFFXVbfBarl981;
FooFFXVbfBarl225*FooFFXVbfBarl293;FooFFXVbfBarl45
FooFFXVbfBarm;PDCLMUTEX FooFFXVbfBarl419;FooFFXVbfBara
FooFFXVbfBarl808;FooFFXVbfBaro*FooFFXVbfBarl392;
FooFFXVbfBaro*FooFFXVbfBarl365;FooFFXVbfBarl318
FooFFXVbfBarl566;FooFFXVbfBarl319*FooFFXVbfBarl272;
FooFFXVbfBare FooFFXVbfBarl594;FooFFXVbfBare
FooFFXVbfBarl568;FooFFXVbfBare FooFFXVbfBarl582;
#if FFX_COMPACTIONMODEL != FFX_COMPACT_NONE
FooFFXVbfBarl134 FooFFXVbfBarl422;FooFFXVbfBarl111
DCLTIMESTAMP FooFFXVbfBarl757;FooFFXVbfBarl111 DCLTIMESTAMP
FooFFXVbfBarl541;FooFFXVbfBara FooFFXVbfBarl429;
FooFFXVbfBara FooFFXVbfBarl484;
#endif
#if FFXCONF_STATS_COMPACTION
FooFFXVbfBarl163 FooFFXVbfBarl176;
#endif
#if VBF_WRITEINT_ENABLED
FooFFXVbfBarl168 FooFFXVbfBarl36;
#endif
#if FFXCONF_NANDSUPPORT
FooFFXVbfBaro*FooFFXVbfBarl366;
#endif
FooFFXVbfBare FooFFXVbfBarl339:1;FooFFXVbfBare
FooFFXVbfBarl327:1;FooFFXVbfBare FooFFXVbfBarl722:1;
FooFFXVbfBare FooFFXVbfBarl287:1;FooFFXVbfBare
FooFFXVbfBarl161:1;FooFFXVbfBare FooFFXVbfBarl437:1;
FooFFXVbfBare FooFFXVbfBarl559:1;FooFFXVbfBare
FooFFXVbfBarl245:1;
#if FFXCONF_QUICKMOUNTSUPPORT
FooFFXVbfBare FooFFXVbfBarl517:1;
#endif
}FooFFXVbfBarl29,FooFFXVbfBarl28, *FooFFXVbfBarl102;
#if FFXCONF_QUICKMOUNTSUPPORT
FooFFXVbfBarb FooFFXVbfBara*FooFFXVbfBarl413;
#define FooFFXVbfBarl1003 0x01
#define FooFFXVbfBarl595 FooFFXVbfBarl30( FooFFXVbfBara)
FooFFXVbfBarl61{FooFFXVbfBarl803=0,FooFFXVbfBarl360,
FooFFXVbfBarl960,FooFFXVbfBarl920,FooFFXVbfBarl951,
FooFFXVbfBarl934,FooFFXVbfBarl917,FooFFXVbfBarl813,
FooFFXVbfBarl474};
#endif
#define FooFFXVbfBarl668 (( FooFFXVbfBarl69)(0xFF))
#define FooFFXVbfBarl1123 (( FooFFXVbfBari)(0xFFFF))
#define FooFFXVbfBarl574 (( FooFFXVbfBara)(0xFFFFFFFFL))
#define FooFFXVbfBarl475( FooFFXVbfBarl67, FooFFXVbfBarl105  ) (( FooFFXVbfBarl50)(( FooFFXVbfBarl67) / FooFFXVbfBarl172  ( FooFFXVbfBarl105)))
#define FooFFXVbfBarl343 ( FooFFXVbfBarl188 - 0)
#define FooFFXVbfBarl433 ( FooFFXVbfBarl188 - 1)
#define FooFFXVbfBarl1040 (16)
#define FooFFXVbfBarl1010 (0)
#define FooFFXVbfBarl1007 (0x00000001)
#define FooFFXVbfBarl1034 (0x00000100)
#define FooFFXVbfBarl1016 (0x00010000)
#define FooFFXVbfBarl1023 (0x01000000)
#define FooFFXVbfBarl379 ( D_UINT32_MAX-1)
#define FooFFXVbfBarl499 ( D_UINT32_MAX)
FooFFXVbfBarj FooFFXVbfBarl805(FooFFXVbfBarc FooFFXVbfBarl29  *FooFFXVbfBars,FooFFXVbfBarl32 FooFFXVbfBarl33);
FooFFXVbfBarj FooFFXVbfBarl534(FooFFXVbfBarc FooFFXVbfBarl29  *FooFFXVbfBars,FooFFXVbfBarc FooFFXVbfBaru*FooFFXVbfBarw, FooFFXVbfBarc FooFFXVbfBarl91*FooFFXVbfBarl273);
FooFFXVbfBarh FooFFXVbfBarl325(FooFFXVbfBarc FooFFXVbfBarl29  *FooFFXVbfBars,FooFFXVbfBarc FooFFXVbfBaru*FooFFXVbfBarw, FooFFXVbfBarc FooFFXVbfBarl37*FooFFXVbfBarl442);FFXSTATUS
FooFFXVbfBarl872(FooFFXVbfBarl29*FooFFXVbfBarl49);
FooFFXVbfBarh FooFFXVbfBarl679(FooFFXVbfBarl29* FooFFXVbfBarl49,FooFFXVbfBara FooFFXVbfBarl65);
FooFFXVbfBaru*FooFFXVbfBarl349(FooFFXVbfBarl29* FooFFXVbfBarl49,FooFFXVbfBarl50 FooFFXVbfBarl59, FooFFXVbfBarl281 FooFFXVbfBarl383);FFXSTATUS
FooFFXVbfBarl903(FooFFXVbfBarl28*FooFFXVbfBark);
FooFFXVbfBarj FooFFXVbfBarl956(FooFFXVbfBaru*FooFFXVbfBarr);
FooFFXVbfBarh FooFFXVbfBarl337(FooFFXVbfBaru*FooFFXVbfBarr);
FooFFXVbfBarh FooFFXVbfBarl423(FooFFXVbfBarc FooFFXVbfBaru* FooFFXVbfBarr);FooFFXVbfBarh FooFFXVbfBarl416(FooFFXVbfBarc FooFFXVbfBaru*FooFFXVbfBarr);FooFFXVbfBarl50
FooFFXVbfBarl326(FooFFXVbfBarc FooFFXVbfBarl29* FooFFXVbfBarl49,FooFFXVbfBarl32 FooFFXVbfBarl43);
FooFFXVbfBarh FooFFXVbfBarl382(FooFFXVbfBarl29* FooFFXVbfBarl49,FooFFXVbfBarl32 FooFFXVbfBarl43, FooFFXVbfBarl50 FooFFXVbfBarl59);FooFFXVbfBarj
FooFFXVbfBarl527(FooFFXVbfBarc FooFFXVbfBaru*FooFFXVbfBarr, FooFFXVbfBarl32 FooFFXVbfBarl43,FooFFXVbfBarl38* FooFFXVbfBarl890);FooFFXVbfBara FooFFXVbfBarl919( FooFFXVbfBarc FooFFXVbfBaru*FooFFXVbfBarr);FooFFXVbfBarl38
FooFFXVbfBarl291(FooFFXVbfBarc FooFFXVbfBaru*FooFFXVbfBarr, FooFFXVbfBarl38 FooFFXVbfBarl33);FooFFXVbfBarl38
FooFFXVbfBarl552(FooFFXVbfBaru*FooFFXVbfBarr, FooFFXVbfBarl38 FooFFXVbfBarl33);FooFFXVbfBarl38
FooFFXVbfBarl717(FooFFXVbfBaru*FooFFXVbfBarr, FooFFXVbfBarl38 FooFFXVbfBarl33);FooFFXVbfBarj
FooFFXVbfBarl399(FooFFXVbfBarc FooFFXVbfBarl68* FooFFXVbfBarl76);FooFFXVbfBarj FooFFXVbfBarl1232( FooFFXVbfBarc FooFFXVbfBarl29*FooFFXVbfBars);FooFFXVbfBarj
FooFFXVbfBarl1210(FooFFXVbfBarc FooFFXVbfBarl29* FooFFXVbfBars);FooFFXVbfBari FooFFXVbfBarl378(FooFFXVbfBarc FooFFXVbfBarl29*FooFFXVbfBars);FooFFXVbfBara
FooFFXVbfBarl172(FooFFXVbfBarc FooFFXVbfBarl29* FooFFXVbfBars);FooFFXVbfBarl32 FooFFXVbfBarl298( FooFFXVbfBarc FooFFXVbfBarl29*FooFFXVbfBars);FooFFXVbfBara
FooFFXVbfBarl364(FooFFXVbfBarc FooFFXVbfBarl29* FooFFXVbfBars);FooFFXVbfBara FooFFXVbfBarl376(FooFFXVbfBarc FooFFXVbfBarl29*FooFFXVbfBars);FFXSTATUS FooFFXVbfBarl978( FooFFXVbfBarl28*FooFFXVbfBark,FooFFXVbfBarl68* FooFFXVbfBarl76);FFXSTATUS FooFFXVbfBarl542(FooFFXVbfBarl28  *FooFFXVbfBark,FooFFXVbfBarl68*FooFFXVbfBarl76);
FooFFXVbfBarl35 FooFFXVbfBarl855(FooFFXVbfBarl29* FooFFXVbfBars,FooFFXVbfBaru*FooFFXVbfBarr,FooFFXVbfBara FooFFXVbfBarl250,FooFFXVbfBara FooFFXVbfBarl47, FooFFXVbfBarc FooFFXVbfBarh*FooFFXVbfBarz);FooFFXVbfBarl35
FooFFXVbfBarl660(FooFFXVbfBarc FooFFXVbfBarl29* FooFFXVbfBars,FooFFXVbfBaru*FooFFXVbfBarw,FooFFXVbfBara FooFFXVbfBarl145,FooFFXVbfBara FooFFXVbfBarl47, FooFFXVbfBaro*FooFFXVbfBarl116);FooFFXVbfBarj
FooFFXVbfBarl856(FooFFXVbfBarl29*FooFFXVbfBars, FooFFXVbfBaru*FooFFXVbfBarr,FooFFXVbfBari FooFFXVbfBarl1071 ,FooFFXVbfBari FooFFXVbfBarl1116);FooFFXVbfBarl35
FooFFXVbfBarl614(FooFFXVbfBarl29*FooFFXVbfBars, FooFFXVbfBaru*FooFFXVbfBarr);FooFFXVbfBarj FooFFXVbfBarl983
(FooFFXVbfBarl29*FooFFXVbfBars);FFXSTATUS FooFFXVbfBarl432( FooFFXVbfBaru*FooFFXVbfBarw);FooFFXVbfBarh FooFFXVbfBarl361
(FooFFXVbfBarc FooFFXVbfBarl28*FooFFXVbfBark);FooFFXVbfBarh
FooFFXVbfBarl334(FooFFXVbfBarc FooFFXVbfBarl28* FooFFXVbfBark);FooFFXVbfBarh FooFFXVbfBarl752(FooFFXVbfBarc FooFFXVbfBaru*FooFFXVbfBarl78);FooFFXVbfBarh
FooFFXVbfBarl682(FooFFXVbfBarc FooFFXVbfBaru* FooFFXVbfBarl806[],FooFFXVbfBare FooFFXVbfBarl330, FooFFXVbfBarj FooFFXVbfBarl972);
#if FFXCONF_NANDSUPPORT
FooFFXVbfBarj FooFFXVbfBarl463(FooFFXVbfBarc FooFFXVbfBarl63  *FooFFXVbfBarl80,FooFFXVbfBare FooFFXVbfBarl55 FooFFXVbfBarl67);FooFFXVbfBarh FooFFXVbfBarl736( FooFFXVbfBarl63*FooFFXVbfBarl80,FooFFXVbfBare FooFFXVbfBarl55 FooFFXVbfBarl67);FooFFXVbfBarh
FooFFXVbfBarl549(FooFFXVbfBarl63*FooFFXVbfBarl80, FooFFXVbfBare FooFFXVbfBarl55 FooFFXVbfBarl67);
#endif
FooFFXVbfBarh FooFFXVbfBarl819(FooFFXVbfBarl28* FooFFXVbfBark,FooFFXVbfBarl55 FooFFXVbfBarl348);
FooFFXVbfBarh FooFFXVbfBarl455(FooFFXVbfBarl28* FooFFXVbfBark,FooFFXVbfBarc FooFFXVbfBaru*FooFFXVbfBarr);
FFXSTATUS FooFFXVbfBarl716(FooFFXVbfBarc FooFFXVbfBarl28* FooFFXVbfBark,FooFFXVbfBarl50 FooFFXVbfBarl149, FooFFXVbfBarl280*FooFFXVbfBarl393);
#if FFXCONF_STATS_VBFREGION
FooFFXVbfBarj FooFFXVbfBarl921(FooFFXVbfBarx FooFFXVbfBarp, FooFFXVbfBarl160*FooFFXVbfBarl72,FooFFXVbfBarj FooFFXVbfBarl384);FooFFXVbfBarj FooFFXVbfBarl774( FooFFXVbfBarx FooFFXVbfBarp);
#endif
#if FFXCONF_NORSUPPORT || FFXCONF_ISWFSUPPORT
FFXSTATUS FooFFXVbfBarl976(FooFFXVbfBarc FooFFXVbfBarl28* FooFFXVbfBark,FooFFXVbfBaru*FooFFXVbfBarr,FooFFXVbfBara FooFFXVbfBarl145,FooFFXVbfBara FooFFXVbfBarl47);FFXSTATUS
FooFFXVbfBarl863(FooFFXVbfBarc FooFFXVbfBarl28* FooFFXVbfBark,FooFFXVbfBaru*FooFFXVbfBarr,FooFFXVbfBarl38 FooFFXVbfBarl123,FooFFXVbfBarc FooFFXVbfBaro* FooFFXVbfBarl611);FFXSTATUS FooFFXVbfBarl964(FooFFXVbfBarc FooFFXVbfBarl28*FooFFXVbfBark,FooFFXVbfBarl32 FooFFXVbfBarl170,FooFFXVbfBarl91*FooFFXVbfBarc FooFFXVbfBarl258,FooFFXVbfBare FooFFXVbfBarl930);
FooFFXVbfBarj FooFFXVbfBarl940(FooFFXVbfBaru*FooFFXVbfBarr, FooFFXVbfBarc FooFFXVbfBaro*FooFFXVbfBarl645);
#endif
#if FFXCONF_NANDSUPPORT
FFXSTATUS FooFFXVbfBarl969(FooFFXVbfBarc FooFFXVbfBarl28* FooFFXVbfBark,FooFFXVbfBaru*FooFFXVbfBarr,FooFFXVbfBara FooFFXVbfBarl145,FooFFXVbfBara FooFFXVbfBarl47);
FooFFXVbfBarj FooFFXVbfBarl515(FooFFXVbfBarl28* FooFFXVbfBark,FooFFXVbfBaru*FooFFXVbfBarr);FFXSTATUS
FooFFXVbfBarl589(FooFFXVbfBarl28*FooFFXVbfBark, FooFFXVbfBaru*FooFFXVbfBarr,FooFFXVbfBarj FooFFXVbfBarl633);
FFXSTATUS FooFFXVbfBarl546(FooFFXVbfBarl28*FooFFXVbfBark, FooFFXVbfBaru*FooFFXVbfBarr);FooFFXVbfBarj FooFFXVbfBarl848
(FooFFXVbfBarc FooFFXVbfBarl28*FooFFXVbfBark,FooFFXVbfBaru* FooFFXVbfBarr);
#endif
#if FFXCONF_QUICKMOUNTSUPPORT
FFXSTATUS FooFFXVbfBarl957(FooFFXVbfBarc FooFFXVbfBarl102 FooFFXVbfBarl49);FFXSTATUS FooFFXVbfBarl982( FooFFXVbfBarl102 FooFFXVbfBarl49);FFXSTATUS FooFFXVbfBarl701
(FooFFXVbfBarc FooFFXVbfBarl102 FooFFXVbfBarl49);
#endif
#if ( VBF_PARTITION_MAX_UNITS < FooFFXVbfBarl182)
#error "Too few logical units in VBF_PARTITION_MAX_UNITS!"
#endif
#ifdef FooFFXVbfBarl1256
#error "VBF_BLOCK_SIZE is obsolete.  Do not define it."
#endif
#if ( VBF_MAX_BLOCK_SIZE < VBF_MIN_BLOCK_SIZE)
#error "Maximum VBF block size is less than minimum!"
#endif
#if ( FFX_MAX_DISKS < 1)
#error "The number of units cannot be zero."
#endif
#if ( FooFFXVbfBarl93 > 4096)
#error                  \
"REGION_CACHE_MAX_ALLOCS is too big."
#endif
#endif
#if FFXCONF_VBFSUPPORT
FooFFXVbfBarl35 FooFFXVbfBarl1415(FooFFXVbfBarl45 FooFFXVbfBarm,FooFFXVbfBara FooFFXVbfBarl39,FooFFXVbfBarl68  *FooFFXVbfBarl76);FFXSTATUS FooFFXVbfBarl1447( FooFFXVbfBarl45 FooFFXVbfBarm,FooFFXVbfBara FooFFXVbfBarl39 ,FooFFXVbfBarc FooFFXVbfBarl68*FooFFXVbfBarl76, FooFFXVbfBarj FooFFXVbfBarl287,FooFFXVbfBarj FooFFXVbfBarl277);FooFFXVbfBare FooFFXVbfBarl1442( FooFFXVbfBarl45 FooFFXVbfBarm,FooFFXVbfBarc FooFFXVbfBarl68  *FooFFXVbfBarl76);FooFFXVbfBarl141 FooFFXVbfBarl1440( FooFFXVbfBarl45 FooFFXVbfBarm);FooFFXVbfBarh
FooFFXVbfBarl1401(FooFFXVbfBarl141 FooFFXVbfBarl192);
FFXSTATUS FooFFXVbfBarl1428(FooFFXVbfBarl141 FooFFXVbfBarl192,FooFFXVbfBarl87 FooFFXVbfBarl98, FooFFXVbfBarh*FooFFXVbfBarz,FooFFXVbfBara FooFFXVbfBarl57);
FFXSTATUS FooFFXVbfBarl1417(FooFFXVbfBarl141 FooFFXVbfBarl192,FooFFXVbfBarl87 FooFFXVbfBarl98, FooFFXVbfBarc FooFFXVbfBarh*FooFFXVbfBarz,FooFFXVbfBara FooFFXVbfBarl57);FFXSTATUS FooFFXVbfBarl1369(FooFFXVbfBarc FooFFXVbfBarl28*FooFFXVbfBark,FooFFXVbfBarl32 FooFFXVbfBarl43,FooFFXVbfBarc FooFFXVbfBarl68* FooFFXVbfBarl76);FooFFXVbfBarl35 FooFFXVbfBarl1348( FooFFXVbfBarc FooFFXVbfBarl28*FooFFXVbfBark,FooFFXVbfBarl32 FooFFXVbfBarl43,FooFFXVbfBare FooFFXVbfBarl295, FooFFXVbfBare FooFFXVbfBarl1186,FooFFXVbfBare FooFFXVbfBarl1468,FooFFXVbfBarc FooFFXVbfBarh*FooFFXVbfBarz );FooFFXVbfBarl35 FooFFXVbfBarl1362(FooFFXVbfBarc FooFFXVbfBarl28*FooFFXVbfBark,FooFFXVbfBarl32 FooFFXVbfBarl43,FooFFXVbfBare FooFFXVbfBarl295, FooFFXVbfBare FooFFXVbfBarl1186,FooFFXVbfBarh*FooFFXVbfBarz );FooFFXVbfBarl35 FooFFXVbfBarl853(FooFFXVbfBarc FooFFXVbfBarl28*FooFFXVbfBark,FooFFXVbfBarl32 FooFFXVbfBarl43,FooFFXVbfBarl68*FooFFXVbfBarl76);
FooFFXVbfBarl35 FooFFXVbfBarl1314(FooFFXVbfBarc FooFFXVbfBarl28*FooFFXVbfBark,FooFFXVbfBarl32 FooFFXVbfBarl43,FooFFXVbfBare FooFFXVbfBarl391, FooFFXVbfBare FooFFXVbfBarl402,FooFFXVbfBarl91[]);FFXSTATUS
FooFFXVbfBarl1327(FooFFXVbfBarc FooFFXVbfBarl28* FooFFXVbfBark,FooFFXVbfBarl32 FooFFXVbfBarl43,FooFFXVbfBare FooFFXVbfBarl391,FooFFXVbfBare FooFFXVbfBarl402);
FooFFXVbfBarl35 FooFFXVbfBarl1305(FooFFXVbfBarc FooFFXVbfBarl28*FooFFXVbfBark,FooFFXVbfBarl32 FooFFXVbfBarl43,FooFFXVbfBare FooFFXVbfBarl391, FooFFXVbfBare FooFFXVbfBarl402);FFXSTATUS FooFFXVbfBarl1423
(FooFFXVbfBarc FooFFXVbfBarl28*FooFFXVbfBark, FooFFXVbfBarl32 FooFFXVbfBarl170,FooFFXVbfBare FooFFXVbfBarl1336,FooFFXVbfBarl32 FooFFXVbfBarl197, FooFFXVbfBare FooFFXVbfBarl1480);
#if FFXCONF_NANDSUPPORT
FFXSTATUS FooFFXVbfBarl1412(FooFFXVbfBarc FooFFXVbfBarl28* FooFFXVbfBark,FooFFXVbfBarl32 FooFFXVbfBarl43,FooFFXVbfBare FooFFXVbfBarl295);FFXSTATUS FooFFXVbfBarl1462(FooFFXVbfBarc FooFFXVbfBarl28*FooFFXVbfBark,FooFFXVbfBarl32 FooFFXVbfBarl43,FooFFXVbfBare FooFFXVbfBarl295);
FooFFXVbfBarj FooFFXVbfBarl1426(FooFFXVbfBarc FooFFXVbfBarl28*FooFFXVbfBark);
#endif
#endif
#ifndef FooFFXVbfBarl310
#define FooFFXVbfBarl310
FooFFXVbfBarb FooFFXVbfBarl61{FooFFXVbfBarl1184=
DCLIO_MAKEFUNCTION(PRODUCTNUM_FLASHFX,0x000),
FooFFXVbfBarl464=DCLIO_MAKEFUNCTION(PRODUCTNUM_FLASHFX, 0x100),FooFFXVbfBarl486,FooFFXVbfBarl467,FooFFXVbfBarl480,
FooFFXVbfBarl440,FooFFXVbfBarl509,FooFFXVbfBarl512,
FooFFXVbfBarl495,FooFFXVbfBarl506,FooFFXVbfBarl487,
FooFFXVbfBarl446,FooFFXVbfBarl505,FooFFXVbfBarl493,
FooFFXVbfBarl436,FooFFXVbfBarl492,FooFFXVbfBarl490,
FooFFXVbfBarl342,FooFFXVbfBarl450,FooFFXVbfBarl496,
FooFFXVbfBarl483,FooFFXVbfBarl427,FooFFXVbfBarl454,
FooFFXVbfBarl508,FooFFXVbfBarl430,FooFFXVbfBarl482,
FooFFXVbfBarl485,FooFFXVbfBarl501,FooFFXVbfBarl426,
FooFFXVbfBarl435,FooFFXVbfBarl476,FooFFXVbfBarl445,
FooFFXVbfBarl461,FooFFXVbfBarl1119=DCLIO_MAKEFUNCTION( PRODUCTNUM_FLASHFX,0x200),FooFFXVbfBarl1112,
FooFFXVbfBarl1172,FooFFXVbfBarl1153,FooFFXVbfBarl1163,
FooFFXVbfBarl1198,FooFFXVbfBarl1230,FooFFXVbfBarl1048,
FooFFXVbfBarl1146,FooFFXVbfBarl1139,FooFFXVbfBarl1093,
FooFFXVbfBarl1221,FooFFXVbfBarl1168,FooFFXVbfBarl1183,
FooFFXVbfBarl1200,FooFFXVbfBarl1249,FooFFXVbfBarl1218,
FooFFXVbfBarl1099,FooFFXVbfBarl1125,FooFFXVbfBarl1191,
FooFFXVbfBarl1058=DCLIO_MAKEFUNCTION(PRODUCTNUM_FLASHFX, 0x280),FooFFXVbfBarl1085,FooFFXVbfBarl1128=
DCLIO_MAKEFUNCTION(PRODUCTNUM_FLASHFX,0x300),
FooFFXVbfBarl1068,FooFFXVbfBarl1054,FooFFXVbfBarl1155,
FooFFXVbfBarl1055,FooFFXVbfBarl1173,FooFFXVbfBarl1142,
FooFFXVbfBarl1136,FooFFXVbfBarl1087,FooFFXVbfBarl1127,
FooFFXVbfBarl1181,FooFFXVbfBarl1101,FooFFXVbfBarl1120=
DCLIO_MAKEFUNCTION(PRODUCTNUM_FLASHFX,0x380),
FooFFXVbfBarl1188,FooFFXVbfBarl1211,FooFFXVbfBarl1258,
FooFFXVbfBarl1113,FooFFXVbfBarl1222,FooFFXVbfBarl1161,
FooFFXVbfBarl1076,FooFFXVbfBarl1117=DCLIO_MAKEFUNCTION( PRODUCTNUM_FLASHFX,0x500),FooFFXVbfBarl1082,
FooFFXVbfBarl1240,FooFFXVbfBarl1098,FooFFXVbfBarl1162,
FooFFXVbfBarl1061,FooFFXVbfBarl1159,FooFFXVbfBarl1244,
FooFFXVbfBarl1092,FooFFXVbfBarl1074,FooFFXVbfBarl1166,
FooFFXVbfBarl1169,FooFFXVbfBarl1106=DCLIO_MAKEFUNCTION( PRODUCTNUM_FLASHFX,0x600),FooFFXVbfBarl1037,
FooFFXVbfBarl1036,FooFFXVbfBarl1015,FooFFXVbfBarl1029,
FooFFXVbfBarl1041,FooFFXVbfBarl1018,FooFFXVbfBarl1032,
FooFFXVbfBarl1022,FooFFXVbfBarl1021,FooFFXVbfBarl1033,
FooFFXVbfBarl1020,FooFFXVbfBarl1030,FooFFXVbfBarl1039,
FooFFXVbfBarl1035,FooFFXVbfBarl1013,FooFFXVbfBarl1006,
FooFFXVbfBarl1028,FooFFXVbfBarl1065,FooFFXVbfBarl1177=
DCLIO_MAKEFUNCTION(PRODUCTNUM_FLASHFX,0x800),
FooFFXVbfBarl1096,FooFFXVbfBarl1185,FooFFXVbfBarl1224,
FooFFXVbfBarl1229,FooFFXVbfBarl1140,FooFFXVbfBarl1046,
FooFFXVbfBarl1167,FooFFXVbfBarl1083,FooFFXVbfBarl1170,
FooFFXVbfBarl1257,FooFFXVbfBarl1154,FooFFXVbfBarl1130,
FooFFXVbfBarl1115,FooFFXVbfBarl1235,FooFFXVbfBarl1067,
FooFFXVbfBarl1080,FooFFXVbfBarl1133,FooFFXVbfBarl1059,
FooFFXVbfBarl1079,FooFFXVbfBarl1063}FooFFXVbfBarl1014;
FooFFXVbfBarb FooFFXVbfBarl61{FooFFXVbfBarl302=0x0100,
FooFFXVbfBarl308=0x0200,FooFFXVbfBarl1094,FooFFXVbfBarl1151
,FooFFXVbfBarl1118,FooFFXVbfBarl1084,FooFFXVbfBarl1042,
FooFFXVbfBarl1212,FooFFXVbfBarl1052,FooFFXVbfBarl299=0x0300
,FooFFXVbfBarl1124,FooFFXVbfBarl1072,FooFFXVbfBarl1245,
FooFFXVbfBarl1203,FooFFXVbfBarl1056,FooFFXVbfBarl1241,
FooFFXVbfBarl1176,FooFFXVbfBarl294=0x0400,FooFFXVbfBarl1238
=0x0500,FooFFXVbfBarl1261,FooFFXVbfBarl1171,
FooFFXVbfBarl1114,FooFFXVbfBarl1147,FooFFXVbfBarl1126,
FooFFXVbfBarl1051,FooFFXVbfBarl1263,FooFFXVbfBarl1156=
0x0600,FooFFXVbfBarl1110,FooFFXVbfBarl1134,
FooFFXVbfBarl1193,FooFFXVbfBarl1138,FooFFXVbfBarl1259,
FooFFXVbfBarl1192,FooFFXVbfBarl1057,FooFFXVbfBarl1158}
FooFFXVbfBarl106;
#define FooFFXVbfBarl1050( FooFFXVbfBarl142) ((   FooFFXVbfBarl106)(( FooFFXVbfBarl142) + ( FooFFXVbfBarl294   - FooFFXVbfBarl302)))
#define FooFFXVbfBarl1220( FooFFXVbfBarl142) ((   FooFFXVbfBarl106)(( FooFFXVbfBarl142) - ( FooFFXVbfBarl299   - FooFFXVbfBarl308)))
FooFFXVbfBarb DCLIOREQUEST FooFFXVbfBart;FooFFXVbfBarb
FooFFXVbfBarf{FooFFXVbfBart FooFFXVbfBarg;FooFFXVbfBara
FooFFXVbfBary;FooFFXVbfBara FooFFXVbfBarl31;FooFFXVbfBaro*
FooFFXVbfBarl41;}FooFFXVbfBarl431;FooFFXVbfBarb
FooFFXVbfBarf{FooFFXVbfBart FooFFXVbfBarg;FooFFXVbfBara
FooFFXVbfBary;FooFFXVbfBara FooFFXVbfBarl31;FooFFXVbfBarc
FooFFXVbfBaro*FooFFXVbfBarl41;}FooFFXVbfBarl514;
FooFFXVbfBarb FooFFXVbfBarf{FooFFXVbfBart FooFFXVbfBarg;
FooFFXVbfBara FooFFXVbfBary;FooFFXVbfBara FooFFXVbfBarl31;
FooFFXVbfBaro*FooFFXVbfBarl41;FooFFXVbfBaro*FooFFXVbfBarl58
;}FooFFXVbfBarl444;FooFFXVbfBarb FooFFXVbfBarf{
FooFFXVbfBart FooFFXVbfBarg;FooFFXVbfBara FooFFXVbfBary;
FooFFXVbfBara FooFFXVbfBarl31;FooFFXVbfBarc FooFFXVbfBaro*
FooFFXVbfBarl41;FooFFXVbfBarc FooFFXVbfBaro*FooFFXVbfBarl58
;}FooFFXVbfBarl425;FooFFXVbfBarb FooFFXVbfBarf{
FooFFXVbfBart FooFFXVbfBarg;FooFFXVbfBara FooFFXVbfBarl75;
FooFFXVbfBara FooFFXVbfBarl47;}FooFFXVbfBarl472,
FooFFXVbfBarl1062,FooFFXVbfBarl434,FooFFXVbfBarl1064;
FooFFXVbfBarb FooFFXVbfBarf{FooFFXVbfBart FooFFXVbfBarg;
FooFFXVbfBara FooFFXVbfBarl75;FooFFXVbfBara FooFFXVbfBarl47
;}FooFFXVbfBarl470,FooFFXVbfBarl1243,FooFFXVbfBarl1223;
FooFFXVbfBarb FooFFXVbfBarf{FooFFXVbfBart FooFFXVbfBarg;
FooFFXVbfBara FooFFXVbfBarl75;FooFFXVbfBara FooFFXVbfBarl47
;}FooFFXVbfBarl451,FooFFXVbfBarl1121,FooFFXVbfBarl1202;
FooFFXVbfBarb FooFFXVbfBarf{FooFFXVbfBart FooFFXVbfBarg;
FooFFXVbfBara FooFFXVbfBarl75;FooFFXVbfBara FooFFXVbfBarl47
;FooFFXVbfBarj FooFFXVbfBarl460;}FooFFXVbfBarl488,
FooFFXVbfBarl1075,FooFFXVbfBarl1165;FooFFXVbfBarb
FooFFXVbfBarf{FooFFXVbfBart FooFFXVbfBarg;FooFFXVbfBarl106
FooFFXVbfBarl296;FooFFXVbfBara FooFFXVbfBary;FooFFXVbfBara
FooFFXVbfBarv;FooFFXVbfBaro*FooFFXVbfBarl41;FooFFXVbfBaro*
FooFFXVbfBarl297;FooFFXVbfBare FooFFXVbfBarl303;}
FooFFXVbfBarl1102,FooFFXVbfBarl1135;FooFFXVbfBarb
FooFFXVbfBarf{FooFFXVbfBart FooFFXVbfBarg;FooFFXVbfBarl106
FooFFXVbfBarl296;FooFFXVbfBara FooFFXVbfBary;FooFFXVbfBara
FooFFXVbfBarv;FooFFXVbfBarc FooFFXVbfBaro*FooFFXVbfBarl41;
FooFFXVbfBarc FooFFXVbfBaro*FooFFXVbfBarl297;FooFFXVbfBare
FooFFXVbfBarl303;}FooFFXVbfBarl1160,FooFFXVbfBarl1255;
FooFFXVbfBarb FooFFXVbfBarf{FooFFXVbfBart FooFFXVbfBarg;
FooFFXVbfBare FooFFXVbfBarl96;}FooFFXVbfBarl1157,
FooFFXVbfBarl1081;FooFFXVbfBarb FooFFXVbfBarf{FooFFXVbfBart
FooFFXVbfBarg;FooFFXVbfBare FooFFXVbfBarl96;}
FooFFXVbfBarl1233,FooFFXVbfBarl1231;
#if FFXCONF_NORSUPPORT || FFXCONF_ISWFSUPPORT
FooFFXVbfBarb FooFFXVbfBarf{FooFFXVbfBart FooFFXVbfBarg;
FooFFXVbfBara FooFFXVbfBarl138;FooFFXVbfBara
FooFFXVbfBarl195;FooFFXVbfBaro*FooFFXVbfBarl116;}
FooFFXVbfBarl466,FooFFXVbfBarl1045,FooFFXVbfBarl1137;
FooFFXVbfBarb FooFFXVbfBarf{FooFFXVbfBart FooFFXVbfBarg;
FooFFXVbfBara FooFFXVbfBarl138;FooFFXVbfBara
FooFFXVbfBarl195;FooFFXVbfBarc FooFFXVbfBaro*
FooFFXVbfBarl116;}FooFFXVbfBarl447,FooFFXVbfBarl1189,
FooFFXVbfBarl1175;FooFFXVbfBarb FooFFXVbfBarf{FooFFXVbfBart
FooFFXVbfBarg;FooFFXVbfBara FooFFXVbfBarl262;FooFFXVbfBara
FooFFXVbfBarl252;}FooFFXVbfBarl500,FooFFXVbfBarl1254;
FooFFXVbfBarb FooFFXVbfBarf{FooFFXVbfBart FooFFXVbfBarg;
FooFFXVbfBara FooFFXVbfBarl262;FooFFXVbfBara
FooFFXVbfBarl252;}FooFFXVbfBarl1204;
#endif
#if FFXCONF_NANDSUPPORT
FooFFXVbfBarb FooFFXVbfBarf{FooFFXVbfBart FooFFXVbfBarg;
FooFFXVbfBara FooFFXVbfBary;FooFFXVbfBara FooFFXVbfBarl31;
FooFFXVbfBaro*FooFFXVbfBarl41;FooFFXVbfBaro*FooFFXVbfBarl58
;}FooFFXVbfBarl462,FooFFXVbfBarl448;FooFFXVbfBarb
FooFFXVbfBarf{FooFFXVbfBart FooFFXVbfBarg;FooFFXVbfBara
FooFFXVbfBary;FooFFXVbfBara FooFFXVbfBarl31;FooFFXVbfBarc
FooFFXVbfBaro*FooFFXVbfBarl41;FooFFXVbfBarc FooFFXVbfBaro*
FooFFXVbfBarl58;}FooFFXVbfBarl478,FooFFXVbfBarl473;
FooFFXVbfBarb FooFFXVbfBarf{FooFFXVbfBart FooFFXVbfBarg;
FooFFXVbfBara FooFFXVbfBary;FooFFXVbfBara FooFFXVbfBarl31;
FooFFXVbfBaro*FooFFXVbfBarl41;FooFFXVbfBaro*FooFFXVbfBarl88
;FooFFXVbfBare FooFFXVbfBarl86;}FooFFXVbfBarl481;
FooFFXVbfBarb FooFFXVbfBarf{FooFFXVbfBart FooFFXVbfBarg;
FooFFXVbfBara FooFFXVbfBary;FooFFXVbfBara FooFFXVbfBarl31;
FooFFXVbfBarc FooFFXVbfBaro*FooFFXVbfBarl41;FooFFXVbfBarc
FooFFXVbfBaro*FooFFXVbfBarl88;FooFFXVbfBare FooFFXVbfBarl86
;}FooFFXVbfBarl441;FooFFXVbfBarb FooFFXVbfBarf{
FooFFXVbfBart FooFFXVbfBarg;FooFFXVbfBara FooFFXVbfBary;
FooFFXVbfBara FooFFXVbfBarl206;FooFFXVbfBaro*
FooFFXVbfBarl58;}FooFFXVbfBarl453;FooFFXVbfBarb
FooFFXVbfBarf{FooFFXVbfBart FooFFXVbfBarg;FooFFXVbfBara
FooFFXVbfBary;FooFFXVbfBara FooFFXVbfBarl206;FooFFXVbfBarc
FooFFXVbfBaro*FooFFXVbfBarl58;}FooFFXVbfBarl479;
FooFFXVbfBarb FooFFXVbfBarf{FooFFXVbfBart FooFFXVbfBarg;
FooFFXVbfBara FooFFXVbfBary;FooFFXVbfBara FooFFXVbfBarl177;
FooFFXVbfBaro*FooFFXVbfBarl88;FooFFXVbfBare FooFFXVbfBarl86
;}FooFFXVbfBarl491;FooFFXVbfBarb FooFFXVbfBarf{
FooFFXVbfBart FooFFXVbfBarg;FooFFXVbfBara FooFFXVbfBary;
FooFFXVbfBara FooFFXVbfBarl177;FooFFXVbfBarc FooFFXVbfBaro*
FooFFXVbfBarl88;FooFFXVbfBare FooFFXVbfBarl86;}
FooFFXVbfBarl345;
#if FFXCONF_BBMSUPPORT
FooFFXVbfBarb FooFFXVbfBarf{FooFFXVbfBart FooFFXVbfBarg;
FooFFXVbfBara FooFFXVbfBarl84;FooFFXVbfBara
FooFFXVbfBarl1195;FooFFXVbfBara FooFFXVbfBarl1100;
FooFFXVbfBara FooFFXVbfBarl119;}FooFFXVbfBarl458,
FooFFXVbfBarl1049,FooFFXVbfBarl459,FooFFXVbfBarl1214;
FooFFXVbfBarb FooFFXVbfBarf{FooFFXVbfBart FooFFXVbfBarg;
FooFFXVbfBara FooFFXVbfBarl503;FooFFXVbfBara FooFFXVbfBarl31
;FooFFXVbfBara FooFFXVbfBarl452;}FooFFXVbfBarl510,
FooFFXVbfBarl1248;
#endif
FooFFXVbfBarb FooFFXVbfBarf{FooFFXVbfBart FooFFXVbfBarg;
FooFFXVbfBara FooFFXVbfBarl117;}FooFFXVbfBarl471,
FooFFXVbfBarl1089,FooFFXVbfBarl1108;FooFFXVbfBarb
FooFFXVbfBarf{FooFFXVbfBart FooFFXVbfBarg;FooFFXVbfBara
FooFFXVbfBarl84;}FooFFXVbfBarl438,FooFFXVbfBarl1043,
FooFFXVbfBarl1090,FooFFXVbfBarl468,FooFFXVbfBarl1237;
FooFFXVbfBarb FooFFXVbfBarf{FooFFXVbfBart FooFFXVbfBarg;
FooFFXVbfBara FooFFXVbfBarl84;FooFFXVbfBara FooFFXVbfBarl119
;}FooFFXVbfBarl504,FooFFXVbfBarl1132,FooFFXVbfBarl1201;
FooFFXVbfBarb FooFFXVbfBarf{FooFFXVbfBart FooFFXVbfBarg;
FooFFXVbfBara FooFFXVbfBary;FooFFXVbfBara FooFFXVbfBarl31;
FooFFXVbfBaro*FooFFXVbfBarl41;}FooFFXVbfBarl428,
FooFFXVbfBarl1148,FooFFXVbfBarl1253;FooFFXVbfBarb
FooFFXVbfBarf{FooFFXVbfBart FooFFXVbfBarg;FooFFXVbfBara
FooFFXVbfBary;FooFFXVbfBara FooFFXVbfBarl31;FooFFXVbfBarc
FooFFXVbfBaro*FooFFXVbfBarl41;}FooFFXVbfBarl513,
FooFFXVbfBarl1226,FooFFXVbfBarl1053;FooFFXVbfBarb
FooFFXVbfBarf{FooFFXVbfBart FooFFXVbfBarg;}FooFFXVbfBarl502
,FooFFXVbfBarl1150,FooFFXVbfBarl1239;
#endif
FooFFXVbfBarf FooFFXVbfBarl282{FooFFXVbfBara FooFFXVbfBarv;
FFXSTATUS FooFFXVbfBarl26;FooFFXVbfBara FooFFXVbfBarl65;
FooFFXVbfBarl608{FooFFXVbfBara FooFFXVbfBarl1103;
FooFFXVbfBara FooFFXVbfBarl119;FooFFXVbfBara
FooFFXVbfBarl407;FooFFXVbfBara FooFFXVbfBarl1078;}
FooFFXVbfBarl414;};
#define FooFFXVbfBarl1141 0xF0000000
#define FooFFXVbfBarl139 0x00000000
#define FooFFXVbfBarl190 0x10000000
#define FooFFXVbfBarl189 0x20000000
#define FooFFXVbfBarl187 0x30000000
#define FooFFXVbfBarl301 0x40000000
#define FooFFXVbfBarl1234 (~( FooFFXVbfBarl306 |   FooFFXVbfBarl304))
#define FooFFXVbfBarl306 0x0000000F
#define FooFFXVbfBarl1111 0x00000000
#define FooFFXVbfBarl1066 0x00000001
#define FooFFXVbfBarl1073 0x00000003
#define FooFFXVbfBarl1194 0x00000005
#define FooFFXVbfBarl304 0x00000070
#define FooFFXVbfBarl1122 0x00000010
#define FooFFXVbfBarl1091 0x00000020
#define FooFFXVbfBarl1129 0x00000040
#define FooFFXVbfBarl1225 0x00000080
#define FooFFXVbfBarl1088 0x00000081
#define FooFFXVbfBarl1197 0x00000082
#define FooFFXVbfBarl469 0x00000083
#define FooFFXVbfBarl1205 0x00000084
#define FooFFXVbfBarl1262 0x00000085
#define FooFFXVbfBarl1144 0x00000086
#define FooFFXVbfBarl1095 0xF0000008
#define FooFFXVbfBarl316 0x0FF00000
#define FooFFXVbfBarl1069 0x000FF000
#define FooFFXVbfBarl1044 0x00000C00
#define FooFFXVbfBarl1250 0x000003F0
#define FooFFXVbfBarl1105 0x00000007
#define FooFFXVbfBarl1179 0x00000000
#define FooFFXVbfBarl1017 0x00000001
#define FooFFXVbfBarl1086 0x00000002
#define FooFFXVbfBarl1077 0x00000004
#define FooFFXVbfBarl1209( FooFFXVbfBarl140) (((   FooFFXVbfBara)( FooFFXVbfBarl140)) << 20)
#define FooFFXVbfBarl1216( FooFFXVbfBarl140) (((   FooFFXVbfBarl140) & FooFFXVbfBarl316) >> 20)
#define FooFFXVbfBarl358( FooFFXVbfBarl185, FooFFXVbfBarl51  ) ( FooFFXVbfBarl185. FooFFXVbfBarv == ( FooFFXVbfBarl51)   && FooFFXVbfBarl185. FooFFXVbfBarl26 == FooFFXVbfBarl44)
#define FooFFXVbfBarl1107 {0, FooFFXVbfBarl44,  \
FooFFXVbfBarl139, {0}}
#define FooFFXVbfBarl314( FooFFXVbfBarl34) {0, (   FooFFXVbfBarl34), FooFFXVbfBarl301, {0}}
#define FooFFXVbfBarl107( FooFFXVbfBarl34) {0, (   FooFFXVbfBarl34), FooFFXVbfBarl190, {0}}
#define FooFFXVbfBarl81( FooFFXVbfBarl34) {0, (   FooFFXVbfBarl34), FooFFXVbfBarl189, {0}}
#define FooFFXVbfBarl184( FooFFXVbfBarl34) {0, (   FooFFXVbfBarl34), FooFFXVbfBarl187, {0}}
#define FooFFXVbfBarl286( FooFFXVbfBarl34) {0, (   FooFFXVbfBarl34), FooFFXVbfBarl139, {0}}
#define FooFFXVbfBarl1215( FooFFXVbfBarl51, FooFFXVbfBarl34  ) {( FooFFXVbfBarl51), ( FooFFXVbfBarl34), FooFFXVbfBarl190 \
, {0}}
#define FooFFXVbfBarl1060( FooFFXVbfBarl51, FooFFXVbfBarl34  ) {( FooFFXVbfBarl51), ( FooFFXVbfBarl34), FooFFXVbfBarl189 \
, {0}}
#define FooFFXVbfBarl1174( FooFFXVbfBarl51, FooFFXVbfBarl34  ) {( FooFFXVbfBarl51), ( FooFFXVbfBarl34), FooFFXVbfBarl187 \
, {0}}
#define FooFFXVbfBarl1242( FooFFXVbfBarl51, FooFFXVbfBarl34  ) {( FooFFXVbfBarl51), ( FooFFXVbfBarl34), FooFFXVbfBarl139 \
, {0}}
#define FooFFXVbfBarl1206 FooFFXVbfBarl314(   FFXSTAT_BADPARAMETER)
#define FooFFXVbfBarl1219 FooFFXVbfBarl107(   FFXSTAT_BADPARAMETER)
#define FooFFXVbfBarl377 FooFFXVbfBarl81(   FFXSTAT_BADPARAMETER)
#define FooFFXVbfBarl1187 FooFFXVbfBarl184(   FFXSTAT_BADPARAMETER)
#define FooFFXVbfBarl1199 FooFFXVbfBarl286(   FFXSTAT_BADPARAMETER)
#define FooFFXVbfBarl1178 FooFFXVbfBarl107( FooFFXVbfBarl44  )
#define FooFFXVbfBarl1109 FooFFXVbfBarl81( FooFFXVbfBarl44)
#define FooFFXVbfBarl1047 FooFFXVbfBarl81( FooFFXVbfBarl284  )
#define FooFFXVbfBarl1228 FooFFXVbfBarl81( FooFFXVbfBarl156  )
#define FooFFXVbfBarl1251 FooFFXVbfBarl107(   FooFFXVbfBarl241)
#define FooFFXVbfBarl1104 FooFFXVbfBarl184(   FooFFXVbfBarl133)
#define FooFFXVbfBarl1217 FooFFXVbfBarl81( FooFFXVbfBarl133  )
#define FooFFXVbfBarl1070 FooFFXVbfBarl107(   FooFFXVbfBarl133)
#endif
#include <dlservice.h>
#ifndef FooFFXVbfBarl1467
#define FooFFXVbfBarl1467
#define FooFFXVbfBarl1889 10
#define FooFFXVbfBarl1822 0xFF
FooFFXVbfBarb FooFFXVbfBarl61 FooFFXVbfBarl1842{
FooFFXVbfBarl1851=0,FooFFXVbfBarl1862,FooFFXVbfBarl1932,
FooFFXVbfBarl1942,FooFFXVbfBarl1779,FooFFXVbfBarl1939,
FooFFXVbfBarl1946,FooFFXVbfBarl1934,FooFFXVbfBarl1800}
FooFFXVbfBarl1484;FooFFXVbfBarb FooFFXVbfBarl61
FooFFXVbfBarl1833{FooFFXVbfBarl1829=0,FooFFXVbfBarl1797,
FooFFXVbfBarl1763,FooFFXVbfBarl1928}FooFFXVbfBarl1330;
FooFFXVbfBarb FooFFXVbfBarl61 FooFFXVbfBarl1809{
FooFFXVbfBarl1918=0,FooFFXVbfBarl1876,FooFFXVbfBarl1893,
FooFFXVbfBarl1867,FooFFXVbfBarl1850,FooFFXVbfBarl1771,
FooFFXVbfBarl1947}FooFFXVbfBarl1246;FooFFXVbfBarf
FooFFXVbfBarl1478;FooFFXVbfBarb FooFFXVbfBarf
FooFFXVbfBarl1478 FooFFXVbfBarl1145;FooFFXVbfBara
FooFFXVbfBarl1789(FooFFXVbfBarl124 FooFFXVbfBarl196);
FooFFXVbfBara FooFFXVbfBarl1824(FooFFXVbfBarl35* FooFFXVbfBarl265);FooFFXVbfBarh FooFFXVbfBarl1892( FooFFXVbfBara FooFFXVbfBarl1798,FooFFXVbfBarl35* FooFFXVbfBarl265);FooFFXVbfBarl1246 FooFFXVbfBarl1778( FooFFXVbfBarl35*FooFFXVbfBarl265);FooFFXVbfBarh
FooFFXVbfBarl1871(FooFFXVbfBarl1246 FooFFXVbfBarl1907, FooFFXVbfBarl35*FooFFXVbfBarl265);FooFFXVbfBarh
FooFFXVbfBarl1881(FooFFXVbfBarl1330 FooFFXVbfBarl1845, FooFFXVbfBarl35*FooFFXVbfBarl265);FooFFXVbfBarl1330
FooFFXVbfBarl1808(FooFFXVbfBarl35*FooFFXVbfBarl265);
FooFFXVbfBarh FooFFXVbfBarl1904(FooFFXVbfBarl124 FooFFXVbfBarl196,FooFFXVbfBarl1145*FooFFXVbfBarl1503, FooFFXVbfBarl35*FooFFXVbfBarl265,FooFFXVbfBara FooFFXVbfBarl95,FooFFXVbfBara FooFFXVbfBarl1870);
FooFFXVbfBarh FooFFXVbfBarl1287(FooFFXVbfBarl35* FooFFXVbfBarl265);FooFFXVbfBarl1145*FooFFXVbfBarl1806( FooFFXVbfBarl124 FooFFXVbfBarl196);FooFFXVbfBarh
FooFFXVbfBarl1897(FooFFXVbfBarl1145*FooFFXVbfBarl1503);
FooFFXVbfBarl1246 FooFFXVbfBarl1844(FooFFXVbfBarl1484 FooFFXVbfBarl1768,FooFFXVbfBarl35*FooFFXVbfBarl265, FooFFXVbfBarl124 FooFFXVbfBarl196);
#endif
#define FooFFXVbfBarl1959 (0xFF)
#define FooFFXVbfBarl2267 (3)
#define FooFFXVbfBarl1750 64
FooFFXVbfBarf FooFFXVbfBarl1012{PDCLMUTEX FooFFXVbfBarl1494
;PDCLSEMAPHORE FooFFXVbfBarl1549;FooFFXVbfBare
FooFFXVbfBarl1026;FooFFXVbfBarl608{FooFFXVbfBarl91
FooFFXVbfBarl273[FooFFXVbfBarl1750];DCL_ALIGNTYPE
FooFFXVbfBarl1869;}FooFFXVbfBarl2415;
#if FFXCONF_STATS_VBFREGION
FooFFXVbfBara FooFFXVbfBarl403;FooFFXVbfBara
FooFFXVbfBarl390;FooFFXVbfBara FooFFXVbfBarl207;
FooFFXVbfBara FooFFXVbfBarl406;FooFFXVbfBarl230*
FooFFXVbfBarl227;
#endif
FooFFXVbfBaru*FooFFXVbfBarl584[1];};FooFFXVbfBarl99
FooFFXVbfBaru*FooFFXVbfBarl2115(FooFFXVbfBarl29* FooFFXVbfBarl183,FooFFXVbfBarl50 FooFFXVbfBarl59);
FooFFXVbfBarl99 FooFFXVbfBaru*FooFFXVbfBarl2211( FooFFXVbfBarl29*FooFFXVbfBarl183,FooFFXVbfBarl50 FooFFXVbfBarl59);FooFFXVbfBarl99 FooFFXVbfBarh
FooFFXVbfBarl1878(FooFFXVbfBarl29*FooFFXVbfBarl183, FooFFXVbfBaru*FooFFXVbfBarr);FooFFXVbfBarl99 FooFFXVbfBari
FooFFXVbfBarl2061(FooFFXVbfBarl29*FooFFXVbfBarl183, FooFFXVbfBaru*FooFFXVbfBarr);FooFFXVbfBarl99 FFXSTATUS
FooFFXVbfBarl2276(FooFFXVbfBarl29*FooFFXVbfBarl183, FooFFXVbfBaru*FooFFXVbfBarr,FooFFXVbfBarl38 FooFFXVbfBarl1525);FooFFXVbfBarl99 FFXSTATUS
FooFFXVbfBarl2003(FooFFXVbfBarl29*FooFFXVbfBarl183, FooFFXVbfBaru*FooFFXVbfBarr,FooFFXVbfBarl38 FooFFXVbfBarl1525);FooFFXVbfBarl99 FFXSTATUS
FooFFXVbfBarl2339(FooFFXVbfBarl29*FooFFXVbfBarl183, FooFFXVbfBarl32 FooFFXVbfBarl43);FooFFXVbfBarl99
FooFFXVbfBarh FooFFXVbfBarl1854(FooFFXVbfBaru*FooFFXVbfBarw ,FooFFXVbfBarl38 FooFFXVbfBarl33,FooFFXVbfBare FooFFXVbfBarl321,FooFFXVbfBarj FooFFXVbfBarl1841);
FooFFXVbfBarl99 FooFFXVbfBarh FooFFXVbfBarl2314( FooFFXVbfBarl198*FooFFXVbfBarl1515,FooFFXVbfBari FooFFXVbfBarl201);FFXSTATUS FooFFXVbfBarl872( FooFFXVbfBarl28*FooFFXVbfBark){FooFFXVbfBarl225*
FooFFXVbfBarl92;FooFFXVbfBarl37 FooFFXVbfBarl1343[
DCL_MUTEXNAMELEN];FooFFXVbfBarl55 FooFFXVbfBarl60;
FooFFXVbfBarl55 FooFFXVbfBarl1496;FooFFXVbfBare
FooFFXVbfBarl1360;FooFFXVbfBare FooFFXVbfBarl1532;
FFXTRACEPRINTF((FooFFXVbfBarl66(FooFFXVbfBarl150,1,0),"\x46\x66\x78\x56\x62\x66\x52\x65\x67\x69\x6f\x6e\x43\x61\x63\x68\x65\x43\x72\x65\x61\x74\x65\x28\x29\x20\x66\x6f\x72\x20\x44\x49\x53\x4b\x25\x75\x2c\x20\x52\x65\x71\x75\x65\x73\x74\x65\x64\x20\x25\x64\x20\x65\x6e\x74\x72\x69\x65\x73\n",FooFFXVbfBark->FooFFXVbfBarl64, VBFCONF_REGIONCACHESIZE));FooFFXVbfBarl27(740,FooFFXVbfBark );FooFFXVbfBarl27(741,!FooFFXVbfBark->FooFFXVbfBarl293);
#if defined( VBFCONF_STATICREGION) || DCL_OSFEATURE_THREADS
FooFFXVbfBarl1532=DCLMIN(2,FooFFXVbfBark->FooFFXVbfBarl211);
#else
FooFFXVbfBarl1532=DCLMIN(1,FooFFXVbfBark->FooFFXVbfBarl211);
#endif
FooFFXVbfBarl1496=VBFCONF_REGIONCACHESIZE;FooFFXVbfBarn( FooFFXVbfBarl1496>0){FooFFXVbfBarl1360=DCLMIN(FooFFXVbfBark ->FooFFXVbfBarl211,(FooFFXVbfBare)FooFFXVbfBarl1496);}
FooFFXVbfBarl62 FooFFXVbfBarn((FooFFXVbfBarl1496<=-1)&&( FooFFXVbfBarl1496>=-100)){FooFFXVbfBarl1360=(FooFFXVbfBark ->FooFFXVbfBarl211* (-FooFFXVbfBarl1496))/100;}
FooFFXVbfBarl62{FFXPRINTF(1,("\x49\x6e\x76\x61\x6c\x69\x64\x20\x72\x65\x67\x69\x6f\x6e\x20\x63\x61\x63\x68\x65\x20\x73\x69\x7a\x65\x20\x6f\x66\x20\x25\x64\x2c\x20\x75\x73\x69\x6e\x67\x20\x61\x20\x64\x65\x66\x61\x75\x6c\x74\x20\x6f\x66\x20\x25\x75\n",FooFFXVbfBarl1496, FooFFXVbfBarl2267));FooFFXVbfBarl1360=FooFFXVbfBarl2267;}
FooFFXVbfBarn(FooFFXVbfBarl1360<FooFFXVbfBarl1532)FooFFXVbfBarl1360
=FooFFXVbfBarl1532;FooFFXVbfBarn(FooFFXVbfBarl1360> FooFFXVbfBarl1959)FooFFXVbfBarl1360=FooFFXVbfBarl1959;
FooFFXVbfBarl92=FooFFXVbfBarl1292(FooFFXVbfBarl30( * FooFFXVbfBarl92)+((FooFFXVbfBarl1360-1) *FooFFXVbfBarl30( FooFFXVbfBarl92->FooFFXVbfBarl584[0])));FooFFXVbfBarn(! FooFFXVbfBarl92)FooFFXVbfBarl46 FooFFXVbfBarl373;
#if FFXCONF_NANDSUPPORT
FFXPRINTF(2,("\x20\x20\x73\x69\x7a\x65\x6f\x66\x28\x44\x69\x73\x63\x61\x72\x64\x53\x65\x74\x29\x20\x20\x20\x20\x25\x31\x30\x6c\x55\n",FooFFXVbfBarl30(FooFFXVbfBarl92-> FooFFXVbfBarl584[0]->FooFFXVbfBarl242)));
#endif
FFXPRINTF(2,("\x20\x20\x73\x69\x7a\x65\x6f\x66\x28\x55\x6e\x69\x74\x49\x6e\x66\x6f\x5b\x5d\x29\x20\x20\x20\x20\x25\x31\x30\x6c\x55\n",FooFFXVbfBarl30(FooFFXVbfBarl92-> FooFFXVbfBarl584[0]->FooFFXVbfBarl53)));FFXPRINTF(2,("\x20\x20\x73\x69\x7a\x65\x6f\x66\x28\x72\x65\x67\x69\x6f\x6e\x41\x6c\x6c\x6f\x63\x5b\x5d\x29\x20\x25\x31\x30\x6c\x55\n",FooFFXVbfBarl30(FooFFXVbfBarl92->FooFFXVbfBarl584[0]-> FooFFXVbfBarl208)));FooFFXVbfBarl27(808,FooFFXVbfBark-> FooFFXVbfBarl64<=255);FooFFXVbfBarl27(809,DCL_MUTEXNAMELEN >=8);FooFFXVbfBarl1495(FooFFXVbfBarl1343,FooFFXVbfBarl30( FooFFXVbfBarl1343),"\x46\x4d\x52\x43\x25\x30\x32\x78", FooFFXVbfBark->FooFFXVbfBarl64);FooFFXVbfBarl92->
FooFFXVbfBarl1494=FooFFXVbfBarl2135(FooFFXVbfBarl1343);
FooFFXVbfBarn(FooFFXVbfBarl92->FooFFXVbfBarl1494){
FooFFXVbfBarl120(FooFFXVbfBarl60=0;FooFFXVbfBarl60<( FooFFXVbfBarl55)FooFFXVbfBarl1360;FooFFXVbfBarl60++){
FooFFXVbfBaru*FooFFXVbfBarl1668;FooFFXVbfBarl92->
FooFFXVbfBarl584[FooFFXVbfBarl60]=FooFFXVbfBarl1292( FooFFXVbfBarl30*FooFFXVbfBarl92->FooFFXVbfBarl584[0]);
FooFFXVbfBarn(!FooFFXVbfBarl92->FooFFXVbfBarl584[ FooFFXVbfBarl60])FooFFXVbfBarl100;FooFFXVbfBarl1668=
FooFFXVbfBarl92->FooFFXVbfBarl584[FooFFXVbfBarl60];
FooFFXVbfBarl1668->FooFFXVbfBarl702=FooFFXVbfBarl996();
FooFFXVbfBarl1495(FooFFXVbfBarl1343,FooFFXVbfBarl30( FooFFXVbfBarl1343),"\x46\x4d\x41\x25\x30\x32\x78\x25\x30\x32\x78",FooFFXVbfBark->FooFFXVbfBarl64,FooFFXVbfBarl60);
FooFFXVbfBarl1668->FooFFXVbfBarl401=FooFFXVbfBarl2135( FooFFXVbfBarl1343);FooFFXVbfBarn(!FooFFXVbfBarl1668-> FooFFXVbfBarl401){FooFFXVbfBarl323(FooFFXVbfBarl1668);
FooFFXVbfBarl100;}FooFFXVbfBarl92->FooFFXVbfBarl1026++;}
FooFFXVbfBarn(FooFFXVbfBarl92->FooFFXVbfBarl1026>= FooFFXVbfBarl1532){FooFFXVbfBarl37 FooFFXVbfBarl1940[
DCL_SEMAPHORENAMELEN];FooFFXVbfBarl27(862,FooFFXVbfBark-> FooFFXVbfBarl64<=255);FooFFXVbfBarl27(863, DCL_SEMAPHORENAMELEN>=8);FooFFXVbfBarl1495( FooFFXVbfBarl1940,FooFFXVbfBarl30(FooFFXVbfBarl1940),"\x46\x53\x52\x43\x25\x30\x32\x78",FooFFXVbfBark-> FooFFXVbfBarl64);FooFFXVbfBarl92->FooFFXVbfBarl1549=
FooFFXVbfBarl1926(FooFFXVbfBarl1940,FooFFXVbfBarl92-> FooFFXVbfBarl1026);}FooFFXVbfBarn((FooFFXVbfBarl92-> FooFFXVbfBarl1026<FooFFXVbfBarl1532)||!FooFFXVbfBarl92-> FooFFXVbfBarl1549){FooFFXVbfBarl120(FooFFXVbfBarl60= FooFFXVbfBarl92->FooFFXVbfBarl1026;FooFFXVbfBarl60>=0; FooFFXVbfBarl60--){FooFFXVbfBarl1701(FooFFXVbfBarl92-> FooFFXVbfBarl584[FooFFXVbfBarl60]->FooFFXVbfBarl401);
FooFFXVbfBarl323(FooFFXVbfBarl92->FooFFXVbfBarl584[ FooFFXVbfBarl60]);}}FooFFXVbfBarl62{
#if FFXCONF_STATS_VBFREGION
FooFFXVbfBarl92->FooFFXVbfBarl227=FooFFXVbfBarl1292( FooFFXVbfBarl30( *FooFFXVbfBarl92->FooFFXVbfBarl227) * FooFFXVbfBark->FooFFXVbfBarl211);FooFFXVbfBarn(! FooFFXVbfBarl92->FooFFXVbfBarl227){FooFFXVbfBarl89("\x46\x46\x58\x3a\x20\x55\x6e\x61\x62\x6c\x65\x20\x74\x6f\x20\x61\x6c\x6c\x6f\x63\x61\x74\x65\x20\x72\x65\x67\x69\x6f\x6e\x20\x74\x72\x61\x63\x6b\x69\x6e\x67\x20\x6d\x65\x6d\x6f\x72\x79\n");}
#endif
FFXPRINTF(2,("\x25\x75\x20\x6f\x66\x20\x25\x55\x20\x72\x65\x67\x69\x6f\x6e\x73\x20\x61\x72\x65\x20\x63\x61\x63\x68\x65\x64\x20\x66\x6f\x72\x20\x44\x49\x53\x4b\x25\x55\n", FooFFXVbfBarl92->FooFFXVbfBarl1026,FooFFXVbfBark-> FooFFXVbfBarl211,FooFFXVbfBark->FooFFXVbfBarl64));
FooFFXVbfBarl27(899,FooFFXVbfBarl92->FooFFXVbfBarl1026<= FooFFXVbfBark->FooFFXVbfBarl211);FooFFXVbfBarn( FooFFXVbfBarl92->FooFFXVbfBarl1026==FooFFXVbfBark-> FooFFXVbfBarl211)FooFFXVbfBark->FooFFXVbfBarl559=TRUE;
FooFFXVbfBark->FooFFXVbfBarl981=FooFFXVbfBarl92->
FooFFXVbfBarl1026;FooFFXVbfBark->FooFFXVbfBarl293=
FooFFXVbfBarl92;FooFFXVbfBarl46 FooFFXVbfBarl44;}}
FooFFXVbfBarl323(FooFFXVbfBarl92);FooFFXVbfBarl46
FooFFXVbfBarl764;}FooFFXVbfBarh FooFFXVbfBarl679( FooFFXVbfBarl29*FooFFXVbfBark,FooFFXVbfBara FooFFXVbfBarl65 ){FooFFXVbfBarl225*FooFFXVbfBarl92;FooFFXVbfBaru*
FooFFXVbfBarr;FooFFXVbfBarl55 FooFFXVbfBarl60;
FFXTRACEPRINTF((FooFFXVbfBarl66(FooFFXVbfBarl150,2,0),"\x46\x66\x78\x56\x62\x66\x52\x65\x67\x69\x6f\x6e\x43\x61\x63\x68\x65\x44\x65\x73\x74\x72\x6f\x79\x28\x29\x20\x70\x56\x42\x46\x3d\x25\x50\x20\x46\x6c\x61\x67\x73\x3d\x25\x6c\x55\n",FooFFXVbfBark,FooFFXVbfBarl65));FooFFXVbfBarl27( 947,FooFFXVbfBark);FooFFXVbfBarl27(948,FooFFXVbfBark-> FooFFXVbfBarl293);FooFFXVbfBarl27(949,!(FooFFXVbfBarl65&( 0xFFFFFFFD)));FooFFXVbfBarl92=FooFFXVbfBark->
FooFFXVbfBarl293;
#if FFXCONF_STATS_VBFREGION
FooFFXVbfBarn(FooFFXVbfBarl92->FooFFXVbfBarl227)FooFFXVbfBarl323
(FooFFXVbfBarl92->FooFFXVbfBarl227);
#endif
FooFFXVbfBarl120(FooFFXVbfBarl60=FooFFXVbfBarl92-> FooFFXVbfBarl1026-1;FooFFXVbfBarl60>=0;FooFFXVbfBarl60--){
FooFFXVbfBarr=FooFFXVbfBarl92->FooFFXVbfBarl584[
FooFFXVbfBarl60];FooFFXVbfBarl153(FooFFXVbfBarl1378(& FooFFXVbfBarr->FooFFXVbfBarl171)){FooFFXVbfBarl337( FooFFXVbfBarr);}
#if FFXCONF_NANDSUPPORT && VBFCONF_DEFERREDDISCARD
{FooFFXVbfBara FooFFXVbfBarl1724=FooFFXVbfBarl65&( 0x00000002);FooFFXVbfBarn(FooFFXVbfBark->FooFFXVbfBarl161&& FooFFXVbfBarr->FooFFXVbfBarl203&&(FooFFXVbfBarl1724!=( 0x00000002))){FFXSTATUS FooFFXVbfBarl26;FooFFXVbfBarl27(993 ,FooFFXVbfBark->FooFFXVbfBarl437);FooFFXVbfBarl27(994, FooFFXVbfBarl1724==(0x00000000)||FooFFXVbfBarl1724==( 0x00000001));FooFFXVbfBarl361(FooFFXVbfBark);{
FooFFXVbfBarl26=FooFFXVbfBarl589(FooFFXVbfBark, FooFFXVbfBarr,(FooFFXVbfBarl1724==(0x00000001)));}
FooFFXVbfBarl334(FooFFXVbfBark);FooFFXVbfBarl27(1002, FooFFXVbfBarl26==FooFFXVbfBarl44);FooFFXVbfBarl27(1003,! FooFFXVbfBarr->FooFFXVbfBarl203);(FooFFXVbfBarh)FooFFXVbfBarl26
;}}
#endif
FooFFXVbfBarl1701(FooFFXVbfBarr->FooFFXVbfBarl401);
FooFFXVbfBarl323(FooFFXVbfBarr);}FooFFXVbfBarl1827( FooFFXVbfBarl92->FooFFXVbfBarl1549);FooFFXVbfBarl1701( FooFFXVbfBarl92->FooFFXVbfBarl1494);FooFFXVbfBarl323( FooFFXVbfBarl92);FooFFXVbfBark->FooFFXVbfBarl293=
FooFFXVbfBarl79;FooFFXVbfBarl46;}FooFFXVbfBarh
FooFFXVbfBarl423(FooFFXVbfBarc FooFFXVbfBaru*FooFFXVbfBarr){
FFXTRACEPRINTF((FooFFXVbfBarl66(FooFFXVbfBarl150,2,0),"\x46\x66\x78\x56\x62\x66\x52\x65\x67\x69\x6f\x6e\x4c\x6f\x63\x6b\x28\x29\x20\x6c\x6f\x63\x6b\x69\x6e\x67\x20\x52\x67\x6e\x3d\x25\x6c\x55\n",FooFFXVbfBarr->FooFFXVbfBarl59));
FooFFXVbfBarn(!FooFFXVbfBarl1696(FooFFXVbfBarr-> FooFFXVbfBarl401)){FooFFXVbfBarl112(1057);}FFXTRACEPRINTF(( FooFFXVbfBarl66(FooFFXVbfBarl150,2,0),"\x46\x66\x78\x56\x62\x66\x52\x65\x67\x69\x6f\x6e\x4c\x6f\x63\x6b\x28\x29\x20\x6c\x6f\x63\x6b\x65\x64\x20\x52\x67\x6e\x3d\x25\x6c\x55\n",FooFFXVbfBarr->FooFFXVbfBarl59));FooFFXVbfBarl46;}
FooFFXVbfBarh FooFFXVbfBarl416(FooFFXVbfBarc FooFFXVbfBaru* FooFFXVbfBarr){FFXTRACEPRINTF((FooFFXVbfBarl66( FooFFXVbfBarl150,2,0),"\x46\x66\x78\x56\x62\x66\x52\x65\x67\x69\x6f\x6e\x55\x6e\x6c\x6f\x63\x6b\x28\x29\x20\x75\x6e\x6c\x6f\x63\x6b\x69\x6e\x67\x20\x52\x67\x6e\x3d\x25\x6c\x55\n",FooFFXVbfBarr->FooFFXVbfBarl59));FooFFXVbfBarn(! FooFFXVbfBarl1738(FooFFXVbfBarr->FooFFXVbfBarl401)){
FooFFXVbfBarl112(1087);}FFXTRACEPRINTF((FooFFXVbfBarl66( FooFFXVbfBarl150,2,0),"\x46\x66\x78\x56\x62\x66\x52\x65\x67\x69\x6f\x6e\x55\x6e\x6c\x6f\x63\x6b\x28\x29\x20\x75\x6e\x6c\x6f\x63\x6b\x65\x64\x20\x52\x67\x6e\x3d\x25\x6c\x55\n",FooFFXVbfBarr->FooFFXVbfBarl59));FooFFXVbfBarl46;}
FooFFXVbfBaru*FooFFXVbfBarl349(FooFFXVbfBarl29* FooFFXVbfBars,FooFFXVbfBarl50 FooFFXVbfBarl149, FooFFXVbfBarl281 FooFFXVbfBarl383){FooFFXVbfBarl225*
FooFFXVbfBarl92=FooFFXVbfBars->FooFFXVbfBarl293;
FooFFXVbfBaru*FooFFXVbfBarr=FooFFXVbfBarl79;
#if FFXCONF_NANDSUPPORT && VBFCONF_DEFERREDDISCARD
FooFFXVbfBarj FooFFXVbfBarl2283=FALSE;
#endif
FFXTRACEPRINTF((FooFFXVbfBarl66(FooFFXVbfBarl150,1, FooFFXVbfBarl283),"\x46\x66\x78\x56\x62\x66\x52\x65\x67\x69\x6f\x6e\x4d\x6f\x75\x6e\x74\x28\x29\x20\x52\x67\x6e\x3d\x25\x6c\x55\x20\x41\x63\x63\x65\x73\x73\x54\x79\x70\x65\x3d\x25\x75\n",FooFFXVbfBarl149,FooFFXVbfBarl383));
FooFFXVbfBarl370("\x46\x66\x78\x56\x62\x66\x52\x65\x67\x69\x6f\x6e\x4d\x6f\x75\x6e\x74",0,0);FooFFXVbfBarl27(1136, FooFFXVbfBars);FooFFXVbfBarl27(1137,FooFFXVbfBarl92);
FooFFXVbfBarl27(1138,FooFFXVbfBarl149<FooFFXVbfBars-> FooFFXVbfBarl211);FooFFXVbfBarl27(1139,FooFFXVbfBarl383> FooFFXVbfBarl879&&FooFFXVbfBarl383<FooFFXVbfBarl833);
FooFFXVbfBarn(FooFFXVbfBarl383==FooFFXVbfBarl404){
#if FFXCONF_NANDSUPPORT && VBFCONF_DEFERREDDISCARD
FooFFXVbfBarn(FooFFXVbfBars->FooFFXVbfBarl437&&! FooFFXVbfBars->FooFFXVbfBarl559){FooFFXVbfBarl361( FooFFXVbfBars);FooFFXVbfBarl2283=TRUE;}
#endif
}FooFFXVbfBarl62 FooFFXVbfBarn(FooFFXVbfBarl383== FooFFXVbfBarl707||FooFFXVbfBarl383==FooFFXVbfBarl535){}
FooFFXVbfBarl62 FooFFXVbfBarn(FooFFXVbfBarl383== FooFFXVbfBarl743){}FooFFXVbfBarl62{FooFFXVbfBarl112(1184);}
FooFFXVbfBarl1696(FooFFXVbfBarl92->FooFFXVbfBarl1494);
#if FFXCONF_STATS_VBFREGION
FooFFXVbfBarl92->FooFFXVbfBarl403++;
#endif
FooFFXVbfBarr=FooFFXVbfBarl2115(FooFFXVbfBars, FooFFXVbfBarl149);FooFFXVbfBarn(!FooFFXVbfBarr){
FooFFXVbfBarl1706(FooFFXVbfBarl92->FooFFXVbfBarl1549);
FooFFXVbfBarr=FooFFXVbfBarl2211(FooFFXVbfBars, FooFFXVbfBarl149);FooFFXVbfBarl372(1219,FooFFXVbfBarr);
FooFFXVbfBarl27(1221,!FooFFXVbfBarr->FooFFXVbfBarl130);
FooFFXVbfBarl1878(FooFFXVbfBars,FooFFXVbfBarr);
FooFFXVbfBarn(FooFFXVbfBarr->FooFFXVbfBarl130){
FooFFXVbfBarn(FooFFXVbfBars->FooFFXVbfBarl272[FooFFXVbfBarr ->FooFFXVbfBarl59].FooFFXVbfBarl344==FooFFXVbfBarl499)FooFFXVbfBarl455
(FooFFXVbfBars,FooFFXVbfBarr);}FooFFXVbfBarl62{
FooFFXVbfBarl337(FooFFXVbfBarr);FooFFXVbfBarr=
FooFFXVbfBarl79;}}FooFFXVbfBarl62{
#if FFXCONF_STATS_VBFREGION
FooFFXVbfBarn(FooFFXVbfBarl92->FooFFXVbfBarl227)FooFFXVbfBarl92
->FooFFXVbfBarl227[FooFFXVbfBarl149].FooFFXVbfBarl207++;
#endif
}
#if FFXCONF_STATS_VBFREGION
FooFFXVbfBarn(FooFFXVbfBarr){FooFFXVbfBarn(FooFFXVbfBarl92 ->FooFFXVbfBarl227)FooFFXVbfBarl92->FooFFXVbfBarl227[
FooFFXVbfBarl149].FooFFXVbfBarl583++;FooFFXVbfBarn( FooFFXVbfBarl383==FooFFXVbfBarl535)FooFFXVbfBarl92->
FooFFXVbfBarl406++;}
#endif
FooFFXVbfBarl1738(FooFFXVbfBarl92->FooFFXVbfBarl1494);
#if FFXCONF_NANDSUPPORT && VBFCONF_DEFERREDDISCARD
FooFFXVbfBarn(FooFFXVbfBarl2283)FooFFXVbfBarl334( FooFFXVbfBars);
#endif
FooFFXVbfBarl381(0UL);FFXTRACEPRINTF((FooFFXVbfBarl66( FooFFXVbfBarl150,2,FooFFXVbfBarl267),"\x46\x66\x78\x56\x62\x66\x52\x65\x67\x69\x6f\x6e\x4d\x6f\x75\x6e\x74\x28\x29\x20\x52\x67\x6e\x3d\x25\x6c\x55\x20\x72\x65\x74\x75\x72\x6e\x69\x6e\x67\x20\x70\x52\x65\x67\x69\x6f\x6e\x3d\x25\x50\n",FooFFXVbfBarl149,FooFFXVbfBarr));FooFFXVbfBarl46
FooFFXVbfBarr;}FooFFXVbfBarh FooFFXVbfBarl337(FooFFXVbfBaru  *FooFFXVbfBarr){FooFFXVbfBarl28*FooFFXVbfBark;
FooFFXVbfBarl1723(FooFFXVbfBarr,FooFFXVbfBarl30( * FooFFXVbfBarr));FFXTRACEPRINTF((FooFFXVbfBarl66( FooFFXVbfBarl150,2,FooFFXVbfBarl283),"\x46\x66\x78\x56\x62\x66\x52\x65\x67\x69\x6f\x6e\x52\x65\x6c\x65\x61\x73\x65\x28\x29\x20\x70\x52\x65\x67\x69\x6f\x6e\x3d\x25\x50\x20\x52\x67\x6e\x3d\x25\x6c\x55\x20\x55\x73\x61\x67\x65\x3d\x25\x6c\x55\n",FooFFXVbfBarr,FooFFXVbfBarr-> FooFFXVbfBarl59,FooFFXVbfBarr->FooFFXVbfBarl171));
FooFFXVbfBarl27(1305,FooFFXVbfBarr->FooFFXVbfBarl183);
FooFFXVbfBarl27(1306,FooFFXVbfBarr->FooFFXVbfBarl183-> FooFFXVbfBarl293);FooFFXVbfBark=FooFFXVbfBarr->
FooFFXVbfBarl183;FooFFXVbfBarr->FooFFXVbfBarl702=
FooFFXVbfBarl996();FooFFXVbfBarn(FooFFXVbfBarl1853(& FooFFXVbfBarr->FooFFXVbfBarl171)==0){FooFFXVbfBarj
FooFFXVbfBarl1321=FooFFXVbfBarl1863(FooFFXVbfBark-> FooFFXVbfBarl293->FooFFXVbfBarl1549);FooFFXVbfBarl27(1315, FooFFXVbfBarl1321);(FooFFXVbfBarh)FooFFXVbfBarl1321;}
FFXTRACEPRINTF((FooFFXVbfBarl66(FooFFXVbfBarl150,2, FooFFXVbfBarl267),"\x46\x66\x78\x56\x62\x66\x52\x65\x67\x69\x6f\x6e\x52\x65\x6c\x65\x61\x73\x65\x28\x29\x20\x63\x6f\x6d\x70\x6c\x65\x74\x65\x64\n"));FooFFXVbfBarl46;}
FooFFXVbfBarj FooFFXVbfBarl956(FooFFXVbfBaru*FooFFXVbfBarr){
FooFFXVbfBarc FooFFXVbfBarl225*FooFFXVbfBarl92=
FooFFXVbfBarr->FooFFXVbfBarl183->FooFFXVbfBarl293;
FFXTRACEPRINTF((FooFFXVbfBarl66(FooFFXVbfBarl494,1,0),"\x46\x66\x78\x56\x62\x66\x52\x65\x67\x69\x6f\x6e\x52\x65\x6d\x6f\x75\x6e\x74\x28\x29\x20\x70\x52\x65\x67\x69\x6f\x6e\x3d\x25\x50\x20\x52\x67\x6e\x3d\x25\x6c\x55\n", FooFFXVbfBarr,FooFFXVbfBarr->FooFFXVbfBarl59));
FooFFXVbfBarl1696(FooFFXVbfBarl92->FooFFXVbfBarl1494);{
FooFFXVbfBarl423(FooFFXVbfBarr);{FooFFXVbfBarr->
FooFFXVbfBarl130=FALSE;FooFFXVbfBarl1878(FooFFXVbfBarr-> FooFFXVbfBarl183,FooFFXVbfBarr);}FooFFXVbfBarl416( FooFFXVbfBarr);}FooFFXVbfBarl1738(FooFFXVbfBarl92-> FooFFXVbfBarl1494);FooFFXVbfBarl46 FooFFXVbfBarr->
FooFFXVbfBarl130;}FFXSTATUS FooFFXVbfBarl903( FooFFXVbfBarl28*FooFFXVbfBark){FooFFXVbfBarl50
FooFFXVbfBarl149;FooFFXVbfBarl27(1387,FooFFXVbfBark);
FooFFXVbfBark->FooFFXVbfBarl272=FooFFXVbfBarl1292( FooFFXVbfBarl30( *FooFFXVbfBark->FooFFXVbfBarl272) * FooFFXVbfBark->FooFFXVbfBarl211);FooFFXVbfBarn(! FooFFXVbfBark->FooFFXVbfBarl272){FFXPRINTF(1,("\x45\x72\x72\x6f\x72\x20\x61\x6c\x6c\x6f\x63\x61\x74\x69\x6e\x67\x20\x25\x6c\x55\x20\x62\x79\x74\x65\x73\x20\x66\x6f\x72\x20\x72\x65\x67\x69\x6f\x6e\x20\x64\x61\x74\x61\x20\x73\x74\x72\x75\x63\x74\x75\x72\x65\x73\n",FooFFXVbfBarl30* FooFFXVbfBark->FooFFXVbfBarl272*FooFFXVbfBark-> FooFFXVbfBarl211));FooFFXVbfBarl46 DCLSTAT_OUTOFMEMORY;}
FooFFXVbfBarl120(FooFFXVbfBarl149=0;FooFFXVbfBarl149< FooFFXVbfBark->FooFFXVbfBarl211;FooFFXVbfBarl149++)FooFFXVbfBark
->FooFFXVbfBarl272[FooFFXVbfBarl149].FooFFXVbfBarl344=
FooFFXVbfBarl499;FooFFXVbfBarl819(FooFFXVbfBark, VBFCONF_COMPACTIONTHRESHOLD);FooFFXVbfBarl46 FooFFXVbfBarl44
;}FooFFXVbfBarl99 FooFFXVbfBaru*FooFFXVbfBarl2115( FooFFXVbfBarl29*FooFFXVbfBarl183,FooFFXVbfBarl50 FooFFXVbfBarl149){FooFFXVbfBare FooFFXVbfBarl1485;
FooFFXVbfBara FooFFXVbfBarl171=0;FooFFXVbfBarl225*
FooFFXVbfBarl92=FooFFXVbfBarl183->FooFFXVbfBarl293;
FooFFXVbfBaru*FooFFXVbfBarr=FooFFXVbfBarl79;FFXTRACEPRINTF( (FooFFXVbfBarl66(FooFFXVbfBarl150,2,FooFFXVbfBarl283),"\x46\x66\x78\x56\x62\x66\x3a\x47\x65\x74\x43\x61\x63\x68\x65\x64\x52\x65\x67\x69\x6f\x6e\x28\x29\x20\x52\x67\x6e\x3d\x25\x6c\x55\n",FooFFXVbfBarl149));FooFFXVbfBarl370("\x46\x66\x78\x56\x62\x66\x3a\x47\x65\x74\x43\x61\x63\x68\x65\x64\x52\x65\x67\x69\x6f\x6e",0,0);FooFFXVbfBarl120( FooFFXVbfBarl1485=0;FooFFXVbfBarl1485<FooFFXVbfBarl92-> FooFFXVbfBarl1026;FooFFXVbfBarl1485++){FooFFXVbfBarn( FooFFXVbfBarl92->FooFFXVbfBarl584[FooFFXVbfBarl1485]-> FooFFXVbfBarl130&&FooFFXVbfBarl92->FooFFXVbfBarl584[ FooFFXVbfBarl1485]->FooFFXVbfBarl59==FooFFXVbfBarl149){
FooFFXVbfBarr=FooFFXVbfBarl92->FooFFXVbfBarl584[
FooFFXVbfBarl1485];FooFFXVbfBarl27(1457,FooFFXVbfBarr-> FooFFXVbfBarl183==FooFFXVbfBarl183);FooFFXVbfBarl171=
FooFFXVbfBarl3375(&FooFFXVbfBarr->FooFFXVbfBarl171);
FooFFXVbfBarn(FooFFXVbfBarl171==1){FooFFXVbfBarl1706( FooFFXVbfBarl92->FooFFXVbfBarl1549);}
#if FFXCONF_STATS_VBFREGION
FooFFXVbfBarl62{FooFFXVbfBarn(FooFFXVbfBarl92-> FooFFXVbfBarl227)FooFFXVbfBarl92->FooFFXVbfBarl227[
FooFFXVbfBarl149].FooFFXVbfBarl610++;}FooFFXVbfBarl92->
FooFFXVbfBarl207++;
#endif
FooFFXVbfBarl27(1488,FooFFXVbfBarl171);FooFFXVbfBarl100;}}
FooFFXVbfBarl381(FooFFXVbfBarr!=FooFFXVbfBarl79);
FooFFXVbfBarn(FooFFXVbfBarr){FFXTRACEPRINTF(( FooFFXVbfBarl66(FooFFXVbfBarl150,2,FooFFXVbfBarl267),"\x46\x66\x78\x56\x62\x66\x3a\x47\x65\x74\x43\x61\x63\x68\x65\x64\x52\x65\x67\x69\x6f\x6e\x28\x29\x20\x52\x67\x6e\x3d\x25\x6c\x55\x20\x66\x6f\x75\x6e\x64\x20\x69\x6e\x20\x73\x6c\x6f\x74\x20\x25\x75\x2c\x20\x4e\x65\x77\x55\x73\x61\x67\x65\x3d\x25\x6c\x55\n",FooFFXVbfBarl149, FooFFXVbfBarl1485,FooFFXVbfBarl171));}FooFFXVbfBarl62{
FFXTRACEPRINTF((FooFFXVbfBarl66(FooFFXVbfBarl150,2, FooFFXVbfBarl267),"\x46\x66\x78\x56\x62\x66\x3a\x47\x65\x74\x43\x61\x63\x68\x65\x64\x52\x65\x67\x69\x6f\x6e\x28\x29\x20\x52\x67\x6e\x3d\x25\x6c\x55\x20\x6e\x6f\x74\x20\x66\x6f\x75\x6e\x64\n",FooFFXVbfBarl149));}FooFFXVbfBarl46
FooFFXVbfBarr;}FooFFXVbfBarl99 FooFFXVbfBaru*
FooFFXVbfBarl2211(FooFFXVbfBarl28*FooFFXVbfBark, FooFFXVbfBarl50 FooFFXVbfBarl149){FooFFXVbfBare
FooFFXVbfBarl60;FooFFXVbfBarl225*FooFFXVbfBarl92=
FooFFXVbfBark->FooFFXVbfBarl293;FooFFXVbfBaru*FooFFXVbfBarr
;FooFFXVbfBara FooFFXVbfBarl1504=0;FooFFXVbfBare
FooFFXVbfBarl1334=FooFFXVbfBarl1342;FooFFXVbfBara
FooFFXVbfBarl1698=D_UINT32_MAX;FFXTRACEPRINTF(( FooFFXVbfBarl66(FooFFXVbfBarl150,2,FooFFXVbfBarl283),"\x46\x66\x78\x56\x62\x66\x3a\x52\x65\x70\x6c\x61\x63\x65\x4c\x52\x55\x43\x61\x63\x68\x65\x45\x6e\x74\x72\x79\x28\x29\x20\x52\x67\x6e\x3d\x25\x6c\x55\n",FooFFXVbfBarl149));
FooFFXVbfBarl370("\x46\x66\x78\x56\x62\x66\x3a\x52\x65\x70\x6c\x61\x63\x65\x4c\x52\x55\x43\x61\x63\x68\x65\x45\x6e\x74\x72\x79",0,0);FooFFXVbfBarl120(FooFFXVbfBarl60=0; FooFFXVbfBarl60<FooFFXVbfBarl92->FooFFXVbfBarl1026; FooFFXVbfBarl60++){FooFFXVbfBaru*FooFFXVbfBarl1666=
FooFFXVbfBarl92->FooFFXVbfBarl584[FooFFXVbfBarl60];
FooFFXVbfBarn(FooFFXVbfBarl1988(&FooFFXVbfBarl1666-> FooFFXVbfBarl171,0,1)==0){FooFFXVbfBara FooFFXVbfBarl1516;
FooFFXVbfBara FooFFXVbfBarl1543;FooFFXVbfBarn(! FooFFXVbfBarl1666->FooFFXVbfBarl130){FooFFXVbfBarn( FooFFXVbfBarl1334!=FooFFXVbfBarl1342)FooFFXVbfBarl1853(& FooFFXVbfBarl92->FooFFXVbfBarl584[FooFFXVbfBarl1334]-> FooFFXVbfBarl171);FooFFXVbfBarl1504=D_UINT32_MAX;
FooFFXVbfBarl1334=FooFFXVbfBarl60;FooFFXVbfBarl1698=0;
FooFFXVbfBarl100;}FooFFXVbfBarl1516=FooFFXVbfBarl1289( FooFFXVbfBarl1666->FooFFXVbfBarl702);FooFFXVbfBarn(! FooFFXVbfBarl1516)FooFFXVbfBarl1516++;FooFFXVbfBarl1543=
FooFFXVbfBarl1378(&FooFFXVbfBark->FooFFXVbfBarl272[ FooFFXVbfBarl1666->FooFFXVbfBarl59].FooFFXVbfBarl344);
FooFFXVbfBarn(FooFFXVbfBarl1543==FooFFXVbfBarl379|| FooFFXVbfBarl1543==FooFFXVbfBarl499)FooFFXVbfBarl1543=0;
FooFFXVbfBarn(FooFFXVbfBarl1516>(FooFFXVbfBarl1504+( FooFFXVbfBarl1504>>2))||(FooFFXVbfBarl1516>( FooFFXVbfBarl1504-(FooFFXVbfBarl1504>>2))&& FooFFXVbfBarl1543<FooFFXVbfBarl1698)){FooFFXVbfBarn( FooFFXVbfBarl1334!=FooFFXVbfBarl1342)FooFFXVbfBarl1853(& FooFFXVbfBarl92->FooFFXVbfBarl584[FooFFXVbfBarl1334]-> FooFFXVbfBarl171);FooFFXVbfBarl1504=FooFFXVbfBarl1516;
FooFFXVbfBarl1334=FooFFXVbfBarl60;FooFFXVbfBarl1698=
FooFFXVbfBarl1543;}FooFFXVbfBarl62{FooFFXVbfBarl1853(& FooFFXVbfBarl1666->FooFFXVbfBarl171);}}}
#if D_DEBUG
FooFFXVbfBarn(FooFFXVbfBarl1334==FooFFXVbfBarl1342)FooFFXVbfBarl682
((FooFFXVbfBarc FooFFXVbfBaru* * )&FooFFXVbfBarl92-> FooFFXVbfBarl584[0],FooFFXVbfBarl92->FooFFXVbfBarl1026, FALSE);
#endif
FooFFXVbfBarl372(1645,FooFFXVbfBarl1334!=FooFFXVbfBarl1342);
FooFFXVbfBarr=FooFFXVbfBarl92->FooFFXVbfBarl584[
FooFFXVbfBarl1334];
#if FFXCONF_NANDSUPPORT && VBFCONF_DEFERREDDISCARD
FooFFXVbfBarn(FooFFXVbfBark->FooFFXVbfBarl161&& FooFFXVbfBarr->FooFFXVbfBarl203){FFXSTATUS FooFFXVbfBarl26;
FooFFXVbfBarl27(1664,!FooFFXVbfBark->FooFFXVbfBarl559);
FooFFXVbfBarl27(1665,FooFFXVbfBark->FooFFXVbfBarl437);
FooFFXVbfBarl26=FooFFXVbfBarl589(FooFFXVbfBark, FooFFXVbfBarr,FALSE);FooFFXVbfBarl27(1669,FooFFXVbfBarl26== FooFFXVbfBarl44);FooFFXVbfBarl27(1670,!FooFFXVbfBarr-> FooFFXVbfBarl203);(FooFFXVbfBarh)FooFFXVbfBarl26;}
#endif
FFXTRACEPRINTF((FooFFXVbfBarl66(FooFFXVbfBarl150,1,0),"\x46\x66\x78\x56\x62\x66\x3a\x52\x65\x70\x6c\x61\x63\x65\x4c\x52\x55\x43\x61\x63\x68\x65\x45\x6e\x74\x72\x79\x28\x29\x20\x4c\x52\x55\x27\x64\x20\x52\x67\x6e\x3d\x25\x6c\x55\x20\x66\x72\x6f\x6d\x20\x73\x6c\x6f\x74\x20\x25\x75\x20\x69\x6e\x20\x74\x68\x65\x20\x63\x61\x63\x68\x65\x2c\x20\x41\x67\x65\x3d\x25\x6c\x58\x20\x43\x6f\x6d\x70\x4b\x65\x79\x3d\x25\x6c\x58\n",FooFFXVbfBarr->FooFFXVbfBarl59, FooFFXVbfBarl1334,FooFFXVbfBarl1504,FooFFXVbfBarl1698));
FooFFXVbfBarr->FooFFXVbfBarl183=FooFFXVbfBark;FooFFXVbfBarr
->FooFFXVbfBarl130=FALSE;FooFFXVbfBarr->FooFFXVbfBarl59=
FooFFXVbfBarl149;FooFFXVbfBarl381(0UL);FFXTRACEPRINTF(( FooFFXVbfBarl66(FooFFXVbfBarl150,1,FooFFXVbfBarl267),"\x46\x66\x78\x56\x62\x66\x3a\x52\x65\x70\x6c\x61\x63\x65\x4c\x52\x55\x43\x61\x63\x68\x65\x45\x6e\x74\x72\x79\x28\x29\x20\x72\x65\x74\x75\x72\x6e\x69\x6e\x67\x20\x70\x52\x65\x67\x69\x6f\x6e\x3d\x25\x50\x20\x52\x67\x6e\x3d\x25\x6c\x55\n",FooFFXVbfBarr,FooFFXVbfBarr->FooFFXVbfBarl59));
FooFFXVbfBarl46 FooFFXVbfBarr;}FooFFXVbfBarl99
FooFFXVbfBarl35 FooFFXVbfBarl2683(FooFFXVbfBarc FooFFXVbfBarl29*FooFFXVbfBars,FooFFXVbfBarc FooFFXVbfBarl91  *FooFFXVbfBarl258,FooFFXVbfBare FooFFXVbfBarl2652){
FooFFXVbfBare FooFFXVbfBarl321,FooFFXVbfBarl2266=0;
FooFFXVbfBarj FooFFXVbfBarl2343=FALSE;FooFFXVbfBarl35
FooFFXVbfBarl=FooFFXVbfBarl377;FFXSTATUS FooFFXVbfBarl26=
FooFFXVbfBarl44;FooFFXVbfBarl120(FooFFXVbfBarl321=0; FooFFXVbfBarl321<FooFFXVbfBars->FooFFXVbfBarl129; FooFFXVbfBarl321++){FooFFXVbfBarn(FooFFXVbfBarl258[ FooFFXVbfBarl321].FooFFXVbfBarl175==FooFFXVbfBarl231&& FooFFXVbfBarl258[FooFFXVbfBarl321].FooFFXVbfBarl275== FooFFXVbfBarl2652){FooFFXVbfBarl2343=TRUE;FooFFXVbfBarl2266
=FooFFXVbfBarl321;}}FooFFXVbfBarn(FooFFXVbfBarl2343){
FooFFXVbfBarl.FooFFXVbfBarl26=FooFFXVbfBarl26;FooFFXVbfBarl
.FooFFXVbfBarv=FooFFXVbfBarl2266;}FooFFXVbfBarl62{
FooFFXVbfBarl.FooFFXVbfBarl26=FooFFXVbfBarl772;}
FooFFXVbfBarl46 FooFFXVbfBarl;}FooFFXVbfBarl99 FFXSTATUS
FooFFXVbfBarl2276(FooFFXVbfBarl29*FooFFXVbfBars, FooFFXVbfBaru*FooFFXVbfBarw,FooFFXVbfBarl38 FooFFXVbfBarl1525){FooFFXVbfBarl38 FooFFXVbfBarl264,
FooFFXVbfBarl123;FooFFXVbfBarl32 FooFFXVbfBarl197,
FooFFXVbfBarl170;FooFFXVbfBarl68 FooFFXVbfBarl104;
FooFFXVbfBarl198*FooFFXVbfBarl1346;FooFFXVbfBarl91*
FooFFXVbfBarl151, *FooFFXVbfBarl1472;FooFFXVbfBare
FooFFXVbfBarl60;FooFFXVbfBarl35 FooFFXVbfBarl=
FooFFXVbfBarl286(FooFFXVbfBarl44);FooFFXVbfBarl27(1801, FooFFXVbfBarw);FFXTRACEPRINTF((FooFFXVbfBarl66( FooFFXVbfBarl150,2,FooFFXVbfBarl283),"\x46\x66\x78\x56\x62\x66\x3a\x48\x61\x6e\x64\x6c\x65\x49\x6e\x74\x65\x72\x72\x75\x70\x74\x65\x64\x43\x6f\x6d\x70\x61\x63\x74\x69\x6f\x6e\x73\x28\x29\x20\x52\x67\x6e\x3d\x25\x6c\x55\x20\x4c\x69\x6d\x69\x74\x3d\x25\x55\n",FooFFXVbfBarw-> FooFFXVbfBarl59,FooFFXVbfBarl1525));FooFFXVbfBarl27(1810, FooFFXVbfBarw->FooFFXVbfBarl215>FooFFXVbfBarl1525);
FooFFXVbfBarl264=FooFFXVbfBarw->FooFFXVbfBarl353;
FooFFXVbfBarl197=FooFFXVbfBarw->FooFFXVbfBarl53[
FooFFXVbfBarl264].FooFFXVbfBarl43;FooFFXVbfBarn( FooFFXVbfBarw->FooFFXVbfBarl159==FooFFXVbfBarl264){
FFXPRINTF(3,("\x46\x66\x78\x56\x62\x66\x3a\x48\x61\x6e\x64\x6c\x65\x49\x6e\x74\x65\x72\x72\x75\x70\x74\x65\x64\x43\x6f\x6d\x70\x61\x63\x74\x69\x6f\x6e\x73\x28\x29\x20\x4d\x65\x74\x61\x64\x61\x74\x61\x20\x77\x61\x73\x20\x77\x72\x69\x74\x74\x65\x6e\n"));FooFFXVbfBarl73 FooFFXVbfBarl1727
;}FooFFXVbfBarl=FooFFXVbfBarl853(FooFFXVbfBars, FooFFXVbfBarl197,&FooFFXVbfBarl104);FooFFXVbfBarn( FooFFXVbfBarl.FooFFXVbfBarl26==(FooFFXVbfBarq(4,20))){
FooFFXVbfBarl386(FooFFXVbfBarw,FooFFXVbfBarl264);
FooFFXVbfBarl1287(&FooFFXVbfBarl);}FooFFXVbfBarn( FooFFXVbfBarl.FooFFXVbfBarl26!=FooFFXVbfBarl44)FooFFXVbfBarl73
FooFFXVbfBarl1727;FooFFXVbfBarl170=(FooFFXVbfBarl32)FooFFXVbfBarl104
.FooFFXVbfBarl794;FooFFXVbfBarn(!FooFFXVbfBarl527( FooFFXVbfBarw,FooFFXVbfBarl170,&FooFFXVbfBarl123)){
FFXPRINTF(3,("\x46\x66\x78\x56\x62\x66\x3a\x48\x61\x6e\x64\x6c\x65\x49\x6e\x74\x65\x72\x72\x75\x70\x74\x65\x64\x43\x6f\x6d\x70\x61\x63\x74\x69\x6f\x6e\x73\x28\x29\x20\x4e\x6f\x74\x68\x69\x6e\x67\x20\x74\x6f\x20\x64\x6f\n"));
FooFFXVbfBarl73 FooFFXVbfBarl1727;}FooFFXVbfBarl1346=&
FooFFXVbfBarw->FooFFXVbfBarl53[FooFFXVbfBarl123];
FooFFXVbfBarn(FooFFXVbfBarl1346->FooFFXVbfBarl169> FooFFXVbfBarw->FooFFXVbfBarl53[FooFFXVbfBarl264]. FooFFXVbfBarl169){FFXPRINTF(3,("\x46\x66\x78\x56\x62\x66\x3a\x48\x61\x6e\x64\x6c\x65\x49\x6e\x74\x65\x72\x72\x75\x70\x74\x65\x64\x43\x6f\x6d\x70\x61\x63\x74\x69\x6f\x6e\x73\x28\x29\x20\x4e\x6f\x74\x20\x6f\x6c\x64\x65\x72\n"));
FooFFXVbfBarl73 FooFFXVbfBarl1727;}FooFFXVbfBarn(( FooFFXVbfBars->FooFFXVbfBarl129==FooFFXVbfBarl1346-> FooFFXVbfBarl147)&&(FooFFXVbfBarw->FooFFXVbfBarl159!= FooFFXVbfBarl123)){
#if FFXCONF_NORSUPPORT || FFXCONF_ISWFSUPPORT
FooFFXVbfBarn(!FooFFXVbfBars->FooFFXVbfBarl161){
FooFFXVbfBarl.FooFFXVbfBarl26=FooFFXVbfBarl964( FooFFXVbfBars,FooFFXVbfBarl170,FooFFXVbfBars-> FooFFXVbfBarl293->FooFFXVbfBarl2415.FooFFXVbfBarl273, FooFFXVbfBarl1750);}
#endif
}FooFFXVbfBarl62{FooFFXVbfBarl1472=FooFFXVbfBarl1376( FooFFXVbfBarl30( *FooFFXVbfBarl1472) *FooFFXVbfBars-> FooFFXVbfBarl129);FooFFXVbfBarn(!FooFFXVbfBarl1472){
FFXPRINTF(1,("\x57\x61\x72\x6e\x69\x6e\x67\x3a\x20\x48\x61\x6e\x64\x6c\x65\x49\x6e\x74\x65\x72\x72\x75\x70\x74\x65\x64\x43\x6f\x6d\x70\x61\x63\x74\x69\x6f\x6e\x73\x28\x29\x20\x63\x61\x6e\x6e\x6f\x74\x20\x61\x6c\x6c\x6f\x63\x61\x74\x65\x20\x61\x6c\x6c\x6f\x63\x61\x74\x69\x6f\x6e\x20\x62\x75\x66\x66\x65\x72\x2e\n"));FooFFXVbfBarl.
FooFFXVbfBarl26=FooFFXVbfBarl373;}FooFFXVbfBarl62{
FooFFXVbfBare FooFFXVbfBarl321;FooFFXVbfBarl120( FooFFXVbfBarl321=0;FooFFXVbfBarl321<FooFFXVbfBars-> FooFFXVbfBarl129;FooFFXVbfBarl321+=(FooFFXVbfBare)FooFFXVbfBarl .FooFFXVbfBarv){FooFFXVbfBarl=FooFFXVbfBarl1314( FooFFXVbfBars,FooFFXVbfBarl170,FooFFXVbfBarl321, FooFFXVbfBarl1750,&FooFFXVbfBarl1472[FooFFXVbfBarl321]);
FooFFXVbfBarn(FooFFXVbfBarl.FooFFXVbfBarl26==(FooFFXVbfBarq (4,20))){FooFFXVbfBarl386(FooFFXVbfBarw,FooFFXVbfBarl123);
FooFFXVbfBarl1287(&FooFFXVbfBarl);}FooFFXVbfBarl62
FooFFXVbfBarn(FooFFXVbfBarl.FooFFXVbfBarl26!= FooFFXVbfBarl44){FooFFXVbfBarl100;}FooFFXVbfBarl27(1938, FooFFXVbfBarl.FooFFXVbfBarv!=0);}FooFFXVbfBarn( FooFFXVbfBarl.FooFFXVbfBarl26==FooFFXVbfBarl44){
FooFFXVbfBarl151=FooFFXVbfBarw->FooFFXVbfBarl208;
FooFFXVbfBarl120(FooFFXVbfBarl60=0;FooFFXVbfBarl60< FooFFXVbfBarl93;FooFFXVbfBarl60++,FooFFXVbfBarl151++){
FooFFXVbfBarn((FooFFXVbfBarl151->FooFFXVbfBarl175== FooFFXVbfBarl231)&&(FooFFXVbfBarl151->FooFFXVbfBarl33== FooFFXVbfBarl264)){FooFFXVbfBarl=FooFFXVbfBarl2683( FooFFXVbfBars,FooFFXVbfBarl1472,FooFFXVbfBarl151-> FooFFXVbfBarl275);FooFFXVbfBarn(FooFFXVbfBarl. FooFFXVbfBarl26==(FooFFXVbfBarq(4,20))){FooFFXVbfBarl386( FooFFXVbfBarw,FooFFXVbfBarl123);FooFFXVbfBarl1287(& FooFFXVbfBarl);}FooFFXVbfBarn(FooFFXVbfBarl.FooFFXVbfBarl26 !=FooFFXVbfBarl44){
#if D_DEBUG
FooFFXVbfBarl752(FooFFXVbfBarw);
#endif
FooFFXVbfBarl100;}FooFFXVbfBarl151->FooFFXVbfBarl33=
FooFFXVbfBarl123;FooFFXVbfBarl151->FooFFXVbfBarl219=
FooFFXVbfBarl.FooFFXVbfBarv;--FooFFXVbfBarl1346->
FooFFXVbfBarl147;FooFFXVbfBarl1854(FooFFXVbfBarw,( FooFFXVbfBarl38)FooFFXVbfBarl151->FooFFXVbfBarl33, FooFFXVbfBarl151->FooFFXVbfBarl219,FALSE);}}}}FooFFXVbfBarn
(FooFFXVbfBarl.FooFFXVbfBarl26==FooFFXVbfBarl44){
FooFFXVbfBarl552(FooFFXVbfBarw,FooFFXVbfBarl264);
FooFFXVbfBarl382(FooFFXVbfBars,FooFFXVbfBarl197, FooFFXVbfBarl343);--FooFFXVbfBarw->FooFFXVbfBarl215;
FooFFXVbfBarl.FooFFXVbfBarl26=FooFFXVbfBarl2339( FooFFXVbfBars,FooFFXVbfBarl197);}FooFFXVbfBarn( FooFFXVbfBarl1472)FooFFXVbfBarl323(FooFFXVbfBarl1472);}
FooFFXVbfBarl1727:FFXTRACEPRINTF((FooFFXVbfBarl66( FooFFXVbfBarl150,2,FooFFXVbfBarl267),"\x46\x66\x78\x56\x62\x66\x3a\x48\x61\x6e\x64\x6c\x65\x49\x6e\x74\x65\x72\x72\x75\x70\x74\x65\x64\x43\x6f\x6d\x70\x61\x63\x74\x69\x6f\x6e\x73\x28\x29\x20\x72\x65\x74\x75\x72\x6e\x69\x6e\x67\x20\x25\x6c\x58\n",FooFFXVbfBarl.FooFFXVbfBarl26));
FooFFXVbfBarl46 FooFFXVbfBarl.FooFFXVbfBarl26;}
FooFFXVbfBarl99 FFXSTATUS FooFFXVbfBarl2003(FooFFXVbfBarl29  *FooFFXVbfBars,FooFFXVbfBaru*FooFFXVbfBarw,FooFFXVbfBarl38 FooFFXVbfBarl1525){FooFFXVbfBarl38 FooFFXVbfBarl33;
FooFFXVbfBare FooFFXVbfBarl1884;FooFFXVbfBarl198*
FooFFXVbfBarl199;FooFFXVbfBarl32 FooFFXVbfBarl43;
FooFFXVbfBarl1884=FooFFXVbfBarw->FooFFXVbfBarl215;
FooFFXVbfBarl33=FooFFXVbfBarw->FooFFXVbfBarl226;
FooFFXVbfBarl153(FooFFXVbfBarw->FooFFXVbfBarl215> FooFFXVbfBarl1525){FooFFXVbfBarn((FooFFXVbfBarl33== FooFFXVbfBarl132)||(FooFFXVbfBarl1884==0))FooFFXVbfBarl46
FooFFXVbfBarl733;--FooFFXVbfBarl1884;FooFFXVbfBarl199=&
FooFFXVbfBarw->FooFFXVbfBarl53[FooFFXVbfBarl33];
FooFFXVbfBarn((FooFFXVbfBarl33!=FooFFXVbfBarw-> FooFFXVbfBarl159)&&(FooFFXVbfBarl199->FooFFXVbfBarl144==0)&& (FooFFXVbfBars->FooFFXVbfBarl129==FooFFXVbfBarl199-> FooFFXVbfBarl147)){FooFFXVbfBarl43=FooFFXVbfBarl199->
FooFFXVbfBarl43;FooFFXVbfBarl33=FooFFXVbfBarl552( FooFFXVbfBarw,FooFFXVbfBarl33);--FooFFXVbfBarw->
FooFFXVbfBarl215;FooFFXVbfBarl382(FooFFXVbfBars, FooFFXVbfBarl43,FooFFXVbfBarl343);}FooFFXVbfBarl62{
FooFFXVbfBarl33=FooFFXVbfBarl291(FooFFXVbfBarw, FooFFXVbfBarl33);}}FooFFXVbfBarl46 FooFFXVbfBarl44;}
FooFFXVbfBarl99 FFXSTATUS FooFFXVbfBarl2339(FooFFXVbfBarl29  *FooFFXVbfBars,FooFFXVbfBarl32 FooFFXVbfBarl43){
FooFFXVbfBarl68 FooFFXVbfBarl104;FooFFXVbfBarl35
FooFFXVbfBarl;FFXTRACEPRINTF((FooFFXVbfBarl66( FFXTRACE_COMPACTION,2,0),"\x46\x66\x78\x56\x62\x66\x3a\x46\x6f\x72\x6d\x61\x74\x53\x70\x61\x72\x65\x4c\x6e\x75\x28\x29\x20\x4c\x4e\x55\x3d\x25\x75\n",FooFFXVbfBarl43));
FooFFXVbfBarl=FooFFXVbfBarl853(FooFFXVbfBars, FooFFXVbfBarl43,&FooFFXVbfBarl104);FooFFXVbfBarl1152( FooFFXVbfBarl.FooFFXVbfBarl26){FooFFXVbfBarl143( FooFFXVbfBarq(4,20)):FooFFXVbfBarl.FooFFXVbfBarl26=
FooFFXVbfBarl44;FooFFXVbfBarl143 FooFFXVbfBarl44:
FooFFXVbfBarn(FooFFXVbfBarl399(&FooFFXVbfBarl104)){++
FooFFXVbfBarl104.FooFFXVbfBarl324;FooFFXVbfBarl104.
FooFFXVbfBarl169=FooFFXVbfBarl574;FooFFXVbfBarl104.
FooFFXVbfBarl794=FooFFXVbfBarl574;FooFFXVbfBarl104.
FooFFXVbfBarl477=FooFFXVbfBarl574;FooFFXVbfBarl.
FooFFXVbfBarl26=FooFFXVbfBarl1369(FooFFXVbfBars, FooFFXVbfBarl43,&FooFFXVbfBarl104);}FooFFXVbfBarl100;
FooFFXVbfBarl143(FooFFXVbfBarq(14,4)):FooFFXVbfBarl143( FooFFXVbfBarq(14,3)):FooFFXVbfBarl.FooFFXVbfBarl26=
FooFFXVbfBarl44;FooFFXVbfBarl100;FooFFXVbfBarl143( FooFFXVbfBarq(4,3)):FooFFXVbfBarl100;FooFFXVbfBarl1227:
FooFFXVbfBarl112(2150);FooFFXVbfBarl100;}FooFFXVbfBarl46
FooFFXVbfBarl.FooFFXVbfBarl26;}FooFFXVbfBarl99 FooFFXVbfBarh
FooFFXVbfBarl1878(FooFFXVbfBarl29*FooFFXVbfBars, FooFFXVbfBaru*FooFFXVbfBarw){FooFFXVbfBarl38 FooFFXVbfBarl33
,FooFFXVbfBarl1260;FFXSTATUS FooFFXVbfBarl26;
FooFFXVbfBarl32 FooFFXVbfBarl2452;
#if FFXCONF_STATS_VBFREGION
DCLTIMESTAMP FooFFXVbfBarl1722;
#endif
FFXTRACEPRINTF((FooFFXVbfBarl66(FooFFXVbfBarl150,2, FooFFXVbfBarl283),"\x46\x66\x78\x56\x62\x66\x3a\x50\x72\x6f\x63\x65\x73\x73\x52\x65\x67\x69\x6f\x6e\x4d\x6f\x75\x6e\x74\x28\x29\x20\x52\x3d\x25\x55\n",FooFFXVbfBarw-> FooFFXVbfBarl59));FooFFXVbfBarl370("\x46\x66\x78\x56\x62\x66\x3a\x50\x72\x6f\x63\x65\x73\x73\x52\x65\x67\x69\x6f\x6e\x4d\x6f\x75\x6e\x74",0,0);
#if FFXCONF_STATS_VBFREGION
FooFFXVbfBarl1722=FooFFXVbfBarl996();
#endif
FooFFXVbfBarl27(2193,!FooFFXVbfBarw->FooFFXVbfBarl130);
FooFFXVbfBarl1260=(FooFFXVbfBarl38)FooFFXVbfBars->
FooFFXVbfBarl165;FooFFXVbfBarn(FooFFXVbfBarw-> FooFFXVbfBarl59==FooFFXVbfBars->FooFFXVbfBarl211-1){
FooFFXVbfBarl2452=(FooFFXVbfBars->FooFFXVbfBarl131-( FooFFXVbfBarl32)(FooFFXVbfBars->FooFFXVbfBarl180))%
FooFFXVbfBars->FooFFXVbfBarl165;FooFFXVbfBarl1260=( FooFFXVbfBarl38)FooFFXVbfBarl2452;FooFFXVbfBarn( FooFFXVbfBarl1260==0){FooFFXVbfBarl1260=(FooFFXVbfBarl38)FooFFXVbfBars
->FooFFXVbfBarl165;}FooFFXVbfBarl27(2212,FooFFXVbfBarl1260 <=FooFFXVbfBars->FooFFXVbfBarl165);}FooFFXVbfBarw->
FooFFXVbfBarl215=FooFFXVbfBarl2061(FooFFXVbfBars, FooFFXVbfBarw);FooFFXVbfBarn(FooFFXVbfBarw-> FooFFXVbfBarl215<FooFFXVbfBarl1260){FFXPRINTF(1,("\x46\x66\x78\x56\x62\x66\x3a\x50\x72\x6f\x63\x65\x73\x73\x52\x65\x67\x69\x6f\x6e\x4d\x6f\x75\x6e\x74\x28\x29\x20\x52\x65\x67\x69\x6f\x6e\x20\x25\x55\x20\x6d\x69\x73\x73\x69\x6e\x67\x20\x75\x6e\x69\x74\x73\x2c\x20\x65\x78\x70\x65\x63\x74\x65\x64\x20\x25\x75\x2c\x20\x66\x6f\x75\x6e\x64\x20\x6f\x6e\x6c\x79\x20\x25\x55\n",FooFFXVbfBarw-> FooFFXVbfBarl59,FooFFXVbfBarl1260,FooFFXVbfBarw-> FooFFXVbfBarl215));FooFFXVbfBarl112(2230);FooFFXVbfBarl73
FooFFXVbfBarl1694;}
#if FFXCONF_NANDSUPPORT
FooFFXVbfBarn(FooFFXVbfBars->FooFFXVbfBarl161&& FooFFXVbfBarw->FooFFXVbfBarl159!=FooFFXVbfBarl132){( FooFFXVbfBarh)FooFFXVbfBarl848(FooFFXVbfBars,FooFFXVbfBarw);
FooFFXVbfBarl27(2264,FooFFXVbfBarw->FooFFXVbfBarl203==FALSE );FooFFXVbfBarw->FooFFXVbfBarl203=FALSE;}FooFFXVbfBarl62{
FooFFXVbfBarl27(2275,FooFFXVbfBarw->FooFFXVbfBarl203==FALSE );}
#endif
FooFFXVbfBarn(FooFFXVbfBarw->FooFFXVbfBarl215> FooFFXVbfBarl1260){FooFFXVbfBarl26=FooFFXVbfBarl2276( FooFFXVbfBars,FooFFXVbfBarw,FooFFXVbfBarl1260);
FooFFXVbfBarn(FooFFXVbfBarl26!=FooFFXVbfBarl44){FFXPRINTF(1 ,("\x48\x61\x6e\x64\x6c\x65\x49\x6e\x74\x65\x72\x72\x75\x70\x74\x65\x64\x43\x6f\x6d\x70\x61\x63\x74\x69\x6f\x6e\x73\x20\x66\x61\x69\x6c\x65\x64\x21\x20\x53\x74\x61\x74\x75\x73\x3d\x25\x6c\x58\x20\x55\x6e\x69\x74\x73\x3d\x25\x55\x20\x43\x6f\x72\x72\x65\x63\x74\x3d\x25\x75\n", FooFFXVbfBarl26,FooFFXVbfBarw->FooFFXVbfBarl215, FooFFXVbfBarl1260));FooFFXVbfBarl338(2294);FooFFXVbfBarl73
FooFFXVbfBarl1694;}}
#if FFXCONF_NORSUPPORT || FFXCONF_ISWFSUPPORT
FooFFXVbfBarn(!FooFFXVbfBars->FooFFXVbfBarl161){
FooFFXVbfBarn(!FooFFXVbfBarl940(FooFFXVbfBarw,FooFFXVbfBars ->FooFFXVbfBarl365)){FooFFXVbfBarl112(2314);FooFFXVbfBars->
FooFFXVbfBarl327=TRUE;FooFFXVbfBarl73 FooFFXVbfBarl1694;}}
#endif
#if FFXCONF_NANDSUPPORT
FooFFXVbfBarn(FooFFXVbfBars->FooFFXVbfBarl161&& FooFFXVbfBarw->FooFFXVbfBarl159==FooFFXVbfBarl132){
FooFFXVbfBarl357(FooFFXVbfBarw->FooFFXVbfBarl242,0, FooFFXVbfBarl30 FooFFXVbfBarw->FooFFXVbfBarl242);}
#endif
FooFFXVbfBarn(FooFFXVbfBarw->FooFFXVbfBarl215!= FooFFXVbfBarl1260){FooFFXVbfBarl26=FooFFXVbfBarl2003( FooFFXVbfBars,FooFFXVbfBarw,FooFFXVbfBarl1260);
FooFFXVbfBarn((FooFFXVbfBarl26!=FooFFXVbfBarl44)||( FooFFXVbfBarw->FooFFXVbfBarl215!=FooFFXVbfBarl1260)){
FooFFXVbfBarn(FooFFXVbfBarl26!=FooFFXVbfBarl44){FFXPRINTF(1 ,("\x46\x66\x78\x56\x62\x66\x3a\x53\x61\x6c\x76\x61\x67\x65\x53\x70\x65\x6e\x74\x55\x6e\x69\x74\x73\x28\x29\x20\x66\x61\x69\x6c\x65\x64\x2c\x20\x73\x74\x61\x74\x75\x73\x20\x25\x6c\x58\n",FooFFXVbfBarl26));}FooFFXVbfBarn( FooFFXVbfBarw->FooFFXVbfBarl215!=FooFFXVbfBarl1260){
FFXPRINTF(1,("\x46\x66\x78\x56\x62\x66\x3a\x50\x72\x6f\x63\x65\x73\x73\x52\x65\x67\x69\x6f\x6e\x4d\x6f\x75\x6e\x74\x28\x29\x3a\x20\x77\x72\x6f\x6e\x67\x20\x6e\x75\x6d\x62\x65\x72\x20\x6f\x66\x20\x75\x6e\x69\x74\x73\x20\x61\x66\x74\x65\x72\x20\x53\x61\x6c\x76\x61\x67\x65\x53\x70\x65\x6e\x74\x55\x6e\x69\x74\x73\x28\x29\x3a\x20\x65\x78\x70\x65\x63\x74\x65\x64\x20\x25\x55\x2c\x20\x67\x6f\x74\x20\x25\x75\n",FooFFXVbfBarl1260,FooFFXVbfBarw-> FooFFXVbfBarl215));}FooFFXVbfBarl338(2354);FooFFXVbfBarl73
FooFFXVbfBarl1694;}}FooFFXVbfBarw->FooFFXVbfBarl307=0;
FooFFXVbfBarw->FooFFXVbfBarl328=0;FooFFXVbfBarl120( FooFFXVbfBarl33=FooFFXVbfBarw->FooFFXVbfBarl226; FooFFXVbfBarl33!=FooFFXVbfBarl132;FooFFXVbfBarl33= FooFFXVbfBarl291(FooFFXVbfBarw,FooFFXVbfBarl33)){
FooFFXVbfBarw->FooFFXVbfBarl307+=FooFFXVbfBarw->
FooFFXVbfBarl53[FooFFXVbfBarl33].FooFFXVbfBarl144;
FooFFXVbfBarw->FooFFXVbfBarl328+=FooFFXVbfBarw->
FooFFXVbfBarl53[FooFFXVbfBarl33].FooFFXVbfBarl147;}
FooFFXVbfBarw->FooFFXVbfBarl130=TRUE;
#if D_DEBUG
FooFFXVbfBarl325(FooFFXVbfBars,FooFFXVbfBarw,"\x46\x66\x78\x56\x62\x66\x3a\x50\x72\x6f\x63\x65\x73\x73\x52\x65\x67\x69\x6f\x6e\x4d\x6f\x75\x6e\x74");
#endif
FooFFXVbfBarl1694:
#if FFXCONF_STATS_VBFREGION
FooFFXVbfBars->FooFFXVbfBarl293->FooFFXVbfBarl390+=
FooFFXVbfBarl1289(FooFFXVbfBarl1722);
#endif
FooFFXVbfBarl381(0UL);FFXTRACEPRINTF((FooFFXVbfBarl66( FooFFXVbfBarl150,2,FooFFXVbfBarl267),"\x46\x66\x78\x56\x62\x66\x3a\x50\x72\x6f\x63\x65\x73\x73\x52\x65\x67\x69\x6f\x6e\x4d\x6f\x75\x6e\x74\x28\x29\x20\x72\x65\x74\x75\x72\x6e\x69\x6e\x67\n"));FooFFXVbfBarl46;}FooFFXVbfBara
FooFFXVbfBarl919(FooFFXVbfBarc FooFFXVbfBaru*FooFFXVbfBarw){
FooFFXVbfBarl46 FooFFXVbfBarw->FooFFXVbfBarl53[
FooFFXVbfBarw->FooFFXVbfBarl353].FooFFXVbfBarl169+1;}
FooFFXVbfBarl99 FooFFXVbfBarh FooFFXVbfBarl2314( FooFFXVbfBarl198*FooFFXVbfBarl1515,FooFFXVbfBari FooFFXVbfBarl201){FooFFXVbfBare FooFFXVbfBarl60,
FooFFXVbfBarl371;FooFFXVbfBarl198 FooFFXVbfBarl1799;
FooFFXVbfBara FooFFXVbfBarl2067;FooFFXVbfBarl27(2438, FooFFXVbfBarl201<=FooFFXVbfBarl210);FooFFXVbfBarl120( FooFFXVbfBarl60=1;FooFFXVbfBarl60<FooFFXVbfBarl201; FooFFXVbfBarl60++){FooFFXVbfBarl1799=FooFFXVbfBarl1515[
FooFFXVbfBarl60];FooFFXVbfBarl371=FooFFXVbfBarl60;
FooFFXVbfBarl2067=FooFFXVbfBarl1799.FooFFXVbfBarl169;
FooFFXVbfBarl153((FooFFXVbfBarl1515[FooFFXVbfBarl371-1]. FooFFXVbfBarl169>FooFFXVbfBarl2067)&&(FooFFXVbfBarl371>0)){
FooFFXVbfBarl1515[FooFFXVbfBarl371]=FooFFXVbfBarl1515[
FooFFXVbfBarl371-1];--FooFFXVbfBarl371;}FooFFXVbfBarl1515[
FooFFXVbfBarl371]=FooFFXVbfBarl1799;}}FooFFXVbfBarl38
FooFFXVbfBarl291(FooFFXVbfBarc FooFFXVbfBaru*FooFFXVbfBarw, FooFFXVbfBarl38 FooFFXVbfBarl33){FooFFXVbfBarn( FooFFXVbfBarl33!=FooFFXVbfBarl132){FooFFXVbfBarl33=
FooFFXVbfBarw->FooFFXVbfBarl53[FooFFXVbfBarl33].
FooFFXVbfBarl288;}FooFFXVbfBarl46 FooFFXVbfBarl33;}
FooFFXVbfBarl38 FooFFXVbfBarl552(FooFFXVbfBaru* FooFFXVbfBarw,FooFFXVbfBarl38 FooFFXVbfBarl33){
FooFFXVbfBarl38 FooFFXVbfBarl336,FooFFXVbfBarl288;
FooFFXVbfBarl38*FooFFXVbfBarl1898, *FooFFXVbfBarl1895;
FooFFXVbfBarl198*FooFFXVbfBarl199;FooFFXVbfBarn( FooFFXVbfBarw->FooFFXVbfBarl215==0){FooFFXVbfBarl338(2503);
FooFFXVbfBarl46 FooFFXVbfBarl132;}FooFFXVbfBarl199=&
FooFFXVbfBarw->FooFFXVbfBarl53[0];FooFFXVbfBarl288=
FooFFXVbfBarl199[FooFFXVbfBarl33].FooFFXVbfBarl288;
FooFFXVbfBarl336=FooFFXVbfBarl199[FooFFXVbfBarl33].
FooFFXVbfBarl336;FooFFXVbfBarn(FooFFXVbfBarl33== FooFFXVbfBarw->FooFFXVbfBarl226){FooFFXVbfBarl1898=&
FooFFXVbfBarw->FooFFXVbfBarl226;}FooFFXVbfBarl62{
FooFFXVbfBarl1898=&FooFFXVbfBarl199[FooFFXVbfBarl336].
FooFFXVbfBarl288;}FooFFXVbfBarn(FooFFXVbfBarl33== FooFFXVbfBarw->FooFFXVbfBarl353){FooFFXVbfBarl1895=&
FooFFXVbfBarw->FooFFXVbfBarl353;}FooFFXVbfBarl62{
FooFFXVbfBarl1895=&FooFFXVbfBarl199[FooFFXVbfBarl288].
FooFFXVbfBarl336;} *FooFFXVbfBarl1898=FooFFXVbfBarl288; *
FooFFXVbfBarl1895=FooFFXVbfBarl336;FooFFXVbfBarl199[
FooFFXVbfBarl33].FooFFXVbfBarl336=FooFFXVbfBarl199[
FooFFXVbfBarl33].FooFFXVbfBarl288=0;FooFFXVbfBarl46
FooFFXVbfBarl288;}FooFFXVbfBarl38 FooFFXVbfBarl717( FooFFXVbfBaru*FooFFXVbfBarw,FooFFXVbfBarl38 FooFFXVbfBarl33 ){FooFFXVbfBarl198*FooFFXVbfBarl199, *FooFFXVbfBarl1924;
FooFFXVbfBarl38 FooFFXVbfBarl1726;FooFFXVbfBarn( FooFFXVbfBarw->FooFFXVbfBarl215>=FooFFXVbfBarl210){
FFXPRINTF(3,("\x70\x54\x68\x69\x73\x52\x65\x67\x69\x6f\x6e\x2d\x3e\x6e\x75\x6d\x55\x6e\x69\x74\x73\x3d\x25\x55\x20\x4c\x4f\x47\x49\x43\x41\x4c\x5f\x55\x4e\x49\x54\x5f\x4d\x41\x58\x3d\x25\x75\n",FooFFXVbfBarw->FooFFXVbfBarl215, FooFFXVbfBarl210));FooFFXVbfBarl338(2564);FooFFXVbfBarl46
FooFFXVbfBarl132;}FooFFXVbfBarl199=&FooFFXVbfBarw->
FooFFXVbfBarl53[FooFFXVbfBarl33];FooFFXVbfBarl27(2572, FooFFXVbfBarl199->FooFFXVbfBarl336==0&&FooFFXVbfBarl199-> FooFFXVbfBarl288==0);FooFFXVbfBarl1726=FooFFXVbfBarw->
FooFFXVbfBarl353;FooFFXVbfBarn(FooFFXVbfBarl1726== FooFFXVbfBarl132){FooFFXVbfBarl27(2577,FooFFXVbfBarw-> FooFFXVbfBarl226==FooFFXVbfBarl132);FooFFXVbfBarl199->
FooFFXVbfBarl336=FooFFXVbfBarl199->FooFFXVbfBarl288=
FooFFXVbfBarl132;FooFFXVbfBarw->FooFFXVbfBarl353=
FooFFXVbfBarw->FooFFXVbfBarl226=FooFFXVbfBarl33;}
FooFFXVbfBarl62{FooFFXVbfBarl27(2583,FooFFXVbfBarw-> FooFFXVbfBarl226!=FooFFXVbfBarl132);FooFFXVbfBarl1924=&
FooFFXVbfBarw->FooFFXVbfBarl53[FooFFXVbfBarl1726];
FooFFXVbfBarl199->FooFFXVbfBarl336=FooFFXVbfBarl1726;
FooFFXVbfBarl199->FooFFXVbfBarl288=FooFFXVbfBarl1924->
FooFFXVbfBarl288;FooFFXVbfBarl1924->FooFFXVbfBarl288=
FooFFXVbfBarl33;FooFFXVbfBarw->FooFFXVbfBarl353=
FooFFXVbfBarl33;}FooFFXVbfBarl46 FooFFXVbfBarl33;}
FooFFXVbfBarl50 FooFFXVbfBarl326(FooFFXVbfBarc FooFFXVbfBarl29*FooFFXVbfBars,FooFFXVbfBarl32 FooFFXVbfBarl43){FooFFXVbfBarl46 FooFFXVbfBars->
FooFFXVbfBarl627[FooFFXVbfBarl43];}FooFFXVbfBarh
FooFFXVbfBarl382(FooFFXVbfBarl29*FooFFXVbfBars, FooFFXVbfBarl32 FooFFXVbfBarl43,FooFFXVbfBarl50 FooFFXVbfBarl149){FooFFXVbfBars->FooFFXVbfBarl627[
FooFFXVbfBarl43]=FooFFXVbfBarl149;}
#define FooFFXVbfBarl1643 D_UINT16_MAX
FooFFXVbfBarl99 FooFFXVbfBari FooFFXVbfBarl2061( FooFFXVbfBarl29*FooFFXVbfBars,FooFFXVbfBaru*FooFFXVbfBarw){
FooFFXVbfBari FooFFXVbfBarl1651;FooFFXVbfBarl38
FooFFXVbfBarl33;FooFFXVbfBari FooFFXVbfBarl1338=0;
FooFFXVbfBarl91*FooFFXVbfBarl1304;FooFFXVbfBarl91*
FooFFXVbfBarc FooFFXVbfBarl258=FooFFXVbfBars->
FooFFXVbfBarl293->FooFFXVbfBarl2415.FooFFXVbfBarl273;
FooFFXVbfBarl68 FooFFXVbfBarl104;FooFFXVbfBarl50
FooFFXVbfBarl149;FooFFXVbfBarl32 FooFFXVbfBarl352,
FooFFXVbfBarl1637;FooFFXVbfBari FooFFXVbfBarl1344;
FooFFXVbfBare FooFFXVbfBarl60;FooFFXVbfBari FooFFXVbfBarl129
;FooFFXVbfBari FooFFXVbfBarl1335;FooFFXVbfBarl38
FooFFXVbfBarl1371;FooFFXVbfBarl35 FooFFXVbfBarl;
#if FFXCONF_NANDSUPPORT
FooFFXVbfBarf{FooFFXVbfBarl91 FooFFXVbfBarl273;
FooFFXVbfBarl91 FooFFXVbfBarl984;}FooFFXVbfBarl332={{
FooFFXVbfBarl368}};
#endif
FFXTRACEPRINTF((FooFFXVbfBarl66(FooFFXVbfBarl150,2, FooFFXVbfBarl283),"\x52\x65\x61\x64\x41\x6c\x6c\x41\x6c\x6c\x6f\x63\x61\x74\x69\x6f\x6e\x73\x49\x6e\x74\x6f\x43\x61\x63\x68\x65\x28\x29\x20\x52\x67\x6e\x3d\x25\x6c\x55\n", FooFFXVbfBarw->FooFFXVbfBarl59));
#if FFXCONF_NANDSUPPORT
FooFFXVbfBarn(FooFFXVbfBars->FooFFXVbfBarl161){
FooFFXVbfBarl357(FooFFXVbfBarw->FooFFXVbfBarl242,0, FooFFXVbfBarl30(FooFFXVbfBarw->FooFFXVbfBarl242));
FooFFXVbfBarw->FooFFXVbfBarl269=0;FooFFXVbfBarw->
FooFFXVbfBarl203=FALSE;}
#endif
#if FFXCONF_NORSUPPORT || FFXCONF_ISWFSUPPORT
FooFFXVbfBarn(!FooFFXVbfBars->FooFFXVbfBarl161){
FooFFXVbfBarl357(FooFFXVbfBars->FooFFXVbfBarl365,0, VBF_MAX_BLOCK_SIZE);}
#endif
FooFFXVbfBarl357(FooFFXVbfBarw->FooFFXVbfBarl208, FooFFXVbfBarl668,FooFFXVbfBarl30 FooFFXVbfBarw-> FooFFXVbfBarl208);FooFFXVbfBarl1304=FooFFXVbfBarw->
FooFFXVbfBarl208;FooFFXVbfBarl129=FooFFXVbfBars->
FooFFXVbfBarl129;FooFFXVbfBarl120(FooFFXVbfBarl60=0; FooFFXVbfBarl60<FooFFXVbfBarl210;++FooFFXVbfBarl60){
FooFFXVbfBarw->FooFFXVbfBarl53[FooFFXVbfBarl60].
FooFFXVbfBarl288=FooFFXVbfBarw->FooFFXVbfBarl53[
FooFFXVbfBarl60].FooFFXVbfBarl336=0;}FooFFXVbfBarl1637=
FooFFXVbfBarl298(FooFFXVbfBars);FooFFXVbfBarl33=0;
FooFFXVbfBarl1338=0;FooFFXVbfBarl120(FooFFXVbfBarl352=0; FooFFXVbfBarl352<FooFFXVbfBarl1637;++FooFFXVbfBarl352){
FooFFXVbfBarl149=FooFFXVbfBarl326(FooFFXVbfBars, FooFFXVbfBarl352);FooFFXVbfBarn((FooFFXVbfBarl149== FooFFXVbfBarw->FooFFXVbfBarl59)&&(FooFFXVbfBarl1338< FooFFXVbfBarl210)){FFXTRACEPRINTF((FooFFXVbfBarl66( FooFFXVbfBarl150,2,0),"\x52\x65\x61\x64\x41\x6c\x6c\x41\x6c\x6c\x6f\x63\x61\x74\x69\x6f\x6e\x73\x49\x6e\x74\x6f\x43\x61\x63\x68\x65\x28\x29\x20\x75\x6e\x69\x74\x20\x25\x55\n",FooFFXVbfBarl352));FooFFXVbfBarl=FooFFXVbfBarl853( FooFFXVbfBars,FooFFXVbfBarl352,&FooFFXVbfBarl104);
FooFFXVbfBarl1152(FooFFXVbfBarl.FooFFXVbfBarl26){
FooFFXVbfBarl143(FooFFXVbfBarq(4,20)):FooFFXVbfBarl386( FooFFXVbfBarw,FooFFXVbfBarl33);FooFFXVbfBarl1287(& FooFFXVbfBarl);FooFFXVbfBarl143 FooFFXVbfBarl44:
FooFFXVbfBarl100;FooFFXVbfBarl143(FooFFXVbfBarq(14,3)):
FooFFXVbfBarl143(FooFFXVbfBarq(14,4)):FFXPRINTF(1,("\x52\x65\x61\x64\x41\x6c\x6c\x41\x6c\x6c\x6f\x63\x61\x74\x69\x6f\x6e\x73\x49\x6e\x74\x6f\x43\x61\x63\x68\x65\x28\x29\x20\x66\x61\x69\x6c\x65\x64\x2c\x20\x53\x74\x61\x74\x75\x73\x3d\x25\x6c\x58\x20\x52\x67\x6e\x3d\x25\x6c\x55\x20\x6c\x6e\x75\x3d\x25\x55\n",FooFFXVbfBarl.FooFFXVbfBarl26, FooFFXVbfBarl149,FooFFXVbfBarl352));FooFFXVbfBarl807;
FooFFXVbfBarl1227:FFXPRINTF(1,("\x52\x65\x61\x64\x41\x6c\x6c\x41\x6c\x6c\x6f\x63\x61\x74\x69\x6f\x6e\x73\x49\x6e\x74\x6f\x43\x61\x63\x68\x65\x28\x29\x20\x66\x61\x69\x6c\x65\x64\x2c\x20\x73\x74\x61\x74\x75\x73\x3d\x25\x6c\x58\n",FooFFXVbfBarl.FooFFXVbfBarl26));FooFFXVbfBarl1338=0;
FooFFXVbfBarl73 FooFFXVbfBarl1873;}FooFFXVbfBarl372(2776, FooFFXVbfBarl104.FooFFXVbfBarl477==FooFFXVbfBarw-> FooFFXVbfBarl59);FooFFXVbfBarw->FooFFXVbfBarl53[
FooFFXVbfBarl33].FooFFXVbfBarl169=FooFFXVbfBarl104.
FooFFXVbfBarl169;FooFFXVbfBarw->FooFFXVbfBarl53[
FooFFXVbfBarl33].FooFFXVbfBarl43=FooFFXVbfBarl352;
FooFFXVbfBarw->FooFFXVbfBarl53[FooFFXVbfBarl33].
FooFFXVbfBarl202=0;FooFFXVbfBarw->FooFFXVbfBarl53[
FooFFXVbfBarl33].FooFFXVbfBarl144=FooFFXVbfBarl129;
FooFFXVbfBarw->FooFFXVbfBarl53[FooFFXVbfBarl33].
FooFFXVbfBarl147=0;FooFFXVbfBarw->FooFFXVbfBarl53[
FooFFXVbfBarl33].FooFFXVbfBarl278=FALSE;++FooFFXVbfBarl33;
++FooFFXVbfBarl1338;}}FooFFXVbfBarl2314((FooFFXVbfBarl198* )FooFFXVbfBarw ->FooFFXVbfBarl53,FooFFXVbfBarl1338);FooFFXVbfBarw->
FooFFXVbfBarl53[0].FooFFXVbfBarl288=FooFFXVbfBarl132;
FooFFXVbfBarw->FooFFXVbfBarl53[0].FooFFXVbfBarl336=
FooFFXVbfBarl132;FooFFXVbfBarw->FooFFXVbfBarl226=0;
FooFFXVbfBarw->FooFFXVbfBarl353=0;FooFFXVbfBarw->
FooFFXVbfBarl215=0;FooFFXVbfBarl120(FooFFXVbfBarl60=1; FooFFXVbfBarl60<FooFFXVbfBarl1338;FooFFXVbfBarl60++){
FooFFXVbfBarl717(FooFFXVbfBarw,(FooFFXVbfBarl38)FooFFXVbfBarl60 );}FooFFXVbfBarl33=FooFFXVbfBarw->FooFFXVbfBarl226;
FooFFXVbfBarw->FooFFXVbfBarl159=FooFFXVbfBarl132;
FooFFXVbfBarl1335=FooFFXVbfBarl1643;FooFFXVbfBarl1371=
FooFFXVbfBarl132;FooFFXVbfBarl153(FooFFXVbfBarl33!= FooFFXVbfBarl132){FooFFXVbfBarl32 FooFFXVbfBarl43=
FooFFXVbfBarw->FooFFXVbfBarl53[FooFFXVbfBarl33].
FooFFXVbfBarl43;FooFFXVbfBarl1344=0;FooFFXVbfBarl290{
FooFFXVbfBarl=FooFFXVbfBarl1314(FooFFXVbfBars, FooFFXVbfBarl43,FooFFXVbfBarl1344,FooFFXVbfBarl1750, FooFFXVbfBarl258);FooFFXVbfBarn(FooFFXVbfBarl. FooFFXVbfBarl26==(FooFFXVbfBarq(4,20))){FooFFXVbfBarl386( FooFFXVbfBarw,FooFFXVbfBarl33);FooFFXVbfBarl1287(& FooFFXVbfBarl);}FooFFXVbfBarn(FooFFXVbfBarl.FooFFXVbfBarl26 !=FooFFXVbfBarl44){FFXPRINTF(1,("\x52\x65\x61\x64\x41\x6c\x6c\x41\x6c\x6c\x6f\x63\x61\x74\x69\x6f\x6e\x73\x49\x6e\x74\x6f\x43\x61\x63\x68\x65\x28\x29\x20\x52\x65\x61\x64\x4c\x6e\x75\x41\x6c\x6c\x6f\x63\x61\x74\x69\x6f\x6e\x73\x20\x66\x6f\x72\x20\x75\x6e\x69\x74\x20\x25\x55\x20\x66\x61\x69\x6c\x65\x64\x3a\x20\x25\x6c\x58\n", FooFFXVbfBarl43,FooFFXVbfBarl.FooFFXVbfBarl26));
FooFFXVbfBarl1338=FooFFXVbfBarl33;FooFFXVbfBarl73
FooFFXVbfBarl1873;}FooFFXVbfBarl27(2864,FooFFXVbfBarl. FooFFXVbfBarv!=0);FooFFXVbfBarl120(FooFFXVbfBarl1651= FooFFXVbfBarl60=0;FooFFXVbfBarl60<FooFFXVbfBarl. FooFFXVbfBarv;FooFFXVbfBarl60++){FooFFXVbfBarn( FooFFXVbfBarl258[FooFFXVbfBarl60].FooFFXVbfBarl175== FooFFXVbfBarl545){FooFFXVbfBarn(FooFFXVbfBarl1335== FooFFXVbfBarl1643){FooFFXVbfBarl1335=FooFFXVbfBarl1344+
FooFFXVbfBarl60;FooFFXVbfBarl1371=FooFFXVbfBarl33;}
FooFFXVbfBarl807;}FooFFXVbfBarn(FooFFXVbfBarl1335!= FooFFXVbfBarl1643){FooFFXVbfBari FooFFXVbfBarl1454;
FooFFXVbfBarl38 FooFFXVbfBarl1400;FooFFXVbfBarl120( FooFFXVbfBarl1400=FooFFXVbfBarl1371;FooFFXVbfBarl1400<= FooFFXVbfBarl33;FooFFXVbfBarl1400=FooFFXVbfBarl291( FooFFXVbfBarw,FooFFXVbfBarl1400)){FooFFXVbfBarn( FooFFXVbfBarl1400==FooFFXVbfBarl1371){FooFFXVbfBarn( FooFFXVbfBarl33==FooFFXVbfBarl1371){FooFFXVbfBarl1454=
FooFFXVbfBarl1344+FooFFXVbfBarl60;}FooFFXVbfBarl62{
FooFFXVbfBarl1454=FooFFXVbfBars->FooFFXVbfBarl129;}
FooFFXVbfBarl1454-=FooFFXVbfBarl1335;}FooFFXVbfBarl62
FooFFXVbfBarn(FooFFXVbfBarl1400==FooFFXVbfBarl33){
FooFFXVbfBarl1454=FooFFXVbfBarl1344+FooFFXVbfBarl60;}
FooFFXVbfBarl62{FooFFXVbfBarl1454=FooFFXVbfBars->
FooFFXVbfBarl129;}FooFFXVbfBarw->FooFFXVbfBarl53[
FooFFXVbfBarl1400].FooFFXVbfBarl202+=FooFFXVbfBarl1454;
FooFFXVbfBarw->FooFFXVbfBarl53[FooFFXVbfBarl1400].
FooFFXVbfBarl144-=FooFFXVbfBarl1454;FooFFXVbfBarw->
FooFFXVbfBarl53[FooFFXVbfBarl1400].FooFFXVbfBarl147+=
FooFFXVbfBarl1454;}FooFFXVbfBarl1335=FooFFXVbfBarl1643;}
#if FFXCONF_NANDSUPPORT
FooFFXVbfBarn(FooFFXVbfBars->FooFFXVbfBarl161){
FooFFXVbfBarl332.FooFFXVbfBarl273=FooFFXVbfBarl258[
FooFFXVbfBarl60];FooFFXVbfBarl332.FooFFXVbfBarl273.
FooFFXVbfBarl33=FooFFXVbfBarl33;FooFFXVbfBarl332.
FooFFXVbfBarl984.FooFFXVbfBarl175=FooFFXVbfBarl368;}
#endif
FooFFXVbfBarn(FooFFXVbfBarl258[FooFFXVbfBarl60]. FooFFXVbfBarl175==FooFFXVbfBarl368){FFXPRINTF(2,("\x55\x6e\x69\x74\x20\x25\x75\x2c\x20\x62\x6c\x6f\x63\x6b\x20\x25\x75\x3a\x20\x69\x6e\x76\x61\x6c\x69\x64\x20\x61\x6c\x6c\x6f\x63\x61\x74\x69\x6f\x6e\x20\x65\x6e\x74\x72\x79\n", FooFFXVbfBarl43,FooFFXVbfBarl1344+FooFFXVbfBarl60));++
FooFFXVbfBarw->FooFFXVbfBarl53[FooFFXVbfBarl33].
FooFFXVbfBarl147;}FooFFXVbfBarl62{FooFFXVbfBarn( FooFFXVbfBarl258[FooFFXVbfBarl60].FooFFXVbfBarl175== FooFFXVbfBarl231){
#if FFXCONF_NANDSUPPORT
FooFFXVbfBarn(FooFFXVbfBars->FooFFXVbfBarl161){
FooFFXVbfBarn(FooFFXVbfBarl258[FooFFXVbfBarl60]. FooFFXVbfBarl275==FooFFXVbfBarl619){FooFFXVbfBarn( FooFFXVbfBarw->FooFFXVbfBarl159!=FooFFXVbfBarl132){
FooFFXVbfBarl332.FooFFXVbfBarl984.FooFFXVbfBarl175=
FooFFXVbfBarl231;FooFFXVbfBarl332.FooFFXVbfBarl984.
FooFFXVbfBarl33=FooFFXVbfBarw->FooFFXVbfBarl159;
FooFFXVbfBarl332.FooFFXVbfBarl984.FooFFXVbfBarl275=
FooFFXVbfBarl619;FooFFXVbfBarl332.FooFFXVbfBarl984.
FooFFXVbfBarl219=FooFFXVbfBarw->FooFFXVbfBarl375;}
FooFFXVbfBarl62{FooFFXVbfBarl332.FooFFXVbfBarl984.
FooFFXVbfBarl175=FooFFXVbfBarl368;}FooFFXVbfBarw->
FooFFXVbfBarl159=FooFFXVbfBarl33;FooFFXVbfBarw->
FooFFXVbfBarl375=FooFFXVbfBarl1344+FooFFXVbfBarl60;++
FooFFXVbfBarw->FooFFXVbfBarl53[FooFFXVbfBarl33].
FooFFXVbfBarl202;++FooFFXVbfBarw->FooFFXVbfBarl53[
FooFFXVbfBarl33].FooFFXVbfBarl147;--FooFFXVbfBarw->
FooFFXVbfBarl53[FooFFXVbfBarl33].FooFFXVbfBarl144;
FooFFXVbfBarl807;}FooFFXVbfBarl332.FooFFXVbfBarl984=
FooFFXVbfBarl1304[FooFFXVbfBarl258[FooFFXVbfBarl60].
FooFFXVbfBarl275];}
#endif
}FooFFXVbfBarl62 FooFFXVbfBarn(FooFFXVbfBarl258[ FooFFXVbfBarl60].FooFFXVbfBarl175==FooFFXVbfBarl415){++
FooFFXVbfBarw->FooFFXVbfBarl53[FooFFXVbfBarl33].
FooFFXVbfBarl147;}FooFFXVbfBarl62{FooFFXVbfBarl338(3052);
FooFFXVbfBarl1338=FooFFXVbfBarl33;FooFFXVbfBarl73
FooFFXVbfBarl1873;}FooFFXVbfBarl258[FooFFXVbfBarl60].
FooFFXVbfBarl33=FooFFXVbfBarl33;FooFFXVbfBarn( FooFFXVbfBarl258[FooFFXVbfBarl60].FooFFXVbfBarl275< FooFFXVbfBarl93){FooFFXVbfBare FooFFXVbfBarl1579=
FooFFXVbfBarl258[FooFFXVbfBarl60].FooFFXVbfBarl275;
FooFFXVbfBarn(FooFFXVbfBarl1304[FooFFXVbfBarl1579]. FooFFXVbfBarl175==(FooFFXVbfBarl69)FooFFXVbfBarl231){++
FooFFXVbfBarw->FooFFXVbfBarl53[FooFFXVbfBarl1304[
FooFFXVbfBarl1579].FooFFXVbfBarl33].FooFFXVbfBarl147;
FooFFXVbfBarl1854(FooFFXVbfBarw,(FooFFXVbfBarl38)FooFFXVbfBarl1304 [FooFFXVbfBarl1579].FooFFXVbfBarl33,FooFFXVbfBarl1304[ FooFFXVbfBarl1579].FooFFXVbfBarl219,TRUE);}
FooFFXVbfBarl1304[FooFFXVbfBarl1579]=FooFFXVbfBarl258[
FooFFXVbfBarl60];}FooFFXVbfBarl62{FFXPRINTF(1,("\x62\x61\x64\x20\x63\x6c\x69\x65\x6e\x74\x20\x62\x6c\x6f\x63\x6b\x20\x6f\x66\x66\x73\x65\x74\x20\x30\x78\x25\x30\x34\x78\x20\x69\x6e\x20\x75\x6e\x69\x74\x20\x25\x75\x20\x61\x74\x20\x69\x6e\x64\x65\x78\x20\x25\x75\n",FooFFXVbfBarl258[ FooFFXVbfBarl60].FooFFXVbfBarl275,FooFFXVbfBarl43, FooFFXVbfBarl1344+FooFFXVbfBarl60));++FooFFXVbfBarw->
FooFFXVbfBarl53[FooFFXVbfBarl33].FooFFXVbfBarl147;}}++
FooFFXVbfBarl1651;FooFFXVbfBarl357(&FooFFXVbfBarl258[ FooFFXVbfBarl60],FooFFXVbfBarl668,FooFFXVbfBarl30( FooFFXVbfBarl258[FooFFXVbfBarl60]));}FooFFXVbfBarl27(3128, FooFFXVbfBarl1651<=FooFFXVbfBarl129);FooFFXVbfBarl27(3129, FooFFXVbfBarl.FooFFXVbfBarv<=D_UINT16_MAX);
FooFFXVbfBarl1344+=(FooFFXVbfBari)FooFFXVbfBarl.
FooFFXVbfBarv;FooFFXVbfBarw->FooFFXVbfBarl53[
FooFFXVbfBarl33].FooFFXVbfBarl202+=FooFFXVbfBarl1651;
FooFFXVbfBarw->FooFFXVbfBarl53[FooFFXVbfBarl33].
FooFFXVbfBarl144-=FooFFXVbfBarl1651;FooFFXVbfBarl27(3137, FooFFXVbfBarw->FooFFXVbfBarl53[FooFFXVbfBarl33]. FooFFXVbfBarl144<=FooFFXVbfBarl129);FooFFXVbfBarl27(3138, FooFFXVbfBarw->FooFFXVbfBarl53[FooFFXVbfBarl33]. FooFFXVbfBarl202<=FooFFXVbfBarl129);FooFFXVbfBarl27(3139, FooFFXVbfBarw->FooFFXVbfBarl53[FooFFXVbfBarl33]. FooFFXVbfBarl202>=FooFFXVbfBarw->FooFFXVbfBarl53[ FooFFXVbfBarl33].FooFFXVbfBarl147);FooFFXVbfBarl27(3140, FooFFXVbfBarw->FooFFXVbfBarl53[FooFFXVbfBarl33]. FooFFXVbfBarl202+FooFFXVbfBarw->FooFFXVbfBarl53[ FooFFXVbfBarl33].FooFFXVbfBarl144==FooFFXVbfBarl129);}
FooFFXVbfBarl153(FooFFXVbfBarl1344<FooFFXVbfBarl129);
FooFFXVbfBarl33=FooFFXVbfBarl291(FooFFXVbfBarw, FooFFXVbfBarl33);}
#if FFXCONF_NANDSUPPORT
FooFFXVbfBarn(FooFFXVbfBars->FooFFXVbfBarl161){
FooFFXVbfBarn(!DCLBITGET(FooFFXVbfBars->FooFFXVbfBarl366, FooFFXVbfBarw->FooFFXVbfBarl59)){FooFFXVbfBarl35
FooFFXVbfBarl1759;FFXPRINTF(2,("\x43\x68\x65\x63\x6b\x69\x6e\x67\x20\x72\x65\x67\x69\x6f\x6e\x20\x25\x55\x20\x6f\x66\x20\x25\x55\n",FooFFXVbfBarw->FooFFXVbfBarl59, FooFFXVbfBars->FooFFXVbfBarl211));DCLBITSET(FooFFXVbfBars-> FooFFXVbfBarl366,FooFFXVbfBarw->FooFFXVbfBarl59);
FooFFXVbfBarn(FooFFXVbfBarl1335!=FooFFXVbfBarl1643){
FFXSTATUS FooFFXVbfBarl26;FooFFXVbfBarl32 FooFFXVbfBarl43=
FooFFXVbfBarw->FooFFXVbfBarl53[FooFFXVbfBarl1371].
FooFFXVbfBarl43;FooFFXVbfBarl26=FooFFXVbfBarl1412( FooFFXVbfBars,FooFFXVbfBarl43,FooFFXVbfBarl1335);
FooFFXVbfBarn(FooFFXVbfBarl26!=FooFFXVbfBarl44){
FooFFXVbfBarl27(3186,FooFFXVbfBarl26==(FooFFXVbfBarq(14,2)));
FFXPRINTF(2,("\x55\x6e\x69\x74\x20\x25\x75\x2c\x20\x62\x6c\x6f\x63\x6b\x20\x25\x75\x3a\x20\x66\x6f\x75\x6e\x64\x20\x70\x72\x6f\x67\x72\x61\x6d\x6d\x65\x64\x20\x62\x69\x74\x73\x20\x69\x6e\x20\x66\x72\x65\x65\x20\x70\x61\x67\x65\n",FooFFXVbfBarl43,FooFFXVbfBarl1335));FooFFXVbfBarl1759=
FooFFXVbfBarl1305(FooFFXVbfBars,FooFFXVbfBarl43, FooFFXVbfBarl1335,1);FooFFXVbfBarn(FooFFXVbfBarl1759. FooFFXVbfBarl26!=FooFFXVbfBarl44){DCLBITCLEAR(FooFFXVbfBars ->FooFFXVbfBarl366,FooFFXVbfBarw->FooFFXVbfBarl59);}
FooFFXVbfBarl332.FooFFXVbfBarl273.FooFFXVbfBarl175=
FooFFXVbfBarl368;++FooFFXVbfBarw->FooFFXVbfBarl53[
FooFFXVbfBarl1371].FooFFXVbfBarl202;++FooFFXVbfBarw->
FooFFXVbfBarl53[FooFFXVbfBarl1371].FooFFXVbfBarl147;--
FooFFXVbfBarw->FooFFXVbfBarl53[FooFFXVbfBarl1371].
FooFFXVbfBarl144;}}FooFFXVbfBarn(FooFFXVbfBarl332. FooFFXVbfBarl273.FooFFXVbfBarl175==FooFFXVbfBarl231){
FooFFXVbfBarl32 FooFFXVbfBarl43=FooFFXVbfBarw->
FooFFXVbfBarl53[FooFFXVbfBarl332.FooFFXVbfBarl273.
FooFFXVbfBarl33].FooFFXVbfBarl43;FooFFXVbfBarl1152( FooFFXVbfBarl1462(FooFFXVbfBars,FooFFXVbfBarl43, FooFFXVbfBarl332.FooFFXVbfBarl273.FooFFXVbfBarl219)){
FooFFXVbfBarl143(FooFFXVbfBarq(4,20)):FooFFXVbfBarl386( FooFFXVbfBarw,FooFFXVbfBarl332.FooFFXVbfBarl273. FooFFXVbfBarl33);FooFFXVbfBarl143 FooFFXVbfBarl44:
FooFFXVbfBarl100;FooFFXVbfBarl143(FooFFXVbfBarq(4,11)):
FFXPRINTF(2,("\x55\x6e\x69\x74\x20\x25\x75\x2c\x20\x62\x6c\x6f\x63\x6b\x20\x25\x75\x3a\x20\x75\x6e\x63\x6f\x72\x72\x65\x63\x74\x61\x62\x6c\x65\x20\x45\x43\x43\x20\x65\x72\x72\x6f\x72\n",FooFFXVbfBarl43,FooFFXVbfBarl332. FooFFXVbfBarl273.FooFFXVbfBarl219));FooFFXVbfBarl1759=
FooFFXVbfBarl1305(FooFFXVbfBars,FooFFXVbfBarl43, FooFFXVbfBarl332.FooFFXVbfBarl273.FooFFXVbfBarl219,1);
FooFFXVbfBarn(FooFFXVbfBarl1759.FooFFXVbfBarl26!= FooFFXVbfBarl44){FooFFXVbfBarl112(3261);FooFFXVbfBars->
FooFFXVbfBarl327=TRUE;}FooFFXVbfBarn(FooFFXVbfBarl332. FooFFXVbfBarl273.FooFFXVbfBarl275==FooFFXVbfBarl619){
FFXPRINTF(2,("\x52\x65\x76\x65\x72\x74\x69\x6e\x67\x20\x74\x6f\x20\x6d\x65\x74\x61\x64\x61\x74\x61\x20\x69\x6e\x20\x75\x6e\x69\x74\x20\x25\x75\x2c\x20\x62\x6c\x6f\x63\x6b\x20\x25\x75\n",FooFFXVbfBarw->FooFFXVbfBarl53[ FooFFXVbfBarl332.FooFFXVbfBarl984.FooFFXVbfBarl33]. FooFFXVbfBarl43,FooFFXVbfBarl332.FooFFXVbfBarl984. FooFFXVbfBarl219));FooFFXVbfBarn(FooFFXVbfBarl332. FooFFXVbfBarl984.FooFFXVbfBarl175==FooFFXVbfBarl231){
FooFFXVbfBarw->FooFFXVbfBarl159=(FooFFXVbfBarl38)( FooFFXVbfBarl332.FooFFXVbfBarl984.FooFFXVbfBarl33);
FooFFXVbfBarw->FooFFXVbfBarl375=(FooFFXVbfBari)( FooFFXVbfBarl332.FooFFXVbfBarl984.FooFFXVbfBarl219);}
FooFFXVbfBarl62{FooFFXVbfBarw->FooFFXVbfBarl159=
FooFFXVbfBarl132;}}FooFFXVbfBarl62{FooFFXVbfBarl1304[
FooFFXVbfBarl332.FooFFXVbfBarl273.FooFFXVbfBarl275]=
FooFFXVbfBarl332.FooFFXVbfBarl984;FooFFXVbfBarn( FooFFXVbfBarl332.FooFFXVbfBarl984.FooFFXVbfBarl175== FooFFXVbfBarl231){FFXPRINTF(2,("\x52\x65\x76\x65\x72\x74\x69\x6e\x67\x20\x74\x6f\x20\x76\x61\x6c\x69\x64\x20\x61\x6c\x6c\x6f\x63\x61\x74\x69\x6f\x6e\x20\x69\x6e\x20\x75\x6e\x69\x74\x20\x25\x75\x2c\x20\x62\x6c\x6f\x63\x6b\x20\x25\x75\n",FooFFXVbfBarw->FooFFXVbfBarl53[ FooFFXVbfBarl332.FooFFXVbfBarl984.FooFFXVbfBarl33]. FooFFXVbfBarl43,FooFFXVbfBarl332.FooFFXVbfBarl984. FooFFXVbfBarl219));--FooFFXVbfBarw->FooFFXVbfBarl53[
FooFFXVbfBarl332.FooFFXVbfBarl984.FooFFXVbfBarl33].
FooFFXVbfBarl147;}FooFFXVbfBarl62{FFXPRINTF(2,("\x52\x65\x76\x65\x72\x74\x69\x6e\x67\x20\x74\x6f\x20\x64\x69\x73\x63\x61\x72\x64\x65\x64\x20\x61\x6c\x6c\x6f\x63\x61\x74\x69\x6f\x6e\n"));}++FooFFXVbfBarw->FooFFXVbfBarl53[
FooFFXVbfBarl332.FooFFXVbfBarl273.FooFFXVbfBarl33].
FooFFXVbfBarl147;}FooFFXVbfBarl100;FooFFXVbfBarl1227:
FooFFXVbfBarl112(3338);FooFFXVbfBars->FooFFXVbfBarl327=TRUE
;FooFFXVbfBarl100;}}}}
#endif
#if D_DEBUG
FooFFXVbfBarl120(FooFFXVbfBarl60=0;FooFFXVbfBarl60< FooFFXVbfBarl93;FooFFXVbfBarl60++){FooFFXVbfBarn( FooFFXVbfBarl1304[FooFFXVbfBarl60].FooFFXVbfBarl175== FooFFXVbfBarl231&&FooFFXVbfBarl1304[FooFFXVbfBarl60]. FooFFXVbfBarl275!=FooFFXVbfBarl60){FFXPRINTF(1,("\x43\x6c\x69\x65\x6e\x74\x20\x61\x64\x64\x72\x65\x73\x73\x20\x66\x6f\x72\x20\x65\x6e\x74\x72\x79\x20\x23\x25\x64\x20\x3d\x20\x25\x64\n",FooFFXVbfBarl60,FooFFXVbfBarl1304[ FooFFXVbfBarl60].FooFFXVbfBarl275));FooFFXVbfBarl112(3363);
}}
#endif
FooFFXVbfBarl1873:FFXTRACEPRINTF((FooFFXVbfBarl66( FooFFXVbfBarl150,2,FooFFXVbfBarl267),"\x52\x65\x61\x64\x41\x6c\x6c\x41\x6c\x6c\x6f\x63\x61\x74\x69\x6f\x6e\x73\x49\x6e\x74\x6f\x43\x61\x63\x68\x65\x28\x29\x20\x72\x65\x74\x75\x72\x6e\x69\x6e\x67\x20\x25\x55\n",FooFFXVbfBarl1338));
FooFFXVbfBarl46 FooFFXVbfBarl1338;}FooFFXVbfBarj
FooFFXVbfBarl527(FooFFXVbfBarc FooFFXVbfBaru*FooFFXVbfBarw, FooFFXVbfBarl32 FooFFXVbfBarl43,FooFFXVbfBarl38* FooFFXVbfBarl890){FooFFXVbfBarl38 FooFFXVbfBarl320;
FooFFXVbfBarl120(FooFFXVbfBarl320=FooFFXVbfBarw-> FooFFXVbfBarl226;FooFFXVbfBarl320!=FooFFXVbfBarl132; FooFFXVbfBarl320=FooFFXVbfBarl291(FooFFXVbfBarw, FooFFXVbfBarl320)){FooFFXVbfBarn(FooFFXVbfBarw-> FooFFXVbfBarl53[FooFFXVbfBarl320].FooFFXVbfBarl43== FooFFXVbfBarl43){FooFFXVbfBarl100;}}FooFFXVbfBarn( FooFFXVbfBarl320<FooFFXVbfBarl210){ *FooFFXVbfBarl890=
FooFFXVbfBarl320;FooFFXVbfBarl46 TRUE;}FooFFXVbfBarl46 FALSE
;}FooFFXVbfBarl99 FooFFXVbfBarh FooFFXVbfBarl1854( FooFFXVbfBaru*FooFFXVbfBarw,FooFFXVbfBarl38 FooFFXVbfBarl33 ,FooFFXVbfBare FooFFXVbfBarl321,FooFFXVbfBarj FooFFXVbfBarl1841){FooFFXVbfBarn(FooFFXVbfBarw-> FooFFXVbfBarl159!=FooFFXVbfBarl132)FooFFXVbfBarl46;
#if FFXCONF_NANDSUPPORT
FooFFXVbfBarn(FooFFXVbfBarw->FooFFXVbfBarl183-> FooFFXVbfBarl161){FooFFXVbfBarn(FooFFXVbfBarl1841){
FooFFXVbfBarl736(FooFFXVbfBarw->FooFFXVbfBarl242, FooFFXVbfBarl33*FooFFXVbfBarw->FooFFXVbfBarl183-> FooFFXVbfBarl129+FooFFXVbfBarl321);}FooFFXVbfBarl62{
FooFFXVbfBarl549(FooFFXVbfBarw->FooFFXVbfBarl242, FooFFXVbfBarl33*FooFFXVbfBarw->FooFFXVbfBarl183-> FooFFXVbfBarl129+FooFFXVbfBarl321);}}
#endif
#if FFXCONF_NORSUPPORT || FFXCONF_ISWFSUPPORT
FooFFXVbfBarn(!FooFFXVbfBarw->FooFFXVbfBarl183-> FooFFXVbfBarl161){FooFFXVbfBarn(FooFFXVbfBarl1841){
DCLBITSET(FooFFXVbfBarw->FooFFXVbfBarl183->FooFFXVbfBarl365 ,FooFFXVbfBarl33*FooFFXVbfBarw->FooFFXVbfBarl183-> FooFFXVbfBarl129+FooFFXVbfBarl321);}FooFFXVbfBarl62{
DCLBITCLEAR(FooFFXVbfBarw->FooFFXVbfBarl183-> FooFFXVbfBarl365,FooFFXVbfBarl33*FooFFXVbfBarw-> FooFFXVbfBarl183->FooFFXVbfBarl129+FooFFXVbfBarl321);}}
#endif
FooFFXVbfBarl46;}
#if FFXCONF_STATS_VBFREGION
FooFFXVbfBarj FooFFXVbfBarl921(FooFFXVbfBarx FooFFXVbfBarp, FooFFXVbfBarl160*FooFFXVbfBarl72,FooFFXVbfBarj FooFFXVbfBarl384){FooFFXVbfBarl102 FooFFXVbfBark= *
FooFFXVbfBarp;FooFFXVbfBarl225*FooFFXVbfBarl92;
FooFFXVbfBarl27(3531,FooFFXVbfBarp);FooFFXVbfBarl27(3532, FooFFXVbfBarl72);FooFFXVbfBarl92=FooFFXVbfBark->
FooFFXVbfBarl293;FooFFXVbfBarl72->FooFFXVbfBarl603=
FooFFXVbfBarl92->FooFFXVbfBarl1026;FooFFXVbfBarl72->
FooFFXVbfBarl601=(FooFFXVbfBare)FooFFXVbfBark->
FooFFXVbfBarl211;FooFFXVbfBarl72->FooFFXVbfBarl403=
FooFFXVbfBarl92->FooFFXVbfBarl403;FooFFXVbfBarl72->
FooFFXVbfBarl207=FooFFXVbfBarl92->FooFFXVbfBarl207;
FooFFXVbfBarl72->FooFFXVbfBarl390=FooFFXVbfBarl92->
FooFFXVbfBarl390;FooFFXVbfBarl72->FooFFXVbfBarl406=
FooFFXVbfBarl92->FooFFXVbfBarl406;FooFFXVbfBarn( FooFFXVbfBarl72->FooFFXVbfBarl374){FooFFXVbfBarl27(3545, FooFFXVbfBarl72->FooFFXVbfBarl227);FooFFXVbfBarl357( FooFFXVbfBarl72->FooFFXVbfBarl227,0,FooFFXVbfBarl30( * FooFFXVbfBarl72->FooFFXVbfBarl227) *FooFFXVbfBarl72-> FooFFXVbfBarl374);FooFFXVbfBarn(FooFFXVbfBark-> FooFFXVbfBarl211>FooFFXVbfBarl72->FooFFXVbfBarl449){
FooFFXVbfBarl72->FooFFXVbfBarl374=DCLMIN3(FooFFXVbfBarl72-> FooFFXVbfBarl374,FooFFXVbfBarl92->FooFFXVbfBarl1026, FooFFXVbfBark->FooFFXVbfBarl211-FooFFXVbfBarl72-> FooFFXVbfBarl449);FooFFXVbfBarl1278(FooFFXVbfBarl72-> FooFFXVbfBarl227,&FooFFXVbfBarl92->FooFFXVbfBarl227[ FooFFXVbfBarl72->FooFFXVbfBarl449],FooFFXVbfBarl30( * FooFFXVbfBarl72->FooFFXVbfBarl227) *FooFFXVbfBarl72-> FooFFXVbfBarl374);}FooFFXVbfBarl62{FooFFXVbfBarl72->
FooFFXVbfBarl374=0;}}FooFFXVbfBarn(FooFFXVbfBarl384)FooFFXVbfBarl46
FooFFXVbfBarl774(FooFFXVbfBarp);FooFFXVbfBarl62
FooFFXVbfBarl46 TRUE;}FooFFXVbfBarj FooFFXVbfBarl774( FooFFXVbfBarx FooFFXVbfBarp){FooFFXVbfBarl102 FooFFXVbfBark
= *FooFFXVbfBarp;FooFFXVbfBarl225*FooFFXVbfBarl92;
FooFFXVbfBarl27(3591,FooFFXVbfBarp);FooFFXVbfBarl92=
FooFFXVbfBark->FooFFXVbfBarl293;FFXPRINTF(1,("\x52\x65\x73\x65\x74\x74\x69\x6e\x67\x20\x56\x42\x46\x20\x72\x65\x67\x69\x6f\x6e\x20\x73\x74\x61\x74\x69\x73\x74\x69\x63\x73\n"));FooFFXVbfBarl92->FooFFXVbfBarl403=0;FooFFXVbfBarl92
->FooFFXVbfBarl390=0;FooFFXVbfBarl92->FooFFXVbfBarl207=0;
FooFFXVbfBarl92->FooFFXVbfBarl406=0;FooFFXVbfBarn( FooFFXVbfBarl92->FooFFXVbfBarl227)FooFFXVbfBarl357( FooFFXVbfBarl92->FooFFXVbfBarl227,0,FooFFXVbfBarl30( * FooFFXVbfBarl92->FooFFXVbfBarl227) *FooFFXVbfBarl92-> FooFFXVbfBarl1026);FooFFXVbfBarl46 TRUE;}
#endif
#endif
