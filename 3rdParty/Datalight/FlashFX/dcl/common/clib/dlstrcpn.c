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
    $Log: dlstrcpn.c $
    Revision 1.7  2009/06/27 20:35:37Z  garyp
    Updated to use new asserts.
    Revision 1.6  2007/12/18 20:12:08Z  brandont
    Updated function headers.
    Revision 1.5  2007/11/03 23:31:10Z  Garyp
    Added the standard module header.
    Revision 1.4  2007/04/06 21:29:30Z  Garyp
    Added asserts.
    Revision 1.3  2006/09/16 19:26:52Z  Garyp
    Minor fix to accommodate the fact that size_t is always unsigned.
    Documentation fixes.
    Revision 1.2  2006/02/03 03:01:24Z  brandont
    Changed the length argument type to size_t.
    Revision 1.1  2005/10/02 03:57:10Z  Pauli
    Initial revision
    Revision 1.3  2005/08/04 16:52:38Z  Garyp
    Modified so that the DCL functions compile with their default names even
    if the OS or project code has redefined the macros to use an alternate
    implementation.
    Revision 1.2  2005/08/03 17:32:14Z  pauli
    Replaced DCL_FuncName with DclFuncName.
    Revision 1.1  2005/07/04 01:16:14Z  pauli
    Initial revision
---------------------------------------------------------------------------*/

#include <dcl.h>

/*  Redefine this to the default value just in case it was defined
    in dlosconf.h.  If that is the case, this function will still
    compile as expected, it just won't be called.
*/
#undef  DclStrNCpy
#define DclStrNCpy  DCLFUNC(DclStrNCpy)


/*-------------------------------------------------------------------
    Public: DclStrNCpy()

    This function copies up to N characters of a null-terminated
    string to a destination buffer.  The result will not be
    null-terminated if the source string is longer than N-1
    characters.

    If the source string is shorter than N-1 characters, the
    remainder of the destination buffer will be filled with
    NULLs.

    Parameters:
        pszDst - A pointer to the destination buffer
        pszSrc - A pointer to the null-terminated string to copy
        nLen   - The maximum number of characters to copy

    Return Value:
        Returns the original pszDst value.
-------------------------------------------------------------------*/
char * DclStrNCpy(
    char           *pszDst,
    const char     *pszSrc,
    size_t          nLen)
{
    char           *pszPos = pszDst;

    DclAssertReadPtr(pszSrc, 0);
    DclAssertWritePtr(pszDst, nLen);

    while(nLen)
    {
        nLen--;

        *pszPos = *pszSrc;

        if(!(*pszPos))
        {
            /*  Fill any remainder of the buffer to NULLs
            */
            while(nLen)
            {
                *pszPos++ = 0;
                nLen--;
            }

            break;
        }

        pszPos++;
        pszSrc++;
    }

    return pszDst;
}


