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

    This FIM emulates NOR flash with RAM.  It is typically only used for
    testing and debugging purposes.  This FIM may be used to emulate specific
    flash parts and reference platforms, both in flash geometry and timing.
    Use the undocumented FFX_NORRAM_EMULATION_TARGET setting, in combination
    with the emulation targets defined in norram.h.

    Regardless whether an emulation target is specified or not, the standard
    options interface is still queried, and any configuration options
    specified therein will override those for the given emulation target.

    The following configuration option settings are used:

    FFXOPT_DEVICE_SETTINGS    - This returns the standard device settings as
                                specified in ffxconf.h, including base address,
                                high and low reserved sizes, and the max device
                                size.

    FFXOPT_DEVICE_BLOCKSIZE   - This is an optional setting that defines the
                                erase zone size to use.

    FFXOPT_FIM_TESTMEMORY     - This is an optional setting which is a boolean
                                flag that determines whether the memory is
                                tested prior to use.  If this value is not
                                set it will be assumed to be TRUE.

    If the DevSettings.pBaseFlashAddress for the Device is FFX_BADADDRESS.
    the FIM will allocate the memory block using DclMemAlloc().

    For a given project, a customized fhoption.c module may be used to provide
    project specific values for the settings described above.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: norram.c $
    Revision 1.37  2009/07/18 01:09:20Z  garyp
    Merged from the v4.0 branch.  Headers updated.
    Revision 1.36  2009/04/06 14:25:19Z  davidh
    Function headers updated for AutoDoc.
    Revision 1.35  2009/04/03 16:07:39Z  glenns
    - Fix Bugzilla #1253: Assign the ffxStat element of the FFXIOSTATUS
      return value to FFXSTAT_FIM_UNSUPPORTEDIOREQUEST
      in the default cases of the IORequest switch statement.
    Revision 1.34  2008/07/23 18:11:37Z  keithg
    Clarified comments with professional customer facing terms.
    Revision 1.33  2008/02/03 01:54:28Z  keithg
    comment updates to support autodoc.
    Revision 1.32  2007/12/20 08:42:59Z  Garyp
    Fixed to use an alignment value of DCL_CPU_ALIGNSIZE, in similar fashion
    to other FIMs and NTMs.
    Revision 1.31  2007/11/03 05:15:46Z  Garyp
    Fixed Bug 1585, where writes of 65536 or larger would hang the system.
    Added the "VERYSMALLPAGES" emulation target.
    Revision 1.30  2007/10/27 16:43:53Z  Garyp
    Updated to use DclBitHighest(), rather than the now obsolete FlashFX
    specific function.
    Revision 1.29  2007/08/04 01:11:52Z  timothyj
    Added assertion for block size divisible by 1024UL.
    Revision 1.28  2007/08/03 03:02:29Z  Garyp
    Fixed another probable merge error.
    Revision 1.27  2007/08/03 02:08:55Z  Garyp
    Fixed a (probable) merge error.
    Revision 1.26  2007/08/03 00:18:06Z  timothyj
    Changed units of reserved space and maximum size to be in terms
    of KB instead of bytes.
    Revision 1.25  2007/04/27 19:01:42Z  Garyp
    Updated so that when using an automatically determined block size, it is
    rounded as necessary to ensure that it is always a power-of-two.
    Revision 1.24  2007/04/06 01:52:42Z  keithg
    Changed the behavior to prevent the creation of disks with erase block
    sizes that were not power of two.  Added EFFICIENCY target emulation.
    Revision 1.23  2007/04/03 20:23:55Z  Garyp
    Updated the erase algorithms to more closely approximate the operations
    done on real flash, in similar fashion to what is done in other FIMs.
    In particular, suspending and resuming erases when emulating timing
    characteristics is much more accurate.
    Revision 1.22  2007/03/02 21:21:30Z  timothyj
    Added computation of simulated uPagesPerBlock to FFXPARAM_PAGE_SIZE handler.
    Revision 1.21  2007/01/31 21:14:07Z  Garyp
    Updated to allow erase-suspend support to be compiled out of the product.
    Revision 1.20  2007/01/20 16:55:01Z  Garyp
    Added debug code.
    Revision 1.19  2007/01/03 23:57:37Z  Garyp
    Removed some dead code.
    Revision 1.18  2007/01/02 21:08:04Z  Garyp
    Fixed so that a dynamically determined erase block size will never be
    larger than 512KB.
    Revision 1.17  2006/12/26 22:09:32Z  Garyp
    Fixed to build cleanly.
    Revision 1.16  2006/12/22 21:42:09Z  Garyp
    Major update to allow emulating the NOR flash on specific target boards.
    both in terms of overall geometry as well as performance (see norram.h
    for more information).
    Revision 1.15  2006/12/22 18:32:31Z  billr
    Fix uninitialized address if getting FFXOPT_DEVICE_SETTINGS fails.
    Revision 1.14  2006/11/08 18:23:00Z  Garyp
    Modified the ParameterGet/Set() functions to return an FFXSTATUS value
    rather than a D_BOOL.
    Revision 1.13  2006/10/23 22:23:52Z  Pauli
    Added descriptive error messages if the NOR RAM fails Create().
    Revision 1.12  2006/10/18 17:52:40Z  billr
    Revert some of the changes made in revision 1.11.
    Revision 1.11  2006/10/10 23:54:08Z  billr
    Add FFXCONF_NORRAM_SIMULATE_DELAY to control simulating real flash timing.
    Revision 1.10  2006/08/18 20:45:37Z  Garyp
    Removed some bogus alignment checks.
    Revision 1.9  2006/05/21 18:15:48Z  Garyp
    Added support for emulating boot blocks, as well as emulating multiple
    chips.
    Revision 1.8  2006/03/19 04:56:18Z  Garyp
    Added pseudo erase suspend/resume support.
    Revision 1.7  2006/03/06 22:15:25Z  Garyp
    Modified to use standard device settings.  Fixed some bogus asserts.
    Revision 1.6  2006/02/15 02:00:53Z  Garyp
    Updated to build cleanly.
    Revision 1.5  2006/02/14 22:53:55Z  Garyp
    Updated to build properly if NOR and Sibley are disabled.
    Revision 1.4  2006/02/11 23:49:11Z  Garyp
    Tweaked to build cleanly.
    Revision 1.3  2006/02/10 21:09:23Z  Garyp
    Renamed the FIMDEVICE structure instantiation for clarity.
    Revision 1.2  2006/02/08 19:35:53Z  Garyp
    Updated to the new style FIM architecture using an IORequest interface.
    Revision 1.1  2005/10/14 02:08:04Z  Pauli
    Initial revision
    Revision 1.2  2005/09/20 19:01:08Z  pauli
    Replaced _syssleep with DclOsSleep.
    Revision 1.1  2005/08/03 19:31:54Z  pauli
    Initial revision
    Revision 1.11  2005/05/06 21:04:04Z  garyp
    Removed some unnecessary prototypes and moved the FIMDEVICE declaration to
    the end of the file.
    Revision 1.10  2005/04/12 08:18:27Z  garyp
    Modified so that if an erase zone size is not explicitly supplied, the
    default value used is scaled based on disk size, up to 256KB max.
    Revision 1.9  2004/12/30 21:38:05Z  GaryP
    Updated to use indents of 4, hard tabs of 8 (if used), and a standard
    comment style.
    Revision 1.8  2004/10/25 07:20:39Z  GaryP
    Tweaked the timing parameters.  Minor display output change.
    Revision 1.7  2004/09/23 23:14:50Z  billr
    Fix compiler warnings.
    Revision 1.6  2004/09/15 02:46:30Z  GaryP
    Modified to build cleanly using the ARM tools.
    Revision 1.5  2004/09/08 19:13:46Z  jaredw
    Fixed definition of function Read to match prototype (as well as API).
    This caused some a need for some type casting on variable pBuffer as well.
    Revision 1.4  2004/09/07 21:37:41Z  GaryP
    Added the ability to emulate flash timing characteristics.
    Revision 1.3  2004/08/24 03:28:18Z  GaryP
    Changed to default to a 64K total, and 8K zone size, if not specified.
    Revision 1.2  2004/08/11 00:51:52Z  GaryP
    Fixed Unmount() to properly examine the conditions under which we release
    resources.  Added debugging code and fixed docs.
    Revision 1.1  2004/08/06 20:05:08Z  GaryP
    Initial revision
