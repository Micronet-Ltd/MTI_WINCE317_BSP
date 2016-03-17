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

    This module contains the FlashFX Hooks Layer implementations for the
    Micron NTM, as used on the TI SDP3430.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: fhmicron.c $
    Revision 1.6  2010/08/04 00:47:09Z  garyp
    Updated to use the GPMC register offset defined in hwomap35x.h.
    Revision 1.5  2010/07/16 15:29:10Z  garyp
    Updated so the hooks module provides the maximum number of chips
    count to the NTM.
    Revision 1.4  2010/07/05 20:06:03Z  garyp
    Added the function FfxHookNTMicronConfigure().  Refactored to work
    in multichip configurations.
    Revision 1.3  2010/06/25 20:06:35Z  glenns
    Add support for Micron M60 boot blocks
    Revision 1.2  2010/01/10 22:36:19Z  garyp
    Updated to specify the alignment requirement.  Removed some dead code.
    Revision 1.1  2009/12/14 03:36:18Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <windows.h>
#include <diskio.h>
#include <oal_memory.h>

#include <flashfx.h>
#include <nandconf.h>
#include <nandctl.h>
#include <hardware/hwomap35x.h>
#include <dlcpuapi.h> 
#include <fhmicron.h>
#include <fhomap3xx.h>
#include <ecc.h>
#include "project.h" 

/*  GPMC_CONFIG bit 4 WRITEPROTECT????
*/
#define NAND_WRITEPROTECT	(0x00000010)


#define OMAP35X_GPMC_FIFOADDRESS0   (0x08000000)
#define OMAP35X_GPMC_FIFOADDRESS1   (0x10000000)    /*  What is this really? */

#define REG32SET(pul, b)            ((pul) |=  (b))
#define REG32CLEAR(pul, b)          ((pul) &= ~(b))

#if USE_PREFETCH_MODE  
#define OMAP35X_GPMC_NAND_FIFOADDRESS(chip) ((chip == 0) ? OMAP35X_GPMC_FIFOADDRESS0 : OMAP35X_GPMC_FIFOADDRESS1)
        
static void IOMemCopy(void *pDest, const void *pSource, size_t nSize);
static void PrefetchSetup(NTMHOOKHANDLE hNtmHook, D_BOOL fWriting, size_t nCount);
#endif 


/*-------------------------------------------------------------------
    Public: FfxHookNTMicronCreate()

    Create a NTM Hook instance which is associated with the Micron NTM.

    Parameters:
        hDev    - The Device handle
        pParams - A pointer to the FFXMICRONPARAMS structure to use.

    Return Value:
        Returns an NTMHOOKHANDLE if successful, or NULL otherwise.
 -------------------------------------------------------------------*/
