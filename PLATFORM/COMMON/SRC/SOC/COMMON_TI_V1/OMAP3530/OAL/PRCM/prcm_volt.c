// All rights reserved ADENEO EMBEDDED 2010
/*
===============================================================================
*             Texas Instruments OMAP(TM) Platform Software
* (c) Copyright Texas Instruments, Incorporated. All Rights Reserved.
*
* Use of this software is controlled by the terms and conditions found
* in the license agreement under which this software has been supplied.
*
===============================================================================
*/
//
//  File: prcm_volt.c
//

#include "omap.h"
#include "omap_prof.h"
#include "omap3530.h"
#include <nkintr.h>
#include <pkfuncs.h>
#include "oalex.h"
#include "prcm_priv.h"
#include "omap3530_dvfs.h"

//------------------------------------------------------------------------------
void
PrcmVoltSetControlMode(
    UINT                    voltCtrlMode,
    UINT                    voltCtrlMask
    )
{
    UINT val;
    
    voltCtrlMode &= voltCtrlMask;
    val = INREG32(&g_pPrcmPrm->pOMAP_GLOBAL_PRM->PRM_VOLTCTRL) & ~voltCtrlMask;
    val |= voltCtrlMode;
    OUTREG32(&g_pPrcmPrm->pOMAP_GLOBAL_PRM->PRM_VOLTCTRL, val);
}

//------------------------------------------------------------------------------
void
PrcmVoltSetControlPolarity(
    UINT                    polMode,
    UINT                    polModeMask
    )
{    
    UINT val;
    
    polMode &= polModeMask;
    val = INREG32(&g_pPrcmPrm->pOMAP_GLOBAL_PRM->PRM_POLCTRL) & ~polModeMask;
    val |= polMode;
    OUTREG32(&g_pPrcmPrm->pOMAP_GLOBAL_PRM->PRM_POLCTRL, val);
}

//------------------------------------------------------------------------------
void
PrcmVoltSetAutoControl(
    UINT                    autoCtrlMode,
    UINT                    autoCtrlMask
    )
{    
    UINT val;
    
    autoCtrlMode &= autoCtrlMask;
    val = INREG32(&g_pPrcmPrm->pOMAP_GLOBAL_PRM->PRM_VOLTCTRL) & ~autoCtrlMask;
    val |= autoCtrlMode;
    OUTREG32(&g_pPrcmPrm->pOMAP_GLOBAL_PRM->PRM_VOLTCTRL, val);
}

