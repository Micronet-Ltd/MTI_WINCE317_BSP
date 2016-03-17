/*---------------------------------------------------------------------------
               ----> DO NOT REMOVE THE FOLLOWING NOTICE <----

                  Copyright (c) 1993 - 2007 Datalight, Inc.
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

    SXIP FIM support for Intel J3 StrataFlash (28F320J3A, 28F640J3A,
    and 23F128J3A).

    Calculations are performed by shifting and masking (and assuming
    2's complement arithmetic) rather than division and modulo as
    the latter two operations generate calls to helper functions that
    are located in a DLL in flash.  While the operations are performed
    in parts of code where it should be safe to do this, avoiding the
    operations altogether facilitates verification that the generated
    code is XIP-safe -- the XIP DLL will not build if it links to any
    external functions.

---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                              Revision History
    $Log: intl2x16.c $
    Revision 1.3  2007/11/03 23:50:12Z  Garyp
    Updated to use the standard module header.
    Revision 1.2  2006/02/07 03:35:58Z  Garyp
    Header updates.
    Revision 1.1  2005/10/25 23:11:36Z  Pauli
    Initial revision
    Revision 1.2  2005/10/26 00:11:36Z  Garyp
    Updated to use some renamed fields.
    Revision 1.1  2005/10/02 02:33:36Z  Garyp
    Initial revision
    Revision 1.1  2005/05/03 01:31:14Z  pauli
    Initial revision
    Revision 1.1  2005/05/03 01:31:14Z  pauli
    Initial revision
    Revision 1.6  2005/05/03 01:31:13Z  garyp
    Removed references to some obsolete fields.
    Revision 1.5  2005/01/28 21:47:15Z  GaryP
    Updated to use the new DCL headers and functions.
    Revision 1.4  2004/12/30 23:17:55Z  GaryP
    Updated to use indents of 4, hard tabs of 8 (if used), and a standard
    comment style.
    Revision 1.3  2004/08/04 22:00:15Z  billr
    Eliminate ulInterleaved in ExtndMediaInfo, no longer used or needed.
    Revision 1.2  2004/01/27 02:32:50Z  garys
    Merge from FlashFXMT
    Revision 1.1.1.2  2004/01/27 02:32:50  garyp
    Modified to use the newly renamed CPU layer functions.
    Revision 1.1  2002/11/13 20:21:00Z  qa
    Initial revision
---------------------------------------------------------------------------*/
#include <windows.h>

#include <flashfx.h>
#include <dlcpuapi.h>

#include <sxip.h>

#include "sxipplat.h"
#include "iJ3A.h"



/*  FIM-specific timing parameters.  ERASE_INCREMENT is the time from
    when an erase is started or resumed to when it is suspended to
    allow interrupts.  ERASE_COUNT is a timeout in terms of the maximum
    number of erase increments that will be tolerated.  It is set to
    roughly twice the chip specification for maximum erase time, taking
    into account the estimated overhead for suspending and resuming.
*/

#define ERASE_INCREMENT (500)   /* microseconds */
#define ERASE_COUNT ((2 * tBLOCK_ERASE) / (ERASE_INCREMENT - 2 * tERASE_SUSPEND))


/*  Macros for building commands/status masks for two chips.
*/

#define LOW_CHIP(x) ((FLASHDATA) (x))
#define HIGH_CHIP(x) ((FLASHDATA) (x) << 16)
#define MAKE_MASK(high, low) (HIGH_CHIP(high) | LOW_CHIP(low))
#define MAKE_CMD(high, low) MAKE_MASK(high, low)
#define BOTHCHIPS(x) MAKE_MASK(x, x)


/*  Commands for both chips
*/

#define i2IDENTIFY              BOTHCHIPS(READ_ID)
#define i2READ_MODE             BOTHCHIPS(READ_ARRAY)
#define i2ERASE                 BOTHCHIPS(BLOCK_ERASE)
#define i2ERASE_START           BOTHCHIPS(BLOCK_ERASE_CONFIRM)
#define i2READ_STATUS           BOTHCHIPS(READ_STATUS)
#define i2CLEAR_STATUS          BOTHCHIPS(CLEAR_STATUS)
#define i2WRITE_TO_BUFF         BOTHCHIPS(WRITE_BUFFER)
#define i2WRITE_FLASH_CONFIRM   BOTHCHIPS(WRITE_BUFFER_CONFIRM)

/*  Status for both chips
*/