---------------------------------------------------------------------------*/

#include <flashfx.h>

#if FFXCONF_NORSUPPORT

#include <fxdriverfwapi.h>
#include <fxdevapi.h>
#include <fimdev.h>
#include "nor.h"

/*-------------------------------------------------------------------
    See norram.h for a list of possible emulation targets, which
    allow this FIM to emulate the flash, both in geometry and
    timing, for a variety of reference platforms.  If an emulation
    target was not defined in ffxconf.h, use the default.
-------------------------------------------------------------------*/
#ifndef FFX_NORRAM_EMULATION_TARGET
#define FFX_NORRAM_EMULATION_TARGET     FFX_EMUL_DEFAULT
#endif

#include "norram.h"


/*  Set this value to FALSE to prevent the RAM from being tested at
    initialization time.  This is useful if very large or very slow
    "RAM" is being used such as would be the case with memory-mapped
    files.
*/
#define DEFAULT_TESTMEMORY      TRUE

/*  ulEraseResult value; used internally to clarify intent.
*/
#define ERASE_IN_PROGRESS       (0)

/*  The minimum amount of time (in microseconds) to let a suspended
    erase progress to ensure that some forward progress is made.
*/
#define MINIMUM_ERASE_INCREMENT (1000)

/*  MXIC datasheet specifies 15 seconds, double it here.
*/
#define ERASE_TIMEOUT           (30 * 1000L)

/*  Erase suspend timeout: Intel J3A spec sheet says 40 microseconds.
*/
#define ERASE_SUSPEND_TIMEOUT   (1)


/*-------------------------------------------------------------------
    The FIM instance data structure.
-------------------------------------------------------------------*/
struct tagFIMDATA
{
    D_BUFFER       *pMemoryBase;
    FIMINFO         FimInfo;
    DCLTIMER        tErase;
    D_UINT32        ulEraseStart;
    D_UINT32        ulEraseResult;
    D_UINT32        ulTimeoutRemaining;
  #if EMUL_TIMING
    DCLTIMESTAMP    tsErase;        /* time the erase was started/resumed */
    D_UINT32        ulEraseMS;      /* time remaining till erase is completed */
  #endif
    unsigned        fWeAllocedTheMemory : 1;
    unsigned        fEraseSuspended     : 1;
  #if EMUL_BOOT_BLOCKS
    unsigned        fInBootBlocks       : 1;
  #endif
};


static D_BOOL   Read(               FIMHANDLE hFim, D_UINT32 ulStart, D_UINT32 ulLength, void *pBuffer);
static D_BOOL   Write(              FIMHANDLE hFim, D_UINT32 ulStart, D_UINT32 ulLength, const void *pBuffer);
static D_BOOL   EraseStart(         FIMHANDLE hFim, D_UINT32 ulStart, D_UINT32 ulLength);
static D_UINT32 ErasePoll(          FIMHANDLE hFim);
#if FFXCONF_ERASESUSPENDSUPPORT
static D_BOOL   EraseSuspend(       FIMHANDLE hFim);
static void     EraseResume(        FIMHANDLE hFim);
#endif
#if EMUL_BOOT_BLOCKS
static D_BOOL   EraseBootBlockStart(FIMHANDLE hFim, D_UINT32 ulStartBB, D_UINT32 ulCount);
#endif


