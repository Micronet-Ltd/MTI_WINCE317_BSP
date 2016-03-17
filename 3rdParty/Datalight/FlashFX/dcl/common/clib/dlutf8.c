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
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dlutf8.c $
    Revision 1.7  2010/04/28 23:31:28Z  garyp
    Updated to build cleanly with the TICCS ToolSet abstraction.
    Revision 1.6  2009/05/15 16:16:24Z  jimmb
    fixed the gnu warning:
    pointer targets in passing argument 1 of DclUtf8CodeLength differ in signedness
    Revision 1.5  2009/05/15 01:57:50Z  keithg
    Added asserts, corrected comments and fixed hungarian notations.
    Revision 1.4  2009/05/14 22:06:57Z  keithg
    Added clarifying comments, Unicode Standard references, and
    improved UTF8 validation to more strictly conform with the standard.
    Renamed functions to refer to scalars rather than UCS4.
    Revision 1.3  2009/05/11 13:55:52Z  jimmb
    Added a cast to remove a gnu compiler warning.
    Revision 1.2  2009/05/09 01:50:25Z  brandont
    Initialized a return argument in DclUtf8ToUcs4 for an error case.
    Revision 1.1  2009/05/08 18:13:50Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <dcl.h>

/*-------------------------------------------------------------------
    Local: DclUtf8CodeLen()

    Determine the number of expected code bytes of a UTF-8 encoded
    sequence.  The byte sequence is not evaluated for conformity,
    only the apparent code length by evaluation of the first byte.

    Parameters:
        pacBuffer - A pointer to a char buffer containing the UTF-8
                    encoded character to examine.

    Return Value:
        Returns the number of bytes that compose the UTF-8 character
        code, or zero if the character does not appear to be the
        start of a UTF-8 encoded character.
-------------------------------------------------------------------*/
static unsigned DclUtf8CodeLength(
    const char     *pacBuffer)
{
    const D_BUFFER *pBuffer = (const D_BUFFER*)pacBuffer;

    DclAssert(pBuffer);

    /*  WARNING! Don't call DclUtf8IsValidChar() from this code as it
                 uses this function, and you'd be waiting a very long
                 time for it to return!
    */

    /*  The Unicode Standard, 3.9 Unicode Encoding Forms, D92
           "Any UTF-8 byte sequence that does not match the
            patterns listed in Table 3-7 is ill-formed."

        These includes at most a four byte sequence.
    */

    if(!(*pBuffer & 0x80))
        return 1;
    else if((*pBuffer & 0xE0) == 0xC0)
        return 2;
    else if((*pBuffer & 0xF0) == 0xE0)
        return 3;
    else if((*pBuffer & 0xF8) == 0xF0)
        return 4;
    else
        return 0;
}


/*-------------------------------------------------------------------
    Protected: DclUtf8IsValidSequence()

    Determine if the specified character sequence represents a well
    formed UTF-8 encoding.  This function makes no attempt to
    interpret the character sequence.

    Parameters:
        pBuffer  - A pointer to a char buffer containing the UTF-8
                     character sequence.
        nBufferLen - The maximum length of the buffer which may be
                     examined.

    Return Value:
        Returns TRUE if the UTF-8 sequence is well-formed, or FALSE
        if the encoding is erroneous or does not fit within the
        given buffer length.
-------------------------------------------------------------------*/
D_BOOL DclUtf8IsValidSequence(
    const char     *pacBuffer,
    size_t          nBufferLen,
    size_t          *pnSequenceLength)
{
    const D_BUFFER *pBuffer = (const D_BUFFER*)pacBuffer;
    unsigned        nCharLen;
    unsigned        index;

    DclAssert(pBuffer);
    DclAssert(nBufferLen);
    DclAssert(pnSequenceLength);

    /*  Start with a zero length sequence, we will update this
        later if the sequence turns out to be well-formed.
    */
    *pnSequenceLength = 0;
    nCharLen = DclUtf8CodeLength((char *)pBuffer);

    /*  If the length is not legal or indeterminate, fail
    */
    if((!nCharLen) || (nCharLen > nBufferLen))
        return FALSE;

    /*  The first byte in a valid UTF-8 sequence includes the
        length of the entire sequence and the start of the
        code point.

        0xxx xxxx   single byte sequence
        110x xxxx   two byte sequence
        1110 xxxx   three byte sequence
        1111 0xxx   four byte sequence
    */
    switch(nCharLen)
    {
        case 1:
            *pnSequenceLength = 1;
            return TRUE;

        /*  For the multi-byte sequences ensure that the encoded scalar
            would not otherwise fit in a smallar sequence.  This ensures
            that all sequences are in their minimal encoding.
        */
        case 2:
            if((*pBuffer & 0xFE) == 0xC0)
                return FALSE;
            break;
        case 3:
            if(*pBuffer == 0xE0 && (*(pBuffer+1) & 0xE0) == 0x80)
                return FALSE;
            break;
        case 4:
            if(*pBuffer == 0xF0 && (*(pBuffer+1) & 0xF0)== 0x80)
                return FALSE;
            break;
        default:
            DclProductionError();
    }


    /*  Each subsequent byte in a well-formed encoding conists
        of the binary values with a pattern of 10xx xxxx and
        all other values are illegal.
    */
    for(index=1; index<nCharLen; ++index)
    {
        if(pBuffer[index] < 0x80 || pBuffer[index] > 0xBF)
        {
            return FALSE;
        }
    }

    *pnSequenceLength = nCharLen;
    return TRUE;
}


