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
    $Log: dlltoa.c $
    Revision 1.7  2009/06/25 19:36:20Z  garyp
    Updated to use new asserts.
    Revision 1.6  2009/05/20 00:44:25Z  garyp
    Fixed Bug 2735 -- Negative numbers don't always format properly.
    Revision 1.5  2009/05/02 00:52:19Z  garyp
    Updated to use size_t types rather than ints.  Properly declared static
    constant data as such.  Documentation updated.  No functional changes.
    Revision 1.4  2009/04/09 14:13:17Z  davidh
    Function headers updated for AutoDoc.
    Revision 1.3  2007/12/18 03:33:14Z  brandont
    Updated function headers.
    Revision 1.2  2007/11/03 23:31:09Z  Garyp
    Added the standard module header.
    Revision 1.1  2006/05/06 22:56:54Z  Garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <dcl.h>


/*-------------------------------------------------------------------
    Public: DclLtoA()

    Format a signed 32-bit integer as a base 10 ASCII string.

    *Note* -- If the output buffer length is exhausted, the
    result will *NOT* be null-terminated.

    *Note* -- If the nFillLen value is greater than or equal to the
    buffer length, the result will not be null-terminated, even if
    the formatted portion of the data is shorter than the buffer
    length.

    Parameters:
        pBuffer      - The output buffer
        pnBufferLen  - A pointer to the output buffer length
        lNum         - The 32-bit signed number to convert
        nFillLen     - The fill length, if any
        cFill        - The fill character to use

    Return Value:
        Returns a size_t type containing the length of the string.
        On exit, the value pointed to by pnBufferLen will be adjusted
        to reflect the buffer space remaining.
-------------------------------------------------------------------*/
size_t DclLtoA(
    char               *pBuffer,
    size_t             *pnBufferLen,
    D_INT32             lNum,
    size_t              nFillLen,
    char                cFill)
{
    #define             MAXNUMLENGTH (16) /*  big enough for a D_INT32 in base 10 */
    static const char   decimal[] = "0123456789";
    char                ach[MAXNUMLENGTH+1];
    size_t              nSize = 0;
    char               *pBuff = pBuffer;
    D_BOOL              fSign = FALSE;

    DclAssertWritePtr(pnBufferLen, sizeof(*pnBufferLen));
    DclAssertWritePtr(pBuffer, *pnBufferLen);

    if(lNum < 0)
    {
        fSign = TRUE;
        lNum = -lNum;
    }

    do
    {
        ach[nSize] = decimal[(unsigned)(lNum % 10)];
        lNum = lNum / 10;
        nSize++;
    }
    while(lNum);

    if(fSign)
    {
        ach[nSize] = '-';
        nSize++;
    }

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
