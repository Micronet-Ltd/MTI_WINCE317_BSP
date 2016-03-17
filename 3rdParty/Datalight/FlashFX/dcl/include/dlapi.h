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
  jurisdictions. 

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

  Notwithstanding the foregoing, Licensee acknowledges that the software may
  be distributed as part of a package containing, and/or in conjunction with
  other source code files, licensed under so-called "open source" software 
  licenses.  If so, the following license will apply in lieu of the terms set
  forth above:

  Redistribution and use of this software in source and binary forms, with or
  without modification, are permitted provided that the following conditions
  are met:
  1. Redistributions of source code must retain the above copyright notice,
     this list of conditions, and the following disclaimer.
  2. Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions, and the following disclaimer in the
     documentation and/or other materials provided with the distribution.
  
  THIS SOFTWARE IS PROVIDED BY DATALIGHT "AS IS" AND ANY EXPRESS OR IMPLIED
  WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MER-
  CHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, OR NON-INFRINGEMENT, ARE
  DISCLAIMED.  IN NO EVENT SHALL DATALIGHT BE LIABLE FOR ANY DIRECT, INDIRECT,
  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
  OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEG-
  LIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Description

    This header defines the public API to the Datalight Common Libraries.

                            DCL Header Hierarchy

    DCL interfaces are classified into separate headers based on their use
    and visibility.  Every source code module in DCL includes either dcl.h
    or dcl_osname.h as the top level header (after any OS-specific headers).
 
    dcl.h        - This is the top-level header for DCL code which is built
                   using the standard Datalight build process.  This header
                   autmatically includes dlapi.h, dlapiint.h, and dlapimap.h
                   (among others).

    dcl_osname.h - This is the OS-specific top-level header for DCL code
                   which may be built outside the standard Datalight build
                   process.  This header automatically includes dlapi.h.

                            Noteworthy Sub-Headers

    dlapi.h      - This header contains the DCL interface definitions for
                   those functions which may be used by code which could be
                   built outside the Datalight build process.  This header
                   MUST not depend on any other headers which reside outside
                   the DCL include directory.  It is automatically included
                   by both dcl.h and dcl_osname.h.

    dlapiint.h   - This header contains the DCL interface definitions for
                   those functions which are used by one or more Datalight
                   products, built using the standard Datalight product
                   build process.  It is automatically include by dcl.h.

    dlapimap.h   - This header contains the DCL interface definitions for
                   those functions which, similar to dlapiint.h, are used
                   internally by Datalight code, however may get remapped
                   by OS specific code.  Typically these functions are C
                   library equivalents.  It is automatically include by
                   dcl.h.

    dlapiprv.h   - This header contains the DCL interface definitions for
                   those functions which are private to DCL, and are not
                   intended for use outside DCL, whether by Datalight code
                   or not.  Typically these functions include low-level
                   primitives which are wrapped by higher level functions
                   which should be used.

    A few discrete headers exist for specific subsystems which must be
    explicitly included if they are to be used (generally they are used
    only internally by DCL).
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dlapi.h $
    Revision 1.39  2010/12/08 19:16:14Z  billr
    Make dlapi.h and dlerrlev.h dual-license headers.
    Revision 1.38  2010/12/08 02:11:42Z  garyp
    Moved DCLSHELLHANDLE and DCLENVHANDLE from dlapi.h into dltypes.h.
    Revision 1.37  2010/10/31 15:20:38Z  garyp
    Updated some prototypes to use size_t where appropriate.
    Revision 1.36  2010/09/19 02:30:50Z  garyp
    Added DclTimerSamplePeriod().
    Revision 1.35  2010/07/31 00:01:23Z  garyp
    Updated to work around the "VA64BUG" issue.
    Revision 1.34  2010/07/15 01:07:01Z  garyp
    Added functionality to reset the profiler data.
    Revision 1.33  2010/04/17 22:29:53Z  garyp
    Updated the profiler summary command to support an "AdjustOverhead" flag.
    Revision 1.32  2009/12/31 17:24:18Z  billr
    Refactor headers for better separation between 0S-dependent and
    OS-independent code.
    Revision 1.31  2009/11/10 19:50:21Z  garyp
    Now use DCLCONF_HIGHRESTIMESTAMP to conditionall the High-Res timestamp
    stuff.
    Revision 1.30  2009/10/19 22:43:35Z  garyp
    Properly conditioned some code to build cleanly with output disabled.
    Revision 1.29  2009/10/09 02:12:34Z  garyp
    Included headers which are common to all ports -- rather than requiring
    that they are specified in dcl_osname.h.
    Revision 1.28  2009/10/06 17:34:02Z  garyp
    Moved some stuff from dlapiint.h into this header, which needs to be
    accessible to stuff built outside the DL build process.
    Revision 1.27  2009/08/07 21:53:19Z  garyp
    Added defaults for a few project configuration settings, to be used in the
    event that the code is being built outside the standard DL build process.
    Revision 1.26  2009/06/25 00:21:52Z  garyp
    Numerous updates pertaining to finalizing the merge from the v4.0 branch.
    Revision 1.25  2009/04/09 22:05:38Z  garyp
    Documentation updated -- no functional changes.
    Revision 1.24  2009/02/21 01:01:16Z  brandont
    Added DCLFUNC for DclEnv group of functions.
    Revision 1.23  2009/02/16 07:04:04Z  keithg
    Corrected comment reguarding timer resolution for DclTimePassed()
    Revision 1.22  2009/02/08 00:59:56Z  garyp
    Merged from the v4.0 branch.  Updated for new timestamp functionality.
    Updated for the modified profiler interface.  Added the D_BYTE data type.
    Added support for DclInputFunction() and DclInputChar().
    Revision 1.21  2008/12/31 21:36:31Z  keithg
    Changed D_BOOL definition to int.  Fixed size booleans are no
    longer necessary.
    Revision 1.20  2008/05/19 18:06:07Z  garyp
    Merged from the WinMobile branch.
    Revision 1.19.1.2  2008/05/19 18:06:07Z  garyp
    Added the DCLINIT64, DCLINITTS, and DCLINITHRTS definitions.
    Revision 1.19  2008/05/03 19:41:29Z  garyp
    Updated to build cleanly when output is disabled.
    Revision 1.18  2008/05/01 22:25:17Z  brandont
    Added the HEXDUMP_NORETURN define.
    Revision 1.17  2008/03/22 19:32:27Z  Garyp
    Minor prototype changes.
    Revision 1.16  2007/11/29 23:18:24Z  Glenns
    Modified DCLTimer structure and member functions to account for system
    timer granularity. See Bugzilla #1624
    Revision 1.15  2007/11/03 23:31:21Z  Garyp
    Added the standard module header.
    Revision 1.14  2007/10/14 16:17:58Z  Garyp
    Added the delay services prototypes.
    Revision 1.13  2007/10/06 01:09:59Z  brandont
    Added a function pointer typedef and an external function pointer
    declaration for DclLogClose.
    Revision 1.12  2007/05/15 02:09:06Z  garyp
    Added an environment manipulation interface.
    Revision 1.11  2007/04/06 02:00:21Z  Garyp
    Added the DclLogWrite() function.
    Revision 1.10  2007/01/13 03:32:57Z  Garyp
    Moved the emulated 64-bit type definition from dlenv.h into dlapi.h because
    the former is not included when building external code.
    Revision 1.9  2007/01/12 04:02:40Z  Garyp
    Added the DCLSHELLHANDLE opaque type so that it can be used by external
    applications not being built by the DL build process.
    Revision 1.8  2007/01/02 23:21:57Z  Garyp
    Added prototypes for new logging functions.
    Revision 1.7  2006/10/26 02:54:24Z  Garyp
    Updated so the high-res timestamp functions can be transparently
    substituted for the millisecond based timestamp functions.
    Revision 1.6  2006/06/22 19:18:30Z  Garyp
    Added high-res timestamp function prototypes.
    Revision 1.5  2006/03/04 17:14:17Z  Garyp
    Added DclProfContextName().
    Revision 1.4  2006/01/08 21:36:08Z  Garyp
    Added DclHexDump() support.
    Revision 1.3  2006/01/02 13:06:20Z  Garyp
    Refactored dlapi.h and moved internal DCL functions (those called only by
    other Datalight code) into dlapiint.h.
    Revision 1.2  2005/12/19 01:22:50Z  garyp
    Modified DclTraceToggle() to operate using the trace initial state, and
    to return the trace enable level.
    Revision 1.1  2005/12/06 15:16:28Z  Pauli
    Initial revision
    Revision 1.7  2005/12/06 15:16:27Z  Garyp
    Added minimal thread manipulation functions.
    Revision 1.6  2005/11/25 23:02:10Z  Garyp
    Added DclRand().
    Revision 1.5  2005/11/13 02:33:29Z  Garyp
    Updated the statistics interface functions to quietly return FALSE if the
    particular stat interface is disabled.
    Revision 1.4  2005/11/06 03:38:20Z  Garyp
    Added prototypes for displaying statistics.
    Revision 1.3  2005/10/12 03:40:20Z  pauli
    Added TraceToggle.
    Revision 1.2  2005/10/07 03:03:51Z  Garyp
    Added the file I/O OS services prototypes and mappings.
    Revision 1.1  2005/10/04 05:51:34Z  Garyp
    Initial revision
    Revision 1.11  2005/09/21 07:18:58Z  garyp
    Added prototypes.
    Revision 1.10  2005/09/20 19:09:40Z  pauli
    Added DclOsSleep.
    Revision 1.9  2005/09/15 00:12:57Z  garyp
    Added DclMemStatistics().
    Revision 1.8  2005/08/25 07:09:52Z  Garyp
    Minor cleanup -- no functional changes.
    Revision 1.7  2005/08/17 09:22:47Z  garyp
    Fixed a duplicated declaration.
    Revision 1.6  2005/08/15 06:54:02Z  garyp
    Added DclProductVersion().
    Revision 1.5  2005/08/04 17:05:06Z  Garyp
    Moved prototypes for the redefinable functions into dlapimap.h.
    Revision 1.4  2005/08/03 18:03:40Z  pauli
    Replaced DCL_FuncName with DclFuncName.
    Revision 1.3  2005/07/31 02:44:17Z  Garyp
    Added prototypes.
    Revision 1.2  2005/07/31 00:38:11Z  Garyp
    Added support for DclUllToA().
    Revision 1.1  2005/07/18 09:25:06Z  pauli
    Initial revision
    Revision 1.7  2005/06/20 20:04:46Z  Pauli
    Included dl64bit.h for 64-bit operations.
    Revision 1.6  2005/06/17 21:40:47Z  PaulI
    Added prototype for DclOsGetChar.
    Revision 1.5  2005/06/15 02:42:32Z  PaulI
    Added condition to avoid including this header multiple times.
    Revision 1.4  2005/06/12 22:52:40Z  PaulI
    Added product numbers.  Defined DCLFUNC based on product being built.
    Revision 1.3  2005/04/16 23:14:11Z  PaulI
    XP Merge
    Revision 1.2.1.5  2005/04/16 23:14:11Z  GaryP
    Made the definition of TRUE and FALSE conditional.
    Revision 1.2.1.4  2005/04/16 10:24:46Z  PaulI
    Prototype update.
    Revision 1.2.1.3  2005/04/12 20:23:44Z  garyp
    Prototype updates.
    Revision 1.2.1.2  2005/04/10 22:30:11Z  garyp
    Added bunches of new stuff to support the newly implemented functions.
    Revision 1.2  2005/02/14 01:39:24Z  GaryP
    Updated to include dltlset.h and dltypes.h.
    Revision 1.1  2005/01/10 08:18:02Z  GaryP
    Initial revision
