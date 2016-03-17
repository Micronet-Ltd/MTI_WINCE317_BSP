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

    This module contains the required functions for filling the
    parameter page information read out from the NAND part to the 
    corresponding structures.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: nandparampage.c $
    Revision 1.4  2011/12/08 23:45:21Z  glenns
    Fix bugs regarding settings for erase cycle rating and BBM
    reserved block setting; clean up some comment formatting.
    Revision 1.3  2011/12/08 21:04:12Z  glenns
    Undo a workaround that was in place for working with certain
    engineering sample parts.
    Revision 1.2  2011/11/30 01:03:22Z  glenns
    Fix Bug 3554.
    Revision 1.1  2011/11/22 16:20:14Z  glenns
    Initial revision
---------------------------------------------------------------------------*/
#include <flashfx.h>
#include <nandid.h>
#include <nandparam.h>

#if FFXCONF_NANDSUPPORT

#define     RESERVED_UNKNOWN            D_UINT32_MAX

/*-------------------------------------------------------------------
    Public: FfxNandDecodeParameterPage()

    This function extracts all relevant information from NAND parameter page and 
    stores them in the NTM's param structure.

    Parameters:
        ParamPageArr    - Parameter Page ARRAY
        pParamPage      - Parameter Structure

    Return Value:
        Returns an FFXSTATUS value indicating success or failure.
-------------------------------------------------------------------*/

FFXSTATUS
FfxNandDecodeParameterPage(
    D_BYTE              *ParamPageArr,
    FFX_PARAMETER_PAGE  *pParamPage)
{
    D_UINT16 uFeaturesSupported;
    D_UINT8  ucBlockEndurance;
    D_UINT8  ucBlockEnduranceMultiplier;

    /* Feature information block
    */
    DCLLE2NE(&uFeaturesSupported, &ParamPageArr[FEATURES_SUPPORTED], sizeof(uFeaturesSupported));

    pParamPage->MultiPlaneProgramAndErase = (uFeaturesSupported & FEATURE_MULTI_PLANE_PROGRAM_ERASE) != 0;
    pParamPage->MultiPlaneRead = (uFeaturesSupported & FEATURE_MULTI_PLANE_READ) != 0;
    pParamPage->DataBusWidth16Bit = (uFeaturesSupported & FEATURE_16_BIT_DATA_BUS) != 0;

    pParamPage->EzNANDSupport = (uFeaturesSupported & FEATURE_EZNAND_SUPPORT) != 0;

    /* Manufacturer information block
    */
    DclMemCpy(pParamPage->acDeviceManufacturer, &ParamPageArr[DEVICE_MANUFACTURER], MAX_SZ_MANUFACTURER);
    DclMemCpy(pParamPage->acDeviceModel, &ParamPageArr[DEVICE_MODEL], MAX_SZ_DEV_MODEL);
    /* NULL terminate the device manufacturer and device model informations for safe printing
    */
    pParamPage->acDeviceManufacturer[MAX_SZ_MANUFACTURER] = '\0';
    pParamPage->acDeviceModel[MAX_SZ_DEV_MODEL] = '\0';
    pParamPage->ucJEDECid = ParamPageArr[JEDEC_MANUFACTURER_ID];

    /* Memory organization block
    */
    DCLLE2NE(&pParamPage->ulPageSize, &ParamPageArr[DATA_BYTES_PER_PAGE], sizeof(pParamPage->ulPageSize));
    DCLLE2NE(&pParamPage->uSpareSize, &ParamPageArr[SPARE_BYTES_PER_PAGE], sizeof(pParamPage->uSpareSize));

    DCLLE2NE(&pParamPage->ulPagePerBlock, &ParamPageArr[PAGES_PER_BLOCK], sizeof(pParamPage->ulPagePerBlock));
    DCLLE2NE(&pParamPage->ulBlocksPerLUN, &ParamPageArr[BLOCKS_PER_LUN], sizeof(pParamPage->ulBlocksPerLUN));

    /*  Be sure separate LUNs are dealt with as separate chips for BBM
        purposes. Otherwise, the following code would be incorrect:
    */
    pParamPage->bNumOfLuns = ParamPageArr[NUM_OF_LUNS];
    DCLLE2NE(&pParamPage->uReservedBlockRating, &ParamPageArr[BAD_BLOCKS_PER_LUN], sizeof(pParamPage->uReservedBlockRating));
    
    /*  See ONFI 2.3, Paragraph 5.7.1.21 for an explanation
        of the following:
    */
    ucBlockEndurance = ParamPageArr[BLOCK_ENDURANCE];
    ucBlockEnduranceMultiplier = ParamPageArr[BLOCK_ENDURANCE_MULTIPLIER];
    pParamPage->uBlockEndurance = ucBlockEndurance;
    {
        D_UINT8 i;
        
        for (i = 0; i < ucBlockEnduranceMultiplier; i++)
            pParamPage->uBlockEndurance *= 10;
    }
    
    pParamPage->bNumOfProgPerPage = ParamPageArr[PROG_PER_PAGE];
    
    return FFXSTAT_SUCCESS;
}

