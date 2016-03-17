/*---------------------------------------------------------------------------
               ----> DO NOT REMOVE THE FOLLOWING NOTICE <----

                  Copyright (c) 1993 - 2011 Datalight, Inc.
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
    $Log: dlhtoul.c $
    Revision 1.8  2011/12/07 18:43:45Z  daniel.lewis
    Moved the hex digit macros into dlmacros.h.
    Revision 1.7  2011/09/18 21:11:37Z  garyp
    Updated documentation and debug code -- no functional changes.
    Revision 1.6  2009/06/25 19:22:44Z  garyp
    Updated to use new asserts.
    Revision 1.5  2009/04/09 14:13:16Z  davidh
    Function headers updated for AutoDoc.
    Revision 1.4  2007/12/18 03:33:31Z  brandont
    Updated function headers.
    Revision 1.3  2007/11/03 23:31:09Z  Garyp
    Added the standard module header.
    Revision 1.2  2006/11/30 01:19:10Z  Garyp
    Changed the calling convention to return a pointer to the character
    following the converted number.
    Revision 1.1  2006/10/05 23:58:24Z  Garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <dcl.h>


/*-------------------------------------------------------------------
    Public: DclHtoUL()

    Convert a hexadecimal ASCII number into a D_UINT32 value.  The
    function processes all hex digits up to a NUL-terminator, or
    to the first non-hex character.  Only hexadecimal digits are
    processed, so leading white space, or a leading "0x" prefix are
    not allowed.

    If pachNum points to an empty string (points to a NUL), this
    function will return NULL, and the value at *pulNum will not
    be modified.

    *Warning* -- This function does not check for overflow.  If there
                 are more significant digits than can be represented
                 in a D_UINT32 variable, the output is unspecified.

    Parameters:
        pachNum - A pointer to a constant array of hex characters.
        pulNum  - A pointer to the location in which to store the
                  D_UINT32 result.  Upon return, this value will be
                  modified ONLY if the function succeeds and the
                  returned pointer is valid (not NULL).

    Return Value:
        Returns a pointer to the byte following the converted number,
        or NULL to indicate failure.
-------------------------------------------------------------------*/
const char * DclHtoUL(
    const char *pachNum,
    D_UINT32   *pulNum)
{
    D_UINT32    ulValue = 0;
    const char *pReturn = NULL;

    DclAssertReadPtr(pachNum, 0);
    DclAssertWritePtr(pulNum, sizeof(*pulNum));

    while(*pachNum)
    {
        char    cDigit = *pachNum;

        if(DclIsDigit(cDigit))
            cDigit -= '0';
        else if(DCLISHEXDIGITU(cDigit))
            cDigit -= ('A' - 10);
        else if(DCLISHEXDIGITL(cDigit))
            cDigit -= ('a' - 10);
        else
            break;

        DclAssert((ulValue & 0xF0000000) == 0);

        ulValue <<= 4;
        ulValue += cDigit;

        pachNum++;
        pReturn = pachNum;
    }

    /*  Modify the number returned only if we found one or more valid
        hex digits.
    */
    if(pReturn)
        *pulNum = ulValue;

    return pReturn;
}


