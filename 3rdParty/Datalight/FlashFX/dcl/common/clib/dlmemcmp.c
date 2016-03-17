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
    $Log: dlmemcmp.c $
    Revision 1.6  2009/06/25 19:37:17Z  garyp
    Updated to use new asserts.
    Revision 1.5  2009/04/09 14:13:19Z  davidh
    Function headers updated for AutoDoc.
    Revision 1.4  2007/12/18 03:32:10Z  brandont
    Updated function headers.
    Revision 1.3  2007/11/03 23:31:10Z  Garyp
    Added the standard module header.
    Revision 1.2  2006/02/03 03:24:37Z  brandont
    Correct handling of a negative length.
    Revision 1.1  2005/10/02 03:57:04Z  Pauli
    Initial revision
    Revision 1.6  2005/08/17 08:07:32Z  garyp
    Updated to build without warnings with the ADS tools.
    Revision 1.5  2005/08/04 04:47:52Z  Garyp
    Modified so that the DCL functions compile with their default names even
    if the OS or project code has redefined the macros to use an alternate
    implementation.
    Revision 1.4  2005/08/03 19:17:48Z  pauli
    Replaced DCL_FuncName with DclFuncName.
    Revision 1.2  2005/07/31 01:57:55Z  Garyp
    Updated to use new profiler leave function which now takes a ulUserData
    parameter.
    Revision 1.1  2005/07/25 21:11:12Z  pauli
    Initial revision
---------------------------------------------------------------------------*/

#include <dcl.h>

/*  Redefine this to the default value just in case it was defined
    in dlosconf.h.  If that is the case, this function will still
    compile as expected, it just won't be called.
*/
#undef  DclMemCmp
#define DclMemCmp   DCLFUNC(DclMemCmp)

/*-------------------------------------------------------------------
    Public: DclMemCmp()

    Compare the contents of two buffers.  This function is similar
    to the standard C memcmp() function.

    Parameters:
        pBuff1  - A pointer to first buffer
        pBuff2  - A pointer to second buffer
        nLen    - The length to compare

    Return Value:
        > 0     - Buffer 1 is greater than Buffer 2
        = 0     - Buffer 1 is equal to Buffer 2
        < 0     - Buffer 1 is less than Buffer 2
-------------------------------------------------------------------*/
int DclMemCmp(
    const void *            pBuff1,
    const void *            pBuff2,
    size_t                  nLen)
{
    const unsigned char *   pPos1 = (const unsigned char *)pBuff1;
    const unsigned char *   pPos2 = (const unsigned char *)pBuff2;
    int                     iRetVal = 0;

    DclProfilerEnter("DclMemCmp", DCLPROF_TOPLEVEL, nLen);

    /*  validate parameters
    */
    DclAssertReadPtr(pBuff1, nLen);
    DclAssertReadPtr(pBuff2, nLen);

    while(nLen > 0)
    {
        iRetVal = (*pPos1) - (*pPos2);
        if(iRetVal)
            break;

        /*  on to next chunk
        */
        pPos1++;
        pPos2++;
        nLen--;
    }

    DclProfilerLeave(0UL);

    /*  done
    */
    return iRetVal;
}