#define i2BIT_ERASE_SUSPENDED   BOTHCHIPS(SR_ESS)
#define i2BIT_DONE              BOTHCHIPS(SR_DONE)
#define i2VALID_STATUS_BITS     BOTHCHIPS(SR_VALID)

/*  Extended status for both chips
*/

#define i2BIT_BUFFER_AVAILABLE  BOTHCHIPS(XSR_WBS)


/*  Erase zone size (2 chips)
*/
#define ZONE_SIZE     (2 * ERASE_BLOCK_SIZE)

/*  Write buffer size.  This is expected always to be a power of 2 and
    a multiple of 4 bytes.
*/

#define WRITE_BUFFER_SIZE (2 * BUFFER_BYTES)    /* there are 2 chips */
#define WRITE_BUFFER_SIZE_MASK (WRITE_BUFFER_SIZE - 1)

/*  Flash data bus width and corresponding data type.
*/

typedef D_UINT32 FLASHDATA;    /* guaranteed to be 32 bits (4 bytes) */

#define FLASH_DATA_BUS_WIDTH (4)
#define FLASH_DATA_BUS_WIDTH_SHIFT (2)



/*  Macro to read ID information and construct a combined manufacturer/device
    ID code.  Assumes FLASHDATA *p.
*/

#define READ_ID_CODE(p) ((p)[0] | ((p)[1] << 8))

/*  Combined manufacturer/device ID codes for supported chips.
*/
#define ID_CODE_28F128_2x16     0x18891889L     /* 256MB (32Mb)    part */
#define ID_CODE_28F640_2x16     0x17891789L     /* 128MB (16Mb) 3V part */
#define ID_CODE_28F320_2x16     0x16891689L     /* 64MB  (8Mb ) 3V part */


/*  Private function prototypes.
*/

static D_BOOL   Probe(
    void *,
    D_UINT32,
    ExtndMediaInfo *);
static D_UINT32 WriteBuffer(
    void *,
    void *,
    D_UINT32);
static void     CancelWrite(
    volatile FLASHDATA *,
    FLASHDATA);
static void     bufcopy(
    FLASHDATA *,
    const FLASHDATA *,
    D_UINT32);
static D_BOOL   EraseBlock(
    void *);




/*------------------------------------------------------------------------
    Parameters:
       dwInst - Value that identifies the instance of the ISR handler
          (not used).
       dwIoControlCode - Specifies the control code for the operation.
       lpInBuf - Pointer to a buffer that contains the data required to
          perform the operation.
       nInBufSize - The size, in bytes, of the buffer pointed to by
          lpInBuffer.
       lpOutBuf - Pointer to a buffer that receives the operation's output
          data. This parameter can be NULL if the dwIoControlCode parameter
          specifies an operation that does not produce output data.
       nOutBufSize - The size, in bytes, of the buffer pointed to by
          lpOutBuffer.
       lpBytesReturned - Pointer to a variable that receives the size,
          in bytes, of the data stored into the buffer pointed to by
          lpOutBuffer.  May be NULL.
    Notes:
       IOCTL interface for the SXIP "installable ISR" DLL.  See the
       Windows CE.net documentation for additional details on this
       interface.

       Supported values of dwIoControlCode and their parameters:

       KERNEL_IOCTL_SXIP_PROBE - probe for flash
          PROBE_INPUT *lpInBuf
             pFlashBase - base virtual address of flash region to probe
             windowSize - size of region to probe
          nInBufSize = sizeof (PROBE_INPUT)
          ExtndMediaInfo *lpOutBuf - receives information about flash
             detected, unchanged if none found.
          nOutBufSize = sizeof (ExtndMediaInfo)

       KERNEL_IOCTL_SXIP_WRITE - write data to flash
          WRITE_INPUT *lpInBuf
             pFlash - virtual address of location in flash to which to
                write.  Must be aligned on four byte boundary.
             pData - virtual address of user buffer containing data to
                write to flash.  Must be aligned on four byte boundary.
             length - length of data to write.  Must be a multiple
                of four bytes.
          nInBufSize = sizeof (WRITE_INPUT)
          D_UINT32 *lpOutBuf - receives number of bytes actually
             written.  Result is always a multiple of four bytes
             and is never zero.  Not modified if write fails.
          nOutBufSize = sizeof (D_UINT32)

       KERNEL_IOCTL_SXIP_ERASE - erase a single physical erase block
          ERASE_INPUT *lpInBuf
             pFlash - virtual address of physical erase block in flash
                to erase.  Must be aligned to an erase block boundary.
          nInBufSize = sizeof (ERASE_INPUT)
          lpOutBuf - not used
          nOutBufSize - not used

    Returns:
       TRUE if the IOCTL succeeded, FALSE if it failed.  Note that
       because this DLL cannot call external APIs, it cannot use
       SetLastError() to return extended error information.

------------------------------------------------------------------------*/

