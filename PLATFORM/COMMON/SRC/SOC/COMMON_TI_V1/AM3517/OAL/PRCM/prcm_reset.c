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
//
//  File: prcm_reset.c
//

#include "omap.h"
#include "omap_prof.h"
#include "am3517.h"
#include <nkintr.h>
#include <pkfuncs.h>
#include "oalex.h"
#include "prcm_priv.h"

//------------------------------------------------------------------------------
BOOL
ResetInitialize()
{
    // clear the reset flags for all the power domains
    OUTREG32(&g_pPrcmPrm->pOMAP_CORE_PRM->RM_RSTST_CORE, 
        DOMAINWKUP_RST | GLOBALWARM_RST | GLOBALCOLD_RST
        );
    
    OUTREG32(&g_pPrcmPrm->pOMAP_EMU_PRM->RM_RSTST_EMU,  
        DOMAINWKUP_RST | GLOBALWARM_RST | GLOBALCOLD_RST
        );
    
    OUTREG32(&g_pPrcmPrm->pOMAP_PER_PRM->RM_RSTST_PER, 
        COREDOMAINWKUP_RST | DOMAINWKUP_RST | GLOBALWARM_RST | GLOBALCOLD_RST
        );
    
    OUTREG32(&g_pPrcmPrm->pOMAP_DSS_PRM->RM_RSTST_DSS, 
        COREDOMAINWKUP_RST | DOMAINWKUP_RST | GLOBALWARM_RST | GLOBALCOLD_RST
        );
    
    OUTREG32(&g_pPrcmPrm->pOMAP_NEON_PRM->RM_RSTST_NEON, 
        COREDOMAINWKUP_RST | DOMAINWKUP_RST | GLOBALWARM_RST | GLOBALCOLD_RST
        );
    
    OUTREG32(&g_pPrcmPrm->pOMAP_SGX_PRM->RM_RSTST_SGX, 
        COREDOMAINWKUP_RST | DOMAINWKUP_RST | GLOBALWARM_RST | GLOBALCOLD_RST
        );

    OUTREG32(&g_pPrcmPrm->pOMAP_USBHOST_PRM->RM_RSTST_USBHOST, 
      COREDOMAINWKUP_RST | DOMAINWKUP_RST | GLOBALWARM_RST | GLOBALCOLD_RST
      );

    OUTREG32(&g_pPrcmPrm->pOMAP_MPU_PRM->RM_RSTST_MPU,         
        COREDOMAINWKUP_RST | DOMAINWKUP_RST | GLOBALWARM_RST | GLOBALCOLD_RST |
        EMULATION_MPU_RST
        );
    
    return TRUE;
}

//------------------------------------------------------------------------------
BOOL
PrcmDomainResetStatus(
    UINT    powerDomain,
    UINT   *pResetStatus,
    BOOL    bClear
    )
{
    // get the prm register for the domain
    //
    BOOL rc = FALSE;
    UINT resetStatus;
    OMAP_PRM_REGS *pPrmRegs = GetPrmRegisterSet(powerDomain);
    if (powerDomain == POWERDOMAIN_WAKEUP || pPrmRegs == NULL) goto cleanUp;

    // get current status 
    resetStatus = INREG32(&pPrmRegs->RM_RSTST_xxx);
    if (pResetStatus) *pResetStatus = resetStatus;
    
    // clear status if requested
    if (bClear) OUTREG32(&pPrmRegs->RM_RSTST_xxx, resetStatus);

    rc = TRUE;
    
cleanUp:
    return rc;
}

//------------------------------------------------------------------------------
BOOL
PrcmDomainReset(
    UINT powerDomain,
    UINT resetMask
    )
{
    BOOL            rc = FALSE;

    UNREFERENCED_PARAMETER(powerDomain);
    UNREFERENCED_PARAMETER(resetMask);

    // currently no reset domain supported

    return rc;
}

//------------------------------------------------------------------------------
BOOL
PrcmDomainResetRelease(
    UINT powerDomain,
    UINT resetMask
    )
{
    BOOL            rc = FALSE;
    
    UNREFERENCED_PARAMETER(powerDomain);
    UNREFERENCED_PARAMETER(resetMask);

    // currently no reset domain supported

    return rc;
}


//------------------------------------------------------------------------------
void
PrcmGlobalReset(
    )
{
    OUTREG32(&g_pPrcmPrm->pOMAP_GLOBAL_PRM->PRM_RSTCTRL, RSTCTRL_RST_GS);
}


//------------------------------------------------------------------------------