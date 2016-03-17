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

    This module contains functions formatting scaled numbers.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dlscale.c $
    Revision 1.5  2010/08/04 00:10:32Z  garyp
    Updated to work around the "VA64BUG" issue.
    Revision 1.4  2010/01/23 18:35:56Z  garyp
    Updated DclScaleItems() to return an empty buffer if either of the
    two operands are zero.  Docs adjusted accordingly.
    Revision 1.3  2010/01/07 02:34:38Z  garyp
    Added DclScaleItems().
    Revision 1.2  2009/02/08 04:45:34Z  garyp
    Updated to deal with the MKS branch first conundrum.
---------------------------------------------------------------------------*/

#include <dcl.h>

#define KILO                    (1024)
#define MEGA                    (KILO * KILO)
#define GIGA                    (KILO * KILO * KILO)
#define TERA                    (UINT64SUFFIX(1099511627776))
#define PETA                    (UINT64SUFFIX(1125899906842624))

#define GIGA_NEXT (UINT64SUFFIX(32212254720))    /* GIGA * NEXT_LEVEL_THRESHOLD */

#define NEXT_LEVEL_THRESHOLD    (30)


/*-------------------------------------------------------------------
    Public: DclScaleBytes()

    This function formats a number, which is supplied as a count of
    bytes, but scales it to KiloBytes, MegaBytes or GigaBytes if it
    is nice and round, or if it exceeds a 30x factor over the lower
    threshold.  The number is output in the forms: "nnn", "nnn KB",
    "~nnn KB", where "KB" could be "MB" or "GB".

    Even if a number is not exactly evenly divisible by KB, MB or
    GB, it may still be formatted and displayed in that fashion.
    For example, if a byte value is passed in which is 33.6 KB,
    even though the value is not evenly divisible by KB, because it
    is over the 30x threshold, it will be rounded up and displayed
    as "~34 KB".

    For any byte value which is under 30KB, and is not evenly
    divisible by 1024, it will be formatted as "nnn", without any
    suffix.

    If the buffer size is not large enough, then the buffer will
    contain an empty string.

    Parameters:
        ulByteValue - The value to scale, in bytes
        pszBuffer   - The buffer to contain the scaled string
        nBufferSize - The size of the output buffer

    Return Value:
        Returns pszBuffer.
-------------------------------------------------------------------*/
char * DclScaleBytes(
    D_UINT32        ulByteValue,
    char           *pszBuffer,
    unsigned        nBufferSize)
{
    DclAssert(pszBuffer);

    if((ulByteValue & (KILO-1)) == 0)
        return DclScaleKB(ulByteValue / KILO, pszBuffer, nBufferSize);

    if(ulByteValue >= KILO * NEXT_LEVEL_THRESHOLD)
    {
        if(DclScaleKB((ulByteValue + KILO/2) / KILO, pszBuffer, nBufferSize))
        {
            if(DclStrLen(pszBuffer) < nBufferSize - 1)
            {
                unsigned nn = nBufferSize - 1;

                /*  This is a good candidate for DclMemMove()
                */
                while(nn)
                {
                    pszBuffer[nn] = pszBuffer[nn-1];
                    nn--;
                }

                pszBuffer[0] = '~';

                return pszBuffer;
            }

            /*  Not enough room to stuff in the '~', and we know that
                we need one, or we would not be in this clause.  Fail
                the call.
            */
            pszBuffer[0] = 0;
        }

        /*  If we get here, either the initial scaling failed,
            or there was not enough room to stuff in the '~'.
            In either case, the string has already been set to
            NULL, so just return.
        */
        return pszBuffer;
    }

    /*  We've handled any cases which are evenly divisible by KiloBytes,
        and cases which are over the 30KB threshold -- evenly divisible
        or not.  If we get here, just format the number and we're done.
    */

    if(DclSNPrintf(pszBuffer, nBufferSize, "%lU", ulByteValue) == -1)
    {
        /*  Buffer size was too small, don't output anything.
        */
        pszBuffer[0] = 0;
    }

    return pszBuffer;
}