/*-------------------------------------------------------------------
    Public: Create()

    The function creates a FIM instance.

    Parameters:
        hDev - The device handle

    Return Value:
        Returns TRUE if successful else FALSE.
-------------------------------------------------------------------*/
static FIMHANDLE Create(
    FFXDEVHANDLE        hDev,
    const FIMINFO     **ppFimInfo)
{
    FIMDATA            *pFim = NULL;
    D_UINT32            ulTotalSize;
    D_UINT32            ulBlockSize;
    FFXDEVSETTINGS      DevSettings;
    unsigned            fTestMemory;

    DclAssert(hDev);
    DclAssert(ppFimInfo);

    /*  TBD: For LFA, if LFA and NOR switches are compatible, there will
        likely need to be some casting and use of the Datalight macros
    */
    if(!FfxHookOptionGet(FFXOPT_DEVICE_SETTINGS, hDev, &DevSettings, sizeof DevSettings))
    {
        DclPrintf("FFX: NORRAM unable to retrieve the Device settings!\n");
        return NULL;
    }

    pFim = DclMemAllocZero(sizeof *pFim);
    if(!pFim)
    {
        DclPrintf("FFX: NORRAM failed to allocate control structure memory.\n");
        return NULL;
    }

    pFim->FimInfo.uDeviceType = DEVTYPE_NOR;
    pFim->FimInfo.uDeviceFlags = EMUL_DEVFLAGS;

    /*  Fill in the total physical array size.  Special case if the value
        is D_UINT32_MAX, indicating that the Device size as specified in
        the ffxconf.h file (including all reserved space) should be used.
    */
    ulTotalSize = EMUL_TOTALSIZE;
    if(ulTotalSize == D_UINT32_MAX)
        ulTotalSize = (DevSettings.ulMaxArraySizeKB + DevSettings.ulReservedBottomKB + DevSettings.ulReservedTopKB) * 1024UL;

    DclAssert(ulTotalSize);

    if(!FfxHookOptionGet(FFXOPT_FIM_TESTMEMORY, hDev, &fTestMemory, sizeof fTestMemory))
    {
        fTestMemory = DEFAULT_TESTMEMORY;
    }

    if(!FfxHookOptionGet(FFXOPT_DEVICE_BLOCKSIZE, hDev, &ulBlockSize, sizeof ulBlockSize))
    {
        /*  A block size was not specified via the options interface, so
            figure out a good block size to use.
        */
      #if EMUL_ERASEZONESIZE == D_UINT32_MAX
        /*  This emulation target has a special flag to just scale the block
            size based on the total physical array size.  However don't use
            a block size larger than 512 because VBF will not compile by
            default.
        */
        ulBlockSize = ulTotalSize / 32;
        if(ulBlockSize > 512 * 1024UL)
        {
            ulBlockSize = 512 * 1024UL;
        }
        else
        {
            /*  Round down if necessary to make sure that the block size
                is a power of two.
            */
            if(!DCLISPOWEROF2(ulBlockSize))
               ulBlockSize = (1 << (DclBitHighest(ulBlockSize) - 1));
        }
      #else
        /*  Otherwise, use the block size specified by the emulation target.
        */
        ulBlockSize = EMUL_ERASEZONESIZE;
      #endif
    }

    DclAssert(ulBlockSize);

    if(!DCLISPOWEROF2(ulBlockSize))
    {
        DclPrintf("FFX: NORRAM Error -- Emulated block size is not a power-of-two!\n");
        DclProductionError();
        goto MountError;
    }

    DclAssert((ulBlockSize % 1024UL) == 0);
    DclAssert(DevSettings.ulReservedBottomKB % (ulBlockSize / 1024) == 0);
    DclAssert(DevSettings.ulReservedTopKB % (ulBlockSize / 1024) == 0);

    if(ulTotalSize % ulBlockSize)
        ulTotalSize -= ulTotalSize % ulBlockSize;

    if(DevSettings.pBaseFlashAddress != FFX_BADADDRESS)
    {
        if(DevSettings.pBaseFlashAddress == NULL)
        {
            DclPrintf("FFX: NORRAM Warning -- A RAM device with a base address of 0?\n");
        }

        pFim->pMemoryBase = DevSettings.pBaseFlashAddress;
    }
    else
    {
        /*  If the RAM disk memory has not yet been allocated, do it now.
        */
        pFim->pMemoryBase = DclMemAlloc(ulTotalSize);
        if(!pFim->pMemoryBase)
        {
            DclPrintf("FFX: NORRAM failed to allocate device storage memory.\n");
            goto MountError;
        }

        pFim->fWeAllocedTheMemory = TRUE;
    }

    pFim->FimInfo.ulTotalBlocks     = ulTotalSize / ulBlockSize;
    pFim->FimInfo.ulChipBlocks      = EMUL_BLOCKS_PER_CHIP;
    pFim->FimInfo.ulBlockSize       = ulBlockSize;
    pFim->FimInfo.uAlignSize        = DCL_CPU_ALIGNSIZE;

  #if EMUL_PAGE_SIZE
    DclAssert(ulBlockSize % EMUL_PAGE_SIZE == 0);
    DclAssert(ulBlockSize / EMUL_PAGE_SIZE <= D_UINT16_MAX);

    pFim->FimInfo.uPageSize         = EMUL_PAGE_SIZE;
    pFim->FimInfo.uPagesPerBlock    = (D_UINT16)(ulBlockSize / EMUL_PAGE_SIZE);
  #endif

  #if EMUL_BOOT_BLOCKS
    pFim->FimInfo.ulBootBlockSize = ulBlockSize / EMUL_BOOTBLOCK_DIVISOR;
    pFim->FimInfo.uBootBlockCountLow = EMUL_BOOTBLOCKS_LOW * EMUL_BOOTBLOCK_DIVISOR;
    pFim->FimInfo.uBootBlockCountHigh = EMUL_BOOTBLOCKS_HIGH * EMUL_BOOTBLOCK_DIVISOR;

    DclPrintf("FFX: NORRAM BootBlockSize=%lUKB Low=%U High=%U\n",
        pFim->FimInfo.ulBootBlockSize / 1024UL,
        pFim->FimInfo.uBootBlockCountLow,
        pFim->FimInfo.uBootBlockCountHigh);
  #endif

    DclPrintf("FFX: NORRAM DEVn BaseAddr=%P Size=%lUKB BlockSize=%lUKB fTest=%U\n",
         pFim->pMemoryBase, ulTotalSize / 1024UL, ulBlockSize / 1024UL, fTestMemory);

    if(fTestMemory)
    {
        D_UINT32       i;

        /*  This block of code ensures that we can actually read and write
            to the entire block of memory.  This is useful in systems where
            the memory allocation may not be successful for whatever reasons,
            and since this FIM is only used for testing, performance is not
            an issue.
        */
        for(i = 0; i < ulTotalSize; ++i)
        {
            D_UCHAR         uc;

            uc = pFim->pMemoryBase[(size_t) i];

            /*  Write out a different value
            */
            uc++;

            pFim->pMemoryBase[(size_t) i] = uc;

            /*  Make sure we can read it back
            */
            if(pFim->pMemoryBase[(size_t) i] != uc)
            {
                DclPrintf("FFX: NORRAM buffer test 1 failed at offset %lU\n", i);

                goto MountError;
            }

            /*  Write out the original value
            */
            uc--;

            pFim->pMemoryBase[(size_t) i] = uc;

            /*  Make sure it was restored successfully
            */
            if(pFim->pMemoryBase[(size_t) i] != uc)
            {
                DclPrintf("FFX: NORRAM buffer test 2 failed at offset %lU\n", i);

                goto MountError;
            }
        }
    }

    *ppFimInfo = &pFim->FimInfo;
    return pFim;

  MountError:

    if(pFim)
    {
        if(pFim->pMemoryBase && pFim->fWeAllocedTheMemory)
            DclMemFree(pFim->pMemoryBase);

        DclMemFree(pFim);
    }

    return NULL;
}