//-----------------------------------------------------------------------------
void
PrcmVoltI2cInitialize(
    VoltageProcessor_e      vp,
    UINT8                   slaveAddr,
    UINT8                   cmdAddr,
    UINT8                   voltAddr,
    BOOL                    bUseCmdAddr
    )
{
    UINT    val;
    
    switch (vp)
        {
        case kVoltageProcessor1:
            // set slave address
            val = INREG32(&g_pPrcmPrm->pOMAP_GLOBAL_PRM->PRM_VC_SMPS_SA);
            val &= ~SMPS_SA0_MASK;
            val |= ((slaveAddr << SMPS_SA0_SHIFT) & SMPS_SA0_MASK);
            OUTREG32(&g_pPrcmPrm->pOMAP_GLOBAL_PRM->PRM_VC_SMPS_SA, val);

            // set voltage register address
            val = INREG32(&g_pPrcmPrm->pOMAP_GLOBAL_PRM->PRM_VC_SMPS_VOL_RA);
            val &= ~SMPS_VOLRA0_MASK;
            val |= ((voltAddr << SMPS_VOLRA0_SHIFT) & SMPS_VOLRA0_MASK);
            OUTREG32(&g_pPrcmPrm->pOMAP_GLOBAL_PRM->PRM_VC_SMPS_VOL_RA, val);

            // set cmd register address
            val = INREG32(&g_pPrcmPrm->pOMAP_GLOBAL_PRM->PRM_VC_SMPS_CMD_RA);
            val &= ~SMPS_CMDRA0_MASK;
            val |= ((cmdAddr << SMPS_CMDRA0_SHIFT) & SMPS_CMDRA0_MASK);
            OUTREG32(&g_pPrcmPrm->pOMAP_GLOBAL_PRM->PRM_VC_SMPS_CMD_RA, val);

            // set mux for voltage control register address            
            val = INREG32(&g_pPrcmPrm->pOMAP_GLOBAL_PRM->PRM_VC_CH_CONF);            
            val &= ~(SMPS_SA0 | SMPS_RAV0 | SMPS_RAC0 | SMPS_CMD0);
            val = (bUseCmdAddr != FALSE) ? val | SMPS_RACEN0 : val & ~SMPS_RACEN0;
            OUTREG32(&g_pPrcmPrm->pOMAP_GLOBAL_PRM->PRM_VC_CH_CONF, val);   
            break;

        case kVoltageProcessor2:
            // set slave address
            val = INREG32(&g_pPrcmPrm->pOMAP_GLOBAL_PRM->PRM_VC_SMPS_SA);
            val &= ~SMPS_SA1_MASK;
            val |= ((slaveAddr << SMPS_SA1_SHIFT) & SMPS_SA1_MASK);
            OUTREG32(&g_pPrcmPrm->pOMAP_GLOBAL_PRM->PRM_VC_SMPS_SA, val);

            // set voltage register address
            val = INREG32(&g_pPrcmPrm->pOMAP_GLOBAL_PRM->PRM_VC_SMPS_VOL_RA);
            val &= ~SMPS_VOLRA1_MASK;
            val |= ((voltAddr << SMPS_VOLRA1_SHIFT) & SMPS_VOLRA1_MASK);
            OUTREG32(&g_pPrcmPrm->pOMAP_GLOBAL_PRM->PRM_VC_SMPS_VOL_RA, val);

            // set cmd register address
            val = INREG32(&g_pPrcmPrm->pOMAP_GLOBAL_PRM->PRM_VC_SMPS_CMD_RA);
            val &= ~SMPS_CMDRA1_MASK;
            val |= ((cmdAddr << SMPS_CMDRA1_SHIFT) & SMPS_CMDRA1_MASK);
            OUTREG32(&g_pPrcmPrm->pOMAP_GLOBAL_PRM->PRM_VC_SMPS_CMD_RA, val);

            // set mux for voltage control register address            
            val = INREG32(&g_pPrcmPrm->pOMAP_GLOBAL_PRM->PRM_VC_CH_CONF);            
            val |= (SMPS_SA1 | SMPS_RAV1 | SMPS_RAC1 | SMPS_CMD1);
            val = (bUseCmdAddr != FALSE) ? val | SMPS_RACEN1 : val & ~SMPS_RACEN1;
            OUTREG32(&g_pPrcmPrm->pOMAP_GLOBAL_PRM->PRM_VC_CH_CONF, val);
            break;
        }
}

//------------------------------------------------------------------------------
void
PrcmVoltI2cSetHighSpeedMode(
    BOOL                    bHSMode,
    BOOL                    bRepeatStartMode,
    UINT8                   mcode
    )
{
    UINT    val;

    val = INREG32(&g_pPrcmPrm->pOMAP_GLOBAL_PRM->PRM_VC_I2C_CFG);

    // build up value
    val &= ~(SMPS_HSEN | SMPS_SREN | SMPS_MCODE_MASK);
    if (bHSMode != FALSE) val |= SMPS_HSEN;
    if (bRepeatStartMode != FALSE) val |= SMPS_SREN;
    val |= ((mcode & SMPS_MCODE_MASK) << SMPS_MCODE_SHIFT);

    // write-up result
    OUTREG32(&g_pPrcmPrm->pOMAP_GLOBAL_PRM->PRM_VC_I2C_CFG, val);
}

//------------------------------------------------------------------------------
void
PrcmVoltInitializeVoltageLevels(
    VoltageProcessor_e      vp,
    UINT                    vddOn,
    UINT                    vddOnLP,
    UINT                    vddRetention,
    UINT                    vddOff
    )
{
    UINT    val;

    val = (vddOn << SMPS_ON_SHIFT);
    val |= (vddOnLP << SMPS_ONLP_SHIFT);
    val |= (vddRetention << SMPS_RET_SHIFT);
    val |= (vddOff << SMPS_OFF_SHIFT);
    
    switch (vp)
        {
        case kVoltageProcessor1:
            OUTREG32(&g_pPrcmPrm->pOMAP_GLOBAL_PRM->PRM_VC_CMD_VAL_0, val);
            break;

        case kVoltageProcessor2:
            OUTREG32(&g_pPrcmPrm->pOMAP_GLOBAL_PRM->PRM_VC_CMD_VAL_1, val);
            break;
        }
}

