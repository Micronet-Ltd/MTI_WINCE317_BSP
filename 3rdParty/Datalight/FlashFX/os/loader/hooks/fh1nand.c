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

    This module contains the default implementations of the OneNAND NTM Hooks
    functions.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: fh1nand.c $
    Revision 1.8  2010/02/12 21:42:17Z  garyp
    Updated to use the slightly revamped virtual memory mapping interface.
    Revision 1.7  2009/12/11 23:04:51Z  garyp
    Minor header and structure updates.
    Revision 1.6  2009/10/14 01:23:32Z  keithg
    Removed now obsolete fxio.h include file.
    Revision 1.5  2009/07/24 01:09:58Z  garyp
    Merged from the v4.0 branch.  Major cleanup to use similar functionality in
    all the OneNAND hooks modules.  Use a standard mechanism to report
    configuration characteristics back to the OneNAND NTM.  Made the
    debugging messages useful.  Support unaligned access.
    Revision 1.4  2009/04/08 19:30:59Z  davidh
    Function headers updated for AutoDoc.
    Revision 1.3  2008/09/29 19:53:08Z  glenns
    Added support for FlexOneNAND to be sure main page data is blank in the
    page register when doing spare-only writes.
    Revision 1.2  2007/11/03 23:49:54Z  Garyp
    Updated to use the standard module header.
    Revision 1.1  2006/10/04 01:04:22Z  brandont
    Initial revision
    Revision 1.6  2006/10/04 01:04:22Z  Garyp
    Updated to use DclMemAllocZero().
    Revision 1.5  2006/03/09 21:16:27Z  timothyj
    Removed unused unnecessary ONENANDINFO parameter
    Revision 1.4  2006/03/09 01:29:16Z  timothyj
    Updated default implementations of all functions such that typically
    FfxHookNTOneNANDCreate() and FfxHookNTOneNANDDestroy() are now the only
    functions that will need customization, to map and unmap the OneNAND
    addresses in an operating-system specific fashion.
    Revision 1.3  2006/02/14 00:48:33Z  Pauli
    Removed obsolete references to oem.h.
    Revision 1.2  2006/02/12 00:17:55Z  Garyp
    Eliminated use of the EXTMEDIAINFO structure.
    Revision 1.1  2006/02/03 23:30:34Z  timothyj
    Initial revision
---------------------------------------------------------------------------*/

#include <flashfx.h>
#include <fxnandapi.h>
#include <onenandconf.h>
#include <onenandctl.h>
#include <onenandregs.h>

/*  Basic OneNAND configuration parameters for this project, based on
    the hardware platform, and the implementation of this Hooks module.
*/
#define IO_WIDTH                    (ONENAND_DATA_WIDTH_BYTES)
#define CLIENT_BUFFER_ALIGNMENT     (1)     /* Module accommodates unaligned accesses */
#define USE_HARDWARE_ECC            (TRUE)  /* Absolutely! */
#define USE_SYNCHRONOUS_BURST_MODE  (FALSE) /* No synchronous burst mode */

struct sNTMHOOK
{
    ONENAND_IO_TYPE volatile *pMappedAddress;
    D_UINT32                    ulPhysMemBase;
};