BOOL localIOControl(
    DWORD dwInst,
    DWORD dwIoControlCode,
    LPVOID lpInBuf,
    DWORD nInBufSize,
    LPVOID lpOutBuf,
    DWORD nOutBufSize,
    LPDWORD lpBytesReturned)
{
    BOOL            result = FALSE;

    switch (dwIoControlCode)
    {

        case KERNEL_IOCTL_SXIP_PROBE:
            if(lpInBuf != NULL
               && nInBufSize == sizeof(PROBE_INPUT)
               && lpOutBuf != NULL && nOutBufSize == sizeof(ExtndMediaInfo))
            {
                PROBE_INPUT    *input = (PROBE_INPUT *) lpInBuf;

                if(Probe(input->pFlashBase, input->windowSize,
                         (ExtndMediaInfo *) lpOutBuf))
                {
                    if(lpBytesReturned)
                        *lpBytesReturned = sizeof(ExtndMediaInfo);
                    result = TRUE;
                }
            }
            break;

        case KERNEL_IOCTL_SXIP_WRITE:
            if(lpInBuf != NULL
               && nInBufSize == sizeof(WRITE_INPUT)
               && lpOutBuf != NULL && nOutBufSize == sizeof(D_UINT32))
            {
                WRITE_INPUT    *input = (WRITE_INPUT *) lpInBuf;
                D_UINT32       written =
                    WriteBuffer(input->pFlash, input->pData, input->length);

                if(written != 0)
                {
                    *(D_UINT32 *) lpOutBuf = written;
                    if(lpBytesReturned)
                        *lpBytesReturned = sizeof(D_UINT32);
                    result = TRUE;
                }
            }
            break;

        case KERNEL_IOCTL_SXIP_ERASE:
            if(lpInBuf != NULL && nInBufSize == sizeof(ERASE_INPUT))
            {
                ERASE_INPUT    *input = (ERASE_INPUT *) lpInBuf;

                if(EraseBlock(input->pFlash))
                {
                    result = TRUE;
                    if(lpBytesReturned)
                        *lpBytesReturned = 0;
                }
            }
            break;

        default:
            break;
    }

    return result;
}





/*------------------------------------------------------------------------
    Parameters:
       pFlashBase - the base virtual address of the region to probe
          for flash.
       windowSize - the size of the region starting at pFlashBase
       pExtndMediaInfo - the ExtndMediaInfo structure to fill in with
          information about the flash found.

    Notes:
       Tests for the presence of any of the flash types that can be
       handled.  If it is found, the ExtndMediaInfo structure is filled
       in with information describing it.  If no flash is found, the
       structure is not modified.

       This function must be called with interrupts disabled.

    Returns:
       TRUE if compatible flash was found, FALSE if not.
------------------------------------------------------------------------*/