---------------------------------------------------------------------------*/

#ifndef DLAPI_H_INCLUDED
#define DLAPI_H_INCLUDED

#ifndef VAARG64_BROKEN
  #define VAARG64_BROKEN      (FALSE)
#endif

#if VAARG64_BROKEN
  #define VA64BUG(ull)    (&ull)
#else
  #define VA64BUG(ull)    (ull)
#endif


/*-------------------------------------------------------------------
    These checks ONLY exist to resolve the problem that externally
    built programs will not have the project definitions of these
    settings.  Define these appropriately and let any issues be
    caught at run-time.

    Conversely, internally built Datalight code will have the project
    definitions, which is highly preferable, and allows more accurate
    compile-time scaling of the product.
-------------------------------------------------------------------*/
#ifndef DCLCONF_OUTPUT_ENABLED
#define DCLCONF_OUTPUT_ENABLED      TRUE
#endif
#ifndef DCLCONF_HIGHRESTIMESTAMP
#define DCLCONF_HIGHRESTIMESTAMP    FALSE
#endif
#ifndef DCL_OSFEATURE_THREADS
#define DCL_OSFEATURE_THREADS       TRUE
#endif


/*-------------------------------------------------------------------
    D_UINT64 (Emulated)

    If the system does not support native 64-bit types, emulate
    one using 2 32-bit values.

    This structure layout is defined such that an emulated 64-bit
    type will (likely) appear in memory as would a native 64-bit
    type in the specified byte order.  This is done purely for
    convenience reasons, as well as simplifying the byte-order
    conversion macros.  The C compiler may choose to do its own
    thing so this cannot be relied upon.
-------------------------------------------------------------------*/
#if DCL_NATIVE_64BIT_SUPPORT
    #define DCLINIT64MIN    D_UINT64_MIN
    #define DCLINIT64MAX    D_UINT64_MAX
