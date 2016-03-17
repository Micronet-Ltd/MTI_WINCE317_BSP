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

    This module contains the default implementations of the RTOS project
    hooks for the PXA320 NTM.

    To customize this for a particular project, copy this module into the
    Project Directory, make your changes, and modify ffxproj.mak to build
    the new module.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: fhpxa320.c $
    Revision 1.10  2010/06/19 17:09:38Z  garyp
    Fixed to include ntmicron.h.
    Revision 1.9  2010/06/19 01:02:54Z  garyp
    Added support for Micron M60 flash, as well as Toshiba timings. 
    Revision 1.8  2010/02/12 22:14:27Z  garyp
    Updated to use the slightly revamped virtual memory mapping interface.
    Revision 1.7  2009/12/11 22:54:41Z  garyp
    Per consultation and experimentation, updated the flash timing parameters.
    Revision 1.6  2009/12/02 17:39:32Z  garyp
    Refactored to provide more flexible customization capabilities at
    startup -- to better accommodate different flash types.  Updated to 
    use DclOsVirtualPointerMap().
    Revision 1.5  2009/10/14 01:26:21Z  keithg
    Removed now obsolete fxio.h include file.
    Revision 1.4  2009/04/08 15:09:50Z  davidh
    Function headers updated for AutoDoc.
    Revision 1.3  2008/01/13 07:29:23Z  keithg
    Function header updates to support autodoc.
    Revision 1.2  2007/11/03 23:50:01Z  Garyp
    Updated to use the standard module header.
    Revision 1.1  2007/03/13 18:45:28Z  rickc
    Initial revision
---------------------------------------------------------------------------*/

#include <flashfx.h>
#include <nandconf.h>
#include <nandctl.h>
#include <nandcmd.h>
#include <fhpxa320.h>
#include <hardware/hwpxa320.h>

#if FFXCONF_NANDSUPPORT_MICRON
#include <ntmicron.h>
#endif


/*  Should this module be copied into the Project Directory and customized
    by the OEM, the following "virtual header" could be split out into a
    separate header file, should the settings need to be shared by more 
    than one module.
*/
/*-------------------------------------------------------------------------*/
/*                          Virtual Header START                           */
/*                                                                         */  
    #define USE_CACHE_MODE    TRUE  /* Must be TRUE for checkin */  
    #define SUPPORT_CS1       TRUE  /* Must be TRUE for checkin */      
    #define USE_M60_ONDIE_EDC TRUE  /* Must be TRUE for checkin */     
    #define M60_EDC4_CHIP        0  /* Chip for which M60_EDC4_BLOCK applies */
    #define M60_EDC4_BLOCK       0  /* First block on M60_EDC4_CHIP which uses on-die 4-bit EDC */
    
    struct sNTMHOOK
    {
        volatile D_UINT32  *pulPXA320Base;
    };
/*                                                                         */  
/*                          Virtual Header END                             */  
/*-------------------------------------------------------------------------*/