//------------------------------------------------------------------------------
void
PrcmVoltSetErrorConfiguration(
    VoltageProcessor_e      vp,
    UINT                    errorOffset,
    UINT                    errorGain
    )
{
    UINT val;

    switch (vp)
        {
        case kVoltageProcessor1:
            val = INREG32(&g_pPrcmPrm->pOMAP_GLOBAL_PRM->PRM_VP1_CONFIG);
            val &= ~(SMPS_ERROROFFSET_MASK | SMPS_ERRORGAIN_MASK);
            val |= ((errorGain << SMPS_ERRORGAIN_SHIFT) & SMPS_ERRORGAIN_MASK);
            val |= ((errorOffset << SMPS_ERROROFFSET_SHIFT) & SMPS_ERROROFFSET_MASK);
            OUTREG32(&g_pPrcmPrm->pOMAP_GLOBAL_PRM->PRM_VP1_CONFIG, val);
            break;

        case kVoltageProcessor2:
            val = INREG32(&g_pPrcmPrm->pOMAP_GLOBAL_PRM->PRM_VP2_CONFIG);
            val &= ~(SMPS_ERROROFFSET_MASK | SMPS_ERRORGAIN_MASK);
            val |= ((errorGain << SMPS_ERRORGAIN_SHIFT) & SMPS_ERRORGAIN_MASK);
            val |= ((errorOffset << SMPS_ERROROFFSET_SHIFT) & SMPS_ERROROFFSET_MASK);
            OUTREG32(&g_pPrcmPrm->pOMAP_GLOBAL_PRM->PRM_VP2_CONFIG, val);
            break;
        }    
}

//------------------------------------------------------------------------------
void
PrcmVoltSetSlewRange(
    VoltageProcessor_e      vp,
    UINT                    minVStep,
    UINT                    minWaitTime,
    UINT                    maxVStep,
    UINT                    maxWaitTime
    )
{
    UINT val;

    switch (vp)
        {
        case kVoltageProcessor1:
            // update min slew values            
            val = (minVStep << SMPS_VSTEPMIN_SHIFT) & SMPS_VSTEPMIN_MASK;
            val |= (minWaitTime << SMPS_SMPSWAITTIMEMIN_SHIFT) & SMPS_SMPSWAITTIMEMIN_MASK; 
            OUTREG32(&g_pPrcmPrm->pOMAP_GLOBAL_PRM->PRM_VP1_VSTEPMIN, val);

            // update max slew values
            val = (maxVStep << SMPS_VSTEPMAX_SHIFT) & SMPS_VSTEPMAX_MASK;
            val |= (maxWaitTime << SMPS_SMPSWAITTIMEMAX_SHIFT) & SMPS_SMPSWAITTIMEMAX_MASK; 
            OUTREG32(&g_pPrcmPrm->pOMAP_GLOBAL_PRM->PRM_VP1_VSTEPMAX, val);
            break;

        case kVoltageProcessor2:
            // update min slew values            
            val = (minVStep << SMPS_VSTEPMIN_SHIFT) & SMPS_VSTEPMIN_MASK;
            val |= (minWaitTime << SMPS_SMPSWAITTIMEMIN_SHIFT) & SMPS_SMPSWAITTIMEMIN_MASK; 
            OUTREG32(&g_pPrcmPrm->pOMAP_GLOBAL_PRM->PRM_VP2_VSTEPMIN, val);

            // update max slew values
            val = (maxVStep << SMPS_VSTEPMAX_SHIFT) & SMPS_VSTEPMAX_MASK;
            val |= (maxWaitTime << SMPS_SMPSWAITTIMEMAX_SHIFT) & SMPS_SMPSWAITTIMEMAX_MASK; 
            OUTREG32(&g_pPrcmPrm->pOMAP_GLOBAL_PRM->PRM_VP2_VSTEPMAX, val);
            break;
        }  
}

