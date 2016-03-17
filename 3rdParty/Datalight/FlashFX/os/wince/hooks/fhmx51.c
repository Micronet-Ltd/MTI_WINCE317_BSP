/*---------------------------------------------------------------------------
               ----> DO NOT REMOVE THE FOLLOWING NOTICE <----

                  Copyright (c) 1993 - 2010 Datalight, Inc.
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

       FfxHookNTMX51Create()
       FfxHookNTMX51Destroy()
       FfxHookNTMX51SetParameters()
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: fhmx51.c $
    Revision 1.1  2011/12/12 18:13:30Z  glenns
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
#include <fhmx51.h>

/*  Private helper functions used to map and unmap physical memory
*/
static FFXSTATUS MapAddress(void **ppVirt, void *pPhys, D_UINT32 ulLength, D_BOOL fCacheable);
static void UnmapAddress(void * pVirt, D_UINT32 ulLength);

/*  Private constants
*/

/*  IP register physical base addresss
*/
#define CSP_BASE_REG_PA_NFC_IP                  (0x83FD8000)
#define CSP_BASE_REGS_SIZE                      (0x00004000)

/*  PLL2 register physical base address
*/
#define CSP_BASE_REG_PA_DPLL2                   (0x83F84000)
#define CSP_BASE_REGS_PLL2_SIZE                 (0x0000002C)
#define MXC_PLL_DP_CTL_UPEN                     (0x20)
#define MXC_PLL_DP_CTL_LRF                      (0x1)

/*  Clock Grating Registers
*/
#define CSP_BASE_REG_PA_CCM                     (0x73FD4000)
#define CSP_BASE_REG_PA_CCM_SIZE                (0x00000084)
#define MXC_CCM_CCGR_CG_MASK                    (0x3)
#define MXC_CCM_CCGR5_CG10_OFFSET_EMI_ENFC      (0x3 << 20)
#define MXC_CCM_CCGR5_CG8_OFFSET_EMI_SLOW       (0x3 << 16)


/*  Private data - there is only one NFC in the MX51, so using a static/global
    is not only reasonable, but preferred.
*/
static void *pIPBase;
static void *pDPLL2;
static void *g_pCCM;

#define IPREG(offset)       ((D_UINT32 *) ((char *)pIPBase + 0x3000 + offset))
#define PLLREG(offset)      ((D_UINT32 *) ((char *)pDPLL2 + offset))
#define GPCCM(offset)       ((D_UINT32 *) ((char *)g_pCCM + offset)) 
#define DP_CTL               PLLREG(0x00)
#define CCM_CCGR5            GPCCM(0x7C)



