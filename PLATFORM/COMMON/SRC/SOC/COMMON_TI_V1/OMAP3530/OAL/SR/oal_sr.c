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
//  File:  oal_sr.c
//
#include "omap.h"
#include "omap3530.h"
#include "oalex.h"
#include "oal_sr.h"

//-----------------------------------------------------------------------------
//
//  Global:  g_pSr
//
//  Reference to SmartReflex registers. Initialized in 
//  SmartReflex_InitializeChannel
//
static OMAP_SMARTREFLEX    *g_pSr[kSmartReflex_ChannelCount];


//-----------------------------------------------------------------------------
void
SmartReflex_InitializeChannel(
    SmartReflexChannel_e    channel,
    OMAP_SMARTREFLEX       *pSr
    )
{
    g_pSr[channel] = pSr;
}

//------------------------------------------------------------------------------
void
SmartReflex_SetAccumulationSize(
    SmartReflexChannel_e    channel,
    UINT                    accumData
    )
{
    UINT                val;
    OMAP_SMARTREFLEX   *pSr = g_pSr[channel];

    val = INREG32(&pSr->sr_regs.omap_35xx.SRCONFIG);
    val &= ~(SRCONFIG_ACCUMDATA_MASK);
    val |= ((accumData << SRCONFIG_ACCUMDATA_SHIFT) & SRCONFIG_ACCUMDATA_MASK);
    OUTREG32(&pSr->sr_regs.omap_35xx.SRCONFIG, val);
}

//------------------------------------------------------------------------------
void
SmartReflex_SetSensorMode_v1(
    SmartReflexChannel_e    channel,
    UINT                    senNMode,
    UINT                    senPMode
    )
{
    UINT                val;
    OMAP_SMARTREFLEX   *pSr = g_pSr[channel];

    UNREFERENCED_PARAMETER(senPMode);

    val = INREG32(&pSr->sr_regs.omap_35xx.SRCONFIG);
    val &= ~(SRCONFIG_SENPENABLE_MASK | SRCONFIG_SENNENABLE_MASK);
    val |= ((senNMode << SRCONFIG_SENNENABLE_SHIFT) & SRCONFIG_SENNENABLE_MASK);
    val |= ((senPMode << SRCONFIG_SENPENABLE_SHIFT) & SRCONFIG_SENPENABLE_MASK);
    OUTREG32(&pSr->sr_regs.omap_35xx.SRCONFIG, val);
}

//------------------------------------------------------------------------------
void
SmartReflex_SetSensorMode_v2(
    SmartReflexChannel_e    channel,
    UINT                    senNMode,
    UINT                    senPMode
    )
{
    UINT                val;
    OMAP_SMARTREFLEX   *pSr = g_pSr[channel];

    val = INREG32(&pSr->sr_regs.omap_37xx.SRCONFIG);
	
    val &= ~(SRCONFIG_SENPENABLE_MASK_37XX | SRCONFIG_SENNENABLE_MASK_37XX);
    val |= ((senNMode << SRCONFIG_SENNENABLE_SHIFT_37XX) & SRCONFIG_SENNENABLE_MASK_37XX);
    val |= ((senPMode << SRCONFIG_SENPENABLE_SHIFT_37XX) & SRCONFIG_SENPENABLE_MASK_37XX);
    OUTREG32(&pSr->sr_regs.omap_37xx.SRCONFIG, val);
}

//------------------------------------------------------------------------------
void
SmartReflex_EnableInterrupts_v1(
    SmartReflexChannel_e    channel,    
    UINT                    mask,
    BOOL                    bEnable
    )
{
    OMAP_SMARTREFLEX *pSr = g_pSr[channel];

    if (bEnable != FALSE)
        {
        SETREG32(&pSr->sr_regs.omap_35xx.ERRCONFIG, mask);
        }
    else
        {
        CLRREG32(&pSr->sr_regs.omap_35xx.ERRCONFIG, mask);
        }
}

