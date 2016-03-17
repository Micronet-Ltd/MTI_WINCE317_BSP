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

    This module contains the implementations for:

       FfxHookNTMX31Create()
       FfxHookNTMX31Destroy()
       FfxHookNTMX31SetParameters()
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: fhmx31.c $
    Revision 1.6  2009/10/14 22:45:02Z  keithg
    Removed now obsolete fxio.h include file; Changed MEM_PUT...
    macros to the new ones DCLMEM... macros.
    Revision 1.5  2009/04/08 20:35:25Z  davidh
    Function headers updated for AutoDoc.
    Revision 1.4  2009/03/26 22:34:37Z  billr
    Fix unbalanced trace indent/undent.
    Revision 1.3  2007/11/03 23:50:11Z  Garyp
    Updated to use the standard module header.
    Revision 1.2  2007/06/26 23:01:22Z  timothyj
    Added volatile keyword for register access.
    Revision 1.1  2007/05/05 02:07:02Z  timothyj
    Initial revision
---------------------------------------------------------------------------*/

/* Windows.h is needed for VirtualAlloc, GetLastError, etc. */
#include <windows.h>
/* ceddk.h is needed for PageSize, etc. */
#include <ceddk.h>

#include <flashfx.h>
#include <oecommon.h>
#include <nandconf.h>
#include <nandctl.h>


/*  Private helper functions used to map and unmap physical memory
*/
static FFXSTATUS MapAddress(void **ppVirt, void *pPhys, D_UINT32 ulLength, D_BOOL fCacheable);
static void UnmapAddress(void * pVirt, D_UINT32 ulLength);


/*  Private data - there is only one NFC in the MX31, so using a static/global
    is not only reasonable, but preferred.
*/
static void *pCCMBase = NULL;

/*  Private constants
*/

/*  CCM register physical base addresss
*/
#define CCM_REGS_PHYS_BASE      (0x53F80000)
#define CCM_REGS_EXTENT         (0x64)

/* CCM registers and bit fields
*/
#define CCMREG(offset)   ((D_UINT32 volatile *) ((char *) pCCMBase + offset))

#define CCM_RCSR                CCMREG(0x0C)
#define RCSR_NF16B              (1U << 31)
#define RCSR_NFMS               (1U << 30)



/*-------------------------------------------------------------------
    Public: FfxHookNTMX31Create()

    Allocate (if necessary) and inialize the private control
    structure.  Typically this will involve mapping the
    MX31 CCM registers into the address space of the
    calling process.

    Parameters:
        hDev - The device handle.

    Return Value:
        Returns a non-NULL opaque private pointer for this module,
        or NULL on failure.
-------------------------------------------------------------------*/
PNANDCTL FfxHookNTMX31Create(
    FFXDEVHANDLE       hDev)
{
    FFXSTATUS    ffxStat;

    (void)hDev;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTMHOOK, 1, TRACEINDENT),
                    "FfxHookNTMX31Create()\n"));

    DclAssert(pCCMBase == NULL);

    ffxStat = MapAddress((void **)&pCCMBase, (void *)CCM_REGS_PHYS_BASE, CCM_REGS_EXTENT, FALSE);
	if (ffxStat != FFXSTAT_SUCCESS)
    {
        FFXPRINTF(1, ("FHMX31: FfxHookNTMX31Create() Unable to map address.  ffxStat: %lX (%u)\n",
            ffxStat, ffxStat));
        return NULL;
    }

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTMHOOK, 1, TRACEUNDENT),
        "FfxHookNTMX31Create() returning MX31 Control: %lX\n", pCCMBase));

    return (PNANDCTL)pCCMBase;
}


/*-------------------------------------------------------------------
    Public: FfxHookNTMX31Destroy()

    Deallocate (if necessary) and deinialize the
    MX31 control structure and any relevant hardware shutdown.
    Typically this will involve unmapping the CCM registers.

    Parameters:
        pMX31Ctl - pointer to the MX31 control structure, private
                   to this module.

    Return Value:
        An FFXSTATUS value indicating success or the nature of
        any failure.
-------------------------------------------------------------------*/
FFXSTATUS FfxHookNTMX31Destroy(PNANDCTL pNC)
{
    DclAssert(pNC);
    DclAssert(pCCMBase);
    DclAssert(pNC == pCCMBase);

    UnmapAddress((void *)pCCMBase,
                 CCM_REGS_EXTENT);

    pCCMBase = NULL;

    return FFXSTAT_SUCCESS;
}

/*-------------------------------------------------------------------
    Public: FfxHookNTMX31SetParameters()

    Configure the MX31 with the chip parameters.  Typically
    this only involves programming the RCSR register in
    a fashion that is synchronized with other system
    components that may be accessing the RCSR register.

    NOTE!   There is a timing hole here that cannot
            be closed solely within the module that calls this
            function.

            System software MUST ensure that this module synchronizes
            access to the RCSR register with any other component that
            may be accessing the RCSR register.

    Parameters:
        pNC          - A pointer to the private NANDCTL structure
        ucWidthBytes - number of bytes of width of the NAND device
                       data bus (1 - x8, 2 - x16)
        uPageSize    - page size of the NAND device


    Return Value:
        Returns TRUE if the system is configured for the width
        and page size specified.
-------------------------------------------------------------------*/
D_BOOL FfxHookNTMX31SetParameters(
    PNANDCTL        pNC,
    D_UCHAR         ucByteWidth,
    D_UINT16        ucPageSize)
{
    D_UINT32    ulRCSRVal;

    DclAssert(pNC == pCCMBase);

    ulRCSRVal = DCLMEMGET32(CCM_RCSR);

    /*  set or clear the bit indicating 16b wide data bus
        for the NFC.
    */
    if (ucByteWidth == 1)
    {
        ulRCSRVal &= ~RCSR_NF16B;
    }
    else if (ucByteWidth == 2)
    {
        ulRCSRVal |= RCSR_NF16B;
    }
    else
    {
        DclError();
        return FALSE;
    }

    if (ucPageSize == 512)
    {
        ulRCSRVal &= ~RCSR_NFMS;
    }
    else if (ucPageSize == 2048)
    {
        ulRCSRVal |= RCSR_NFMS;
    }
    else
    {
        DclError();
        return FALSE;
    }

    DCLMEMPUT32(CCM_RCSR, ulRCSRVal);

    return TRUE;
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

    /*  TBD: Put this in ALL implementations of MapAddress
    */
    if (ulLength < (D_UINT32)PAGE_SIZE)
    {
        ulLength = PAGE_SIZE;
    }

    ulAlignedLength = ulLength & ~(PAGE_SIZE - 1);

    /*  The flash address and length were both specified, map it and
        record the virtual address.
    */
    pVirt = VirtualAlloc(0, ulAlignedLength, MEM_RESERVE, PAGE_NOACCESS);
    if(!pVirt)
    {
        DWORD dwLastError = GetLastError();

        FFXPRINTF(1, ("FHMX31: VirtualAlloc() failed, LastError: 0x%08X (%u)\n",
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

        FFXPRINTF(1, ("FHMX31: VirtualCopy() failed, LastError: 0x%08X (%u)\n",
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


