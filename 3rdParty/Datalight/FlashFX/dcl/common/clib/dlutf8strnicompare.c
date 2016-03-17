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

    This module contains functions to perform UTF-8, case-insensitive string
    comparisons.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dlutf8strnicompare.c $
    Revision 1.7  2009/05/21 04:03:14Z  garyp
    Documentation updated -- no functional changes.
    Revision 1.6  2009/05/20 00:10:58Z  keithg
    Added clarifying comments regarding the evaluation of a successful
    comparason to include differing string lengths.
    Revision 1.5  2009/05/17 23:22:00Z  keithg
    Updated function header to clarify return values and state information.
    Revision 1.4  2009/05/15 01:42:33Z  keithg
    Updated to use the renamed UTF to lower functions.
    Revision 1.3  2009/05/14 23:44:59Z  keithg
    Updated to reflect new names of the UTF-8 functions.
    Revision 1.2  2009/05/09 01:53:24Z  brandont
    Renamed the NOS_UTF_PARITAL_MBS_1 and NOS_UTF_PARITAL_MBS_2 definitions and
    moved them to dlapiint.h.  Also updated how these flags were set so that
    they will not overwrite the previous contents.
    Revision 1.1  2009/05/08 18:52:14Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <dcl.h>


/*-------------------------------------------------------------------
    Protected: DclUtf8StrNICmp()

    Perform a case-insensitive comparison of at most the first N
    bytes in two UTF-8 strings.  The number of bytes of each string
    are returned in the pnProcessedX parameters.  Note the counts
    may be different due to variable length UTF-8 encodings.

    This function is locale-independent.

    Parameters:
        pacBuffer1     - A pointer to a char buffer containing UTF-8
                         string 1.
        pacBuffer2     - A pointer to a char buffer containing UTF-8
                         string 2.
        nBuffer1Size   - The maximum bytes to compare in pacBuffer1
        nBuffer2Size   - The maximum bytes to compare in pacBuffer2
        pnProcessed1   - Populated with the number of bytes processed
                         in pacBuffer1.
        pnProcessed2   - Populated with the number of bytesprocessed
                         in pacBuffer2.
        pnCmpFlags     - A pointer to the results flags which indicate
                         if a partial UTF-8 sequence is encountered.

    Return Value:
        Returns 0 if the strings are equal up to the end of either
        buffer and excluding excluding any incomplete UTF-8 sequence
        at the end of either buffer, in which case pnCmpFlags bits
        DCL_UTF8_PARTIAL_BUFFER_x may be set to indicate which
        buffer(s) holds an unterminated UTF-8 character.

        Note this means that two strings of different sizes may
        return zero indicating a successful compare. The count of
        bytes that were evaluated are returned in *pnProcessedX.

        If the strings do not match, non-zero is returned.  If the
        strings are within the same language set, 1 will indicate that
        string 1 is greater than string 2 and -1 if string 2 is greater
        than string 1.
-------------------------------------------------------------------*/
int DclUtf8StrNICmp(
    const char     *pacBuffer1,
    const char     *pacBuffer2,
    size_t          nBuffer1Size,
    size_t          nBuffer2Size,
    size_t         *pnProcessed1,
    size_t         *pnProcessed2,
    unsigned       *pnCmpFlags)
{
    D_UINT32        ulCodePoint1;
    D_UINT32        ulCodePoint2;
    size_t          nBytes1 = 0;
    size_t          nBytes2 = 0;
    int             iResult = 0;

    DclAssert(pacBuffer1);
    DclAssert(pacBuffer2);
    DclAssert(nBuffer1Size);
    DclAssert(nBuffer2Size);
    DclAssert(pnProcessed1);
    DclAssert(pnProcessed2);
    DclAssert(pnCmpFlags);

    *pnProcessed1 = 0;
    *pnProcessed2 = 0;
    *pnCmpFlags = 0;

    while(nBuffer1Size && nBuffer2Size)
    {
        ulCodePoint1 = DclUtf8ToScalar(pacBuffer1, nBuffer1Size, &nBytes1);
        if(nBytes1 == 0)
        {
            /*  Contains a partial multibyte sequence
            */
            *pnCmpFlags |= DCL_UTF8_PARTIAL_BUFFER_1;
        }

        ulCodePoint2 = DclUtf8ToScalar(pacBuffer2, nBuffer2Size, &nBytes2);
        if(nBytes2 == 0)
        {
            /*  Contains a partial multibyte sequence
            */
            *pnCmpFlags |= DCL_UTF8_PARTIAL_BUFFER_2;
        }

        if(*pnCmpFlags)
        {
            break;
        }

        ulCodePoint1 = DclUtfScalarToLower(ulCodePoint1);
        ulCodePoint2 = DclUtfScalarToLower(ulCodePoint2);

        if(ulCodePoint1 != ulCodePoint2)
        {
            /*  Strings don't match
            */
            if(ulCodePoint1 > ulCodePoint2)
            {
                iResult = 1;
            }
            else
            {
                iResult = -1;
            }

            break;
        }

        /*  Reality check
        */
        DclAssert(nBuffer1Size >= nBytes1);
        DclAssert(nBuffer2Size >= nBytes2);

        *pnProcessed1   += nBytes1;
        *pnProcessed2   += nBytes2;
        pacBuffer1      += nBytes1;
        pacBuffer2      += nBytes2;
        nBuffer1Size    -= nBytes1;
        nBuffer2Size    -= nBytes2;
    }

    return iResult;
}