#else
    typedef struct
    {
      #if DCL_BIG_ENDIAN
        D_UINT32        ulLowDword;
        D_UINT32        ulHighDword;
      #else
        D_UINT32        ulHighDword;
        D_UINT32        ulLowDword;
      #endif
    } D_UINT64;

    #define DCLINIT64MIN    {0,0}
    #define DCLINIT64MAX    {0xFFFFFFFF,0xFFFFFFFF}
#endif


/*-------------------------------------------------------------------
    Type: DCLTIMER

    A DCLTIMER structure is used by the timer services to track
    timing information.  This structure is defined in a non-opaque
    fashion to allow it to be declared on the caller's stack to
    allow thread-safe implementations without requiring explicit
    memory allocations.  However the contents of this structure are
    to be considered private to the timer functions and must not be
    manipulated by timer client code.
-------------------------------------------------------------------*/
typedef struct
{
    D_UINT32    ulElapsed;
    D_UINT32    ulLastTick;
    D_UINT32    ulEndTicks;
    D_UINT16    uGranularity;
    D_BOOL      fExpired;
} DCLTIMER;

#define         DclTimerServiceInit         DCLFUNC(DclTimerServiceInit)
#define         DclTimerSet                 DCLFUNC(DclTimerSet)
#define         DclTimerExpired             DCLFUNC(DclTimerExpired)
#define         DclTimerRemaining           DCLFUNC(DclTimerRemaining)
#define         DclTimerElapsed             DCLFUNC(DclTimerElapsed)
#define         DclTimerSamplePeriod        DCLFUNC(DclTimerSamplePeriod)

