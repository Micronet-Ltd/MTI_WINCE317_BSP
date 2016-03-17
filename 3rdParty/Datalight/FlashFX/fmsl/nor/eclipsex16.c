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
  jurisdictions.  The software may be subject to one or more US patents, 
  including without limitation US Patents: US#5860082, US#6260156.  Patents
  may be pending.

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

    This module contains the FIM code for Spansion Eclipse Flash. It is based
    on the iswfx16 FIM.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: eclipsex16.c $
    Revision 1.8  2010/05/19 19:22:25Z  garyp
    Commented out some unused variables so we build cleanly with the
    TICCS ToolSet abstraction.
    Revision 1.7  2009/04/03 05:40:39Z  keithg
    Fixed bug 2521: removed obsolete READ_BACK_VERIFY.
    Revision 1.6  2008/11/27 07:31:53Z  glenns
    Removed declaration of "PrototypeInit" function- no longer used.
    Revision 1.5  2008/09/17 19:44:56Z  Glenns
    Updated FIM to use production-level parts by doing the following:
     - Updated device ID code offsets
     - Changing the Mount procedure to use JEDEC standard method
       for obtaining manufacturer ID code
     - Removing the prototype initialization feature, which is no longer
       required.
    Revision 1.4  2008/07/23 18:04:37Z  keithg
    Clarified comments with professional customer facing terms.
    Revision 1.3  2008/05/01 22:24:45Z  Glenns
    - Added support for Engineering Sample devices.
    - Fixed function header comment blocks to support Autodoc.
    Revision 1.2  2008/03/23 02:25:15Z  Garyp
    Added a typecast so we build cleanly with MSVC6.
    Revision 1.1  2008/03/05 00:22:46Z  Glenns
    Initial revision
---------------------------------------------------------------------------*/

#include <flashfx.h>
#include <fimdev.h>
#include "nor.h"

#if FFXCONF_ISWFSUPPORT

/*  Configure for a 1x16 layout
*/
#define FLASH_INTERLEAVE    1       /* Number of flash chips interleaved    */
#define FLASH_BUS_WIDTH     16      /* Overall data bus width in bits       */

#include "flashcmd.h"
/* flash specific intelcmd header must be included first */
#include "amdcmd.h"
#include "amdcmd_eclipse.h"

#define FLASH_PAGE_SIZE                512 * FLASH_INTERLEAVE

/*  For bitfield-programmed pages, input buffer must have 16 * FLASH_INTERLEAVE
    valid bytes and then next 16 * FLASH_INTERLEAVE are invalid bytes
*/
/*  valid size in Bytes
*/
#define FLASH_BITFIELD_VALID_SIZE       16 * FLASH_INTERLEAVE
/*  invalid size in Bytes
*/
#define FLASH_BITFIELD_INVALID_SIZE     16 * FLASH_INTERLEAVE

#define SECTOR_MASK     0xFFFFE000 /* used to get sector base address */
#define PAGE_MASK       0xFFFFFE00 /* 21 bits for page address */
#define OFFSET_MASK     0x000001FF /* 9 bits for offset (bits 0-8, bit 0 is always 0 for 1x16) */

/* "magic" addresses for chip commands:
*/
#define LATCH_OFFSET1 0x555UL
#define LATCH_OFFSET2 0xAAAUL
#define LATCH_OFFSET3 0x55UL

typedef struct tagFIMEXTRA
{
    DCLTIMER        tErase;
    D_UINT32        ulEraseStart;
    D_UINT32        ulEraseResult;
    D_UINT32        ulTimeoutRemaining;
    PFLASHDATA      pMedia;
    D_UINT16        uMaxWriteBuff;
} FIMEXTRA;

/*  ulEraseResult value; used internally to clarify intent.
*/
#define ERASE_IN_PROGRESS       (0)

/*  The minimum amount of time (in microseconds) to let a suspended
    erase progress to ensure that some forward progress is made.
*/
#define MINIMUM_ERASE_INCREMENT (1000)

/*  Datasheet specifies 2MB/second = 16 sectors/second
*   = 63 milliseconds / sector
*/
#define ERASE_TIMEOUT           (63 * 1000L)

/*  Erase suspend timeout: datasheet specifies 25 microseconds
*/
#define ERASE_SUSPEND_TIMEOUT   (1)

/*  Chip spec is TBD - may have to update.  1 page is typically 2.05 milliseconds
*/
#define WRITE_TIMEOUT           (40) /* 40 ms */

/*  Write State Machine microsecond delay for valid status
*/
#define WSM_DELAY               (1)

/*
 * Device offsets for DeviceID2 and DeviceID3
 */

#define DEVID2_OFFSET 0x0c
#define DEVID3_OFFSET 0x0e

/*  prototypes for local functions
*/
static D_BOOL IsStateMachineReady(PFLASHDATA pMedia);
static void IssueCAP1Command(PFLASHDATA pMedia, FLASHDATA command);
static void IssueCAP2Command(PFLASHDATA pMedia, FLASHDATA command);
static void IssueCAP3Command(PFLASHDATA pMedia, FLASHDATA command);
static D_BOOL ReadJEDECManufacturerID(FLASHIDCODES *ID, PFLASHDATA pMedia);
static void   ResetFlash(PFLASHDATA pMedia);
static FLASHDATA ReadStatus(PFLASHDATA pMedia);
static void ResetStatus(PFLASHDATA pMedia);
static D_BOOL WriteBufferedBytes(
    PFLASHDATA pMedia,
    D_UINT16 *puData,
    D_UINT32 ulLength,
    PEXTMEDIAINFO pEMI,
    D_UINT32 ulFlashOffset,
    D_BOOL bIsControl);

/*  Although Spansion Eclipse parts support a sector-locking mechanism,
    they are not sector-locked upon POR. With that in mind, this FIM
    does not implement block-unlocking treatments. A framework for an
    unlocking procedure is left here for someone who may someday wish
    to manipulate the Eclipse sector protection mechanism, but in the
    default configuration it is not compiled in.
*/
#if 0
static D_BOOL UnlockBlocks(D_UINT32 ulLength, FFXFIMBOUNDS *pBounds, PEXTMEDIAINFO pEMI);
#endif

/* Define maximum number of bytes that can be written with a single
   write-buffer command. For Spansion Eclipse, buf size = 512
*/
#define MAX_WRITE_BUFF             0x200

/*  All parts supported in this FIM have 128KB erase zones, and
    any of them that support boot blocks have 4 32KB boot block
    zones -- multiply appropriately for the interleave value.
*/
#define ZONE_SIZE               (0x00020000UL * FLASH_INTERLEAVE) /* 128KB */

