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

/*---------------------------------------------------------------------------
                                Description

    This header file contains macros and symbols used to implement the
    trace facility in FlashFX.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: fxtrace.h $
    Revision 1.17  2012/03/02 19:04:25Z  garyp
    Added FFXTRACE_FIM.
    Revision 1.16  2010/12/12 06:51:47Z  garyp
    Added a missing const keyword.
    Revision 1.15  2010/01/04 21:54:49Z  billr
    FfxDecodeIOStatus() gets called even when output is disabled.
    Revision 1.14  2009/12/31 17:24:42Z  billr
    Refactor headers for better separation between 0S-dependent and
    OS-independent code.
    Revision 1.13  2009/07/18 01:13:44Z  garyp
    Merged from the v4.0 branch.  Added FFXTRACE_REGION.
    Revision 1.12  2009/04/14 20:43:36Z  garyp
    Added documentation -- no functional changes.
    Revision 1.11  2009/02/09 00:58:44Z  garyp
    Added FFXTRACE_ALWAYS.
    Revision 1.10  2008/05/03 20:26:37Z  garyp
    Updated to build cleanly when output is disabled.
    Revision 1.9  2007/11/03 23:49:38Z  Garyp
    Updated to use the standard module header.
    Revision 1.8  2007/09/08 17:18:35Z  Garyp
    Modified so the trace related functions are compiled and exposed even when
    building in release mode, though they would not normally get linked in.
    Revision 1.7  2007/03/18 17:55:41Z  Garyp
    Added FfxTraceMaskGet() and FfxTraceMaskSwap().
    Revision 1.6  2007/03/10 04:55:05Z  Garyp
    Updated to prevent inclusion more than once.
    Revision 1.5  2006/08/21 21:59:08Z  Garyp
    Added FFXTRACE_WEARLEVELING.
    Revision 1.4  2006/02/08 21:52:10Z  Garyp
    Added trace levels.
    Revision 1.3  2006/01/07 22:02:46Z  Garyp
    Added FFXTRACE_DRIVERFW.
    Revision 1.2  2006/01/02 02:21:58Z  Garyp
    New trace types.
    Revision 1.1  2005/11/14 14:31:36Z  Pauli
    Initial revision
    Revision 1.5  2005/11/14 14:31:35Z  Garyp
    New trace levels.
    Revision 1.4  2005/10/25 04:15:52Z  Garyp
    Added FFXTRACE_NTM.
    Revision 1.3  2005/10/22 07:00:19Z  garyp
    Minor symbol renaming exercise.
    Revision 1.2  2005/10/14 02:02:26Z  Garyp
    Added FFXTRACE_COMPACTION.
    Revision 1.1  2005/08/21 10:42:10Z  Garyp
    Initial revision
---------------------------------------------------------------------------*/

#ifndef FXTRACE_H_INCLUDED
#define FXTRACE_H_INCLUDED

#if D_DEBUG && DCLCONF_OUTPUT_ENABLED


/*-------------------------------------------------------------------
    Trace Classes

    The various FFXTRACE_class values are used in the FFXTRACEPRINTF
    macro.

    Use the TRACEALWAYS class to generate a message regardless what
    the FFX_TRACEMASK setting is in ffxconf.h (if the debug level is
    OK).

    A maximum of 32 levels are currently supported since the
    FFX_TRACEMASK is a bitmapped field, 32-bits wide.
-------------------------------------------------------------------*/
#define FFXTRACE_ALWAYS         DCLTRACE_ALWAYS /* The same for all products */
#define FFXTRACE_DRIVER         1
#define FFXTRACE_DRIVERFW       2
#define FFXTRACE_VBF            3
#define FFXTRACE_COMPACTION     4
#define FFXTRACE_MEDIAMGR       5
#define FFXTRACE_FML            6
#define FFXTRACE_BBM            7
#define FFXTRACE_NAND           8
#define FFXTRACE_NTM            9
#define FFXTRACE_NTMHOOK       10
#define FFXTRACE_CONFIG        11
#define FFXTRACE_EXTAPI        12
#define FFXTRACE_DEVMGR        13
#define FFXTRACE_WEARLEVELING  14
#define FFXTRACE_REGION        15
#define FFXTRACE_FIM           16