/*-------------------------------------------------------------------
    FfxHookNTMX51Create()

    Description
        Allocate (if necessary) and inialize the private control
        structure.  Typically this will involve mapping the
        MX51 NFC_IP registers into the address space of the
        calling process.

    Parameters
        hDev    - The device handle.

    Return Value
        Returns a non-NULL opaque private pointer for this module,
        or NULL on failure.
-------------------------------------------------------------------*/
PNANDCTL FfxHookNTMX51Create(
    FFXDEVHANDLE       hDev,
    FFXMX51PARAMS      *pParams
)
{
    FFXSTATUS    ffxStat;
    D_UINT32     uiReg;
    (void)hDev;
    
    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTMHOOK, 1, TRACEINDENT),
                    "FfxHookNTMX51Create()\n"));

    ffxStat = MapAddress((void **)&pDPLL2, (void *)CSP_BASE_REG_PA_DPLL2, CSP_BASE_REGS_PLL2_SIZE, FALSE);
    if (ffxStat != FFXSTAT_SUCCESS)
    {
        FFXPRINTF(1, ("FHMX51: FfxHookNTMX51Create() Unable to map address.  ffxStat: %lX (%u)\n",
            ffxStat, ffxStat));
        return NULL;
    }

    /* Double check PLL2 is enabled
    */
    uiReg = DCLMEMGET32(DP_CTL);
    uiReg |= MXC_PLL_DP_CTL_UPEN;
    DCLMEMPUT32(DP_CTL, uiReg);

    while((DCLMEMGET32(DP_CTL) & MXC_PLL_DP_CTL_LRF) == 0);

    ffxStat = MapAddress((void **)&g_pCCM, (void *)CSP_BASE_REG_PA_CCM, CSP_BASE_REG_PA_CCM_SIZE, FALSE);
    if (ffxStat != FFXSTAT_SUCCESS)
    {
        FFXPRINTF(1, ("FHMX51: FfxHookNTMX51Create() Unable to map address.  ffxStat: %lX (%u)\n",
            ffxStat, ffxStat));
        return NULL;
    }
    
    /* Enable the ENFC and EMI SLOW  clock
    */
    
    uiReg = DCLMEMGET32(CCM_CCGR5);
    uiReg |= (MXC_CCM_CCGR5_CG10_OFFSET_EMI_ENFC | MXC_CCM_CCGR5_CG8_OFFSET_EMI_SLOW);
    DCLMEMPUT32(CCM_CCGR5, uiReg);
    
    /* Un map everything, since we are not going to use it any where
    */
    UnmapAddress((void *)pDPLL2, CSP_BASE_REGS_PLL2_SIZE);
    UnmapAddress((void *)g_pCCM, CSP_BASE_REG_PA_CCM_SIZE);
    pDPLL2 = NULL;
    g_pCCM = NULL;

    
    ffxStat = MapAddress((void **)&pIPBase, (void *)CSP_BASE_REG_PA_NFC_IP, CSP_BASE_REGS_SIZE, FALSE);
    if (ffxStat != FFXSTAT_SUCCESS)
    {
        FFXPRINTF(1, ("FHMX51: FfxHookNTMX51Create() Unable to map address.  ffxStat: %lX (%u)\n",
            ffxStat, ffxStat));
        return NULL;
    }
    
    pParams->nMaxChips                  = 1;
    pParams->nAlignment                 = sizeof(NAND_IO_TYPE);
 
    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTMHOOK, 1, TRACEUNDENT),
                    "FfxHookNTMX51Create() returning MX51 Control: %lX\n", pIPBase));

    return(PNANDCTL)pIPBase;
}


/*-------------------------------------------------------------------
    FfxHookNTMX51Destroy() - deallocate (if necessary) and
       deinitialize the MX51 control structure.

    Description - Deallocate (if necessary) and deinialize the
       MX51 control structure and any relevant hardware shutdown.
       Typically this will involve unmapping the NFC_IP registers.

    Parameters
       pMX51Ctl - pointer to the MX51 control structure, private
          to this module.

    Returns
       An FFXSTATUS value indicating success or the nature of
          any failure.
-------------------------------------------------------------------*/
FFXSTATUS FfxHookNTMX51Destroy(PNANDCTL pNC)
{
    DclAssert(pNC);
    DclAssert(pIPBase);
    DclAssert(pNC == pIPBase);

    UnmapAddress((void *)pIPBase, CSP_BASE_REGS_SIZE);

    pIPBase = NULL;


    return FFXSTAT_SUCCESS;
}

/*-------------------------------------------------------------------
    FfxHookNTMX51SetParameters()

    Description
        

    Parameters
        pNC      - A pointer to the private NANDCTL structure
      

    Return Value
        Returns TRUE if the system is configured for the width
        and page size specified.

-------------------------------------------------------------------*/
D_BOOL FfxHookNTMX51GetIPBaseAdr(
    PNANDCTL        pNC,
    void            **pBuffer
    )
{


    DclAssert(pNC == pIPBase);
    *(void **)pBuffer = pIPBase;
    
    return(TRUE);
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

        FFXPRINTF(1, ("FHMX51: VirtualAlloc() failed, LastError: 0x%08X (%u)\n",
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

        FFXPRINTF(1, ("FHMX51: VirtualCopy() failed, LastError: 0x%08X (%u)\n",
            dwLastError, dwLastError));

        VirtualFree(pVirt, 0, MEM_RELEASE);

        return FFXSTAT_PROJMAPMEMFAILED;
    }

    (D_UINT32)pVirt += ulPhysOffset;

    *ppVirt = pVirt;

    return FFXSTAT_SUCCESS;
}

static void UnmapAddress(void *pVirt, D_UINT32 ulLength)
{
    VirtualFree(pVirt, 0, MEM_RELEASE);
}