static D_BOOL Probe(
    void * pFlashBase,
    D_UINT32 windowSize,
    ExtndMediaInfo * pExtndMediaInfo)
{
    FLASHDATA       idCode;
    volatile FLASHDATA *pMedia = pFlashBase;
    volatile FLASHDATA *pProbe;
    size_t          deviceSize;
    D_UINT32       intState;
    D_BOOL          detected = TRUE;

    if(pFlashBase == NULL || windowSize == 0)
    {
        return FALSE;
    }

    intState = DclCpuInterruptDisable();
    *pMedia = i2IDENTIFY;
    idCode = READ_ID_CODE(pMedia);

    /*  Select the appropriate type of chip and save its size to be
        stored later in the ExtndMediaInfo structure.
    */

    switch (idCode)
    {
        case ID_CODE_28F128_2x16:
            deviceSize = CHIP_SIZE_28F128 * 2;
            break;
        case ID_CODE_28F640_2x16:
            deviceSize = CHIP_SIZE_28F640 * 2;
            break;
        case ID_CODE_28F320_2x16:
            deviceSize = CHIP_SIZE_28F320 * 2;
            break;
        default:
            detected = FALSE;
    }

    /*  Search for more pairs of the same type of chips starting right
        after the first pair.  To detect aliasing, the first pair is
        left in Identify mode while probing at device size increments
        beyond there.  Note that this is not 100% reliable, as a
        subsequent pair of chips might just happen to contain data
        matching the ID code.
    */

    if(detected)
    {
        for(pProbe = pMedia + (deviceSize >> FLASH_DATA_BUS_WIDTH_SHIFT);
            pProbe < pMedia + (windowSize >> FLASH_DATA_BUS_WIDTH_SHIFT);
            pProbe += (deviceSize >> FLASH_DATA_BUS_WIDTH_SHIFT))
        {
            FLASHDATA       probeIdCode;

            /*  If this next address returns the ID code of the first pair of
                chips, it's probably aliased, so quit the search.
            */

            if(READ_ID_CODE(pProbe) == idCode)
            {
                break;
            }

            /*  Try to ID a pair of chips at this address.  If the ID doesn't
                match the first pair, quit the search.
            */

            *pProbe = i2IDENTIFY;
            probeIdCode = READ_ID_CODE(pProbe);
            *pProbe = i2READ_MODE;
            (void)*pProbe;      /* flush write */
            if(probeIdCode != idCode)
            {
                break;
            }
        }

        /*  Place the first pair of chips back in Read Array mode.
        */

        *pMedia = i2READ_MODE;
        (void)*pMedia;          /* flush write */
        DclCpuFlushICache();

        /*  Fill in all the information about the flash that was found.
        */

        /*  The search loop left pProbe at the first location after the
            last pair of chips found, so the size is the difference (in
            chars) of pProbe and pMedia.  If this is greater than the
            window size, limit it to the window size.

            NOTE: This is different from previous practice, which would
            report a size equal to a multiple of the chip size even if
            that exceeded the window size (which could be detected here)
            or would run off the end of a chip because WindowMap(0) did
            not map to the beginning of the first chip (which could not
            be detected here).
        */

        /*  (pProbe - pMedia) is positive, as pProbe was formed by adding
            positive offsets to pMedia.
        */
        pExtndMediaInfo->ulTotalSize = pExtndMediaInfo->ulTotalPhysicalSize =
            min((unsigned)((char *)pProbe - (char *)pMedia), windowSize);

        /*  Set all of the fields in the ExtndMediaInfo structure that are
            the same for all chips and configurations.
        */
        pExtndMediaInfo->uDeviceType = DEV_NOR;
        pExtndMediaInfo->ulEraseZoneSize = ZONE_SIZE;
        pExtndMediaInfo->ulWindowSize = windowSize;
        pExtndMediaInfo->uPageSize = 0;
        pExtndMediaInfo->uSpareSize = 0;
    }

    DclCpuInterruptRestore(intState);
    return detected;
}




/*------------------------------------------------------------------------
    Parameters:
       pFlash - pointer to the location in flash to write to.  Must be
                aligned to a FLASHDATA boundary.
       pData -  pointer to user buffer containing data to write.  Must be
                aligned to a FLASHDATA boundary.
       length - length in bytes to write.  This may exceed the buffer
                size.  Must be a non-zero multiple of sizeof (FLASHDATA).
    Notes:
       Uses the Write to Buffer command to write to flash.  Depending
       on alignment of the destination, up to a full buffer is written.
    Returns:
       The number of bytes written.  Zero indicates an error.
------------------------------------------------------------------------*/

