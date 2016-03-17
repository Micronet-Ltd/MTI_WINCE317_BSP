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

    Byte-order conversion operations.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dlend.c $
    Revision 1.7  2009/06/28 01:01:04Z  garyp
    Updated to use the new assert macros.  Documentation updated -- no
    functional changes.
    Revision 1.6  2008/05/29 19:53:45Z  garyp
    Merged from the WinMobile branch.
    Revision 1.5.1.2  2008/05/29 19:53:45Z  garyp
    Modified the data type used in DclCopyLittleEndianToEmulated64() so
    that calling the function does not require a typecast.
    Revision 1.5  2007/12/18 04:06:41Z  brandont
    Updated function headers.
    Revision 1.4  2007/11/03 23:31:13Z  Garyp
    Added the standard module header.
    Revision 1.3  2006/10/05 23:05:00Z  Garyp
    Added new functions to support proper handling of emulated 64-bit types.
    Revision 1.2  2006/10/02 06:35:06Z  Garyp
    Added debug code and cleaned up the docs.
---------------------------------------------------------------------------*/

#include <dcl.h>


/*-------------------------------------------------------------------
    Public: DclReverseInPlace()

    Reverses the byte order of the data pointed to by pData.

    Parameters:
        pData   - A pointer into the data to reverse
        uLength - The number of bytes to reverse

    Return Value:
        None.
-------------------------------------------------------------------*/
void DclReverseInPlace(
    void           *pData,
    D_UINT16        uLength)
{
    D_UINT16        uIndex;
    D_UINT8         ucTemp;
    D_UINT8        *pucDat = pData;

    DclAssertWritePtr(pData, uLength);
    DclAssert(uLength);
    DclAssert(uLength % 2 == 0);
  #if !DCL_NATIVE_64BIT_SUPPORT
    /* Use 64-bit macros for D_UINT64
    */
    DclAssert(uLength <= sizeof(D_UINT32));
  #endif

    /*  Start at offset 0
    */
    uIndex = 0;
    --uLength;
    while(uIndex < uLength)
    {
        /*  Swap the first and last byte
        */
        ucTemp = pucDat[uIndex];
        pucDat[uIndex] = pucDat[uLength];
        pucDat[uLength] = ucTemp;

        /*  Goto next two bytes
        */
        --uLength;
        ++uIndex;
    }

    return;
}


/*-------------------------------------------------------------------
    Public: DclCopyAndReverse()

    Copies the source data to the destination buffer, reversing
    the bytes along the way.

    Parameters:
        pDest    - A pointer into the destination buffer
        pSrc     - A pointer into the source data
        uLength  - The number of bytes to process

    Return Value:
        None.
-------------------------------------------------------------------*/
void DclCopyAndReverse(
    void           *pDest,
    const void     *pSrc,
    D_UINT16        uLength)
{
    const D_UINT8  *pucSrc = pSrc;
    D_UINT8        *pucDst = pDest;

    DclAssertReadPtr(pSrc, uLength);
    DclAssertWritePtr(pDest, uLength);
    DclAssert(uLength);
  #if !DCL_NATIVE_64BIT_SUPPORT
    /* Use 64-bit macros for D_UINT64
    */
    DclAssert(uLength <= sizeof(D_UINT32));
  #endif

    while(uLength)
    {
        *pucDst = pucSrc[uLength - 1];
        pucDst++;
        uLength--;
    }

    return;
}


#if !DCL_NATIVE_64BIT_SUPPORT

/*-------------------------------------------------------------------
    Public: DclInPlaceEmulated64ToLittleEndian()

    This function converts an emulated 64-bit type to little-
    endian byte order.

    Parameters:
        pData    - A pointer into the data to process

    Return Value:
        None.
-------------------------------------------------------------------*/
void DclInPlaceEmulated64ToLittleEndian(
    void       *pData)
{
    D_UINT64    ull64;

    DclAssertWritePtr(pData, 0);

    ull64 = *(D_UINT64*)pData;

    DclCopyEmulated64ToLittleEndian(pData, &ull64);

    return;
}


/*-------------------------------------------------------------------
    Public: DclInPlaceLittleEndianToEmulated64()

    This function converts a little-endian data-type to an
    emulated 64-bit type.

    Parameters:
        pData    - A pointer into the data to process

    Return Value:
        None.
-------------------------------------------------------------------*/
void DclInPlaceLittleEndianToEmulated64(
    void       *pData)
{
    D_UINT64    ull64;

    DclAssertWritePtr(pData, 0);

    ull64 = *(D_UINT64*)pData;

    DclCopyLittleEndianToEmulated64(pData, (D_BUFFER*)&ull64);

    return;
}


/*-------------------------------------------------------------------
    Public: DclCopyEmulated64ToLittleEndian()

    This function copies an emulated 64-bit type to a buffer and
    places the results in little- endian byte order.

    Parameters:
        pBuff    - A pointer to the destination buffer
        pullSrc  - A pointer to the source emulated 64-bit type.

    Return Value:
        None.
-------------------------------------------------------------------*/
void DclCopyEmulated64ToLittleEndian(
    D_BUFFER       *pBuff,
    const D_UINT64 *pullSrc)
{
    DclAssertReadPtr(pullSrc, 0);
    DclAssertWritePtr(pBuff, 0);

  #if DCL_BIG_ENDIAN
    DclCopyAndReverse(pBuff, &pullSrc->ulHighDword, sizeof(pullSrc->ulHighDword));
    pBuff += sizeof(pullSrc->ulHighDword);
    DclCopyAndReverse(pBuff, &pullSrc->ulLowDword, sizeof(pullSrc->ulLowDword));
  #else
    DclMemCpy(pBuff, &pullSrc->ulHighDword, sizeof(pullSrc->ulHighDword));
    pBuff += sizeof(pullSrc->ulHighDword);
    DclMemCpy(pBuff, &pullSrc->ulLowDword, sizeof(pullSrc->ulLowDword));
  #endif

    return;
}


/*-------------------------------------------------------------------
    Public: DclCopyLittleEndianToEmulated64()

    This function copies little-endian source data and converts
    it to an emulated 64-bit type.

    Parameters:
        pDestEmul64 - A pointer to the destination emulated 64-bit
                      type.
        pBuff       - A pointer to the little-endian source data

    Return Value:
        None
-------------------------------------------------------------------*/
void DclCopyLittleEndianToEmulated64(
    void           *pDestEmul64,
    const D_BUFFER *pBuff)
{
    D_UINT64       *pullDest = (D_UINT64*)pDestEmul64;

    DclAssertReadPtr(pBuff, 0);
    DclAssertWritePtr(pDestEmul64, 0);

  #if DCL_BIG_ENDIAN
    DclCopyAndReverse(&pullDest->ulHighDword, pBuff, sizeof(pullDest->ulHighDword));
    pBuff += sizeof(pullDest->ulHighDword);
    DclCopyAndReverse(&pullDest->ulLowDword, pBuff, sizeof(pullDest->ulLowDword));
  #else
    DclMemCpy(&pullDest->ulHighDword, pBuff, sizeof(pullDest->ulHighDword));
    pBuff += sizeof(pullDest->ulHighDword);
    DclMemCpy(&pullDest->ulLowDword, pBuff, sizeof(pullDest->ulLowDword));
  #endif

    return;
}

#endif