/*-------------------------------------------------------------------
    Public: DclScaleKB()

    This function formats a number, which is supplied as a count
    of KiloBytes, but scales it to MegaBytes or GigaBytes if it
    is nice and round, or if it exceeds a 30x factor over the
    lower threshold.  The number is output in the forms: "nnn KB"
    or "~nnn KB".

    Even if a number is not exactly evenly divisible by MB or GB,
    it may still be formatted and displayed in that fashion.  For
    example, if a KB value is passed in which is 33.6 MB, even
    though the value is not evenly divisible by MB, because it is
    over the 30x threshold, it will be rounded up and displayed
    as "~34 MB".

    If the buffer size is not large enough, then the buffer will
    contain an empty string.

    Parameters:
        ulKBValue   - The value to scale, in KB
        pszBuffer   - The buffer to contain the scaled string
        nBufferSize - The size of the output buffer

    Return Value:
        Returns pszBuffer.
-------------------------------------------------------------------*/
char * DclScaleKB(
    D_UINT32        ulKBValue,
    char           *pszBuffer,
    unsigned        nBufferSize)
{
    const char     *pszLabel = "KB";
    const char      szFormat[] = "~%lU %s";
    const char     *pszFormat = &szFormat[0];

    DclAssert(pszBuffer);

    /*  Determine if and how to appropriately scale the number.
    */
    if(!ulKBValue)
    {
        /*  If the number is 0, just format as KB.
        */
        pszFormat++;
    }
    else if(!(ulKBValue & (MEGA - 1)))
    {
        /*  An even number of GB.  Skip past the ~ since this will
            not be an approximation.
        */
        pszFormat++;
        ulKBValue /= MEGA;
        pszLabel = "GB";
    }
    else if(ulKBValue > MEGA * NEXT_LEVEL_THRESHOLD)
    {
        /*  Round up to an approximate number of GB.
        */
        ulKBValue = (ulKBValue + (MEGA / 2)) / MEGA;
        pszLabel = "GB";
    }
    else if(!(ulKBValue & (KILO - 1)))
    {
        /*  An even number of MB.  Skip past the ~ since this will
            not be an approximation.
        */
        pszFormat++;
        ulKBValue /= KILO;
        pszLabel = "MB";
    }
    else if(ulKBValue > KILO * NEXT_LEVEL_THRESHOLD)
    {
        /*  Round up to an approximate number of MB.
        */
        ulKBValue = (ulKBValue + (KILO / 2)) / KILO;
        pszLabel = "MB";
    }
    else
    {
        /*  Skip past the ~ since this will not be an approximation
        */
        pszFormat++;
    }

    /*  Format the string with the size scaled appropriately.
    */
    if(DclSNPrintf(pszBuffer, nBufferSize, pszFormat, ulKBValue, pszLabel) == -1)
    {
        /*  Buffer size was too small, don't output anything.
        */
        pszBuffer[0] = 0;
    }

    return pszBuffer;
}


