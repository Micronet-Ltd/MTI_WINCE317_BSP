/*---------------------------------------------------------------------------
               ----> DO NOT REMOVE THE FOLLOWING NOTICE <----

                  Copyright (c) 1993 - 2010 Datalight, Inc.
                       All Rights Reserved Worldwide.

  Datalight, Incorporated is a Washington State corporation currently located
  at:
        21520 30th Dr SE, Suite 110,      Tel:  425-951-8086
        Bothell, WA  98021                Fax:  425-951-8094
        USA                               Web:  http://www.datalight.com

  This software, including without limitation all source code and documen-
  tation, is a trade secret and the confidential property of Datalight, 
  Inc., protected under the copyright laws of the United States and other
  jurisdictions.  Patents may be pending.

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

/*---------------------------------------------------------------------------
                                Description

    This header ensures that the basic variables required for compilation of
    Datalight code are initialized.  This includes the following:

    1) Ensuring that the D_* command-line variables which the ToolSet
       configures are initialized.
    2) Ensuring that internal settings configured by the CPU and ToolSet
       specific headers are initialized.
    3) Initializing default types and settings for those items which the
       CPU and ToolSet specific headers declined to initialize.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dlenv.h $
    Revision 1.10  2010/03/10 21:57:28Z  jimmb
    Added Linux specific i386 and powerpc.
    Revision 1.9  2009/11/25 22:38:23Z  keithg
    Corrected Bug 2894, removed obsolete DCLPACK definition.
    Revision 1.8  2008/06/16 21:08:15Z  jimmb
    I'm going make a judgment call and say that the PPC should be
    Treat the PPC more like the ARM and less like the MIPS, default to
    4 byte alignment requirements, The PPC seems to indicate that it is
    more tolerant of misaligned accesses.
    Revision 1.7  2007/11/03 23:31:21Z  Garyp
    Added the standard module header.
    Revision 1.6  2007/02/01 19:59:01Z  Garyp
    Updated the SH cpu family to use an alignment of 4.
    Revision 1.5  2007/01/13 03:32:55Z  Garyp
    Moved the emulated 64-bit type definition from dlenv.h into dlapi.h because
    the former is not included when building external code.
    Revision 1.4  2006/12/08 03:09:21Z  Garyp
    Updated to validate that the D_CPUFAMILY and D_CPUTYPE symbols are defined.
    Modified to initialize the processor alignment boundary based on the CPU
    family setting.
    Revision 1.3  2006/10/05 23:41:24Z  Garyp
    Modified the D_UINT64 structure for emulated 64-bit types so that it more
    closely approximates a native endian value regardless whether the system
    is big or little-endian.
    Revision 1.2  2006/08/21 20:08:18Z  billr
    dlend.h already makes sure DCL_BIG_ENDIAN is defined.  Checking it here
    doesn't work in some OS environments because it comes from dlosconf.h.
    Revision 1.1  2006/08/15 19:52:38Z  Garyp
    Initial revision
---------------------------------------------------------------------------*/

#ifndef DLENV_H_INCLUDED
#define DLENV_H_INCLUDED

#include <dlcpu.h>

/*-------------------------------------------------------------------
    Validate that all the required command-line defined symbols are
    set.  These are typically all initialized by the ToolSet code
    (toolinit.bat and toolset.mak), and then formed into command-
    line symbol definitions by dcl.mak.

    D_PRODUCTNUM is the product number for the master product being
    built.  For example if DCL is being built as a part of FlashFX
    (as a sub-product), the D_PRODUCTNUM value will be set to
    FlashFX even when compiling the DCL code.  The D_PRODUCTNUM
    value originates in the prodinfo.bat file for each product.
-------------------------------------------------------------------*/

#ifndef D_PRODUCTNUM
    #error "DCL: dlenv.h: ToolSet error -- D_PRODUCTNUM is not defined"
#endif

#ifndef D_PRODUCTVER
    #error "DCL: dlenv.h: ToolSet error -- D_PRODUCTVER is not defined"
#endif

#ifndef D_PRODUCTBUILD
    #error "DCL: dlenv.h: ToolSet error -- D_PRODUCTBUILD is not defined"
#endif

#ifndef D_TOOLSETNUM
    #error "DCL: dlenv.h: ToolSet error -- D_TOOLSETNUM is not defined"
#endif

#ifndef D_CPUFAMILY
    #error "DCL: dlenv.h: ToolSet error -- D_CPUFAMILY is not defined"
#endif

#ifndef D_CPUTYPE
    #error "DCL: dlenv.h: ToolSet error -- D_CPUTYPE is not defined"
#endif

#ifndef D_DEBUG
    #error "DCL: dlenv.h: ToolSet error -- D_DEBUG is not defined"
#endif

#ifndef D_OSVER
    #error "DCL: dlenv.h: ToolSet error -- D_OSVER is not defined"
#endif


/*-------------------------------------------------------------------
    D_PRODUCTLOCALE is used to denote the code base that a given
    module belongs to.  Therefore is is defined in the master header
    for each product, but is only defined if it is not already set.
    For those situations where one code base includes master headers
    for more than one code base (reliance.h and dcl.h for example),
    the first one will take precedence.

    Do not confuse D_PRODUCTLOCALE with D_PRODUCTNUM, described
    above.
-------------------------------------------------------------------*/

#ifndef D_PRODUCTLOCALE
    #error "DCL: dlenv.h: D_PRODUCTLOCALE is not defined"
#endif


/*-------------------------------------------------------------------
    Validate that the CPU and ToolSet specific headers have defined
    at least the minimal set of information we require by this point.
-------------------------------------------------------------------*/
#ifndef DCL_NATIVE_64BIT_SUPPORT
    #error "DCL: dlenv.h: DCL_NATIVE_64BIT_SUPPORT is not defined"
#endif


/*-------------------------------------------------------------------
    DCL_CPU_ALIGNSIZE

    This value defines the CPU specific alignment requirements.

    Most processors used in the embedded environment have strict
    data alignment requirements, and the compilers used therein
    ensure this alignment.  However some processors, notably the
    x86 do not require alignment, even though it is desirable for
    performance reasons.

    If DCL_CPU_ALIGNSIZE has not already been defined, define it
    here, based on the CPU family specified, or whether native
    64-bit types are supported or not.
-------------------------------------------------------------------*/
#if !defined(DCL_CPU_ALIGNSIZE)
    #if D_CPUFAMILY == D_CPUFAMILY_simulator
        #define DCL_CPU_ALIGNSIZE   (1)
    #elif D_CPUFAMILY == D_CPUFAMILY_x86
        #define DCL_CPU_ALIGNSIZE   (1)
    #elif D_CPUFAMILY == D_CPUFAMILY_i386
        #define DCL_CPU_ALIGNSIZE   (1)    
    #elif D_CPUFAMILY == D_CPUFAMILY_arm
        #define DCL_CPU_ALIGNSIZE   (4)
    #elif D_CPUFAMILY == D_CPUFAMILY_sh
        #define DCL_CPU_ALIGNSIZE   (4)
    #elif D_CPUFAMILY == D_CPUFAMILY_ppc
        #define DCL_CPU_ALIGNSIZE   (4)
    #elif D_CPUFAMILY == D_CPUFAMILY_powerpc
        #define DCL_CPU_ALIGNSIZE   (4)
    #elif DCL_NATIVE_64BIT_SUPPORT
        #define DCL_CPU_ALIGNSIZE   (8)
    #else
        #define DCL_CPU_ALIGNSIZE   (4)
    #endif
#endif


#endif /* DLENV_H_INCLUDED */

