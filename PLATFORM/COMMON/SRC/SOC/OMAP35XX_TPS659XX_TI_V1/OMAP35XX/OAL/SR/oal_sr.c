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
#include <windows.h>
#include <omap35xx.h>
#include <oal.h>
#include <oalex.h>
#include <oal_sr.h>

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

    val = INREG32(&pSr->SRCONFIG);
    val &= ~(SRCONFIG_ACCUMDATA_MASK);
    val |= ((accumData << SRCONFIG_ACCUMDATA_SHIFT) & SRCONFIG_ACCUMDATA_MASK);
    OUTREG32(&pSr->SRCONFIG, val);
}

//------------------------------------------------------------------------------
void
SmartReflex_SetSensorMode(
    SmartReflexChannel_e    channel,
    UINT                    senNMode,
    UINT                    senPMode
    )
{
    UINT                val;
    OMAP_SMARTREFLEX   *pSr = g_pSr[channel];

    val = INREG32(&pSr->SRCONFIG);
    val &= ~(SRCONFIG_SENPENABLE_MASK | SRCONFIG_SENNENABLE_MASK);
    val |= ((senNMode << SRCONFIG_SENNENABLE_SHIFT) & SRCONFIG_SENNENABLE_MASK);
    val |= ((senNMode << SRCONFIG_SENPENABLE_SHIFT) & SRCONFIG_SENPENABLE_MASK);
    OUTREG32(&pSr->SRCONFIG, val);
}

//------------------------------------------------------------------------------
void
SmartReflex_EnableInterrupts(
    SmartReflexChannel_e    channel,    
    UINT                    mask,
    BOOL                    bEnable
    )
{
    OMAP_SMARTREFLEX *pSr = g_pSr[channel];

    if (bEnable != FALSE)
        {
        SETREG32(&pSr->ERRCONFIG, mask);
        }
    else
        {
        CLRREG32(&pSr->ERRCONFIG, mask);
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
        SETREG32(&pSr->SRCONFIG, SRCONFIG_ERRORGEN_EN);
        }
    else
        {
        CLRREG32(&pSr->SRCONFIG, SRCONFIG_ERRORGEN_EN);
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
        SETREG32(&pSr->SRCONFIG, SRCONFIG_MINMAXAVG_EN);
        }
    else
        {
        CLRREG32(&pSr->SRCONFIG, SRCONFIG_MINMAXAVG_EN);
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
        SETREG32(&pSr->SRCONFIG, SRCONFIG_SENENABLE);
        }
    else
        {
        CLRREG32(&pSr->SRCONFIG, SRCONFIG_SENENABLE);
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
        SETREG32(&pSr->SRCONFIG, SRCONFIG_SRENABLE);
        }
    else
        {
        CLRREG32(&pSr->SRCONFIG, SRCONFIG_SRENABLE);
        }
}

//------------------------------------------------------------------------------
void 
SmartReflex_SetIdleMode(
    SmartReflexChannel_e    channel,
    UINT                    idleMode
    )
{
    UINT              val;
    OMAP_SMARTREFLEX *pSr = g_pSr[channel];

    val = INREG32(&pSr->ERRCONFIG);
    val &= ~ERRCONFIG_CLKACTIVITY_MASK;
    val |= ((idleMode << ERRCONFIG_CLKACTIVITY_SHIFT) & ERRCONFIG_CLKACTIVITY_MASK);
    OUTREG32(&pSr->ERRCONFIG, val);
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
    val = INREG32(&pSr->SRCONFIG);
    val &= ~SRCONFIG_SRCLKLENGTH_MASK;
    val |= ((srClkLengthT << SRCONFIG_SRCLKLENGTH_SHIFT) & SRCONFIG_SRCLKLENGTH_MASK);
    OUTREG32(&pSr->SRCONFIG , val);

    // set scale value for SenN, SenP, SenNGain, SenPGain
    val = ((rnsenn << NVALUERECIPROCAL_RNSENN_SHIFT) & NVALUERECIPROCAL_RNSENN_MASK);
    val |= ((rnsenp << NVALUERECIPROCAL_RNSENP_SHIFT) & NVALUERECIPROCAL_RNSENP_MASK);
    val |= ((senngain << NVALUERECIPROCAL_SENNGAIN_SHIFT) & NVALUERECIPROCAL_SENNGAIN_MASK);
    val |= ((senpgain << NVALUERECIPROCAL_SENPGAIN_SHIFT) & NVALUERECIPROCAL_SENPGAIN_MASK);
    OUTREG32(&pSr->NVALUERECIPROCAL, val);
}

//------------------------------------------------------------------------------
void
SmartReflex_SetErrorControl(
    SmartReflexChannel_e    channel,
    UINT                    errorWeight,
    UINT                    errorMax,
    UINT                    errorMin
    )
{
    UINT              val;
    OMAP_SMARTREFLEX *pSr = g_pSr[channel];

    val = INREG32(&pSr->ERRCONFIG);
    val &= ~(ERRCONFIG_ERRMAXLIMIT_MASK | ERRCONFIG_ERRMINLIMIT_MASK | ERRCONFIG_ERRWEIGHT_MASK);
    val |= ((errorWeight << ERRCONFIG_ERRWEIGHT_SHIFT) & ERRCONFIG_ERRWEIGHT_MASK);
    val |= ((errorMax << ERRCONFIG_ERRMAXLIMIT_SHIFT) & ERRCONFIG_ERRMAXLIMIT_MASK);
    val |= ((errorMin << ERRCONFIG_ERRMINLIMIT_SHIFT) & ERRCONFIG_ERRMINLIMIT_MASK);
    OUTREG32(&pSr->ERRCONFIG, val);
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

    val = INREG32(&pSr->AVGWEIGHT);
    val &= ~(AVGWEIGHT_SENNAVGWEIGHT_MASK | AVGWEIGHT_SENPAVGWEIGHT_MASK);
    val |= ((senNWeight << AVGWEIGHT_SENNAVGWEIGHT_SHIFT) & AVGWEIGHT_SENNAVGWEIGHT_MASK);
    val |= ((senPWeight << AVGWEIGHT_SENPAVGWEIGHT_SHIFT) & AVGWEIGHT_SENPAVGWEIGHT_MASK);
    OUTREG32(&pSr->AVGWEIGHT, val);
}

//------------------------------------------------------------------------------
UINT32
SmartReflex_ClearInterruptStatus(
    SmartReflexChannel_e    channel,
    UINT                    mask
    )
{
    UINT              rc;
    UINT              intrStatus;
    UINT              val;
    OMAP_SMARTREFLEX *pSr = g_pSr[channel];

    // clear only the interrupt status in the mask
    rc = INREG32(&pSr->ERRCONFIG);

    // Get the non interrupt status part of ERRCONFIG register
    val = rc & ~ERRCONFIG_INTR_SR_MASK;

    // Get the interrupt status bit to be cleared
    mask &= ERRCONFIG_INTR_SR_MASK;
    intrStatus = rc & mask;

    // Clear the interrupt status
    val |= intrStatus;
    OUTREG32(&pSr->ERRCONFIG, val);

    // return the status prior to clearing the status
    return rc & ERRCONFIG_INTR_SR_MASK;
}

//------------------------------------------------------------------------------
UINT32
SmartReflex_GetStatus(
    SmartReflexChannel_e    channel
    )
{
    return INREG32(&g_pSr[channel]->SRSTATUS);
}

//------------------------------------------------------------------------------