/* List of Eclipse parts. May need to be expanded when new parts come
   available. See "amdcmd_eclipse.h" for definitions of the "ID_xxx"
   macros.
*/
static ECLIPSECHIPID ChipTable[] =
{
    {ID_S29XS02GS_PROTOTYPE2},
    {ID_S29XS512S},
    {ID_S29XS01GS},
    {ID_S29XS02GS},
    {ID_S29XS02GS_PROTOTYPE},
    {ID_ENDOFECLIPSELIST}
};


/*-------------------------------------------------------------------
    Public: Mount()

        Determines if the media is indeed supported.  If so the
        ExtndMediaInfo structure is updated to reflect it.

    Parameters:
        pEMI - A pointer to the ExtndMediaInfo structure to use.

    Return:
        Returns TRUE if successful, else FALSE
-------------------------------------------------------------------*/
static D_BOOL Mount(
    PEXTMEDIAINFO   pEMI)
{
    D_UINT32        ulAddress = 0L;
    PFLASHDATA      pMedia;
    unsigned        i;
    FLASHIDCODES    ID;
    FFXFIMBOUNDS    bounds;

    DclAssert(pEMI);

    pEMI->uDeviceType       = DEV_ISWF;
    pEMI->ulEraseZoneSize   = ZONE_SIZE;

    /*  Map in the next offset
    */
    FfxDevGetArrayBounds(pEMI->hDev, &bounds);
    if(!FfxFimNorWindowCreate(pEMI->hDev, ulAddress, &bounds, (volatile void **)&pMedia))
        return FALSE;

    /*  ID first chip
    */
    ResetFlash(pMedia);
    IssueCAP3Command(pMedia, AMDCMD_IDENTIFY);
    if (!ReadJEDECManufacturerID(&ID,pMedia))
    {
        FFXPRINTF(1, ("FFX: FIM eclipsex16:   Invalid flash ID: Data0/1=%04x/%04x\n",
                  ID.data0, ID.data1));
        ResetFlash(pMedia);
        return FALSE;
    }
    for(i = 0; ChipTable[i].chipParams.ulChipSize != 0; i++)
    {
        if (ID.idMfg == ChipTable[i].chipParams.idMfg)
        {
        FLASHDATA id;

            if(!FfxFimNorWindowCreate(pEMI->hDev, ulAddress + DEVID2_OFFSET, &bounds, (volatile void **)&pMedia))
                return FALSE;
            id = *(FLASHDATA *)pMedia;
            if (id == ChipTable[i].DevID2)
            {
                if(!FfxFimNorWindowCreate(pEMI->hDev, ulAddress + DEVID3_OFFSET, &bounds, (volatile void **)&pMedia))
                    return FALSE;
                id = *(FLASHDATA *)pMedia;
                if (id == ChipTable[i].DevID3)
                {
                    pEMI->ulDeviceSize = ChipTable[i].chipParams.ulChipSize * FLASH_INTERLEAVE;
                    FFXPRINTF(1, ("FFX: FIM eclipsex16:   Mfg/Dev=%04x/%04x supported\n", ID.idMfg, ID.idDev));
                    break;
                }
            }
        }
    }

    if(ChipTable[i].chipParams.ulChipSize == D_UINT32_MAX)
    {
        FFXPRINTF(1, ("FFX: FIM eclipsex16:   Mfg/Dev=%04x/%04x unsupported\n", ID.idMfg, ID.idDev));

        /*  Otherwise, device not found, so restore the original state
            and return device not found.
        */
        ResetFlash(pMedia);

        return FALSE;
    }

    /*  The algorithm to search for additional devices given below is
        currently disabled for the Eclipse FIM because the platform on
        which it was developed (Win32 using a Spansion PISMO2 PCI card)
        combined with the part's dual-chip-enable design caused this
        code to access an illegal address under certain circumstances.
        It may be re-enabled if it is desired to use it on a different
        platform.

        The implication is that this section of code has not been tested
        with the Eclipse.
    */

#if 0
    /*  Search for additional devices in a linear array.  Leave the first
        chips in Identify mode to detect wrap around (aliasing).  Since we've
        already identified the first devices, start at the next offset.
    */
    for(ulAddress = pEMI->ulDeviceSize;
        ulAddress < MAX_ARRAY;
        ulAddress += pEMI->ulDeviceSize)
    {
        FLASHIDCODES    ID2;

        if(!FfxFimNorWindowCreate(pEMI->hDev, ulAddress, &bounds, (volatile void **)&pMedia))
            break;

        /*  Check for wrap around
        */
        GETFLASHIDCODES(&ID2, pMedia);
        if((ID2.data0 == ID.data0) && (ID2.data1 == ID.data1))
            break;

        /*  ID each chip
        */
        ResetFlash(pMedia);
        IssueCAP3Command(pMedia, AMDCMD_IDENTIFY);
        GETFLASHIDCODES(&ID2, pMedia);
        if((ID2.data0 != ID.data0) || (ID2.data1 != ID.data1))
             break;

        /*  Reset the flash to read mode.  The first device in the array is
            not included in this loop, so we don't need any special cases.
        */
        ResetFlash(pMedia);
    }
#endif

    /*  Restore the first chip to read mode and establish some device
        parameters
    */
    if(!FfxFimNorWindowCreate(pEMI->hDev, 0L, &bounds, (volatile void **)&pMedia))
        return FALSE;
    ResetFlash(pMedia);
    pEMI->ulTotalSize = pEMI->ulDeviceSize;
    pEMI->pFimExtra = DclMemAlloc(sizeof *pEMI->pFimExtra);
    if(!pEMI->pFimExtra)
        return FALSE;
    DclMemSet(pEMI->pFimExtra, 0, sizeof *pEMI->pFimExtra);
    pEMI->pFimExtra->uMaxWriteBuff = MAX_WRITE_BUFF;

    /*  must know the page size due to the different modes of programming
        available
    */
    pEMI->uPageSize = FLASH_PAGE_SIZE;

    return TRUE;
}


/*-------------------------------------------------------------------
    Public: Unmount()

        This function dismounts the FIM and releases any allocated
        resources.

    Parameters:
        pEMI - A pointer to the ExtndMediaInfo structure to use.

    Return:
        None
-------------------------------------------------------------------*/
static void Unmount(
    PEXTMEDIAINFO   pEMI)
{
    DclAssert(pEMI);
    DclAssert(pEMI->pFimExtra);

    DclMemFree(pEMI->pFimExtra);
    pEMI->pFimExtra = NULL;
}