//------------------------------------------------------------------------------
void
SmartReflex_EnableInterrupts_v2(
    SmartReflexChannel_e    channel,    
    UINT                    mask,
    BOOL                    bEnable
    )
{
    OMAP_SMARTREFLEX *pSr = g_pSr[channel];
    UINT32 *pReg, new_mask;

    if(mask & ERRCONFIG_VP_BOUNDINT_EN)
    {
        pReg = (UINT32 *)&pSr->sr_regs.omap_37xx.ERRCONFIG;
        new_mask = ERRCONFIG_VP_BOUNDINT_EN_37XX;
	 if (bEnable != FALSE)
	 {
	    SETREG32(pReg, new_mask);
	 }
	 else
	 {
	    CLRREG32(pReg, new_mask);
	 }
    }
    else
    {
        pReg = 	(UINT32 *)&pSr->sr_regs.omap_37xx.IRQENABLE_SET;
        switch (mask)
        {
            case ERRCONFIG_MCU_DISACKINT_EN:
                mask = IRQENABLE_SET_MCUDISABLEACKINTSTATENA;
                break;
            case ERRCONFIG_MCU_BOUNDINT_EN:
                mask = IRQENABLE_SET_MCUBOUNDSINTENA;
                break;
            case ERRCONFIG_MCU_VALIDINT_EN:
                mask = IRQENABLE_SET_MCUVALIDINTENA;
                break;
            case ERRCONFIG_MCU_ACCUMINT_EN:
                mask = IRQENABLE_SET_MCUACCUMINTENA;
                break;
            case ERRCONFIG_INTR_SR_MASK:
		  mask = IRQENABLE_MASK;
	     default:
                mask = 0;
                break;
        }
	 if (bEnable != FALSE)
	 {
	    SETREG32(pReg, mask);
	 }
	 else
	 {
	    CLRREG32(pReg, mask);
	 }
		
    }

}

//------------------------------------------------------------------------------
void
SmartReflex_EnableErrorGeneratorBlock(
    SmartReflexChannel_e    channel,
    BOOL                    bEnable
    )
{
    OMAP_SMARTREFLEX *pSr = g_pSr[channel];
    
    if (bEnable != FALSE)
        {
        SETREG32(&pSr->sr_regs.omap_35xx.SRCONFIG, SRCONFIG_ERRORGEN_EN);
        }
    else
        {
        CLRREG32(&pSr->sr_regs.omap_35xx.SRCONFIG, SRCONFIG_ERRORGEN_EN);
        }
}

//------------------------------------------------------------------------------
void
SmartReflex_EnableMinMaxAvgBlock(
    SmartReflexChannel_e    channel,
    BOOL                    bEnable
    )
{
    OMAP_SMARTREFLEX *pSr = g_pSr[channel];
    
    if (bEnable != FALSE)
        {
        SETREG32(&pSr->sr_regs.omap_35xx.SRCONFIG, SRCONFIG_MINMAXAVG_EN);
        }
    else
        {
        CLRREG32(&pSr->sr_regs.omap_35xx.SRCONFIG, SRCONFIG_MINMAXAVG_EN);
        }
}

//------------------------------------------------------------------------------
void
SmartReflex_EnableSensorBlock(
    SmartReflexChannel_e    channel,
    BOOL                    bEnable
    )
{
    OMAP_SMARTREFLEX *pSr = g_pSr[channel];
    
    if (bEnable != FALSE)
        {
        SETREG32(&pSr->sr_regs.omap_35xx.SRCONFIG, SRCONFIG_SENENABLE);
        }
    else
        {
        CLRREG32(&pSr->sr_regs.omap_35xx.SRCONFIG, SRCONFIG_SENENABLE);
        }
}

//------------------------------------------------------------------------------
void
SmartReflex_Enable(
    SmartReflexChannel_e    channel,
    BOOL                    bEnable
    )
{
    OMAP_SMARTREFLEX *pSr = g_pSr[channel];
    
    if (bEnable != FALSE)
        {
        SETREG32(&pSr->sr_regs.omap_35xx.SRCONFIG, SRCONFIG_SRENABLE);
        }
    else
        {
        CLRREG32(&pSr->sr_regs.omap_35xx.SRCONFIG, SRCONFIG_SRENABLE);
        }
}