/*-------------------------------------------------------------------
    Public: Destroy()

    Destroy a FIM instance and release any allocated resources.

    Parameters:
        hFim - The FIM instance handle

    Return Value:
        None
-------------------------------------------------------------------*/
static void Destroy(
    FIMHANDLE  hFim)
{
    DclAssert(hFim);
    DclAssert(hFim->pMemoryBase);

    if(hFim->pMemoryBase && hFim->fWeAllocedTheMemory)
        DclMemFree(hFim->pMemoryBase);

    DclMemFree(hFim);

    return;
}


/*-------------------------------------------------------------------
    Pulic: ParameterGet()

    Get a FlashFX system parameter.  If the parameter ID is not
    recognized for this layer, the request will be passed to the
    next lower software layer.

    Parameters:
        hFim      - The FIM instance handle
        id        - The parameter identifier (FFXPARAM_*).
        pBuffer   - A pointer to the buffer in which to store the
                    value.  May be NULL.
        ulBuffLen - The size of buffer.  May be zero if pBuffer is
                    NULL.

    Return Value:
        Returns an FFXSTATUS code indicating success or failure.
-------------------------------------------------------------------*/
static FFXSTATUS ParameterGet(
    FIMHANDLE       hFim,
    FFXPARAM        id,
    void           *pBuffer,
    D_UINT32        ulBuffLen)
{
    FFXSTATUS       ffxStat = FFXSTAT_BADPARAMETER;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_CONFIG, 1, TRACEINDENT),
        "NORRAM:ParameterGet() hFim=%P ID=%x pBuff=%P Len=%lU\n",
        hFim, id, pBuffer, ulBuffLen));

    FFXPRINTF(1, ("NORRAM:ParameterGet() Unsupported FFXPARAM ID %u\n", id));

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_CONFIG, 1, TRACEUNDENT),
        "NORRAM:ParameterGet() returning status%lX\n", ffxStat));

    return ffxStat;
}


/*-------------------------------------------------------------------
    Public: ParameterSet()

    Set a FlashFX system parameter.  If the parameter ID is not
    recognized for this layer, the request will be passed to the
    next lower software layer.

    Parameters:
        hFim      - The FIM instance handle
        id        - The parameter identifier (FFXPARAM_*).
        pBuffer   - A pointer to the parameter data.
        ulBuffLen - The parameter data length.

    Return Value:
        Returns an FFXSTATUS code indicating success or failure.
-------------------------------------------------------------------*/
static FFXSTATUS ParameterSet(
    FIMHANDLE       hFim,
    FFXPARAM        id,
    const void     *pBuffer,
    D_UINT32        ulBuffLen)
{
    FFXSTATUS       ffxStat = FFXSTAT_BADPARAMETER;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_CONFIG, 1, TRACEINDENT),
        "NORRAM:ParameterSet() hFim=%P ID=%x pBuff=%P Len=%lU\n",
        hFim, id, pBuffer, ulBuffLen));

    switch(id)
    {
        case FFXPARAM_PAGE_SIZE:
            if(ulBuffLen == sizeof hFim->FimInfo.uPageSize)
            {
                hFim->FimInfo.uPageSize = *(D_UINT16*)pBuffer;
                hFim->FimInfo.uPagesPerBlock = (D_UINT16)(hFim->FimInfo.ulBlockSize / hFim->FimInfo.uPageSize);
                ffxStat = FFXSTAT_SUCCESS;
            }
            break;

        default:
            FFXPRINTF(1, ("NORRAM:ParameterSet() Unsupported FFXPARAM ID %u\n", id));
            break;
    }

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_CONFIG, 1, TRACEUNDENT),
        "NORRAM:ParameterSet() returning status=%lX\n", ffxStat));

    return ffxStat;
}


