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

    This module contain functions for manipulating bit values.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dlbit.c $
    Revision 1.6  2010/10/18 03:16:57Z  garyp
    Added bitmap range functions.
    Revision 1.5  2010/04/18 20:41:31Z  garyp
    Documentation updated -- no functional changes.
    Revision 1.4  2009/06/28 01:02:19Z  garyp
    Added DclBitCountArray().
    Revision 1.3  2007/12/18 03:50:01Z  brandont
    Updated function headers.
    Revision 1.2  2007/11/03 23:31:13Z  Garyp
    Added the standard module header.
    Revision 1.1  2007/10/27 16:43:58Z  Garyp
    Initial revision
    ---------------------
    Bill Roman 2004-07-19
---------------------------------------------------------------------------*/

#include <dcl.h>


/*-------------------------------------------------------------------
    Public: DclBitCount()

    Determines the number of "one" bits in a 32-bit value.

    Parameters:
        ulValue - The value to examine

    Return Value:
        Returns a count of the number of bits which are set.
-------------------------------------------------------------------*/
unsigned DclBitCount(
    D_UINT32            ulValue)
{
    register unsigned   nn = 0;
    register D_UINT32   vv;

    for(vv = ulValue; vv != 0; vv >>= 1)
    {
        if(vv & 1)
            nn++;
    }

    return nn;
}


/*-------------------------------------------------------------------
    Public: DclBitCountArray()

    Count the number of one bits, within an array of bits.  This 
    function is designed to operate on bits stored in byte arrays.

    *Note* -- The DCLBITGET(), DCLBITSET(), and DCLBITCLEAR() macros
              make no adjustments for byte-ordering.  This means that
              when using datatypes wider than a byte on a big-endian
              system, that this function will not work properly
              unless the total count of bits is evenly divisible by
              the number of bits in the integral data type.

    Parameters:
        pBuffer - A pointer to an array of D_BYTEs (D_BUFFER)
        nLen    - The size of the array in bits.  The last D_BYTE
                  in the array must have zeros in all bits beyond
                  nLen.

    Return Value:
        The number of bits which are set.
-------------------------------------------------------------------*/
unsigned DclBitCountArray(
    const D_BUFFER *pBuffer,
    unsigned        nLen)
{
    #define         D_BYTE_WIDTH    (8)
    unsigned        nCount = 0;

    while(nLen)
    {
        D_BYTE byt = *pBuffer++;

        /*  Process 8 bits at a time
        */
        while(byt)
        {
            nCount++;
            byt ^= (byt & (~byt + 1));      /* clear lowest bit */
        }

        if(nLen > D_BYTE_WIDTH)
            nLen -= D_BYTE_WIDTH;
        else
            nLen = 0;
    }

    return nCount;
}


/*-------------------------------------------------------------------
    Public: DclBitHighest()

    Find the highest set bit in a 32-bit value.

    Parameters:
        ulValue - The number in which to find the highest bit

    Return Value:
        Zero if ulValue is zero.  Otherwise, the index of the highest
        set bit, with one representing the lowest bit.
-------------------------------------------------------------------*/
unsigned DclBitHighest(
    D_UINT32            ulValue)
{
    register unsigned   nn = 0;
    register D_UINT32   vv;

    for(vv = ulValue; vv != 0; vv >>= 1)
    {
        nn++;
    }

    return nn;
}


/*-------------------------------------------------------------------
    Public: DclBitmapRangeGet()

    Return the number of matching sequential bits starting at the 
    extent specified by pExtent.  The state of the bits is returned
    in pnValue.

    Parameters:
        pBuffer  - A pointer to the bitmap to examine
        ulOffset - The offset of the first bit to examine.
        ulCount  - The number of bits to examine.
        pnValue  - A pointer to an unsigned variable to receive
                   the state of the bits.

    Return Value:
        Returns the count of matching bits.
-------------------------------------------------------------------*/
D_UINT32 DclBitmapRangeGet(
    const D_BUFFER *pBuffer, 
    D_UINT32        ulOffset,
    D_UINT32        ulCount,
    unsigned       *pnValue) 
{
    D_UINT32        ulBit;
    D_UINT32        ulLength = 1;
    
    DclAssertReadPtr(pBuffer, 0);
    DclAssertWritePtr(pnValue, sizeof(*pnValue));
    DclAssert(ulCount);

    *pnValue = DCLBITGET(pBuffer, ulOffset);

    for(ulBit = ulOffset + 1; 
        ulLength < ulCount; 
        ulBit++)
    {
        /*  If the next bit does not match the first bit, we're done...
        */
        if(DCLBITGET(pBuffer, ulBit) != (int)*pnValue)
            break;

        ulLength++;
    }

    return ulLength;
}


/*-------------------------------------------------------------------
    Public: DclBitmapRangeClear()

    Clear a range of bits in a bitmap to 0.

    Parameters:
        pBuffer  - A pointer to the bitmap to examine
        ulOffset - The offset of the first bit to clear.
        ulCount  - The number of bits to clear.

    Return Value:
        None.
-------------------------------------------------------------------*/
void DclBitmapRangeClear(
    D_BUFFER       *pBuffer, 
    D_UINT32        ulOffset,
    D_UINT32        ulCount)
{
    D_UINT32        ulBit;

    DclAssertWritePtr(pBuffer, 0);
    DclAssert(ulCount);

    for(ulBit = ulOffset; 
        ulBit < ulOffset + ulCount; 
        ulBit++)
    {
        DCLBITCLEAR(pBuffer, ulBit);
    }

    return;
}


/*-------------------------------------------------------------------
    Public: DclBitmapRangeSet()

    Set a range of bits in a bitmap to 1.

    Parameters:
        pBuffer  - A pointer to the bitmap to examine
        ulOffset - The offset of the first bit to set.
        ulCount  - The number of bits to set.

    Return Value:
        None.
 -------------------------------------------------------------------*/
void DclBitmapRangeSet(
    D_BUFFER       *pBuffer, 
    D_UINT32        ulOffset,
    D_UINT32        ulCount)
{
    D_UINT32        ulBit;
    
    DclAssertWritePtr(pBuffer, 0);
     DclAssert(ulCount);

    for(ulBit = ulOffset; 
        ulBit < ulOffset + ulCount; 
        ulBit++)
    {
        DCLBITSET(pBuffer, ulBit);
    }

    return;
}