DCLSTATUS       DclTimerServiceInit(DCLINSTANCEHANDLE hDclInst);
DCLSTATUS       DclTimerSet(      DCLTIMER *pT, D_UINT32 ulMillisec);
D_BOOL          DclTimerExpired(  DCLTIMER *pT);
D_UINT32        DclTimerRemaining(DCLTIMER *pT);
D_UINT32        DclTimerElapsed(  DCLTIMER *pT);
D_UINT32        DclTimerSamplePeriod(void);


/*-------------------------------------------------------------------
    Timestamp Functions

    Note that the Timestamp services are constructed so that
    the high-res timers can be transparently substituted for
    the standard-res timers, without calling code knowing the
    difference.

    Type: DCLTIMESTAMP

    A DCLTIMESTAMP type is used to track timer information.  This
    type must be operated on solely by the timer functions --
    software should not attempt to interpret the meaning of this
    type.

    Type: DCLHRTIMESTAMP

    A DCLHRTIMESTAMP type is used to track high-res timer infor-
    mation.  This type must be operated on solely by the high-res
    timer functions -- software should not attempt to interpret the
    meaning of this type.
-------------------------------------------------------------------*/
#if DCLCONF_HIGHRESTIMESTAMP
  /*  Map the regular millisecond based timestamp functions onto
      the high-res timestamp functions.
  */
  typedef D_UINT64          DCLHRTIMESTAMP;
  typedef DCLHRTIMESTAMP    DCLTIMESTAMP;
  #define       DCLINITHRTS DCLINIT64MIN

  #define       DclHighResTimeServiceInit   DCLFUNC(DclHighResTimeServiceInit)
  #define       DclHighResTimeStamp         DCLFUNC(DclHighResTimeStamp)
  #define       DclHighResTimePassed        DCLFUNC(DclHighResTimePassed)
  #define       DclHighResTimePassedMS      DCLFUNC(DclHighResTimePassedMS)
  #define       DclHighResTimePassedUS      DCLFUNC(DclHighResTimePassedUS)

  #define       DclTimeStamp                DCLFUNC(DclHighResTimeStamp)
  #define       DclTimePassed               DCLFUNC(DclHighResTimePassedMS)
  #define       DclTimePassedUS             DCLFUNC(DclHighResTimePassedUS)
  #define       DclTimePassedNS             DCLFUNC(DclHighResTimePassed)
  #define       DCLINITTS   DCLINITHRTS

  DCLSTATUS     DclHighResTimeServiceInit(DCLINSTANCEHANDLE hDclInst);
  DCLTIMESTAMP  DclHighResTimeStamp(void);
  D_UINT64      DclHighResTimePassed(  DCLTIMESTAMP tTimeStamp);/* nanoseconds */
  D_UINT32      DclHighResTimePassedMS(DCLTIMESTAMP tTimeStamp);
  D_UINT32      DclHighResTimePassedUS(DCLTIMESTAMP tTimeStamp);

