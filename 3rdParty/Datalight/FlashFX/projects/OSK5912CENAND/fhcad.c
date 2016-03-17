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

    This module contains the FlashFX Hooks Layer default implementations for:

       FfxHookNTCADCreate()
       FfxHookNTCADDestroy()
       FfxHookNTCADSetChipSelect()
       FfxHookNTCADReadyWait()
       FfxHookNTCADSetChipCommand()
       FfxHookNTCADSetChipCommandNoWait()
       FfxHookNTCADDataIn()
       FfxHookNTCADDataOut()
       FfxHookNTCADStatusIn()
       FfxHookNTCADAddressOut()

    ToDo:
        This code needs to be updated to unmap the address upon destruction
        of the instance.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: fhcad.c $
    Revision 1.4  2009/10/14 23:59:57Z  keithg
    Removed now obsolete fxio.h include file.
    Revision 1.3  2007/11/03 23:50:35Z  Garyp
    Updated to use the standard module header.
    Revision 1.2  2007/05/11 00:14:51Z  neals
    Added includes for windows.h and ceddk.h to enable build in Platform Builder
    Revision 1.1  2006/07/10 22:38:46Z  Garyp
    Initial revision
    Revision 1.3  2006/07/09 02:39:18Z  Garyp
    Fixed to build cleanly (hopefully).
    Revision 1.2  2006/07/06 02:09:11Z  Garyp
    Updated to use the Create/Destroy metaphor.  Change the return values to
    use standard FFXSTATUS values.
    Revision 1.1  2006/05/08 18:23:12Z  timothyj
    Initial revision
---------------------------------------------------------------------------*/

/* Windows.h is needed for VirtualAlloc, GetLastError, etc. */
#include <windows.h>
/* ceddk.h is needed for PageSize, etc. */
#include <ceddk.h>

#include <flashfx.h>
#include <nandconf.h>
#include <nandctl.h>

#include "omap5912.h"

static volatile D_UINT32 * gEMIFS_BASE = NULL;
static volatile NAND_IO_TYPE * gNAND_CTLR_BASE = NULL;

/*
 * Private helper functions used to map and unmap physical memory
 */
static FFXSTATUS MapAddress(void **ppVirt, void *pPhys, D_UINT32 ulLength, D_BOOL fCacheable);
static void UnmapAddress(void * pVirt, D_UINT32 ulLength);

typedef struct nand_ctl
{
    /*  Declare the NANDCTL structure here.
    */
    unsigned    placeholder;
} NANDCTL;


/*-------------------------------------------------------------------
    FfxHookNTCADCreate()

    Description
        This function creates a NAND Control Module instance which
        is associated with the ntcad NTM.

    Parameters
        hDev        - The Device handle

    Return Value
        Returns a pointer to the NANDCTL structure to use if
        successful, otherwise NULL.
 -------------------------------------------------------------------*/
NANDCTL * FfxHookNTCADCreate(
    FFXDEVHANDLE hDev)
{
    FFXSTATUS    ffxStat;
    D_UINT32     ulRegisterVal;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTMHOOK, 1, 0), "FfxHookNTCADCreate()\n"));

    DclAssert(hDev);

    (void)hDev;

    ffxStat = MapAddress((void **)&gEMIFS_BASE, (void *)EMIFS_BASE_PHYS, EMIFS_BASE_EXTENT, FALSE);
	if (ffxStat != FFXSTAT_SUCCESS)
	{
        FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTMHOOK, 1, TRACEUNDENT),
            "FfxHookNTCADFindHardware() Unable to map EMIFS address.  ffxStat: %lX (%u)\n",
            ffxStat, ffxStat));
        return NULL;
	}

    ffxStat = MapAddress((void **)&gNAND_CTLR_BASE, (void *)NAND_CTRL_BASE_PHYS, NAND_CTRL_BASE_EXTENT, FALSE);
	if (ffxStat != FFXSTAT_SUCCESS)
	{
        FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTMHOOK, 1, TRACEUNDENT),
            "FfxHookNTCADFindHardware() Unable to map NAND controller address.  ffxStat: %lX (%u)\n",
            ffxStat, ffxStat));
        UnmapAddress((void *)gEMIFS_BASE, EMIFS_BASE_EXTENT);
        return NULL;
	}

    /*  Set EMIFS Config Register
    */
    ulRegisterVal = READ_REGISTER32(EMIFS_CONFIG);
    WRITE_REGISTER32(EMIFS_CONFIG, ulRegisterVal | EMIFS_CONFIG_FLAGS);

    /*  Set EMIFS CCS2 Register
    */
    WRITE_REGISTER32(EMIFS_CCS2, EMIFS_CCS2_VALUE);

    /*  Always succeed -- physical addresses used directly below
    */
    return (PNANDCTL)1;
}