NTMHOOKHANDLE FfxHookNTMicronCreate(
    FFXDEVHANDLE        hDev,
    FFXMICRONPARAMS    *pParams)
{
    D_UINT32            ulConfigVal;
    NTMHOOKHANDLE       hNtmHook;
	static D_BOOL       fInitialized = FALSE;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTMHOOK, 1, 0), "FfxHookNTMicronCreate()\n"));

    DclAssert(hDev);
    DclAssertWritePtr(pParams, sizeof(*pParams));
    DclAssert(pParams->nStructLen == sizeof(*pParams));

    hNtmHook = DclMemAllocZero(sizeof(*hNtmHook));
    if(!hNtmHook)
    {
        FFXPRINTF(1, ("FfxHookNTMicronCreate() Unable to allocate memory!\n"));
        return NULL;
    }

    /*  Initialize the current chip select to an illegal value
    */
    hNtmHook->nChip = UINT_MAX;
 
    /*  NOTE: Once DclOsVirtualPointerMap() is implemented properly for
              WinceBL, this code should be updated to use that function.
    */
    if(DclCpuIsMmuOn())
    {
        FFXPRINTF(1, ("MMU is On\n"));
        hNtmHook->pulGPMCBase = OALPAtoVA(OMAP35X_GPMC_PHYSICAL_OFFSET, FALSE);
    }
    else
    {
        FFXPRINTF(1, ("MMU is Off\n"));
        hNtmHook->pulGPMCBase = (volatile D_UINT32*)OMAP35X_GPMC_PHYSICAL_OFFSET;
    }

	if(!fInitialized)
    {
        fInitialized = TRUE;

        /*  Disable write protect
        */
        ulConfigVal = OMAP35X_GPMC_CONFIG(hNtmHook->pulGPMCBase);
        ulConfigVal |= NAND_WRITEPROTECT;           
        OMAP35X_GPMC_CONFIG(hNtmHook->pulGPMCBase) = ulConfigVal;

        /*  Disable HW ECC for the moment
        */
        ulConfigVal = OMAP35X_GPMC_ECC_CONFIG(hNtmHook->pulGPMCBase);
        ulConfigVal &= ~OMAP35X_GPMC_ECC_CONFIG_ENABLE;
        OMAP35X_GPMC_ECC_CONFIG(hNtmHook->pulGPMCBase) = ulConfigVal;

      #if USE_HARDWARE_ECC  
        /*
     	    ECCSIZE1 : 512 (For main page area (2k), 
      	    ECCSIZE0 : 64 ?? for spare area) ??
    	    and selects ECCSIZE1 for result register	     
        */
	    ulConfigVal = (0xFF << 22) | (0xFF << 12) | OMAP35X_GPMC_ECC_SIZE_RESULT1_SIZE1;
        OMAP35X_GPMC_ECC_SIZE(hNtmHook->pulGPMCBase) = ulConfigVal;

    	FFXPRINTF(1, ("FfxHookNTMicronCreate() Hardware ECC Enabled\n"));
      #endif

        /*  Disable prefetch for the moment
        */
        OMAP35X_GPMC_PREFETCH_CONFIG1(hNtmHook->pulGPMCBase) = 0;
        OMAP35X_GPMC_PREFETCH_CONFIG2(hNtmHook->pulGPMCBase) = 0;
        OMAP35X_GPMC_PREFETCH_CONTROL(hNtmHook->pulGPMCBase) = 0;

        /*  Display a number of configuration register values
        */
        FFXPRINTF(1, ("    GPMC         GPMC_REVISION: %lX\n", OMAP35X_GPMC_REVISION(hNtmHook->pulGPMCBase)));
        FFXPRINTF(1, ("Configuration   GPMC_SYSCONFIG: %lX\n", OMAP35X_GPMC_SYSCONFIG(hNtmHook->pulGPMCBase))); 
        FFXPRINTF(1, ("  Registers     GPMC_SYSSTATUS: %lX\n", OMAP35X_GPMC_SYSSTATUS(hNtmHook->pulGPMCBase)));
        FFXPRINTF(1, ("                GPMC_IRQSTATUS: %lX\n", OMAP35X_GPMC_IRQSTATUS(hNtmHook->pulGPMCBase)));
        FFXPRINTF(1, ("                GPMC_IRQENABLE: %lX\n", OMAP35X_GPMC_IRQENABLE(hNtmHook->pulGPMCBase)));
        FFXPRINTF(1, ("          GPMC_TIMEOUT_CONTROL: %lX\n", OMAP35X_GPMC_TIMEOUT_CONTROL(hNtmHook->pulGPMCBase)));
        FFXPRINTF(1, ("              GPMC_ERR_ADDRESS: %lX\n", OMAP35X_GPMC_ERR_ADDRESS(hNtmHook->pulGPMCBase)));
        FFXPRINTF(1, ("                 GPMC_ERR_TYPE: %lX\n", OMAP35X_GPMC_ERR_TYPE(hNtmHook->pulGPMCBase)));
        FFXPRINTF(1, ("                   GPMC_CONFIG: %lX\n", OMAP35X_GPMC_CONFIG(hNtmHook->pulGPMCBase)));
        FFXPRINTF(1, ("                   GPMC_STATUS: %lX\n", OMAP35X_GPMC_STATUS(hNtmHook->pulGPMCBase)));
        FFXPRINTF(1, ("               GPMC_ECC_CONFIG: %lX\n", OMAP35X_GPMC_ECC_CONFIG(hNtmHook->pulGPMCBase)));
        FFXPRINTF(1, ("              GPMC_ECC_CONTROL: %lX\n", OMAP35X_GPMC_ECC_CONTROL(hNtmHook->pulGPMCBase)));
        FFXPRINTF(1, ("                 GPMC_ECC_SIZE: %lX\n", OMAP35X_GPMC_ECC_SIZE(hNtmHook->pulGPMCBase)));
        FFXPRINTF(1, ("         GPMC_PREFETCH_CONTROL: %lX\n", OMAP35X_GPMC_PREFETCH_CONTROL(hNtmHook->pulGPMCBase)));
        FFXPRINTF(1, ("         GPMC_PREFETCH_CONFIG1: %lX\n", OMAP35X_GPMC_PREFETCH_CONFIG1(hNtmHook->pulGPMCBase)));
        FFXPRINTF(1, ("         GPMC_PREFETCH_CONFIG2: %lX\n", OMAP35X_GPMC_PREFETCH_CONFIG2(hNtmHook->pulGPMCBase)));
        FFXPRINTF(1, ("          GPMC_PREFETCH_STATUS: %lX\n", OMAP35X_GPMC_PREFETCH_STATUS(hNtmHook->pulGPMCBase)));
    }

    pParams->nMaxChips                  = MAX_CHIPS;
    pParams->nTagOffset                 = TAG_OFFSET;
    pParams->fSupportCacheModeReads     = SUPPORT_CACHE_MODE_READS;
    pParams->fSupportCacheModeWrites    = SUPPORT_CACHE_MODE_WRITES;
    pParams->fSupportDualPlanes         = SUPPORT_DUAL_PLANE_OPS;
    pParams->nAlignment                 = sizeof(NAND_IO_TYPE);
    pParams->nM60EDC4Chip               = M60_EDC4_FIRST_CHIP;
    pParams->ulM60EDC4Block             = M60_EDC4_FIRST_BLOCK;

  #if USE_PREFETCH_MODE  
    /*  When using prefetch mode, buffers must be aligned
        on 32-bit boundaries.
    */
    pParams->nAlignment = 4;
  #endif

    return hNtmHook;
}


/*-------------------------------------------------------------------
    Public: FfxHookNTMicronDestroy()

    Destroy a Micron NTM Hook instance.

    Parameters:
        hNtmHook - The hook handle returned from the Create() function.

    Return Value:
        An FFXSTATUS value indicating success or the nature of
        any failure.
-------------------------------------------------------------------*/
FFXSTATUS FfxHookNTMicronDestroy(
    NTMHOOKHANDLE hNtmHook)
{
    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTMHOOK, 1, 0),
        "FfxHookNTMicronDestroy() hNtmHook=%P\n", hNtmHook));

    DclAssertReadPtr(hNtmHook, sizeof(hNtmHook));
  
    return DclMemFree(hNtmHook);
}