//------------------------------------------------------------------------------
void 
SmartReflex_SetIdleMode_v1(
    SmartReflexChannel_e    channel,
    UINT                    idleMode
    )
{
    UINT              val;
    OMAP_SMARTREFLEX *pSr = g_pSr[channel];

    val = INREG32(&pSr->sr_regs.omap_35xx.ERRCONFIG);
    val &= ~ERRCONFIG_CLKACTIVITY_MASK;
    val |= ((idleMode << ERRCONFIG_CLKACTIVITY_SHIFT) & ERRCONFIG_CLKACTIVITY_MASK);
    OUTREG32(&pSr->sr_regs.omap_35xx.ERRCONFIG, val);
}
//------------------------------------------------------------------------------
void 
SmartReflex_SetIdleMode_v2(
    SmartReflexChannel_e    channel,
    UINT                    idleMode
    )
{
    UINT              val;
    OMAP_SMARTREFLEX *pSr = g_pSr[channel];

    val = INREG32(&pSr->sr_regs.omap_37xx.ERRCONFIG);
    val &= ~ERRCONFIG_CLKACTIVITY_MASK_37XX;
    val |= ((idleMode << ERRCONFIG_CLKACTIVITY_SHIFT_37XX) & ERRCONFIG_CLKACTIVITY_MASK_37XX);
	
    OUTREG32(&pSr->sr_regs.omap_37xx.ERRCONFIG, val);
}

//------------------------------------------------------------------------------
void
SmartReflex_SetSensorReferenceData(
    SmartReflexChannel_e     channel,
    SmartReflexSensorData_t *pSensorData    
    )
{
    UINT              val;
    UINT              srClkLengthT = pSensorData->srClkLengthT;
    UINT              rnsenp = pSensorData->rnsenp;
    UINT              rnsenn = pSensorData->rnsenn;
    UINT              senpgain = pSensorData->senpgain;
    UINT              senngain = pSensorData->senngain;
    OMAP_SMARTREFLEX *pSr = g_pSr[channel];

    // update smartreflex clk sampling frequency
    val = INREG32(&pSr->sr_regs.omap_35xx.SRCONFIG);
    val &= ~SRCONFIG_SRCLKLENGTH_MASK;
    val |= ((srClkLengthT << SRCONFIG_SRCLKLENGTH_SHIFT) & SRCONFIG_SRCLKLENGTH_MASK);
    OUTREG32(&pSr->sr_regs.omap_35xx.SRCONFIG , val);

    // set scale value for SenN, SenP, SenNGain, SenPGain
    val = ((rnsenn << NVALUERECIPROCAL_RNSENN_SHIFT) & NVALUERECIPROCAL_RNSENN_MASK);
    val |= ((rnsenp << NVALUERECIPROCAL_RNSENP_SHIFT) & NVALUERECIPROCAL_RNSENP_MASK);
    val |= ((senngain << NVALUERECIPROCAL_SENNGAIN_SHIFT) & NVALUERECIPROCAL_SENNGAIN_MASK);
    val |= ((senpgain << NVALUERECIPROCAL_SENPGAIN_SHIFT) & NVALUERECIPROCAL_SENPGAIN_MASK);
    OUTREG32(&pSr->sr_regs.omap_35xx.NVALUERECIPROCAL, val);
}

//------------------------------------------------------------------------------
void
SmartReflex_SetErrorControl_v1(
    SmartReflexChannel_e    channel,
    UINT                    errorWeight,
    UINT                    errorMax,
    UINT                    errorMin
    )
{
    UINT              val;
    OMAP_SMARTREFLEX *pSr = g_pSr[channel];

    val = INREG32(&pSr->sr_regs.omap_35xx.ERRCONFIG);
    val &= ~(ERRCONFIG_ERRMAXLIMIT_MASK | ERRCONFIG_ERRMINLIMIT_MASK | ERRCONFIG_ERRWEIGHT_MASK);
    val |= ((errorWeight << ERRCONFIG_ERRWEIGHT_SHIFT) & ERRCONFIG_ERRWEIGHT_MASK);
    val |= ((errorMax << ERRCONFIG_ERRMAXLIMIT_SHIFT) & ERRCONFIG_ERRMAXLIMIT_MASK);
    val |= ((errorMin << ERRCONFIG_ERRMINLIMIT_SHIFT) & ERRCONFIG_ERRMINLIMIT_MASK);
    OUTREG32(&pSr->sr_regs.omap_35xx.ERRCONFIG, val);
}

