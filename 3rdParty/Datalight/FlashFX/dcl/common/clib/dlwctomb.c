/*---------------------------------------------------------------------------
               ----> DO NOT REMOVE THE FOLLOWING NOTICE <----

                  Copyright (c) 1993 - 2010 Datalight, Inc.
                       All Rights Reserved Worldwide.

  Datalight, Incorporated is a Washington State corporation currently located
  at:
        21520 30th Dr SE, Suite 110,      Tel:  425-951-8086
        Bothell, WA  98021                Fax:  425-951-8094
        USA                               Web:  http://www.datalight.com

  This software, including without limitation all source code and documen-
  tation, is a trade secret and the confidential property of Datalight, 
  Inc., protected under the copyright laws of the United States and other
  jurisdictions.  Patents may be pending.

  U.S. Government Restricted Rights:  Use, duplication, reproduction, or
  transfer of this commercial product and accompanying documentation is
  restricted in accordance with FAR 12.212 and DFARS 227.7202 and by a
  License Agreement.

  IN ADDITION TO COPYRIGHT AND PATENT LAW, THIS SOFTWARE IS PROTECTED UNDER
  CONTRACT(S) BETWEEN DATALIGHT, INC. AND THE LICENSEE ("BINDING AGREEMENTS").
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

    This module contains a helper routine to convert a wide character
    string to a multibyte character string.

    This conversion algorithm was derived from RFC3269.  Note that this
    implementation does not take into account UCS-4/UTF16 or larger:

    Encoding a character to UTF-8 proceeds as follows:

    1.  Determine the number of octets required from the character number
        and the first column of the table above.  It is important to note
        that the rows of the table are mutually exclusive, i.e., there is
        only one valid way to encode a given character.

    2.  Prepare the high-order bits of the octets as per the second
        column of the table.

    3.  Fill in the bits marked x from the bits of the character number,
        expressed in binary.  Start by putting the lowest-order bit of
        the character number in the lowest-order position of the last
        octet of the sequence, then put the next higher-order bit of the
        character number in the next higher-order position of that octet,
        etc.  When the x bits of the last octet are filled in, move on to
        the next to last octet, then to the preceding one, etc. until all
        x bits are filled in.

    The complete text to RFC3629 can be found at

        http://www.ietf.org/rfc/rfc3629.txt
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dlwctomb.c $
    Revision 1.9  2010/07/21 15:04:38Z  garyp
    Documentation corrections -- no functional changes.
    Revision 1.8  2009/05/08 15:00:03Z  jimmb
    Removed windows specific include files
    Revision 1.7  2009/05/07 22:57:32Z  garyp
    Corrected to remove an unused variable.
    Revision 1.6  2009/05/07 22:14:38Z  garyp
    Cleaned up the documentation and code formatting.  Corrected to use
    proper Hungarian notation.  Changed a few datatypes to use size_t.  No
    functional changes.
    Revision 1.5  2009/01/26 21:48:25Z  johnb
    Corrected spelling errors in function headers
    Revision 1.4  2009/01/26 18:04:42Z  johnb
    Clarified description of DclWideCharToMultiByte, specifically the how
    DclWideCharToMultiByte functions when the parameter nWideCharStrLen is
    greater than 0.
    Revision 1.3  2009/01/20 18:38:11Z  johnb
    Updated DclWideCharToMultiByte and DclMultByteToWideChar to use char rather
    than D_UINT8.
    Revision 1.2  2009/01/19 22:29:49Z  jimmb
    Corrected warning found by GNU compiler.
    Revision 1.1  2009/01/16 23:01:22Z  johnb
    Initial revision
---------------------------------------------------------------------------*/
#include <dcl.h>

#define UTF8_2_BYTE_SEQ 0xC0
#define UTF8_3_BYTE_SEQ 0xE0