/*-------------------------------------------------------------------
    Public: Read()

        Read a given number of bytes of data from the media in standard
        page mode.

    Parameters:
        pEMI    - A pointer to the ExtndMediaInfo structure to use
        ulStart - Starting offset in bytes to begin the access
        uLength - Number of bytes to transfer
        pBuffer - Pointer to client supplied transfer area

    Return:
        Returns TRUE if successful, else FALSE.
-------------------------------------------------------------------*/
static D_BOOL Read(
    PEXTMEDIAINFO   pEMI,
    D_UINT32        ulStart,
    D_UINT16        uLength,
    void           *pBuffer)
{
#if 0
PFLASHDATA  pSrc;
PFLASHDATA  pDest = (PFLASHDATA)pBuffer;
#endif

    DclAssert(pEMI);
    DclAssert(ulStart < MAX_ARRAY);
    DclAssert(uLength);
    DclAssert(pBuffer);
    DclAssert((ulStart % sizeof(FLASHDATA)) == 0);
    DclAssert((uLength % sizeof(FLASHDATA)) == 0);
#if 0
    while(uLength)
    {
        D_UINT32    ulWindowSize;

        /*  Get the media pointer and the max size we can access with it
        */
        ulWindowSize = FfxFimNorWindowMap(pEMI->hDev, ulStart, (volatile void **)&pSrc);
        if(!ulWindowSize)
            break;
        DclAssert((ulWindowSize % sizeof(FLASHDATA)) == 0);

        while(ulWindowSize && uLength)
        {
            D_UINT32    ulThisLength = DCLMIN(uLength, ulWindowSize);
            /*  Move this chunk of data
            */
            *pDest = *pSrc;
            pDest++;
            pSrc++;
            uLength         -= sizeof(FLASHDATA);
            ulWindowSize    -= sizeof(FLASHDATA);
            ulStart         += sizeof(FLASHDATA);

            DclAssert(ulStart);
        }
    }

    return (uLength == 0);


#else

    DclAssert(pEMI);

    return FfxFimNorRead(pEMI, ulStart, uLength, pBuffer);
#endif
}

/*-------------------------------------------------------------------
    Public: ReadControl()

        Read a given number of bytes of data from the media in bitfield
        mode.

        This function will handle formatting the data properly (ie, reading
        the programmed areas of flash to the buffer- see the Spansion
        Eclipse datasheet for details).

    Parameters:
        pEMI    - A pointer to the ExtndMediaInfo structure to use
        ulStart - high 22 bits are the physical address of page start.
                  offset into page: bits 0-9 (bit 9 must be zero for 1x16)
        uLength - Number of bytes to transfer
        pBuffer - Pointer to client supplied transfer area

    Return:
        Returns TRUE if successful, else FALSE.
-------------------------------------------------------------------*/
static D_BOOL ReadControl(
    PEXTMEDIAINFO   pEMI,
    D_UINT32        ulStart,
    D_UINT16        uLength,
    void           *pBuffer)
{
    D_UINT16        uThisLength = 0;
    D_UINT32        ulFlashOffset = 0;
    D_UINT32        ulOffsetInPage = 0;
    D_UINT16       *puDataPtr = (D_UINT16 *)pBuffer;
    D_BOOL          fSuccess = TRUE;

    DclAssert(pEMI);
    DclAssert(ulStart % sizeof(D_UINT32) == 0L);
    DclAssert(uLength);
    DclAssert(pBuffer);
    DclAssert(puDataPtr);
    DclAssert(uLength % sizeof(*puDataPtr) == 0L);
    DclAssert(ulStart % sizeof(*puDataPtr) == 0L);

    /*  extract page address
    */
    ulFlashOffset = ulStart & PAGE_MASK;

    /*  Get the offset within the page and check that it is with in the
        allowed size.
    */
    ulOffsetInPage = ulStart & OFFSET_MASK;

    /*  This should be less than FLASH_PAGE_SIZE/2 values will be coming in
        0-512.
    */
    DclAssert(ulOffsetInPage < (FLASH_PAGE_SIZE/2));
    if(ulOffsetInPage >= (FLASH_PAGE_SIZE/2))
    {
        return FALSE;
    }

    /*  Convert the offset within the page to account for the invalid regions.
    */
    ulOffsetInPage = ((ulOffsetInPage / FLASH_BITFIELD_VALID_SIZE) *
                      (FLASH_BITFIELD_VALID_SIZE + FLASH_BITFIELD_INVALID_SIZE))+
                      (ulOffsetInPage % FLASH_BITFIELD_VALID_SIZE);

    ulFlashOffset += ulOffsetInPage;

    /*  Verify user address and length parameters within the media
        boundaries.
    */
    DclAssert((ulFlashOffset + uLength) <= pEMI->ulTotalSize);

    /*  Align the first read to the control mode region size
    */
    uThisLength = (D_UINT16) DCLMIN(uLength,
                      (FLASH_BITFIELD_VALID_SIZE -
                      (ulFlashOffset % (FLASH_BITFIELD_VALID_SIZE + FLASH_BITFIELD_INVALID_SIZE))));

    /*  Now read the data.
    */
    while(uLength)
    {
        /*  FfxFimNorRead will figure out window map.
        */
        fSuccess = FfxFimNorRead(pEMI, ulFlashOffset, uThisLength, puDataPtr);

        DclAssert(fSuccess);

        if(!fSuccess)
        {
            /*  Clear the error status, reset to read mode and return
            */
            fSuccess = FALSE;
            break;
        }

        /*  Go to the next offset
        */
        ulFlashOffset += (uThisLength + FLASH_BITFIELD_INVALID_SIZE);
        puDataPtr += (uThisLength / FLASH_BUS_BYTES);
        uLength -= uThisLength;

        /*  Recalculate the length of the next request
        */
        if(uLength < FLASH_BITFIELD_VALID_SIZE)
        {
            uThisLength = uLength;
        }
        else
        {
            uThisLength = FLASH_BITFIELD_VALID_SIZE;
        }
    }

    return fSuccess;
}