//------------------------------------------------------------------------------
void
SmartReflex_SetErrorControl_v2(
    SmartReflexChannel_e    channel,
    UINT                    errorWeight,
    UINT                    errorMax,
    UINT                    errorMin
    )
{
    UINT              val;
    OMAP_SMARTREFLEX *pSr = g_pSr[channel];

    val = INREG32(&pSr->sr_regs.omap_37xx.ERRCONFIG);
    val &= ~(ERRCONFIG_ERRMAXLIMIT_MASK | ERRCONFIG_ERRMINLIMIT_MASK | ERRCONFIG_ERRWEIGHT_MASK);
    val |= ((errorWeight << ERRCONFIG_ERRWEIGHT_SHIFT) & ERRCONFIG_ERRWEIGHT_MASK);
    val |= ((errorMax << ERRCONFIG_ERRMAXLIMIT_SHIFT) & ERRCONFIG_ERRMAXLIMIT_MASK);
    val |= ((errorMin << ERRCONFIG_ERRMINLIMIT_SHIFT) & ERRCONFIG_ERRMINLIMIT_MASK);
	
    OUTREG32(&pSr->sr_regs.omap_37xx.ERRCONFIG, val);
}

//------------------------------------------------------------------------------
void
SmartReflex_SetAvgWeight(
    SmartReflexChannel_e    channel,
    UINT                    senNWeight,
    UINT                    senPWeight
    )
{
    UINT              val;
    OMAP_SMARTREFLEX *pSr = g_pSr[channel];

    val = INREG32(&pSr->sr_regs.omap_35xx.AVGWEIGHT);
    val &= ~(AVGWEIGHT_SENNAVGWEIGHT_MASK | AVGWEIGHT_SENPAVGWEIGHT_MASK);
    val |= ((senNWeight << AVGWEIGHT_SENNAVGWEIGHT_SHIFT) & AVGWEIGHT_SENNAVGWEIGHT_MASK);
    val |= ((senPWeight << AVGWEIGHT_SENPAVGWEIGHT_SHIFT) & AVGWEIGHT_SENPAVGWEIGHT_MASK);
	
    OUTREG32(&pSr->sr_regs.omap_35xx.AVGWEIGHT, val);
}

//------------------------------------------------------------------------------
UINT32
SmartReflex_ClearInterruptStatus_v1(
    SmartReflexChannel_e    channel,
    UINT                    mask
    )
{
    UINT              rc;
    UINT              intrStatus;
    UINT              val;
    OMAP_SMARTREFLEX *pSr = g_pSr[channel];

    // clear only the interrupt status in the mask
    rc = INREG32(&pSr->sr_regs.omap_35xx.ERRCONFIG);

    // Get the non interrupt status part of ERRCONFIG register
    val = rc & ~ERRCONFIG_INTR_SR_MASK;

    // Get the interrupt status bit to be cleared
    mask &= ERRCONFIG_INTR_SR_MASK;
    intrStatus = rc & mask;

    // Clear the interrupt status
    val |= intrStatus;
    OUTREG32(&pSr->sr_regs.omap_35xx.ERRCONFIG, val);

    // return the status prior to clearing the status
    return rc & ERRCONFIG_INTR_SR_MASK;
}