#else

  typedef D_UINT32          DCLTIMESTAMP;
  #define       DclTimeStamp                DCLFUNC(DclTimeStamp)
  #define       DclTimePassed               DCLFUNC(DclTimePassed)
  #define       DclTimePassedUS             DCLFUNC(DclTimePassedUS)
  #define       DclTimePassedNS             DCLFUNC(DclTimePassedNS)
  #define       DCLINITTS   0

  DCLTIMESTAMP  DclTimeStamp(void);
  D_UINT32      DclTimePassed(  DCLTIMESTAMP tTimeStamp);       /* milliseconds */
  D_UINT32      DclTimePassedUS(DCLTIMESTAMP tTimeStamp);
  D_UINT64      DclTimePassedNS(DCLTIMESTAMP tTimeStamp);
#endif


/*-------------------------------------------------------------------
    Delay Functions
-------------------------------------------------------------------*/
#define         DclNanosecondDelay          DCLFUNC(DclNanosecondDelay)
#define         DclNanosecondDelayMinimum   DCLFUNC(DclNanosecondDelayMinimum)

DCLSTATUS       DclNanosecondDelay(D_UINT32 ulNanoseconds);
D_UINT32        DclNanosecondDelayMinimum(void);


/*-------------------------------------------------------------------
    Input/Output Functions
-------------------------------------------------------------------*/
typedef DCLSTATUS (*PFNDCLINPUTCHAR)(void *pContext, D_UCHAR *puChar, unsigned nFlags);
typedef void      (*PFNDCLOUTPUTSTRING)(void *pContext, const char *pszString);

typedef struct sDCLINPUTINFO
{
    void                   *pContext;
    PFNDCLINPUTCHAR         pfnInputChar;
} DCLINPUTINFO;

typedef struct sDCLOUTPUTINFO
{
    void                   *pContext;
    PFNDCLOUTPUTSTRING      pfnOutputString;
} DCLOUTPUTINFO;

#define         DclInputServiceInit     DCLFUNC(DclInputServiceInit)
#define         DclInputChar            DCLFUNC(DclInputChar)
#define         DclOutputServiceInit    DCLFUNC(DclOutputServiceInit)
#define         DclOutputString         DCLFUNC(DclOutputString)

DCLSTATUS       DclInputServiceInit(DCLINSTANCEHANDLE hDclInst);
DCLSTATUS       DclInputChar(DCLINSTANCEHANDLE hDclInst, D_UCHAR *puChar, unsigned nFlags);
DCLSTATUS       DclOutputServiceInit(DCLINSTANCEHANDLE hDclInst);
DCLSTATUS       DclOutputString(DCLINSTANCEHANDLE hDclInst, const char *pszStr);


/*-------------------------------------------------------------------
    Environment Manipulation Functions

    Type: DCLENVHANDLE

    A DCLENVHANDLE is an opaque pointer which is used to reference
    an "environment" object which is created with DclEnvCreate().
-------------------------------------------------------------------*/
#define         DclEnvCreate            DCLFUNC(DclEnvCreate)
#define         DclEnvDestroy           DCLFUNC(DclEnvDestroy)
#define         DclEnvDisplay           DCLFUNC(DclEnvDisplay)
#define         DclEnvSubstitute        DCLFUNC(DclEnvSubstitute)
#define         DclEnvVarAdd            DCLFUNC(DclEnvVarAdd)
#define         DclEnvVarDelete         DCLFUNC(DclEnvVarDelete)
#define         DclEnvVarLookup         DCLFUNC(DclEnvVarLookup)

