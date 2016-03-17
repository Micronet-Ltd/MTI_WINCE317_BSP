/*
================================================================================
*             Texas Instruments OMAP(TM) Platform Software
* (c) Copyright Texas Instruments, Incorporated. All Rights Reserved.
*
* Use of this software is controlled by the terms and conditions found
* in the license agreement under which this software has been supplied.
*
================================================================================
*/
//
// oal_cpuver.c
//
#pragma warning(push)
#pragma warning(disable: 4115)
#include <windows.h>
#pragma warning(pop)

#include <omap_cpuver.h>
#include "omap3530.h"

chip_info_tag OMAPCpuInfo[]=
{
    /* 3530 */
    { CPU_OMAP3530, HAWKEYE_OMAP35XX, CPU_FAMILY_OMAP35XX, 0x0C00},
    /* 3525 */
    { CPU_OMAP3525, HAWKEYE_OMAP35XX, CPU_FAMILY_OMAP35XX, 0x4C00},
    /* 3515 */
    { CPU_OMAP3515, HAWKEYE_OMAP35XX, CPU_FAMILY_OMAP35XX, 0x1C00},
    /* 3503 */
    { CPU_OMAP3503, HAWKEYE_OMAP35XX, CPU_FAMILY_OMAP35XX, 0x5C00},

    /* 3730 */
    { CPU_DM3730, HAWKEYE_DM37XX, CPU_FAMILY_DM37XX, 0x0C00},
    /* 3725 */
    { CPU_DM3725, HAWKEYE_DM37XX, CPU_FAMILY_DM37XX, 0x4C00},
    /* 3715 */
    { CPU_DM3715, HAWKEYE_DM37XX, CPU_FAMILY_DM37XX, 0x1C00},
    /* 3703 */
    { CPU_DM3703, HAWKEYE_DM37XX, CPU_FAMILY_DM37XX, 0x5C00},
    { 0, 0, 0, 0},
    
};

/* 
    Return Value:
    bit 0-7 -- CPU Revision
    bit 8-15 -- CPU family
    bit 16-31 -- Chip ID
    */
UINT32
Get_CPUVersion(void)
{
    //----------------------------------------------------------------------
    // Determion CPU revison
    //----------------------------------------------------------------------
    UINT32 hawkeye_id, dwOmapFeature;
    UINT8 version;
    UINT32 CpuRevision;
    OMAP_SYSC_GENERAL_REGS * pSyscGeneralRegs = OALPAtoUA(OMAP_SYSC_GENERAL_REGS_PA);
    chip_info_tag *chip_info = OMAPCpuInfo;

    hawkeye_id = (INREG32(OALPAtoUA(OMAP_IDCODE_REGS_PA)) & 0xFFFFF000) ;
    
    switch ( hawkeye_id )
    {
        case 0x0B6D6000:
            CpuRevision = CPU_FAMILY_OMAP35XX_REVISION_ES_1_0;
            version = 0;
        break;

        case 0x0B7AE000:
        case 0x1B7AE000:	// ES2.0 
            CpuRevision = CPU_FAMILY_OMAP35XX_REVISION_ES_2_0;
            version = 1;
        break;

        case 0x2B7AE000:	// ES2.1
            CpuRevision = CPU_FAMILY_OMAP35XX_REVISION_ES_2_1;
            version = 2;
        break;

        case 0x3B7AE000:	// ES3.0
            CpuRevision = CPU_FAMILY_OMAP35XX_REVISION_ES_3_0;
            version = 5;
        break;

        case 0x4B7AE000:
            CpuRevision = CPU_FAMILY_OMAP35XX_REVISION_ES_3_1;
            version = 6;
        break;

        case 0x0B891000:
            CpuRevision = CPU_FAMILY_DM37XX_REVISION_ES_1_0;
            version = 0;
        break;

        case 0x1B891000:
            CpuRevision = CPU_FAMILY_DM37XX_REVISION_ES_1_1;
            version = 1;
        break;

        case 0x2B891000:
            CpuRevision = CPU_FAMILY_DM37XX_REVISION_ES_1_2;
            version = 2;
        break;

        default:
            CpuRevision = (UINT32)CPU_REVISION_UNKNOWN;
            version = 0xFF;
        break;
    }  

#if 0 /* uncomment this code if using older revs of OMAP35XX Silicon */
    if (((CpuRevision >> CPU_FAMILY_SHIFT) & CPU_REVISION_MASK) == CPU_FAMILY_OMAP35XX)
    {
        // Some ES2.1 silicon has incorrectly burned fuses indicating a different revision
        // Read the CRC in public ROM area to distinguish between revisions
        switch (INREG32(OALPAtoUA(PUBLIC_ROM_CRC_PA)))
        {
            case PUBLIC_ROM_CRC_ES2_0:
            // CRC indicates this is ES2.0
            if (version != 1)
            {
                // Id register indicated this was some other rev, indicate ES2.0 with wrong Id
                CpuRevision = CPU_FAMILY_OMAP35XX_REVISION_ES_2_0_CRC;
                version = 3;
            }
            break;
    
            case PUBLIC_ROM_CRC_ES2_1:
            // CRC indicates this is ES2.1
            if (version < 2)
            {
                // Id register indicated this was some other rev, indicate ES2.1 with wrong Id
                CpuRevision = CPU_FAMILY_OMAP35XX_REVISION_ES_2_1_CRC;
                version = 4;
            }
            break;
    
            default:
            break;
        }          
    }
#endif

    /* If only the version is unknown, determine the family and use future for version number */
    if (CpuRevision == (UINT32)CPU_REVISION_UNKNOWN)
    {
        switch (hawkeye_id & 0x0FFFF000)
        {
            case 0x0B6D6000:
            case 0x0B7AE000:
                CpuRevision = CPU_FAMILY_OMAP35XX_REVISION_ES_FUTURE;
            break;

            
            case 0x0B891000:
                CpuRevision = CPU_FAMILY_DM37XX_REVISION_ES_FUTURE;                
            break;            
        }
    }

    /* get chip id  for 3517 chips */
    if((CpuRevision & 0xff00) == (CPU_FAMILY_AM35XX << CPU_FAMILY_SHIFT))
    {
        CpuRevision |= (CPU_AM3517 << CHIP_ID_SHIFT);
    }
    else
    {
        dwOmapFeature = INREG32(&pSyscGeneralRegs->zzzReserved11[0]) & CHIP_FEATURE_MASK;
        while(chip_info->chip_id != 0)    
        {
            if((chip_info->family == CPU_FAMILY(CpuRevision)) &&
                (chip_info->feature == dwOmapFeature)
              )
            {
                 CpuRevision |= (chip_info->chip_id << CHIP_ID_SHIFT);
            }
	     chip_info++;
        }
    }
	
    return (CpuRevision);
}

UINT32 CPUMaxSpeed[4]=
{
    720,   /*CPU_FAMILY_OMAP35XX*/
    1000, /*CPU_FAMILY_DM37XX*/
    720,   /*CPU_FAMILY_AM35XX*/
    0
};

UINT32
Get_CPUMaxSpeed(UINT32 cpu_family)
{
    UINT32 speed=0;
	
    if(VALID_CPU_FAMILY(cpu_family))
    {
        speed = CPUMaxSpeed[cpu_family];
    }
    return (speed);
}
