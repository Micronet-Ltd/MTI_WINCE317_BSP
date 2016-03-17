/*---------------------------------------------------------------------------
               ----> DO NOT REMOVE THE FOLLOWING NOTICE <----

                  Copyright (c) 1993 - 2011 Datalight, Inc.
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

    This module contains the default implementations for:

       FfxHookNTMX51Create()
       FfxHookNTMX51Destroy()
       FfxHookNTMX51SetParameters()
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: fhmx51.c $
    Revision 1.4  2011/12/13 22:37:42Z  glenns
    Add code to avoid compiler warning on unused statics.
    Revision 1.3  2011/12/13 18:26:51Z  glenns
    Fix typo in comment- no functional changes
    Revision 1.2  2011/12/13 05:49:18Z  glenns
    Correct compile error if DCLCONF_IOMEM is not set.
    Revision 1.1  2011/12/12 17:49:58Z  glenns
    Initial revision
---------------------------------------------------------------------------*/
#include <flashfx.h>
#include <oecommon.h>
#include <nandconf.h>
#include <nandctl.h>
#include <dlosmapmem.h>
#include <fhmx51.h>
 
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
    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTMHOOK, 1, TRACEINDENT),
                    "FfxHookNTMX51Create()\n"));

    /*  Here would be a good place to initialize the base pointers
        and set up certain registers in the MX51 controller. The
        code that is compiled out below is a sample of code that
        has been shown to work with a certain development board that
        uses the MX51 controller, with suitable definitions of
        MEM_GET_32 and MEM_PUT_32. For most abstractions, the
        procedures "DclOsMemIn32" and "DclOsMemOut32" defined in
        dlosiomem.h can be used for this purpose.
    */
    
#if 0    
    if (DclOsVirtualPointerMap(&pDPLL2, CSP_BASE_REG_PA_DPLL2, CSP_BASE_REGS_PLL2_SIZE, FALSE)
        != DCLSTAT_SUCCESS)
    {
        pDPLL2 = NULL;
    }


    uiReg = MEM_GET_32(DP_CTL) | MXC_PLL_DP_CTL_UPEN ;
    MEM_PUT_32(DP_CTL,uiReg);
    while((MEM_GET_32(DP_CTL) & MXC_PLL_DP_CTL_LRF) == 0);

    if (DclOsVirtualPointerMap(&g_pCCM, CSP_BASE_REG_PA_CCM, CSP_BASE_REG_PA_CCM_SIZE, FALSE)
        != DCLSTAT_SUCCESS)
    {
        g_pCCM = NULL;
    }

    
    /* Enable the ENFC and EMI SLOW  clock
    */
    
    uiReg = MEM_GET_32(CCM_CCGR5);
    uiReg |= (MXC_CCM_CCGR5_CG10_OFFSET_EMI_ENFC | MXC_CCM_CCGR5_CG8_OFFSET_EMI_SLOW);
    MEM_PUT_32(CCM_CCGR5,uiReg);
    
    DclOsVirtualPointerUnmap(pDPLL2, CSP_BASE_REG_PA_DPLL2, CSP_BASE_REGS_PLL2_SIZE);
    DclOsVirtualPointerUnmap(g_pCCM, CSP_BASE_REG_PA_CCM, CSP_BASE_REG_PA_CCM_SIZE);
    
    if (DclOsVirtualPointerMap(&pIPBase, CSP_BASE_REG_PA_NFC_IP, CSP_BASE_REGS_SIZE, FALSE)
        != DCLSTAT_SUCCESS)
    {
        pIPBase = NULL;
    }
   
    pParams->nMaxChips                  = 1;
    pParams->nAlignment                 = sizeof(NAND_IO_TYPE);
 
    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTMHOOK, 1, TRACEUNDENT),
                    "FfxHookNTMX51Create() returning MX51 Control: %lX\n", pIPBase));

#endif

    (void)pDPLL2;
    (void)g_pCCM;
    
    (void)hDev;
    (void)pParams;
    DclProductionError();
    
    return NULL;
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

    return DclOsVirtualPointerUnmap(pIPBase, CSP_BASE_REG_PA_NFC_IP, CSP_BASE_REGS_SIZE);
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