/*-------------------------------------------------------------------
    Public: IORequest()

    Parameters:
        hFim - The FIM instance handle
        pIOR - A pointer to the FFXIOREQUEST structure

    Return Value:
-------------------------------------------------------------------*/
static FFXIOSTATUS IORequest(
    FIMHANDLE           hFim,
    FFXIOREQUEST       *pIOR)
{
    FFXIOSTATUS         ioStat;

    DclAssert(pIOR);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_FML, 2, TRACEINDENT),
        "NORRAM:IORequest() hFim=%P Func=%x\n", hFim, pIOR->ioFunc));

    DclProfilerEnter("NORRAM:IORequest", 0, 0);

    DclAssert(hFim);

    DclMemSet(&ioStat, 0, sizeof ioStat);

    switch(pIOR->ioFunc)
    {
      #if EMUL_BOOT_BLOCKS
        case FXIOFUNC_FIM_ERASE_BOOT_BLOCK_START:
        {
            FFXIOR_FIM_ERASE_BOOT_BLOCK_START *pReq = (FFXIOR_FIM_ERASE_BOOT_BLOCK_START*)pIOR;

            DclAssert(pReq->ior.ulReqLen == sizeof *pReq);

            ioStat.ulFlags = IOFLAGS_BLOCK;

            if(!EraseBootBlockStart(hFim, pReq->ulStartBB, pReq->ulBBCount))
                ioStat.ffxStat = FFXSTAT_FIM_ERASEFAILED;
            else
                ioStat.ffxStat = FFXSTAT_SUCCESS;

            break;
        }
      #endif

        case FXIOFUNC_FIM_ERASE_START:
        {
            FFXIOR_FIM_ERASE_START *pReq = (FFXIOR_FIM_ERASE_START*)pIOR;

            DclAssert(pReq->ior.ulReqLen == sizeof *pReq);

            ioStat.ulFlags = IOFLAGS_BLOCK;

            if(!EraseStart(hFim,
                pReq->ulStartBlock * hFim->FimInfo.ulBlockSize,
                pReq->ulCount * hFim->FimInfo.ulBlockSize))
            {
                ioStat.ffxStat = FFXSTAT_FIM_ERASEFAILED;
            }
            else
            {
                ioStat.ffxStat = FFXSTAT_SUCCESS;
            }

            break;
        }

        case FXIOFUNC_FIM_ERASE_POLL:
        {
            FFXIOR_FIM_ERASE_POLL   *pReq = (FFXIOR_FIM_ERASE_POLL*)pIOR;
            D_UINT32                ulLength;

            (void)pReq;
            DclAssert(pReq->ior.ulReqLen == sizeof *pReq);

            ioStat.ulFlags = IOFLAGS_BLOCK;

            ulLength = ErasePoll(hFim);
            if(ulLength == ERASE_FAILED)
            {
                ioStat.ffxStat = FFXSTAT_FIM_ERASEFAILED;
            }
            else
            {
                if(ulLength)
                {
                  #if EMUL_BOOT_BLOCKS
                    if(hFim->fInBootBlocks)
                    {
                        DclAssert(ulLength % hFim->FimInfo.ulBootBlockSize == 0);
                        ioStat.ulCount = ulLength / hFim->FimInfo.ulBootBlockSize;
                    }
                    else
                  #endif
                    {
                        DclAssert(ulLength % hFim->FimInfo.ulBlockSize == 0);
                        ioStat.ulCount = ulLength / hFim->FimInfo.ulBlockSize;
                    }
                    ioStat.ffxStat = FFXSTAT_SUCCESS;
                }
                else
                {
                    ioStat.ffxStat = FFXSTAT_FIM_ERASEINPROGRESS;
                }
            }

            break;
        }

      #if FFXCONF_ERASESUSPENDSUPPORT
        case FXIOFUNC_FIM_ERASE_SUSPEND:
        {
            FFXIOR_FIM_ERASE_SUSPEND *pReq = (FFXIOR_FIM_ERASE_SUSPEND*)pIOR;

            (void)pReq;
            DclAssert(pReq->ior.ulReqLen == sizeof *pReq);

            ioStat.ulFlags = IOFLAGS_BLOCK;

            if(!EraseSuspend(hFim))
                ioStat.ffxStat = FFXSTAT_FIM_ERASEFAILED;
            else
                ioStat.ffxStat = FFXSTAT_SUCCESS;

            break;
        }

        case FXIOFUNC_FIM_ERASE_RESUME:
        {
            FFXIOR_FIM_ERASE_RESUME *pReq = (FFXIOR_FIM_ERASE_RESUME*)pIOR;

            (void)pReq;
            DclAssert(pReq->ior.ulReqLen == sizeof *pReq);

            ioStat.ulFlags = IOFLAGS_BLOCK;

            EraseResume(hFim);
            ioStat.ffxStat = FFXSTAT_SUCCESS;

            break;
        }
      #endif

        case FXIOFUNC_FIM_READ_GENERIC:
        {
            FFXIOR_FIM_READ_GENERIC *pReq = (FFXIOR_FIM_READ_GENERIC*)pIOR;

            DclAssert(pReq->ior.ulReqLen == sizeof *pReq);
            DclAssert(pReq->ulCount);
            DclAssert(hFim->FimInfo.uPageSize);

            /*  All these operations are page ops.
            */
            ioStat.ulFlags = IOFLAGS_PAGE;

            switch(pReq->ioSubFunc)
            {
                case FXIOSUBFUNC_FIM_READ_PAGES:
                {
                    DclAssert(pReq->pPageData);

                    if(!Read(hFim,
                        pReq->ulStartPage * hFim->FimInfo.uPageSize,
                        pReq->ulCount * hFim->FimInfo.uPageSize,
                        pReq->pPageData))
                    {
                        ioStat.ffxStat = FFXSTAT_FIMIOERROR;
                    }
                    else
                    {
                        ioStat.ffxStat = FFXSTAT_SUCCESS;
                        ioStat.ulCount = pReq->ulCount;
                    }

                    break;
                }

                default:
                {
                    FFXPRINTF(1, ("NORRAM:IORequest() Bad Read Generic SubFunction %x\n", pReq->ioSubFunc));
                    ioStat.ffxStat = FFXSTAT_FIM_UNSUPPORTEDIOREQUEST;
                    DclError();
                    break;
                }
            }

            break;
        }

        case FXIOFUNC_FIM_WRITE_GENERIC:
        {
            FFXIOR_FIM_WRITE_GENERIC *pReq = (FFXIOR_FIM_WRITE_GENERIC*)pIOR;

            DclAssert(pReq->ior.ulReqLen == sizeof *pReq);
            DclAssert(pReq->ulCount);
            DclAssert(hFim->FimInfo.uPageSize);

            switch(pReq->ioSubFunc)
            {
                case FXIOSUBFUNC_FIM_WRITE_PAGES:
                {
                    DclAssert(pReq->pPageData);

                    if(!Write(hFim,
                        pReq->ulStartPage * hFim->FimInfo.uPageSize,
                        pReq->ulCount * hFim->FimInfo.uPageSize,
                        pReq->pPageData))
                    {
                        ioStat.ffxStat = FFXSTAT_FIMIOERROR;
                    }
                    else
                    {
                        ioStat.ffxStat = FFXSTAT_SUCCESS;
                        ioStat.ulCount = pReq->ulCount;
                    }

                    break;
                }

                default:
                {
                    FFXPRINTF(1, ("NORRAM:IORequest() Bad Write Generic SubFunction %x\n", pReq->ioSubFunc));
                    ioStat.ffxStat = FFXSTAT_FIM_UNSUPPORTEDIOREQUEST;
                    DclError();
                    break;
                }
            }

            break;
        }

        /*  These are NOR/Sibley specific cases, which are plainly valid for
            the NORRAM FIM, however since this FIM builds even when NOR/Sibley
            support is disabled, we can't compile this code since these are
            only defined when NOR/Sibley is enabled.
        */
        case FXIOFUNC_FIM_READ_CONTROLDATA:
        {
            FFXIOR_FIM_READ_CONTROLDATA *pReq = (FFXIOR_FIM_READ_CONTROLDATA*)pIOR;

            DclAssert(pReq->ior.ulReqLen == sizeof *pReq);
            DclAssert(pReq->ulLength);
            DclAssert(pReq->pData);

            ioStat.ulFlags = IOFLAGS_BYTE;

            if(!Read(hFim, pReq->ulOffset, pReq->ulLength, pReq->pData))
            {
                ioStat.ffxStat = FFXSTAT_FIMIOERROR;
            }
            else
            {
                ioStat.ffxStat = FFXSTAT_SUCCESS;
                ioStat.ulCount = pReq->ulLength;
            }

            break;
        }

        case FXIOFUNC_FIM_WRITE_CONTROLDATA:
        {
            FFXIOR_FIM_WRITE_CONTROLDATA *pReq = (FFXIOR_FIM_WRITE_CONTROLDATA*)pIOR;

            DclAssert(pReq->ior.ulReqLen == sizeof *pReq);
            DclAssert(pReq->ulLength);
            DclAssert(pReq->pData);

            ioStat.ulFlags = IOFLAGS_BYTE;

            if(!Write(hFim, pReq->ulOffset, pReq->ulLength, pReq->pData))
            {
                ioStat.ffxStat = FFXSTAT_FIMIOERROR;
            }
            else
            {
                ioStat.ffxStat = FFXSTAT_SUCCESS;
                ioStat.ulCount = pReq->ulLength;
            }

            break;
        }

        default:
        {
            FFXPRINTF(1, ("NORRAM:IORequest() Bad Function %x\n", pIOR->ioFunc));
            ioStat.ffxStat = FFXSTAT_FIM_UNSUPPORTEDIOREQUEST;
            DclError();
            break;
        }
    }

    DclProfilerLeave(0UL);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_FML, 2, TRACEUNDENT),
        "NORRAM:IORequest() returning %s\n", FfxDecodeIOStatus(&ioStat)));

    return ioStat;
}


