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

    This module contains functions for converting UTF-8 strings to lowercase.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dlutf8strlower.c $
    Revision 1.5  2009/05/19 16:02:38Z  garyp
    Clarified documentation and added debug code.
    Revision 1.4  2009/05/16 15:31:31Z  keithg
    Corrected spelling error, added clarification to function header comments.
    Revision 1.3  2009/05/15 01:42:36Z  keithg
    Updated to use the renamed UTF to lower functions.
    Revision 1.2  2009/05/14 23:45:23Z  keithg
    Updated to reflect new names of the UTF-8 functions.
    Revision 1.1  2009/05/08 18:52:14Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <dcl.h>


/*-------------------------------------------------------------------
    Protected: DclUtf8StringToLower()

    Convert a UTF-8 string to lower case in the specified output
    buffer.  Processing will stop when the first partial or invalid
    UTF-8 sequence is encountered, or when either buffer is
    exhausted.

    The output buffer will be null terminated only if the source
    data includes a terminating null, and the source length counts
    it.

    The UTF-8 input string will not stop where a multicharacter
    sequence is required to perform a case conversion.

    *Note* -- This function does not support overlapping buffers.

    Parameters:
        pacInputBuffer     - Pointer to the input string.
        nInputBuffSize     - Size in bytes of the input string.
        pacOutputBuffer    - Pointer to the output buffer.
        nOutputBuffSize    - Maximum space in the output buffer.
        pnProcessedInput   - The number of bytes processed in the
                             input buffer.
        pnProcessedOutput  - The number of bytes that reside in the
                             output buffer.

    Return Value:
        None.
-------------------------------------------------------------------*/
void DclUtf8StringToLower(
    const char *pacInputBuffer,
    size_t      nInputBuffSize,
    char       *pacOutputBuffer,
    size_t      nOutputBuffSize,
    size_t     *pnProcessedInput,
    size_t     *pnProcessedOutput)
{
    D_UINT32    ulCodePoint;
    size_t      nProcessedInput = 0;
    size_t      nProcessedOutput = 0;
    size_t      nInBytes;
    size_t      nOutBytes;

    DclAssert(pacInputBuffer);
    DclAssert(nInputBuffSize);
    DclAssert(pacOutputBuffer);
    DclAssert(nOutputBuffSize);
    DclAssert(pnProcessedInput);
    DclAssert(pnProcessedOutput);
    DclAssert((pacInputBuffer >= pacOutputBuffer + nOutputBuffSize)
           || (pacOutputBuffer >= pacInputBuffer + nInputBuffSize));

    while(nInputBuffSize && nOutputBuffSize)
    {
        char chr = *pacInputBuffer;

        /*  If this is a UTF-8 character with the high bit set...
        */
        if(chr & 0x80)
        {
            ulCodePoint = DclUtf8ToScalar(pacInputBuffer, nInputBuffSize, &nInBytes);
            if(nInBytes == 0)
            {
                /*  Contains a partial or invalid multibyte sequence
                */
                break;
            }

            ulCodePoint = DclUtfScalarToLower(ulCodePoint);

            nOutBytes = DclUtf8FromScalar(ulCodePoint, pacOutputBuffer, nOutputBuffSize);
            if(nOutBytes == 0)
            {
                /*  Insufficient room
                */
                break;
            }
        }
        else
        {
            /*  A regular ASCII character, so handle the optimal way
            */
            if(chr >= 'A' && chr <= 'Z')
                chr += 0x20;

            *pacOutputBuffer = chr;

            nInBytes = 1;
            nOutBytes = 1;
        }

        /*  Reality check
        */
        DclAssert(nInputBuffSize >= nInBytes);
        DclAssert(nOutputBuffSize >= nOutBytes);

        nProcessedInput     += nInBytes;
        nProcessedOutput    += nOutBytes;
        pacInputBuffer      += nInBytes;
        pacOutputBuffer     += nOutBytes;
        nInputBuffSize      -= nInBytes;
        nOutputBuffSize     -= nOutBytes;
    }

    *pnProcessedInput = nProcessedInput;
    *pnProcessedOutput = nProcessedOutput;

    return;
}

