/*---------------------------------------------------------------------------
               ----> DO NOT REMOVE THE FOLLOWING NOTICE <----

                  Copyright (c) 1993 - 2011 Datalight, Inc.
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

    This module contains functions for calculating ratios and percentages,
    without using floating point math.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dlratio.c $
    Revision 1.6  2011/03/02 01:57:46Z  garyp
    Enhanced DclRatio() and DclRatio64() to allow zero decimal places
    to be specified.  Fixed documentation problems.
    Revision 1.5  2010/08/04 00:10:32Z  garyp
    Updated to work around the "VA64BUG" issue.
    Revision 1.4  2009/11/05 02:23:46Z  garyp
    Added DclRatio64().
    Revision 1.3  2007/12/18 03:54:55Z  brandont
    Updated function headers.
    Revision 1.2  2007/11/03 23:31:13Z  Garyp
    Added the standard module header.
    Revision 1.1  2007/04/22 19:42:12Z  Garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <dcl.h>


/*-------------------------------------------------------------------
    Public: DclRatio()

    Return a ratio value formatted as a floating point string
    accurate to the specified number of decimal places.  This
    function is similar to DclRatio64() but with smaller data
    types.  See that function for a detailed description.

    Parameters:
        pBuffer    - A pointer to the buffer in which to store the
                     null terminated results.
        nBufferLen - The length of the output buffer.
        ulDividend - The "total" value to divide.
        ulDivisor  - The portion of ulDividend for which to calculate
                     the ratio (may be greater than ulDividend).
        nDecPlaces - The number of decimal places to use, from 0 to 9.

    Return Value:
        Returns pBuffer.
-------------------------------------------------------------------*/
char * DclRatio(
    char       *pBuffer,
    unsigned    nBufferLen,
    D_UINT32    ulDividend,
    D_UINT32    ulDivisor,
    unsigned    nDecPlaces)
{
    DclAssert(pBuffer);
    DclAssert(nBufferLen);
    DclAssert(nDecPlaces <= 9);

    return DclRatio64(pBuffer, nBufferLen, ulDividend, ulDivisor, nDecPlaces);
}


/*-------------------------------------------------------------------
    Public: DclRatio64()

    Return a ratio value formatted as a floating point string
    accurate to the specified number of decimal places.  The 
    function exists to provide floating point style output 
    without using any actual floating point types.

    This function may scale the numbers down to avoid overflow
    at the high end.  Likewise, potential divide-by-zero errors
    are internally avoided.  Here are some examples:

    <pre>
        Dividend  Divisor   DecPlaces   Result
        --------  -------   ---------   ------
        12133     28545     2           "0.42"
        1539      506       2           "3.04"
    </pre>

    To get a number formatted as a percentage, take the take the
    portion of the total (normally the smaller part), multiply it
    by 100, and pass it to this function as the Dividend, pass the
    "total" value to this function as the Divisor, and specify the
    desired number of decimal places.

    For example, if you have a disk format overhead value of N blocks
    out of a total of Y blocks on the disk, and you want to display 
    the format overhead as a percentage, you would use a function call
    similar to:

    <pre>
        DclRatio64(szBuffer, sizeof(szBuffer), N*100, Y, 2);
    </pre>

    If N=145, Y=4096, and decimal places is 2, the resulting output
    would be "3.54".

    The string returned will always be null-terminated, even if it
    means stomping on the least significant decimal digit.

    If either the dividend or divisor values are zero, the string
    "0.0" will be returned, with the prescribed number of decimal
    places.

    *Note* -- This function has "reasonable" limits which meet the
              needs of the various supplemental utilities which use
              this function.  Extremely large ratios, or using many
              decimal places may not function as desired.

    Parameters:
        pBuffer     - A pointer to the buffer in which to store the
                      null terminated results.
        nBufferLen  - The length of the output buffer.
        ullDividend - The "total" value to divide.
        ullDivisor  - The portion of ulDividend for which to calculate
                      the ratio (may be greater than ulDividend).
        nDecPlaces  - The number of decimal places to use, from 0 to 9.

    Return Value:
        Returns pBuffer.
-------------------------------------------------------------------*/
char * DclRatio64(
    char       *pBuffer,
    unsigned    nBufferLen,
    D_UINT64    ullDividend,
    D_UINT64    ullDivisor,
    unsigned    nDecPlaces)
{
    DclAssert(pBuffer);
    DclAssert(nBufferLen);
    DclAssert(nDecPlaces <= 9);     /* arbitrary */

    if(ullDivisor && ullDividend)
    {
        unsigned    ii;
        D_UINT32    ulFactor = 1;
        D_UINT64    ullDecimal;
        D_UINT64    ullTemp;

        for(ii=1; ii<=nDecPlaces; ii++)
            ulFactor *= 10;

        ullDecimal = DclMulDiv64(ullDividend, ulFactor, ullDivisor);

        /*  Shouldn't really be calling this function in a situation
            where we can overflow at this point...
        */            
        DclAssert(ullDecimal != D_UINT64_MAX);

        if(ullDivisor <= ullDividend)
            ullDecimal %= ulFactor;

        ullTemp = ullDividend / ullDivisor;

        if(nDecPlaces)
            DclSNPrintf(pBuffer, nBufferLen, "%llU.%0*llU", VA64BUG(ullTemp), nDecPlaces, VA64BUG(ullDecimal));
        else
            DclSNPrintf(pBuffer, nBufferLen, "%llU", VA64BUG(ullTemp));
    }
    else
    {
        /*  If either the dividend or divisor is zero, then just output
            a "0.0" string with the prescribed number of decimal places.
        */
        if(nDecPlaces)
            DclSNPrintf(pBuffer, nBufferLen, "0.%0*u", nDecPlaces, 0);
        else
            DclStrNCpy(pBuffer, "0", nBufferLen);
    }

    /*  Ensure the returned buffer is always null-terminated
    */
    pBuffer[nBufferLen-1] = 0;

    return pBuffer;
}