/*-------------------------------------------------------------------
    Public: FfxHookNTMicronConfigure()

    Configure an NTM Hook instance for the given chip.  If the NTM
    instance supports multiple chips, this function will be called 
    for each chip.

    Parameters:
        hNtmHook  - The hook handle returned from the Create() function.
        nChip     - The chip being configured.
        pChipInfo - A pointer to the FFXNANDCHIP structure to use
        pabID     - A pointer to an array of NAND ID bytes, which will
                    contain at least NAND_ID_SIZE entries.

    Return Value:
        Returns an FFXSTATUS code indicating the operation result.
-------------------------------------------------------------------*/
FFXSTATUS FfxHookNTMicronConfigure(
    NTMHOOKHANDLE       hNtmHook,
    unsigned            nChip,
    const FFXNANDCHIP  *pChipInfo,
    D_BYTE             *pabID)
{
    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTMHOOK, 1, 0),
        "FfxHookNTMicronConfigure() hNtmHook=%P Chip=%u pChipInfo=%P pabID=%P\n", 
        hNtmHook, nChip, pChipInfo, pabID));

    DclAssertWritePtr(hNtmHook, sizeof(hNtmHook));
    DclAssertReadPtr(pChipInfo, sizeof(*pChipInfo));
    DclAssertWritePtr(pabID, NAND_ID_SIZE);
    DclAssert(nChip < MAX_CHIPS);
    DclAssert(!DCLBITGET(&hNtmHook->nValidChipMask, nChip));

    DCLBITSET(&hNtmHook->nValidChipMask, nChip);

  #if USE_PREFETCH_MODE  
    if(DclCpuIsMmuOn())
        hNtmHook->apFIFO[nChip] = OALPAtoVA(OMAP35X_GPMC_NAND_FIFOADDRESS(nChip), FALSE);
    else
        hNtmHook->apFIFO[nChip] = (volatile D_UINT32*)OMAP35X_GPMC_NAND_FIFOADDRESS(nChip);
 
	FFXPRINTF(1, ("FfxHookNTMicronConfigure: Prefetch enabled for CS%u, FIFO mapped address %lX to %lX\n", 
		nChip, OMAP35X_GPMC_NAND_FIFOADDRESS(nChip), hNtmHook->apFIFO[nChip]));
  #endif

  #if D_DEBUG
    FFXPRINTF(1, ("GPMC CS%u Configuration Registers:\n", nChip));  
    if(nChip == 0)
    {
        FFXPRINTF(1, ("              GPMC_CS0_CONFIG1: %lX\n", OMAP35X_GPMC_CS0_CONFIG1(hNtmHook->pulGPMCBase)));
        FFXPRINTF(1, ("              GPMC_CS0_CONFIG2: %lX\n", OMAP35X_GPMC_CS0_CONFIG2(hNtmHook->pulGPMCBase)));
        FFXPRINTF(1, ("              GPMC_CS0_CONFIG3: %lX\n", OMAP35X_GPMC_CS0_CONFIG3(hNtmHook->pulGPMCBase)));
        FFXPRINTF(1, ("              GPMC_CS0_CONFIG4: %lX\n", OMAP35X_GPMC_CS0_CONFIG4(hNtmHook->pulGPMCBase)));
        FFXPRINTF(1, ("              GPMC_CS0_CONFIG5: %lX\n", OMAP35X_GPMC_CS0_CONFIG5(hNtmHook->pulGPMCBase)));
        FFXPRINTF(1, ("              GPMC_CS0_CONFIG6: %lX\n", OMAP35X_GPMC_CS0_CONFIG6(hNtmHook->pulGPMCBase)));
        FFXPRINTF(1, ("              GPMC_CS0_CONFIG7: %lX\n", OMAP35X_GPMC_CS0_CONFIG7(hNtmHook->pulGPMCBase)));
    }
    else
    {
        FFXPRINTF(1, ("              GPMC_CS1_CONFIG1: %lX\n", OMAP35X_GPMC_CS1_CONFIG1(hNtmHook->pulGPMCBase)));
        FFXPRINTF(1, ("              GPMC_CS1_CONFIG2: %lX\n", OMAP35X_GPMC_CS1_CONFIG2(hNtmHook->pulGPMCBase)));
        FFXPRINTF(1, ("              GPMC_CS1_CONFIG3: %lX\n", OMAP35X_GPMC_CS1_CONFIG3(hNtmHook->pulGPMCBase)));
        FFXPRINTF(1, ("              GPMC_CS1_CONFIG4: %lX\n", OMAP35X_GPMC_CS1_CONFIG4(hNtmHook->pulGPMCBase)));
        FFXPRINTF(1, ("              GPMC_CS1_CONFIG5: %lX\n", OMAP35X_GPMC_CS1_CONFIG5(hNtmHook->pulGPMCBase)));
        FFXPRINTF(1, ("              GPMC_CS1_CONFIG6: %lX\n", OMAP35X_GPMC_CS1_CONFIG6(hNtmHook->pulGPMCBase)));
        FFXPRINTF(1, ("              GPMC_CS1_CONFIG7: %lX\n", OMAP35X_GPMC_CS1_CONFIG7(hNtmHook->pulGPMCBase)));
    }
  #endif

    return FFXSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    Public: FfxHookNTMicronSetChipSelect()

    Select a particular NAND chip.  Any control bits are set to
    zero and all other chips are deselected.

    Parameters:
        hNtmHook - The NTM Hook handle
        nChip    - The chip number 
        nMode    - The mode to use, either MODE_READ or MODE_WRITE

    Return Value:
        Returns an FFXSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
FFXSTATUS FfxHookNTMicronSetChipSelect(
    NTMHOOKHANDLE       hNtmHook,
    unsigned            nChip,
    CHIP_SELECT_MODE    nMode)
{
    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTMHOOK, 1, 0),
        "FfxHookNTMicronSetChipSelect() hNtmHook=%P OldChip=%u NewChip=%u Mode=%s\n",
         hNtmHook, hNtmHook->nChip, nChip, nMode == MODE_WRITE ? "MODE_WRITE" : "MODE_READ"));

    DclAssert(hNtmHook);
    DclAssert(nChip < MAX_CHIPS);

    /*  Can't assert this because we must set the chip select in
        order to do the Read-ID, and we have to do that before
        the "configure" function is called which initializes the
        mask bits.

    DclAssert(DCLBITGET(&hNtmHook->nValidChipMask, nChip));
    */        

    (void)nMode;

    hNtmHook->nChip = nChip;

    return FFXSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    Public: FfxHookNTMicronReadyWait()

    Poll the BSY/RDY pin of the NAND chip until it indicates that
    the device is ready or a timeout limit is reached.  This is
    often called before a command is written to the flash to be
    sure the device will listen before the command is sent.  The
    chip selects must/will be set prior to this function call.
    This function could look at the busy pins of the device or
    send a read status command.

    The timeout limit is specified by NAND_TIME_OUT in nandconf.h.

    Parameters:
        hNtmHook - The NTM Hook handle

    Return Value:
        Returns an FFXSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
