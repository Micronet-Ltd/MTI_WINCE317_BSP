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
    $Log: dlmemset.c $
    Revision 1.6  2009/06/25 20:07:39Z  garyp
    Updated to use new asserts.
    Revision 1.5  2009/04/09 14:13:22Z  davidh
    Function headers updated for AutoDoc.
    Revision 1.4  2007/12/18 19:21:59Z  brandont
    Updated function headers.
    Revision 1.3  2007/11/03 23:31:10Z  Garyp
    Added the standard module header.
    Revision 1.2  2006/02/03 03:25:07Z  brandont
    Correct handling of a negative length.
    Revision 1.1  2005/10/02 03:57:06Z  Pauli
    Initial revision
    Revision 1.5  2005/08/04 04:47:53Z  Garyp
    Modified so that the DCL functions compile with their default names even
    if the OS or project code has redefined the macros to use an alternate
    implementation.
    Revision 1.4  2005/08/03 19:17:46Z  pauli
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
#undef  DclMemSet
#define DclMemSet   DCLFUNC(DclMemSet)

/*-------------------------------------------------------------------
    Public: DclMemSet()

    Initialize a memory buffer with the specified character.
    This is similar to the standard C memset() function.

    Parameters:
        pDst    - A pointer to the destination buffer
        chr     - The fill character
        nLen    - The length to fill

    Return Value:
        Returns the original destination pointer.
-------------------------------------------------------------------*/
void * DclMemSet(
    void *          pDst,
    int             chr,
    size_t          nLen)
{
    char *          pPos = (char *)pDst;

    DclProfilerEnter("DclMemSet", DCLPROF_TOPLEVEL, nLen);

    DclAssertWritePtr(pDst, nLen);

    while(nLen > 0)
    {
        *pPos = (unsigned char)chr;
        pPos++;
        nLen--;
    }

    DclProfilerLeave(0UL);

    return pDst;
}
