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

    This module contains the main project function for a Windows CE project
    for the MX31 using NAND.  It displays a number of important settings.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: ffxmain.c $
    Revision 1.3  2007/11/03 23:50:32Z  Garyp
    Updated to use the standard module header.
    Revision 1.2  2007/07/30 18:13:25Z  timothyj
    Fixed math for computing MFI for timing.
    Revision 1.1  2007/03/23 23:02:30Z  timothyj
    Initial revision
---------------------------------------------------------------------------*/

/* Windows.h is needed for VirtualAlloc, GetLastError, etc. */
#include <windows.h>
/* ceddk.h is needed for PageSize, etc. */
#include <ceddk.h>

#include <flashfx.h>

/*  Private helper functions for mapping and unmapping addresses
*/
static FFXSTATUS MapAddress( void **ppVirt, void *pPhys, D_UINT32 ulLength, D_BOOL fCacheable );
static void UnmapAddress(void * pVirt, D_UINT32 ulLength);

/*-------------------------------------------------------------------
    FfxProjMain()

    Description
        This function is called by the FlashFX device driver at
        initialization time, immediately prior to the display of
        the FlashFX copyright messages.

    Parameters
        None

    Return Value
        Returns TRUE if the driver is to continue loading, or FALSE
        to cause the driver load process to be cancelled.
-------------------------------------------------------------------*/
D_BOOL FfxProjMain(void)
{

    FFXSTATUS           ffxStat;
    volatile D_UINT32 * pClockRegs;
    volatile D_UINT32 * pPinRegs;


    DclPrintf("MCIMX31LITEKIT Project for Windows CE 5.0\n");

    ffxStat = MapAddress( (void **)&pClockRegs, (void *)0x53f80000, PAGE_SIZE, FALSE );
    if (ffxStat == FFXSTAT_SUCCESS)
    {

        ffxStat = MapAddress( (void **)&pPinRegs, (void *)0x43FAC000, PAGE_SIZE, FALSE );
        if (ffxStat == FFXSTAT_SUCCESS)
        {
            /*  Print the pin configuration registers so we can verify the BSP set
                them as expected.
            */
            volatile D_UINT32 *pCCMR =  pClockRegs;     /* 0x53f80000 */
            volatile D_UINT32 *pPDR0 =  pClockRegs + 1; /* 0x53F80004 */
            volatile D_UINT32 *pRCSR =  pClockRegs + 3; /* 0x53F8000C */
            volatile D_UINT32 *pMPCTL = pClockRegs + 4; /* 0x53F80010 */

            volatile D_UINT32 *pGPR = pPinRegs + 2;                                /* 0x43FAC008 */
            volatile D_UINT32 *pSW_MUX_CTL_NFWP_B_NFCE_B_NFRB_D15 = pPinRegs + 52; /* 0x43FAC0D0 */
            volatile D_UINT32 *pSW_PAD_CTL_D2_D1_D0 = pPinRegs + 146;              /* 0x43FAC248 */
            volatile D_UINT32 *pSW_PAD_CTL_D5_D4_D3 = pPinRegs + 147;              /* 0x43FAC24C */
            volatile D_UINT32 *pSW_PAD_CTL_D8_D7_D6 = pPinRegs + 148;              /* 0x43FAC250 */
            volatile D_UINT32 *pSW_PAD_CTL_D11_D10_D9 = pPinRegs + 149;            /* 0x43FAC254 */
            volatile D_UINT32 *pSW_PAD_CTL_D14_D13_D12 = pPinRegs + 150;           /* 0x43FAC258 */
            volatile D_UINT32 *pSW_PAD_CTL_NFCE_B_NFRB_D15 = pPinRegs + 151;       /* 0x43FAC25C */

            D_UINT32 ulPD;
            D_UINT32 ulMFD;
            D_UINT32 ulMFI;
            D_INT32 lMFN;
            D_UINT32 ulFvco;
            D_UINT32 ulHclk;
            D_UINT32 ulNfcClk;
            D_UINT32 ulHclkDiv;
            D_UINT32 ulNfcDiv;
            D_UINT32 ulThns;

            DclPrintf("\"***\" indicates unexpected value.\n");
            DclPrintf("RCSR:                                %lX\n", *pRCSR);
            DclPrintf("\tRCSR.NF16B:     %s\n", *pRCSR & 0x80000000 ? "*** - 16bit" : "8bit"); 
            DclPrintf("\tRCSR.NFMS:      %s\n", *pRCSR & 0x40000000 ? "*** - 2KB Pages" : "512B pages"); 
            DclPrintf("GPR:                                 %lX\n", *pGPR);
            DclPrintf("\tGPR.GPR[5]:     %s\n", *pGPR & 0x00000020 ? "*** - Enable ATA Data 7-13 on NANDF contacts" : "Inactive");
            DclPrintf("\tGPR.GPR[6]:     %s\n", *pGPR & 0x00000040 ? "*** - Enable ATA signals on NANDF contacts" : "Inactive");
            DclPrintf("CCMR:                                %lX\n", *pCCMR);
            DclPrintf("\tCCMR.FPMF:      %s\n", *pCCMR & 0x04000000 ? "1024" : "*** - Unknown");
            DclPrintf("\tCCMR.MDS:       %s\n", *pCCMR & 0x00000080 ? "*** - Reference clock is the MCU clock domain source (bypass)" : "MCU PLL is the MCU clock domain source");
            DclPrintf("\tCCMR.MPE:       %s\n", *pCCMR & 0x00000008 ? "MCU PLL is enabled" : "*** - MCU PLL is disabled");
            switch ((*pCCMR & 0x00000006) >> 1)
            {
            case 0:
            case 3:
                DclPrintf("\tCCMR.PRCS:     *** - Reserved\n");
                break;
            case 1:
                DclPrintf("\tCCMR.PRCS:     *** - FPM\n");
                break;
            case 2:
                DclPrintf("\tCCMR.PRCS:     CKIH\n");
                break;
            default:
                DclError();
                break;

            }

            DclPrintf("PDR0:                                %lX\n", *pPDR0);
            ulNfcDiv = ((*pPDR0 & 0x00000700) >> 7) + 1;
            DclPrintf("\tPDR0.NFC_PODF:  %u (Divide by %u)\n", (*pPDR0 & 0x00000700) >> 7, ulNfcDiv);
            ulHclkDiv = ((*pPDR0 & 0x00000038) >> 3) + 1;
            DclPrintf("\tPDR0.MAX_PODF:  %u (Divide by %u)\n", (*pPDR0 & 0x00000038) >> 3, ulHclkDiv);

            DclPrintf("MPCTL:                               %lX\n", *pMPCTL);
            ulPD = ((*pMPCTL & 0x3C000000) >> 26) + 1;
            DclPrintf("\tMPCTL.PD:       %u (Divide by %u)\n", (*pMPCTL & 0x3C000000) >> 26, ulPD);
            ulMFD = ((*pMPCTL & 0x03FF00000) >> 16) + 1;
            DclPrintf("\tMPCTL.MFD:      %u (Use %u in eqn)\n", (*pMPCTL & 0x03FF00000) >> 16, ulMFD);
            ulMFI = ((*pMPCTL & 0x00003C00) >> 10);
            ulMFI = DCLMAX(ulMFI, 5);
            DclPrintf("\tMPCTL.MFI:      %u (Use %u in eqn)\n", (*pMPCTL & 0x00003C00) >> 10, ulMFI);

            lMFN = *pMPCTL & 0x000003FF;
            if (lMFN > 511)
            {
                lMFN-=1024;
            }

            DclPrintf("\tMPCTL.MFN:      %d\n", lMFN);

            ulFvco = (26000000 * 2 * ulMFI / ulPD)
                   + (26000000 * 2 * lMFN / ulMFD / ulPD);

            DclPrintf("\tComputed Fvco = %u (assumes 26MHZ Fref)\n", ulFvco);

            ulHclk = ulFvco / ulHclkDiv;
            DclPrintf("\tComputed hclk = %u (assumes Fvco above, MCU PLL selected)\n", ulHclk);
            ulNfcClk = ulHclk / ulNfcDiv;
            DclPrintf("\tComputed nfcclk = %u (assumes hclk above)\n", ulNfcClk);

            // compute period (in .5 nanoseconds intervals, so integer math works OK)
            // 'h' in 'hns' stands for 'half', as in 'half nanoseconds'
            ulThns = 2000000000 / ulNfcClk;
    DclPrintf("\tComputed 2T (Clock period in .5 ns intervals) = %u\n", ulThns);
    DclPrintf("\tNF1  (tCLS) NFCLE Setup Time    = minimum %uns\n", (ulThns - 2) / 2);
    DclPrintf("\tNF2  (tCLH) NFCLE Hold Time     = minimum %uns\n", (ulThns - 4) / 2);
    DclPrintf("\tNF3  (tCS)  NFCE Setup Time     = minimum %uns\n", (ulThns - 2) / 2);
    DclPrintf("\tNF4  (tCH)  NFCE Hold Time      = minimum %uns\n", (ulThns - 4) / 2);
    DclPrintf("\tNF5  (tWP)  NF_WP Pulse Width   = %uns\n", (ulThns - 3) / 2);
    DclPrintf("\tNF6  (tALS) NFALE Setup Time    = minimum %uns\n", ulThns / 2);
    DclPrintf("\tNF7  (tALH) NFALE Hold Time     = minimum %uns\n", (ulThns - 6) / 2);
    DclPrintf("\tNF8  (tDS)  Data Setup Time     = minimum %uns\n", ulThns / 2);
    DclPrintf("\tNF9  (tDH)  Data Hold Time      = minimum %uns\n", (ulThns - 10) / 2);
    DclPrintf("\tNF10 (tWC)  Write Cycle Time    = minimum %uns\n", ulThns);
    DclPrintf("\tNF11 (tWH)  NFWE Hold Time      = %uns\n", (ulThns - 5) / 2);
    DclPrintf("\tNF12 (tRR)  Ready to NFRE Low   = minimum %uns\n", 3 * ulThns);
    DclPrintf("\tNF13 (tRP)  NFRE Pulse Width    = minimum %uns\n", 3 * ulThns / 4);
    DclPrintf("\tNF14 (tRC)  READ Cycle Time     = minimum %uns\n", ulThns);
    DclPrintf("\tNF15 (tREH) NFRE High Hold Time = %uns\n", (ulThns - 10) / 4);


            DclPrintf("SW_MUX_CTL_NFWP_B_NFCE_B_NFRB_D15:   %lX\n", *pSW_MUX_CTL_NFWP_B_NFCE_B_NFRB_D15);
            DclPrintf("SW_PAD_CTL_D2_D1_D0:                 %lX\n", *pSW_PAD_CTL_D2_D1_D0);
            DclPrintf("SW_PAD_CTL_D5_D4_D3:                 %lX\n", *pSW_PAD_CTL_D5_D4_D3);
            DclPrintf("SW_PAD_CTL_D8_D7_D6:                 %lX\n", *pSW_PAD_CTL_D8_D7_D6);
            DclPrintf("SW_PAD_CTL_D11_D10_D9:               %lX\n", *pSW_PAD_CTL_D11_D10_D9);
            DclPrintf("SW_PAD_CTL_D14_D13_D12:              %lX\n", *pSW_PAD_CTL_D14_D13_D12);
            DclPrintf("SW_PAD_CTL_NFCE_B_NFRB_D15:          %lX\n", *pSW_PAD_CTL_NFCE_B_NFRB_D15);


            UnmapAddress((void *)pPinRegs, PAGE_SIZE);
            UnmapAddress((void *)pClockRegs, PAGE_SIZE);

            return TRUE;
        }

    }

    return FALSE;

}