FFXSTATUS FfxHookNTMicronReadyWait(
    NTMHOOKHANDLE       hNtmHook)
{
    DCLTIMER            t;
    D_UINT32            ulStatusVal;
    D_UINT32            ulWaitReadyBit;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTMHOOK, 1, 0),
        "FfxHookNTMicronReadyWait() hNtmHook=%P Chip=%u\n", hNtmHook, hNtmHook->nChip));

    DclAssert(hNtmHook);
    DclAssert(hNtmHook->nChip < MAX_CHIPS);

    /*  Can't assert this because we must set the chip select in
        order to do the Read-ID, and we have to do that before
        the "configure" function is called which initializes the
        mask bits.

    DclAssert(DCLBITGET(&hNtmHook->nValidChipMask, hNtmHook->nChip));
    */        

    ulWaitReadyBit = (hNtmHook->nChip == 0) ? OMAP35X_GPMC_STATUS_WAIT0_READY : OMAP35X_GPMC_STATUS_WAIT1_READY;

    DclTimerSet(&t, NAND_TIME_OUT);
    
    while(!DclTimerExpired(&t))
    {
        ulStatusVal = OMAP35X_GPMC_STATUS(hNtmHook->pulGPMCBase);
        if(ulStatusVal & ulWaitReadyBit)
            return FFXSTAT_SUCCESS;
    }

    /*  Check one more time to make sure we were not interrupted
        and did in fact finish.
    */
    if(OMAP35X_GPMC_STATUS(hNtmHook->pulGPMCBase) & ulWaitReadyBit)
        return FFXSTAT_SUCCESS;
    else
        return FFXSTAT_FIMTIMEOUT;
    }


/*-------------------------------------------------------------------
    Public: FfxHookNTMicronSetChipCommand()

    Wait for the chip to be ready and then send a command using
    FfxHookNTMicronSetChipCommandNoWait().

    Parameters:
        hNtmHook  - The NTM Hook handle
        bCommand  - The command to send

    Return Value:
        Returns an FFXSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
FFXSTATUS FfxHookNTMicronSetChipCommand(
    NTMHOOKHANDLE   hNtmHook,
    D_BYTE          bCommand)
{
    FFXSTATUS       ffxStat;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTMHOOK, 1, 0),
        "FfxHookNTMicronSetChipCommand() hNtmHook=%P Chip=%u Cmd=%02x\n", 
        hNtmHook, hNtmHook->nChip, bCommand));

    DclAssert(hNtmHook);
    DclAssert(hNtmHook->nChip < MAX_CHIPS);

    /*  Can't assert this because we must set the chip select in
        order to do the Read-ID, and we have to do that before
        the "configure" function is called which initializes the
        mask bits.

    DclAssert(DCLBITGET(&hNtmHook->nValidChipMask, hNtmHook->nChip));
    */        

    ffxStat = FfxHookNTMicronReadyWait(hNtmHook);

    if(ffxStat == FFXSTAT_SUCCESS)
        ffxStat = FfxHookNTMicronSetChipCommandNoWait(hNtmHook, bCommand);

    return ffxStat;
}


/*-------------------------------------------------------------------
    Public: FfxHookNTMicronSetChipCommandNoWait()

    Send a command to the chip without waiting for status.

    Universal "send arbitrary command" helper routine used by all
    higher level chip-specific routines. This routine only takes
    care of making sure that the proper interface pins are
    manipulated so that the command is entered into a previously
    selected chip.  The appropriate chip must already be selected
    before calling this routine and subsequent address,
    confirmation and data bytes must be sent to the chips using
    calls to additional helper routines.

    Parameters:
        hNtmHook  - The NTM Hook handle
        bCommand  - The command to send

    Return Value:
        Returns an FFXSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
FFXSTATUS FfxHookNTMicronSetChipCommandNoWait(
    NTMHOOKHANDLE   hNtmHook,
    D_BYTE          bCommand)
{
    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTMHOOK, 1, 0),
        "FfxHookNTMicronSetChipCommandNoWait() hNtmHook=%P Chip=%u Cmd=%02x\n", 
        hNtmHook, hNtmHook->nChip, bCommand));

    DclAssert(hNtmHook);
    DclAssert(hNtmHook->nChip < MAX_CHIPS);

    /*  Can't assert this because we must set the chip select in
        order to do the Read-ID, and we have to do that before
        the "configure" function is called which initializes the
        mask bits.

    DclAssert(DCLBITGET(&hNtmHook->nValidChipMask, hNtmHook->nChip));
    */        

    OMAP3XX_GPMC_NAND_PUT(COMMAND, hNtmHook->nChip, hNtmHook->pulGPMCBase, bCommand);

    return FFXSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    Public: FfxHookNTMicronDataIn()

    Perform a platform-dependent stream input operation from the
    NAND data port and return the result.

    Parameters:
        hNtmHook - The NTM Hook handle
        pBuffer  - A pointer to a buffer to be filled with the data
                   read from the device
        nCount   - Number of NAND_IO_TYPE elements in the buffer
                   to be read from the device

    Return Value:
        None.