static D_UINT32 WriteBuffer(
    void *pFlash,
    void *pData,
    D_UINT32 length)
{
    volatile FLASHDATA *pMedia = pFlash;
    FLASHDATA       buffer[WRITE_BUFFER_SIZE / sizeof(FLASHDATA)];
    FLASHDATA      *pBuffer = buffer;
    D_UINT32       count;      /* count of FLASHDATA items contained in length */
    FLASHDATA       status;
    D_UINT32       intState;

    /*  The length cannot exceed the flash's write buffer size.  If the starting
        address in flash is not a write buffer boundary, further limit
        the length to write only up to the write buffer boundary.  This
        aligns subsequent writes, improving performance.
    */

    length = min(length,
                 (WRITE_BUFFER_SIZE -
                  ((uintptr_t) pMedia & WRITE_BUFFER_SIZE_MASK)));
    count = length >> FLASH_DATA_BUS_WIDTH_SHIFT;

    /*  If the count is zero, either length was zero (so why was this
        function called?) or it was less than sizeof (FLASHDATA).
    */

    if(count == 0)
    {
        return 0;
    }

    /*  Must copy data into a safe buffer - pData could point into
        the XIP image.
    */

    bufcopy(pBuffer, pData, count);

    /*  Clear any remaining status the chip may have from a previous
        operation, then check its status.
    */

    intState = DclCpuInterruptDisable();
    *pMedia = i2CLEAR_STATUS;
    *pMedia = i2READ_STATUS;

    if((*pMedia & i2VALID_STATUS_BITS) != i2BIT_DONE)
    {

        /*  The flash should always have been left in an idle state
            by previous operations (i.e., not in Program Suspend or
            Erase Suspend state), and the Clear Status command should
            have cleared any errors.  Any bits set other than the
            DONE bits indicate something is horribly wrong.
        */

        *pMedia = i2READ_MODE;
        (void)*pMedia;          /* flush write */
        DclCpuFlushICache();
        DclCpuInterruptRestore(intState);
        return 0;
    }

    /*  The chips say they are ready, so start the write.  Since
        they were ready, the status should always indicate "Buffer
        Available" for both chips.
    */

    *pMedia = i2WRITE_TO_BUFF;
    status = *pMedia;
    if((status & i2BIT_BUFFER_AVAILABLE) != i2BIT_BUFFER_AVAILABLE)
    {

        /*  One or both chips failed to report Buffer Available.
            Something is very wrong.  Get the flash back into
            Read Array mode and return an error indication.
        */

        CancelWrite(pMedia, status);
        DclCpuInterruptRestore(intState);
        return 0;
    }

    /*  Send the count to both chips, then feed them their respective
        data.  No timeout is set, as there is no way short of hardware
        reset to stop Write Buffer once it is confirmed.
    */

    *pMedia = BOTHCHIPS(count - 1);
    while(count--)
    {
        *pMedia++ = *pBuffer++;
    }
    pMedia = pFlash;

    *pMedia = i2WRITE_FLASH_CONFIRM;
    (void)*pMedia;              /* flush write */
    SXIP_delay(WSM_DELAY);

    while((*pMedia & i2BIT_DONE) != i2BIT_DONE)
    {
        /*  empty
        */
    }

    /*  Read the completion status, and return to Read Array mode.
    */

    status = *pMedia;
    *pMedia = i2READ_MODE;
    (void)*pMedia;              /* flush write */
    DclCpuFlushICache();
    DclCpuInterruptRestore(intState);

    return ((status & i2VALID_STATUS_BITS) == i2BIT_DONE ? length : 0);
}




/*------------------------------------------------------------------------
    Parameters:
       pMedia - pointer to the location in flash to write to.  Must be
                aligned to a FLASHDATA boundary.
       status - last status from the flash
    Notes:
       Cancel a Write Buffer command that was rejected by at least one
       chip (it didn't set the Buffer Available bit in the Extended
       Status Register).

       When this function returns, it is safe to return to XIP code.
    Returns:
       Nothing.
------------------------------------------------------------------------*/

static void CancelWrite(
    volatile FLASHDATA * pMedia,
    FLASHDATA status)
{

    if((status & i2BIT_BUFFER_AVAILABLE) != 0)
    {
        FLASHDATA       command;

        /*  One of the chips reported Buffer Available.  It's
            necessary to complete the command sequence by sending
            a dummy count and data, then abort the command sequence
            by sending a command other than Write Confirm.
        */

        if((status & LOW_CHIP(XSR_WBS)) != 0)
        {
            command = MAKE_CMD(READ_STATUS, 0);
        }
        else
        {
            command = MAKE_CMD(0, READ_STATUS);
        }

        /*  The chip that reported Buffer Available gets a count
            value of 0 (indicating one word), then data of 0; the
            other chip just gets ReadStatus commands.
        */

        *pMedia = command;
        *pMedia = command;
    }

    /*  Both chips should now be in Read Status mode, and should
        have their DONE bit set, possibly with some other status
        bits.  Check this, and don't continue until it's so.
    */

    do
    {
        *pMedia = i2READ_STATUS;
    }
    while((*pMedia & i2BIT_DONE) != i2BIT_DONE);

    /*  Now it should be possible to return to Read Array mode.
    */

    *pMedia = i2READ_MODE;
    (void)*pMedia;              /* flush write */
    DclCpuFlushICache();
}




/*------------------------------------------------------------------------
    Parameters:
       to - pointer to destination buffer.
       from - pointer to source data.
       count - number of FLASHDATA items to copy
    Notes:
       Special copy function to copy to a local buffer instead of
       using memcpy() (which can't be called from the XIP DLL because
       it is probably in flash).
    Returns:
       Nothing.
------------------------------------------------------------------------*/