/*-------------------------------------------------------------------
    FfxHookNTCADDestroy()

    Description
        This function destroys a NAND Control Module instance.

    Parameters
        pNC         - A pointer to the NANDCTL structure

    Return Value
        Returns an FFXSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
FFXSTATUS FfxHookNTCADDestroy(
    NANDCTL        *pNC)
{
    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTMHOOK, 1, 0),
        "FfxHookNTCADDestroy() pNC=%P\n", pNC));

    DclAssert(pNC);

    (void)pNC;

    return FFXSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    FfxHookNTCADSetChipSelect()

    Description
        This function selects a particular NAND chip.  Any control
        bits are set to zero and all other chips are deselected.

    Parameters
        pNC      - A pointer to the NANDCTL structure for the device
        ulChip   - The chip number (high bits of linear address)

    Return Value
        Returns an FFXSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
FFXSTATUS FfxHookNTCADSetChipSelect(
    NANDCTL            *pNC,
    D_UINT32            ulChip,
    CHIP_SELECT_MODE    mode)
{
    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTMHOOK, 1, 0),
        "FfxHookNTCADSetChipSelect() pNC=%P Chip=%lU Mode=%s\n",
         pNC, ulChip, mode == MODE_WRITE ? "MODE_WRITE" : "MODE_READ"));

    DclAssert(pNC);

    /*  Avoid compiler warnings
    */
    (void)pNC;
    (void)ulChip;
    (void)mode;

    return FFXSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    FfxHookNTCADReadyWait()

    Description
        Poll the BSY/RDY pin of the NAND chip until it indicates that
        the device is ready or a timeout limit is reached.  This is
        often called before a command is written to the flash to be
        sure the device will listen before the command is sent.  The
        chip selects must/will be set prior to this function call.
        This function could look at the busy pins of the device or
        send a read status command.

        The timeout limit is specified by NAND_TIME_OUT in
        nandconf.h.

    Parameters
        pNC      - A pointer to the NANDCTL structure for the device

    Return Value
        Returns an FFXSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
FFXSTATUS FfxHookNTCADReadyWait(
    NANDCTL    *pNC)
{
    DCLTIMER    t;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTMHOOK, 1, 0),
        "FfxHookNTCADReadyWait() pNC=%P\n", pNC));

    DclAssert(pNC);

    /*  Avoid compiler warnings
    */
    (void)pNC;

    DclTimerSet(&t, NAND_TIME_OUT);
    while(!DclTimerExpired(&t))
    {
        if(READ_REGISTER32(EMIFS_CONFIG) &NAND_RDY)
        {
            return FFXSTAT_SUCCESS;
        }
    }

    /*  Check one more time to make sure we were not interrupted
        and did in fact finish.
    */
    if(READ_REGISTER32(EMIFS_CONFIG) &NAND_RDY)
    {
        return FFXSTAT_SUCCESS;
    }
    else
    {
        return FFXSTAT_FIMTIMEOUT;
    }
}