/*-------------------------------------------------------------------
    Public: Write()

        Writes a given number of bytes of data out to the media, in
        standard page mode.
        It does not return until the data is programmed.

    Parameters:
        pEMI    - A pointer to the ExtndMediaInfo structure to use.
        ulStart - Starting offset in bytes to begin the access
        uLength - Number of bytes to transfer
        pBuffer - Pointer to client supplied transfer area

    Return Value:
        Returns TRUE if successful, else FALSE.
-------------------------------------------------------------------*/
static D_BOOL Write(
    PEXTMEDIAINFO   pEMI,
    D_UINT32        ulStart,
    D_UINT16        uLength,
    void           *pBuffer)
{
    D_UINT16       *puDataPtr = (D_UINT16 *)pBuffer;
    D_BOOL          fSuccess = TRUE;
    PFLASHDATA      pMedia = NULL;

    DclAssert(pEMI);
    DclAssert(ulStart % sizeof(D_UINT32) == 0L);
    DclAssert(uLength);
    DclAssert(pBuffer);
    DclAssert(puDataPtr);
    DclAssert(uLength % sizeof(*puDataPtr) == 0L);
    DclAssert(ulStart % sizeof(*puDataPtr) == 0L);

    /*  Verify user address and length parameters within the media
        boundaries.
    */
    DclAssert((ulStart + uLength) <= pEMI->ulTotalSize);

    while(uLength)
    {
        D_UINT32    ulWindowSize;

        /*  Get the media pointer and the max size we can access with it
        */
        ulWindowSize = FfxFimNorWindowMap(pEMI->hDev, ulStart, (volatile void **)&pMedia);
        if(!ulWindowSize)
        {
            fSuccess = FALSE;
            break;
        }

        /*  Move each window worth of data into the flash memory
            (Allow WriteBufferedBytes to handle buffer alignment to page)
        */
        while(ulWindowSize && uLength)
        {
            D_UINT32    ulThisLength = DCLMIN(uLength, ulWindowSize);

            DclAssert((ulThisLength % FLASH_BUS_BYTES) == 0L);

            fSuccess = WriteBufferedBytes(pMedia, puDataPtr, ulThisLength, pEMI, ulStart, FALSE);
            if(!fSuccess)
            {
                /*  Clear the error status, reset to read mode and return
                */
                DclError();
                break;
            }

            /*  If we have written everything, get outta here.
            */
            uLength -= (D_UINT16)ulThisLength;
            if(uLength == 0)
                break;

            /*  Go to the next offset
            */
            ulWindowSize    -= ulThisLength;
            ulStart         += ulThisLength;
            puDataPtr       += ulThisLength / FLASH_BUS_BYTES;

            DclAssert(ulStart);
            DclAssert(puDataPtr);
        }

        if(!fSuccess)
            break;
    }

    /*  Be sure we leave the flash in the read mode
    */
    ResetFlash(pMedia);

    return fSuccess;
}

/*-------------------------------------------------------------------
    Public: WriteControl()

        Writes a given number of bytes of data out to the media, using
        Spansion Eclipse bitfield programming.
        It does not return until the data is programmed.

        This function will handle formatting the data properly (ie, writing
        to only the appropriate areas of the buffer). See the Eclipse
        datasheet for details.

    Parameters:
        pEMI    - A pointer to the ExtndMediaInfo structure to use.
        ulStart - high 22 bits are the physical address of page start.
                  offset into page: bits 0-9 (bit 9 must be zero for 1x16)
        uLength - Number of bytes to transfer
        pBuffer - Pointer to client supplied transfer area

    Return:
        Returns TRUE if successful, else FALSE.
-------------------------------------------------------------------*/
static D_BOOL WriteControl(
    PEXTMEDIAINFO   pEMI,
    D_UINT32        ulStart,
    D_UINT16        uLength,
    void           *pBuffer)
{
    D_UINT32        ulFlashOffset = 0;
    D_UINT32        ulOffsetInPage = 0;
    D_UINT16       *puDataPtr = (D_UINT16 *)pBuffer;
    D_BOOL          fSuccess = TRUE;
    PFLASHDATA      pMedia = NULL;

    DclAssert(pEMI);
    DclAssert(uLength);
    DclAssert(pBuffer);
    DclAssert(ulStart % sizeof(D_UINT32) == 0L);
    DclAssert(puDataPtr);
    DclAssert(uLength % sizeof(*puDataPtr) == 0L);
    DclAssert(ulStart % sizeof(*puDataPtr) == 0L);

    /*  First get the address for the start of the page.
    */
    ulFlashOffset = ulStart & PAGE_MASK;

    /*  Get the offset within the page and check that it is with in the
        allowed size.
    */
    ulOffsetInPage = ulStart & OFFSET_MASK;

    /*  This should be less than FLASH_PAGE_SIZE / 2 values will be coming
        in 0 - FLASH_PAGESIZE / 2
    */
    DclAssert(ulOffsetInPage < (FLASH_PAGE_SIZE / 2));
    if(ulOffsetInPage >= (FLASH_PAGE_SIZE / 2))
    {
        return FALSE;
    }

    /*  Convert the offset within the page to account for the invalid regions.
    */
    ulOffsetInPage = ((ulOffsetInPage / FLASH_BITFIELD_VALID_SIZE) *
                     (FLASH_BITFIELD_VALID_SIZE + FLASH_BITFIELD_INVALID_SIZE))+
                     (ulOffsetInPage % FLASH_BITFIELD_VALID_SIZE);

    ulFlashOffset += ulOffsetInPage;

    /*  Verify user address and length parameters within the media boundaries.
    */
    DclAssert((ulFlashOffset + uLength) <= pEMI->ulTotalSize);


    /*  Move each window worth of data into the flash memory
        (allow WriteBufferedBytes to align buffer to control mode regions)
    */

    while(uLength)
    {
        D_UINT32    ulWindowSize;

        /*  Get the media pointer and the max size we can access with it
        */
        ulWindowSize = FfxFimNorWindowMap(pEMI->hDev, ulFlashOffset, (volatile void **)&pMedia);
        if(!ulWindowSize)
        {
            fSuccess = FALSE;
            break;
        }

        /*  Move each window worth of data into the flash memory
            (Allow WriteBufferedBytes to handle buffer alignment to page)
        */
        while(ulWindowSize && uLength)
        {
            D_UINT32    ulThisLength = DCLMIN(uLength, ulWindowSize);

            DclAssert((ulThisLength % FLASH_BUS_BYTES) == 0L);

            fSuccess = WriteBufferedBytes(pMedia, puDataPtr, ulThisLength, pEMI, ulFlashOffset, TRUE);
            if(!fSuccess)
            {
                /*  Clear the error status, reset to read mode and return
                */
                DclError();
                break;
            }

            /*  If we have written everything, get outta here.
            */
            uLength -= (D_UINT16)ulThisLength;
            if(uLength == 0)
                break;

            /*  Go to the next offset
            */
            ulWindowSize    -= ulThisLength;
            ulFlashOffset   += (ulThisLength + FLASH_BITFIELD_INVALID_SIZE);
            puDataPtr       += (D_UINT16) ulThisLength / FLASH_BUS_BYTES;
        }

        if(!fSuccess)
            break;
    }

    /*  Be sure we leave the flash in the read mode
    */
    ResetFlash(pMedia);

    return fSuccess;
}