-------------------------------------------------------------------*/
void FfxHookNTMicronDataIn(
    NTMHOOKHANDLE   hNtmHook,  
    NAND_IO_TYPE   *pBuffer,
    unsigned        nCount)
{
    unsigned        nBytes = nCount * sizeof(*pBuffer);
    D_UINT32        ulOldIdleMode;
    
    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTMHOOK, 3, 0),
        "FfxHookNTMicronDataIn() hNtmHook=%P Chip=%u pBuff=%P Count=%u\n", 
        hNtmHook, hNtmHook->nChip, pBuffer, nCount));

    DclAssert(hNtmHook);
    DclAssert(pBuffer);
    DclAssert(nCount);
    DclAssert(hNtmHook->nChip < MAX_CHIPS);

    /*  Can't assert this because we must set the chip select in
        order to do the Read-ID, and we have to do that before
        the "configure" function is called which initializes the
        mask bits.

    DclAssert(DCLBITGET(&hNtmHook->nValidChipMask, hNtmHook->nChip));
    */        

    /*  Force the chip to NO IDLE Mode while Reads and Writes are happening.
        Otherwise the GPMC goes to idle mode and some time doesn't respond as
        required.  Problem identified when enabled both Prefetch and Hardware 
        ECC modes together.
    */
    ulOldIdleMode = OMAP35X_GPMC_SYSCONFIG(hNtmHook->pulGPMCBase);
    OMAP35X_GPMC_SYSCONFIG(hNtmHook->pulGPMCBase) = OMAP35X_GPMC_SYSCONFIG_NO_IDLE;

  #if USE_PREFETCH_MODE
    if(nBytes >= OMAP35X_GPMC_PREFETCH_FIFO_BYTES)
    {
        DclAssert(nBytes % OMAP35X_GPMC_PREFETCH_FIFO_BYTES == 0);
        
        PrefetchSetup(hNtmHook, FALSE, nBytes);

        REG32SET(OMAP35X_GPMC_PREFETCH_CONTROL(hNtmHook->pulGPMCBase), OMAP35X_GPMC_PREFETCH_CONTROL_STARTENGINE);

        while(nBytes)
        {
            D_UINT32    ulFifoLev = 0;

            /*  Wait for the FIFO threshold to be reached
            */
            while(ulFifoLev < OMAP35X_GPMC_PREFETCH_FIFO_BYTES)
            {
                ulFifoLev = OMAP35X_GPMC_PREFETCH_STATUS(hNtmHook->pulGPMCBase);
                ulFifoLev &= OMAP35X_GPMC_PREFETCH_STATUS_FIFOPOINTER_MASK;
                ulFifoLev >>= OMAP35X_GPMC_PREFETCH_STATUS_FIFOPOINTER_SHIFT;
            }

            /*  Copy the data
            */
            IOMemCopy(pBuffer, (D_UINT8*)hNtmHook->apFIFO[hNtmHook->nChip], OMAP35X_GPMC_PREFETCH_FIFO_BYTES);
            pBuffer += OMAP35X_GPMC_PREFETCH_FIFO_BYTES / sizeof(*pBuffer);
            nBytes -= OMAP35X_GPMC_PREFETCH_FIFO_BYTES;
        }
    }
  #endif

    while(nBytes)
    {
        *pBuffer++ = OMAP3XX_GPMC_NAND_GET(DATA, hNtmHook->nChip, hNtmHook->pulGPMCBase);

        nBytes -= sizeof(*pBuffer);           
    }

    /*  Restore the old vaues.
    */
    OMAP35X_GPMC_SYSCONFIG(hNtmHook->pulGPMCBase) = ulOldIdleMode;

    return;
}


/*-------------------------------------------------------------------
    Public: FfxHookNTMicronDataOut()

    Perform a platform-dependent stream output operation to the
    NAND data port, storing the provided data.

    Parameters:
        hNtmHook - The NTM Hook handle
        pBuffer  - A pointer to a buffer containing the data to be
                   sent to the device
        nCount   - Number of NAND_IO_TYPE elements in the buffer
                   to be sent to the device

    Return Value:
        None.
-------------------------------------------------------------------*/
void FfxHookNTMicronDataOut(
    NTMHOOKHANDLE       hNtmHook,
    const NAND_IO_TYPE *pBuffer,
    unsigned            nCount)
{
    unsigned            nBytes = nCount * sizeof(*pBuffer);
    D_UINT32            ulOldIdleMode;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTMHOOK, 3, 0),
        "FfxHookNTMicronDataOut() hNtmHook=%P Chip=%u pBuff=%P Count=%u\n", 
        hNtmHook, hNtmHook->nChip, pBuffer, nCount));

    DclAssert(hNtmHook);
    DclAssert(pBuffer);
    DclAssert(nCount);
    DclAssert(hNtmHook->nChip < MAX_CHIPS);
    DclAssert(DCLBITGET(&hNtmHook->nValidChipMask, hNtmHook->nChip));

    /*  Force the chip to NO IDLE Mode while Reads and Writes are happening.
        Otherwise the GPMC goes to idle mode and some time does n't respond as 
        required.  Problem identified when enabled both Prefetch and Hardware 
        ECC modes together.
    */
    ulOldIdleMode = OMAP35X_GPMC_SYSCONFIG(hNtmHook->pulGPMCBase);
    OMAP35X_GPMC_SYSCONFIG(hNtmHook->pulGPMCBase) = OMAP35X_GPMC_SYSCONFIG_NO_IDLE;

  #if USE_PREFETCH_MODE  
    if(nBytes >= OMAP35X_GPMC_PREFETCH_FIFO_BYTES)
    {
        DclAssert(nBytes % OMAP35X_GPMC_PREFETCH_FIFO_BYTES == 0);
        
        PrefetchSetup(hNtmHook, TRUE, nBytes);

        REG32SET(OMAP35X_GPMC_PREFETCH_CONTROL(hNtmHook->pulGPMCBase), OMAP35X_GPMC_PREFETCH_CONTROL_STARTENGINE);

        while(nBytes)
        {
            D_UINT32    ulFifoLev = 0;

            /*  Copy the data
            */
            IOMemCopy((D_UINT8*)hNtmHook->apFIFO[hNtmHook->nChip], pBuffer, OMAP35X_GPMC_PREFETCH_FIFO_BYTES);
            pBuffer += OMAP35X_GPMC_PREFETCH_FIFO_BYTES / sizeof(*pBuffer);
            nBytes -= OMAP35X_GPMC_PREFETCH_FIFO_BYTES;

            /*  Wait for the FIFO threshold to be reached
            */
            while(ulFifoLev < OMAP35X_GPMC_PREFETCH_FIFO_BYTES)
            {
                ulFifoLev = OMAP35X_GPMC_PREFETCH_STATUS(hNtmHook->pulGPMCBase);
                ulFifoLev &= OMAP35X_GPMC_PREFETCH_STATUS_FIFOPOINTER_MASK;
                ulFifoLev >>= OMAP35X_GPMC_PREFETCH_STATUS_FIFOPOINTER_SHIFT;
            }
        }
    }
  #endif

    while(nBytes)        
    {
        OMAP3XX_GPMC_NAND_PUT(DATA, hNtmHook->nChip, hNtmHook->pulGPMCBase, *pBuffer++);

        while(TRUE)
        {
            D_UINT32 ulStatusVal = OMAP35X_GPMC_STATUS(hNtmHook->pulGPMCBase);

            if(ulStatusVal & OMAP35X_GPMC_STATUS_WRITEBUFFEREMPTY)
                break;
        }

        nBytes -= sizeof(*pBuffer);            
    }
    
    /*  Restore the old vaues.
    */
    OMAP35X_GPMC_SYSCONFIG(hNtmHook->pulGPMCBase) = ulOldIdleMode;
    
    return;
}