/*-------------------------------------------------------------------
    FfxHookNTCADSetChipCommand()

    Description
        Waits for the chip to be ready and then sends a command using
        FfxHookNTCADSetChipCommandNoWait().

    Parameters
        pNC       - A pointer to the NANDCTL structure for the device
        ucCommand - the command to send

    Return Value
        Returns an FFXSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
FFXSTATUS FfxHookNTCADSetChipCommand(
    NANDCTL    *pNC,
    D_UCHAR     ucCommand)
{
    FFXSTATUS   ffxStat;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTMHOOK, 1, 0),
        "FfxHookNTCADSetChipCommand() pNC=%P Cmd=%02X\n", pNC, (D_UINT16)ucCommand));

    DclAssert(pNC);

    ffxStat = FfxHookNTCADReadyWait(pNC);

    if(ffxStat == FFXSTAT_SUCCESS)
        ffxStat = FfxHookNTCADSetChipCommandNoWait(pNC, ucCommand);

    return ffxStat;
}


/*-------------------------------------------------------------------
    FfxHookNTCADSetChipCommandNoWait()

    Description
        Universal "send arbitrary command" helper routine used by all
        higher level chip-specific routines. This routine only takes
        care of making sure that the proper interface pins are
        manipulated so that the command is entered into a previously
        selected chip.  The appropriate chip must already be selected
        before calling this routine and subsequent address,
        confirmation and data bytes must be sent to the chips using
        calls to additional helper routines.

    Parameters
        pNC       - A pointer to the NANDCTL structure for the device
        ucCommand - the command to send

    Return Value
        Returns an FFXSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
FFXSTATUS FfxHookNTCADSetChipCommandNoWait(
    NANDCTL        *pNC,
    D_UCHAR         ucCommand)
{
    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTMHOOK, 1, 0),
        "FfxHookNTCADSetChipCommandNoWait() pNC=%P Cmd=%02X\n", pNC, (D_UINT16)ucCommand));

    DclAssert(pNC);

    /*  Avoid compiler warnings
    */
    (void)pNC;

    WRITE_REGISTER16(NAND_CTLR_COMMAND, (D_UINT16) ucCommand);

    return FFXSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    FfxHookNTCADDataIn()

    Description
        Performs a single platform-dependent input operation from the
        NAND data port and returns the result.

    Parameters
        pNC       - A pointer to the NANDCTL structure for the device
        pBuffer   - A pointer to a buffer containing the data to be
                    sent to the device
        uCount    - Number of NAND_IO_TYPE elements in the buffer
                    to be sent to the device

    Return Value
        The data read from the chip.
-------------------------------------------------------------------*/
void FfxHookNTCADDataIn(
    NANDCTL        *pNC,
    NAND_IO_TYPE   *pBuffer,
    D_UINT16        uCount)
{
    D_UINT16        uCounter;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTMHOOK, 3, 0),
        "FfxHookNTCADDataIn() pNC=%P pBuff=%P Count=%U\n", pNC, pBuffer, uCount));

    DclAssert(pNC);

    /*  Avoid compiler warnings
    */
    (void)pNC;

    for (uCounter = 0; uCounter < uCount; uCounter++)
    {
        pBuffer[uCounter] = (NAND_IO_TYPE)READ_REGISTER16(NAND_CTLR_DATA);
    }

}


/*-------------------------------------------------------------------
    FfxHookNTCADDataOut()

    Description
        Performs a single platform-dependent output operation to the
        NAND data port, storing the provided data.

    Parameters
        pNC       - A pointer to the NANDCTL structure for the device
        pBuffer   - A pointer to a buffer containing the data to be
                    sent to the device
        uCount    - Number of NAND_IO_TYPE elements in the buffer
                    to be sent to the device

    Return Value
        None
-------------------------------------------------------------------*/
void FfxHookNTCADDataOut(
    NANDCTL            *pNC,
    const NAND_IO_TYPE *pBuffer,
    D_UINT16            uCount)
{
    D_UINT16            uCounter;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTMHOOK, 3, 0),
        "FfxHookNTCADDataOut() pNC=%P pBuff=%P Count=%U\n", pNC, pBuffer, uCount));

    DclAssert(pNC);

    /*  Avoid compiler warnings
    */
    (void)pNC;

    for (uCounter = 0; uCounter < uCount; uCounter++)
    {
        WRITE_REGISTER16(NAND_CTLR_DATA, pBuffer[uCounter]);
    }
}