/*-------------------------------------------------------------------
    Public: EraseStart()

        Attempts to initiate an erase operation.  If it is started
        successfully, the only FIM functions that can then be called
        are EraseSuspend() and ErasePoll().  The operation must s
        ubsequently be monitored by calls to ErasePoll().

        If it is not started successfully, those functions may not
        be called.  The flash is restored to a readable state if
        possible, but this cannot always be guaranteed.

    Parameters:
        pEMI    - A pointer to the ExtndMediaInfo structure to use.
        ulStart - Starting offset in bytes to begin the erase.  This
                  must be on a physical erase zone boundary.
        uLength - Number of bytes to erase.  This must be the exact
                  total length of one or more physical erase zones
                  starting at ulStart.

    Return:
        Returns TRUE if successful, else FALSE.
-------------------------------------------------------------------*/
static D_BOOL EraseStart(
    PEXTMEDIAINFO   pEMI,
    D_UINT32        ulStart,
    D_UINT32        ulLength)
{
    PFLASHDATA      pMedia;
    FLASHDATA       chipStatus;
    PFIMEXTRA       pFimExtra;
/*    D_UINT32        ulBaseLatch;  */

    DclAssert(pEMI);
    pFimExtra = pEMI->pFimExtra;
    DclAssert(pFimExtra);

/*    ulBaseLatch = ulStart & (~(ZONE_SIZE - 1));  Unused ?? */

    DclAssert(ulLength >= ZONE_SIZE);
    DclAssert(ulLength % ZONE_SIZE == 0);
    DclAssert(ulStart % ZONE_SIZE == 0);
    if(ulLength < ZONE_SIZE ||
       ulLength % ZONE_SIZE != 0 ||
       ulStart % ZONE_SIZE != 0)
    {
        return FALSE;
    }

    /*  Verify user address and length parameters within the media
        boundaries.
    */
    DclAssert(ulStart < pEMI->ulTotalSize);
    DclAssert(pEMI->ulTotalSize - ulStart >= ulLength);


    if(!FfxFimNorWindowMap(pEMI->hDev, ulStart, (volatile void **)&pMedia))
        return FALSE;

    /*  Clear any status from previous operations, then check the status.
    */
    ResetStatus(pMedia);
    chipStatus = ReadStatus(pMedia);

    /* If the chip is ready, start the erase operation
    */
    if((chipStatus & ECLIPSESTAT_MASK) == ECLIPSESTAT_DEVRDY)
    {
        /*  The flash appears to be ready, so start the erase.
        */
        IssueCAP1Command(pMedia, AMDCMD_ERASE1);
        IssueCAP2Command(pMedia, AMDCMD_ERASE2);

        /*  Set the timeout for the operation now that it's started.
        */
        DclTimerSet(&pEMI->pFimExtra->tErase, ERASE_TIMEOUT);

        /*  Record the erase address for ErasePoll(), EraseSuspend, and
            EraseResume() to use.
        */
        pEMI->pFimExtra->ulEraseStart = ulStart;

        /*  Clear the erase result to signify erase in progress.
        */
        pEMI->pFimExtra->ulEraseResult = ERASE_IN_PROGRESS;

        /*  Remember the media address, save repeated calls to
            FfxFimNorWindowMap() from ErasePoll().
        */
        pEMI->pFimExtra->pMedia = pMedia;

        return TRUE;
    }
    else
    {
        /*  There's probably something horrid going on like an erase that
            timed out and got suspended.  Try to return the flash to Read
            Array mode and return a failure indication.
        */
        ResetFlash(pMedia);
        pFimExtra->ulEraseResult = FIMMT_ERASE_FAILED;
        return FALSE;
    }
}


/*-------------------------------------------------------------------
    Public: EraseSuspend()

        Suspend an erase operation currently in progress, and return
        the flash to normal read mode.  When this function returns,
        the flash may be read.

        If the flash does not support suspending erases, this
        function is not implemented, and the EraseSuspend entry in
        the FIMDEVICE structure must be NULL.

    Parameters:
        pEMI     - A pointer to the ExtndMediaInfo structure to use

    Return:
        Returns TRUE if successful, else FALSE.
-------------------------------------------------------------------*/
static D_BOOL EraseSuspend(
    PEXTMEDIAINFO   pEMI)
{
    D_BOOL          fResult = TRUE;
    PFIMEXTRA       pFimExtra = pEMI->pFimExtra;
    PFLASHDATA      pMedia;
    FLASHDATA       chipStatus;

    DclAssert(pFimExtra);
    pMedia = pFimExtra->pMedia;

    /*  Save the remaining timeout period.
    */
    pFimExtra->ulTimeoutRemaining = DclTimerRemaining(&pFimExtra->tErase);

    /*  The flash is expected to be in Read Status mode. Also,
        don't need to worry about CAP command; any address in this
        erase zone will do.
    */
    *pMedia = AMDCMD_ERASE_SUSPEND;

    /*  Wait a while for the flash to go into erase suspend.
    */
    DclTimerSet(&pFimExtra->tErase, ERASE_SUSPEND_TIMEOUT);
    while(!DclTimerExpired(&pFimExtra->tErase))
    {
        chipStatus = ReadStatus(pMedia);
        if((chipStatus & ECLIPSESTAT_DEVRDY) == ECLIPSESTAT_DEVRDY)
            break;
    }

    /*  Check the status after a possible timeout.  A higher priority
        thread could have preempted between setting the timer or
        checking the status in the loop and checking for expiration.
    */
    chipStatus = ReadStatus(pMedia);
    if((chipStatus & ECLIPSESTAT_DEVRDY) != ECLIPSESTAT_DEVRDY)
    {
        /*  It really timed out.  This is a Bad Thing.  Record the failure.
        */
        pFimExtra->ulEraseResult = FIMMT_ERASE_FAILED;
        fResult = FALSE;
    }

    /*  Return the flash to Read Array mode whether or not the suspend
        command appeared to have worked (it can't hurt).
    */
    ResetFlash(pMedia);

    return fResult;
}


