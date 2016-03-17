/*---------------------------------------------------------------------------
               ----> DO NOT REMOVE THE FOLLOWING NOTICE <----

                  Copyright (c) 1993 - 2010 Datalight, Inc.
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

    This module contain functions for formatting bit-flag names.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dlfmtbitflags.c $
    Revision 1.2  2010/01/07 02:36:40Z  garyp
    Fixed DclFormatBitFlags() to behave as documented in the event
    that there are no bits set.  Clarified the documentation.
    Revision 1.1  2009/12/03 02:50:44Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <dcl.h>


/*-------------------------------------------------------------------
    Protected: DclFormatBitFlags()

    Parse a bitmapped flags value and build a string containing the
    bit names, from the ppszNameList array.  The first name in the
    list corresponds to bit 0, etc.  If the named bits are sparse, 
    any unused bits should be designated with a NULL pointer.

    pszBuffer must point to an already initialized string of zero or
    more bytes.  The output will be concatenated onto this string.
    nBuffLen must point to the full amount of space available,
    including any which pre-exists in pszBuffer.

    Each name in the output string will be separated by a space, with
    the final name followed by a trailing space.  In the event that
    no bits are set, then a single, null-terminated space will be
    concatenated onto pszBuffer.

    Any bit which is set which does not have a name, will be output
    in the form "Bit-n".  To allow an undocumented bit name which will
    be silently ignored, set the name to an empty string.
 
    Parameters:
        pszBuffer    - A pointer to the output buffer.
        nBuffLen     - The output buffer length.
        ppszNameList - An array of pointers to null-terminated bit
                       names.
        nNames       - The number of names in ppszNameList.
        ulFlags      - The bitmapped flags value to parse.
 
    Return Value:
        Returns TRUE if all is good, or FALSE if the output buffer
        is not large enough.  In the event that the output buffer
        is too small, the resulting output will always be null-
        terminated.
-------------------------------------------------------------------*/
D_BOOL DclFormatBitFlags(
    char           *pszBuffer,
    size_t          nBuffLen,
    const char    **ppszNameList,
    unsigned        nNames,
    D_UINT32        ulFlags)
{
    unsigned        nBit; 

    DclAssert(nNames);
    DclAssert(nBuffLen >= 2);
    DclAssertWritePtr(pszBuffer, nBuffLen);
    DclAssertReadPtr(ppszNameList, sizeof(*ppszNameList) * nNames);

    if(!ulFlags)
    {
        /*  If there are no bits set, then output a single space.
        */
        DclStrNCat(pszBuffer, " ", nBuffLen - DclStrLen(pszBuffer));
        return TRUE;
    }
    
    for(nBit=0; nBit<sizeof(ulFlags) * CHAR_BIT; nBit++)
    {
        if(ulFlags & (1 << nBit))
        {
            size_t nRemaining = nBuffLen - DclStrLen(pszBuffer);

            /*  If the bit is set, and the name is not NULL.
            */
            if(nBit < nNames && ppszNameList[nBit])
            {
                /*  Only output something if this is not an empty string
                */
                if(*(ppszNameList[nBit]))
                {
                    DclStrNCat(pszBuffer, ppszNameList[nBit], nRemaining);
                    DclStrNCat(pszBuffer, " ", nRemaining);
                }
            }
            else
            {
                int iLen = DclSNPrintf(&pszBuffer[DclStrLen(pszBuffer)], nRemaining, "Bit-%u ", nBit);
                if(iLen == -1)
                {
                    pszBuffer[nBuffLen-1] = 0;
                    return FALSE;
                }
            }

            /*  Quit if there's no more room
            */
            if(DclStrLen(pszBuffer) >= nBuffLen)
            {
                /*  Should not ever happen with the protections above...
                */
                DclAssert(DclStrLen(pszBuffer) == nBuffLen);
                return FALSE;
            }
        }
    }                

    return TRUE;
}