/*-------------------------------------------------------------------
    Public: Read()

    Performs transfers data from the RAM into the client
    buffer.  This is performed by a simple call to memmove.

    Parameters:
        hFim     - The FIM instance handle
        ulStart  - Starting offset in bytes to begin the access
        ulLength - Number of bytes to transfer
        pBuffer  - Pointer to client supplied transfer area

    Return Value:
        Returns TRUE if successful else FALSE.
-------------------------------------------------------------------*/
static D_BOOL Read(
    FIMHANDLE      hFim,
    D_UINT32       ulStart,
    D_UINT32       ulLength,
    void          *pBuffer)
{
    D_BUFFER      *pBuff = pBuffer;

    /*  Some error checking to ensure we don't corrupt memory.  We use
        production asserts here since this code is only used for testing
        purposes, and we want any issues to have as high a visibility
        as possible.
    */
    if(!hFim)
    {
        DclProductionError();
        return FALSE;
    }

    if(!pBuffer)
    {
        DclProductionError();
        return FALSE;
    }

    if(ulStart >= hFim->FimInfo.ulTotalBlocks * hFim->FimInfo.ulBlockSize)
    {
        DclProductionError();
        return FALSE;
    }

    if((ulStart + ulLength) > hFim->FimInfo.ulTotalBlocks * hFim->FimInfo.ulBlockSize)
    {
        DclProductionError();
        return FALSE;
    }

  #if EMUL_TIMING
    /*  Emulate real flash characteristics by delaying a certain
        amount for each page's worth of data we read.
    */
    if(ulLength >= hFim->FimInfo.uPageSize)
        _sysdelay((ulLength / hFim->FimInfo.uPageSize) * EMUL_DELAY_READ_US);
    else
        _sysdelay(EMUL_DELAY_READ_US / (hFim->FimInfo.uPageSize / ulLength));
  #endif

    while(ulLength)
    {
        D_UINT32 ulThisLength = DCLMIN(ulLength, hFim->FimInfo.ulBlockSize);

        /*  Really simple, just copy the memory
        */
        DclMemCpy(pBuff, &hFim->pMemoryBase[(size_t) ulStart], ulThisLength);

        pBuff       += ulThisLength;
        ulStart     += ulThisLength;
        ulLength    -= ulThisLength;
    }

    return TRUE;
}


/*-------------------------------------------------------------------
    Public: Write()

    Parameters:
        hFim     - The FIM instance handle
        ulStart  - Starting offset in bytes to begin the access
        ulLength - Number of bytes to transfer
        pBuffer  - Pointer to client supplied transfer area

    Return Value:
        Returns TRUE if successful else FALSE.
-------------------------------------------------------------------*/
static D_BOOL Write(
    FIMHANDLE       hFim,
    D_UINT32       ulStart,
    D_UINT32       ulLength,
    const void    *pBuffer)
{
    D_UINT32       ii;

    /*  Some error checking to ensure we don't corrupt memory.  We use
        production asserts here since this code is only used for testing
        purposes, and we want any issues to have as high a visibility
        as possible.
    */
    if(!hFim)
    {
        DclProductionError();
        return FALSE;
    }

    if(!pBuffer)
    {
        DclProductionError();
        return FALSE;
    }

    if(ulStart >= hFim->FimInfo.ulTotalBlocks * hFim->FimInfo.ulBlockSize)
    {
        DclProductionError();
        return FALSE;
    }

    if((ulStart + ulLength) > hFim->FimInfo.ulTotalBlocks * hFim->FimInfo.ulBlockSize)
    {
        DclProductionError();
        return FALSE;
    }

  #if EMUL_TIMING
    /*  Emulate real flash characteristics by delaying a certain
        amount for each page's worth of data we write.
    */
    if(ulLength >= hFim->FimInfo.uPageSize)
        _sysdelay((ulLength / hFim->FimInfo.uPageSize) * EMUL_DELAY_WRITE_US);
    else
        _sysdelay(EMUL_DELAY_WRITE_US / (hFim->FimInfo.uPageSize / ulLength));
  #endif

    /*  Flash only writes from ones to zeros so just perform
        an AND operation between the source and destination --
        will only program bits set to zero.
    */
    for(ii = 0; ii < ulLength; ii++)
    {
        hFim->pMemoryBase[(size_t) ulStart + ii] &= *((D_BUFFER*)pBuffer + ii);
    }

    return TRUE;
}