/*-------------------------------------------------------------------
    Public: EraseResume()

        Resumes an erase that was successfully suspended by
        EraseSuspend().  Once it is resumed, the only FIM functions
        that can be called are EraseSuspend() and ErasePoll().

        If the flash does not support suspending erases, this
        function is not implemented, and the EraseSuspend entry
        in the FIMDEVICE structure must be NULL.

    Parameters:
        pEMI     - A pointer to the ExtndMediaInfo structure to use

    Return:
        None
-------------------------------------------------------------------*/
static void EraseResume(
    PEXTMEDIAINFO   pEMI)
{
    PFIMEXTRA       pFimExtra = pEMI->pFimExtra;
    PFLASHDATA      pMedia;
    FLASHDATA       chipStatus;

    DclAssert(pFimExtra);

    /*  If an error occurred in EraseSuspend() the final result of the
        erase was already recorded.  Only operate on the flash if this
        hasn't happened yet.
    */
    if(pFimExtra->ulEraseResult == ERASE_IN_PROGRESS)
    {
        /*  Erases are suspended to perform other operations, so it's
            necessary to remap the window now.
        */
        if(!FfxFimNorWindowMap(pEMI->hDev, pFimExtra->ulEraseStart, (volatile void **)&pMedia))
            return;

        pFimExtra->pMedia = pMedia;

        /*  Put the flash in Read Status mode.
        */
        ResetFlash(pMedia);

        /*  Be sure the device is in Ready mode- if it isn't, it means
            that the dvice is still actively doing a program or erase
            operation, in which case it was an error to try to call this
            procedure:
        */

        chipStatus = ReadStatus(pMedia);
        if (!(chipStatus & ECLIPSESTAT_DEVRDY))
            DclError();

        /*  It's possible that  the chip finished its erase before
            EraseSuspend() was called.  Don't try to resume a chip
            that is not suspended: experiment shows that it will
            (sometimes?) return to Read Array mode.
        */

        if (!(chipStatus & ECLIPSESTAT_ERASE_SUSPEND))
        {
            /* Device is ready, but not in the Erase Suspend state.
               Erase operation must have completed or errored out
               before we tried to suspend it. Simply return and let
               ErasePoll pick up on the results of all that.
            */
            return;
        }
        else
        {
            /* We are  in erase suspend state. Issue the Erase Resume
               command. Don't need to use CAP addressing because any
               address in the erase zone will do.
            */
            *pMedia = AMDCMD_ERASE2;

            /*  Need to find a cleaner way to do this.
                Guarantee a minimum erase increment.
            */
            _sysdelay(MINIMUM_ERASE_INCREMENT);

            /*  Restart the timer.  Note that this will be done in the (unlikely)
                case that chip had already finished when they were suspended.
                This is harmless, as ErasePoll() checks their status before checking
                for timeout.
            */
            DclTimerSet(&pFimExtra->tErase, pFimExtra->ulTimeoutRemaining);
        }
    }
}


/*-------------------------------------------------------------------
    Public: ErasePoll()

        Monitor the status of an erase begun with EraseStart().

        If the erase fails, attempts to return the flash to its
        normal read mode.  Depending on the type of flash, this
        may or may not be possible.  If it is possible, it may be
        achieved by suspending the erase operation rather than by
        terminating it.  In this case, it may be possible to read
        the flash, but not to erase it further.

        This function may be called with the flash either in read
        array mode or in read status mode.

    Parameters:
        pEMI     - A pointer to the ExtndMediaInfo structure to use

    Return:
        If the erase is still in progress, returns 0.  The only FIM
        functions that can then be called are EraseSuspend() and
        ErasePoll().

        If the erase completed successfully, returns the length of
        the erase zone actually erased.  This may be less than the
        ulLength value supplied to EraseStart().  The flash is in
        normal read mode.

        If the erase failed, returns FIMMT_ERASE_FAILED, which is a
        value that could never be a valid erase length.  The flash
        is returned to normal read mode if possible, but this may not
        be possible in all cases (for example, if the flash does not
        support suspending an erase, and the operation times out).
-------------------------------------------------------------------*/
static D_UINT32 ErasePoll(
    PEXTMEDIAINFO   pEMI)
{
    PFIMEXTRA       pFimExtra = pEMI->pFimExtra;
    PFLASHDATA      pMedia;
    FLASHDATA       chipStatus;

    DclAssert(pFimExtra);

    /*  ErasePoll() may be called multiple times even after the
        operation has completed.  Only check the flash if it
        has not already been seen to have finished its operation.
    */
    if(pFimExtra->ulEraseResult == ERASE_IN_PROGRESS)
    {
        /*  The erase was still in progress the last time it was checked.
            The erase is not done until both chips' status registers have
            the DONE bit set.
        */
        pMedia = pFimExtra->pMedia;
        chipStatus = ReadStatus(pMedia);
        if((chipStatus & ECLIPSESTAT_DEVRDY) == ECLIPSESTAT_DEVRDY)
        {
            /*  If the erase status or sector lock status bits are set,
                the erase failed. If neither of these bits are set and
                the device is indicating ready, the erase succeeded
            */
            chipStatus = ReadStatus(pMedia);
            if((chipStatus & ECLIPSESTAT_ERASE_STATUS) ||
               (chipStatus & ECLIPSESTAT_SECTLOCK_STATUS))
            {
                /*  The status register indicates something other than
                    normal completion.
                */
                pFimExtra->ulEraseResult = FIMMT_ERASE_FAILED;

            }
            else
            {
                /*  The erase has completed successfully.  One erase zone
                    has been erased.
                */
                pFimExtra->ulEraseResult = ZONE_SIZE;
            }

            /*  Whether or not there was an error, return the flash to
                Read Array mode.
            */
            ResetFlash(pMedia);
        }
        else if(DclTimerExpired(&pFimExtra->tErase))
        {
            chipStatus = ReadStatus(pMedia);
            if ((chipStatus & ECLIPSESTAT_DEVRDY) != ECLIPSESTAT_DEVRDY)
            {
                /*  The erase hasn't finished, and the timeout has elapsed.
                    Try to return the flash to Read Array mode by suspending
                    the erase, and return a failure indication.

                    The extra check of status after the timeout check may
                    look a little odd -- wasn't it just checked above?  This
                    is to handle perverse platforms on which power management
                    can suspend the CPU any old time, but the flash keeps
                    erasing.  Yes, this really happens.
                */
                EraseSuspend(pEMI);
                pFimExtra->ulEraseResult = FIMMT_ERASE_FAILED;
            }
        }
    }
    return pFimExtra->ulEraseResult;
}


/*-------------------------------------------------------------------
    Private: IsStateMachineReady()

        Polls until the ready bit is set, or the timeout has elapsed.

    Parameters:
        pMedia - The current location in the media

    Return:
        Returns TRUE if ready, or FALSE if timed out.
-------------------------------------------------------------------*/
static D_BOOL IsStateMachineReady(
    PFLASHDATA      pMedia)
{
    DCLTIMER        timer;
    FLASHDATA       chipStatus;

    DclTimerSet(&timer, WRITE_TIMEOUT);

    while(!DclTimerExpired(&timer))
    {
        chipStatus = ReadStatus(pMedia);
        if((chipStatus & ECLIPSESTAT_DEVRDY) == ECLIPSESTAT_DEVRDY)
            return TRUE;
    }

    /*  Timed out.  Check status one last time, as it could have gotten
        done between checks.
    */
    chipStatus = ReadStatus(pMedia);
    if((chipStatus & ECLIPSESTAT_DEVRDY) == ECLIPSESTAT_DEVRDY)
        return TRUE;

    FFXPRINTF(1, ("FFX: flash timeout with status = %08lx\n", *pMedia));
    DclError();

    return FALSE;
}