//------------------------------------------------------------------------------
void
PrcmVoltSetLimits(
    VoltageProcessor_e      vp,
    UINT                    minVolt,
    UINT                    maxVolt,
    UINT                    timeOut
    )
{
    UINT val;

    switch (vp)
        {
        case kVoltageProcessor1:            
            val = (maxVolt << SMPS_VDDMAX_SHIFT) & SMPS_VDDMAX_MASK;
            val |= (minVolt << SMPS_VDDMIN_SHIFT) & SMPS_VDDMIN_MASK;
            val |= (timeOut << SMPS_TIMEOUT_SHIFT) & SMPS_TIMEOUT_MASK;
            OUTREG32(&g_pPrcmPrm->pOMAP_GLOBAL_PRM->PRM_VP1_VLIMITTO, val);
            break;

        case kVoltageProcessor2:          
            val = (maxVolt << SMPS_VDDMAX_SHIFT) & SMPS_VDDMAX_MASK;
            val |= (minVolt << SMPS_VDDMIN_SHIFT) & SMPS_VDDMIN_MASK;
            val |= (timeOut << SMPS_TIMEOUT_SHIFT) & SMPS_TIMEOUT_MASK;
            OUTREG32(&g_pPrcmPrm->pOMAP_GLOBAL_PRM->PRM_VP2_VLIMITTO, val);
            break;
        }  
}

//------------------------------------------------------------------------------
void
PrcmVoltSetVoltageLevel(
    VoltageProcessor_e      vp,
    UINT                    vdd,
    UINT                    mask
    )
{
    UINT    val;
    volatile unsigned int *pPRM_VC_CMD_VAL;

    // get register to modify
    switch (vp)
        {
        case kVoltageProcessor1:
            pPRM_VC_CMD_VAL = &g_pPrcmPrm->pOMAP_GLOBAL_PRM->PRM_VC_CMD_VAL_0;
            break;

        case kVoltageProcessor2:
            pPRM_VC_CMD_VAL = &g_pPrcmPrm->pOMAP_GLOBAL_PRM->PRM_VC_CMD_VAL_1;
            break;

        default:
            return;
        }

    // update register with appropriate voltage level
    val = INREG32(pPRM_VC_CMD_VAL);
    switch (mask)
        {
        case SMPS_ON_MASK:
            val &= ~SMPS_ON_MASK;
            val |= ((vdd << SMPS_ON_SHIFT) & SMPS_ON_MASK);
            break;

        case SMPS_ONLP_MASK:
            val &= ~SMPS_ONLP_MASK;
            val |= ((vdd << SMPS_ONLP_SHIFT) & SMPS_ONLP_MASK);
            break;

        case SMPS_RET_MASK:
            val &= ~SMPS_RET_MASK;
            val |= ((vdd << SMPS_RET_SHIFT) & SMPS_RET_MASK);
            break;

        case SMPS_OFF_MASK:
            val &= ~SMPS_OFF_MASK;
            val |= ((vdd << SMPS_OFF_SHIFT) & SMPS_OFF_MASK);
            break;
        }

    OUTREG32(pPRM_VC_CMD_VAL, val);
}

//------------------------------------------------------------------------------
void
PrcmVoltSetInitVddLevel(
    VoltageProcessor_e      vp,
    UINT                    initVolt
    )
{
    UINT val;

    switch (vp)
        {
        case kVoltageProcessor1:
            val = INREG32(&g_pPrcmPrm->pOMAP_GLOBAL_PRM->PRM_VP1_CONFIG);
            val &= ~SMPS_INITVOLTAGE_MASK;
            val |= ((initVolt << SMPS_INITVOLTAGE_SHIFT) & SMPS_INITVOLTAGE_MASK);
            OUTREG32(&g_pPrcmPrm->pOMAP_GLOBAL_PRM->PRM_VP1_CONFIG, val);
            break;

        case kVoltageProcessor2:
            val = INREG32(&g_pPrcmPrm->pOMAP_GLOBAL_PRM->PRM_VP2_CONFIG);
            val &= ~SMPS_INITVOLTAGE_MASK;
            val |= ((initVolt << SMPS_INITVOLTAGE_SHIFT) & SMPS_INITVOLTAGE_MASK);
            OUTREG32(&g_pPrcmPrm->pOMAP_GLOBAL_PRM->PRM_VP2_CONFIG, val);
            break;
        }
}