/*-------------------------------------------------------------------
    Public: FfxHookNTPxa320Create()

    Allocate and initialize the PXA320 control structure, as well as
    map the PXA320 NAND controller's memory into the address space of
    the calling process.  Also supplies basic run-time configuration
    information to the NTM.

    Parameters:
        hDev    - The Device handle.
        pParams - A pointer to the FFXPXA320PARAMS structure to use.

    Return Value:
        Returns a NTMHOOKHANDLE if successful, or NULL otherwise.
-------------------------------------------------------------------*/
NTMHOOKHANDLE FfxHookNTPxa320Create(
    FFXDEVHANDLE        hDev,
    FFXPXA320PARAMS    *pParams)
{
    FFXSTATUS           ffxStat;
    NTMHOOKHANDLE       hNtmHook;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTMHOOK, 1, 0),
        "FfxHookNTPxa320Create() PXA320 offset: %lX\n", PXA3XX_DFC_REGS_OFFSET));

    DclAssert(hDev);
    DclAssertWritePtr(pParams, sizeof(*pParams));
    DclAssert(pParams->nStructLen == sizeof(*pParams));

    (void)hDev;

    hNtmHook = DclMemAllocZero(sizeof(*hNtmHook));
    if(!hNtmHook)
    {
        FFXPRINTF(1, ("FfxHookNTPxa320Create() Unable to allocate memory!\n"));
        return NULL;
    }

    ffxStat = DclOsVirtualPointerMap((void**)&hNtmHook->pulPXA320Base, PXA3XX_DFC_REGS_OFFSET,
                                     PXA3XX_DFC_REGS_EXTENT * sizeof(NAND_IO_TYPE), FALSE);
    if(ffxStat != FFXSTAT_SUCCESS)
    {
        FFXPRINTF(1, ("FfxHookNTPxa320Create() Unable to map register base %lX, Status=%lX\n", PXA3XX_DFC_REGS_OFFSET, ffxStat));

        return NULL;
    }

    /*  Insert any additional code to initialize the PXA320 control
        structure here.
    */
  
    /*  Specify the default timing characteristics.  These settings
        must be generic enough to allow the NAND ID to be read, 
        regardless of the flash type.  Once the ID has been read,
        FfxHookNTPxa320Configure() will be called to allow optimal
        timings to be specified.
    */    
    pParams->ulDefaultTR0           = PXA320_DFC_DEFAULT_TR0;
    pParams->ulDefaultTR1           = PXA320_DFC_DEFAULT_TR1;
    pParams->pulPXA320Base          = hNtmHook->pulPXA320Base;
    pParams->fSupportCacheMode      = USE_CACHE_MODE;
    pParams->fSupportCS1            = SUPPORT_CS1;
    pParams->fSupportM60OnDieEDC    = USE_M60_ONDIE_EDC;
    pParams->nM60EDC4Chip           = M60_EDC4_CHIP;
    pParams->ulM60EDC4Block         = M60_EDC4_BLOCK;

    return hNtmHook;
}


/*-------------------------------------------------------------------
    Public: FfxHookNTPxa320Destroy()

    Deallocate and deinitialize the PXA320 control structure and
    corresponding PXA320 NAND controller.

    Parameters:
        hNtmHook - The hook handle returned from the Create() function.

    Return Value:
        An FFXSTATUS value indicating success or the nature of
        any failure.
-------------------------------------------------------------------*/
FFXSTATUS FfxHookNTPxa320Destroy(
    NTMHOOKHANDLE hNtmHook)
{
    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTMHOOK, 1, 0),
        "FfxHookNTPxa320Destroy() hNtmHook=%P\n", hNtmHook));

    DclAssertReadPtr(hNtmHook, sizeof(hNtmHook));

    (void)DclOsVirtualPointerUnmap((void*)hNtmHook->pulPXA320Base, 
                                   PXA3XX_DFC_REGS_OFFSET, 
                                   PXA3XX_DFC_REGS_EXTENT * sizeof(NAND_IO_TYPE));
	
    return DclMemFree(hNtmHook);
}