/*-------------------------------------------------------------------
    Public: FfxHookNTMicronPageRead()

    Perform a platform-dependent page read, with or without ECC.

    If a pabECC pointer is provided, the buffer must be large enough
    to contain all the ECC bytes generated for during the operation, 
    based on the style of ECC scheme being used.  See the functions
    FfxHookEccCalcStart() and FfxHookEccCalcRead().

    Parameters:
        hNtmHook  - The NTM Hook handle
        pBuffer   - A pointer to a buffer to be filled with the data
                    read from the device.
        nCount    - Number of NAND_IO_TYPE elements in the buffer
                    to be read from the device.
        pabECC    - A pointer to an array of bytes to receive the ECC
                    for the page, or NULL if no ECC is to be performed.

    Return Value:
        Returns an FFXSTATUS code indicating the results.
-------------------------------------------------------------------*/
FFXSTATUS FfxHookNTMicronPageRead(
    NTMHOOKHANDLE   hNtmHook,
    NAND_IO_TYPE   *pBuffer,
    unsigned        nCount,
    D_BUFFER       *pabECC)
{
    size_t          nBytes = nCount * sizeof(*pBuffer);
    
    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTMHOOK, 2, 0),
        "FfxHookNTMicronPageRead() hNtmHook=%P Chip=%u pBuff=%P Count=%u pECCBuff=%P\n",
        hNtmHook, hNtmHook->nChip, pBuffer, nCount, pabECC));

    DclAssert(hNtmHook);
    DclAssertWritePtr(pBuffer, nCount * sizeof(*pBuffer));
    DclAssert(nCount);
    DclAssert((nBytes % DATA_BYTES_PER_ECC) == 0);
    DclAssert(hNtmHook->nChip < MAX_CHIPS);
    DclAssert(DCLBITGET(&hNtmHook->nValidChipMask, hNtmHook->nChip));

  #if USE_PREFETCH_MODE  
    {
        D_BUFFER   *pOriginalBuffer = (D_BUFFER*)pBuffer;
        D_UINT32    ulOldIdleMode;
        
        DclAssert(nBytes % OMAP35X_GPMC_PREFETCH_FIFO_BYTES == 0);
        
        /*  Force the chip to NO IDLE Mode while Reads and Writes are happening.
            Otherwise the GPMC goes to idle mode and some time doesn't respond 
            as required.  Problem identified when enabled both Prefetch and
            Hardware ECC modes together.
        */
        ulOldIdleMode = OMAP35X_GPMC_SYSCONFIG(hNtmHook->pulGPMCBase);
        OMAP35X_GPMC_SYSCONFIG(hNtmHook->pulGPMCBase) = OMAP35X_GPMC_SYSCONFIG_NO_IDLE;

        /*  Specification (1.1.5.14.4.2): Hw ECC should be enabled before 
            enabling Prefetch Engine
        */        
        if(pabECC)
            FfxHookEccCalcStart(hNtmHook, pOriginalBuffer, nCount * sizeof(*pBuffer), pabECC, ECC_MODE_READ);

        PrefetchSetup(hNtmHook, FALSE, nBytes);
        
        REG32SET(OMAP35X_GPMC_PREFETCH_CONTROL(hNtmHook->pulGPMCBase), OMAP35X_GPMC_PREFETCH_CONTROL_STARTENGINE);

        while(nBytes)
        {
            D_UINT32    ulFifoLev = 0;

            /*  Wait for the FIFO threshold to be reached
            */
            while(ulFifoLev < OMAP35X_GPMC_PREFETCH_FIFO_BYTES)
            {
                ulFifoLev = OMAP35X_GPMC_PREFETCH_STATUS(hNtmHook->pulGPMCBase);
                ulFifoLev &= OMAP35X_GPMC_PREFETCH_STATUS_FIFOPOINTER_MASK;
                ulFifoLev >>= OMAP35X_GPMC_PREFETCH_STATUS_FIFOPOINTER_SHIFT;
            }

            /*  Copy the data
            */
            IOMemCopy(pBuffer, (D_UINT8*)hNtmHook->apFIFO[hNtmHook->nChip], OMAP35X_GPMC_PREFETCH_FIFO_BYTES);
            pBuffer += OMAP35X_GPMC_PREFETCH_FIFO_BYTES / sizeof(*pBuffer);
            nBytes -= OMAP35X_GPMC_PREFETCH_FIFO_BYTES;
        }

        if(pabECC)
            FfxHookEccCalcRead(hNtmHook, pOriginalBuffer, nCount * sizeof(*pBuffer), pabECC, ECC_MODE_READ);
        
        /*  Restore the old vaues.
        */
        OMAP35X_GPMC_SYSCONFIG(hNtmHook->pulGPMCBase) = ulOldIdleMode;
    }
    
  #else

    if(pabECC)
        FfxHookEccCalcStart(hNtmHook, (const D_BUFFER*)pBuffer, nBytes, pabECC, ECC_MODE_READ);

    FfxHookNTMicronDataIn(hNtmHook, pBuffer, nCount);

    if(pabECC)
        FfxHookEccCalcRead(hNtmHook, (const D_BUFFER*)pBuffer, nBytes, pabECC, ECC_MODE_READ);

  #endif

    return FFXSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    Public: FfxHookNTMicronPageWrite()

    Perform a platform-dependent page write, with or without ECC.

    If a pSpare pointer is provided, ECC will be calculated and
    placed into the spare area at the standard locations for 
    OffsetZero style flash.
    
    Parameters:
        hNtmHook  - The NTM Hook handle
        pData     - A pointer to a buffer containing the data to be
                    sent to the device
        nCount    - Number of NAND_IO_TYPE elements in the buffer
                    to be sent to the device
        pSpare    - A pointer to the spare area in which to place the
                    ECC for the data, or NULL if no ECC is to be 
                    performed.

    Return Value:
        Returns an FFXSTATUS code indicating the results.
