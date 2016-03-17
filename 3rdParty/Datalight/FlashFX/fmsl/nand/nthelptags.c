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
  jurisdictions.  The software may be subject to one or more of these US
  patents: US#5860082, US#6260156.  Patents may be pending.

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

    This module contains helper functions which are used by multiple NTMs.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: nthelptags.c $
    Revision 1.6  2009/12/21 17:44:43Z  billr
    Make definition of FfxNtmHelpIsRangeErased1Bit() agree with
    prototype (thank you, gcc!).  The mismatch matters on 64-bit
    builds.
    Revision 1.5  2009/12/11 23:54:20Z  garyp
    Updated to use some functions which were renamed to avoid naming
    conflicts.  
    Revision 1.4  2009/10/06 19:13:55Z  garyp
    Renamed some functions to avoid collisions.
    Revision 1.3  2009/04/18 00:23:39Z  garyp
    Cleaned up the documentation -- no functional changes.
    Revision 1.2  2009/04/02 14:33:32Z  davidh
    Function headers updated for AutoDoc.
    Revision 1.1  2009/03/23 19:42:12Z  keithg
    Initial revision
---------------------------------------------------------------------------*/

#include <flashfx.h>

#if FFXCONF_NANDSUPPORT

#include <fimdev.h>
#include <ecc.h>
#include <nandconf.h>
#include <nandctl.h>
#include "nandid.h"
#include "nand.h"


/*-------------------------------------------------------------------
    Public: FfxNtmHelpTagEncode()

    Encode a FlashFX tag in the standard format.  The first two bytes
    are the standard media-independent value, followed by a check
    byte calculated from that value, and a Hamming code ECC for the
    whole thing.

    Parameters:
        pDest - The 4-byte destination buffer to fill.
        pSrc  - The source buffer containing the 2-byte tag data.

    Return Value:
        None.
-------------------------------------------------------------------*/
void FfxNtmHelpTagEncode(
    D_BUFFER       *pDest,
    const D_BUFFER *pSrc)
{
    #if LEGACY_ENCODED_TAG_SIZE != 4
    #error "FFX: Bad legacy encoded tag size"
    #endif

    #if LEGACY_TAG_SIZE != 2
    #error "FFX: Bad legacy tag size"
    #endif

    DclAssert(pDest);
    DclAssert(pSrc);

    pDest[0] = pSrc[0];
    pDest[1] = pSrc[1];
    pDest[2] = (D_UINT8)(pDest[0] ^ ~pDest[1]);
    pDest[3] = (D_UINT8)DclHammingCalculate(&pDest[0], 3, 0);

    /*  An encoded tag can never appear to be erased or within 1 bit thereof
    */
    DclAssert(!FfxNtmHelpIsRangeErased1Bit(pDest, LEGACY_ENCODED_TAG_SIZE));
}


/*-------------------------------------------------------------------
    Public: FfxNtmHelpTagDecode()

    Decode a FlashFX tag which was encoded with FfxNtmHelpTagEncode().

    Parameters:
        pDest - The destination buffer to fill with the 2-byte tag
                data.
        pSrc  - The 4-byte source buffer to process.

    Return Value:
        Returns TRUE if the tag was properly decoded and placed in
        the buffer specified by pDest, otherwise FALSE.  If FALSE
        is returned, the buffer specified by pDest will be set to
        NULLs.
-------------------------------------------------------------------*/
D_BOOL FfxNtmHelpTagDecode(
    D_BUFFER       *pDest,
    const D_BUFFER *pSrc)
{
    D_UINT32        ecc;
    D_BUFFER        TempBuff[LEGACY_ENCODED_TAG_SIZE];

    DclAssert(LEGACY_ENCODED_TAG_SIZE == sizeof(D_UINT32));

    DclAssert(pDest);
    DclAssert(pSrc);

    /*  If the encoded tag is in the fully erased state, return
        successfully, after having set the decoded tag to the
        erased state as well.

        It is possible that a page/tag which is otherwise erased
        has a single bit error in the tag area which makes it
        appear as if the tag has been written.  In this event,
        we let the normal hamming code and check byte stuff
        fail, and return that the tag is bogus.  The allocator
        will then recycle the page, and it will subsequently
        get erased and re-used.
    */
    if(*(D_UINT32*)pSrc == ERASED32)
    {
        pDest[0] = ERASED8;
        pDest[1] = ERASED8;

        return TRUE;
    }

    DclMemCpy(TempBuff, pSrc, LEGACY_ENCODED_TAG_SIZE);

    ecc = DclHammingCalculate(&TempBuff[0], 3, TempBuff[3]);

    if(ecc != 0)
        DclHammingCorrect(&TempBuff[0], 3, ecc);

    /*  Check the validation code
    */
    if(TempBuff[2] != (D_UCHAR) (TempBuff[0] ^ ~TempBuff[1]))
    {
        FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, 0),
            "FfxNtmHelpTagDecode() Unrecoverable tag error, Src=%lX ecc=%lX Corrected=%lX\n",
            *(D_UINT32*)pSrc, ecc, *(D_UINT32*)&TempBuff[0]));

        pDest[0] = 0;
        pDest[1] = 0;

        return FALSE;
    }

    pDest[0] = TempBuff[0];
    pDest[1] = TempBuff[1];

    return TRUE;
}


/*-------------------------------------------------------------------
    Public: FfxNtmHelpIsRangeErased1Bit()

    This function determines if the byte range is fully erased, or
    within one bit thereof.

    Parameters:
        pData - A pointer to the byte array to examine.
        nLen  - The number of bytes to examine.

    Return Value:
        Returns TRUE if the data is fully erased, or within a single
        bit of being erase, or FALSE otherwise.
-------------------------------------------------------------------*/
D_BOOL FfxNtmHelpIsRangeErased1Bit(
    const D_BUFFER *pData,
    size_t          nLen)
{
    unsigned        nOffBits = 0;

    DclAssert(pData);
    DclAssert(nLen);

    while(nLen)
    {
        if(*pData != ERASED8)
        {
            /*  If we already found an off bit, and the current byte is
                non-0xFF, then we know this is not an erased range.
            */
            if(nOffBits)
                return FALSE;

            nOffBits = 8 - DclBitCount((D_UINT32)*pData);
            if(nOffBits > 1)
                return FALSE;
        }

        pData++;
        nLen--;
    }

    DclAssert(nOffBits < 2);

    return TRUE;
}

#endif  /* FFXCONF_NANDSUPPORT */