//------------------------------------------------------------------------------
UINT32
SmartReflex_ClearInterruptStatus_v2(
    SmartReflexChannel_e    channel,
    UINT                    mask
    )
{
    UINT              new_mask, rc=0;
    UINT              intrStatus;
    UINT              val, reg_mask=0;
    OMAP_SMARTREFLEX *pSr = g_pSr[channel];
    UINT32 *pReg=NULL;

    if(mask & ERRCONFIG_VP_BOUNDINT_ST)
    {
        pReg = 	(UINT32 *)&pSr->sr_regs.omap_37xx.ERRCONFIG;
        reg_mask = ERRCONFIG_VP_BOUNDINT_ST_37XX;
        new_mask = ERRCONFIG_VP_BOUNDINT_ST_37XX;
	 // clear only the interrupt status in the mask
	 rc = INREG32(pReg);

	 // Get the non interrupt status part of ERRCONFIG register
	 val = rc & ~reg_mask;
         // Get the interrupt status bit to be cleared
	 intrStatus = rc & new_mask;

	 // Clear the interrupt status
	 val |= intrStatus;
	 OUTREG32(pReg, val);
    }
    if(mask & ERRCONFIG_INTR_SR_MASK)
    {
        pReg = 	(UINT32 *)&pSr->sr_regs.omap_37xx.IRQENABLE_CLR;
        reg_mask = IRQENABLE_MASK;
        switch (mask)
        {
            case ERRCONFIG_MCU_DISACKINT_ST:
                mask = IRQENABLE_CLR_MCUDISABLEACKINTSTATENA;
                break;
            case ERRCONFIG_MCU_BOUNDINT_ST:
                mask = IRQENABLE_CLR_MCUVALIDINTENA;
                break;
            case ERRCONFIG_MCU_VALIDINT_ST:
                mask = IRQENABLE_CLR_MCUVALIDINTENA;
                break;
            case ERRCONFIG_MCU_ACCUMINT_ST:
                mask = IRQENABLE_CLR_MCUACCUMINTENA;
                break;
            case ERRCONFIG_INTR_SR_MASK:
                mask = IRQENABLE_MASK;
	     default:
                mask = 0;
                break;
        }
		
    }

    if((pReg != NULL) && (reg_mask))
    {
        // clear only the interrupt status in the mask
	 rc = INREG32(pReg);

	 // Get the non interrupt status part of ERRCONFIG register
	 val = rc & ~reg_mask;

	 // Get the interrupt status bit to be cleared
	 mask &= reg_mask;
	 intrStatus = rc & mask;

        // Clear the interrupt status
	 val |= intrStatus;
	 OUTREG32(pReg, val);
    }
    // return the status prior to clearing the status
    return (rc & reg_mask);
}

//------------------------------------------------------------------------------
UINT32
SmartReflex_GetStatus(
    SmartReflexChannel_e    channel
    )
{
    return INREG32(&g_pSr[channel]->sr_regs.omap_35xx.SRSTATUS);
}

void  
SmartReflex_dump_register_v1(
    SmartReflexChannel_e channel
    )
{
#ifndef SHIP_BUILD
    OMAP_SMARTREFLEX *pSr = g_pSr[channel];
	
    OALMSG( 1, (L"+SmartReflex register DUMP:\r\n"));

    OALMSG( 1, (L"\tSRCONFIG:\t\t%x\r\n", pSr->sr_regs.omap_35xx.SRCONFIG));
    OALMSG( 1, (L"\tSRSTATUS:\t\t%x\r\n", pSr->sr_regs.omap_35xx.SRSTATUS));
    OALMSG( 1, (L"\tSENVAL:\t\t%x\r\n", pSr->sr_regs.omap_35xx.SENVAL));
    OALMSG( 1, (L"\tSENMIN:\t\t%x\r\n", pSr->sr_regs.omap_35xx.SENMIN));
    OALMSG( 1, (L"\tSENMAX:\t\t%x\r\n", pSr->sr_regs.omap_35xx.SENMAX));
    OALMSG( 1, (L"\tSENAVG:\t\t%x\r\n", pSr->sr_regs.omap_35xx.SENAVG));
    OALMSG( 1, (L"\tAVGWEIGHT:\t\t%x\r\n", pSr->sr_regs.omap_35xx.AVGWEIGHT));
    OALMSG( 1, (L"\tNVALUERECIPROCAL:\t\t%x\r\n", pSr->sr_regs.omap_35xx.NVALUERECIPROCAL));
    OALMSG( 1, (L"\tSENERROR:\t\t%x\r\n", pSr->sr_regs.omap_35xx.SENERROR));
    OALMSG( 1, (L"\tERRCONFIG:\t\t%x\r\n", pSr->sr_regs.omap_35xx.ERRCONFIG));
#else
	UNREFERENCED_PARAMETER(channel);
#endif
}
void  
SmartReflex_dump_register_v2(
    SmartReflexChannel_e channel
    )
{
#ifndef SHIP_BUILD
    OMAP_SMARTREFLEX *pSr = g_pSr[channel];
	
    OALMSG( 1, (L"+SmartReflex register DUMP:\r\n"));

    OALMSG( 1, (L"\tSRCONFIG:\t\t%x\r\n", pSr->sr_regs.omap_37xx.SRCONFIG));
    OALMSG( 1, (L"\tSRSTATUS:\t\t%x\r\n", pSr->sr_regs.omap_37xx.SRSTATUS));
    OALMSG( 1, (L"\tSENVAL:\t\t%x\r\n", pSr->sr_regs.omap_37xx.SENVAL));
    OALMSG( 1, (L"\tSENMIN:\t\t%x\r\n", pSr->sr_regs.omap_37xx.SENMIN));
    OALMSG( 1, (L"\tSENMAX:\t\t%x\r\n", pSr->sr_regs.omap_37xx.SENMAX));
    OALMSG( 1, (L"\tSENAVG:\t\t%x\r\n", pSr->sr_regs.omap_37xx.SENAVG));
    OALMSG( 1, (L"\tAVGWEIGHT:\t\t%x\r\n", pSr->sr_regs.omap_37xx.AVGWEIGHT));
    OALMSG( 1, (L"\tNVALUERECIPROCAL:\t\t%x\r\n", pSr->sr_regs.omap_37xx.NVALUERECIPROCAL));
    OALMSG( 1, (L"\tIRQSTATUS_RAW:\t\t%x\r\n", pSr->sr_regs.omap_37xx.IRQSTATUS_RAW));
    OALMSG( 1, (L"\tIRQSTATUS:\t\t%x\r\n", pSr->sr_regs.omap_37xx.IRQSTATUS));
    OALMSG( 1, (L"\tIRQENABLE_SET:\t\t%x\r\n", pSr->sr_regs.omap_37xx.IRQENABLE_SET));
    OALMSG( 1, (L"\tIRQENABLE_CLR:\t\t%x\r\n", pSr->sr_regs.omap_37xx.IRQENABLE_CLR));
    OALMSG( 1, (L"\tSENERROR:\t\t%x\r\n", pSr->sr_regs.omap_37xx.SENERROR));
    OALMSG( 1, (L"\tERRCONFIG:\t\t%x\r\n", pSr->sr_regs.omap_37xx.ERRCONFIG));
#else
	UNREFERENCED_PARAMETER(channel);
#endif
}