//------------------------------------------------------------------------------
void
PrcmVoltEnableTimeout(
    VoltageProcessor_e      vp,
    BOOL                    bEnable
    )
{
    switch (vp)
        {
        case kVoltageProcessor1:
            if (bEnable != FALSE)
                {
                SETREG32(&g_pPrcmPrm->pOMAP_GLOBAL_PRM->PRM_VP1_CONFIG, SMPS_TIMEOUTEN);
                }
            else
                {
                CLRREG32(&g_pPrcmPrm->pOMAP_GLOBAL_PRM->PRM_VP1_CONFIG, SMPS_TIMEOUTEN);
                }
            break;

        case kVoltageProcessor2:
            if (bEnable != FALSE)
                {
                SETREG32(&g_pPrcmPrm->pOMAP_GLOBAL_PRM->PRM_VP2_CONFIG, SMPS_TIMEOUTEN);
                }
            else
                {
                CLRREG32(&g_pPrcmPrm->pOMAP_GLOBAL_PRM->PRM_VP2_CONFIG, SMPS_TIMEOUTEN);
                }
            break;
        } 
}

//------------------------------------------------------------------------------
void
PrcmVoltEnableVp(
    VoltageProcessor_e      vp,
    BOOL                    bEnable
    )
{
    switch (vp)
        {
        case kVoltageProcessor1:
            if (bEnable != FALSE)
                {
                SETREG32(&g_pPrcmPrm->pOMAP_GLOBAL_PRM->PRM_VP1_CONFIG, SMPS_VPENABLE);
                }
            else
                {
                CLRREG32(&g_pPrcmPrm->pOMAP_GLOBAL_PRM->PRM_VP1_CONFIG, SMPS_VPENABLE);
                }
            break;

        case kVoltageProcessor2:
            if (bEnable != FALSE)
                {
                SETREG32(&g_pPrcmPrm->pOMAP_GLOBAL_PRM->PRM_VP2_CONFIG, SMPS_VPENABLE);
                }
            else
                {
                CLRREG32(&g_pPrcmPrm->pOMAP_GLOBAL_PRM->PRM_VP2_CONFIG, SMPS_VPENABLE);
                }
            break;
        }  
}

//------------------------------------------------------------------------------
void
PrcmVoltFlushVoltageLevels(
    VoltageProcessor_e      vp
    )
{
    switch (vp)
        {
        case kVoltageProcessor1:
            SETREG32(&g_pPrcmPrm->pOMAP_GLOBAL_PRM->PRM_VP1_CONFIG, SMPS_INITVDD | SMPS_FORCEUPDATE);
            CLRREG32(&g_pPrcmPrm->pOMAP_GLOBAL_PRM->PRM_VP1_CONFIG, SMPS_INITVDD | SMPS_FORCEUPDATE);
            break;

        case kVoltageProcessor2:
            SETREG32(&g_pPrcmPrm->pOMAP_GLOBAL_PRM->PRM_VP2_CONFIG, SMPS_INITVDD | SMPS_FORCEUPDATE);
            CLRREG32(&g_pPrcmPrm->pOMAP_GLOBAL_PRM->PRM_VP2_CONFIG, SMPS_INITVDD | SMPS_FORCEUPDATE);
            break;

        default:
            return;
        }
}

//-----------------------------------------------------------------------------
BOOL
PrcmVoltIdleCheck(
    VoltageProcessor_e      vp
    )
{
    UINT vpStatus;
    BOOL rc = FALSE;

    switch (vp)
        {
        case kVoltageProcessor1:
            vpStatus = INREG32(&g_pPrcmPrm->pOMAP_GLOBAL_PRM->PRM_VP1_STATUS);
            break;

        case kVoltageProcessor2:
            vpStatus = INREG32(&g_pPrcmPrm->pOMAP_GLOBAL_PRM->PRM_VP2_STATUS);
            break;

        default:
            goto cleanUp;
         }

    if (vpStatus & SMPS_VPINIDLE)
        {
        rc = TRUE;
        }

cleanUp:
    return rc;
}

