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
/* COBF by BB -- 'vbfdisp.c' obfuscated at Tue Apr 24 23:21:13 2012
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
#ifndef FooFFXVbfBarl2565
#define FooFFXVbfBarl2565
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
#define FooFFXVbfBarl3402 0xCCDD55AAUL
#define FooFFXVbfBarl2917 0x55AACCDDUL
FFXREQHANDLE FooFFXVbfBarl2965(FooFFXVbfBare FooFFXVbfBarl64 );FFXSTATUS FooFFXVbfBarl3204(FFXREQHANDLE FooFFXVbfBarl90, FooFFXVbfBart*FooFFXVbfBarl56);FFXSTATUS FooFFXVbfBarl3224( FFXREQHANDLE FooFFXVbfBarl90);FFXSTATUS FooFFXVbfBarl3078( FooFFXVbfBart*FooFFXVbfBarl56);FFXSTATUS FooFFXVbfBarl3384( FooFFXVbfBart*FooFFXVbfBarl56);FFXSTATUS FooFFXVbfBarl2797( FooFFXVbfBart*FooFFXVbfBarl56);FFXSTATUS FooFFXVbfBarl3414( FooFFXVbfBart*FooFFXVbfBarl56);
#if FooFFXVbfBarl74 && FFXCONF_VBFSUPPORT
FFXSTATUS FooFFXVbfBarl2561(FooFFXVbfBart*FooFFXVbfBarl56);
#endif
#endif
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
#ifndef FooFFXVbfBarl2590
#define FooFFXVbfBarl2590
FooFFXVbfBarb FooFFXVbfBarf{FooFFXVbfBart FooFFXVbfBarl279;
FooFFXVbfBarl45 FooFFXVbfBarm;FooFFXVbfBarx FooFFXVbfBarp;}
FooFFXVbfBarl2340;FooFFXVbfBarb FooFFXVbfBarf{FooFFXVbfBart
FooFFXVbfBarl279;FooFFXVbfBarx FooFFXVbfBarp;FooFFXVbfBara
FooFFXVbfBarl65;FFXSTATUS FooFFXVbfBarl26;}
FooFFXVbfBarl2138;FooFFXVbfBarb FooFFXVbfBarf{FooFFXVbfBart
FooFFXVbfBarl279;FooFFXVbfBare FooFFXVbfBarl64;
FooFFXVbfBarx FooFFXVbfBarp;}FooFFXVbfBarl2145;
FooFFXVbfBarb FooFFXVbfBarf{FooFFXVbfBart FooFFXVbfBarl279;
FooFFXVbfBarx FooFFXVbfBarp;FooFFXVbfBarl251*
FooFFXVbfBarl543;FFXSTATUS FooFFXVbfBarl26;}
FooFFXVbfBarl2363;FooFFXVbfBarb FooFFXVbfBarf{FooFFXVbfBart
FooFFXVbfBarl279;FooFFXVbfBarx FooFFXVbfBarp;
FooFFXVbfBarl266*FooFFXVbfBarl224;FFXSTATUS FooFFXVbfBarl26
;}FooFFXVbfBarl2454;FooFFXVbfBarb FooFFXVbfBarf{
FooFFXVbfBart FooFFXVbfBarl279;FooFFXVbfBarx FooFFXVbfBarp;
FooFFXVbfBara FooFFXVbfBarl380;FooFFXVbfBarl268*
FooFFXVbfBarl292;FFXSTATUS FooFFXVbfBarl26;}
FooFFXVbfBarl2270;FooFFXVbfBarb FooFFXVbfBarf{FooFFXVbfBart
FooFFXVbfBarl279;FooFFXVbfBarx FooFFXVbfBarp;
FooFFXVbfBarl270*FooFFXVbfBarl550;FFXSTATUS FooFFXVbfBarl26
;}FooFFXVbfBarl2389;FooFFXVbfBarb FooFFXVbfBarf{
FooFFXVbfBart FooFFXVbfBarl279;FooFFXVbfBarx FooFFXVbfBarp;
FooFFXVbfBara FooFFXVbfBary;FooFFXVbfBara FooFFXVbfBarl31;
FooFFXVbfBaro*FooFFXVbfBarz;FooFFXVbfBarl35 FooFFXVbfBarl;}
FooFFXVbfBarl2430;FooFFXVbfBarb FooFFXVbfBarf{FooFFXVbfBart
FooFFXVbfBarl279;FooFFXVbfBarx FooFFXVbfBarp;FooFFXVbfBara
FooFFXVbfBary;FooFFXVbfBara FooFFXVbfBarl31;FooFFXVbfBarc
FooFFXVbfBaro*FooFFXVbfBarz;FooFFXVbfBarl35 FooFFXVbfBarl;}
FooFFXVbfBarl2282;FooFFXVbfBarb FooFFXVbfBarf{FooFFXVbfBart
FooFFXVbfBarl279;FooFFXVbfBarx FooFFXVbfBarp;FooFFXVbfBara
FooFFXVbfBary;FooFFXVbfBara FooFFXVbfBarl31;FooFFXVbfBarl35
FooFFXVbfBarl;}FooFFXVbfBarl2318;FooFFXVbfBarb FooFFXVbfBarf
{FooFFXVbfBart FooFFXVbfBarl279;FooFFXVbfBarx FooFFXVbfBarp
;FooFFXVbfBarl35 FooFFXVbfBarl;FooFFXVbfBara FooFFXVbfBarl65
;}FooFFXVbfBarl1976;FooFFXVbfBarb FooFFXVbfBarf{
FooFFXVbfBart FooFFXVbfBarl279;FooFFXVbfBarx FooFFXVbfBarp;
FooFFXVbfBara FooFFXVbfBarv;}FooFFXVbfBarl2226;
FooFFXVbfBarb FooFFXVbfBarf{FooFFXVbfBart FooFFXVbfBarl279;
FooFFXVbfBarx FooFFXVbfBarp;FooFFXVbfBara FooFFXVbfBarv;}
FooFFXVbfBarl2004;FooFFXVbfBarb FooFFXVbfBarf{FooFFXVbfBart
FooFFXVbfBarl279;FooFFXVbfBarl45 FooFFXVbfBarm;
FooFFXVbfBara FooFFXVbfBarl322;FooFFXVbfBara
FooFFXVbfBarl1311;FFXSTATUS FooFFXVbfBarl26;FooFFXVbfBarj
FooFFXVbfBarl277;}FooFFXVbfBarl2199;FooFFXVbfBarb
FooFFXVbfBarf{FooFFXVbfBart FooFFXVbfBarl279;FooFFXVbfBarx
FooFFXVbfBarp;FooFFXVbfBara FooFFXVbfBarl2687;}
FooFFXVbfBarl2378;FooFFXVbfBarb FooFFXVbfBarf{FooFFXVbfBart
FooFFXVbfBarl279;FooFFXVbfBarx FooFFXVbfBarp;FooFFXVbfBara
FooFFXVbfBarl340;FooFFXVbfBare FooFFXVbfBarl222;
FooFFXVbfBarl388 FooFFXVbfBarl2533;}FooFFXVbfBarl2082;
FooFFXVbfBarb FooFFXVbfBarf{FooFFXVbfBart FooFFXVbfBarl279;
FooFFXVbfBarx FooFFXVbfBarp;FFXSTATUS FooFFXVbfBarl26;}
FooFFXVbfBarl2316;
#endif
FFXSTATUS FooFFXVbfBarl2561(FooFFXVbfBart*FooFFXVbfBarl571){
FooFFXVbfBarl1014 FooFFXVbfBarl1565;FFXSTATUS
FooFFXVbfBarl26=FooFFXVbfBarl44;FooFFXVbfBarn(! FooFFXVbfBarl571){FooFFXVbfBarl112(165);FooFFXVbfBarl46( FooFFXVbfBarq(5,10));}FooFFXVbfBarl370("\x46\x66\x78\x56\x62\x66\x49\x6f\x63\x74\x6c\x44\x69\x73\x70\x61\x74\x63\x68",0,0);FooFFXVbfBarl1565=FooFFXVbfBarl571->ioFunc;
FFXTRACEPRINTF((FooFFXVbfBarl66(FooFFXVbfBarl162,3,0),"\x46\x66\x78\x56\x62\x66\x49\x6f\x63\x74\x6c\x44\x69\x73\x70\x61\x74\x63\x68\x28\x29\x20\x46\x75\x6e\x63\x74\x69\x6f\x6e\x20\x30\x78\x25\x78\n",FooFFXVbfBarl1565));
FooFFXVbfBarl1152(FooFFXVbfBarl1565){FooFFXVbfBarl143
FooFFXVbfBarl1037:{FooFFXVbfBarl2340*FooFFXVbfBarl56=( FooFFXVbfBarl2340* )FooFFXVbfBarl571;FooFFXVbfBarn( FooFFXVbfBarl56->FooFFXVbfBarl279.ulReqLen!=FooFFXVbfBarl30 ( *FooFFXVbfBarl56))FooFFXVbfBarl73 FooFFXVbfBarl935;
FFXTRACEPRINTF((FooFFXVbfBarl66(FooFFXVbfBarl162,2,0),"\x44\x69\x73\x70\x61\x74\x63\x68\x20\x46\x66\x78\x56\x62\x66\x43\x72\x65\x61\x74\x65\x28\x29\x20\x68\x46\x4d\x4c\x3d\x25\x50\n",FooFFXVbfBarl56->FooFFXVbfBarm));
FooFFXVbfBarl56->FooFFXVbfBarp=FooFFXVbfBarl523( FooFFXVbfBarl56->FooFFXVbfBarm);FooFFXVbfBarl100;}
FooFFXVbfBarl143 FooFFXVbfBarl1036:{FooFFXVbfBarl2138*
FooFFXVbfBarl56=(FooFFXVbfBarl2138* )FooFFXVbfBarl571;
FooFFXVbfBarn(FooFFXVbfBarl56->FooFFXVbfBarl279.ulReqLen!= FooFFXVbfBarl30( *FooFFXVbfBarl56))FooFFXVbfBarl73
FooFFXVbfBarl935;FFXTRACEPRINTF((FooFFXVbfBarl66( FooFFXVbfBarl162,2,0),"\x44\x69\x73\x70\x61\x74\x63\x68\x20\x46\x66\x78\x56\x62\x66\x44\x65\x73\x74\x72\x6f\x79\x28\x29\x20\x68\x56\x42\x46\x3d\x25\x50\x20\x46\x6c\x61\x67\x73\x3d\x25\x6c\x55\n",FooFFXVbfBarl56->FooFFXVbfBarp, FooFFXVbfBarl56->FooFFXVbfBarl65));FooFFXVbfBarl56->
FooFFXVbfBarl26=FooFFXVbfBarl516(FooFFXVbfBarl56-> FooFFXVbfBarp,FooFFXVbfBarl56->FooFFXVbfBarl65);
FooFFXVbfBarl100;}FooFFXVbfBarl143 FooFFXVbfBarl1015:{
FooFFXVbfBarl2145*FooFFXVbfBarl56=(FooFFXVbfBarl2145* )FooFFXVbfBarl571
;FooFFXVbfBarn(FooFFXVbfBarl56->FooFFXVbfBarl279.ulReqLen!= FooFFXVbfBarl30( *FooFFXVbfBarl56))FooFFXVbfBarl73
FooFFXVbfBarl935;FFXTRACEPRINTF((FooFFXVbfBarl66( FooFFXVbfBarl162,2,0),"\x44\x69\x73\x70\x61\x74\x63\x68\x20\x46\x66\x78\x56\x62\x66\x48\x61\x6e\x64\x6c\x65\x28\x29\x20\x44\x69\x73\x6b\x3d\x25\x75\n",FooFFXVbfBarl56-> FooFFXVbfBarl64));FooFFXVbfBarl56->FooFFXVbfBarp=
FooFFXVbfBarl531(FooFFXVbfBarl56->FooFFXVbfBarl64);
FooFFXVbfBarl100;}FooFFXVbfBarl143 FooFFXVbfBarl1029:{
FooFFXVbfBarl2363*FooFFXVbfBarl56=(FooFFXVbfBarl2363* )FooFFXVbfBarl571
;FooFFXVbfBarn(FooFFXVbfBarl56->FooFFXVbfBarl279.ulReqLen!= FooFFXVbfBarl30( *FooFFXVbfBarl56))FooFFXVbfBarl73
FooFFXVbfBarl935;FFXTRACEPRINTF((FooFFXVbfBarl66( FooFFXVbfBarl162,2,0),"\x44\x69\x73\x70\x61\x74\x63\x68\x20\x46\x66\x78\x56\x62\x66\x44\x69\x73\x6b\x49\x6e\x66\x6f\x28\x29\x20\x68\x56\x42\x46\x3d\x25\x50\x20\x70\x49\x6e\x66\x6f\x3d\x25\x50\n",FooFFXVbfBarl56->FooFFXVbfBarp, FooFFXVbfBarl56->FooFFXVbfBarl543));FooFFXVbfBarl56->
FooFFXVbfBarl26=FooFFXVbfBarl421(FooFFXVbfBarl56-> FooFFXVbfBarp,FooFFXVbfBarl56->FooFFXVbfBarl543);
FooFFXVbfBarl100;}FooFFXVbfBarl143 FooFFXVbfBarl1041:{
FooFFXVbfBarl2454*FooFFXVbfBarl56=(FooFFXVbfBarl2454* )FooFFXVbfBarl571
;FooFFXVbfBarn(FooFFXVbfBarl56->FooFFXVbfBarl279.ulReqLen!= FooFFXVbfBarl30( *FooFFXVbfBarl56))FooFFXVbfBarl73
FooFFXVbfBarl935;FFXTRACEPRINTF((FooFFXVbfBarl66( FooFFXVbfBarl162,2,0),"\x44\x69\x73\x70\x61\x74\x63\x68\x20\x46\x66\x78\x56\x62\x66\x44\x69\x73\x6b\x4d\x65\x74\x72\x69\x63\x73\x28\x29\x20\x68\x56\x42\x46\x3d\x25\x50\x20\x70\x44\x69\x73\x6b\x4d\x65\x74\x73\x3d\x25\x50\n", FooFFXVbfBarl56->FooFFXVbfBarp,FooFFXVbfBarl56-> FooFFXVbfBarl224));FooFFXVbfBarl56->FooFFXVbfBarl26=
FooFFXVbfBarl789(FooFFXVbfBarl56->FooFFXVbfBarp, FooFFXVbfBarl56->FooFFXVbfBarl224);FooFFXVbfBarl100;}
FooFFXVbfBarl143 FooFFXVbfBarl1018:{FooFFXVbfBarl2270*
FooFFXVbfBarl56=(FooFFXVbfBarl2270* )FooFFXVbfBarl571;
FooFFXVbfBarn(FooFFXVbfBarl56->FooFFXVbfBarl279.ulReqLen!= FooFFXVbfBarl30( *FooFFXVbfBarl56))FooFFXVbfBarl73
FooFFXVbfBarl935;FFXTRACEPRINTF((FooFFXVbfBarl66( FooFFXVbfBarl162,2,0),"\x44\x69\x73\x70\x61\x74\x63\x68\x20\x46\x66\x78\x56\x62\x66\x52\x65\x67\x69\x6f\x6e\x4d\x65\x74\x72\x69\x63\x73\x28\x29\x20\x68\x56\x42\x46\x3d\x25\x50\x20\x52\x67\x6e\x3d\x25\x6c\x55\x20\x70\x52\x67\x6e\x4d\x65\x74\x73\x3d\x25\x50\n",FooFFXVbfBarl56-> FooFFXVbfBarp,FooFFXVbfBarl56->FooFFXVbfBarl380, FooFFXVbfBarl56->FooFFXVbfBarl292));FooFFXVbfBarl56->
FooFFXVbfBarl26=FooFFXVbfBarl765(FooFFXVbfBarl56-> FooFFXVbfBarp,FooFFXVbfBarl56->FooFFXVbfBarl380, FooFFXVbfBarl56->FooFFXVbfBarl292);FooFFXVbfBarl100;}
FooFFXVbfBarl143 FooFFXVbfBarl1032:{FooFFXVbfBarl2389*
FooFFXVbfBarl56=(FooFFXVbfBarl2389* )FooFFXVbfBarl571;
FooFFXVbfBarn(FooFFXVbfBarl56->FooFFXVbfBarl279.ulReqLen!= FooFFXVbfBarl30( *FooFFXVbfBarl56))FooFFXVbfBarl73
FooFFXVbfBarl935;FFXTRACEPRINTF((FooFFXVbfBarl66( FooFFXVbfBarl162,2,0),"\x44\x69\x73\x70\x61\x74\x63\x68\x20\x46\x66\x78\x56\x62\x66\x55\x6e\x69\x74\x4d\x65\x74\x72\x69\x63\x73\x28\x29\x20\x68\x56\x42\x46\x3d\x25\x50\x20\x70\x55\x6e\x69\x74\x4d\x65\x74\x73\x3d\x25\x50\n", FooFFXVbfBarl56->FooFFXVbfBarp,FooFFXVbfBarl56-> FooFFXVbfBarl550));FooFFXVbfBarl56->FooFFXVbfBarl26=
FooFFXVbfBarl721(FooFFXVbfBarl56->FooFFXVbfBarp, FooFFXVbfBarl56->FooFFXVbfBarl550);FooFFXVbfBarl100;}
FooFFXVbfBarl143 FooFFXVbfBarl1022:{FooFFXVbfBarl2430*
FooFFXVbfBarl56=(FooFFXVbfBarl2430* )FooFFXVbfBarl571;
FooFFXVbfBarn(FooFFXVbfBarl56->FooFFXVbfBarl279.ulReqLen!= FooFFXVbfBarl30( *FooFFXVbfBarl56))FooFFXVbfBarl73
FooFFXVbfBarl935;FFXTRACEPRINTF((FooFFXVbfBarl66( FooFFXVbfBarl162,2,0),"\x44\x69\x73\x70\x61\x74\x63\x68\x20\x46\x66\x78\x56\x62\x66\x52\x65\x61\x64\x50\x61\x67\x65\x73\x28\x29\x20\x68\x56\x42\x46\x3d\x25\x50\x20\x53\x74\x61\x72\x74\x50\x61\x67\x65\x3d\x25\x6c\x58\x20\x43\x6f\x75\x6e\x74\x3d\x25\x6c\x58\x20\x70\x42\x75\x66\x66\x3d\x25\x50\n",FooFFXVbfBarl56->FooFFXVbfBarp,FooFFXVbfBarl56 ->FooFFXVbfBary,FooFFXVbfBarl56->FooFFXVbfBarl31, FooFFXVbfBarl56->FooFFXVbfBarz));FooFFXVbfBarl56->
FooFFXVbfBarl=FooFFXVbfBarl536(FooFFXVbfBarl56-> FooFFXVbfBarp,FooFFXVbfBarl56->FooFFXVbfBary, FooFFXVbfBarl56->FooFFXVbfBarl31,FooFFXVbfBarl56-> FooFFXVbfBarz);FooFFXVbfBarl100;}FooFFXVbfBarl143
FooFFXVbfBarl1021:{FooFFXVbfBarl2282*FooFFXVbfBarl56=( FooFFXVbfBarl2282* )FooFFXVbfBarl571;FooFFXVbfBarn( FooFFXVbfBarl56->FooFFXVbfBarl279.ulReqLen!=FooFFXVbfBarl30 ( *FooFFXVbfBarl56))FooFFXVbfBarl73 FooFFXVbfBarl935;
FFXTRACEPRINTF((FooFFXVbfBarl66(FooFFXVbfBarl162,2,0),"\x44\x69\x73\x70\x61\x74\x63\x68\x20\x46\x66\x78\x56\x62\x66\x57\x72\x69\x74\x65\x50\x61\x67\x65\x73\x28\x29\x20\x68\x56\x42\x46\x3d\x25\x50\x20\x53\x74\x61\x72\x74\x50\x61\x67\x65\x3d\x25\x6c\x58\x20\x43\x6f\x75\x6e\x74\x3d\x25\x6c\x58\x20\x70\x42\x75\x66\x66\x3d\x25\x50\n", FooFFXVbfBarl56->FooFFXVbfBarp,FooFFXVbfBarl56-> FooFFXVbfBary,FooFFXVbfBarl56->FooFFXVbfBarl31, FooFFXVbfBarl56->FooFFXVbfBarz));FooFFXVbfBarl56->
FooFFXVbfBarl=FooFFXVbfBarl609(FooFFXVbfBarl56-> FooFFXVbfBarp,FooFFXVbfBarl56->FooFFXVbfBary, FooFFXVbfBarl56->FooFFXVbfBarl31,FooFFXVbfBarl56-> FooFFXVbfBarz);FooFFXVbfBarl100;}FooFFXVbfBarl143
FooFFXVbfBarl1033:{FooFFXVbfBarl2318*FooFFXVbfBarl56=( FooFFXVbfBarl2318* )FooFFXVbfBarl571;FooFFXVbfBarn( FooFFXVbfBarl56->FooFFXVbfBarl279.ulReqLen!=FooFFXVbfBarl30 ( *FooFFXVbfBarl56))FooFFXVbfBarl73 FooFFXVbfBarl935;
FFXTRACEPRINTF((FooFFXVbfBarl66(FooFFXVbfBarl162,2,0),"\x44\x69\x73\x70\x61\x74\x63\x68\x20\x46\x66\x78\x56\x62\x66\x44\x69\x73\x63\x61\x72\x64\x50\x61\x67\x65\x73\x28\x29\x20\x68\x56\x42\x46\x3d\x25\x50\x20\x53\x74\x61\x72\x74\x50\x61\x67\x65\x3d\x25\x6c\x58\x20\x43\x6f\x75\x6e\x74\x3d\x25\x6c\x58\n",FooFFXVbfBarl56->FooFFXVbfBarp, FooFFXVbfBarl56->FooFFXVbfBary,FooFFXVbfBarl56-> FooFFXVbfBarl31));FooFFXVbfBarl56->FooFFXVbfBarl=
FooFFXVbfBarl585(FooFFXVbfBarl56->FooFFXVbfBarp, FooFFXVbfBarl56->FooFFXVbfBary,FooFFXVbfBarl56-> FooFFXVbfBarl31);FooFFXVbfBarl100;}FooFFXVbfBarl143
FooFFXVbfBarl1020:{FooFFXVbfBarl1976*FooFFXVbfBarl56=( FooFFXVbfBarl1976* )FooFFXVbfBarl571;FooFFXVbfBarn( FooFFXVbfBarl56->FooFFXVbfBarl279.ulReqLen!=FooFFXVbfBarl30 ( *FooFFXVbfBarl56))FooFFXVbfBarl73 FooFFXVbfBarl935;
FFXTRACEPRINTF((FooFFXVbfBarl66(FooFFXVbfBarl162,2,0),"\x44\x69\x73\x70\x61\x74\x63\x68\x20\x46\x66\x78\x56\x62\x66\x43\x6f\x6d\x70\x61\x63\x74\x28\x29\x20\x68\x56\x42\x46\x3d\x25\x50\x20\x46\x6c\x61\x67\x73\x3d\x25\x6c\x55\n", FooFFXVbfBarl56->FooFFXVbfBarp,FooFFXVbfBarl56-> FooFFXVbfBarl65));FooFFXVbfBarl56->FooFFXVbfBarl=
FooFFXVbfBarl410(FooFFXVbfBarl56->FooFFXVbfBarp, FooFFXVbfBarl56->FooFFXVbfBarl65);FooFFXVbfBarl100;}
#if FFX_COMPACTIONMODEL != FFX_COMPACT_NONE
FooFFXVbfBarl143 FooFFXVbfBarl1030:{FooFFXVbfBarl2226*
FooFFXVbfBarl56=(FooFFXVbfBarl2226* )FooFFXVbfBarl571;
FooFFXVbfBarn(FooFFXVbfBarl56->FooFFXVbfBarl279.ulReqLen!= FooFFXVbfBarl30( *FooFFXVbfBarl56))FooFFXVbfBarl73
FooFFXVbfBarl935;FFXTRACEPRINTF((FooFFXVbfBarl66( FooFFXVbfBarl162,2,0),"\x44\x69\x73\x70\x61\x74\x63\x68\x20\x46\x66\x78\x56\x62\x66\x43\x6f\x6d\x70\x61\x63\x74\x49\x64\x6c\x65\x53\x75\x73\x70\x65\x6e\x64\x28\x29\x20\x68\x56\x42\x46\x3d\x25\x50\n",FooFFXVbfBarl56->FooFFXVbfBarp ));FooFFXVbfBarl56->FooFFXVbfBarv=FooFFXVbfBarl602( FooFFXVbfBarl56->FooFFXVbfBarp);FooFFXVbfBarl100;}
FooFFXVbfBarl143 FooFFXVbfBarl1039:{FooFFXVbfBarl2004*
FooFFXVbfBarl56=(FooFFXVbfBarl2004* )FooFFXVbfBarl571;
FooFFXVbfBarn(FooFFXVbfBarl56->FooFFXVbfBarl279.ulReqLen!= FooFFXVbfBarl30( *FooFFXVbfBarl56))FooFFXVbfBarl73
FooFFXVbfBarl935;FFXTRACEPRINTF((FooFFXVbfBarl66( FooFFXVbfBarl162,2,0),"\x44\x69\x73\x70\x61\x74\x63\x68\x20\x46\x66\x78\x56\x62\x66\x43\x6f\x6d\x70\x61\x63\x74\x49\x64\x6c\x65\x52\x65\x73\x75\x6d\x65\x28\x29\x20\x68\x56\x42\x46\x3d\x25\x50\n",FooFFXVbfBarl56->FooFFXVbfBarp));
FooFFXVbfBarl56->FooFFXVbfBarv=FooFFXVbfBarl749( FooFFXVbfBarl56->FooFFXVbfBarp);FooFFXVbfBarl100;}
#endif
#if FFXCONF_QUICKMOUNTSUPPORT
FooFFXVbfBarl143 FooFFXVbfBarl1028:{FooFFXVbfBarl2316*
FooFFXVbfBarl56=(FooFFXVbfBarl2316* )FooFFXVbfBarl571;
FooFFXVbfBarn(FooFFXVbfBarl56->FooFFXVbfBarl279.ulReqLen!= FooFFXVbfBarl30( *FooFFXVbfBarl56))FooFFXVbfBarl73
FooFFXVbfBarl935;FFXTRACEPRINTF((FooFFXVbfBarl66( FooFFXVbfBarl162,2,0),"\x44\x69\x73\x70\x61\x74\x63\x68\x20\x46\x66\x78\x56\x62\x66\x44\x69\x73\x61\x62\x6c\x65\x53\x74\x61\x74\x65\x53\x61\x76\x65\x28\x29\x20\x68\x56\x42\x46\x3d\x25\x50\x20\x43\x75\x73\x68\x69\x6f\x6e\x3d\x25\x6c\x55\x20\x4e\x75\x6d\x53\x70\x61\x72\x65\x73\x3d\x25\x6c\x55\x20\x46\x61\x73\x74\x3d\x25\x75\n", FooFFXVbfBarl56->FooFFXVbfBarp));FooFFXVbfBarn(! FooFFXVbfBarl56->FooFFXVbfBarp){FooFFXVbfBarl56->
FooFFXVbfBarl26=FooFFXVbfBarl156;}FooFFXVbfBarl62{
FooFFXVbfBarl664(FooFFXVbfBarl56->FooFFXVbfBarp);
FooFFXVbfBarl56->FooFFXVbfBarl26=FooFFXVbfBarl44;}
FooFFXVbfBarl100;}
#endif
FooFFXVbfBarl143 FooFFXVbfBarl1035:{FooFFXVbfBarl2199*
FooFFXVbfBarl56=(FooFFXVbfBarl2199* )FooFFXVbfBarl571;
FooFFXVbfBarn(FooFFXVbfBarl56->FooFFXVbfBarl279.ulReqLen!= FooFFXVbfBarl30( *FooFFXVbfBarl56))FooFFXVbfBarl73
FooFFXVbfBarl935;FFXTRACEPRINTF((FooFFXVbfBarl66( FooFFXVbfBarl162,2,0),"\x44\x69\x73\x70\x61\x74\x63\x68\x20\x46\x66\x78\x56\x62\x66\x46\x6f\x72\x6d\x61\x74\x28\x29\x20\x68\x46\x4d\x4c\x3d\x25\x50\x20\x43\x75\x73\x68\x69\x6f\x6e\x3d\x25\x6c\x55\x20\x4e\x75\x6d\x53\x70\x61\x72\x65\x73\x3d\x25\x6c\x55\x20\x46\x61\x73\x74\x3d\x25\x75\n",FooFFXVbfBarl56->FooFFXVbfBarm,FooFFXVbfBarl56-> FooFFXVbfBarl322,FooFFXVbfBarl56->FooFFXVbfBarl1311, FooFFXVbfBarl56->FooFFXVbfBarl277));FooFFXVbfBarl56->
FooFFXVbfBarl26=FooFFXVbfBarl592(FooFFXVbfBarl56-> FooFFXVbfBarm,FooFFXVbfBarl56->FooFFXVbfBarl322, FooFFXVbfBarl56->FooFFXVbfBarl1311,FooFFXVbfBarl56-> FooFFXVbfBarl277);FooFFXVbfBarl100;}FooFFXVbfBarl143
FooFFXVbfBarl1013:{FooFFXVbfBarl2378*FooFFXVbfBarl56=( FooFFXVbfBarl2378* )FooFFXVbfBarl571;FooFFXVbfBarn( FooFFXVbfBarl56->FooFFXVbfBarl279.ulReqLen!=FooFFXVbfBarl30 ( *FooFFXVbfBarl56))FooFFXVbfBarl73 FooFFXVbfBarl935;
FFXTRACEPRINTF((FooFFXVbfBarl66(FooFFXVbfBarl162,2,0),"\x44\x69\x73\x70\x61\x74\x63\x68\x20\x46\x66\x78\x56\x62\x66\x54\x65\x73\x74\x52\x65\x67\x69\x6f\x6e\x4d\x6f\x75\x6e\x74\x50\x65\x72\x66\x28\x29\x20\x68\x56\x42\x46\x3d\x25\x50\n",FooFFXVbfBarl56->FooFFXVbfBarp));FooFFXVbfBarl56->
FooFFXVbfBarl2687=FooFFXVbfBarl699(FooFFXVbfBarl56-> FooFFXVbfBarp);FooFFXVbfBarl100;}FooFFXVbfBarl143
FooFFXVbfBarl1006:{FooFFXVbfBarl2082*FooFFXVbfBarl56=( FooFFXVbfBarl2082* )FooFFXVbfBarl571;FooFFXVbfBarn( FooFFXVbfBarl56->FooFFXVbfBarl279.ulReqLen!=FooFFXVbfBarl30 ( *FooFFXVbfBarl56))FooFFXVbfBarl73 FooFFXVbfBarl935;
FFXTRACEPRINTF((FooFFXVbfBarl66(FooFFXVbfBarl162,2,0),"\x44\x69\x73\x70\x61\x74\x63\x68\x20\x46\x66\x78\x56\x62\x66\x54\x65\x73\x74\x57\x72\x69\x74\x65\x49\x6e\x74\x65\x72\x72\x75\x70\x74\x69\x6f\x6e\x73\x28\x29\x20\x68\x56\x42\x46\x3d\x25\x50\x20\x75\x6c\x53\x65\x65\x64\x3d\x25\x6c\x58\x20\x56\x65\x72\x62\x6f\x73\x69\x74\x79\x3d\x25\x75\n",FooFFXVbfBarl56->FooFFXVbfBarp,FooFFXVbfBarl56-> FooFFXVbfBarl340,FooFFXVbfBarl56->FooFFXVbfBarl222));
#if VBF_WRITEINT_ENABLED
FooFFXVbfBarl56->FooFFXVbfBarl2533=FooFFXVbfBarl635( FooFFXVbfBarl56->FooFFXVbfBarp,FooFFXVbfBarl56-> FooFFXVbfBarl340,FooFFXVbfBarl56->FooFFXVbfBarl222);
#else
FooFFXVbfBarl89("\x46\x46\x58\x3a\x20\x57\x72\x69\x74\x65\x20\x49\x6e\x74\x65\x72\x72\x72\x75\x70\x74\x69\x6f\x6e\x20\x74\x65\x73\x74\x69\x6e\x67\x20\x69\x73\x20\x6e\x6f\x74\x20\x65\x6e\x61\x62\x6c\x65\x64\x2e\n");
#endif
FooFFXVbfBarl100;}FooFFXVbfBarl1227:{FFXPRINTF(1,("\x46\x66\x78\x56\x62\x66\x49\x6f\x63\x74\x6c\x44\x69\x73\x70\x61\x74\x63\x68\x28\x29\x20\x46\x75\x6e\x63\x74\x69\x6f\x6e\x20\x30\x78\x25\x78\x20\x75\x6e\x6b\x6e\x6f\x77\x6e\n", FooFFXVbfBarl1565));FooFFXVbfBarl26=(FooFFXVbfBarq(5,15));
FooFFXVbfBarl100;}}FooFFXVbfBarl73 FooFFXVbfBarl369;
FooFFXVbfBarl935:FFXPRINTF(1,("\x46\x66\x78\x56\x62\x66\x49\x6f\x63\x74\x6c\x44\x69\x73\x70\x61\x74\x63\x68\x28\x29\x20\x42\x61\x64\x20\x49\x2f\x4f\x20\x72\x65\x71\x75\x65\x73\x74\x20\x70\x61\x63\x6b\x65\x74\x20\x6c\x65\x6e\x67\x74\x68\x20\x66\x6f\x72\x20\x66\x75\x6e\x63\x74\x69\x6f\x6e\x20\x30\x78\x25\x78\n",FooFFXVbfBarl1565));
FooFFXVbfBarl26=(FooFFXVbfBarq(5,11));FooFFXVbfBarl369:
FooFFXVbfBarl381(0);FooFFXVbfBarl46 FooFFXVbfBarl26;}
#endif