-------------------------------------------------------------------*/
FFXSTATUS FfxHookNTMicronPageWrite(
    NTMHOOKHANDLE       hNtmHook,
    const NAND_IO_TYPE *pData,
    unsigned            nCount,
    D_BUFFER           *pSpare)
{
    size_t              nBytes = nCount * sizeof(*pData);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTMHOOK, 2, 0),
        "FfxHookNTMicronPageWrite() hNtmHook=%P Chip=%u pData=%P Count=%u pSpare=%P\n",
        hNtmHook, hNtmHook->nChip, pData, nCount, pSpare));

    DclAssert(hNtmHook);
    DclAssertReadPtr(pData, nCount * sizeof(*pData));
    DclAssert(nCount);  
    DclAssert((nBytes % DATA_BYTES_PER_ECC) == 0);
    DclAssert(hNtmHook->nChip < MAX_CHIPS);
    DclAssert(DCLBITGET(&hNtmHook->nValidChipMask, hNtmHook->nChip));

    if(!pSpare)
    {
        /*  If there is no spare pointer, then no ECC is being used, which
            is not a normal use case.  Just ignore the USE_PREFETCH_MODE
            setting in this special case, and simplify the rest of the code.
        */            
        FfxHookNTMicronDataOut(hNtmHook, pData, nCount);
    }
    else
    {
        D_BUFFER            abECC[MAX_ECC_BYTES_PER_PAGE];

      #if USE_PREFETCH_MODE  
        const D_BUFFER     *pOriginalBuffer = (const D_BUFFER*)pData;
        D_UINT32            ulOldIdleMode;
        
        DclAssert(nBytes % OMAP35X_GPMC_PREFETCH_FIFO_BYTES == 0);
        
        /*  Force the chip to NO IDLE Mode while Reads and Writes are happening.
            Otherwise the GPMC goes to idle mode and some time does not respond
            as required.  Problem identified when enabled both Prefetch and
            Hardware ECC modes together.
        */
        ulOldIdleMode = OMAP35X_GPMC_SYSCONFIG(hNtmHook->pulGPMCBase);
        OMAP35X_GPMC_SYSCONFIG(hNtmHook->pulGPMCBase) = OMAP35X_GPMC_SYSCONFIG_NO_IDLE;

        /*  Specification (1.1.5.14.4.2): HW ECC should be enabled before 
            enabling Prefetch Engine
        */ 
        FfxHookEccCalcStart(hNtmHook, pOriginalBuffer, nCount * sizeof(*pData), abECC, ECC_MODE_WRITE);
        
        PrefetchSetup(hNtmHook, TRUE, nBytes);        
        
        REG32SET(OMAP35X_GPMC_PREFETCH_CONTROL(hNtmHook->pulGPMCBase), OMAP35X_GPMC_PREFETCH_CONTROL_STARTENGINE);

        while(nBytes)
        {
            D_UINT32    ulFifoLev = 0;

            /*  Copy the data
            */
            IOMemCopy((D_UINT8*)hNtmHook->apFIFO[hNtmHook->nChip], pData, OMAP35X_GPMC_PREFETCH_FIFO_BYTES);
            pData += OMAP35X_GPMC_PREFETCH_FIFO_BYTES / sizeof(*pData);
            nBytes -= OMAP35X_GPMC_PREFETCH_FIFO_BYTES;

            /*  Wait for the FIFO threshold to be reached
            */
            while(ulFifoLev < OMAP35X_GPMC_PREFETCH_FIFO_BYTES)
            {
                ulFifoLev = OMAP35X_GPMC_PREFETCH_STATUS(hNtmHook->pulGPMCBase);
                ulFifoLev &= OMAP35X_GPMC_PREFETCH_STATUS_FIFOPOINTER_MASK;
                ulFifoLev >>= OMAP35X_GPMC_PREFETCH_STATUS_FIFOPOINTER_SHIFT;
            }
        }

        FfxHookEccCalcRead(hNtmHook, pOriginalBuffer, nCount * sizeof(*pData), abECC, ECC_MODE_WRITE);

        /*  Restore the old vaues
        */
        OMAP35X_GPMC_SYSCONFIG(hNtmHook->pulGPMCBase) = ulOldIdleMode;

      #else

        FfxHookEccCalcStart(hNtmHook, (const D_BUFFER*)pData, nBytes, abECC, ECC_MODE_WRITE);

        FfxHookNTMicronDataOut(hNtmHook, pData, nCount);

        FfxHookEccCalcRead(hNtmHook, (const D_BUFFER*)pData, nBytes, abECC, ECC_MODE_WRITE);

      #endif

        /*  Place the ECC data into the spare area.
        */            
        DclMemCpy(pSpare+ECC_OFFSET, abECC, ECC_LEN);

        pSpare[FLAGS_OFFSET] = (D_UINT8)LEGACY_WRITTEN_WITH_ECC;
    }

    return FFXSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    Public: FfxHookNTMicronStatusIn()

    Perform a single platform-dependent input operation from the
    NAND data port, for a status read, and returns the result.

    Parameters:
        hNtmHook - The NTM Hook handle

    Return Value:
        The status data read from the chip.