#if EMUL_BOOT_BLOCKS
/*-------------------------------------------------------------------
    Public: EraseBootBlockStart()

    Attempts to initiate an erase operation.  If it is started
    successfully, the only FIM functions that can then be called
    are EraseSuspend() and ErasePoll().  The operation must
    subsequently be monitored by calls to ErasePoll().

    If it is not started successfully, those functions may not
    be called.  The flash is restored to a readable state if
    possible, but this cannot always be guaranteed.

    Parameters:
        hFim     - The FIM instance handle
        ulStart  - Starting offset in bytes to begin the erase.  This
                   must be on a physical erase zone boundary.
        ulLength - Number of bytes to erase.  This must be the exact
                   total length of one or more physical erase zones
                   starting at ulStart.

    Return Value:
        TRUE  - If the erase was started successfully
        FALSE - If the erase failed
-------------------------------------------------------------------*/
static D_BOOL EraseBootBlockStart(
    FIMHANDLE       hFim,
    D_UINT32        ulStartBB,
    D_UINT32        ulCount)
{
    D_UINT32        i;
    D_UINT32        ulChipBlock;
    D_UINT32        ulLength;

    DclAssert(hFim);
    DclAssert(ulCount);

    /*  Some error checking to ensure we don't corrupt memory.  We use
        production asserts here since this code is only used for testing
        purposes, and we want any issues to have as high a visibility
        as possible.
    */
    if(!hFim)
    {
        DclProductionError();
        return FALSE;
    }

    /*  Figure out which full sized block within the chip that this
        request is falling in.
    */
    ulChipBlock = (ulStartBB / EMUL_BOOTBLOCK_DIVISOR) % hFim->FimInfo.ulChipBlocks;

    if(ulChipBlock >= EMUL_BOOTBLOCKS_LOW &&
        ulChipBlock < hFim->FimInfo.ulChipBlocks - EMUL_BOOTBLOCKS_HIGH)
    {
        DclProductionError();
        return FALSE;
    }

    /*  A single request cannot span a full erase block boundary
    */
    if(ulCount > EMUL_BOOTBLOCK_DIVISOR - (ulStartBB % EMUL_BOOTBLOCK_DIVISOR))
    {
        DclProductionError();
        return FALSE;
    }

    DclProductionAssert(!hFim->fEraseSuspended);

    hFim->fInBootBlocks = TRUE;

    /*  Set the timeout for the operation now that it's started.
    */
    DclTimerSet(&hFim->tErase, ERASE_TIMEOUT);

    /*  Record the erase address for ErasePoll(), EraseSuspend, and
        EraseResume() to use.
    */
    hFim->ulEraseStart = ulStartBB * (hFim->FimInfo.ulBlockSize / EMUL_BOOTBLOCK_DIVISOR);

    /*  Clear the erase result to signify erase in progress.
    */
    hFim->ulEraseResult = ERASE_IN_PROGRESS;

    /*  Never erase more than a single zone.
    */
    ulLength = hFim->FimInfo.ulBlockSize / EMUL_BOOTBLOCK_DIVISOR;

  #if EMUL_TIMING
    /*  Record the current time and the total number of milliseconds it
        should take to erase this block.
    */
    hFim->tsErase = DclTimeStamp();
    hFim->ulEraseMS = EMUL_SLEEP_ERASE_US / EMUL_BOOTBLOCK_DIVISOR / 1000;
  #endif

    /*  Set all bytes in this block to FF.  We're erasing the block now,
        however, if EMUL_TIMING is enabled, we will not recognize it as
        being erased until the full amount of time has elapsed.
    */
    for(i = 0; i < ulLength; ++i)
    {
        hFim->pMemoryBase[(size_t) hFim->ulEraseStart + (size_t) i] = (D_UCHAR)0xFF;
    }

    return TRUE;
}
#endif


/*-------------------------------------------------------------------
    Pubic: EraseStart()

    Attempts to initiate an erase operation.  If it is started
    successfully, the only FIM functions that can then be called
    are EraseSuspend() and ErasePoll().  The operation must
    subsequently be monitored by calls to ErasePoll().

    If it is not started successfully, those functions may not
    be called.  The flash is restored to a readable state if
    possible, but this cannot always be guaranteed.

    Parameters:
        hFim     - The FIM instance handle
        ulStart  - Starting offset in bytes to begin the erase.  This
                   must be on a physical erase zone boundary.
        ulLength - Number of bytes to erase.  This must be the exact
                   total length of one or more physical erase zones
                   starting at ulStart.

    Return Value:
        TRUE  - If the erase was started successfully
        FALSE - If the erase failed
-------------------------------------------------------------------*/
static D_BOOL EraseStart(
    FIMHANDLE       hFim,
    D_UINT32        ulStart,
    D_UINT32        ulLength)
{
    D_UINT32        i;

    /*  Some error checking to ensure we don't corrupt memory.  We use
        production asserts here since this code is only used for testing
        purposes, and we want any issues to have as high a visibility
        as possible.
    */
    if(!hFim)
    {
        DclProductionError();
        return FALSE;
    }

    if(ulStart >= hFim->FimInfo.ulTotalBlocks * hFim->FimInfo.ulBlockSize)
    {
        DclProductionError();
        return FALSE;
    }

    if((ulStart + ulLength) > hFim->FimInfo.ulTotalBlocks * hFim->FimInfo.ulBlockSize)
    {
        DclProductionError();
        return FALSE;
    }

    if(ulStart % hFim->FimInfo.ulBlockSize)
    {
        DclProductionError();
        return FALSE;
    }

    if(ulLength % hFim->FimInfo.ulBlockSize)
    {
        DclProductionError();
        return FALSE;
    }

    DclProductionAssert(!hFim->fEraseSuspended);

  #if EMUL_BOOT_BLOCKS
    hFim->fInBootBlocks = FALSE;
  #endif

    /*  Set the timeout for the operation now that it's started.
    */
    DclTimerSet(&hFim->tErase, ERASE_TIMEOUT);

    /*  Record the erase address for ErasePoll(), EraseSuspend, and
        EraseResume() to use.
    */
    hFim->ulEraseStart = ulStart;

    /*  Clear the erase result to signify erase in progress.
    */
    hFim->ulEraseResult = ERASE_IN_PROGRESS;

    /*  Never erase more than a single zone.
    */
    ulLength = hFim->FimInfo.ulBlockSize;

  #if EMUL_TIMING
    /*  Record the current time and the total number of milliseconds it
        should take to erase this block.
    */
    hFim->tsErase = DclTimeStamp();
    hFim->ulEraseMS = EMUL_SLEEP_ERASE_US / 1000;
  #endif

    /*  Set all bytes in this block to FF.  We're erasing the block now,
        however, if EMUL_TIMING is enabled, we will not recognize it as
        being erased until the full amount of time has elapsed.
    */
    for(i = 0; i < ulLength; ++i)
    {
        hFim->pMemoryBase[(size_t) ulStart + (size_t) i] = (D_UCHAR)0xFF;
    }

    return TRUE;
}