/*-------------------------------------------------------------------
    Trace Mask Bits

    The various FFXTRACEBIT_class values are used to create the
    FFX_TRACEMASK value which is used to customize the trace output.
    This is a bit-mapped mask that is defined in ffxconf.h.
-------------------------------------------------------------------*/
#define FFXTRACEBIT_NONE            0
#define FFXTRACEBIT_ALL             D_UINT32_MAX
#define FFXTRACEBIT_DRIVER          ((D_UINT32)1 << (FFXTRACE_DRIVER-1))
#define FFXTRACEBIT_DRIVERFW        ((D_UINT32)1 << (FFXTRACE_DRIVERFW-1))
#define FFXTRACEBIT_VBF             ((D_UINT32)1 << (FFXTRACE_VBF-1))
#define FFXTRACEBIT_COMPACTION      ((D_UINT32)1 << (FFXTRACE_COMPACTION-1))
#define FFXTRACEBIT_MEDIAMGR        ((D_UINT32)1 << (FFXTRACE_MEDIAMGR-1))
#define FFXTRACEBIT_FML             ((D_UINT32)1 << (FFXTRACE_FML-1))
#define FFXTRACEBIT_BBM             ((D_UINT32)1 << (FFXTRACE_BBM-1))
#define FFXTRACEBIT_NAND            ((D_UINT32)1 << (FFXTRACE_NAND-1))
#define FFXTRACEBIT_NTM             ((D_UINT32)1 << (FFXTRACE_NTM-1))
#define FFXTRACEBIT_NTMHOOK         ((D_UINT32)1 << (FFXTRACE_NTMHOOK-1))
#define FFXTRACEBIT_CONFIG          ((D_UINT32)1 << (FFXTRACE_CONFIG-1))
#define FFXTRACEBIT_EXTAPI          ((D_UINT32)1 << (FFXTRACE_EXTAPI-1))
#define FFXTRACEBIT_DEVMGR          ((D_UINT32)1 << (FFXTRACE_DEVMGR-1))
#define FFXTRACEBIT_WEARLEVELING    ((D_UINT32)1 << (FFXTRACE_WEARLEVELING-1))
#define FFXTRACEBIT_REGION          ((D_UINT32)1 << (FFXTRACE_REGION-1))
#define FFXTRACEBIT_FIM             ((D_UINT32)1 << (FFXTRACE_FIM-1))


/*-------------------------------------------------------------------
    Macro: FFXPRINTF()

    Display debug output if the module is compiled with a D_DEBUG
    level >= to "nLevel".

    FFXPRINTF() may <only> be used from with the DCL code base.

    Note that specifying nLevel as '0' is not suppported -- use
    DclPrintf() instead.

    The syntax is...

    FFXPRINTF(nLevel, (DclPrintf args));

    Parameters:
        nLevel           - The debug level, from 1 to 3
        (DclPrintf args) - The standard DclPrintf() arguments
                           enclosed in parenthesis.

    Return Value:
        None.
-------------------------------------------------------------------*/

/*-------------------------------------------------------------------
    Macro: FFXTRACEPRINTF()

    Display debug output if the module in question is compiled with
    a D_DEBUG level >= to "nLevel" and if the FFXTRACEMASK value
    specifies that the given class is to be handled.

    The syntax is...

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_class, nLevel, flags), DclPrintf args));

    The "FFXTRACE_class" value may be "TRACEALWAYS" so that no class
    match is required in the TRACEFFXMASK value.  This is useful for
    debug output that you want to always display (assuming "nLevel"
    matches), similar to FFXPRINTF(), however unlike FFXPRINTF() it
    gives the ability to specify indentation.

    The "ulFlags" parameter may be TRACEINDENT, TRACEUNDENT, or
    TRACENODENT.  Indents must match undents, however the debug
    level "nlevel" does NOT need to be the same where indents and
    undents are specified.  TRACEOFF disables tracing until a
    TRACEON is received.  Nested TRACEOFFs must be matched by an
    equal number of TRACEONs.

    Parameters:
        ulFlags          - The trace flags, as described above.
        (DclPrintf args) - The standard DclPrintf() arguments
                           enclosed in parenthesis.

    Return Value:
        None.
-------------------------------------------------------------------*/
int         FfxDebugPrintf(const char *pszFmt, ...);
int         FfxTracePrintf(D_UINT32 ulFlags, const char *pszFmt, ...);
D_UINT32    FfxTraceMaskGet(void);
D_UINT32    FfxTraceMaskSwap(D_UINT32 ulNewMask);

#define FFXPRINTF(lev, txt) (((lev)<=D_DEBUG) ? (void)FfxDebugPrintf txt : ((void)0))
#define FFXTRACEPRINTF(params)  FfxTracePrintf params

#else

#define FFXPRINTF(lev, txt)
#define FFXTRACEPRINTF(params)

#endif  /* D_DEBUG && DCLCONF_OUTPUT_ENABLED */

/*  This function is used in a few non-debug messages.
*/
const char *FfxDecodeIOStatus(const FFXIOSTATUS *pIOStatus);


#endif  /* FXTRACE_H_INCLUDED */

