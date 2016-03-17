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

    This header file contains macros and symbols used to implement the
    trace facility in DCL.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dltrace.h $
    Revision 1.22  2010/11/01 06:47:02Z  garyp
    Expose more trace related stuff even when output support is disabled,
    to accommodate the Reliance TraceLog feature.
    Revision 1.21  2010/11/01 03:28:34Z  garyp
    Cleaned up the status bit definitions.
    Revision 1.20  2010/04/17 22:10:33Z  garyp
    Added DCLTRACE_BLOCKDEV.  Documentation updated.
    Revision 1.19  2009/11/20 18:23:15Z  garyp
    Added DCLTRACEBIT_CRITSEC.
    Revision 1.18  2009/07/16 21:42:54Z  garyp
    Relocated the DCLTRACE_BACKTRACEBUFFERSIZE so things build
    cleanly when output is disabled.
    Revision 1.17  2009/07/15 06:07:48Z  keithg
    Added FTPFS tracing bit definitions.
    Revision 1.16  2009/07/01 03:49:06Z  garyp
    Updated to allow DCLPRINTF() and DCLTRACEPRINTF() to be always
    defined, even when the header is used in non-DCL code.
    Revision 1.15  2009/06/25 00:35:59Z  garyp
    Added DCLTRACE_SERVICE.  Added support for backtracing.
    Revision 1.14  2009/04/14 20:43:34Z  garyp
    Updated documentation -- no functional changes.
    Revision 1.13  2009/02/08 01:21:05Z  garyp
    Merged from the v4.0 branch.  Added DCLTRACE_EXTAPI.  Added DCLTRACE_ALWAYS.
    Added trace levels for use in Windows specific code.
    Revision 1.12  2008/05/03 02:47:46Z  garyp
    Updated to build cleanly when output is disabled.
    Revision 1.11  2008/03/22 19:32:28Z  Garyp
    Minor prototype changes.
    Revision 1.10  2007/11/03 23:31:21Z  Garyp
    Added the standard module header.
    Revision 1.9  2007/10/20 22:20:41Z  Garyp
    Added DclTraceIndentGet/Set().
    Revision 1.8  2007/06/20 21:30:34Z  Garyp
    Added trace levels for the requestor interface.
    Revision 1.7  2007/03/18 17:55:41Z  Garyp
    Added DclTraceMaskGet() and DclTraceMaskSwap().
    Revision 1.6  2006/11/14 21:14:59Z  Garyp
    Updated so that DCLPRINTF() and DCLTRACEPRINTF() are only defined if we are
    building in the DCL locale.
    Revision 1.5  2006/11/03 21:46:40Z  Garyp
    Added DCLFUNC() declarations for DclTracePrintf() and DclDebugPrintf().
    Revision 1.4  2006/08/18 20:20:06Z  Garyp
    Minor cleanup.  Changed some terminology for clarity -- nothing functional.
    Revision 1.3  2006/05/26 19:54:04Z  Garyp
    Documentation fixes.
    Revision 1.2  2006/01/07 16:05:52Z  Garyp
    Improved parameter validation.
    Revision 1.1  2005/10/20 21:29:24Z  Pauli
    Initial revision
    Revision 1.2  2005/10/20 22:29:24Z  garyp
    Added the "global" trace capability.
    Revision 1.1  2005/08/03 19:03:02Z  Garyp
    Initial revision
    Revision 1.2  2005/08/03 19:03:02Z  pauli
    Replaced DCL_FuncName with DclFuncName.
    Revision 1.1  2005/07/14 18:56:12Z  pauli
    Initial revision
---------------------------------------------------------------------------*/

#ifndef DLTRACE_H_INCLUDED
#define DLTRACE_H_INCLUDED

/*-------------------------------------------------------------------
    DCLTRACEPRINTF Flags Layout

               High 16-bits           Low 16-bits
            rrrr rrdd rrrf ffff - pppp pccc cccc clll
            \     /\/ \ /\    /   \    /\        /\ /
             \   /  |  |  \  /     \  /  \      /  |
              \ / Debug|  Flags   Locale  \    / Level
           Reserved1   |                  Class
                   Reserved2

    Notes:
        Debug - Denotes the D_DEBUG value for the actual calling
                module.  This may not match the value that dltrace.c
                was compiled with, which is why this is passed at
                run-time rather than using the compile-time value
                for dltrace.c.
        Level - Denotes the desired debug level.  The specified debug
                message will be displayed if "Debug" >= "Level" (and
                other conditions such as "Class" are met).
       Locale - This is the D_PRODUCTLOCALE value, as defined in the
                master header for the code base being built.  It is
                NOT the D_PRODUCTNUM value, which denotes the master
                product being built.
-------------------------------------------------------------------*/
#define DCLTRACEDEBUGBITS      (2)        /* debug bits */
#define DCLTRACEDEBUGMASK      ((1 << DCLTRACEDEBUGBITS) - 1)
#define DCLTRACEDEBUGSHIFT     (24)

