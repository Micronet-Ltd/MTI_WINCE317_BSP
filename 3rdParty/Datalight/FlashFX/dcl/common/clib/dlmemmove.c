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
    general Standard C library functions.  This is done for compatibility,
    portability, and code size reasons.

    These functions are NOT intended to be complete, 100% ANSI C compatible
    implementations, but rather address specific needs.

    See the specific function headers for more information about departures
    from the ANSI standard, and missing or extended functionality.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                              Revision History
    $Log: dlmemmove.c $
    Revision 1.2  2009/04/09 14:13:21Z  davidh
    Function headers updated for AutoDoc.
    Revision 1.1  2008/07/23 19:37:20Z  keithg
    Initial revision
---------------------------------------------------------------------------*/

#include <dcl.h>

/*  Redefine this to the default value just in case it was defined
    in dlosconf.h.  If that is the case, this function will still
    compile as expected, it just won't be called.
*/
#undef  DclMemMove
#define DclMemMove   DCLFUNC(DclMemMove)

/*-------------------------------------------------------------------
    Public: DclMemMove()

    Move buffer contents from one address to another.  This is
    similar to the standard C memmove() function in that it will
    handle overlapping memory regions.

    Parameters:
        pDst - A pointer to the destination buffer
        pSrc - A pointer to the source buffer
        nLen - The length to copy

    Return Value:
        Returns the original destination pointer.
-------------------------------------------------------------------*/
void * DclMemMove(
    void *          pDst,
    const void *    pSrc,
    size_t          nLen)
{
    D_UINT8        *pbDst;
    D_UINT8        const *pbSrc;

    DclProfilerEnter("DclMemMove", DCLPROF_TOPLEVEL, nLen);

    DclAssert(pDst);
    DclAssert(pSrc);

    pbDst = (D_UINT8 *) pDst;
    pbSrc = (D_UINT8 const *) pSrc;

    /*  For buffers where the destination is lower than the source,
        we can just copy from the source to the destination.
    */
    if( (pbDst < pbSrc) || (pbDst >= (pbSrc + nLen)) )
    {
        while(nLen)
        {
            *pbDst = *pbSrc;
            ++pbDst;
            ++pbSrc;
            --nLen;
        }
    }

    /*  Otherwise, copy from the end of the source to the
        destination (backwards) to avoid corrupting the data.
    */
    else
    {
        pbDst += nLen - 1;
        pbSrc += nLen - 1;

        while(nLen)
        {
            *pbDst = *pbSrc;
            --pbDst;
            --pbSrc;
            --nLen;
        }
    }

    DclProfilerLeave(0UL);

    return pDst;
}


