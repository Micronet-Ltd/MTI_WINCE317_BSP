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
    $Log: dlstrncm.c $
    Revision 1.5  2009/06/27 20:35:38Z  garyp
    Updated to use new asserts.
    Revision 1.4  2007/12/18 03:43:48Z  brandont
    Updated function headers.
    Revision 1.3  2007/11/03 23:31:10Z  Garyp
    Added the standard module header.
    Revision 1.2  2006/02/03 03:01:46Z  brandont
    Changed int argument to size_t.  Simplified the implementation by
    modeling the strnicmp() implementation.
    Revision 1.1  2005/12/02 17:18:02Z  Pauli
    Initial revision
---------------------------------------------------------------------------*/

#include <dcl.h>

/*  Redefine this to the default value just in case it was defined
    in dlosconf.h.  If that is the case, this function will still
    compile as expected, it just won't be called.
*/
#undef  DclStrNCmp
#define DclStrNCmp   DCLFUNC(DclStrNCmp)

/*-------------------------------------------------------------------
    Public: DclStrNCmp()

    Limited length case sensitive compare.  Perform a case insensitive
    string compare with a limit on the number of characters.

    Parameters:
        psz1 - A pointer to a string fragment or null terminated.
        psz2 - A pointer to a string fragment or null terminated.
        nLen - Length to compare.

    Return Value:
        Returns 0       No difference
        Returns +/-1    Different
-------------------------------------------------------------------*/
int DclStrNCmp(
    const char     *psz1,
    const char     *psz2,
    size_t          nLen)
{
    unsigned char   chr1;
    unsigned char   chr2;

    DclAssertReadPtr(psz1, 0);
    DclAssertReadPtr(psz2, 0);

    while(nLen > 0)
    {
        chr1 = *psz1;
        chr2 = *psz2;

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