/*-------------------------------------------------------------------
    Protected: DclUtf8ToScalar()

    Convert a UTF-8 character to a U+ scalar value.  The scalar
    may include any single Unicode character and explicitly excludes
    any surrogate pairs.

    Note that in an error condition, no replacement or interpretation
    is made of the sequence.  The caller must determine the appropriate
    action to take.

    Parameters:
        pacBuffer        - A pointer to a char buffer containing the
                           UTF-8 character to convert.
        nBuffSize        - The input buffer size.
        pnBytesProcessed - A pointer to a buffer to received the
                           count of UTF-8 bytes which were processed.

    Return Value:
        Returns the Unicode scalar and number of bytes decoded from
        the input buffer in pnBytesProcessed.  On detection of an
        ill-formed sequence D_UINT32_MAX is returned and zero bytes
        are processed.
-------------------------------------------------------------------*/
D_UINT32 DclUtf8ToScalar(
    const char     *pacBuffer,
    size_t          nBuffSize,
    size_t         *pnBytesProcessed)
{
    const D_BUFFER *pBuffer = (const D_BUFFER*)pacBuffer;
    size_t          nBytes;
    unsigned        nMask;
    D_UINT32        ulCodePoint;
    D_BOOL          fValidCharacter;
    unsigned        index;

    DclAssert(pacBuffer);
    DclAssert(nBuffSize);
    DclAssert(pnBytesProcessed);

    /*  Zero bytes are processed until the entire sequence is
        successfully decoded.
    */
    *pnBytesProcessed = 0;

    fValidCharacter = DclUtf8IsValidSequence((char *)pBuffer, nBuffSize, &nBytes);
    if(!fValidCharacter || nBytes > nBuffSize || nBytes == 0)
    {
        return D_UINT32_MAX;
    }

    /*  Determine the number of bytes in the sequence and mask
        for the scalar data bits in the first byte.  All subsequent
        bytes have fixed widths.

        The Unicode Standard, 3.9 Unicode Encoding Forms, D92 Table 3-6
          U-00000000 – U-0000007F: 0xxxxxxx
          U-00000080 – U-000007FF: 110xxxxx 10xxxxxx
          U-00000800 – U-0000FFFF: 1110xxxx 10xxxxxx 10xxxxxx
          U-00010000 – U-0010FFFF: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
    */
    switch(nBytes)
    {
        case 1:
            nMask = 0x7f;
            break;
        case 2:
            nMask = 0x1f;
            break;
        case 3:
            nMask = 0x0f;
            break;
        case 4:
            nMask = 0x07;
            break;

        default:
            return D_UINT32_MAX;
    }

    if(nBytes > nBuffSize)
    {
        /*  Insufficent bytes in the buffer to decode
        */
        *pnBytesProcessed = 0;
        return D_UINT32_MAX;
    }

    /*  Process the first byte relative to the mask
    */
    ulCodePoint = *pBuffer & nMask;

    /*  Process any remaining bytes.
    */
    for(index=1; index<nBytes; ++index)
    {
        ulCodePoint <<= 6;
        ulCodePoint |= pBuffer[index] & 0x3F;
    }

    /*  At this point we have a scalar value - check to ensure
        there are no surrogate pairs in this encoding:

        The Unicode Standard, 3.9 Unicode Encoding Forms, D92
           "Because surrogate code points are not Unicode scalar
            values, any UTF-8 byte sequence that would otherwise
            map to code points D800..DFFF is ill-formed."

        The Unicode Standard, 3.9 Unicode Encoding Forms, D76
           "Unicode scalar value: Any Unicode code point except
            high-surrogate and low-surrogate code points. As a
            result of this definition, the set of Unicode scalar
            values consists of the ranges 0 to D7FF and E000
            to 10FFFF, inclusive."
    */

    if( (ulCodePoint >= 0xd800 && ulCodePoint < 0xe000)
        || (ulCodePoint > 0x10ffff) )
            return D_UINT32_MAX;

    /* Return the U+ scalar */
    *pnBytesProcessed = nBytes;
    return ulCodePoint;
}