//-----------------------------------------------------------------------------
UINT
PrcmVoltGetVoltageRampDelay(
    VoltageProcessor_e      vp
    )
{
    UINT curVoltSteps = 0;
    UINT newVoltSteps = 0;
    UINT delay = 0;

    switch (vp)
        {
        case kVoltageProcessor1:
            curVoltSteps = INREG32(&g_pPrcmPrm->pOMAP_GLOBAL_PRM->PRM_VP1_VOLTAGE);
            curVoltSteps &= SMPS_VOLTAGE_MASK;
            curVoltSteps >>= SMPS_VOLTAGE_SHIFT;
            
            newVoltSteps = INREG32(&g_pPrcmPrm->pOMAP_GLOBAL_PRM->PRM_VP1_CONFIG);
            newVoltSteps &= SMPS_INITVOLTAGE_MASK;
            newVoltSteps >>= SMPS_INITVOLTAGE_SHIFT;
            break;

        case kVoltageProcessor2:
            curVoltSteps = INREG32(&g_pPrcmPrm->pOMAP_GLOBAL_PRM->PRM_VP2_VOLTAGE);
            curVoltSteps &= SMPS_VOLTAGE_MASK;
            curVoltSteps >>= SMPS_VOLTAGE_SHIFT;

            newVoltSteps = INREG32(&g_pPrcmPrm->pOMAP_GLOBAL_PRM->PRM_VP2_CONFIG);
            newVoltSteps &= SMPS_INITVOLTAGE_MASK;
            newVoltSteps >>= SMPS_INITVOLTAGE_SHIFT;
            break;
        }

    if (newVoltSteps < curVoltSteps)
        {
        goto cleanUp;
        }

    // delay = DeltaVDD / Slew Rate
    // deltaVDD = steps * 12.5mV , Slew Rate - 4 mv/us
    // To avoid floting point operation, formula used - (steps*25)/(slewrate*2)
    delay = ((newVoltSteps-curVoltSteps)*25)>>3;
    
cleanUp:
    return delay;
}


/**
 * PrcmVoltScaleVoltageABB - controls ABB ldo during voltage scaling
 * @target_volt: target voltage determines if ABB ldo is active or bypassed
 *
 * Adaptive Body-Bias is a technique in all OMAP silicon that uses the 45nm
 * process.  ABB can boost voltage in high OPPs for silicon with weak
 * characteristics (forward Body-Bias) as well as lower voltage in low OPPs
 * for silicon with strong characteristics (Reverse Body-Bias).
 *
 * Only Foward Body-Bias for operating at high OPPs is implemented below, per
 * recommendations from silicon team.
 * Reverse Body-Bias for saving power in active cases and sleep cases is not
 * yet implemented.
 */
