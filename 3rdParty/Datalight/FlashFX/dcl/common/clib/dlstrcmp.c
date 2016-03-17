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
    compatibility, portability, and/or code size reasons.

    These functions are NOT intended to be complete, 100% ANSI C compatible
    implementations, but rather are designed address specific needs.

    See the specific function headers for more information about departures
    from the ANSI standard, and missing or extended functionality.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dlstrcmp.c $
    Revision 1.7  2009/06/25 20:09:37Z  garyp
    Updated to use new asserts.
    Revision 1.6  2009/05/21 04:04:13Z  garyp
    Documentation updated -- no functional changes.
    Revision 1.5  2007/12/18 19:24:44Z  brandont
    Updated function headers.
    Revision 1.4  2007/11/03 23:31:10Z  Garyp
    Added the standard module header.
    Revision 1.3  2006/05/28 19:30:02Z  Garyp
    Added DclStrCmp().  Fixed nLen check in DclStrNICmp() since size_t is
    always unsigned.
    Revision 1.2  2006/02/03 03:01:45Z  brandont
    Changed the length argument type to size_t.  Correct handling of a
    negative length.
    Revision 1.1  2005/10/02 03:57:10Z  Pauli
    Initial revision
    Revision 1.3  2005/08/04 16:52:39Z  Garyp
    Modified so that the DCL functions compile with their default names even
    if the OS or project code has redefined the macros to use an alternate
    implementation.
    Revision 1.2  2005/08/03 19:17:46Z  pauli
    Replaced DCL_FuncName with DclFuncName.
    Revision 1.1  2005/07/03 23:56:52Z  pauli
    Initial revision
---------------------------------------------------------------------------*/

#include <dcl.h>

/*  Redefine these values to the default value just in case it was
    defined in dlosconf.h.  If that is the case, these functions will
    still compile as expected, they just won't be called.
*/
#undef  DclStrCmp
#define DclStrCmp   DCLFUNC(DclStrCmp)
#undef  DclStrICmp
#define DclStrICmp  DCLFUNC(DclStrICmp)
#undef  DclStrNICmp
#define DclStrNICmp DCLFUNC(DclStrNICmp)


/*-------------------------------------------------------------------
    Public: DclStrCmp()

    Perform a case sensitive string compare.

    Parameters:
        pszStr1 - A pointer to null-terminated string 1
        pszStr2 - A pointer to null-terminated string 2

    Return Value:
        Returns 0 if the strings are equal.  Returns 1 if string 1 is
        greater than string 2.  Return -1 if string 2 is greater than
        string 1.
-------------------------------------------------------------------*/
int DclStrCmp(
    const char     *psz1,
    const char     *psz2)
{
    unsigned char   chr1;
    unsigned char   chr2;

    DclAssertReadPtr(psz1, 0);
    DclAssertReadPtr(psz2, 0);

    while(TRUE)
    {
        chr1 = *psz1;
        chr2 = *psz2;

        if(chr1 > chr2)
            return 1;

        if(chr1 < chr2)
            return -1;

        if(!chr1)
            return 0;

        psz1++;
        psz2++;
    }
}


/*-------------------------------------------------------------------
    Public: DclStrICmp()

    Perform a case-insensitive string compare.

    Parameters:
        pszStr1 - A pointer to null-terminated string 1
        pszStr2 - A pointer to null-terminated string 2

    Return Value:
        Returns 0 if the strings are equal.  Returns 1 if string 1 is
        greater than string 2.  Return -1 if string 2 is greater than
        string 1.
-------------------------------------------------------------------*/
int DclStrICmp(
    const char     *psz1,
    const char     *psz2)
{
    unsigned char   chr1;
    unsigned char   chr2;

    DclAssertReadPtr(psz1, 0);
    DclAssertReadPtr(psz2, 0);

    while(TRUE)
    {
        chr1 = *psz1;
        chr2 = *psz2;

        if(chr1 >= 'A' && chr1 <= 'Z')
            chr1 += 'a' - 'A';

        if(chr2 >= 'A' && chr2 <= 'Z')
            chr2 += 'a' - 'A';

        if(chr1 > chr2)
            return 1;

        if(chr1 < chr2)
            return -1;

        if(!chr1)
            return 0;

        psz1++;
        psz2++;
    }
}


/*-------------------------------------------------------------------
    Public: DclStrNICmp()

    Perform a case-insensitive comparison of the first N characters
    in a string.

    Parameters:
        pszStr1 - A pointer to null-terminated string 1
        pszStr2 - A pointer to null-terminated string 2
        nLen    - The maximum length to compare

    Return Value:
        Returns 0 if the strings are equal.  Returns 1 if string 1 is
        greater than string 2.  Return -1 if string 2 is greater than
        string 1.
-------------------------------------------------------------------*/
int DclStrNICmp(
    const char     *psz1,
    const char     *psz2,
    size_t          nLen)
{
    unsigned char   chr1;
    unsigned char   chr2;

    DclAssertReadPtr(psz1, nLen);
    DclAssertReadPtr(psz2, nLen);

    while(nLen)
    {
        chr1 = *psz1;
        chr2 = *psz2;

        if(chr1 >= 'A' && chr1 <= 'Z')
            chr1 += 'a' - 'A';

        if(chr2 >= 'A' && chr2 <= 'Z')
            chr2 += 'a' - 'A';

        if(chr1 > chr2)
            return 1;

        if(chr1 < chr2)
            return -1;

        if(!chr1)
            break;

        psz1++;
        psz2++;

        nLen--;
    }

    return 0;
}