/*-------------------------------------------------------------------
    Public: DclScaleItems()

    Format a number, which is supplied as a count of items, each a
    specific bytes length in size, and scale the total byte count to
    Peta, Tera, Giga, Mega, Kilo, or Bytes.

    The number is output in the forms: "nnn KB" or "~nnn KB".

    Even if a number is not exactly evenly divisible, it may still
    be formatted and displayed in that fashion.  For example, if a
    byte value is passed in which is 33.6 MB, even though the value
    is not evenly divisible by MB, because it is over the 30x 
    threshold, it will be rounded up and displayed as "~34 MB".

    If the buffer size is not large enough, then the buffer will
    contain an empty string.

    If either ullItems or ulItemSize is zero, then the output buffer
    will contain an empty string.

    Parameters:
        ullItems    - The number of items
        ulItemSize  - The size of each item, in bytes
        pszBuffer   - The buffer to contain the scaled string
        nBufferSize - The size of the output buffer

    Return Value:
        Returns pszBuffer.
-------------------------------------------------------------------*/
char * DclScaleItems(
    D_UINT64        ullItems,
    D_UINT32        ulItemSize,
    char           *pszBuffer,
    size_t          nBufferSize)
{
    const char     *pszLabel = "Bytes";
    const char      szFormat[] = "~%llU %s";
    const char     *pszFormat = &szFormat[0];
    D_UINT64        ullTotalBytes = ullItems * ulItemSize;

    DclAssert(nBufferSize);
    DclAssertWritePtr(pszBuffer, nBufferSize);

  #if D_DEBUG
    /*  Preprocessors can't (generally) handle 64-bit constant math
        without the proper suffixes (and some can't handle it even
        with the suffixes), so do these check at run-time.
    */        
    {
        D_UINT64    ullTemp = GIGA;

        ullTemp *= KILO;
        DclAssert(ullTemp == TERA);

        ullTemp *= KILO;
        DclAssert(ullTemp == PETA);

        ullTemp = GIGA;
        ullTemp *= NEXT_LEVEL_THRESHOLD;
        DclAssert(ullTemp == GIGA_NEXT);
     }
  #endif

    if(!ullItems || !ulItemSize)
    {
        pszBuffer[0] = 0;
        return pszBuffer;
    }
  
    /*  Determine if and how to appropriately scale the number.
    */
    if(!ullTotalBytes)
    {
        /*  If the number is 0, just format as bytes
        */
        pszFormat++;
    }
    else if(!(ullTotalBytes & (PETA - 1)))
    {
        /*  An even number of PB.  Skip past the ~ since this will
            not be an approximation.
        */
        pszFormat++;
        ullTotalBytes /= PETA;
        pszLabel = "PB";
    }
    else if(ullTotalBytes > PETA * NEXT_LEVEL_THRESHOLD)
    {
        /*  Round up to an approximate number of PB.
        */
        ullTotalBytes = (ullTotalBytes + (PETA / 2)) / PETA;
        pszLabel = "PB";
    }
    else if(!(ullTotalBytes & (TERA - 1)))
    {
        /*  An even number of TB.  Skip past the ~ since this will
            not be an approximation.
        */
        pszFormat++;
        ullTotalBytes /= TERA;
        pszLabel = "TB";
    }
    else if(ullTotalBytes > TERA * NEXT_LEVEL_THRESHOLD)
    {
        /*  Round up to an approximate number of TB.
        */
        ullTotalBytes = (ullTotalBytes + (TERA / 2)) / TERA;
        pszLabel = "TB";
    }
    else if(!(ullTotalBytes & (GIGA - 1)))
    {
        /*  An even number of GB.  Skip past the ~ since this will
            not be an approximation.
        */
        pszFormat++;
        ullTotalBytes /= GIGA;
        pszLabel = "GB";
    }
    else if(ullTotalBytes > GIGA_NEXT)
    {
        /*  Round up to an approximate number of GB.
        */
        ullTotalBytes = (ullTotalBytes + (GIGA / 2)) / GIGA;
        pszLabel = "GB";
    }
    else if(!(ullTotalBytes & (MEGA - 1)))
    {
        /*  An even number of GB.  Skip past the ~ since this will
            not be an approximation.
        */
        pszFormat++;
        ullTotalBytes /= MEGA;
        pszLabel = "MB";
    }
    else if(ullTotalBytes > MEGA * NEXT_LEVEL_THRESHOLD)
    {
        /*  Round up to an approximate number of MB.
        */
        ullTotalBytes = (ullTotalBytes + (MEGA / 2)) / MEGA;
        pszLabel = "MB";
    }
    else if(!(ullTotalBytes & (KILO - 1)))
    {
        /*  An even number of KB.  Skip past the ~ since this will
            not be an approximation.
        */
        pszFormat++;
        ullTotalBytes /= KILO;
        pszLabel = "KB";
    }
    else if(ullTotalBytes > KILO * NEXT_LEVEL_THRESHOLD)
    {
        /*  Round up to an approximate number of KB.
        */
        ullTotalBytes = (ullTotalBytes + (KILO / 2)) / KILO;
        pszLabel = "KB";
    }
    else
    {
        /*  Skip past the ~ since this will not be an approximation
        */
        pszFormat++;
    }

    /*  Format the string with the size scaled appropriately.
    */
    if(DclSNPrintf(pszBuffer, nBufferSize, pszFormat, VA64BUG(ullTotalBytes), pszLabel) == -1)
    {
        /*  Buffer size was too small, don't output anything.
        */
        pszBuffer[0] = 0;
    }

    return pszBuffer;
}