DCLENVHANDLE    DclEnvCreate(size_t nEnvLen, const char *pszImport);
DCLSTATUS       DclEnvDestroy(   DCLENVHANDLE hEnv);
DCLSTATUS       DclEnvDisplay(   DCLENVHANDLE hEnv, const char *pszMatch);
DCLSTATUS       DclEnvSubstitute(DCLENVHANDLE hEnv, char *pszBuffer, size_t nMaxLen);
DCLSTATUS       DclEnvVarAdd(    DCLENVHANDLE hEnv, const char *pszVar, const char *pszValue);
size_t          DclEnvVarDelete( DCLENVHANDLE hEnv, const char *pszVar);
const char *    DclEnvVarLookup( DCLENVHANDLE hEnv, const char *pszVar);


/*-------------------------------------------------------------------
    Trace and Debugging Functions
-------------------------------------------------------------------*/
#define HEXDUMP_UINT8       (0x0001)
#define HEXDUMP_UINT16      (0x0002)
#define HEXDUMP_UINT32      (0x0004)
/* #define HEXDUMP_UINT64   (0x0008) Future use */
#define HEXDUMP_SIZEMASK    (0x00FF)
#define HEXDUMP_NOOFFSET    (0x0100)
#define HEXDUMP_NOASCII     (0x0200)
#define HEXDUMP_NORETURN    (0x0400)

#if D_DEBUG && DCLCONF_OUTPUT_ENABLED
#define         DclTraceToggle          DCLFUNC(DclTraceToggle)
int             DclTraceToggle(void);
#endif

#if DCLCONF_OUTPUT_ENABLED
  #define       DclHexDump              DCLFUNC(DclHexDump)
  D_BOOL        DclHexDump(const char *pszTitle, unsigned fOptions, unsigned nWidth, D_UINT32 ulCount, const void *pData);
#else
  #define       DclHexDump              (void)
#endif

#if DCLCONF_OUTPUT_ENABLED
  #define       DclPrintf               DCLFUNC(DclPrintf)
  int           DclPrintf(const char *pszFmt, ...);
#else
  #define       DclPrintf               (void)
#endif


/*-------------------------------------------------------------------
    Profiler Interface
-------------------------------------------------------------------*/
#define         DclProfEnable           DCLFUNC(DclProfEnable)
#define         DclProfDisable          DCLFUNC(DclProfDisable)
#define         DclProfReset            DCLFUNC(DclProfReset)
#define         DclProfSummary          DCLFUNC(DclProfSummary)
#define         DclProfToggle           DCLFUNC(DclProfToggle)

DCLSTATUS       DclProfEnable(void);
DCLSTATUS       DclProfDisable(void);
DCLSTATUS       DclProfReset(void);
DCLSTATUS       DclProfSummary(D_BOOL fReset, D_BOOL fShort, D_BOOL fAdjustOverhead);
DCLSTATUS       DclProfToggle(D_BOOL fSummaryReset);


/*-------------------------------------------------------------------
    Miscellaneous Functions 
-------------------------------------------------------------------*/
#define         DclArgClear             DCLFUNC(DclArgClear)
#define         DclArgCount             DCLFUNC(DclArgCount)
#define         DclArgRetrieve          DCLFUNC(DclArgRetrieve)

D_BOOL          DclArgClear(char *pszArgString, D_UINT16 uArgNum);
D_UINT16        DclArgCount(const char *pszArgString);
D_BOOL          DclArgRetrieve(const char *pszArgString, D_UINT16 uArgNum, D_UINT16 uBuffLen, char *pachBuffer);


/*-------------------------------------------------------------------
    Include DCL headers which provide access to the portions of
    the publicly exposed DCL API which are not OS-specific.

    Note the use of quotes rather than angle brackets.  Because this
    code may be built outside the Datalight build process, this
    header may only include Datalight headers which can be located
    relative to the current directory in which this header resides.
-------------------------------------------------------------------*/
#include "dlstatus.h"
#include "dliosys.h"
#include "dltools.h"
#include "dlerrlev.h"


#endif  /* DLAPI_H_INCLUDED */