/*-------------------------------------------------------------------
    Public: FfxHookNTOneNANDCreate()

    Allocate (if necessary) and initialize the OneNAND control
    structure.  Typically this will involve allocating memory
    for the OneNAND control structure, as well as mapping the
    OneNAND device's memory into the address space of the calling
    process.

    Parameters:
        hDev    - The Device handle.
        pONP    - A pointer to the FFXONENANDPARAMS structure to use.

    Return Value:
        Returns a pointer to an allocated data structure for this
        private data for this module, or NULL on failure.
-------------------------------------------------------------------*/
NTMHOOKHANDLE FfxHookNTOneNANDCreate(
    FFXDEVHANDLE        hDev,
    FFXONENANDPARAMS   *pONP)
{
    FFXSTATUS           ffxStat;
    D_UINT32            ulPhysMemBase;
    NTMHOOKHANDLE       hHook = NULL;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTMHOOK, 1, TRACEINDENT),
        "FfxHookNTOneNANDCreate() hDev=%P pONP=%P\n", hDev, pONP));

    DclAssert(hDev);
    DclAssert(pONP);
    DclAssert(pONP->nStructLen == sizeof(*pONP));
    DclAssert(ONENAND_DATA_WIDTH_BYTES <= 2);

    if(FfxHookOptionGet(FFXOPT_FLASH_START, hDev, &ulPhysMemBase, sizeof ulPhysMemBase))
    {
        FFXPRINTF(1, ("FfxHookNTOneNANDCreate() OneNAND base address (FFXOPT_FLASH_START): %lX\n", ulPhysMemBase));

        hHook = DclMemAllocZero(sizeof(*hHook));
        if(hHook)
        {
            hHook->ulPhysMemBase = ulPhysMemBase;
            
            /*  Insert additional code to initialize the OneNAND control
                structure here
            */
            ffxStat = DclOsVirtualPointerMap((void**)&hHook->pMappedAddress,
                ulPhysMemBase, ONENAND_REGS_EXTENT * sizeof(ONENAND_IO_TYPE), FALSE);

    	    if (ffxStat != FFXSTAT_SUCCESS)
    	    {
                FFXPRINTF(1, ("FfxHookNTOneNANDCreate() Unable to map address, status=%%lX\n", ffxStat));
                DclMemFree(hHook);
                hHook = NULL;
    	    }
          #if D_DEBUG
            else
            {
                FfxNtmOneNANDDumpRegisters(hHook);
            }
          #endif
        }
    }
    else
    {
        FFXPRINTF(1, ("FfxHookNTOneNANDCreate() FfxHookOptionGet for start address failed.\n"));
    }

    if(hHook)
    {
        pONP->nTimeoutMS            = ONENAND_TIME_OUT;
        pONP->nIOWidth              = IO_WIDTH;
        pONP->nAlignmentBoundary    = CLIENT_BUFFER_ALIGNMENT;
        pONP->fUseHardwareECC       = USE_HARDWARE_ECC;
        pONP->fUseSynchBurstMode    = USE_SYNCHRONOUS_BURST_MODE;
    }

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTMHOOK, 1, TRACEUNDENT),
        "FfxHookNTOneNANDCreate() returning OneNAND Control: %P\n", hHook));

    return hHook;
}


/*-------------------------------------------------------------------
    Public: FfxHookNTOneNANDDestroy()

    Deallocate (if necessary) and deinitialize the OneNAND control
    structure and corresponding OneNAND device.  Typically this will
    involve allocating memory for the OneNAND control structure, as
    well as mapping the OneNAND device's memory into the address
    space of the calling process.

    Parameters:
        hHook - The NTM Hook handle

    Return Value:
        An FFXSTATUS value indicating success or the nature of
        any failure.
-------------------------------------------------------------------*/
FFXSTATUS FfxHookNTOneNANDDestroy(
    NTMHOOKHANDLE   hHook)
{
    FFXSTATUS       ffxStat;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTMHOOK, 1, 0),
        "FfxHookNTOneNANDDestroy() hHook=%P\n", hHook));

    DclAssert(hHook);

    /*  Insert any additional code to deinitialize the OneNAND control
        structure here
    */

    ffxStat = DclOsVirtualPointerUnmap((void*)hHook->pMappedAddress,
                                        hHook->ulPhysMemBase,
                                        ONENAND_REGS_EXTENT * sizeof(ONENAND_IO_TYPE));

    DclAssert(ffxStat == FFXSTAT_SUCCESS);

    DclMemFree(hHook);

    return ffxStat;
}