/*-------------------------------------------------------------------
    Public: FfxNandFillChipInfo()

    This function copies the read out parameter page data to 
    FFXNANDCHIP structure. Costumized for the purpose.

    Parameters:        
        pParamPage      - Parameter Structure

    Return Value:
        Returns an FFXNANDCHIP structure filled based up on 
        parameter page information.
-------------------------------------------------------------------*/
const FFXNANDCHIP *
FfxNandFillChipInfo(const D_UCHAR *aucID,
    const FFX_PARAMETER_PAGE *pParamPage)
{
    FFXNANDCHIPCLASS * pFfxChipClass = NULL;
    FFXNANDCHIP * pChip = NULL;

    D_UINT8                 uDevIDLen = NAND_ID_SIZE - 2, uNandFlags = 0;

    pFfxChipClass = DclMemAllocZero( sizeof(FFXNANDCHIPCLASS) );
    if( !pFfxChipClass )
        return NULL;
    
    pChip = DclMemAllocZero( sizeof(FFXNANDCHIP) );
    if( !pChip )
    {
        DclMemFree(pFfxChipClass);
        return NULL;
    }

    pFfxChipClass->uPageSize             = (D_UINT16)pParamPage->ulPageSize;
    pFfxChipClass->uSpareSize            = pParamPage->uSpareSize;
    pFfxChipClass->ulChipBlocks          = pParamPage->ulBlocksPerLUN * pParamPage->bNumOfLuns;
    pFfxChipClass->ulBlockSize           = pParamPage->ulPagePerBlock * pParamPage->ulPageSize;
    pFfxChipClass->ulEraseCycleRating    = pParamPage->uBlockEndurance;
    pFfxChipClass->ulBBMReservedRating   = pParamPage->uReservedBlockRating;
    switch(pParamPage->ucJEDECid)
    {
        case NAND_MFG_MICRON:
            pFfxChipClass->fProgramOnce               = 1;
            if(pParamPage->EzNANDSupport)
            {
                pFfxChipClass->uEdcRequirement        = 0;
                pFfxChipClass->uEdcCapability         = 0;
            }

            if(pParamPage->MultiPlaneProgramAndErase && pParamPage->MultiPlaneRead)
                pFfxChipClass->fMicronDualPlaneOps    = 1;

            switch(pParamPage->ulPageSize)
            {
                case 8192:

                    pFfxChipClass->ReadConfirm           = 1;
                    pFfxChipClass->Samsung2kOps          = 1;
                    
                    pFfxChipClass->uLinearPageAddrMSB    = 13;
                    pFfxChipClass->uChipPageIndexLSB     = 16;
                    pFfxChipClass->uLinearBlockAddrLSB   = 13;
                    pFfxChipClass->uLinearChipAddrMSB    = 35;

                    uNandFlags = pParamPage->DataBusWidth16Bit ? CHIPFLAGS_8KB_16BIT : CHIPFLAGS_8KB_8BIT;                    
                    break;

                default:
                    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, 0), "FfxNandFillChipInfo: Unsupported page size.\n"));
                    goto FfxNandFillChipInfoErrorCleanup;
            }
            break;

        default:
            FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, 0), "FfxNandFillChipInfo: Unsupported EZNand device.\n"));
            goto FfxNandFillChipInfoErrorCleanup;
    }

    /* Fill in the CHIP Class structure with the learned data from param page
    */
    pChip->uDevIdLength = uDevIDLen;
    DclMemCpy(pChip->aucDevID, &aucID[1], uDevIDLen);
    pChip->bFlags = uNandFlags;
        
#if VERBOSE_CHIP_INFO
    pChip->pPartNumber = pParamPage->acDeviceModel;
#endif
    pChip->pChipClass = pFfxChipClass;

    return (const FFXNANDCHIP *)pChip;

FfxNandFillChipInfoErrorCleanup:
    DclError();
    DclMemFree(pFfxChipClass);
    DclMemFree(pChip);
    return NULL;
}

#endif

