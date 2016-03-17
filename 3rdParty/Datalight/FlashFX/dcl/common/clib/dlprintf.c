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
  jurisdictions.  Patents may be pending.

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

    The functions in this module are used internally by Datalight products.
    Typically these functions are used rather than the similarly named,
    general Standard C library functions.  Typically this is done for
    compatibility, portability, and code size reasons.

    These functions are NOT intended to be complete, 100% ANSI C compatible
    implementations, but rather are designed address specific needs.

    See the specific function headers for more information about departures
    from the ANSI standard, and missing or extended functionality.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dlprintf.c $
    Revision 1.13  2009/11/02 17:52:47Z  garyp
    Updated DclVPrintf() to return 0 if DclOutputString() failed.
    Revision 1.12  2009/06/24 21:59:54Z  garyp
    Updated to use new asserts.
    Revision 1.11  2009/05/21 00:10:27Z  garyp
    Documentation updated -- no functional changes.
    Revision 1.10  2009/04/09 22:05:37Z  garyp
    Documentation updated -- no functional changes.
    Revision 1.9  2009/02/08 00:14:04Z  garyp
    Merged from the v4.0 branch.  Updated DclOutputString() to take an extra
    parameter.
    Revision 1.8  2008/05/03 19:53:57Z  garyp
    Updated to build cleanly when output is disabled.
    Revision 1.7  2007/12/18 19:22:38Z  brandont
    Updated function headers.
    Revision 1.6  2007/11/03 23:31:10Z  Garyp
    Added the standard module header.
    Revision 1.5  2007/03/06 03:46:51Z  Garyp
    Updated to use a renamed setting.
    Revision 1.4  2006/10/03 21:13:57Z  Garyp
    Updated to allow output to be entirely disabled (and the code not pulled
    into the image).
    Revision 1.3  2006/08/18 18:33:57Z  Garyp
    Modified to quiely handle truncated output, and preserve \n termination
    if the output is trucated.
    Revision 1.2  2006/01/04 02:05:46Z  Garyp
    Updated to use some renamed settings.
    Revision 1.1  2005/10/02 03:57:06Z  Pauli
    Initial revision
    Revision 1.2  2005/06/13 19:34:38Z  PaulI
    Modified to be snprintf() and vsnprintf() compatible and use counted
    buffers to avoid buffer overflow.
    Revision 1.1  2005/04/10 19:56:20Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <stdarg.h>

#include <dcl.h>

#if DCLCONF_OUTPUT_ENABLED

#include <dlprintf.h>

/*-------------------------------------------------------------------
    Public: DclVPrintf()

    Print formatted data using a pointer to a variable length
    argument list.

    This function provides a subset of the ANSI C vprintf()
    functionality with several extensions to support fixed
    size data types.

    See DclVSNPrintf() for the list of supported types.

    This function accommodates a maximum output length of
    DCLOUTPUT_BUFFERLEN.  If this function must truncate the
    output, and the original string was \n terminated, the
    truncated output will be \n terminated as well.

    Parameters:
        pszFmt  - A pointer to the null-terminated format string
        arglist - The variable length argument list

    Return Value:
        Returns an int containing the length processed, or zero if
        an error occurred.  Running out of buffer space is not
        considered an error -- the output is simply truncated and
        the length which was processed is returned.
-------------------------------------------------------------------*/
int DclVPrintf(
    const char     *pszFmt,
    va_list         arglist)
{
    int             nLen;
    char            achBuffer[DCLOUTPUT_BUFFERLEN];

    DclAssertReadPtr(pszFmt, 0);

    nLen = DclVSNPrintf(achBuffer, sizeof(achBuffer), pszFmt, arglist);

    DclAssert(nLen < DCLOUTPUT_BUFFERLEN);

    if(nLen < 0)
    {
        /*  DclVSNPrintf() will return -1 if we ran out of buffer
            space.  If this happens truncate the data.
        */

        /*  Return the full buffer length, minus the null-terminator
        */
        nLen = sizeof(achBuffer) - 1;

        /*  Ensure we are null terminated.
        */
        achBuffer[nLen] = 0;

        /*  If the original string was \n terminated and the new one is
            not, due to truncation, stuff a \n into the new one.
        */
        if(pszFmt[DclStrLen(pszFmt)-1] == '\n')
            achBuffer[nLen-1] = '\n';
    }

    if(DclOutputString(NULL, achBuffer) == DCLSTAT_SUCCESS)
        return nLen;
    else
        return 0;
}


/*-------------------------------------------------------------------
    Public: DclPrintf()

    Print formatted data with a variable length argument list.

    This function provides a subset of the ANSI C printf()
    functionality with several extensions to support fixed
    size data types.

    See DclVSNPrintf() for the list of supported types.

    Parameters:
        pszFmt  - A pointer to the null-terminated format string
        ...     - The variable length argument list

    Return Value:
        Returns an int containing the length processed.
-------------------------------------------------------------------*/
int DclPrintf(
    const char     *pszFmt,
    ...)
{
    int             nLen;
    va_list         arglist;

    DclAssertReadPtr(pszFmt, 0);

    va_start(arglist, pszFmt);

    nLen = DclVPrintf(pszFmt, arglist);

    va_end(arglist);

    return nLen;
}

#endif

