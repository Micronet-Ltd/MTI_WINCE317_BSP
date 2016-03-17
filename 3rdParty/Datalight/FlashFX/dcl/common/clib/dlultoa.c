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

    This module contains helper routines for converting binary numbers to
    ASCII.

    There is no ANSI C equivalent for this function, however because it is
    similar in nature to other ANSI C style functions, this module resides
    in the "Clib" directory.
 ---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dlultoa.c $
    Revision 1.5  2009/05/02 17:11:37Z  garyp
    Updated to use size_t types rather than ints.  Properly declared static
    constant data as such.  Documentation updated.  No functional changes.
    Revision 1.4  2007/12/18 03:34:07Z  brandont
    Updated function headers.
    Revision 1.3  2007/11/03 23:31:10Z  Garyp
    Added the standard module header.
    Revision 1.2  2006/05/06 22:56:53Z  Garyp
    Modified to update the buffer space remaining value upon exit.
    Revision 1.1  2005/10/02 03:57:14Z  Pauli
    Initial revision
---------------------------------------------------------------------------*/

#include <dcl.h>


/*-------------------------------------------------------------------
    Public: DclULtoA()

    Format an unsigned 32-bit integer as an ASCII string in the
    given base.  16 and 10 are common, but any base from 2 to 16
    will work.  A radix of 2 or 16 will be padded to length with
    the value in cFill.

    Note that if the output buffer length is exhausted, the
    result will *NOT* be null-terminated.

    Parameters:
        pBuffer      - The output buffer
        pnBufferLen  - A pointer to the output buffer length
        ulNum        - The 32-bit unsigned number to convert
        nRadix       - The radix to use (2 through 16)
        nFillLen     - The fill length, if any
        cFill        - The fill character to use

    Return Value:
        Returns a size_t type containing the length of the string.
        On exit, the value pointed to by pnBufferLen will be adjusted
        to reflect the buffer space remaining.
-------------------------------------------------------------------*/
size_t DclULtoA(
    char               *pBuffer,
    size_t             *pnBufferLen,
    D_UINT32            ulNum,          /*  Fixed width 32-bit */
    unsigned            nRadix,
    size_t              nFillLen,
    char                cFill)
{
    #define             MAXNUMLENGTH (32) /* big enough for a D_UINT32 in radix 2 to 16 */
    static const char   hex[] = "0123456789ABCDEF";
    char                ach[MAXNUMLENGTH+1];
    size_t              nSize = 0;
    char               *pBuff = pBuffer;

    DclAssert(pBuffer);
    DclAssert(nRadix >= 2);
    DclAssert(nRadix <= 16);
    DclAssert(pnBufferLen);

    do
    {
        ach[nSize] = hex[(unsigned)(ulNum % nRadix)];
        ulNum = ulNum / nRadix;
        nSize++;
    }
    while(ulNum);

    /*  user may have asked for a fill char
    */
    if(nFillLen > nSize)
    {
        nFillLen -= nSize;

        while(nFillLen > 0 && *pnBufferLen)
        {
            *pBuff = cFill;
            pBuff++;
            nFillLen--;
            (*pnBufferLen)--;
        }
    }

    /*  reverse the string
    */
    while(nSize && *pnBufferLen)
    {
        nSize--;
        *pBuff = ach[nSize];
        pBuff++;
        (*pnBufferLen)--;
    }

    if(*pnBufferLen)
        *pBuff = 0;

    DclAssert((D_UINTPTR)pBuff >= (D_UINTPTR)pBuffer);

    return (size_t)(pBuff - pBuffer);
}