/*-------------------------------------------------------------------
    Protected: DclUtf8FromScalar()

    Convert a U+ scalar to a UTF-8 character sequence.  The scalar
    may include any single Unicode character and explicitly excludes
    any surrogate pairs.

    Parameters:
        ulCodePoint - The UCS-4 encoded character to convert.
        pacBuffer   - A pointer to the output char buffer to fill.
        nBuffSize   - The size of pBuffer.

    Return Value:
        Returns the number of bytes in pBuffer which were used,
        or zero on error.  Possible errors include an invalid
        scalar or insufficient buffer space.
-------------------------------------------------------------------*/
size_t DclUtf8FromScalar(
    D_UINT32        ulCodePoint,
    char           *pacBuffer,
    size_t          nBuffSize)
{
    D_BUFFER        *pBuffer = (D_BUFFER*)pacBuffer;
    size_t          nBytes;
    size_t          nTempBytes;
    unsigned        nHighBits;

    DclAssert(pBuffer);
    DclAssert(nBuffSize);

    /*  Handle the simple case first, the first 127 values
        map directly to a single byte ASCII character.
    */
    if(ulCodePoint <= 0x7F)
    {
        *pBuffer = (D_UINT8)ulCodePoint;
        return 1;
    }

    /*  Verify the surrogate pairs are not being used which
        must be considered an error condition, since we do not
        support them and limit the scalar to that of UTF-8 valid
        range.
    */
    if( (ulCodePoint >= 0xd800 && ulCodePoint < 0xe000)
        || (ulCodePoint > 0x10ffff) )
    {
        return 0;
    }

    /*  Determine the number of bytes needed to encode this
        scalar and what bit pattern is needed for the initial
        byte in the UTF-8 character sequence.
    */
    if(ulCodePoint <= 0x7FF)
    {
        nBytes = 2;
        nHighBits = 0xC0;
    }
    else if(ulCodePoint <= 0xFFFF)
    {
        nBytes = 3;
        nHighBits = 0xE0;
    }
    else if(ulCodePoint <= 0x1FFFFF)
    {
        nBytes = 4;
        nHighBits = 0xF0;
    }
    else
    {
        /*  The only possible remaining values are not legal.
            Inidicate that zero bytes were processed.
        */
        return 0;
    }

    /*  Check for sufficient space in the buffer
    */
    if(nBytes > nBuffSize)
        return 0;

    /*  Copy in each six bits into the UTF-8 sequence.  The last
        value will be small enough due to the range checking above.
    */
    *pBuffer = 0;

    nTempBytes = nBytes;

    while(nTempBytes)
    {
        *(pBuffer + nTempBytes - 1) = (D_UINT8)((ulCodePoint & 0x3F) | 0x80);
        ulCodePoint >>= 6;
        nTempBytes--;
    }

    *pBuffer |= (D_UINT8)nHighBits;

    return nBytes;
}