static void bufcopy(
    FLASHDATA * to,
    const FLASHDATA * from,
    D_UINT32 count)
{
    while(count--)
    {
        *to++ = *from++;
    }
}




/*------------------------------------------------------------------------
    Parameters:
       pFlash - pointer to the location in flash to erase.  Must be
                aligned to a FLASHDATA boundary.
    Notes:
       Erase one physical erase block of flash.  The erase operation
       is done in brief increments: suspended while pending interrupts
       are allowed, then resumed.
    Returns:
       TRUE if the erase succeeded, FALSE if it did not.
------------------------------------------------------------------------*/

static D_BOOL EraseBlock(
    void *pFlash)
{
    volatile FLASHDATA *pMedia = pFlash;
    FLASHDATA       suspendCmd = BOTHCHIPS(BLOCK_ERASE_SUSPEND);
    FLASHDATA       resumeCmd = BOTHCHIPS(BLOCK_ERASE_RESUME);
    unsigned long   timer;
    unsigned long   countdown = ERASE_COUNT;
    FLASHDATA       status;
    D_UINT32       intState;

    /*  Clear any clearable errors from previous operations.
    */

    intState = DclCpuInterruptDisable();
    *pMedia = i2CLEAR_STATUS;

    /*  Start the erase operation.
    */

    *pMedia = i2ERASE;
    *pMedia = i2ERASE_START;
    SXIP_delay(WSM_DELAY);

    timer = SXIP_set_timer(ERASE_INCREMENT);
    while((*pMedia & i2BIT_DONE) != i2BIT_DONE)
    {
        if(SXIP_timeout(timer))
        {

            /*  The erase increment has elapsed and the erase isn't done.
                Suspend erase on the chip(s) still erasing and allow
                interrupts.
            */

            *pMedia = suspendCmd;
            while((*pMedia & i2BIT_DONE) != i2BIT_DONE)
            {
                /*  just have to wait, nothing more we can do!
                */
            }

            /*  It's possible that both chips finished erasing between the
                last status check and sending the suspend command.
            */

            status = *pMedia;
            if((status & i2BIT_ERASE_SUSPENDED) == 0)
            {
                break;          /* both chips finished */
            }

            /*  Check for timeout.  It's based on counting the number
                of erase increments completed.
            */

            if(--countdown == 0)
            {
                /*  Exit with at least one chip in Erase Suspend state.
                    Further erase or program operations will be prevented
                    by this condition.
                */

                break;
            }

            /*  At least one chip is still erasing.  If one has finished
                erasing, modify suspendCmd and resumeCmd to place that
                chip in Read Status mode.
            */

            if((status & LOW_CHIP(SR_ESS)) == 0)
            {                   /* low chip done */
                suspendCmd = MAKE_CMD(BLOCK_ERASE_SUSPEND, READ_STATUS);
                resumeCmd = MAKE_CMD(BLOCK_ERASE_RESUME, READ_STATUS);
            }
            else if((status & HIGH_CHIP(SR_ESS)) == 0)
            {                   /* high chip done */
                suspendCmd = MAKE_CMD(READ_STATUS, BLOCK_ERASE_SUSPEND);
                resumeCmd = MAKE_CMD(READ_STATUS, BLOCK_ERASE_RESUME);
            }

            /*  Return the flash to Read Array mode, and allow any pending
                interrupts to occur.
            */

            *pMedia = i2READ_MODE;
            (void)*pMedia;
            DclCpuFlushICache();
            DclCpuInterruptRestore(intState);
            DclCpuInterruptDisable();

            /*  Resume erasing.
            */

            *pMedia = resumeCmd;
            SXIP_delay(WSM_DELAY);
            timer = SXIP_set_timer(ERASE_INCREMENT);

        }                       /* end if (erase increment expired) */
    }                           /* end while (both chips not done erasing) */

    /*  Read the completion status, and return to Read Array mode.
    */

    status = *pMedia;
    *pMedia = i2READ_MODE;
    (void)*pMedia;              /* flush write */
    DclCpuFlushICache();
    DclCpuInterruptRestore(intState);

    return ((status & i2VALID_STATUS_BITS) == i2BIT_DONE);
}




/*  Standard DLL entry point.
*/

BOOL WINAPI DllMain(
    HANDLE hinstDLL,
    DWORD dwReason,
    LPVOID lpvReserved)
{
    return TRUE;
}