#define DCLTRACEFLAGSBITS      (5)        /* flags bits */
#define DCLTRACEFLAGSMASK      ((1 << DCLTRACEFLAGSBITS) - 1)
#define DCLTRACEFLAGSSHIFT     (16)

#define DCLTRACELOCALEBITS     (5)        /* product locale bits */
#define DCLTRACELOCALEMASK     ((1 << DCLTRACELOCALEBITS) - 1)
#define DCLTRACELOCALESHIFT    (11)

#define DCLTRACECLASSBITS      (8)        /* class bits */
#define DCLTRACECLASSMASK      ((1 << DCLTRACECLASSBITS) - 1)
#define DCLTRACECLASSSHIFT     (3)

#define DCLTRACELEVELBITS      (3)        /* level bits */
#define DCLTRACELEVELMASK      ((1 << DCLTRACELEVELBITS) - 1)
#define DCLTRACELEVELSHIFT     (0)

#if 0
--- These macros are not currently required ---

#define DCLTRACERESERVE1BITS   (6)        /* reserved1 bits */
#define DCLTRACERESERVE1MASK   ((1 << DCLTRACERESERVE1BITS) - 1)
#define DCLTRACERESERVE1SHIFT  (26)

#define DCLTRACERESERVE2BITS   (3)        /* reserved2 bits */
#define DCLTRACERESERVE2MASK   ((1 << DCLTRACERESERVE2BITS) - 1)
#define DCLTRACERESERVE2SHIFT  (21)

#define DCLTRACEGETRESERVE1(rs1) (((D_UINT32)(rs1) >> (DCLTRACERESERVE1SHIFT)) & DCLTRACERESERVE1MASK)
#define DCLTRACESETRESERVE1(rs1) ((D_UINT32)((rs1) &   DCLTRACERESERVE1MASK) << (DCLTRACERESERVE1SHIFT))

#define DCLTRACEGETRESERVE2(rs2) (((D_UINT32)(rs2) >> (DCLTRACERESERVE2SHIFT)) & DCLTRACERESERVE2MASK)
#define DCLTRACESETRESERVE2(rs2) ((D_UINT32)((rs2) &   DCLTRACERESERVE2MASK) << (DCLTRACERESERVE2SHIFT))
#endif

#define DCLTRACEGETDEBUG(dbg)    (((D_UINT32)(dbg) >> (DCLTRACEDEBUGSHIFT)) & DCLTRACEDEBUGMASK)
#define DCLTRACESETDEBUG(dbg)    ((D_UINT32)((dbg) &   DCLTRACEDEBUGMASK) << (DCLTRACEDEBUGSHIFT))

#define DCLTRACEGETLOCALE(loc)   (((D_UINT32)(loc) >> (DCLTRACELOCALESHIFT)) & DCLTRACELOCALEMASK)
#define DCLTRACESETLOCALE(loc)   ((D_UINT32)((loc) &   DCLTRACELOCALEMASK) << (DCLTRACELOCALESHIFT))

#define DCLTRACEGETCLASS(clas)   (((D_UINT32)(clas) >> (DCLTRACECLASSSHIFT)) & DCLTRACECLASSMASK)
#define DCLTRACESETCLASS(clas)   ((D_UINT32)((clas) &   DCLTRACECLASSMASK) << (DCLTRACECLASSSHIFT))

#define DCLTRACEGETFLAGS(flg)    (((D_UINT32)(flg) >> (DCLTRACEFLAGSSHIFT)) & DCLTRACEFLAGSMASK)
#define DCLTRACESETFLAGS(flg)    ((D_UINT32)((flg) &   DCLTRACEFLAGSMASK) << (DCLTRACEFLAGSSHIFT))

#define DCLTRACEGETLEVEL(lev)    (((D_UINT32)(lev) >> (DCLTRACELEVELSHIFT)) & DCLTRACELEVELMASK)
#define DCLTRACESETLEVEL(lev)    ((D_UINT32)((lev) &   DCLTRACELEVELMASK) << (DCLTRACELEVELSHIFT))