/*-------------------------------------------------------------------
    Public: FfxHookNTOneNANDReadRegister()

    Read a value from the OneNAND register at the given offset,
    typically by using the address stored in the NTMHOOKHANDLE
    structure.

    Parameters:
        hHook   - The NTM Hook handle
        uOffset - Offset from the beginning of the OneNAND of the
                  register from which this function will read a
                  value.

    Return Value:
        Returns the value at the specified offset.
-------------------------------------------------------------------*/
ONENAND_IO_TYPE FfxHookNTOneNANDReadRegister(
    NTMHOOKHANDLE               hHook,
    D_UINT16                    uOffset)
{
    ONENAND_IO_TYPE             Value;
    ONENAND_IO_TYPE volatile   *pReg;

    pReg = (ONENAND_IO_TYPE volatile *)(hHook->pMappedAddress + uOffset);

    Value = *pReg;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTMHOOK, 3, 0),
        "FfxHookNTOneNANDReadRegister() hHook=%P uOffset=%X returning %u\n", hHook, uOffset, Value));

    return Value;
}

/*-------------------------------------------------------------------
    Public: FfxHookNTOneNANDReadBuffer()

    Read a buffer of data from the OneNAND device, starting at the
    specified offset, and extending for the specified uCount of
    ONENAND_IO_TYPE-size data.

    Parameters:
        hHook   - The NTM Hook handle
        pBuffer - Buffer into which the data will be stored.
        uOffset - Offset (in units of sizeof ONENAND_IO_TYPE)
                  from the beginning of the OneNAND of the register
                  from which this function will read a value.
        uCount  - The number of bytes to be copied from the
                  OneNAND buffer into the caller's buffer.

    Return Value:
        None.
-------------------------------------------------------------------*/
void FfxHookNTOneNANDReadBuffer(
    NTMHOOKHANDLE               hHook,
    void                       *pBuffer,
    D_UINT16                    uOffset,
    D_UINT16                    uCount)
{
    D_UINT16                    uCurIo;
    ONENAND_IO_TYPE volatile   *pSrcBuffer;
    ONENAND_IO_TYPE            *pDstBuffer;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTMHOOK, 2, 0),
        "FfxHookNTOneNANDReadBuffer() hHook=%P pBuffer=%P uOffset=%X uCount=%X\n",
        hHook, pBuffer, uOffset, uCount));

    DclAssert(hHook);
    DclAssert(pBuffer);

    pSrcBuffer = hHook->pMappedAddress + uOffset;
    pDstBuffer = (ONENAND_IO_TYPE *)pBuffer;

    /*  NOTE:  This can often be optimized in a processor-dependent fashion.
               For instance, some processors can generate two 16-bit bus
               cycles per iteration of the loop if the memory is accessed
               as if it were 32 bits wide.
    */

  #if (ONENAND_DATA_WIDTH_BYTES == 2) && (DCL_CPU_ALIGNSIZE > 1)
    /*  Where OneNAND is mapped 16 bits wide, we need to account
        for alignment of the destination pointer on some processor
        architectures
    */
    if((D_UINTPTR)pDstBuffer & 1)
    {
        D_UINT8    *pDstByteBuffer = (D_UINT8*)pDstBuffer;

        /*  The destination pointer is unaligned, access it as a byte
            pointer, but use the source address as-is.
        */

        /*  This loop only works for even-numbered counts, which should
            be the case per the calling code.  Assert here, in case that
            ever changes we'll see the problem right here instead of some
            subtle, difficult-to track down failure.
        */
        DclAssert((uCount & 1) == 0);

        for(uCurIo = 0;
            uCurIo < uCount;
            uCurIo += sizeof(ONENAND_IO_TYPE))
        {
            D_UINT16    uSrc16;

            uSrc16 = *pSrcBuffer++;

          #if DCL_BIG_ENDIAN
            *pDstByteBuffer++ = (D_UINT8)(uSrc16 >> 8);
            *pDstByteBuffer++ = (D_UINT8)(uSrc16 & 0x00FF);
          #else
            *pDstByteBuffer++ = (D_UINT8)(uSrc16 & 0x00FF);
            *pDstByteBuffer++ = (D_UINT8)(uSrc16 >> 8);
          #endif
        }
    }
    else
  #endif
    {
        /*  Destination address is aligned, move the native amount
            in optimal fashion.
        */
        for(uCurIo = 0;
            uCurIo < uCount;
            uCurIo += sizeof(ONENAND_IO_TYPE))
        {
            *pDstBuffer++ = *pSrcBuffer++;
        }
    }

    return;
}


