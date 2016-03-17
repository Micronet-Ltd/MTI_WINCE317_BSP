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

    This header defines the interface between the Toshiba PBA-NAND NTM and 
    its hooks functions.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: nandparam.h $
    Revision 1.3  2011/12/08 22:42:22Z  glenns
    Fix bugs regarding settings for erase cycle rating and BBM
    reserved block setting; clean up some comment formatting.
    Revision 1.2  2011/11/30 01:03:22Z  glenns
    Fix Bug 3554.
    Revision 1.1  2011/11/17 18:43:50Z  glenns
    Initial revision
    Revision 1.1  2011/03/10
    Initial revision
---------------------------------------------------------------------------*/

#ifndef NANDPARAM_H_INCLUDED
#define NANDPARAM_H_INCLUDED

#define FFX_ONFI_SIG                    "ONFI"
#define ONFI_SIG_SZ                     (4)
#define NAND_PARAM_PAGE_SZ              (256)
#define MAX_SZ_DEV_MODEL                (20)
#define MAX_SZ_MANUFACTURER             (12)

/*
    Parameter Page structure Defenition
*/

typedef struct
{
    D_UINT8     bNumOfProgPerPage;
    D_UINT8     bNumOfLuns;
    D_UINT16    uSpareSize;
    D_UINT16    uReservedBlockRating;
    D_UINT16    uBlockEndurance;

    D_UINT32    ulPageSize;
    D_UINT32    ulPagePerBlock;
    D_UINT32    ulBlocksPerLUN;
    /*..Adding 1 to accommodate the null terminator
    */
    D_CHAR      acDeviceManufacturer[MAX_SZ_MANUFACTURER+1];
    D_CHAR      acDeviceModel[MAX_SZ_DEV_MODEL+1];
    D_UCHAR     ucJEDECid;
    unsigned    EzNANDSupport:1;
    unsigned    PageRegClear:1;
    unsigned    ExtendedParamPage:1;
    unsigned    MultiPlaneRead:1;
    unsigned    SourceSynch:1;
    unsigned    OddToEvenPageCopyBack:1;
    unsigned    MultiPlaneProgramAndErase:1;
    unsigned    NonSeqPageProgramming:1;
    unsigned    MultipleLUNOPeration:1;
    unsigned    DataBusWidth16Bit:1;
}FFX_PARAMETER_PAGE;

/*---------------------------------------------------------
    MACRO DEFENITIONS FOR NAND PARAMETER PAGE.
---------------------------------------------------------*/
#define SIGNATURE                       (0)
#define REVISION_NUMBER                 (4)
#define FEATURES_SUPPORTED              (6)
#define OPTIONAL_COMMANDS_SUPPORTED     (8)
#define EXTENDED_PARAM_PAGE_LEN         (12)
#define NUM_OF_PARAM_PAGES              (14)
#define MANUFACTURER_INFO_BLOCK         (32)
#define MEMORY_ORGANIZATION_BLOCK       (80)    
#define ELECTRICAL_PARAMS_BLOCK         (128)
#define VENDOR_BLOCK                    (164)

/*
    MEMORY_ORGANIZATION_BLOCK : SUB Defenitions
*/
#define DATA_BYTES_PER_PAGE             (80)
#define SPARE_BYTES_PER_PAGE            (84)
#define PAGES_PER_BLOCK                 (92)
#define BLOCKS_PER_LUN                  (96)
#define NUM_OF_LUNS                     (100)
#define BAD_BLOCKS_PER_LUN              (103)
#define BLOCK_ENDURANCE                 (105)
#define BLOCK_ENDURANCE_MULTIPLIER      (106)
#define PROG_PER_PAGE                   (110)

/*
    MANUFACTURER_INFO_BLOCK : SUB Defenitions
*/
#define DEVICE_MANUFACTURER             (32)
#define DEVICE_MODEL                    (44)
#define JEDEC_MANUFACTURER_ID           (64)

/*
    REVISION_NUMBER : Bit Defenitions
*/
#define REV_NUM_ONFI_2_3                (0x20)
#define REV_NUM_ONFI_2_2                (0x10)
#define REV_NUM_ONFI_2_1                (0x08)
#define REV_NUM_ONFI_2_0                (0x04)
#define REV_NUM_ONFI_1_0                (0x02)

/*
    FEATURES_SUPPORTED : Bit Defenitions
*/
#define FEATURE_EZNAND_SUPPORT              (0x0200)
#define FEATURE_PAGE_REGISTER_CLEAR         (0x0100)

#define FEATURE_EXTENDED_PARAM_PAGE         (0x0080)
#define FEATURE_MULTI_PLANE_READ            (0x0040)
#define FEATURE_SOURCE_SYNCH                (0x0020)
#define FEATURE_ODD_EVEN_COPY_BACK_SUPPORT  (0x0010)

#define FEATURE_MULTI_PLANE_PROGRAM_ERASE   (0x0008)
#define FEATURE_NON_SEQ_PAGE_PROGRAMMING    (0x0004)
#define FEATURE_MULTIPLE_LUN_SUPPORT        (0x0002)
#define FEATURE_16_BIT_DATA_BUS             (0x0001)

/*---------------------------------------------------------
   Function Prototypes
---------------------------------------------------------*/
FFXSTATUS FfxNandDecodeParameterPage(D_BYTE *ParamPageArr, FFX_PARAMETER_PAGE *pParamPage);
const FFXNANDCHIP *FfxNandFillChipInfo(const D_UCHAR *aucID, const FFX_PARAMETER_PAGE *pParamPage);
#endif  /* NANDPARAM_H_INCLUDED */