/*-------------------------------------------------------------------
    FfxHookNTCADStatusIn()

    Description
        Performs a single platform-dependent input operation from the
        NAND data port and returns the result.

    Parameters
        pNC       - A pointer to the NANDCTL structure for the device

    Return Value
        The data read from the chip.
-------------------------------------------------------------------*/
NAND_IO_TYPE FfxHookNTCADStatusIn(
    NANDCTL        *pNC)
{
    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTMHOOK, 3, 0),
        "FfxHookNTCADStatusIn() pNC=%P\n", pNC));

    DclAssert(pNC);

    /*  Avoid compiler warnings
    */
    (void)pNC;

    return (NAND_IO_TYPE)READ_REGISTER16(NAND_CTLR_DATA);
}

/*-------------------------------------------------------------------
    FfxHookNTCADAddressOut()

    Description
        Performs a single platform-dependent output operation to the
        NAND address port, storing the provided address value.

    Parameters
        pNC      - A pointer to the NANDCTL structure for the device
        address  - the data to store to the address port

    Return Value
        None
-------------------------------------------------------------------*/
void FfxHookNTCADAddressOut(
    NANDCTL    *pNC,
    D_UCHAR     address)
{
    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTMHOOK, 1, 0),
        "FfxHookNTCADAddressOut() pNC=%P Addr=%02X\n", pNC, (D_UINT16)address));

    DclAssert(pNC);

    /*  Avoid compiler warnings
    */
    (void)pNC;

    WRITE_REGISTER16(NAND_CTLR_ADDR, (D_UINT16) address);
}


/*
 * VirtualCopy is provided in one of the standard DLLs on Windows CE, but the
 *   prototype is in the platform-specific header files.
 */
extern BOOL VirtualCopy(LPVOID lpvDest, LPVOID lpvSrc, DWORD cbSize, DWORD fdwProtect);

static FFXSTATUS MapAddress( void **ppVirt, void *pPhys, D_UINT32 ulLength, D_BOOL fCacheable )
{

    void *pVirt;
    D_UINT32 ulPhysAligned;
    D_UINT32 ulPhysOffset;
    D_UINT32 ulAlignedLength;
    D_UINT32 ulFlags;

    ulPhysAligned = (unsigned long)pPhys & ~(PAGE_SIZE - 1);
    ulPhysOffset = (unsigned long)pPhys & (PAGE_SIZE - 1);

    if (ulPhysOffset != 0)
    {
        ulLength += PAGE_SIZE;
    }

    ulAlignedLength = ulLength & ~(PAGE_SIZE - 1);

    /*  The flash address and length were both specified, map it and
        record the virtual address.
    */
    pVirt = VirtualAlloc(0, ulAlignedLength, MEM_RESERVE, PAGE_NOACCESS);
    if(!pVirt)
    {
        DWORD dwLastError = GetLastError();

        FFXPRINTF(1, ("FHCAD: VirtualAlloc() failed, LastError: 0x%08X (%u)\n",
            dwLastError, dwLastError));
        return FFXSTAT_PROJMAPMEMFAILED;
    }

    ulFlags = PAGE_READWRITE | PAGE_PHYSICAL;
    if (!fCacheable)
    {
        ulFlags |= PAGE_NOCACHE;
    }

    /*
     * Note: The following cast and shift assume the pointer of specific size.
     */
    if(!VirtualCopy(pVirt, (void *)(ulPhysAligned >> 8),
                ulAlignedLength, ulFlags))
    {
        DWORD dwLastError = GetLastError();

        FFXPRINTF(1, ("FHCAD: VirtualCopy() failed, LastError: 0x%08X (%u)\n",
            dwLastError, dwLastError));

        VirtualFree(pVirt, 0, MEM_RELEASE);

        return FFXSTAT_PROJMAPMEMFAILED;
    }

    (D_UINT32)pVirt += ulPhysOffset;

    *ppVirt = pVirt;

    return FFXSTAT_SUCCESS;
}

static void UnmapAddress(void * pVirt, D_UINT32 ulLength)
{
    VirtualFree(pVirt, 0, MEM_RELEASE);
}