/*-------------------------------------------------------------------
    Public: DclWideCharToMultiByte()

    Convert a wide character (UNICODE) string to a multibyte (UTF8)
    string.

    Parameters:
        pWideCharStr       - Pointer to the wide character source
                             string to convert.
        iWideCharStrLen      Size, in D_UINT16 values, of the string
                             indicated by pWideCharStr.  If this
                             parameter is set -1, the function assumes
                             the string to be null terminated and
                             calculates the length automatically,
                             including the null terminator.  If this
                             parameter is 0, the function returns
                             failure.  If this parameter is greater
                             than 0, then this function converts the
                             specified number of wide characters into
                             multibyte chararacters regardless of
                             embedded null characters.  The resulting
                             multibyte character string may not be
                             null terminated.  Null characters in the
                             source string will be processed and
                             converted to multibyte characters.
        pMultiByteBuffer   - Pointer to a buffer that recieves the
                             converted multibyte character string.
        nMultiByteBufLimit   Maximum size, in bytes, of the buffer
                             indicated by pMultiByteBuffer.  If this
                             parameter is set to 0, the function
                             returns the required buffer size for
                             pMultiByteBuffer and makes no use of the
                             output parameter itself.
        pnNumConverted     - If conversion is successful, this contains
                             the number of bytes written to the buffer
                             indicated by pMultiByteBuffer including
                             the null terminating character.  If the
                             function succeeds and nMultiByteBufLimit
                             is 0, the return value is the required
                             size, in bytes, for the buffer indicated
                             by pMultiByteBuffer.  If this parameter
                             is passed as NULL, the function returns
                             failure.

    Return Value:
        Returns a DCLSTATUS code indicating the operation result.
-------------------------------------------------------------------*/
DCLSTATUS DclWideCharToMultiByte(
    const D_UINT16 *pWideCharStr,
    int             iWideCharStrLen,
    char           *pMultiByteBuffer,
    size_t          nMultiByteBufLimit,
    size_t         *pnNumConverted)
{
    D_BOOL          fCountOnly  = FALSE;
    int             iSrcIndex   = 0;
    unsigned        nOutputLen  = 0;
    DCLSTATUS       dclStat     = DCLSTAT_SUCCESS;
    D_UINT8        *pDestStr    = (D_UINT8*)pMultiByteBuffer;

    /*  Validate the parameters
    */
    if (!pWideCharStr || iWideCharStrLen == 0 || !pnNumConverted ||
        (nMultiByteBufLimit != 0 && !pMultiByteBuffer) )
    {
        return DCLSTAT_BADPARAMETER;
    }

    /*  Does the source string length need to be calculated?
    */
    if (iWideCharStrLen == -1)
    {
        iWideCharStrLen = 0;
        while (pWideCharStr[iWideCharStrLen] != (D_UINT16)0)
            iWideCharStrLen++;

        /*  Count the null character
        */
        iWideCharStrLen++;
    }

    /*  See if we are converting or determining how much space
        is needed to convert
    */
    if ( nMultiByteBufLimit == 0 )
    {
        /*  set the flag to signal that we are just counting
            the space and not converting
        */
        fCountOnly = TRUE;
    }

    /*  For each UNICODE character, we use the value to determine
        how to convert each character.  From RFC2044:

            UCS-2 range (hex.)      UTF-8 octet sequence (binary)
            0000-007F               0xxxxxxx
            0080-07FF               110xxxxx 10xxxxxx
            0800-FFFF               1110xxxx 10xxxxxx 10xxxxxx
    */

    for ( iSrcIndex=0; iSrcIndex < iWideCharStrLen; iSrcIndex++ )
    {

        /*  Check to see if we have reached the maximum length of the
            destination buffer to prevent overflowing the destination buffer.
        */
        if ( fCountOnly == FALSE && nOutputLen >= nMultiByteBufLimit )
        {
            /* overflow detected
            */
            dclStat = DCLSTAT_LIMITREACHED;
            break;
        }

        /*  If character is less than 0x0080, treat it as an single byte
            UTF8 byte sequence
        */
        if ( pWideCharStr[iSrcIndex] < 0x0080 )
        {
            if (fCountOnly == FALSE)
                pDestStr[nOutputLen] = (D_UINT8)pWideCharStr[iSrcIndex];

            nOutputLen++;
        }
        else if ( pWideCharStr[iSrcIndex] < 0x0800 )
        {
            /*  If character is >= 0x0080 and < 0x0800, it is a two byte
                UTF8 byte sequence.
            */
            if (fCountOnly == FALSE)
            {
                if (nOutputLen+2 >= nMultiByteBufLimit)
                {
                    /*  This conversion would overflow the buffer limit
                        report the error.
                    */
                    dclStat = DCLSTAT_LIMITREACHED;
                    break;
                }
                else
                {
                    pDestStr[nOutputLen]   = ((pWideCharStr[iSrcIndex] >> 6) | UTF8_2_BYTE_SEQ);
                    pDestStr[nOutputLen+1] = ((pWideCharStr[iSrcIndex] & 0x3F) | 0x80);
                }
            }
            nOutputLen += 2;
        }
        else
        {
            /*  If character is >= 0x0800, it is a three byte UTF8 byte sequence.

                NOTE: in RFC 2044, the range 0xD800-0xDFFF is a special range
                for mapping UCS-4 characters into UTF16.  When checking this
                range against the Win32 functions, WideCharToMultiByte, this
                range is not taken into consideration, but uses the algorithm
                below to convert this range into UTF8.
            */
            if (fCountOnly == FALSE)
            {
                if (nOutputLen+3 >= nMultiByteBufLimit)
                {
                    /*  This conversion would overflow the buffer limit
                        report the error.
                    */
                    dclStat = DCLSTAT_LIMITREACHED;
                    break;
                }
                else
                {
                    pDestStr[nOutputLen]   = ((pWideCharStr[iSrcIndex] >> 12) | UTF8_3_BYTE_SEQ);
                    pDestStr[nOutputLen+1] = (((pWideCharStr[iSrcIndex] >> 6) & 0x3F) | 0x80);
                    pDestStr[nOutputLen+2] = ((pWideCharStr[iSrcIndex] & 0x3F) | 0x80);
                }
            }
            nOutputLen += 3;
        }

    }

    /*  Return the number of bytes converted or would be converted
    */
    *pnNumConverted = nOutputLen;

    return dclStat;
}