/*-------------------------------------------------------------------
    Private: IssueCAP1Command()

        Issues the command specified by the "command" parameter
        to the CAP1 offset of the erase block referenceed by the
        "pMedia" paremeter.

    Parameters:
        pMedia - A pointer to flash media
        command - a command to be issued to the CAP1 offset.
        (Refer to Eclipse documentation for explanation of
        "CAP1").

    Return:
        None
-------------------------------------------------------------------*/

static void IssueCAP1Command(PFLASHDATA pMedia, FLASHDATA command)
{
PFLASHDATA pLatch1Addr = pMedia + LATCH_OFFSET1;

    *pLatch1Addr = command;
}

/*-------------------------------------------------------------------
    Private: IssueCAP2Command()

        Issues the command specified by the "command" parameter
        to the CAP2 offset of the erase block referenceed by the
        "pMedia" paremeter.

    Parameters:
        pMedia - A pointer to flash media
        command - a command to be issued to the CAP2 offset.
        (Refer to Eclipse documentation for explanation of
        "CAP2").

    Return:
        None
-------------------------------------------------------------------*/

static void IssueCAP2Command(PFLASHDATA pMedia, FLASHDATA command)
{
PFLASHDATA pLatch2Addr = pMedia + LATCH_OFFSET2;

    *pLatch2Addr = command;
}

/*-------------------------------------------------------------------
    Private: IssueCAP3Command()

    Description
        Issues the command specified by the "command" parameter
        to the CAP3 offset of the erase block referenceed by the
        "pMedia" paremeter.

    Parameters:
        pMedia - A pointer to flash media
        command - a command to be issued to the CAP3 offset.
        (Refer to Eclipse documentation for explanation of
        "CAP3").

    Return:
        None
-------------------------------------------------------------------*/

static void IssueCAP3Command(PFLASHDATA pMedia, FLASHDATA command)
{
PFLASHDATA pLatch3Addr = pMedia + LATCH_OFFSET3;

    *pLatch3Addr = command;
}

/*-------------------------------------------------------------------
    ReadJEDECManufacturerID()

    Description
        Uses the JEDEC standard for retrieving the manufacturer
        ID code from the device. It scans the first MAX_MFGID_SCAN
        locations withing the device starting at offset zero
        and looks for the first location that does *NOT* have
        the JEDEC Continuation Code in it. Once it finds such
        a location, it inserts that into the "MfgID" element
        of the "ID" parameter. the "DevID" element will be set to
        the continuation code; it is the responsibility of the
        caller to fill in additional device ID codes as needed.

        NOTE- this procedure does not place the device into the
        required mode for reading device codes; it assumes the
        device is already in that state. It is the caller's
        responsibility to ensure this.

    Parameters
        pID - a pointer to the FLASHIDCODES structure used to
        store device manufacturer and ID information
        pMedia - A pointer to flash media

    Return Value
        None
-------------------------------------------------------------------*/
#define MAX_MFGID_SCAN 16
static D_BOOL ReadJEDECManufacturerID(FLASHIDCODES *pID, PFLASHDATA pMedia)
{
D_UINT16 i;
FLASHDATA data;

    for (i = 0; i < MAX_MFGID_SCAN; i++)
    {
        data = *(pMedia + i);
        if (data != AMDSTAT_JEDECCONTINUATION)
        {
            pID->idMfg = data;
            pID->idDev = AMDSTAT_JEDECCONTINUATION;
            break;
        }
    }
    if (i == MAX_MFGID_SCAN) return FALSE;
    else return TRUE;
}

/*-------------------------------------------------------------------
    ResetFlash()

        Resets the flash memory by clearing the error status and
        returning to the read mode.

    Parameters:
        pMedia - A pointer to flash media

    Return:
        None
-------------------------------------------------------------------*/
static void ResetFlash(
    PFLASHDATA      pMedia)
{
    /*  Initialize flash memory for word x16 mode
    */
    *pMedia = AMDCMD_READ_MODE;
    return;
}

/*-------------------------------------------------------------------
    Private: ReadStatus()

        Reads the flash status register. Device automatically
        returns to the read mode.

    Parameters:
        pMedia - A pointer to flash media

    Return:
        status register value.
-------------------------------------------------------------------*/
static FLASHDATA ReadStatus(
    PFLASHDATA      pMedia)
{
FLASHDATA chipStatus = 0;

    /*  Send the "read sector status" command
    */
    IssueCAP1Command(pMedia,ECLIPSECMD_READ_STATUS);
    chipStatus = *pMedia;
    return chipStatus;
}

/*-------------------------------------------------------------------
    Private: ResetStatus()

        Resets the flash status register. This clears any error
        indications that might exist. Device automatically
        returns to the read mode.

    Parameters:
        pMedia - A pointer to flash media

    Return:
        None
-------------------------------------------------------------------*/
static void ResetStatus(
    PFLASHDATA      pMedia)
{
    IssueCAP1Command(pMedia,ECLIPSECMD_CLEAR_STATUS);
    return;
}