-------------------------------------------------------------------*/
NAND_IO_TYPE FfxHookNTMicronStatusIn(
    NTMHOOKHANDLE       hNtmHook)
{
    NAND_IO_TYPE        tStatus;

    DclAssert(hNtmHook);
    DclAssert(hNtmHook->nChip < MAX_CHIPS);
    DclAssert(DCLBITGET(&hNtmHook->nValidChipMask, hNtmHook->nChip));

    tStatus = OMAP3XX_GPMC_NAND_GET(DATA, hNtmHook->nChip, hNtmHook->pulGPMCBase);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTMHOOK, 3, 0),
        "FfxHookNTMicronStatusIn() hNtmHook=%P Chip=%u returning status %x\n", 
        hNtmHook, hNtmHook->nChip, tStatus));

    return tStatus;
}


/*-------------------------------------------------------------------
    Public: FfxHookNTMicronAddressOut()

    Perform a single platform-dependent output operation to the
    NAND address port, storing the provided address value.

    Parameters:
        hNtmHook - The NTM Hook handle
        bAddress - The data to store to the address port

    Return Value:
        None.
-------------------------------------------------------------------*/
void FfxHookNTMicronAddressOut(
    NTMHOOKHANDLE   hNtmHook,
    D_BYTE          bAddress)
{
    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTMHOOK, 1, 0),
        "FfxHookNTMicronAddressOut() hNtmHook=%P Chip=%u Addr=%02x\n", 
        hNtmHook, hNtmHook->nChip, bAddress));

    DclAssert(hNtmHook);
    DclAssert(hNtmHook->nChip < MAX_CHIPS);

    /*  Can't assert this because we must set the chip select in
        order to do the Read-ID, and we have to do that before
        the "configure" function is called which initializes the
        mask bits.

    DclAssert(DCLBITGET(&hNtmHook->nValidChipMask, hNtmHook->nChip));
    */        

    OMAP3XX_GPMC_NAND_PUT(ADDRESS, hNtmHook->nChip, hNtmHook->pulGPMCBase, bAddress);
}


/*-------------------------------------------------------------------
    Public: FfxHookNTMicronGetPageStatus()

    Extract the page status bits from the supplied spare area data.

    Parameters:
        hNtmHook - The NTM Hook handle
        pSpare   - A pointer to a buffer containing the spare area 
                   data.

    Return Value:
        Returns a D_UINT32 containing the page status bits.
-------------------------------------------------------------------*/
D_UINT32 FfxHookNTMicronGetPageStatus(
    NTMHOOKHANDLE   hNtmHook,
    const D_BUFFER *pSpare)
{
    DclAssert(hNtmHook);
    DclAssert(pSpare);
    DclAssert(hNtmHook->nChip < MAX_CHIPS);
    DclAssert(DCLBITGET(&hNtmHook->nValidChipMask, hNtmHook->nChip));

    return FfxNtmHelpGetPageStatus(pSpare, TAG_OFFSET, FLAGS_OFFSET);
}


#if USE_PREFETCH_MODE  

/*-------------------------------------------------------------------
    Local: PrefetchSetup()

    Parameters:
        hNtmHook - The NTM Hook handle

    Return Value:
        None.
-------------------------------------------------------------------*/
static void PrefetchSetup(
    NTMHOOKHANDLE   hNtmHook,
    D_BOOL          fWriting,
    size_t          nCount)
{
    D_UINT32        ulMode;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTMHOOK, 3, 0),
        "FfxHookNTMicron:PrefetchSetup() hNtmHook=%P Chip=%u fWriting=%u Count=%u\n", 
        hNtmHook, hNtmHook->nChip, fWriting, nCount));

    DclAssertReadPtr(hNtmHook, sizeof(*hNtmHook));
    DclAssert(nCount);
    DclAssert(hNtmHook->nChip < MAX_CHIPS);
    DclAssert(DCLBITGET(&hNtmHook->nValidChipMask, hNtmHook->nChip));

    /*  Disable the engine
    */
    OMAP35X_GPMC_PREFETCH_CONTROL(hNtmHook->pulGPMCBase) = 0;

    /*  Configure the mode bits
    */
    ulMode = (OMAP35X_GPMC_PREFETCH_CONFIG1_ENGINECHIPSELECT(hNtmHook->nChip) |
              OMAP35X_GPMC_PREFETCH_CONFIG1_ENABLEOPTIMIZEDACCESS |
              OMAP35X_GPMC_PREFETCH_CONFIG1_CYCLEOPTIMIZE(1) |
              OMAP35X_GPMC_PREFETCH_CONFIG1_PFPWWEIGHTEDPRIO(0x0F) |
              OMAP35X_GPMC_PREFETCH_CONFIG1_PFPWENROUNDROBIN |
              OMAP35X_GPMC_PREFETCH_CONFIG1_FIFOTHRESHOLD(OMAP35X_GPMC_PREFETCH_FIFO_BYTES) |
              OMAP35X_GPMC_PREFETCH_CONFIG1_ENABLEENGINE);

    if(fWriting)
        ulMode |= OMAP35X_GPMC_PREFETCH_CONFIG1_ACCESSWRITEPOST;

    OMAP35X_GPMC_PREFETCH_CONFIG2(hNtmHook->pulGPMCBase) = nCount;
    OMAP35X_GPMC_PREFETCH_CONFIG1(hNtmHook->pulGPMCBase) = ulMode;

    return;
}


/*-------------------------------------------------------------------
    Local: IOMemCopy()

    Parameters:

    Return Value:
        None.
-------------------------------------------------------------------*/
static void IOMemCopy(
    void       *pDest,
    const void *pSource,
    size_t      nSize)
{
    DclAssert(nSize == 64);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTMHOOK, 3, 0),
        "FfxHookNTMicron:IOMemCopy() pDest=%P pSrc=%P Count=%u\n", 
        pDest, pSource, nSize));

    /*  If not aligned...
    */
    if((((D_UINTPTR)pDest) & 0x3) || (((D_UINTPTR)pSource) & 0x3))
    {
        const D_UINT8  *pSrc = (const D_UINT8*)pSource;
        D_UINT8        *pDst = (D_UINT8*)pDest;

        while(nSize--)
            *pDst++ = *pSrc++;
    }
    else
    {
        /*  Copy 2 32-byte chunks, aligned on 32-bit boundaries
        */
        DclCpuMemCpyAligned32_32(pDest, pSource, 2);
    }

    return;
}

#endif


