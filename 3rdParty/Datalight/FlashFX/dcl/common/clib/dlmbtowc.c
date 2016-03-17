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

    This module contains a helper routine to convert a multibyte character
    string to a wide character string.

    This conversion algorithm was derived from RFC3269.  Note that this
    implementation does not take into account UCS-4/UTF16 or larger:

    Decoding a UTF-8 character proceeds as follows:

    1.  Initialize a binary number with all bits set to 0.  Up to 21 bits
        may be needed.

    2.  Determine which bits encode the character number from the number
        of octets in the sequence and the second column of the table
        above (the bits marked x).

    3.  Distribute the bits from the sequence to the binary number, first
        the lower-order bits from the last octet of the sequence and
        proceeding to the left until no x bits are left.  The binary
        number is now equal to the character number.

    The complete text to RFC3629 can be found at

        http://www.ietf.org/rfc/rfc3629.txt
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dlmbtowc.c $
    Revision 1.8  2010/07/21 15:04:20Z  garyp
    Documentation corrections -- no functional changes.
    Revision 1.7  2009/05/06 18:05:15Z  garyp
    Cleaned up the documentation and code formatting.  Corrected to use
    proper Hungarian notation.  Changed a few datatypes to use size_t.  No
    functional changes.
    Revision 1.6  2009/01/27 18:34:26Z  johnb
    corrected compiler warning by replacing pSrcStr with pMultiByteStr
    in call to DclStrLen()
    Revision 1.5  2009/01/26 21:47:47Z  johnb
    Corrected spelling errors in function headers
    Revision 1.4  2009/01/26 21:36:14Z  johnb
    Removed include file dlmbwc.h.  Removed unnecessary code.  Cleaned up
    description of DclMultiBytetoWideChar, specifically how this function
    operates when the parameter nMultiByteStrLen is greater than 0.  Modified
    to use DclStrLen when function is determining the string length to convert
    Made local function static and added standard Datalight function header.
    Revision 1.3  2009/01/21 19:21:45Z  johnb
    Updated DclWideCharToMultiByte and DclMultByteToWideChar to use char rather
    than D_UINT8.  Also updated algorithm to check invalid multibyte code
    sequences.
    Revision 1.2  2009/01/19 22:27:07Z  jimmb
    Corrected warning found by GNU compiler.
    Revision 1.1  2009/01/19 18:51:02Z  johnb
    Initial revision
---------------------------------------------------------------------------*/

#include <dcl.h>


static D_BOOL IsLegalMultiByte(const char *pSource, unsigned nLength);

static const char UTF8SequenceLength[128] =
{
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,    /* 0x80 - 0x8F */
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,    /* 0x90 - 0x9F */
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,    /* 0xA0 - 0xAF */
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,    /* 0xB0 - 0xBF */
    0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,    /* 0xC0 - 0xCF */
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,    /* 0xD0 - 0xDF */
    2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,    /* 0xE0 - 0xEF */
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0     /* 0xF0 - 0xFF */
};