/*-------------------------------------------------------------------
    Public: FfxHookNTOneNANDWriteRegister()

    Write a value into the OneNAND register at the given offset,
    typically by using the address stored in the NTMHOOKHANDLE
    structure.

    Parameters:
        hHook   - The NTM Hook handle
        uOffset - offset from the beginning of the OneNAND of the
                  register into which this function will write a
                  value.
        Value   - value to place into the OneNAND register.

    Return Value:
        None.
-------------------------------------------------------------------*/
void FfxHookNTOneNANDWriteRegister(
    NTMHOOKHANDLE               hHook,
    D_UINT16                    uOffset,
    ONENAND_IO_TYPE             Value)
{
    ONENAND_IO_TYPE volatile   *pReg;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTMHOOK, 3, 0),
        "FfxHookNTOneNANDWriteRegister() hHook=%P uOffset=%X Value=%x\n",
        hHook, uOffset, Value));

    DclAssert(hHook);

    pReg = (ONENAND_IO_TYPE volatile *)(hHook->pMappedAddress + uOffset);

    *pReg = Value;

    return;
}


/*-------------------------------------------------------------------
    Public: FfxHookNTOneNANDWriteBuffer()

    Write a buffer of data to the OneNAND device, starting at the
    specified offset, and extending for the specified uCount of
    ONENAND_IO_TYPE-size data.

    Parameters:
        hHook   - The NTM Hook handle
        pBuffer - buffer from which the data will be copied.  The
                  address will be aligned on a sector (512B) boundary
                  and accessible in any units (byte, word, etc.).
        uOffset - offset from the beginning of the OneNAND of the
                  OneNAND buffer into which this function copy the
                  data.
        uCount  - the number of bytes to be copied from the caller's
                  buffer into the OneNAND buffer.

    Return Value:
        None.
-------------------------------------------------------------------*/
void FfxHookNTOneNANDWriteBuffer(
    NTMHOOKHANDLE               hHook,
    const void                 *pBuffer,
    D_UINT16                    uOffset,
    D_UINT16                    uCount)
{
    D_UINT16                    uCurIo;
    ONENAND_IO_TYPE            *pSrcBuffer;
    ONENAND_IO_TYPE volatile   *pDstBuffer;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTMHOOK, 2, 0),
        "FfxHookNTOneNANDWriteBuffer() hHook=%P pBuffer=%P uOffset=%X uCount=%X\n",
        hHook, pBuffer, uOffset, uCount));

    DclAssert(hHook);

    pSrcBuffer = (ONENAND_IO_TYPE *)pBuffer;
    pDstBuffer = hHook->pMappedAddress + uOffset;

    /*  NOTE:  This can often be optimized in a processor-dependant fashion.
               For instance, some processors can generate two 16-bit bus
               cycles per iteration of the loop if the memory is accessed
               as if it were 32 bits wide.
    */
	if (pBuffer)
	{
      #if (ONENAND_DATA_WIDTH_BYTES == 2) && (DCL_CPU_ALIGNSIZE > 1)
        /*  Where OneNAND is mapped 16 bits wide, we need to account
            for alignment of the destination pointer on some processor
            architectures
        */
        if((D_UINTPTR)pSrcBuffer & 1)
        {
            D_UINT8    *pSrcByteBuffer = (D_UINT8*)pSrcBuffer;

            /*  The source pointer is unaligned, access it as a byte
                pointer, but use the destination address as-is.
            */

            /*  This loop only works for even-numbered counts, which should
                be the case per the calling code.  Assert here, in case that
                ever changes we'll see the problem right here instead of some
                subtle, difficult-to track down failure.
            */
            DclAssert((uCount & 1) == 0);

            for(uCurIo = 0;
                uCurIo < uCount;
                uCurIo += sizeof(ONENAND_IO_TYPE))
            {
                D_UINT16    uDst16;

              #if DCL_BIG_ENDIAN
                uDst16 = ((D_UINT16)(*pSrcByteBuffer++)) << 8;
                uDst16 |= *pSrcByteBuffer++;
              #else
                uDst16 = *pSrcByteBuffer++;
                uDst16 |= ((D_UINT16)(*pSrcByteBuffer++)) << 8;
              #endif

                *pDstBuffer++ = uDst16;

            }
        }
        else
      #endif
        {
            /*  Source address is aligned, move the native amount
                in optimal fashion.
            */
            for(uCurIo = 0;
                uCurIo < uCount;
                uCurIo += sizeof(ONENAND_IO_TYPE))
            {
                *pDstBuffer++ = *pSrcBuffer++;
            }
        }
	}
	else
	{
        /*  If no data to write, write all ones to preserve existing data
        */
		for(uCurIo = 0;
			uCurIo < uCount;
			uCurIo += sizeof(ONENAND_IO_TYPE))
		{
			*pDstBuffer++ = ONENAND_BLANKDATA;
		}
	}

    return;
}