/*  This is a general macro for combining all the elements
    together to make a DCLTRACEPRINTF FLAGS value.
*/
#define MAKETRACEFLAGS(clas, lev, flg)      \
    ((D_UINT32)(                            \
    DCLTRACESETDEBUG(D_DEBUG) |             \
    DCLTRACESETLOCALE(D_PRODUCTLOCALE) |    \
    DCLTRACESETCLASS(clas) |                \
    DCLTRACESETLEVEL(lev) |                 \
    DCLTRACESETFLAGS(flg)))

/*-------------------------------------------------------------------
    Trace Flags
-------------------------------------------------------------------*/
#define TRACENODENT         (0x00)
#define TRACEINDENT         (0x01)
#define TRACEUNDENT         (0x02)
#define TRACEON             (0x04)
#define TRACEOFF            (0x08)
#define TRACEFORCEON        (0x10)


#if D_DEBUG && DCLCONF_OUTPUT_ENABLED


/*-------------------------------------------------------------------
    Trace Classes

    The various DCLTRACE_class values are used in the DCLTRACEPRINTF
    macro.

    Use the TRACEALWAYS class to generate a message regardless what
    the DCLTRACEMASK setting is in dclconf.h (if the debug level is
    OK).

    A maximum of 32 levels are currently supported since the
    DCLTRACEMASK is a bitmapped field, 32-bits wide.
-------------------------------------------------------------------*/

#define DCLTRACE_ALWAYS         0
#define DCLTRACE_MEM            1
#define DCLTRACE_MUTEX          2
#define DCLTRACE_SEMAPHORE      3
#define DCLTRACE_HEAP           4
#define DCLTRACE_REQUESTOR      5
#define DCLTRACE_WINAPP         6
#define DCLTRACE_WINAPPFW       7
#define DCLTRACE_WINGUI         8
#define DCLTRACE_EXTAPI         9
#define DCLTRACE_SERVICE       10
#define DCLTRACE_FTPFS         11
#define DCLTRACE_CRITSEC       12
#define DCLTRACE_BLOCKDEV      13

#define TRACEALWAYS             DCLTRACE_ALWAYS /* Deprecated, use DCLTRACE_ALWAYS */


/*-------------------------------------------------------------------
    Trace Mask Bits

    The various DCLTRACEBIT_class values are used to create the
    DCLTRACEMASK value which is used to customize the trace output.
    This is a bit-mapped mask that is defined in dclconf.h.
-------------------------------------------------------------------*/
#define DCLTRACEBIT_NONE        0
#define DCLTRACEBIT_ALL         D_UINT32_MAX
#define DCLTRACEBIT_MEM         ((D_UINT32)1 << (DCLTRACE_MEM-1))
#define DCLTRACEBIT_MUTEX       ((D_UINT32)1 << (DCLTRACE_MUTEX-1))
#define DCLTRACEBIT_SEMAPHORE   ((D_UINT32)1 << (DCLTRACE_SEMAPHORE-1))
#define DCLTRACEBIT_HEAP        ((D_UINT32)1 << (DCLTRACE_HEAP-1))
#define DCLTRACEBIT_REQUESTOR   ((D_UINT32)1 << (DCLTRACE_REQUESTOR-1))
#define DCLTRACEBIT_WINAPP      ((D_UINT32)1 << (DCLTRACE_WINAPP-1))
#define DCLTRACEBIT_WINAPPFW    ((D_UINT32)1 << (DCLTRACE_WINAPPFW-1))
#define DCLTRACEBIT_WINGUI      ((D_UINT32)1 << (DCLTRACE_WINGUI-1))
#define DCLTRACEBIT_EXTAPI      ((D_UINT32)1 << (DCLTRACE_EXTAPI-1))
#define DCLTRACEBIT_SERVICE     ((D_UINT32)1 << (DCLTRACE_SERVICE-1))
#define DCLTRACEBIT_FTPFS       ((D_UINT32)1 << (DCLTRACE_FTPFS-1))
#define DCLTRACEBIT_CRITSEC     ((D_UINT32)1 << (DCLTRACE_CRITSEC-1))
#define DCLTRACEBIT_BLOCKDEV    ((D_UINT32)1 << (DCLTRACE_BLOCKDEV-1))