SmartReflex_Functions_t SmartReflex_Functions_v1=
{
    &SmartReflex_InitializeChannel,
    &SmartReflex_SetAccumulationSize,
    &SmartReflex_SetSensorMode_v1,
    &SmartReflex_SetIdleMode_v1,
    &SmartReflex_SetSensorReferenceData,
    &SmartReflex_SetErrorControl_v1,
    &SmartReflex_EnableInterrupts_v1,
    &SmartReflex_EnableErrorGeneratorBlock,
    &SmartReflex_EnableMinMaxAvgBlock,
    &SmartReflex_EnableSensorBlock,
    &SmartReflex_Enable,
    &SmartReflex_ClearInterruptStatus_v1,
    &SmartReflex_GetStatus,
    &SmartReflex_SetAvgWeight,
    &SmartReflex_dump_register_v1
};

SmartReflex_Functions_t SmartReflex_Functions_v2=
{
    &SmartReflex_InitializeChannel,
    &SmartReflex_SetAccumulationSize,
    &SmartReflex_SetSensorMode_v2,
    &SmartReflex_SetIdleMode_v2,
    &SmartReflex_SetSensorReferenceData,
    &SmartReflex_SetErrorControl_v2,
    &SmartReflex_EnableInterrupts_v2,
    &SmartReflex_EnableErrorGeneratorBlock,
    &SmartReflex_EnableMinMaxAvgBlock,
    &SmartReflex_EnableSensorBlock,
    &SmartReflex_Enable,
    &SmartReflex_ClearInterruptStatus_v2,
    &SmartReflex_GetStatus,
    &SmartReflex_SetAvgWeight,
    &SmartReflex_dump_register_v2
};


SmartReflex_Functions_t* 
SmartReflex_Get_functions_hander(
    DWORD Version
    )
{
    SmartReflex_Functions_t* pFunc = NULL;

    switch (Version)
    {
        case 1:
            pFunc = &SmartReflex_Functions_v1;
            break;
			
        case 2:
            pFunc = &SmartReflex_Functions_v2;
            break;
			
        default:
            OALMSG( OAL_ERROR, (L"\r\nSmartReflex_Get_functions_hander: Invalid SmartReflex version:%d\r\n", Version));
            break;
    }
    return (pFunc);
}
//------------------------------------------------------------------------------