/*-------------------------------------------------------------------
    Public: FfxHookNTOneNANDReadyWait()

    Wait for a OneNAND operation to complete, typically by polling
    the interrupt status register, polling a GPIO for the OneNAND
    INT signal, or waiting for the processor interrupt to be
    signalled.

    Parameters:
        hHook - The NTM Hook handle

    Return Value:
        An FFXSTATUS value indicating success or the nature of
        any failure.
-------------------------------------------------------------------*/
FFXSTATUS FfxHookNTOneNANDReadyWait(
    NTMHOOKHANDLE       hHook)
{
    DCLTIMER            t;
    ONENAND_IO_TYPE     IntStatus = 0;
    FFXSTATUS           ffxStat;

    DclTimerSet(&t, ONENAND_TIME_OUT);

    /*  NOTE:  This can often be optimized in a processor, OS, and
               configuration-dependant fashion.  For instance, the INT
               pin from the OneNAND chip can be wired to a processor
               GPIO input, in which case this function could read the
               GPIO, which may be more efficient than reading a register
               across a bus.  Similarly, if the INT pin from the OneNAND
               chip is configured to interrupt the processor, and the
               operating system on which this code running supplies the
               appropriate primitives, this function could yield the
               processor and block the thread on an OS primitive (such
               as a semaphore), resulting in more CPU bandwidth
               available for handling other tasks.

               Many possibilities exist, and which are appropriate are
               heavily system and application dependant.
    */

    /*  Loop while no interrupt indication from the OneNAND chip's register
    */
    while (((IntStatus & ONENAND_REG_INTSTATUS_INT) == 0) && (!DclTimerExpired(&t)))
    {
        IntStatus = FfxHookNTOneNANDReadRegister(hHook, ONENAND_REG_INTERRUPT);
    }

	/*  If there was an interrupt (regardless of whether the timeout has
	    elapsed) then the function was successful.  Otherwise, to get here,
	    the timeout must have occurred.

        Read the register again in case the processor was busy between the
        time the interrupt occurred and the loop above was able to read it,
        and therefore time passed, but the device was indeed ready.
    */
    IntStatus = FfxHookNTOneNANDReadRegister(hHook, ONENAND_REG_INTERRUPT);

	if (IntStatus & ONENAND_REG_INTSTATUS_INT)
 	    ffxStat = FFXSTAT_SUCCESS;
	else
	    ffxStat = FFXSTAT_FIMTIMEOUT;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTMHOOK, 3, 0),
        "FfxHookNTOneNANDReadyWait() hHook=%P returning %lX\n", hHook, ffxStat));

    return ffxStat;
}



