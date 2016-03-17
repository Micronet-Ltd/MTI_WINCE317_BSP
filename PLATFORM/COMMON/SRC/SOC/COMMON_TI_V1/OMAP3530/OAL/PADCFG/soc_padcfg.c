// All rights reserved ADENEO EMBEDDED 2010
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

#include "omap3530.h"
#include "omap3530_config.h"
#include "sdk_padcfg.h"
#include "soc_cfg.h"

static UINT16* g_pPadCfg = NULL;
static UINT16* g_pWkupPadCfg = NULL;

static _inline void MapPadRegisters()
{
    if (g_pPadCfg == NULL)
    {
        g_pPadCfg = OALPAtoUA(OMAP_SYSC_PADCONFS_REGS_PA);
    }
    
    if (g_pWkupPadCfg == NULL)
    {
        g_pWkupPadCfg = OALPAtoUA(OMAP_SYSC_PADCONFS_WKUP_REGS_PA);
    }
}

BOOL SOCSetPadConfig(UINT16 padId, UINT16 cfg)
{
    UINT16 hwcfg = 0;

    MapPadRegisters();

    
    //Translate config to CONTROL_PADCONF_X format
    hwcfg |= (cfg & INPUT_ENABLED)          ?   INPUT_ENABLE : 0;
    hwcfg |= (cfg & PULL_RESISTOR_ENABLED)  ?   (cfg & PULLUP_RESISTOR) ? PULL_UP : PULL_DOWN : 0;    
    hwcfg |= ((cfg >> 3) & 0x7) << 0;
    
    //write CONTROL_PADCONF_X. don't worry about the offset it has been checked by upper layers
    if (padId < FIRST_WKUP_PAD_INDEX)
    {
        OUTREG16(&g_pPadCfg[padId],hwcfg);   
    }
    else
    {
        OUTREG16(&g_pWkupPadCfg[padId-FIRST_WKUP_PAD_INDEX],hwcfg);  
    }

    return TRUE;
}
BOOL SOCGetPadConfig(UINT16 padId, UINT16* pCfg)
{
    UINT16 hwcfg;
    UINT16 cfg = 0;    

    MapPadRegisters();

    //read CONTROL_PADCONF_X. don't worry about the offset it has been checked by upper layers
    if (padId < FIRST_WKUP_PAD_INDEX)
    {
        hwcfg = INREG16(&g_pPadCfg[padId]);   
    }
    else
    {
        hwcfg = INREG16(&g_pWkupPadCfg[padId-FIRST_WKUP_PAD_INDEX]);  
    }
    //Translate CONTROL_PADCONF_X format to the API format
    cfg |= (hwcfg & INPUT_ENABLE) ? INPUT_ENABLED : 0;

    switch (hwcfg & PULL_UP)
    {
    case PULL_UP:
        cfg |= PULL_RESISTOR_ENABLED | PULLUP_RESISTOR;
        break;
    case PULL_DOWN:
        cfg |= PULL_RESISTOR_ENABLED | PULLDOWN_RESISTOR;
        break;
    case PULL_INACTIVE:
        cfg |= PULL_RESISTOR_DISABLED;
        break;
    }

    cfg |= ((hwcfg >> 0) & 0x7) << 3;

    *pCfg = cfg;

    return TRUE;
}

//------------------------------------------------------------------------------