/*-------------------------------------------------------------------
    Public: DclMultiByteToWideChar()

    Convert a multibyte (UTF8) string to a wide character (UNICODE)
    string.

    Parameters:
        pMultiByteStr     - Pointer to the multibyte source string to
                            convert.
        iMultiByteStrLen  - Size, in bytes, of the string indicated
                            by pMultiByteStr.  If this parameter is
                            set to -1, the function assumes the string
                            is null terminated and calculates the
                            length automatically, including the null
                            terminator.  If this parameter is 0, the 
                            function returns failure.  If this parameter
                            is greater than 0, then this function
                            converts the specified number of multibyte
                            characters into wide chararacters, regardless
                            of embedded null characters.  The resulting
                            wide char character string may not be null
                            terminated.  Null characters in the source 
                            string will be processed and converted to
                            wide characters.
        pWideCharBuffer   - Pointer to a buffer that recieves the
                            converted wide character string.
        nWideCharBufLimit - Maximum size, in D_UINT16 values, of the
                            buffer indicated by pWideCharBuffer.  If
                            this parameter is set to 0, the function
                            returns the required buffer size for
                            pWideCharBuffer and makes no use of the
                            output parameter itself.
        pnNumConverted    - If conversion is successful, this contains
                            the number of D_UINT16 values written to
                            the buffer indicated by pWideCharBuffer
                            including the null terminating character.
                            If successful and nWideCharBufLimit is 0,
                            the return value is the required size, in
                            D_UINT16 values, for the buffer indicated
                            by pWideCharBuffer.  If this parameter is
                            not provided, the function returns failure.

    Return Value:
        Returns a DCLSTATUS code indicating the operation result.
-------------------------------------------------------------------*/
DCLSTATUS DclMultiByteToWideChar(
    const char *pMultiByteStr,
    int         iMultiByteStrLen,
    D_UINT16   *pWideCharBuffer,
    size_t      nWideCharBufLimit,
    size_t     *pnNumConverted)
{
    D_UINT8    *pSrcStr     = (D_UINT8*)pMultiByteStr;
    D_BOOL      fCountOnly  = FALSE;
    int         nSrcIndex   = 0;
    unsigned    nOutputLen  = 0;
    DCLSTATUS   dclStat     = DCLSTAT_SUCCESS;
    D_UINT16    uResult     = 0;
    unsigned    nShiftVal;
    unsigned    nFinalMask;

    /*  validate the parameters
    */
    if (!pMultiByteStr || iMultiByteStrLen == 0 || !pnNumConverted ||
        (nWideCharBufLimit != 0 && !pWideCharBuffer) )
    {
        return DCLSTAT_BADPARAMETER;
    }

    /*  does the source string length need to be calculated?
    */
    if (iMultiByteStrLen == -1)
    {
        /*  get the length of the string and include the null character
        */
        iMultiByteStrLen = DclStrLen(pMultiByteStr) + 1;
    }

    /*  See if we are converting or determining how much space
        is needed to convert
    */
    if ( nWideCharBufLimit == 0 )
    {
        /*  set the flag to signal that we are just counting
            the space and not converting
        */
        fCountOnly = TRUE;
    }

    /*  for each src character, we must determine if this is a one,
        two or three byte UTF8 character sequence.
    */
    for (nSrcIndex=0 ; nSrcIndex < iMultiByteStrLen; )
    {
        int nMBSeqLen = *pSrcStr < 0x80 ? 0 : UTF8SequenceLength[*pSrcStr & 0x7F];

        /*  clear result placeholder
        */
        uResult = 0;

        /*  check to see if we have reached the maximum length of the
            destination buffer to prevent overflowing the destination buffer.
        */
        if ( fCountOnly == FALSE && nOutputLen >= nWideCharBufLimit )
        {
            /* overflow detected
            */
            dclStat = DCLSTAT_LIMITREACHED;
            break;
        }

        /*  are there enough characters for this
            multibyte sequence?
        */
        if (nMBSeqLen+nSrcIndex >= iMultiByteStrLen)
        {
            dclStat = DCLSTAT_LIMITREACHED;
            break;
        }

        /*  is this sequence a valid multibyte sequence?
        */
        if (!IsLegalMultiByte((const char *)pSrcStr, nMBSeqLen+1))
        {
            dclStat = DCLSTAT_BADMULTIBYTECHAR;
            break;
        }

        DclAssert(nMBSeqLen >= 0);
        DclAssert(nMBSeqLen <= 2);

        nShiftVal = 0x1F;
        nFinalMask = 0x7F;

        switch (nMBSeqLen)
        {
            case 2:
                uResult += (D_UINT16)(*pSrcStr++ & 0x0F) << 12;
                nShiftVal = 0x3F;
                nSrcIndex++;

            case 1:
                uResult += (D_UINT16)(*pSrcStr++ & nShiftVal) << 6;
                nFinalMask = 0x3F;
                nSrcIndex++;

            case 0:
                uResult += (D_UINT16)(*pSrcStr++ & nFinalMask);
                nSrcIndex++;
        }

        if (fCountOnly == FALSE)
            pWideCharBuffer[nOutputLen] = uResult;

        nOutputLen++;

    }

    /*  return the number of bytes converted or would be converted
    */
    *pnNumConverted = nOutputLen;

    return dclStat;

}


/*-------------------------------------------------------------------
    Local: IsLegalMultiByte()

    Determine if the specified multi-byte sequence is valid.

    Parameters:
        pSource - Pointer to the multibyte source string to validate
        nLength - Length of the multibyte sequence to validate.
                  Supported lengths are 1 through 3.  All other
                  lengths are not valid and this function will return
                  FALSE.

    Return Value:
        Returns TRUE if the multibyte sequence is valid, or FALSE
        otherwise.
-------------------------------------------------------------------*/
static D_BOOL IsLegalMultiByte(
    const char             *pSource,
    unsigned                nLength)
{
    unsigned char           cUTF8Val;
    const unsigned char    *pSrcPtr = (const unsigned char *)pSource + nLength;

    switch (nLength)
    {
        default:
            return FALSE;

        case 3:
            if ((cUTF8Val = (*--pSrcPtr)) < 0x80 || (cUTF8Val > 0xBF))
                return FALSE;
        case 2:
            if ((cUTF8Val = (*--pSrcPtr)) < 0x80 || (cUTF8Val > 0xBF))
                return FALSE;

            switch ((unsigned char)*pSource)
            {
                case 0xE0:
                    if (cUTF8Val < 0xA0)
                        return FALSE;
                    break;

                /*  This code is manages the UNICODE values 0xD800 to 0xDFFFF
                    which are reserved for use with UTF16.  Microsoft currently
                    allows these codes to be converted from UNICODE to UTF8
                    and then back again.  See RFC3629 Section 3 more more
                    information about this range of UTF8 characters

                case 0xED:
                    if (cUTF8Val > 0x9F)
                        return FALSE;
                    break;
                */

                default:
                    if (cUTF8Val < 0x80)
                        return FALSE;
            }
        case 1:
            if ((unsigned char)*pSource >= 0x80 && (unsigned char)*pSource < 0xC2)
                return FALSE;
    }

    if ((unsigned char)*pSource >= 0xF0)
        return FALSE;

    return TRUE;
}