/*-------------------------------------------------------------------
    Public: WriteBufferedBytes()

        Write the specified bytes to flash using "buffered" writes.

    Parameters:
        pMedia          - The current location in the media
        puData          - The current location in the data
        ulLength        - The length to write
        pEMI            - A pointer to the ExtndMediaInfo structure
        ulFlashOffset   - used to help find program buffer alignment-
                            DO NOT USE TO ACCESS ACTUAL ADDRESS
        bIsControl      - is control mode?

        NOTE: PFLASHDATA is D_PUINT for this size FLASH_BUS_WIDTH (flashcmd.h)
              puData is D_PUINT instead of D_PUBYTE.

    Return:
        Returns TRUE if successful, else FALSE.
-------------------------------------------------------------------*/
static D_BOOL WriteBufferedBytes(
    PFLASHDATA      pMedia,
    D_UINT16       *puData,
    D_UINT32        ulLength,
    PEXTMEDIAINFO   pEMI,
    D_UINT32        ulFlashOffset,
    D_BOOL          bIsControl)
{
    D_UINT16        uFlashIndex = 0;
    /* for control mode, the uBuffIndex != uFlashIndex due to invalid zone */
    D_UINT16        uBuffIndex = 0;
    D_UINT16        i;
    D_UINT16        uByteBuffSize = 0;
    D_UINT32        ulSectorBase;
	D_UINT32		ulCurrentOffset;
    PFLASHDATA      pSectorBase;
    FLASHDATA       SectorStatus;

    DclAssert(pEMI);
    DclAssert( (ulLength % FLASH_BUS_BYTES) == 0L);

    /* Get a pointer for issuing commands to the part. This must be done
       because pMedia may not have the appropriate alignment to access the
       "magic" addresses used to issue commands.
    */
	ulCurrentOffset = ulFlashOffset;
    ulSectorBase = ulCurrentOffset & (~(pEMI->ulEraseZoneSize - 1));
    if(!FfxFimNorWindowMap(pEMI->hDev, ulSectorBase, (volatile void **)&pSectorBase))
        return FALSE;

    /*  Clear any leftover error status.
    */
    ResetFlash(pMedia);
    IssueCAP1Command(pSectorBase,ECLIPSECMD_CLEAR_STATUS);

    /*  Buffer should always be ready, die if not
    */
    if(!IsStateMachineReady(pSectorBase))
    {
        DclError();
        return FALSE;
    }

    /*  try to set correct buff size read in relation to buffer before
        going in.
    */
    if(bIsControl)
    {
        uByteBuffSize = (D_UINT16)(FLASH_BITFIELD_VALID_SIZE - (ulFlashOffset %
                (FLASH_BITFIELD_VALID_SIZE + FLASH_BITFIELD_INVALID_SIZE)));
    }
    else
    {
        uByteBuffSize = (D_UINT16)(pEMI->pFimExtra->uMaxWriteBuff -
                (ulFlashOffset % pEMI->pFimExtra->uMaxWriteBuff));
    }

    /*  Program ulLength bytes into flash via buffer
    */
    while(ulLength)
    {
        D_UINT16         uByteCount;

        uByteCount = 0;

        /* recalculate the pSectorBase for chip commands in case a
           sector boundary has been crossed:
        */
        if(!FfxFimNorWindowMap(pEMI->hDev,
                               ulCurrentOffset & (~(pEMI->ulEraseZoneSize - 1)),
                               (volatile void **)&pSectorBase))
            return FALSE;

        /*  Issue Write to Buffer Command (Block Start)
        */
        if (bIsControl)
        {
            IssueCAP1Command(pSectorBase, ECLIPSECMD_BITFIELD_LOAD);
        }
        else
        {
            IssueCAP1Command(pSectorBase, ACMD_WRITETOBUFFER);
        }

        /*  Write byte count (Block Start)
        */
        uByteCount = (D_UINT16) ulLength;

        /*  Make sure uByteCount is within uMaxWriteBuff range.
        */
        if(bIsControl)
        {
            if(uByteCount > (pEMI->pFimExtra->uMaxWriteBuff>>1))
            {
               uByteCount = (pEMI->pFimExtra->uMaxWriteBuff>>1);
            }
        }
        else
        {
            if(uByteCount > pEMI->pFimExtra->uMaxWriteBuff)
            {
               uByteCount = pEMI->pFimExtra->uMaxWriteBuff;
            }
        }

        uByteCount = (D_UINT16) DCLMIN(uByteCount, uByteBuffSize);

        ulLength -= uByteCount;
		DclAssert((D_UINT32_MAX - uByteCount) > ulCurrentOffset);
        ulCurrentOffset += uByteCount;

        /*  Issue the word count. In 16 bit mode, we write out
            words instead of bytes
        */
        IssueCAP2Command(pSectorBase, (FLASHDATA)((uByteCount / FLASH_BUS_BYTES) - 1));

        /*  Write Buffer Data, (Start Address)
        */
        for(i = 0; i < (uByteCount / FLASH_BUS_BYTES); i++)
        {
            pMedia[uFlashIndex + i] = puData[uBuffIndex + i];
        }

        /*  Program Buffer to Flash Confirm
        */
        IssueCAP1Command(pSectorBase, AMDCMD_BUFFERTOFLASH);

        _sysdelay(WSM_DELAY);

        if(!IsStateMachineReady(pSectorBase))
            return FALSE;

        /*  Check for error
        */

        SectorStatus = ReadStatus(pSectorBase);
        if(SectorStatus & ECLIPSESTAT_PROGRAM_STATUS)
        {
            FFXPRINTF(2, ("program failure with status = %08lx\n", pMedia[uFlashIndex]));
            FFXPRINTF(2, ("uFlashIndex=%04x\n", uFlashIndex));
            FFXPRINTF(2, ("read #2 status = %08lx\n", pMedia[uFlashIndex]));
            FFXPRINTF(2, ("read #3 status = %08lx\n", pMedia[uFlashIndex]));
            DclError();
            return FALSE;
        }

        /*  Update index.
        */
        uBuffIndex += uByteCount / FLASH_BUS_BYTES;

        /*  uFlashIndex update is dependent on memory access type.
        */
        if(bIsControl)
        {
            /*  Must increment past invalid flash area.
            */
            uFlashIndex += (uByteCount + FLASH_BITFIELD_INVALID_SIZE)/FLASH_BUS_BYTES;

            /*  Make sure uByteBuffSize is set to maximum size if it is
                not already.
            */
            uByteBuffSize = FLASH_BITFIELD_VALID_SIZE;
        }
        else
        {
            uFlashIndex = uBuffIndex;

            /*  Make sure uByteBuffSize is set to maximum size if it is
                not already.
            */
            uByteBuffSize = pEMI->pFimExtra->uMaxWriteBuff;
        }
    }

    return TRUE;
}

/*  Although Spansion Eclipse parts support a sector-locking mechanism,
    they are not sector-locked upon hard reset. With that in mind, this
    FIM does not imlement block-unlocking treatments. A framework for
    an unlocking procedure is left here for someone who may someday wish
    to manipulate the Eclipse sector protection mechanism, but in the
    default configuration it is not compiled in.
*/
#if 0

/*-------------------------------------------------------------------
    UnlockBlocks()

    Description
        Unlocks each block within the usable area of the flash array.
        Does not unlock the blocks within the reserved space.  This
        should only be called by Mount().

    Parameters
        ulLength - The total length of the flash array
        pBounds  - A pointer to the FFXFIMBOUNDS structure to use
        pEMI     - pointer to the extended medio info structure

    Return Value
        Returns TRUE if successful, else FALSE.
-------------------------------------------------------------------*/
static D_BOOL UnlockBlocks(
    D_UINT32        ulLength,
    FFXFIMBOUNDS   *pBounds,
    PEXTMEDIAINFO   pEMI)
{

    return TRUE;
}

#endif

/*-------------------------------------------------------------------
    FIMDEVICE Declaration

    This structure declaration is used to define the entry points
    into the FIM.  This is declared at the end of the module to
    eliminate the need for what would be duplicated function
    prototypes in all the FIMs.
-------------------------------------------------------------------*/
FIMDEVICE FFXFIM_eclipsex16 =
{
    Mount,
    Unmount,
    Read,
    Write,
    EraseStart,
    ErasePoll,
    EraseSuspend,
    EraseResume,
    ReadControl,
    WriteControl
};

#endif

