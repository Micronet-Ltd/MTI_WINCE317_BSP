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

    This module contains code to provide a hex dump capability, with a
    variety of formatting options.  While this code is compiled regardless
    whether building in RELEASE or DEBUG mode, typically it is only used
    for debugging, and would not be included in RELEASE images.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dlhexdmp.c $
    Revision 1.11  2009/06/24 22:13:32Z  garyp
    Documentation updates only -- no functional changes.
    Revision 1.10  2009/04/16 00:24:50Z  garyp
    Documentation updates -- no functional changes.
    Revision 1.9  2009/02/08 00:14:05Z  garyp
    Merged from the v4.0 branch.  Updated DclOutputString() to take an extra
    parameter.
    Revision 1.8  2008/05/03 19:41:29Z  garyp
    Changed D_UINT16 types to unsigned.  Updated to build cleanly when
    output is disabled.
    Revision 1.7  2008/05/02 19:32:13Z  brandont
    Added support for HEXDUMP_NORETURN.
    Revision 1.6  2007/12/18 20:37:42Z  brandont
    Updated function headers.
    Revision 1.5  2007/11/03 23:31:10Z  Garyp
    Added the standard module header.
    Revision 1.4  2007/01/03 22:13:33Z  Garyp
    Fixed so the ASCII display of '%' is disallowed.
    Revision 1.3  2007/01/02 23:18:30Z  Garyp
    Updated to support displaying ASCII characters for byte-oriented dumps.
    Changed the maximum number of elements on a line from 64 to 32.  Fixed
    to use DclOutputString() rather than DclPrintf() to avoid the inherent
    output length limitations in the latter.
    Revision 1.2  2006/11/29 23:18:47Z  Garyp
    Updated to allow a NULL data pointer since it may be legimately possible
    to dump something at address 0.
    Revision 1.1  2006/01/08 21:36:10Z  Garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <dcl.h>

#if DCLCONF_OUTPUT_ENABLED

#define MAXELEMENTS         (32)
#define MAXELEMENTWIDTH     (9)     /* D_UINT32 in hex, plus one space */
#define MAXOUTPUTSTRINGSIZE ((MAXELEMENTS * MAXELEMENTWIDTH) + 16)