/*
 * VirtualCopy is provided in one of the standard DLLs on Windows CE, but the
 *   prototype is in the platform-specific header files.
 */
extern BOOL VirtualCopy(LPVOID lpvDest, LPVOID lpvSrc, DWORD cbSize, DWORD fdwProtect);

/*-------------------------------------------------------------------
    MapAddress() - Map a virtual pointer to a physical address.

    Description - Call the operating system to retrive
        a value suitable for use as a pointer, from the
        physical address.

    Parameters
       ppVirt - On return, points to the virtual address.
       pPhys - Physical address to map.
       ulLength - length of the region to map.  Does not need
        to be aligned.
       fCacheable - indicates whether the virtual pointer will
        use the CPU cache.

    Returns
       An FFXSTATUS value indicating success or the nature of
          any failure.
-------------------------------------------------------------------*/
static FFXSTATUS MapAddress( void **ppVirt, void *pPhys, D_UINT32 ulLength, D_BOOL fCacheable )
{

    void     *pVirt;
    D_UINT32  ulPhysAligned;
    D_UINT32  ulPhysOffset;
    D_UINT32  ulAlignedLength;
    D_UINT32  ulFlags;

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

        FFXPRINTF(1, ("FH1NAND: VirtualAlloc() failed, LastError: 0x%08X (%u)\n",
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

        FFXPRINTF(1, ("FH1NAND: VirtualCopy() failed, LastError: 0x%08X (%u)\n",
            dwLastError, dwLastError));

        VirtualFree(pVirt, 0, MEM_RELEASE);

        return FFXSTAT_PROJMAPMEMFAILED;
    }

    (D_UINT32)pVirt += ulPhysOffset;

    *ppVirt = pVirt;

    return FFXSTAT_SUCCESS;
}

/*-------------------------------------------------------------------
    UnmapAddress() - Unmap a virtual pointer to a physical address.

    Description - Call the operating system to release
        a pointer (previously acquired with MapAddress).

    Parameters
       pVirt - The virtual address to unmap.
       ulLength - length of the region to unmap.  Does not need
        to be aligned, must match the length passed to MapAddress.
-------------------------------------------------------------------*/
static void UnmapAddress(void * pVirt, D_UINT32 ulLength)
{
    VirtualFree(pVirt, 0, MEM_RELEASE);
}