/*-------------------------------------------------------------------
    Macro: DCLPRINTF()

    Display debug output if the module is compiled with a D_DEBUG
    level >= to "nLevel".

    DCLPRINTF() may <only> be used from with the DCL code base.

    Note that specifying nLevel as '0' is not suppported -- use
    DclPrintf() instead.

    The syntax is...

    DCLPRINTF(nLevel, (DclPrintf args));

    Parameters:
        nLevel           - The debug level, from 1 to 3
        (DclPrintf args) - The standard DclPrintf() arguments
                           enclosed in parenthesis.

    Return Value:
        None.
-------------------------------------------------------------------*/

/*-------------------------------------------------------------------
    Macro:  DCLTRACEPRINTF()

    Display debug output if the module in question is compiled with
    a D_DEBUG level >= to "nLevel" and if the DCLTRACEMASK value
    specifies that the given class is to be handled.

    The syntax is...

    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_class, nLevel, flags), DclPrintf args));

    The "DCLTRACE_class" value may be "TRACEALWAYS" so that no class
    match is required in the TRACEDCLMASK value.  This is useful for
    debug output that you want to always display (assuming "nLevel"
    matches), similar to DCLPRINTF(), however unlike DCLPRINTF() it
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
#define     DclDebugPrintf      DCLFUNC(DclDebugPrintf)
#define     DclTracePrintf      DCLFUNC(DclTracePrintf)
#define     DclTraceIndentGet   DCLFUNC(DclTraceIndentGet)
#define     DclTraceIndentSet   DCLFUNC(DclTraceIndentSet)
#define     DclTraceMaskGet     DCLFUNC(DclTraceMaskGet)
#define     DclTraceMaskSwap    DCLFUNC(DclTraceMaskSwap)
#define     DclTraceDumpBackTrace   DCLFUNC(DclTraceDumpBackTrace)

int         DclDebugPrintf(const char *pszFmt, ...);
int         DclTracePrintf(D_UINT32 ulFlags, const char *pszFmt, ...);
unsigned    DclTraceIndentGet(void);
void        DclTraceIndentSet(unsigned nIndent);
D_UINT32    DclTraceMaskGet(void);
D_UINT32    DclTraceMaskSwap(D_UINT32 ulNewMask);
unsigned    DclTraceDumpBackTrace(unsigned nMaxRecords);

/*  Normally the DCLPRINTF() and DCLTRACEPRINTF() macros are only
    used within DCL code itself, however there are some higher level
    macros such as DCLWINMESSAGETRACEPRINT() which are defined to
    use those macros, but themselves may be used in non-DCL code.
*/
#define DCLPRINTF(lev, txt) (((lev)<=D_DEBUG) ? (void)DclDebugPrintf txt : ((void)0))
#define DCLTRACEPRINTF(params)  DclTracePrintf params


/*-------------------------------------------------------------------
    Macro: TRACEGLOBALSTART()

    Force all trace messages less than or equal to the specified debug
    level to be displayed, regardless of the trace class flags.

    Parameters:
        nLevel    - The debug level, from 0 to 3.
        szMessage - The null-terminated message to display.

    Return Value:
        None.
-------------------------------------------------------------------*/
#define TRACEGLOBALSTART(nLevel, szMessage)      \
    DclTracePrintf(MAKETRACEFLAGS(TRACEALWAYS, nLevel, TRACEINDENT|TRACEFORCEON), "Global Trace Start - " szMessage "\n");


/*-------------------------------------------------------------------
    Macro: TRACEGLOBALSTOP()

    Stop global tracing which was started with TRACEGLOBALSTART().

    Parameters:
        None.

    Return Value:
        None.
-------------------------------------------------------------------*/
#define TRACEGLOBALSTOP()               \
    DclTracePrintf(MAKETRACEFLAGS(TRACEALWAYS, 1, TRACEUNDENT|TRACEOFF), "Global Trace Stop\n");


#else

#undef DCLTRACEMASK
#define DCLTRACEMASK (0)

#define DclTraceIndentGet()     (0)
#define DclTraceIndentSet       (void)

#define DCLPRINTF(lev, txt)
#define DCLTRACEPRINTF(params)


#endif  /* D_DEBUG && DCLCONF_OUTPUT_ENABLED */


/*  The BackTrace feature is not officially documented or supported at
    this time.  Initialize it such that it is disabled if it has not
    already been turned on somewhere (such as dclconf.h).
*/
#ifndef DCLTRACE_BACKTRACEBUFFERSIZE
#define DCLTRACE_BACKTRACEBUFFERSIZE   (0)
#endif


#endif  /* DLTRACE_H_INCLUDED */