/*-------------------------------------------------------------------
    Public: FfxHookNTPxa320Configure()

    Perform any necessary configuration of the PXA320 NAND controller,
    as necessary for the flash with the specified NAND ID.

    *Note* -- In the event that there are multiple instances of this
              NTM, this function will only be called when the first
              instance is initialized (the PXA320 has only one set of
              timing registers, regardless whether one or two chips
              are used).  This means that if there are multiple NAND
              chips which have different timing characteristics, the
              timing for the first chip will be what is used.

    Parameters:
        hNtmHook  - The hook handle returned from the Create() function.
        pChipInfo - A pointer to the FFXNANDCHIP structure to use
        pabID     - A pointer to an array of NAND ID bytes, which will
                    contain at least NAND_ID_SIZE entries.

    Return Value:
        An FFXSTATUS value indicating success or the nature of
        any failure.
-------------------------------------------------------------------*/
FFXSTATUS FfxHookNTPxa320Configure(
    NTMHOOKHANDLE       hNtmHook,
    const FFXNANDCHIP  *pChipInfo,
    D_BYTE             *pabID)
{
    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTMHOOK, 1, 0),
        "FfxHookNTPxa320Configure() hNtmHook=%P pChipInfo=%P pabID=%P\n", hNtmHook, pChipInfo, pabID));

    DclAssertWritePtr(hNtmHook, sizeof(hNtmHook));
    DclAssertReadPtr(pChipInfo, sizeof(*pChipInfo));
    DclAssertWritePtr(pabID, NAND_ID_SIZE);

    if((pChipInfo->bFlags & CHIPINT_MASK) == CHIPINT_16BIT)
        PXA3XX_NDCR(hNtmHook->pulPXA320Base) |= (PXA3XX_NDCR_DWIDTH_C | PXA3XX_NDCR_DWIDTH_M);

    if(pChipInfo->pChipClass->uPageSize == 2048)
        PXA3XX_NDCR(hNtmHook->pulPXA320Base) |= (PXA3XX_NDCR_PAGE_SZ_2048 | PXA3XX_NDCR_RA_START | PXA3XX_NDCR_PG_PER_BLK_64);

    /*  Timing should come from an ID table in the NTM.  This isn't yet
        implemented, it is a possible future performance optimization.
    */
    
  #if FFXCONF_NANDSUPPORT_MICRON
    if(pabID[0] == NAND_MFG_MICRON)
    {
        PXA3XX_NDTR0CS0(hNtmHook->pulPXA320Base) = PXA320_tCH_CS_WH_WP_RH_RP(45, 25, 15, 25, 35, 35);

        if(USE_M60_ONDIE_EDC && ((pabID[4] & MICRONID4_ONDIE_EDC_MASK) == MICRONID4_ONDIE_EDC_4BIT))
        {
            FFXPRINTF(1, ("FfxHookNTPxa320Configure() using Micron M60 flash timing values\n"));
            PXA3XX_NDTR1CS0(hNtmHook->pulPXA320Base) = PXA320_tR_WHR_AR(70106, 10, 29);   
        }
        else
        {
            FFXPRINTF(1, ("FfxHookNTPxa320Configure() using Micron default flash timing values\n"));
            PXA3XX_NDTR1CS0(hNtmHook->pulPXA320Base) = PXA320_tR_WHR_AR(25010, 15, 35);
        }           
    }
    else
  #endif
  #if FFXCONF_NANDSUPPORT_NUMONYX
    if(pabID[0] == NAND_MFG_NUMONYX)
    {
        FFXPRINTF(1, ("FfxHookNTPxa320Configure() using Numonyx flash timing values\n"));
        
        PXA3XX_NDTR0CS0(hNtmHook->pulPXA320Base) = PXA320_tCH_CS_WH_WP_RH_RP(45, 25, 15, 35, 35, 45);
        PXA3XX_NDTR1CS0(hNtmHook->pulPXA320Base) = PXA320_tR_WHR_AR(25010, 15, 15); 
    }
    else
  #endif
  #if FFXCONF_NANDSUPPORT_TOSHIBA
    if(pabID[0] == NAND_MFG_TOSHIBA)
    {
        FFXPRINTF(1, ("FfxHookNTPxa320Configure() using Toshiba flash timing values\n"));
        PXA3XX_NDTR0CS0(hNtmHook->pulPXA320Base) = PXA320_tCH_CS_WH_WP_RH_RP(5, 15, 15, 25, 35, 35);
        PXA3XX_NDTR1CS0(hNtmHook->pulPXA320Base) = PXA320_tR_WHR_AR(29925, 105, 15);
    }
    else
  #endif
    {
        (void)pabID;
        FFXPRINTF(1, ("FfxHookNTPxa320Configure() using default flash timing values\n"));
    }

    return FFXSTAT_SUCCESS;  
}
 