int PrcmVoltScaleVoltageABB(UINT32 target_opp_no)
{
	UINT32 sr2en_enabled;
	UINT32 timeout;
	int sr2_wtcnt_value;
    UINT32 regVal =0;
    UINT32 tcrr;

	/* calculate SR2_WTCNT_VALUE settling time */
	sr2_wtcnt_value = (ABB_MAX_SETTLING_TIME * (PrcmClockGetClockRate(SYS_CLK)) / 8);

	/* has SR2EN been enabled previously? */
	sr2en_enabled = INREG32(&g_pPrcmPrm->pOMAP_GLOBAL_PRM->PRM_LDO_ABB_CTRL) & SMPS_SR2EN;

    // enable voltage processor
    PrcmVoltEnableVp(kVDD1, TRUE);

	/* select fast, nominal or slow OPP for ABB ldo */
	if (target_opp_no >= ABB_FAST_OPP_VAL) {
        
		/* program for fast opp - enable FBB */
        regVal = INREG32(&g_pPrcmPrm->pOMAP_GLOBAL_PRM->PRM_LDO_ABB_SETUP);
        regVal = (regVal & ~(SMPS_OPP_SEL_MASK)) | (ABB_FAST_OPP << SMPS_OPP_SEL_SHIFT);
        OUTREG32(&g_pPrcmPrm->pOMAP_GLOBAL_PRM->PRM_LDO_ABB_SETUP,regVal);
        
		/* enable the ABB ldo if not done already */
		if (!sr2en_enabled)
			SETREG32(&g_pPrcmPrm->pOMAP_GLOBAL_PRM->PRM_LDO_ABB_CTRL,SMPS_SR2EN);
        
	} else if (sr2en_enabled) {
	
		/* program for nominal opp - bypass ABB ldo */        
        regVal = INREG32(&g_pPrcmPrm->pOMAP_GLOBAL_PRM->PRM_LDO_ABB_SETUP);
        regVal = (regVal & ~(SMPS_OPP_SEL_MASK)) | (ABB_NOMINAL_OPP << SMPS_OPP_SEL_SHIFT);
        OUTREG32(&g_pPrcmPrm->pOMAP_GLOBAL_PRM->PRM_LDO_ABB_SETUP,regVal);
        
	} else {
	
		/* nothing to do here yet... might enable RBB here someday */
		goto cleanUp;
        
	}

	/* set ACTIVE_FBB_SEL for all 45nm silicon */
    SETREG32(&g_pPrcmPrm->pOMAP_GLOBAL_PRM->PRM_LDO_ABB_CTRL,SMPS_ACTIVE_FBB_SEL);

	/* program settling time of 30us for ABB ldo transition */
    regVal = INREG32(&g_pPrcmPrm->pOMAP_GLOBAL_PRM->PRM_LDO_ABB_CTRL);
    regVal = (regVal & ~(SMPS_SR2_WTCNT_VALUE_MASK)) | (sr2_wtcnt_value << SMPS_SR2_WTCNT_VALUE_SHIFT);
    OUTREG32(&g_pPrcmPrm->pOMAP_GLOBAL_PRM->PRM_LDO_ABB_CTRL,regVal);
    
	/* clear ABB ldo interrupt status */    
    OUTREG32(&g_pPrcmPrm->pOMAP_OCP_SYSTEM_PRM->PRM_IRQSTATUS_MPU, PRM_IRQENABLE_ABB_LDO_TRANXDONE_ST);
    	
	/* enable ABB LDO OPP change */
    SETREG32(&g_pPrcmPrm->pOMAP_GLOBAL_PRM->PRM_LDO_ABB_SETUP,SMPS_OPP_CHANGE);

	
	/* wait until OPP change completes */
    timeout=tcrr=OALTimerGetCount();
    while (((INREG32(&g_pPrcmPrm->pOMAP_OCP_SYSTEM_PRM->PRM_IRQSTATUS_MPU) & 
			    PRM_IRQENABLE_ABB_LDO_TRANXDONE_ST) == 0) &&
        ((timeout - tcrr) < ABB_MAX_SETTLING_TIME_IN_TICKS))
    {
        timeout=OALTimerGetCount();        
    }

	if (timeout >= tcrr+ABB_MAX_SETTLING_TIME_IN_TICKS)
		RETAILMSG(0,(L"ABB: TRANXDONE timed out waiting for OPP change\r\n"));
    
    timeout=tcrr=OALTimerGetCount();
	
	/* Clear all pending TRANXDONE interrupts/status */
	while ((timeout - tcrr) < ABB_MAX_SETTLING_TIME_IN_TICKS) {
		OUTREG32(&g_pPrcmPrm->pOMAP_OCP_SYSTEM_PRM->PRM_IRQSTATUS_MPU, PRM_IRQENABLE_ABB_LDO_TRANXDONE_ST);
		if (!(INREG32(&g_pPrcmPrm->pOMAP_OCP_SYSTEM_PRM->PRM_IRQSTATUS_MPU) & 
			    PRM_IRQENABLE_ABB_LDO_TRANXDONE_ST))
			break;

		timeout=OALTimerGetCount();  
	}
	if (timeout >= tcrr+ABB_MAX_SETTLING_TIME_IN_TICKS)
		RETAILMSG(0,(L"ABB: TRANXDONE timed out trying to clear status\n"));

cleanUp:
    // disable the voltage processor sub-chip
    PrcmVoltEnableVp(kVDD1, FALSE);
	return 0;
}


