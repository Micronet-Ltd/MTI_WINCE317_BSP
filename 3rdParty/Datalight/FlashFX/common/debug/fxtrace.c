/*---------------------------------------------------------------------------
               ----> DO NOT REMOVE THE FOLLOWING NOTICE <----

                  Copyright (c) 1993 - 2009 Datalight, Inc.
                       All Rights Reserved Worldwide.

  Datalight, Incorporated is a Washington State corporation located at:

        21520 30th Dr SE, Suite 110,      Tel:  425-951-8086
        Bothell, WA  98021                Fax:  425-951-8094
        USA                               Web:  http://www.datalight.com

  This software, including without limitation all source code and documen-
  tation, is the confidential, trade secret property of Datalight, Inc.
  and is protected under the copyright laws of the United States and other
  jurisdictions.  The software may be subject to one or more of these US
  patents: US#5860082, US#6260156.  Patents may be pending.

  In addition to civil penalties for infringement of copyright under appli-
  cable U.S. law, 17 U.S.C. 1204 provides criminal penalties for violation
  of (a) the restrictions on circumvention of copyright protection systems
  found in 17 U.S.C. 1201 and (b) the protections for the integrity of
  copyright management information found in 17 U.S.C. 1202.

  U.S. Government Restricted Rights:  Use, duplication, reproduction, or
  transfer of this commercial product and accompanying documentation is
  restricted in accordance with FAR 12.212 and DFARS 227.7202 and by a
  License Agreement.

  IN ADDITION TO COPYRIGHT AND PATENT LAW, THIS SOFTWARE IS PROTECTED UNDER
  A SOURCE CODE AGREEMENT, NON-DISCLOSURE AGREEMENT, AND/OR SIMILAR BINDING
  CONTRACT BETWEEN DATALIGHT, INC. AND THE LICENSEE ("BINDING AGREEMENTS").
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

    This module contains functions used for managing the FlashFX interface
    to the DCL trace mechanisms.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: fxtrace.c $
    Revision 1.7  2009/03/31 18:08:21Z  davidh
    Function headers updated for AutoDoc.
    Revision 1.6  2008/04/28 13:58:37Z  garyp
    Updated to build cleanly when output is disabled.
    Revision 1.5  2008/01/13 07:26:18Z  keithg
    Function header updates to support autodoc.
    Revision 1.4  2007/11/03 23:49:30Z  Garyp
    Updated to use the standard module header.
    Revision 1.3  2007/09/08 17:18:35Z  Garyp
    Modified so the trace related functions are compiled and exposed even when
    building in release mode, though they would not normally get linked in.
    Revision 1.2  2007/03/18 17:55:43Z  Garyp
    Added FfxTraceMaskGet() and FfxTraceMaskSwap().  Now store the trace mask
    in a static global so that it can be modified on the fly in a debugger
    environment.
    Revision 1.1  2005/10/22 06:00:20Z  Pauli
    Initial revision
    Revision 1.2  2005/10/22 07:00:19Z  garyp
    Minor symbol renaming exercise.
    Revision 1.1  2005/08/03 19:03:02Z  Garyp
    Initial revision
    Revision 1.2  2005/08/03 19:03:02Z  pauli
    Replaced DCL_FuncName with DclFuncName.
    Revision 1.1  2005/07/07 02:26:28Z  pauli
    Initial revision
---------------------------------------------------------------------------*/

#include <stdarg.h>

#include <flashfx.h>

#if D_DEBUG && DCLCONF_OUTPUT_ENABLED

#include <dlprintf.h>

static D_UINT32 ulFfxTraceMask = FFX_TRACEMASK;

/*  The ulFfxTraceMask is declared as such so that in a debugger
    environment, it may be interactively patched if need be.
*/

/*-------------------------------------------------------------------
    Public: FfxDebugPrintf()

    This function displays debug output with a number of
    formatting options.  See fxtrace.h for more information.

    This function provides a subset of the ANSI C printf()
    functionality with several extensions to support fixed
    size data types.

    See DclVSprintf() for the list of supported types.

    Parameters:
        pszFmt - A pointer to the null-terminated format string
        ...    - The variable length argument list

    Return Value:
        Returns the length processed.
-------------------------------------------------------------------*/
int FfxDebugPrintf(
    const char     *pszFmt,
    ...)
{
    va_list         arglist;
    int             nReturn;

    va_start(arglist, pszFmt);

    /*  We use a debug level of '0' here because the macro that
        invoked us has already qualified the debug level.  Using
        '0' ensures that the output will always be displayed.
    */
    nReturn = DclTraceVPrintf(MAKETRACEFLAGS(TRACEALWAYS, 0, 0),
        FFXTRACEBIT_ALL, pszFmt, arglist);

    va_end(arglist);

    return nReturn;
}


/*-------------------------------------------------------------------
    Public: FfxTracePrintf()

    This function displays debug output with a number of
    formatting options.  See dltrace.h for more information.

    This function provides a subset of the ANSI C printf()
    functionality with several extensions to support fixed
    size data types.

    See DclVSprintf() for the list of supported types.

    Parameters:
        ulFlags - The trace flags
        pszFmt  - A pointer to the null-terminated format string
        ...     - The variable length argument list

    Return Value:
        Returns the length processed.
-------------------------------------------------------------------*/
int FfxTracePrintf(
    D_UINT32        ulFlags,
    const char     *pszFmt,
    ...)
{
    va_list         arglist;
    int             nReturn;

    va_start(arglist, pszFmt);

    nReturn = DclTraceVPrintf(ulFlags, ulFfxTraceMask, pszFmt, arglist);

    va_end(arglist);

    return nReturn;
}


/*-------------------------------------------------------------------
    Public: FfxTraceMaskGet()

    This function returns the current FlashFX trace mask value.

    Parameters:
        None.

    Return Value:
        Returns the current FlashFX trace mask value.
-------------------------------------------------------------------*/
D_UINT32 FfxTraceMaskGet(void)
{
    return ulFfxTraceMask;
}


/*-------------------------------------------------------------------
    Public: FfxTraceMaskSwap()

    This function swaps the FlashFX trace mask value -- setting a
    new mask and returning the original.

    Parameters:
        The new FlashFX trace mask value.

    Return Value:
        Returns the old FlashFX trace mask value.
-------------------------------------------------------------------*/
D_UINT32 FfxTraceMaskSwap(
    D_UINT32    ulNewMask)
{
    D_UINT32    ulOldMask = ulFfxTraceMask;

    ulFfxTraceMask = ulNewMask;

    return ulOldMask;
}

#endif  /* D_DEBUG && DCLCONF_OUTPUT_ENABLED */