#if FFXCONF_ERASESUSPENDSUPPORT
/*-------------------------------------------------------------------
    Public: EraseSuspend()

    Suspend an erase operation currently in progress, and return
    the flash to normal read mode.  When this function returns,
    the flash may be read.

    If the flash does not support suspending erases, this
    function is not implemented, and the EraseSuspend entry in
    the FIMDEVICE structure must be NULL.

    Parameters:
        hFim - The FIM instance handle

    Return Value:
        Returns TRUE if successful, else FALSE.
-------------------------------------------------------------------*/
static D_BOOL EraseSuspend(
    FIMHANDLE       hFim)
{
    D_BOOL          fResult = TRUE;

  #if EMUL_TIMING
    {
        D_UINT32 ulElapsed = DclTimePassed(hFim->tsErase);

        /*  Take the amount of erase time which has elapsed so far, and
            calculate the remaining amount of time to erase, once we
            have resumed erasing.
        */
        if(hFim->ulEraseMS >= ulElapsed)
            hFim->ulEraseMS -= ulElapsed;
        else
            hFim->ulEraseMS = 0;
    }
  #endif

    /*  Save the remaining timeout period.
    */
    hFim->ulTimeoutRemaining = DclTimerRemaining(&hFim->tErase);

    /*  The flash is expected to be in Read Status mode.
    */

    /*  Issue the erase-suspend command here */

    /*  Wait a while for the flash to go into erase suspend.
    */
    DclTimerSet(&hFim->tErase, ERASE_SUSPEND_TIMEOUT);
    while(!DclTimerExpired(&hFim->tErase))
    {
        if(1  /* command has completed */ )
            break;
    }

    /*  Check the status after a possible timeout.  A higher priority
        thread could have preempted between setting the timer or
        checking the status in the loop and checking for expiration.
    */
    if(0 /* command has not completed */)
    {
        /*  It really timed out.  This is a Bad Thing.  Record the failure.
        */
        hFim->ulEraseResult = FIMMT_ERASE_FAILED;
        fResult = FALSE;
    }

    /*  Return the flash to Read Array mode whether or not the suspend
        command appeared to have worked (it can't hurt).
    */

    /*  Issue the "return-to-read-mode" command here */

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
        hFim     - The FIM instance handle

    Return Value:
        None
-------------------------------------------------------------------*/
static void EraseResume(
    FIMHANDLE       hFim)
{
    DclAssert(hFim);

    /*  If an error occurred in EraseSuspend() the final result of the
        erase was already recorded.  Only operate on the flash if this
        hasn't happened yet.
    */
    if(hFim->ulEraseResult == ERASE_IN_PROGRESS)
    {
#if 0
        /*  Erases are suspended to perform other operations, so it's
            necessary to remap the window now.
        */
        pMedia = FfxHookWindowMap(hFim->hDev, hFim->ulEraseStart);
        pFimExtra->pMedia = pMedia;

        /*  Clear status from another operation, and put the flash in Read
            Status mode.
        */
        *pMedia = INTLCMD_CLEAR_STATUS;
        *pMedia = INTLCMD_READ_STATUS;

        /*  It's possible that one or both of the chips finished its
            erase before EraseSuspend was called.  Don't try to resume a
            chip that is not suspended: experiment shows that it will
            (sometimes?) return to Read Array mode.

            If one of the chips is showing error status, there's no point
            in trying to resume.
        */
        stat = *pMedia & INTLSTAT_STATUS_MASK;

        if(stat != INTLSTAT_DONE)
        {
            /*  Change any OK status codes to READSTATUS commands, and
                any ERASESUSPENDED status codes to ERASERESUME commands.
            */
            TRANSFORMSTATUS(&stat, ISTAT_OK, ICMD_READSTAT);
            TRANSFORMSTATUS(&stat, ISTAT_ERASESUSPENDED, ICMD_ERASERESUME);

            if(ISCOMPLETETRANSFORMATION(stat, ICMD_READSTAT, ICMD_ERASERESUME))
            {
                /*  If everything was changed to one code or the other,
                    we're good, so issue the command.
                */
                *pMedia = stat;
            }
            else
            {
                /*  Presumably errored out in some fashion.  ErasePoll() will
                    report that the erase failed.
                */
                DclError();
            }
        }
#endif
        /*  Need to find a cleaner way to do this.
            Guarantee a minimum erase increment.
        */
        _sysdelay(MINIMUM_ERASE_INCREMENT);

        /*  Restart the timer.  Note that this will be done in the (unlikely)
            case that both chips had already finished when they were suspended.
            This is harmless, as ErasePoll() checks their status before checking
            for timeout.
        */
        DclTimerSet(&hFim->tErase, hFim->ulTimeoutRemaining);

      #if EMUL_TIMING
        /*  We are now restarting the erase, and have already calculated
            the remaining time when we did the suspend.   Now just reset
            the start time to NOW.
        */
        hFim->tsErase = DclTimeStamp();
      #endif
    }

    return;
}
#endif


/*-------------------------------------------------------------------
    Public: ErasePoll()

    Monitor the status of an erase begun with EraseStart().

    If the erase fails, attempts to return the flash to its
    normal read mode.  Depending on the type of flash, this may
    or may not be possible.  If it is possible, it may be
    achieved by suspending the erase operation rather than by
    terminating it.  In this case, it may be possible to read
    the flash, but not to erase it further.

    This function may be called with the flash either in read
    array mode or in read status mode.

    Parameters:
        hFim - The FIM instance handle

    Return Value:
        If the erase is still in progress, returns 0.  The only FIM
        functions that can then be called are EraseSuspend() and
        ErasePoll().

        If the erase completed successfully, returns the length of
        the erase zone actually erased.  This may be less than the
        ulLength value supplied to EraseStart().  The flash is in
        normal read mode.

        If the erase failed, returns ERASE_FAILED, which is a
        value that could never be a valid erase length.  The flash
        is returned to normal read mode if possible, but this may not
        be possible in all cases (for example, if the flash does not
        support suspending an erase, and the operation times out).
-------------------------------------------------------------------*/
static D_UINT32 ErasePoll(
    FIMHANDLE    hFim)
{
    DclAssert(hFim);

    DclProductionAssert(!hFim->fEraseSuspended);

    /*  ErasePoll() may be called multiple times even after the
        operation has completed.  Only check the flash if it
        has not already been seen to have finished its operation.
    */
    if(hFim->ulEraseResult == ERASE_IN_PROGRESS)
    {
      #if EMUL_TIMING
        if(DclTimePassed(hFim->tsErase) < hFim->ulEraseMS)
            return 0;
      #endif

        /*  We never erase more than one zone, and it will never fail for
            this FIM, so return one erase zone size.
        */
      #if EMUL_BOOT_BLOCKS
        if(hFim->fInBootBlocks)
            hFim->ulEraseResult = hFim->FimInfo.ulBlockSize / EMUL_BOOTBLOCK_DIVISOR;
        else
      #endif
            hFim->ulEraseResult = hFim->FimInfo.ulBlockSize;
    }

    return hFim->ulEraseResult;
}


/*-------------------------------------------------------------------
    FIMDEVICE Declaration

    This structure declaration is used to define the entry points
    into the FIM.  This is declared at the end of the module to
    eliminate the need for what would be duplicated function
    prototypes in all the FIMs.
-------------------------------------------------------------------*/
FIMDEVICE FFXFIM_norram =
{
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    Create,
    Destroy,
    IORequest,
    ParameterGet,
    ParameterSet
};




#endif  /* FFXCONF_NORSUPPORT */