/*-------------------------------------------------------------------
    Public: DclHexDump()

    Output data in hexadecimal format.  If the data is formatted with
    HEXDUMP_UINT8, the characters will be displayed in ASCII format
    as well, unless the HEXDUMP_NOASCII flag is used.

    Parameters:
        pszTitle - The output title.  May be NULL if the title is not
                   to be displayed.  If the title is not terminated
                   with a newline, the first line of the hex dump
                   will appear on the same line.
        fOptions - The flags to use.  One or more of the following
                   values:
                 + HEXDUMP_UINT8    - Format as unsigned 8-bit elements
                 + HEXDUMP_UINT16   - Format as unsigned 16-bit elements
                 + HEXDUMP_UINT32   - Format as unsigned 32-bit elements
                 + HEXDUMP_NOOFFSET - Don't output an offset
                 + HEXDUMP_NOASCII  - Don't output ASCII chars
                 + HEXDUMP_NORETURN - Don't output a trailing newline
        nWidth   - The number of elements to place on each line.
                   This can have a maximum value of 32.
        ulCount  - The total number of elements to display
        pData    - A pointer to the data to dump


    Return Value:
        Returns TRUE if successsful, otherwise FALSE.
-------------------------------------------------------------------*/
D_BOOL DclHexDump(
    const char *pszTitle,
    unsigned    fOptions,
    unsigned    nWidth,
    D_UINT32    ulCount,
    const void *pBuffer)
{
    unsigned    nElementSize = fOptions & HEXDUMP_SIZEMASK;
    char        szBuffer[MAXOUTPUTSTRINGSIZE];
    char        szElement[MAXELEMENTWIDTH];
    D_UINT32    ulOffset = 0;
    D_BUFFER   *pData = (D_BUFFER*)pBuffer;

    DclAssert(fOptions);
    DclAssert(nWidth);
    DclAssert(ulCount);

    if(pszTitle)
        DclOutputString(NULL, pszTitle);

    /*  Can't do anything if a count is not supplied
    */
    if(!ulCount)
        return FALSE;

    /*  Typically we should not be so tolerant of improperly specified
        parameters, but because (ostensibly) this code is only used
        during debugging, we're a little more lenient about this
        (nevertheless, we still assert about it up above). Therefore,
        use default width and size parameters if necessary.
    */
    if(!nElementSize)
        nElementSize = 1;

    if(nWidth > MAXELEMENTS)
        nWidth = MAXELEMENTS;

    if(!nWidth)
        nWidth = 16 / nElementSize;

    switch(nElementSize)
    {
        case 1:
            DclStrCpy(szElement, "%02X ");
            break;
        case 2:
            DclStrCpy(szElement, "%04X ");
            break;
        case 4:
            DclStrCpy(szElement, "%08lX ");
            break;
        default:
            /*  Only a cosmic ray should cause this to happen...
            */
            DclProductionError();
            return FALSE;
    }

    while(ulCount)
    {
        unsigned    kk;
        unsigned    nPos = 0;
        unsigned    nBasePos;
        unsigned    nThisWidth = (unsigned)DCLMIN(nWidth, ulCount);

        if(!(fOptions & HEXDUMP_NOOFFSET))
        {
            /*  Make the offset display only as wide as needs to be for the
                amount of data we are displaying.
            */
            if(ulCount * nElementSize > D_UINT16_MAX)
                nPos = DclSNPrintf(szBuffer, MAXOUTPUTSTRINGSIZE, "%08lX: ", ulOffset);
            else
                nPos = DclSNPrintf(szBuffer, MAXOUTPUTSTRINGSIZE, "%04lX: ", ulOffset);
        }

        /*  Save this offset in the event that we are outputting ASCII chars
        */
        nBasePos = nPos;

        /*  Initialize this area of the buffer to spaces in the event that
            we are only outputting a partial line.
        */
        if((nElementSize == 1) && (!(fOptions & HEXDUMP_NOASCII)))
            DclMemSet(&szBuffer[nBasePos], ' ', nWidth*4);

        for(kk=0; kk<nThisWidth; kk++)
        {
            if(nElementSize == 1)
            {
                D_UINT8     u8 = *(D_UINT8*)pData;
                char        tmpbuff[4];

                /*  Use an intermediate buffer so that the final NULL output,
                    for the final element processed by DclSNPrintf() will not
                    trash the first ASCII character (if used).
                */
                DclSNPrintf(tmpbuff, sizeof(tmpbuff), szElement, u8);

                DclMemCpy(&szBuffer[nPos], tmpbuff, 3);

                nPos += 3;

                if(!(fOptions & HEXDUMP_NOASCII))
                {
                    /*  Special case for a percent sign because some of the low
                        level output functions will use printf() functionality.
                    */
                    if(u8 == '%')
                        u8 = '.';

                    szBuffer[nBasePos + (nWidth*3) + kk] = ((u8 >= 32 && u8 < 128) ? u8 : '.');
                }
            }
            else if(nElementSize == 2)
            {
                nPos += DclSNPrintf(&szBuffer[nPos], MAXOUTPUTSTRINGSIZE - nPos, szElement, *(D_UINT16*)pData);
            }
            else if(nElementSize == 4)
            {
                nPos += DclSNPrintf(&szBuffer[nPos], MAXOUTPUTSTRINGSIZE - nPos, szElement, *(D_UINT32*)pData);
            }

            pData += nElementSize;
            ulOffset += nElementSize;
        }

        /*  Adjust the ending position if writing out ASCII characters
        */
        if((nElementSize == 1) && (!(fOptions & HEXDUMP_NOASCII)))
            nPos = nBasePos + (nWidth*4) + 1;

        /*  Reality check to make sure we did not overflow our buffer
        */
        DclAssert(nPos < MAXOUTPUTSTRINGSIZE);

        /*  Tack on a newline and a NULL
        */
        if(fOptions & HEXDUMP_NORETURN)
        {
            szBuffer[nPos-1] = 0;
            szBuffer[nPos-0] = 0;
        }
        else
        {
            szBuffer[nPos-1] = '\n';
            szBuffer[nPos-0] = 0;
        }

        /*  Output the string
        */
        DclOutputString(NULL, szBuffer);

        ulCount -= nThisWidth;
    }

    return TRUE;
}


#endif  /* DCLCONF_OUTPUT_ENABLED */


