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
//  File: prcm_clock.c
//
#include "omap.h"
#include "omap_prof.h"
#include "am3517.h"
#include "omap_led.h"
#include <nkintr.h>
#include <pkfuncs.h>
#include "oalex.h"
#include "prcm_priv.h"
#include "interrupt_struct.h"


//-----------------------------------------------------------------------------
// Externs
extern SrcClockMap s_SrcClockTable;

//------------------------------------------------------------------------------
//
//  External:  g_pSysCtrlGenReg
//
//  reference to system control general register set
//
extern OMAP_SYSC_GENERAL_REGS     *g_pSysCtrlGenReg;

//-----------------------------------------------------------------------------
// dpll configuration table
//
static
DpllState_t _dpll_1 = {
    0,  
    DPLL_RAMPTIME_DISABLE >> DPLL_RAMPTIME_SHIFT,  
    DPLL_FREQSEL(1) >> DPLL_FREQSEL_SHIFT,  
    0,  
    DPLL_MODE_LOWPOWER_BYPASS >> DPLL_MODE_SHIFT,  
    DPLL_CLK_SRC(1) >> DPLL_CLK_SRC_SHIFT,    
    DPLL_MULT(0) >> DPLL_MULT_SHIFT,   
    DPLL_DIV(0) >> DPLL_DIV_SHIFT, 
    DPLL_AUTOIDLE_DISABLED >> DPLL_AUTOIDLE_SHIFT,
    1
};

static
DpllState_t _dpll_2 = {
    0,  
    DPLL_RAMPTIME_DISABLE >> DPLL_RAMPTIME_SHIFT,  
    DPLL_FREQSEL(1) >> DPLL_FREQSEL_SHIFT,  
    0,  
    DPLL_MODE_LOWPOWER_STOP >> DPLL_MODE_SHIFT,  
    DPLL_CLK_SRC(1) >> DPLL_CLK_SRC_SHIFT,    
    DPLL_MULT(0) >> DPLL_MULT_SHIFT,   
    DPLL_DIV(0) >> DPLL_DIV_SHIFT, 
    DPLL_AUTOIDLE_DISABLED >> DPLL_AUTOIDLE_SHIFT,
    1
};

static
DpllState_t _dpll_3 = {
    0,  
    DPLL_RAMPTIME_DISABLE >> DPLL_RAMPTIME_SHIFT,  
    DPLL_FREQSEL(1) >> DPLL_FREQSEL_SHIFT,  
    0,  
    DPLL_MODE_LOWPOWER_BYPASS >> DPLL_MODE_SHIFT,  
    DPLL_CLK_SRC(1) >> DPLL_CLK_SRC_SHIFT,    
    DPLL_MULT(0) >> DPLL_MULT_SHIFT,   
    DPLL_DIV(0) >> DPLL_DIV_SHIFT, 
    DPLL_AUTOIDLE_DISABLED >> DPLL_AUTOIDLE_SHIFT,
    1
};

static
DpllState_t _dpll_4 = {
    0,  
    DPLL_RAMPTIME_DISABLE >> DPLL_RAMPTIME_SHIFT,  
    DPLL_FREQSEL(1) >> DPLL_FREQSEL_SHIFT,  
    0,  
    DPLL_MODE_LOWPOWER_STOP >> DPLL_MODE_SHIFT,  
    DPLL_CLK_SRC(1) >> DPLL_CLK_SRC_SHIFT,    
    DPLL_MULT(0) >> DPLL_MULT_SHIFT,   
    DPLL_DIV(0) >> DPLL_DIV_SHIFT, 
    DPLL_AUTOIDLE_DISABLED >> DPLL_AUTOIDLE_SHIFT,
    1
};

static
DpllState_t _dpll_5 = {
    0,  
    DPLL_RAMPTIME_DISABLE >> DPLL_RAMPTIME_SHIFT,  
    DPLL_FREQSEL(1) >> DPLL_FREQSEL_SHIFT,  
    0,  
    DPLL_MODE_LOWPOWER_STOP >> DPLL_MODE_SHIFT,  
    DPLL_CLK_SRC(1) >> DPLL_CLK_SRC_SHIFT,    
    DPLL_MULT(0) >> DPLL_MULT_SHIFT,   
    DPLL_DIV(0) >> DPLL_DIV_SHIFT, 
    DPLL_AUTOIDLE_DISABLED >> DPLL_AUTOIDLE_SHIFT,
    1
};


//-----------------------------------------------------------------------------
// clock divisor table
//
SrcClockDivisorTable_t _ssr_fclk = {1, {{kCOREX2_FCLK, 1}}};
//SrcClockDivisorTable_t _cam_fclk = {1, {{kDPLL4_CLKOUT_M5X2, 16}}};
SrcClockDivisorTable_t _dss1_fclk= {1, {{kDPLL4_CLKOUT_M4X2, 16}}};
SrcClockDivisorTable_t _usim_fclk= {2, {{kSYS_CLK, 2}, {kCM_USIM_CLK, 0}}};
SrcClockDivisorTable_t _cm_usim_fclk={2, {{k96M_FCLK, 0}, {k120M_FCLK, 0}}};
SrcClockDivisorTable_t _sgx_fclk = {4, {{kCORE_CLK, 0}, {kCM_96M_FCLK, 3}, {kPRM_96M_192M_ALWON_CLK, 4}, {kCOREX2_FCLK, 6}}};
SrcClockDivisorTable_t _tv_fclk =  {2, {{kDPLL4_M3X2_CLK, 16}, {kSYS_ALTCLK, 0}}};

//-----------------------------------------------------------------------------
// initialize voltage domain ref count
//
VddRefCountTable s_VddTable = {    
    0,                      // kVDD1
    0,                      // kVDD2
    0,                      // kVDD3
    0,                      // kVDD4
    0,                      // kVDD5
    0,                      // kVDD_EXT
    0,                      // kVDDS
    0,                      // kVDDPLL
    0,                      // kVDDADAC
    0,                      // kMMC_VDDS
};


//-----------------------------------------------------------------------------
// initialize dpll domain ref count
//
DpllMap s_DpllTable = {
    { 
    kVDD1,      0,  &_dpll_1    // kDPLL1
    }, { 
    kVDD1,      0,  &_dpll_2    // kDPLL2
    }, { 
    kVDD2,      0,  &_dpll_3    // kDPLL3
    }, { 
    kVDD4,      0,  &_dpll_4    // kDPLL4
    }, { 
    kVDD5,      0,  &_dpll_5    // kDPLL5
    }, { 
    kVDD_EXT,   0,   NULL       // kDPLL_EXT
    }, 
};


//-----------------------------------------------------------------------------
// initialize dpll clkout ref count
//
DpllClkOutMap s_DpllClkOutTable = {
    { 
    kDPLL_EXT,  0           // kEXT_32KHZ
    }, { 
    kDPLL1,     0           // kDPLL1_CLKOUT_M2X2
    }, { 
    kDPLL2,     0           // kDPLL2_CLKOUT_M2
    }, { 
    kDPLL3,     0           // kDPLL3_CLKOUT_M2
    }, { 
    kDPLL3,     0           // kDPLL3_CLKOUT_M2X2
    }, { 
    kDPLL3,     0           // kDPLL3_CLKOUT_M3X2
    }, { 
    kDPLL4,     0           // kDPLL4_CLKOUT_M2X2
    }, { 
    kDPLL4,     0           // kDPLL4_CLKOUT_M3X2
    }, { 
    kDPLL4,     0           // kDPLL4_CLKOUT_M4X2
    }, { 
    kDPLL4,     0           // kDPLL4_CLKOUT_M5X2
    }, { 
    kDPLL4,     0           // kDPLL4_CLKOUT_M6X2
    }, { 
    kDPLL5,     0           // kDPLL5_CLKOUT_M2
    }, { 
    kDPLL_EXT,  0           // kEXT_SYS_CLK
    }, { 
    kDPLL_EXT,  0           // kEXT_ALT
    }, { 
    kDPLL_EXT,  0           // kEXT_MISC
    }
};

SrcClockMap s_SrcClockTable = {
    {
/*
    Parent Clock      RefCount  isDPLL  SrcClockDivisorTbl  clock
--------------------------------------------------------------------------
*/
    kDPLL1_CLKOUT_M2X2,     0,  TRUE,   NULL,               kDPLL1_M2X2_CLK
    }, {
    kDPLL2_CLKOUT_M2,       0,  TRUE,   NULL,               kDPLL2_M2_CLK
    }, {
    kDPLL3_CLKOUT_M2,       0,  TRUE,   NULL,               kCORE_CLK
    }, {
    kDPLL3_CLKOUT_M2X2,     0,  TRUE,   NULL,               kCOREX2_FCLK
    }, {
    kDPLL3_CLKOUT_M3X2,     0,  TRUE,   NULL,               kEMUL_CORE_ALWON_CLK
    }, {
    kDPLL4_CLKOUT_M2X2,     0,  TRUE,   NULL,               kPRM_96M_192M_ALWON_CLK
    }, {
    kDPLL4_CLKOUT_M3X2,     0,  TRUE,   NULL,               kDPLL4_M3X2_CLK
    }, {
    kDPLL4_CLKOUT_M4X2,     0,  TRUE,   &_dss1_fclk,        kDSS1_ALWON_FCLK
    },
    /*{
    kDPLL4_CLKOUT_M5X2,     0,  TRUE,   &_cam_fclk,         kCAM_MCLK
    },*/
    {
    kDPLL4_CLKOUT_M6X2,     0,  TRUE,   NULL,               kEMUL_PER_ALWON_CLK
    }, {
    kDPLL5_CLKOUT_M2,       0,  TRUE,   NULL,               k120M_FCLK
    }, {
    kEXT_32KHZ,             0,  TRUE,   NULL,               k32K_FCLK
    }, {
    kEXT_SYS_CLK,           0,  TRUE,   NULL,               kSYS_CLK
    }, {
    kEXT_ALT,               0,  TRUE,   NULL,               kSYS_ALTCLK
    }, {
    kINT_OSC,               0,  TRUE,   NULL,               kSECURE_32K_FCLK
    }, {
    kEXT_MISC,              0,  TRUE,   NULL,               kMCBSP_CLKS
    }, {
    kSYS_CLK,               0,  FALSE,  NULL,               kUSBTLL_SAR_FCLK
    }, {
    kSYS_CLK,               0,  FALSE,  NULL,               kUSBHOST_SAR_FCLK
    }, {
    kSYS_CLK,               0,  FALSE,  NULL,               kEFUSE_ALWON_FCLK
    },
    {
    kSYS_CLK,               0,  FALSE,  NULL,               kSR_ALWON_FCLK
    },
    {
    kSYS_CLK,               0,  FALSE,  NULL,               kDPLL1_ALWON_FCLK
    }, {
    kSYS_CLK,               0,  FALSE,  NULL,               kDPLL2_ALWON_FCLK
    }, {
    kSYS_CLK,               0,  FALSE,  NULL,               kDPLL3_ALWON_FCLK
    }, {
    kSYS_CLK,               0,  FALSE,  NULL,               kDPLL4_ALWON_FCLK
    }, {
    kSYS_CLK,               0,  FALSE,  NULL,               kDPLL5_ALWON_FCLK
    }, {
    kSYS_CLK,               0,  FALSE,  NULL,               kCM_SYS_CLK
    }, {
    kSYS_CLK,               0,  FALSE,  NULL,               kDSS2_ALWON_FCLK
    }, {
    kSYS_CLK,               0,  FALSE,  NULL,               kWKUP_L4_ICLK
    }, {
    k32K_FCLK,              0,  FALSE,  NULL,               kCM_32K_CLK
    }, {
    kCM_32K_CLK,            0,  FALSE,  NULL,               kCORE_32K_FCLK
    }, {
    k32K_FCLK,              0,  FALSE,  NULL,               kWKUP_32K_FCLK
    }, {
    k32K_FCLK,              0,  FALSE,  NULL,               kPER_32K_ALWON_FCLK
    }, {
    k120M_FCLK,             0,  FALSE,  NULL,               kCORE_120M_FCLK
    }, {
    kPRM_96M_192M_ALWON_CLK,0,  FALSE,  NULL,               kCM_96M_FCLK
    }, {
    kPRM_96M_192M_ALWON_CLK,0,  FALSE,  NULL,				k96M_ALWON_FCLK
    }, {
    kCORE_CLK,              0,  FALSE,  NULL,				kL3_ICLK
    }, {
    kL3_ICLK,               0,  FALSE,  NULL,				kL4_ICLK
    }, {
    kL4_ICLK,               0,  FALSE,  NULL,				kUSB_L4_ICLK
    }, {
    kL4_ICLK,               0,  FALSE,  NULL,				kRM_ICLK
    }, {
    kCORE_CLK,              0,  FALSE,  NULL,				kDPLL1_FCLK
    }, {
    kCORE_CLK,              0,  FALSE,  NULL,				kDPLL2_FCLK
    }, {
    kL3_ICLK,               0,  FALSE,  NULL,				kCORE_L3_ICLK
    }, {
    kL4_ICLK,               0,  FALSE,  NULL,				kCORE_L4_ICLK
    }, {
    kL3_ICLK,               0,  FALSE,  NULL,				kSECURITY_L3_ICLK
    }, {
    kL4_ICLK,               0,  FALSE,  NULL,				kSECURITY_L4_ICLK1
    }, {
    kL4_ICLK,               0,  FALSE,  NULL,				kSECURITY_L4_ICLK2
    }, {
    kL3_ICLK,               0,  FALSE,  NULL,				kSGX_L3_ICLK
    }, {
    kL4_ICLK,               0,  FALSE,  NULL,				kSSI_L4_ICLK
    }, {
    kL3_ICLK,               0,  FALSE,  NULL,				kDSS_L3_ICLK
    }, {
    kL4_ICLK,               0,  FALSE,  NULL,				kDSS_L4_ICLK
    },
    {
    kL3_ICLK,               0,  FALSE,  NULL,				kCAM_L3_ICLK
    }, {
    kL4_ICLK,               0,  FALSE,  NULL,				kCAM_L4_ICLK
    },
    {
    kL3_ICLK,               0,  FALSE,  NULL,				kUSBHOST_L3_ICLK
    }, {
    kL4_ICLK,               0,  FALSE,  NULL,				kUSBHOST_L4_ICLK
    }, {
    kL4_ICLK,               0,  FALSE,  NULL,				kPER_L4_ICLK
    }, {
    kL4_ICLK,               0,  FALSE,  NULL,				kSR_L4_ICLK
    }, {
    kCOREX2_FCLK,           0,  FALSE,  &_ssr_fclk,         kSSI_SSR_FCLK
    }, {
    kCOREX2_FCLK,           0,  FALSE,  NULL,				kSSI_SST_FCLK
    }, {
    k96M_FCLK,              0,  FALSE,  NULL,				kCORE_96M_FCLK
    }, {
    k96M_FCLK,              0,  FALSE,  NULL,				kDSS_96M_FCLK
    },
    {
    k96M_FCLK,              0,  FALSE,  NULL,				kCSI2_96M_FCLK
    },
    {
    k48M_FCLK,              0,  FALSE,  NULL,				kCORE_48M_FCLK
    }, {
    k48M_FCLK,              0,  FALSE,  NULL,				kUSBHOST_48M_FCLK
    }, {
    k48M_FCLK,              0,  FALSE,  NULL,				kPER_48M_FCLK
    }, {
    k48M_FCLK,              0,  FALSE,  NULL,				k12M_FCLK
    }, {
    k12M_FCLK,              0,  FALSE,  NULL,				kCORE_12M_FCLK
    }, {
    k54M_FCLK,              0,  FALSE,  NULL,				kDSS_TV_FCLK
    }, {
    k120M_FCLK,             0,  FALSE,  NULL,				kUSBHOST_120M_FCLK
    }, {
    kCM_96M_FCLK/*kDPLL5_M2_CLK*/,  0,  FALSE,  &_cm_usim_fclk, kCM_USIM_CLK
    }, {
    kSYS_CLK/*kCM_USIM_CLK*/,       0,  FALSE,  &_usim_fclk, kUSIM_FCLK
    }, {
    kCM_96M_FCLK/*kCM_SYS_CLK*/,    0,  FALSE,  NULL   ,            k96M_FCLK
    }, {
    kCM_96M_FCLK/*kSYS_ALTCLK*/,    0,  FALSE,  NULL   ,            k48M_FCLK
    }, {
    kDPLL4_M3X2_CLK/*kSYS_ALTCLK*/, 0,  FALSE,  &_tv_fclk, 			k54M_FCLK
    }, {
    kCORE_CLK/*kCM_96M_FCLK, kPRM_96M_192M_ALWON_CLK, kCOREX2_FCLK*/, 0, FALSE, &_sgx_fclk, kSGX_FCLK
    }, {
    k32K_FCLK/*kSYS_CLK*/,          0,  FALSE,  NULL   ,            kGPT1_FCLK
    }, {
    kSYS_CLK/*k32K_FCLK*/,          0,  FALSE,  NULL   ,            kGPT2_ALWON_FCLK
    }, {
    kSYS_CLK/*k32K_FCLK*/,          0,  FALSE,  NULL   ,            kGPT3_ALWON_FCLK
    }, {
    kSYS_CLK/*k32K_FCLK*/,          0,  FALSE,  NULL   ,            kGPT4_ALWON_FCLK
    }, {
    kSYS_CLK/*k32K_FCLK*/,          0,  FALSE,  NULL   ,            kGPT5_ALWON_FCLK
    }, {
    kSYS_CLK/*k32K_FCLK*/,          0,  FALSE,  NULL   ,            kGPT6_ALWON_FCLK
    }, {
    kSYS_CLK/*k32K_FCLK*/,          0,  FALSE,  NULL   ,            kGPT7_ALWON_FCLK
    }, {
    kSYS_CLK/*k32K_FCLK*/,          0,  FALSE,  NULL   ,            kGPT8_ALWON_FCLK
    }, {
    kSYS_CLK/*k32K_FCLK*/,          0,  FALSE,  NULL   ,            kGPT9_ALWON_FCLK
    }, {
    kCM_SYS_CLK/*kCM_32K_CLK*/,     0,  FALSE,  NULL   ,            kGPT10_FCLK
    }, {
    kCM_SYS_CLK/*kCM_32K_CLK*/,     0,  FALSE,  NULL   ,            kGPT11_FCLK
    }, {
    kCORE_96M_FCLK/*kMCBSP_CLKS*/,  0,  FALSE,  NULL   ,            kMCBSP1_CLKS
    } , {
    k96M_ALWON_FCLK/*kMCBSP_CLKS*/, 0,  FALSE,  NULL   ,            kMCBSP2_CLKS
    } , {
    k96M_ALWON_FCLK/*kMCBSP_CLKS*/, 0,  FALSE,  NULL   ,            kMCBSP3_CLKS
    } , {
    k96M_ALWON_FCLK/*kMCBSP_CLKS*/, 0,  FALSE,  NULL   ,            kMCBSP4_CLKS
    } , {
    kCORE_96M_FCLK/*kMCBSP_CLKS*/,  0,  FALSE,  NULL   ,            kMCBSP5_CLKS
    }    
};

//-----------------------------------------------------------------------------
static
BOOL
_ClockInitialize(
    DpllState_t    *pDpll,    
    UINT            cm_clken_pll,
    UINT            cm_clksel_pll,
    UINT            cm_autoidle_pll,
    UINT            outputDivisor
    )
{
    BOOL rc = TRUE;
    OALMSG(OAL_FUNC, (L"+_ClockInitialize("
        L"pDpll=0x%08X, cm_clken_pll=0x%08X, cm_clksel_pll=0x%08X"
        L"cm_autoidle_pll=0x%08X)\r\n", pDpll,
        cm_clken_pll, cm_clksel_pll, cm_autoidle_pll)
        );

    // all values are normalized and then cached in SDRAM

    // save autoidle modes
    pDpll->dpllAutoidleState = (cm_autoidle_pll & DPLL_AUTOIDLE_MASK) >> DPLL_AUTOIDLE_SHIFT;

    // save dpll modes
    pDpll->lowPowerEnabled = (cm_clken_pll & EN_DPLL_LPMODE) >> EN_DPLL_LPMODE_SHIFT;
    pDpll->dpllMode = (cm_clken_pll & DPLL_MODE_MASK) >> DPLL_MODE_SHIFT;
    pDpll->driftGuard = (cm_clken_pll & EN_DPLL_DRIFTGUARD) >> EN_DPLL_DRIFTGUARD_SHIFT;
    pDpll->rampTime = (cm_clken_pll & DPLL_RAMPTIME_MASK) >> DPLL_RAMPTIME_SHIFT;

    // frequency info
    pDpll->freqSelection = (cm_clken_pll & DPLL_FREQSEL_MASK) >> DPLL_FREQSEL_SHIFT;
    pDpll->sourceDivisor = (cm_clksel_pll & DPLL_CLK_SRC_MASK) >> DPLL_CLK_SRC_SHIFT;
    pDpll->divisor = (cm_clksel_pll & DPLL_DIV_MASK) >> DPLL_DIV_SHIFT;
    pDpll->multiplier = (cm_clksel_pll & DPLL_MULT_MASK) >> DPLL_MULT_SHIFT;
    pDpll->outputDivisor = outputDivisor;

    OALMSG(OAL_FUNC, (L"-_ClockInitialize()=%d\r\n", rc));
    return rc;    
}

//-----------------------------------------------------------------------------
static
BOOL
_ClockHwUpdateParentClock(
    UINT clockId
    )
{
    UINT i;
    UINT val;
    BOOL rc = FALSE;
    UINT parentClock;
    SrcClockDivisorTable_t *pDivisors;
    OALMSG(OAL_FUNC, (L"+_ClockHwUpdateParentClock(clockId=%d)\r\n", clockId));

    // quick check for valid source clock id's
    if (clockId > kSOURCE_CLOCK_COUNT) goto cleanUp;

    // initialize local var.
    pDivisors = s_SrcClockTable[clockId].pDivisors;
    parentClock = s_SrcClockTable[clockId].parentClk;

    switch (clockId)
        {
        case kGPT2_ALWON_FCLK:
            // verify parent clock is valid
            switch(parentClock)
                {
                case kSYS_CLK:
                    SETREG32(&g_pPrcmCm->pOMAP_PER_CM->CM_CLKSEL_PER, CLKSEL_GPT2);
                    break;

                case k32K_FCLK:
                    CLRREG32(&g_pPrcmCm->pOMAP_PER_CM->CM_CLKSEL_PER, CLKSEL_GPT2);
                    break;

                default:
                    goto cleanUp;
                }
            break;

        case kGPT3_ALWON_FCLK:
            // verify parent clock is valid
            switch(parentClock)
                {
                case kSYS_CLK:
                    SETREG32(&g_pPrcmCm->pOMAP_PER_CM->CM_CLKSEL_PER, CLKSEL_GPT3);
                    break;

                case k32K_FCLK:
                    CLRREG32(&g_pPrcmCm->pOMAP_PER_CM->CM_CLKSEL_PER, CLKSEL_GPT3);
                    break;

                default:
                    goto cleanUp;
                }
            break;

        case kGPT4_ALWON_FCLK:
            // verify parent clock is valid
            switch(parentClock)
                {
                case kSYS_CLK:
                    SETREG32(&g_pPrcmCm->pOMAP_PER_CM->CM_CLKSEL_PER, CLKSEL_GPT4);
                    break;

                case k32K_FCLK:
                    CLRREG32(&g_pPrcmCm->pOMAP_PER_CM->CM_CLKSEL_PER, CLKSEL_GPT4);
                    break;

                default:
                    goto cleanUp;
                }
            break;
            
        case kGPT5_ALWON_FCLK:
            // verify parent clock is valid
            switch(parentClock)
                {
                case kSYS_CLK:
                    SETREG32(&g_pPrcmCm->pOMAP_PER_CM->CM_CLKSEL_PER, CLKSEL_GPT5);
                    break;

                case k32K_FCLK:
                    CLRREG32(&g_pPrcmCm->pOMAP_PER_CM->CM_CLKSEL_PER, CLKSEL_GPT5);
                    break;

                default:
                    goto cleanUp;
                }
            break;
            
        case kGPT6_ALWON_FCLK:
            // verify parent clock is valid
            switch(parentClock)
                {
                case kSYS_CLK:
                    SETREG32(&g_pPrcmCm->pOMAP_PER_CM->CM_CLKSEL_PER, CLKSEL_GPT6);
                    break;

                case k32K_FCLK:
                    CLRREG32(&g_pPrcmCm->pOMAP_PER_CM->CM_CLKSEL_PER, CLKSEL_GPT6);
                    break;

                default:
                    goto cleanUp;
                }
            break;
            
        case kGPT7_ALWON_FCLK:
            // verify parent clock is valid
            switch(parentClock)
                {
                case kSYS_CLK:
                    SETREG32(&g_pPrcmCm->pOMAP_PER_CM->CM_CLKSEL_PER, CLKSEL_GPT7);
                    break;

                case k32K_FCLK:
                    CLRREG32(&g_pPrcmCm->pOMAP_PER_CM->CM_CLKSEL_PER, CLKSEL_GPT7);
                    break;

                default:
                    goto cleanUp;
                }
            break;
            
        case kGPT8_ALWON_FCLK:
            // verify parent clock is valid
            switch(parentClock)
                {
                case kSYS_CLK:
                    SETREG32(&g_pPrcmCm->pOMAP_PER_CM->CM_CLKSEL_PER, CLKSEL_GPT8);
                    break;

                case k32K_FCLK:
                    CLRREG32(&g_pPrcmCm->pOMAP_PER_CM->CM_CLKSEL_PER, CLKSEL_GPT8);
                    break;

                default:
                    goto cleanUp;
                }
            break;
            
        case kGPT9_ALWON_FCLK:
            // verify parent clock is valid
            switch(parentClock)
                {
                case kSYS_CLK:
                    SETREG32(&g_pPrcmCm->pOMAP_PER_CM->CM_CLKSEL_PER, CLKSEL_GPT9);
                    break;

                case k32K_FCLK:
                    CLRREG32(&g_pPrcmCm->pOMAP_PER_CM->CM_CLKSEL_PER, CLKSEL_GPT9);
                    break;

                default:
                    goto cleanUp;
                }
            break;
            
        case kGPT1_FCLK:
            // verify parent clock is valid
            switch(parentClock)
                {
                case kSYS_CLK:
                    SETREG32(&g_pPrcmCm->pOMAP_WKUP_CM->CM_CLKSEL_WKUP, CLKSEL_GPT1);
                    SETREG32(&g_pPrcmRestore->CM_CLKSEL_WKUP, CLKSEL_GPT1);
                    break;

                case k32K_FCLK:
                    CLRREG32(&g_pPrcmCm->pOMAP_WKUP_CM->CM_CLKSEL_WKUP, CLKSEL_GPT1);
                    CLRREG32(&g_pPrcmRestore->CM_CLKSEL_WKUP, CLKSEL_GPT1);
                    break;

                default:
                    goto cleanUp;
                }
            break;
            
        case kGPT10_FCLK:
            // verify parent clock is valid
            switch(parentClock)
                {
                case kCM_SYS_CLK:
                    SETREG32(&g_pPrcmCm->pOMAP_CORE_CM->CM_CLKSEL_CORE, CLKSEL_GPT10);
                    SETREG32(&g_pPrcmRestore->CM_CLKSEL_CORE, CLKSEL_GPT10);
                    break;

                case kCM_32K_CLK:
                    CLRREG32(&g_pPrcmCm->pOMAP_CORE_CM->CM_CLKSEL_CORE, CLKSEL_GPT10);
                    CLRREG32(&g_pPrcmRestore->CM_CLKSEL_CORE, CLKSEL_GPT10);
                    break;

                default:
                    goto cleanUp;
                }
            break;
            
        case kGPT11_FCLK:
            // verify parent clock is valid
            switch(parentClock)
                {
                case kCM_SYS_CLK:
                    SETREG32(&g_pPrcmCm->pOMAP_CORE_CM->CM_CLKSEL_CORE, CLKSEL_GPT11);
                    SETREG32(&g_pPrcmRestore->CM_CLKSEL_CORE, CLKSEL_GPT11);
                    break;

                case kCM_32K_CLK:
                    CLRREG32(&g_pPrcmCm->pOMAP_CORE_CM->CM_CLKSEL_CORE, CLKSEL_GPT11);
                    CLRREG32(&g_pPrcmRestore->CM_CLKSEL_CORE, CLKSEL_GPT11);
                    break;

                default:
                    goto cleanUp;
                }
            break;

        case k48M_FCLK:                
            // verify parent clock is valid
            switch(parentClock)
                {
                case kCM_96M_FCLK:
                    CLRREG32(&g_pPrcmCm->pOMAP_CLOCK_CONTROL_CM->CM_CLKSEL1_PLL, SOURCE_48M);
                    CLRREG32(&g_pPrcmRestore->CM_CLKSEL1_PLL, SOURCE_48M);
                    break;

                case kSYS_ALTCLK:
                    SETREG32(&g_pPrcmCm->pOMAP_CLOCK_CONTROL_CM->CM_CLKSEL1_PLL, SOURCE_48M);
                    SETREG32(&g_pPrcmRestore->CM_CLKSEL1_PLL, SOURCE_48M);
                    break;

                default:
                    goto cleanUp;
                }
            break;

        case k96M_FCLK:
            // verify parent clock is valid
            switch(parentClock)
                {
                case kCM_96M_FCLK:
                    CLRREG32(&g_pPrcmCm->pOMAP_CLOCK_CONTROL_CM->CM_CLKSEL1_PLL, SOURCE_96M);
                    CLRREG32(&g_pPrcmRestore->CM_CLKSEL1_PLL, SOURCE_96M);
                    break;

                case kCM_SYS_CLK:
                    SETREG32(&g_pPrcmCm->pOMAP_CLOCK_CONTROL_CM->CM_CLKSEL1_PLL, SOURCE_96M);
                    SETREG32(&g_pPrcmRestore->CM_CLKSEL1_PLL, SOURCE_96M);
                    break;

                default:
                    goto cleanUp;
                }
            break;

        case kDSS1_ALWON_FCLK:
            // format divider
            val = INREG32(&g_pPrcmCm->pOMAP_DSS_CM->CM_CLKSEL_DSS);
            val = (val & ~CLKSEL_DSS1_MASK);
            val |= CLKSEL_DSS1(pDivisors->SourceClock[0].divisor);

            // write to hw
            OUTREG32(&g_pPrcmCm->pOMAP_DSS_CM->CM_CLKSEL_DSS, val);
            break;

        case kSSI_SSR_FCLK:
            // format divider
            val = INREG32(&g_pPrcmCm->pOMAP_CORE_CM->CM_CLKSEL_CORE);
            val = (val & ~CLKSEL_SSI_MASK);
            val |= CLKSEL_SSI(pDivisors->SourceClock[0].divisor);

            // write to hw
            OUTREG32(&g_pPrcmCm->pOMAP_CORE_CM->CM_CLKSEL_CORE, val);
            OUTREG32(&g_pPrcmRestore->CM_CLKSEL_CORE, val);
            break;

        case kSGX_FCLK:
            // verify parent clock is valid            
            for (i = 0; i < pDivisors->count; ++i)
                {
                if (parentClock == pDivisors->SourceClock[i].id)
                    {
                    // format divider
                    val = INREG32(&g_pPrcmCm->pOMAP_SGX_CM->CM_CLKSEL_SGX);
                    val = (val & ~CLKSEL_SGX_MASK);
                    val |= CLKSEL_SGX(pDivisors->SourceClock[i].divisor);

                    // write to hw
                    OUTREG32(&g_pPrcmCm->pOMAP_SGX_CM->CM_CLKSEL_SGX, val);
                    break;
                    }
                }
            break;

        case k54M_FCLK:
            switch (parentClock)
                {
                case kDPLL4_M3X2_CLK:
                    // format divider
                    val = INREG32(&g_pPrcmCm->pOMAP_DSS_CM->CM_CLKSEL_DSS);
                    val = (val & ~CLKSEL_TV_MASK);
                    val |= CLKSEL_TV(pDivisors->SourceClock[0].divisor);

                    // write to hw
                    OUTREG32(&g_pPrcmCm->pOMAP_DSS_CM->CM_CLKSEL_DSS, val);
                    CLRREG32(&g_pPrcmCm->pOMAP_CLOCK_CONTROL_CM->CM_CLKSEL1_PLL, SOURCE_54M);
                    CLRREG32(&g_pPrcmRestore->CM_CLKSEL1_PLL, SOURCE_54M);
                    break;

                case kSYS_ALTCLK:
                    // write to hw
                    SETREG32(&g_pPrcmCm->pOMAP_CLOCK_CONTROL_CM->CM_CLKSEL1_PLL, SOURCE_54M);
                    SETREG32(&g_pPrcmRestore->CM_CLKSEL1_PLL, SOURCE_54M);
                    break;

                default:
                    goto cleanUp;
                }
            break;

        case kUSIM_FCLK:
        case kCM_USIM_CLK:
            // ignore if parent clock *is not* the CM_USIM_CLK
            if (s_SrcClockTable[kUSIM_FCLK].parentClk == kCM_USIM_CLK)
                {   
                // make sure divisor pointer points to kCM_USIM_CLK divisor;
                pDivisors = s_SrcClockTable[kCM_USIM_CLK].pDivisors;
                
                // verify parent clock is valid
                for (i = 0; i < pDivisors->count; ++i)
                    {
                    if (parentClock == pDivisors->SourceClock[i].id)
                        {
                        // format divider
                        val = INREG32(&g_pPrcmCm->pOMAP_WKUP_CM->CM_CLKSEL_WKUP);
                        val = (val & ~CLKSEL_USIMOCP_MASK);
                        val |= CLKSEL_USIMOCP(pDivisors->SourceClock[i].divisor);

                        // write to hw
                        OUTREG32(&g_pPrcmCm->pOMAP_WKUP_CM->CM_CLKSEL_WKUP, val);
                        OUTREG32(&g_pPrcmRestore->CM_CLKSEL_WKUP, val);
                        break;
                        }
                    }
                }
            else
                {
                 // format divider
                val = INREG32(&g_pPrcmCm->pOMAP_WKUP_CM->CM_CLKSEL_WKUP);
                val = (val & ~CLKSEL_USIMOCP_MASK);
                val |= CLKSEL_USIMOCP(pDivisors->SourceClock[0].divisor);

                // write to hw
                OUTREG32(&g_pPrcmCm->pOMAP_WKUP_CM->CM_CLKSEL_WKUP, val);
                OUTREG32(&g_pPrcmRestore->CM_CLKSEL_WKUP, val);
                }
            break;

        case kMCBSP1_CLKS:
            // verify parent clock is valid
            switch(parentClock)
                {
                case kCORE_96M_FCLK:
                    CLRREG32(&g_pSysCtrlGenReg->CONTROL_DEVCONF0, DEVCONF0_MCBSP1_CLKS);
                    break;

                case kMCBSP_CLKS:
                    SETREG32(&g_pSysCtrlGenReg->CONTROL_DEVCONF0, DEVCONF0_MCBSP1_CLKS);
                    break;
                }
            break;

       case kMCBSP2_CLKS:
            // verify parent clock is valid
            switch(parentClock)
                {
                case k96M_ALWON_FCLK:
                    CLRREG32(&g_pSysCtrlGenReg->CONTROL_DEVCONF0, DEVCONF0_MCBSP2_CLKS);
                    break;

                case kMCBSP_CLKS:
                    SETREG32(&g_pSysCtrlGenReg->CONTROL_DEVCONF0, DEVCONF0_MCBSP2_CLKS);
                    break;
                }
            break;

        case kMCBSP3_CLKS:
            // verify parent clock is valid
            switch(parentClock)
                {
                case k96M_ALWON_FCLK:
                    CLRREG32(&g_pSysCtrlGenReg->CONTROL_DEVCONF1, DEVCONF1_MCBSP3_CLKS);
                    break;

                case kMCBSP_CLKS:
                    SETREG32(&g_pSysCtrlGenReg->CONTROL_DEVCONF1, DEVCONF1_MCBSP3_CLKS);
                    break;
                }
            break;

        case kMCBSP4_CLKS:
            // verify parent clock is valid
            switch(parentClock)
                {
                case k96M_ALWON_FCLK:
                    CLRREG32(&g_pSysCtrlGenReg->CONTROL_DEVCONF1, DEVCONF1_MCBSP4_CLKS);
                    break;

                case kMCBSP_CLKS:
                    SETREG32(&g_pSysCtrlGenReg->CONTROL_DEVCONF1, DEVCONF1_MCBSP4_CLKS);
                    break;
                }
            break;

        case kMCBSP5_CLKS:
            // verify parent clock is valid
            switch(parentClock)
                {
                case kCORE_96M_FCLK:
                    CLRREG32(&g_pSysCtrlGenReg->CONTROL_DEVCONF1, DEVCONF1_MCBSP5_CLKS);
                    break;

                case kMCBSP_CLKS:
                    SETREG32(&g_pSysCtrlGenReg->CONTROL_DEVCONF1, DEVCONF1_MCBSP5_CLKS);
                    break;
                }
            break;
  
        }

    rc = TRUE;
            
cleanUp:    
    OALMSG(OAL_FUNC, (L"-_ClockHwUpdateParentClock()=%d\r\n", rc));
    return rc;
}

//-----------------------------------------------------------------------------
static
BOOL
_ClockHwUpdateDpllState(
    UINT dpll,
    UINT ffMask
    )
{
    BOOL rc = TRUE;
    UINT cm_clken_pll;
    UINT cm_autoidle_pll;
    
    // update the following hw registers
    // CM_AUTOIDLE_PLL_xxx
    // CM_CLKEN_PLL_xxx
    // CM_CLKSELn_PLL_xxx
    // CM_CLKSTCTRL_xxx


    switch (dpll)
        {
        case kDPLL1:               
            cm_clken_pll = INREG32(&g_pPrcmCm->pOMAP_MPU_CM->CM_CLKEN_PLL_MPU);
            cm_autoidle_pll = INREG32(&g_pPrcmCm->pOMAP_MPU_CM->CM_AUTOIDLE_PLL_MPU);
            if (ffMask & DPLL_UPDATE_LPMODE)
                {
                cm_clken_pll &= ~EN_DPLL_LPMODE_MASK;
                cm_clken_pll |= _dpll_1.lowPowerEnabled << EN_DPLL_LPMODE_SHIFT;
                }

            if (ffMask & DPLL_UPDATE_DRIFTGUARD)
                {
                cm_clken_pll &= ~EN_DPLL_DRIFTGUARD_MASK;
                cm_clken_pll |= _dpll_1.driftGuard << EN_DPLL_DRIFTGUARD_SHIFT;
                }

            if (ffMask & DPLL_UPDATE_RAMPTIME)
                {
                cm_clken_pll &= ~DPLL_RAMPTIME_MASK;
                cm_clken_pll |= _dpll_1.rampTime << DPLL_RAMPTIME_SHIFT;                
                }

            if (ffMask & DPLL_UPDATE_DPLLMODE)
                {
                cm_clken_pll &= ~DPLL_MODE_MASK;
                cm_clken_pll |= _dpll_1.dpllMode << DPLL_MODE_SHIFT; 
                }

            if (ffMask & DPLL_UPDATE_AUTOIDLEMODE)
                {
                cm_autoidle_pll &= ~DPLL_AUTOIDLE_MASK;
                cm_autoidle_pll |= _dpll_1.dpllAutoidleState << DPLL_AUTOIDLE_SHIFT;
                }

            OUTREG32(&g_pPrcmCm->pOMAP_MPU_CM->CM_CLKEN_PLL_MPU, cm_clken_pll);
            OUTREG32(&g_pPrcmCm->pOMAP_MPU_CM->CM_AUTOIDLE_PLL_MPU, cm_autoidle_pll);

            // Save modifications to scratchpad register as well
            OUTREG32(&g_pPrcmRestore->CM_CLKEN_PLL_MPU, cm_clken_pll);        
            OUTREG32(&g_pPrcmRestore->CM_AUTOIDLE_PLL_MPU, cm_autoidle_pll);
            break;

        case kDPLL2:
            // No dPLL2 on AM3517
            break;

        case kDPLL3:
            cm_clken_pll = INREG32(&g_pPrcmCm->pOMAP_CLOCK_CONTROL_CM->CM_CLKEN_PLL);
            cm_autoidle_pll = INREG32(&g_pPrcmCm->pOMAP_CLOCK_CONTROL_CM->CM_AUTOIDLE_PLL);
            if (ffMask & DPLL_UPDATE_LPMODE)
                {
                cm_clken_pll &= ~EN_DPLL_LPMODE;
                cm_clken_pll |= _dpll_3.lowPowerEnabled << EN_DPLL_LPMODE_SHIFT;
                }

            if (ffMask & DPLL_UPDATE_DRIFTGUARD)
                {
                cm_clken_pll &= ~EN_DPLL_DRIFTGUARD;
                cm_clken_pll |= _dpll_3.driftGuard << EN_DPLL_DRIFTGUARD_SHIFT;
                }

            if (ffMask & DPLL_UPDATE_RAMPTIME)
                {
                cm_clken_pll &= ~DPLL_RAMPTIME_MASK;
                cm_clken_pll |= _dpll_3.rampTime << DPLL_RAMPTIME_SHIFT;
                }

            if (ffMask & DPLL_UPDATE_DPLLMODE)
                {
                cm_clken_pll &= ~DPLL_MODE_MASK;
                cm_clken_pll |= _dpll_3.dpllMode << DPLL_MODE_SHIFT; 
                }

            if (ffMask & DPLL_UPDATE_AUTOIDLEMODE)
                {
                cm_autoidle_pll &= ~DPLL_AUTOIDLE_MASK;
                cm_autoidle_pll |= _dpll_3.dpllAutoidleState << DPLL_AUTOIDLE_SHIFT;
                }

            OUTREG32(&g_pPrcmCm->pOMAP_CLOCK_CONTROL_CM->CM_CLKEN_PLL, cm_clken_pll);
            OUTSYSREG32(OMAP_PRCM_CLOCK_CONTROL_CM_REGS, CM_AUTOIDLE_PLL, cm_autoidle_pll);

            // Save modifications to scratchpad register as well
            OUTREG32(&g_pPrcmRestore->CM_CLKEN_PLL, cm_clken_pll);
            break;

        case kDPLL4:
            cm_clken_pll = INREG32(&g_pPrcmCm->pOMAP_CLOCK_CONTROL_CM->CM_CLKEN_PLL);
            cm_autoidle_pll = INREG32(&g_pPrcmCm->pOMAP_CLOCK_CONTROL_CM->CM_AUTOIDLE_PLL);
            if (ffMask & DPLL_UPDATE_LPMODE)
                {
                cm_clken_pll &= ~(EN_DPLL_LPMODE << DPLL_PER_MODE_SHIFT);
                cm_clken_pll |= (_dpll_4.lowPowerEnabled << EN_DPLL_LPMODE_SHIFT) << DPLL_PER_MODE_SHIFT;
                }

            if (ffMask & DPLL_UPDATE_DRIFTGUARD)
                {
                cm_clken_pll &= ~(EN_DPLL_DRIFTGUARD << DPLL_PER_MODE_SHIFT);
                cm_clken_pll |= (_dpll_4.driftGuard << EN_DPLL_DRIFTGUARD_SHIFT) << DPLL_PER_MODE_SHIFT;                
                }

            if (g_dwCpuFamily != CPU_FAMILY_DM37XX)
            if (ffMask & DPLL_UPDATE_RAMPTIME)
                {
                cm_clken_pll &= ~(DPLL_RAMPTIME_MASK << DPLL_PER_MODE_SHIFT);
                cm_clken_pll |= (_dpll_4.rampTime << DPLL_RAMPTIME_SHIFT) << DPLL_PER_MODE_SHIFT;                
                }

            if (ffMask & DPLL_UPDATE_DPLLMODE)
                {
                cm_clken_pll &= ~(DPLL_MODE_MASK << DPLL_PER_MODE_SHIFT);
                cm_clken_pll |= (_dpll_4.dpllMode << DPLL_MODE_SHIFT) << DPLL_PER_MODE_SHIFT; 
                }

            if (ffMask & DPLL_UPDATE_AUTOIDLEMODE)
                {
                cm_autoidle_pll &= ~(DPLL_AUTOIDLE_MASK << DPLL_PER_IDLE_SHIFT);
                cm_autoidle_pll |= (_dpll_4.dpllAutoidleState << DPLL_AUTOIDLE_SHIFT) << DPLL_PER_IDLE_SHIFT;
                }

            OUTREG32(&g_pPrcmCm->pOMAP_CLOCK_CONTROL_CM->CM_CLKEN_PLL, cm_clken_pll);
            OUTSYSREG32(OMAP_PRCM_CLOCK_CONTROL_CM_REGS, CM_AUTOIDLE_PLL, cm_autoidle_pll);

            // Save modifications to scratchpad register as well
            OUTREG32(&g_pPrcmRestore->CM_CLKEN_PLL, cm_clken_pll);
            break;

        case kDPLL5:
            cm_clken_pll = INREG32(&g_pPrcmCm->pOMAP_CLOCK_CONTROL_CM->CM_CLKEN2_PLL);
            cm_autoidle_pll = INREG32(&g_pPrcmCm->pOMAP_CLOCK_CONTROL_CM->CM_AUTOIDLE2_PLL);
            if (ffMask & DPLL_UPDATE_LPMODE)
                {
                cm_clken_pll &= ~EN_DPLL_LPMODE;
                cm_clken_pll |= _dpll_5.lowPowerEnabled << EN_DPLL_LPMODE_SHIFT;
                }

            if (ffMask & DPLL_UPDATE_DRIFTGUARD)
                {
                cm_clken_pll &= ~EN_DPLL_DRIFTGUARD;
                cm_clken_pll |= _dpll_5.driftGuard << EN_DPLL_DRIFTGUARD_SHIFT;                
                }

            if (ffMask & DPLL_UPDATE_RAMPTIME)
                {
                cm_clken_pll &= ~DPLL_RAMPTIME_MASK;
                cm_clken_pll |= _dpll_5.rampTime << DPLL_RAMPTIME_SHIFT;                
                }

            if (ffMask & DPLL_UPDATE_DPLLMODE)
                {
                cm_clken_pll &= ~DPLL_MODE_MASK;
                cm_clken_pll |= _dpll_5.dpllMode << DPLL_MODE_SHIFT; 
                }

            if (ffMask & DPLL_UPDATE_AUTOIDLEMODE)
                {
                cm_autoidle_pll &= ~DPLL_AUTOIDLE_MASK;
                cm_autoidle_pll |= _dpll_5.dpllAutoidleState << DPLL_AUTOIDLE_SHIFT;
                }
            
            OUTSYSREG32(OMAP_PRCM_CLOCK_CONTROL_CM_REGS, CM_CLKEN2_PLL, cm_clken_pll);

            // CM_AUTOIDLE_PLL is shadowed for CORE OFF support, so OUTSYSREG32 is used
            // instead of OUTREG32
            OUTSYSREG32(OMAP_PRCM_CLOCK_CONTROL_CM_REGS, CM_AUTOIDLE2_PLL, cm_autoidle_pll);

            break;

        case kDPLL_EXT:
            break;

        default:
            rc = FALSE;
        }

    return rc;
}

//-----------------------------------------------------------------------------
static
BOOL
_ClockUpdateDpllOutput(
    int dpllClkId,
    BOOL bEnable
    )
{
    int addRef;
    if (!g_bSingleThreaded)
        OALMSG(OAL_FUNC, (L"+_ClockUpdateDpllOutput"
            L"(srcClkId=%d, bEnable=%d)\r\n", dpllClkId, bEnable)
            );

    // quick check for valid dpll source clock id's
    if ((UINT)dpllClkId > kDPLL_CLKOUT_COUNT) goto cleanUp;

    // setup increment value
    addRef = (bEnable != FALSE) ? 1 : -1;

    // update dpll refCount
    if ((s_DpllClkOutTable[dpllClkId].refCount == 0 && bEnable == TRUE) ||
        (s_DpllClkOutTable[dpllClkId].refCount == 1 && bEnable == FALSE))
        {
        int dpllId = s_DpllClkOutTable[dpllClkId].dpllDomain;

        // update vdd refCount
        if ((s_DpllTable[dpllId].refCount == 0 && bEnable == TRUE) ||
            (s_DpllTable[dpllId].refCount == 1 && bEnable == FALSE))
            {
            // increment vdd refCount
            int vddId = s_DpllTable[dpllId].vddDomain;            
            s_VddTable[vddId] += addRef;   
            }

        // incrment dpll refCount
        s_DpllTable[dpllId].refCount += addRef;
        }

    // increment dpllClkSrc refCount
    s_DpllClkOutTable[dpllClkId].refCount += addRef;    

cleanUp:    
    if (!g_bSingleThreaded)
        OALMSG(OAL_FUNC, (L"-_ClockUpdateDpllOutput()=%d\r\n", TRUE));
    return TRUE;
}

//-----------------------------------------------------------------------------
void
_ClockHwUpdateDpllFrequency(
    UINT dpllId
    )
{
    BOOL bEnable;
    UINT cm_clken_pll;
    UINT cm_clksel_pll;
    UINT outputDivisor;
    UINT sdrcRfrCtrl0, sdrcRfrCtrl1;
    
    switch (dpllId)
        {
        case kDPLL1:
            cm_clken_pll = INREG32(&g_pPrcmCm->pOMAP_MPU_CM->CM_CLKEN_PLL_MPU);
            cm_clksel_pll = INREG32(&g_pPrcmCm->pOMAP_MPU_CM->CM_CLKSEL1_PLL_MPU);

            // update frequency selection and frequency
            cm_clken_pll &= ~DPLL_FREQSEL_MASK;
            cm_clken_pll |= DPLL_FREQSEL(_dpll_1.freqSelection); 
            cm_clksel_pll &= ~(DPLL_CLK_SRC_MASK | DPLL_MULT_MASK | DPLL_DIV_MASK);
            cm_clksel_pll |= DPLL_CLK_SRC(_dpll_1.sourceDivisor);
            cm_clksel_pll |= DPLL_MULT(_dpll_1.multiplier) | DPLL_DIV(_dpll_1.divisor);
            outputDivisor = _dpll_1.outputDivisor << DPLL_MPU_CLKOUT_DIV_SHIFT;

            OUTREG32(&g_pPrcmCm->pOMAP_MPU_CM->CM_CLKEN_PLL_MPU, cm_clken_pll);            
            OUTREG32(&g_pPrcmCm->pOMAP_MPU_CM->CM_CLKSEL1_PLL_MPU, cm_clksel_pll);
            OUTREG32(&g_pPrcmCm->pOMAP_MPU_CM->CM_CLKSEL2_PLL_MPU, outputDivisor);

            OUTREG32(&g_pPrcmRestore->CM_CLKEN_PLL_MPU, cm_clken_pll);
            OUTREG32(&g_pPrcmRestore->CM_CLKSEL1_PLL_MPU, cm_clksel_pll);
            OUTREG32(&g_pPrcmRestore->CM_CLKSEL2_PLL_MPU, outputDivisor);
            break;

        case kDPLL2:
            // No dPLL2 on AM3517
            break;

        case kDPLL3:
            cm_clken_pll = INREG32(&g_pPrcmCm->pOMAP_CLOCK_CONTROL_CM->CM_CLKEN_PLL);
            cm_clksel_pll = INREG32(&g_pPrcmCm->pOMAP_CLOCK_CONTROL_CM->CM_CLKSEL1_PLL);

            // update frequency selection and frequency
            if (g_dwCpuFamily != CPU_FAMILY_DM37XX)
			    {
            cm_clken_pll &= ~DPLL_FREQSEL_MASK;
            cm_clken_pll |= DPLL_FREQSEL(_dpll_3.freqSelection)  /*|(0x3 << 8) | (1 << 3)*/;
                }
            cm_clksel_pll &= ~DPLL_CORE_CLKOUT_DIV_MASK;
            cm_clksel_pll &= ~((DPLL_MULT_MASK | DPLL_DIV_MASK) << DPLL_CORE_CLKSEL_SHIFT);
            cm_clksel_pll |= (DPLL_MULT(_dpll_3.multiplier) | DPLL_DIV(_dpll_3.divisor)) << DPLL_CORE_CLKSEL_SHIFT;
            cm_clksel_pll |= _dpll_3.outputDivisor << DPLL_CORE_CLKOUT_DIV_SHIFT;

            //  Need to perform this in SRAM
            bEnable = INTERRUPTS_ENABLE(FALSE);   

            sdrcRfrCtrl0 = INREG32(&g_pSDRCRegs->SDRC_RFR_CTRL_0);
            sdrcRfrCtrl1 = INREG32(&g_pSDRCRegs->SDRC_RFR_CTRL_1);

            sdrcRfrCtrl0 &= ~SDRC_RFR_CTRL_ARCV_MASK;
            sdrcRfrCtrl1 &= ~SDRC_RFR_CTRL_ARCV_MASK;
            
            // Update the ARCV before updating the core frequency
            if (_dpll_3.outputDivisor == 1)
                {
                // Core @ 332 MHZ
                sdrcRfrCtrl0 |= (g_pCPUInfo->SDRC_HIGH_RFR_FREQ << SDRC_RFR_CTRL_ARCV_SHIFT);
                sdrcRfrCtrl1 |= (g_pCPUInfo->SDRC_HIGH_RFR_FREQ << SDRC_RFR_CTRL_ARCV_SHIFT);
                g_pCPUInfo->DPLL_ARGS[0] = DVFS_HIGH_OPP_STALL;
                }
            else
                {
                // Core @ 166 MHZ
                sdrcRfrCtrl0 |= (g_pCPUInfo->SDRC_LOW_RFR_FREQ << SDRC_RFR_CTRL_ARCV_SHIFT);
                sdrcRfrCtrl1 |= (g_pCPUInfo->SDRC_LOW_RFR_FREQ << SDRC_RFR_CTRL_ARCV_SHIFT);
                g_pCPUInfo->DPLL_ARGS[0] = DVFS_LOW_OPP_STALL;
                }
            
            OUTREG32(&g_pSDRCRegs->SDRC_RFR_CTRL_0, sdrcRfrCtrl0);
            OUTREG32(&g_pSDRCRegs->SDRC_RFR_CTRL_1, sdrcRfrCtrl1);

            OMAP_PROFILE_MARK(PROFILE_CORE1_DVFS_BEGIN, 0);
            fnOALUpdateCoreFreq(g_pCPUInfo, cm_clken_pll, cm_clksel_pll);
            OMAP_PROFILE_MARK(PROFILE_CORE1_DVFS_END, 0);
            INTERRUPTS_ENABLE(bEnable);

            // save updated frequencies
            OUTREG32(&g_pPrcmRestore->CM_CLKEN_PLL, cm_clken_pll);
            OUTREG32(&g_pPrcmRestore->CM_CLKSEL1_PLL, cm_clksel_pll);

            // save updated sdram timings
            OUTREG32(&g_pSdrcRestore->DLLA_CTRL, INREG32(&g_pSDRCRegs->SDRC_DLLA_CTRL));
            OUTREG32(&g_pSdrcRestore->RFR_CTRL_0, INREG32(&g_pSDRCRegs->SDRC_RFR_CTRL_0));
            OUTREG32(&g_pSdrcRestore->RFR_CTRL_1, INREG32(&g_pSDRCRegs->SDRC_RFR_CTRL_1));
            break;

        case kDPLL4:
            cm_clken_pll = INREG32(&g_pPrcmCm->pOMAP_CLOCK_CONTROL_CM->CM_CLKEN_PLL);
            cm_clksel_pll = INREG32(&g_pPrcmCm->pOMAP_CLOCK_CONTROL_CM->CM_CLKSEL2_PLL);

            // update frequency selection and frequency
            if (g_dwCpuFamily != CPU_FAMILY_DM37XX)
			    {
            cm_clken_pll &= ~(DPLL_FREQSEL_MASK << DPLL_PER_MODE_SHIFT);
            cm_clken_pll |= DPLL_FREQSEL(_dpll_4.freqSelection) << DPLL_PER_MODE_SHIFT;
                }
            cm_clksel_pll &= ~(DPLL_MULT_MASK | DPLL_DIV_MASK);
            cm_clksel_pll |= DPLL_MULT(_dpll_4.multiplier) | DPLL_DIV(_dpll_4.divisor);

            OUTREG32(&g_pPrcmCm->pOMAP_CLOCK_CONTROL_CM->CM_CLKEN_PLL, cm_clken_pll);            
            OUTREG32(&g_pPrcmCm->pOMAP_CLOCK_CONTROL_CM->CM_CLKSEL2_PLL, cm_clksel_pll);

            OUTREG32(&g_pPrcmRestore->CM_CLKEN_PLL, cm_clken_pll);
            OUTREG32(&g_pPrcmRestore->CM_CLKSEL2_PLL, cm_clksel_pll);
            break;

        case kDPLL5:
            cm_clken_pll = INREG32(&g_pPrcmCm->pOMAP_CLOCK_CONTROL_CM->CM_CLKEN2_PLL);
            cm_clksel_pll = INREG32(&g_pPrcmCm->pOMAP_CLOCK_CONTROL_CM->CM_CLKSEL4_PLL);

            // update frequency selection and frequency
            if (g_dwCpuFamily != CPU_FAMILY_DM37XX)
			    {
                cm_clken_pll &= ~(DPLL_FREQSEL_MASK);
                cm_clken_pll |= DPLL_FREQSEL(_dpll_5.freqSelection);
                }
            cm_clksel_pll &= ~(DPLL_MULT_MASK | DPLL_DIV_MASK);
            cm_clksel_pll |= DPLL_MULT(_dpll_5.multiplier) | DPLL_DIV(_dpll_5.divisor);

            OUTSYSREG32(OMAP_PRCM_CLOCK_CONTROL_CM_REGS, CM_CLKEN2_PLL, cm_clken_pll);
            OUTREG32(&g_pPrcmCm->pOMAP_CLOCK_CONTROL_CM->CM_CLKSEL4_PLL, cm_clksel_pll);
            // OMAP_PRCM_RESTORE_REGS does not have element for CM_CLKSEL4_PLL
            //OUTREG32(&g_pPrcmRestore->CM_CLKSEL4_PLL, cm_clksel_pll);
            break;
        }
}

//-----------------------------------------------------------------------------
BOOL
ClockInitialize()
{
    BOOL rc = TRUE;
    UINT i;
    UINT cm_clken_pll;
    UINT cm_clksel_pll;
    UINT cm_autoidle_pll;
    UINT outputDivisor;
    UINT cm_clksel_per;
    UINT parentClock = 0;
    SrcClockDivisorTable_t *pDivisors;

    OALMSG(OAL_FUNC, (L"+ClockInitialize()\r\n"));

    // initialize dpll settings with what's current set in hw

    // dpll 1    
    cm_clken_pll = INREG32(&g_pPrcmCm->pOMAP_MPU_CM->CM_CLKEN_PLL_MPU);
    cm_clksel_pll = INREG32(&g_pPrcmCm->pOMAP_MPU_CM->CM_CLKSEL1_PLL_MPU);
    cm_autoidle_pll = INREG32(&g_pPrcmCm->pOMAP_MPU_CM->CM_AUTOIDLE_PLL_MPU);
    outputDivisor = INREG32(&g_pPrcmCm->pOMAP_MPU_CM->CM_CLKSEL2_PLL_MPU) >> DPLL_MPU_CLKOUT_DIV_SHIFT;
    _ClockInitialize(&_dpll_1, cm_clken_pll, cm_clksel_pll, 
        cm_autoidle_pll, outputDivisor);

    // dpll 3
    cm_clken_pll = INREG32(&g_pPrcmCm->pOMAP_CLOCK_CONTROL_CM->CM_CLKEN_PLL);
    cm_clksel_pll = INREG32(&g_pPrcmCm->pOMAP_CLOCK_CONTROL_CM->CM_CLKSEL1_PLL) >> DPLL_CORE_CLKSEL_SHIFT;
    cm_autoidle_pll = INREG32(&g_pPrcmCm->pOMAP_CLOCK_CONTROL_CM->CM_AUTOIDLE_PLL);
    outputDivisor = INREG32(&g_pPrcmCm->pOMAP_CLOCK_CONTROL_CM->CM_CLKSEL1_PLL) >> DPLL_CORE_CLKOUT_DIV_SHIFT;
    _ClockInitialize(&_dpll_3, cm_clken_pll, cm_clksel_pll, 
        cm_autoidle_pll, outputDivisor);

    // dpll 4
    outputDivisor = (UINT)-1;
    cm_clken_pll = INREG32(&g_pPrcmCm->pOMAP_CLOCK_CONTROL_CM->CM_CLKEN_PLL) >> DPLL_PER_MODE_SHIFT;
    cm_clksel_pll = INREG32(&g_pPrcmCm->pOMAP_CLOCK_CONTROL_CM->CM_CLKSEL2_PLL);
    cm_autoidle_pll = INREG32(&g_pPrcmCm->pOMAP_CLOCK_CONTROL_CM->CM_AUTOIDLE_PLL) >> DPLL_PER_MODE_SHIFT;
    _ClockInitialize(&_dpll_4, cm_clken_pll, cm_clksel_pll, 
        cm_autoidle_pll, outputDivisor);

    // dpll 5
    outputDivisor = (UINT)-1;
    cm_clken_pll = INREG32(&g_pPrcmCm->pOMAP_CLOCK_CONTROL_CM->CM_CLKEN2_PLL);
    cm_clksel_pll = INREG32(&g_pPrcmCm->pOMAP_CLOCK_CONTROL_CM->CM_CLKSEL4_PLL);
    cm_autoidle_pll = INREG32(&g_pPrcmCm->pOMAP_CLOCK_CONTROL_CM->CM_AUTOIDLE2_PLL);
    _ClockInitialize(&_dpll_5, cm_clken_pll, cm_clksel_pll, 
        cm_autoidle_pll, outputDivisor);

    // Save dss clksel configuration
    pDivisors = s_SrcClockTable[kDSS1_ALWON_FCLK].pDivisors;
    pDivisors->SourceClock[0].divisor = CLKSEL_DSS1(INREG32(&g_pPrcmCm->pOMAP_DSS_CM->CM_CLKSEL_DSS));

    // Save Tv out clksel configuration
    pDivisors = s_SrcClockTable[k54M_FCLK].pDivisors;
    pDivisors->SourceClock[0].divisor = CLKSEL_TV(INREG32(&g_pPrcmCm->pOMAP_DSS_CM->CM_CLKSEL_DSS));

    // set GPTimer2(used as high perfomance timer)  to use SYSCLK
    SETREG32(&g_pPrcmCm->pOMAP_PER_CM->CM_CLKSEL_PER, CLKSEL_GPT2);

    // Save  PER CLKSEL configuration    
    cm_clksel_per = INREG32(&g_pPrcmCm->pOMAP_PER_CM->CM_CLKSEL_PER);
    s_SrcClockTable[kGPT2_ALWON_FCLK].parentClk = (cm_clksel_per & CLKSEL_GPT2) ? kSYS_CLK : k32K_FCLK;
    s_SrcClockTable[kGPT3_ALWON_FCLK].parentClk = (cm_clksel_per & CLKSEL_GPT3) ? kSYS_CLK : k32K_FCLK;
    s_SrcClockTable[kGPT4_ALWON_FCLK].parentClk = (cm_clksel_per & CLKSEL_GPT4) ? kSYS_CLK : k32K_FCLK;
    s_SrcClockTable[kGPT5_ALWON_FCLK].parentClk = (cm_clksel_per & CLKSEL_GPT5) ? kSYS_CLK : k32K_FCLK;
    s_SrcClockTable[kGPT6_ALWON_FCLK].parentClk = (cm_clksel_per & CLKSEL_GPT6) ? kSYS_CLK : k32K_FCLK;
    s_SrcClockTable[kGPT7_ALWON_FCLK].parentClk = (cm_clksel_per & CLKSEL_GPT7) ? kSYS_CLK : k32K_FCLK;
    s_SrcClockTable[kGPT8_ALWON_FCLK].parentClk = (cm_clksel_per & CLKSEL_GPT8) ? kSYS_CLK : k32K_FCLK;
    s_SrcClockTable[kGPT9_ALWON_FCLK].parentClk = (cm_clksel_per & CLKSEL_GPT9) ? kSYS_CLK : k32K_FCLK;    

    // Save Sgx clksel configuration
    cm_clksel_pll = INREG32(&g_pPrcmCm->pOMAP_SGX_CM->CM_CLKSEL_SGX);
    switch (cm_clksel_pll & CLKSEL_SGX_MASK)
        {
        case 0:
            parentClock = kCORE_CLK;
            outputDivisor = 3;
            break;

        case 1:
            parentClock = kCORE_CLK;
            outputDivisor = 4;
            break;

        case 2:
            parentClock = kCORE_CLK;
            outputDivisor = 6;
            break;

        case 3:
            parentClock = kCM_96M_FCLK;
            outputDivisor = 1;
            break;

        case 4:
            parentClock = kPRM_96M_192M_ALWON_CLK;
            outputDivisor = 1;
            break;

        case 5:
            parentClock = kCORE_CLK;
            outputDivisor = 2;
            break;

        case 6:
            parentClock = kCOREX2_FCLK;
            outputDivisor = 3;
            break;

        case 7:
            parentClock = kCOREX2_FCLK;
            outputDivisor = 5;
            break;
        }

    pDivisors = s_SrcClockTable[kSGX_FCLK].pDivisors;
    for (i = 0; i < pDivisors->count; ++i)
        {
        if (parentClock == pDivisors->SourceClock[i].id)
            {
            s_SrcClockTable[kSGX_FCLK].parentClk = parentClock;
            pDivisors->SourceClock[i].divisor = outputDivisor;
            break;
            }
        }

    OALMSG(OAL_FUNC, (L"-_ClockInitialize()=%d\r\n", rc));
    return rc;    
}

//-----------------------------------------------------------------------------
BOOL
ClockUpdateParentClock(
    int srcClkId,
    BOOL bEnable
    )
{
    BOOL rc = FALSE;
    if (!g_bSingleThreaded)
        OALMSG(OAL_FUNC, (L"+ClockUpdateParentClock"
            L"(srcClkId=%d, bEnable=%d)\r\n", srcClkId, bEnable)
            );

    // quick check for valid source clock id's
    if ((UINT)srcClkId > kSOURCE_CLOCK_COUNT) goto cleanUp;
    // check if clock is being enabled/disabled
    if ((s_SrcClockTable[srcClkId].refCount == 0 && bEnable == TRUE) ||
        (s_SrcClockTable[srcClkId].refCount == 1 && bEnable == FALSE))
        {         
        if (s_SrcClockTable[srcClkId].bIsDpllSrcClk == TRUE)
            {
            // update dpll output clocks
            _ClockUpdateDpllOutput(s_SrcClockTable[srcClkId].parentClk, bEnable);
            }
        else
            {
            ClockUpdateParentClock(s_SrcClockTable[srcClkId].parentClk, bEnable);
            }
        }
    // increment src clk refCount
    s_SrcClockTable[srcClkId].refCount += (bEnable != FALSE) ? 1 : -1;
    
    rc = TRUE;
cleanUp:    
    if (!g_bSingleThreaded)
        OALMSG(OAL_FUNC, (L"-ClockUpdateParentClock()=%d\r\n", rc));
    return rc;
}

//-----------------------------------------------------------------------------
BOOL
PrcmClockSetDivisor(
    UINT clockId,
    UINT parentClockId,
    UINT divisor
    )
{
    static BYTE _usim_sysclk_map[] = {(BYTE)-1, (BYTE)1, (BYTE)2};
    static BYTE _sgx_core_clk[]             = {(BYTE)-1, (BYTE)-1, (BYTE)5, (BYTE)0, (BYTE)1, (BYTE)-1, (BYTE)2};
    static BYTE _sgx_96M_fclk[]             = {(BYTE)-1, (BYTE)3};
    static BYTE _sgx_96M_192_alwon_clk[]    = {(BYTE)-1, (BYTE)4};
    static BYTE _sgx_corex2_fclk[]          = {(BYTE)-1, (BYTE)-1, (BYTE)-1, (BYTE)6, (BYTE)-1, (BYTE)7};
    static BYTE _usim_96m_map[] =    {(BYTE)-1, (BYTE)-1, (BYTE)3, (BYTE)-1, (BYTE)4, (BYTE)-1, (BYTE)-1, (BYTE)-1, (BYTE)5, (BYTE)-1, (BYTE)6};
    static BYTE _usim_120m_map[] =   {(BYTE)-1, (BYTE)-1, (BYTE)-1, (BYTE)-1,  (BYTE)7, (BYTE)-1, (BYTE)-1, (BYTE)-1,  (BYTE)8, (BYTE)-1, 
                                      (BYTE)-1, (BYTE)-1, (BYTE)-1, (BYTE)-1, (BYTE)-1, (BYTE)-1,  (BYTE)9, (BYTE)-1, (BYTE)-1, (BYTE)-1,
                                      (BYTE)10};

    UINT i;
    BYTE real_divisor = 0;
    BOOL rc = FALSE;
    SrcClockDivisorTable_t *pDivisors;
    
    OALMSG(OAL_FUNC, (L"+PrcmClockSetDivisor"
        L"(clockId=%d, parentClockId=%d, divisor=%d)\r\n", 
        clockId, parentClockId, divisor)
        );

    pDivisors = s_SrcClockTable[clockId].pDivisors;

    // get sync handle
    Lock(Mutex_Clock);
    
    switch (clockId)
        {            
        case kSSI_SSR_FCLK:
            // validate parameters
            if (parentClockId != pDivisors->SourceClock[0].id || divisor > 8) goto cleanUp;

            // store divisor settings
            pDivisors->SourceClock[0].divisor = divisor;
            rc = _ClockHwUpdateParentClock(clockId);
            break;

        case kDSS1_ALWON_FCLK:
            // validate parameters
            if (parentClockId != pDivisors->SourceClock[0].id || divisor > 16) goto cleanUp;

            // store divisor settings
            pDivisors->SourceClock[0].divisor = divisor;
            rc = _ClockHwUpdateParentClock(clockId);
            break;

        case k54M_FCLK:
            // validate parameters
            if (parentClockId != pDivisors->SourceClock[0].id || divisor > 16) goto cleanUp;

            // store divisor settings
            pDivisors->SourceClock[0].divisor = divisor;
            rc = _ClockHwUpdateParentClock(clockId);
            break;

        case kSGX_FCLK:
            switch (parentClockId)
                {
                case kCORE_CLK:
                    if (divisor >= sizeof(_sgx_core_clk) || _sgx_core_clk[divisor] == -1) goto cleanUp;
                    real_divisor = _sgx_core_clk[divisor];
                    break;

                case kCM_96M_FCLK:
                    if (divisor >= sizeof(_sgx_96M_fclk) || _sgx_96M_fclk[divisor] == -1) goto cleanUp;
                     real_divisor = _sgx_96M_fclk[divisor];
                    break;

                case kPRM_96M_192M_ALWON_CLK:
                    if (divisor >= sizeof(_sgx_96M_192_alwon_clk) || _sgx_96M_192_alwon_clk[divisor] == -1) goto cleanUp;
                    real_divisor = _sgx_96M_192_alwon_clk[divisor];
                    break;

                case kCOREX2_FCLK:
                    if (divisor >= sizeof(_sgx_corex2_fclk) || _sgx_corex2_fclk[divisor] == -1) goto cleanUp;
                    real_divisor = _sgx_corex2_fclk[divisor];
                    break;
                }

            // store divisor settings
            for (i = 0; i < pDivisors->count; ++i)
                {
                if (parentClockId == pDivisors->SourceClock[i].id)
                    {
                    pDivisors->SourceClock[i].divisor = real_divisor;
                    break;
                    }
                }
            rc = _ClockHwUpdateParentClock(clockId);
            break;

        case kUSIM_FCLK:
            // validate parameters
            if (parentClockId != pDivisors->SourceClock[0].id || 
                divisor >= sizeof(_usim_sysclk_map) || 
                _usim_sysclk_map[divisor] == -1) 
                {
                goto cleanUp;
                }

            // store divisor settings
            pDivisors->SourceClock[0].divisor = _usim_sysclk_map[divisor];
            rc = _ClockHwUpdateParentClock(clockId);
            break;

        case kCM_USIM_CLK:

            if (parentClockId == pDivisors->SourceClock[0].id)
                {
                if (divisor >= sizeof(_usim_96m_map) || _usim_96m_map[divisor] == -1) goto cleanUp;

                pDivisors->SourceClock[0].divisor = _usim_96m_map[divisor];
                }
            else if (parentClockId == pDivisors->SourceClock[1].id)
                {
                if (divisor >= sizeof(_usim_120m_map) || _usim_120m_map[divisor] == -1) goto cleanUp;

                pDivisors->SourceClock[0].divisor = _usim_120m_map[divisor];
                }
            else
                {
                goto cleanUp;
                }
            
            rc = _ClockHwUpdateParentClock(clockId);
            break;            
        }

cleanUp: 
    
    // release sync handle    
    Unlock(Mutex_Clock);
    OALMSG(OAL_FUNC, (L"-PrcmClockSetDivisor()=%d\r\n", rc));
    return rc;
}

//-----------------------------------------------------------------------------
BOOL
PrcmClockSetParent(
    UINT clockId,
    UINT newParentClockId
    )
{    
    BOOL rc = FALSE;
    UINT oldParentClockId;
    OALMSG(OAL_FUNC, (L"+PrcmClockSetParent"
        L"(clockId=%d, newParentClockId=%d)\r\n", clockId, newParentClockId)
        );

    // quick check for valid source clock id's
    if (clockId > kSOURCE_CLOCK_COUNT) goto cleanUp;

    // get sync handle
    Lock(Mutex_Clock);
    oldParentClockId = s_SrcClockTable[clockId].parentClk;
    switch (clockId)
        {
        case kGPT1_FCLK:
        case kGPT2_ALWON_FCLK:
        case kGPT3_ALWON_FCLK:
        case kGPT4_ALWON_FCLK:
        case kGPT5_ALWON_FCLK:
        case kGPT6_ALWON_FCLK:
        case kGPT7_ALWON_FCLK:
        case kGPT8_ALWON_FCLK:
        case kGPT9_ALWON_FCLK:
            // verify parent clock is valid
            switch(newParentClockId)
                {
                case kSYS_CLK:
                case k32K_FCLK:                    
                    break;

                default: 
                    goto cleanUp;
                }
            break;
            
        case kGPT10_FCLK:
        case kGPT11_FCLK:
            // verify parent clock is valid
            switch(newParentClockId)
                {
                case kCM_SYS_CLK:
                case kCM_32K_CLK:                    
                    break;

                default: 
                    goto cleanUp;
                }
            break;

        case kSGX_FCLK:
            // verify parent clock is valid
            switch(newParentClockId)
                {
                case kCORE_CLK:
                case kCM_96M_FCLK:                    
                case kCOREX2_FCLK:
                case kPRM_96M_192M_ALWON_CLK:
                    break;

                default: 
                    goto cleanUp;
                }
            break;

        case k54M_FCLK:
            // verify parent clock is valid
            switch(newParentClockId)
                {
                case kDPLL4_M3X2_CLK:
                case kSYS_ALTCLK:                    
                    break;

                default: 
                    goto cleanUp;
                }
            break;

        case k48M_FCLK:
            // verify parent clock is valid
            switch(newParentClockId)
                {
                case kCM_96M_FCLK:
                case kSYS_ALTCLK:                    
                    break;

                default: 
                    goto cleanUp;
                }
            break;

        case k96M_FCLK:
            // verify parent clock is valid
            switch(newParentClockId)
                {
                case kCM_96M_FCLK:
                case kCM_SYS_CLK:                    
                    break;

                default:
                    goto cleanUp;
                }
            break;

        case kUSIM_FCLK:  
            // verify parent clock is valid
            switch(newParentClockId)
                {
                case kSYS_CLK:
                case kCM_USIM_CLK:                    
                    break;

                default: 
                    goto cleanUp;
                }
            break;

        case kCM_USIM_CLK:  
            // verify parent clock is valid
            switch(newParentClockId)
                {
                case kCM_96M_FCLK:
                case k120M_FCLK:                    
                    break;

                default: 
                    goto cleanUp;
                }
            break;

        case kMCBSP1_CLKS:
        case kMCBSP5_CLKS:
            // verify parent clock is valid
            switch(newParentClockId)
                {
                case kCORE_96M_FCLK:
                case kMCBSP_CLKS:
                    break;

                default:
                    goto cleanUp;
                }
            break;

        case kMCBSP2_CLKS:
        case kMCBSP3_CLKS:
        case kMCBSP4_CLKS:
            // verify parent clock is valid
            switch(newParentClockId)
                {
                case k96M_ALWON_FCLK:
                case kMCBSP_CLKS:
                    break;

                default:
                    goto cleanUp;
                }
            break;

        }

    // check if clock is enabled.  If so then release old src clocks
    // and enable new ones
    if (s_SrcClockTable[clockId].refCount > 0)
        {
        ClockUpdateParentClock(newParentClockId, TRUE);
        ClockUpdateParentClock(oldParentClockId, FALSE);
        }

    // update state tree
    s_SrcClockTable[clockId].parentClk = newParentClockId;
    rc = _ClockHwUpdateParentClock(clockId);
    
            
cleanUp:    
    // release sync handle    
    Unlock(Mutex_Clock);
    OALMSG(OAL_FUNC, (L"-PrcmClockSetParent()=%d\r\n", TRUE));
    return TRUE;
}

//-----------------------------------------------------------------------------
BOOL
PrcmClockGetParentClockRefcount(
    UINT     clockId,
    UINT     nLevel,
    LONG    *pRefCount
    )
{
    BOOL rc = FALSE;
    OALMSG(OAL_FUNC, (L"+PrcmClockGetParentClockRefcount"
        L"(clockId=%d, nLevel=%d)\r\n", clockId, nLevel)
        );

    switch (nLevel)
        {
        case 1:
            if (clockId < kSOURCE_CLOCK_COUNT)
                {
                *pRefCount = s_SrcClockTable[clockId].refCount;
                rc = TRUE;
                }
            break;

        case 2:
            if (clockId < kDPLL_CLKOUT_COUNT)
                {
                *pRefCount = s_DpllClkOutTable[clockId].refCount;
                rc = TRUE;
                }
            break;

        case 3:
            if (clockId < kDPLL_COUNT)
                {
                *pRefCount = s_DpllTable[clockId].refCount;
                rc = TRUE;
                }
            break;

        case 4:
            if (clockId < kVDD_COUNT)
                {
                *pRefCount = s_VddTable[clockId];
                rc = TRUE;
                }
            break;
        }
    

    OALMSG(OAL_FUNC, (L"+PrcmClockGetParentClockRefcount()=%d\r\n", rc));
    return rc;
}

//-----------------------------------------------------------------------------
BOOL
PrcmClockGetParentClockInfo(
    UINT                clockId,
    UINT                nLevel,
    SourceClockInfo_t  *pInfo
    )
{
    BOOL rc = FALSE;
    UINT parentClock;
    OALMSG(OAL_FUNC, (L"+PrcmClockGetParentClockInfo"
        L"(clockId=%d, nLevel=%d, pInfo=0x%08X)\r\n", clockId, nLevel, pInfo)
        );

    // get parent clock
    switch (nLevel)
        {
        case 1:            
            if (clockId < kSOURCE_CLOCK_COUNT)
                {
                parentClock = s_SrcClockTable[clockId].parentClk;
                if (s_SrcClockTable[clockId].bIsDpllSrcClk) ++nLevel;                    
                }
            else
                {
                goto cleanUp;
                }
            break;

        case 2:
            if (clockId < kDPLL_CLKOUT_COUNT)
                {
                parentClock = s_DpllClkOutTable[clockId].dpllDomain;
                ++nLevel;                    
                }
            else
                {
                goto cleanUp;
                }
            break;

        case 3:
            if (clockId < kDPLL_COUNT)
                {
                parentClock = s_DpllTable[clockId].vddDomain;
                ++nLevel;                    
                }
            else
                {
                goto cleanUp;
                }
            break;

        default:
            goto cleanUp;
        }


    // get parent information
    //
    pInfo->nLevel = nLevel;
    pInfo->clockId = parentClock;
    rc = PrcmClockGetParentClockRefcount(parentClock, nLevel, &pInfo->refCount);

cleanUp:    
    OALMSG(OAL_FUNC, (L"+PrcmClockGetParentClockInfo()=%d\r\n", rc));
    return rc;
}

//-----------------------------------------------------------------------------
BOOL
PrcmClockSetDpllState(
    IOCTL_PRCM_CLOCK_SET_DPLLSTATE_IN *pInfo
    )
{
    BOOL rc = FALSE;
    DpllState_t *pDpll;
    OALMSG(OAL_FUNC, (L"+PrcmClockSetDpllState(pInfo=0x%08X)\r\n", pInfo));

    Lock(Mutex_DeviceClock);
    if (pInfo->dpllId > kDPLL5) goto cleanUp;

    // get dpll pointer
    pDpll = s_DpllTable[pInfo->dpllId].pDpllInfo;
    if (pDpll == NULL) goto cleanUp;

    if (pInfo->ffMask & DPLL_UPDATE_LPMODE)
        {
        pDpll->lowPowerEnabled = pInfo->lowPowerEnabled ? 0x01 : 0;
        }

    if (pInfo->ffMask & DPLL_UPDATE_DRIFTGUARD)
        {
        pDpll->driftGuard = pInfo->driftGuardEnabled ? 0x01 : 0;
        }

    if (pInfo->ffMask & DPLL_UPDATE_RAMPTIME)
        {
        pDpll->rampTime = (pInfo->rampTime & DPLL_RAMPTIME_MASK) >> DPLL_RAMPTIME_SHIFT;               
        }

    if (pInfo->ffMask & DPLL_UPDATE_DPLLMODE)
        {
        pDpll->dpllMode = (pInfo->dpllMode & DPLL_MODE_MASK) >> DPLL_MODE_SHIFT;
        }

    if (pInfo->ffMask & DPLL_UPDATE_AUTOIDLEMODE)
        {
        pDpll->dpllAutoidleState = (pInfo->dpllAutoidleState & DPLL_AUTOIDLE_MASK) >> DPLL_AUTOIDLE_SHIFT;
        }
    
    rc = _ClockHwUpdateDpllState(pInfo->dpllId, pInfo->ffMask);
    

cleanUp:    
    Unlock(Mutex_DeviceClock);
    OALMSG(OAL_FUNC, (L"+PrcmClockSetDpllState()=%d\r\n", rc));
    return rc;
}

//-----------------------------------------------------------------------------
BOOL
PrcmClockSetDpllFrequency(
    UINT dpllId,
    UINT m,
    UINT n,
    UINT freqSel,
    UINT outputDivisor
    )
{
    BOOL rc = FALSE;
    DpllState_t *pDpll;
    OALMSG(OAL_FUNC, (L"+PrcmClockSetDpllFrequency"
        L"(dpllId=%d, m=%d, n=%d, freqSel=%d)\r\n", 
        dpllId, m, n, freqSel
        ));

    Lock(Mutex_DeviceClock);
    if (dpllId > kDPLL5) goto cleanUp;
    pDpll = s_DpllTable[dpllId].pDpllInfo;

    // check if parameters are already set
    if (pDpll->freqSelection != freqSel ||
        pDpll->divisor != n             ||
        pDpll->multiplier != m          ||
        pDpll->outputDivisor != outputDivisor)
        {
        // frequency info
        pDpll->freqSelection = freqSel;
        pDpll->divisor = n;
        pDpll->multiplier = m;
        pDpll->outputDivisor = outputDivisor;
    
        _ClockHwUpdateDpllFrequency(dpllId);
        }

    rc = TRUE;
    
cleanUp:    
    Unlock(Mutex_DeviceClock);
    OALMSG(OAL_FUNC, (L"+PrcmClockSetDpllFrequency()=%d\r\n", rc));
    return rc;
}    

//-----------------------------------------------------------------------------
BOOL
PrcmClockSetDpllAutoIdleState(
    UINT dpllId,
    UINT dpllAutoidleState
    )
{
    BOOL rc = FALSE;
    DpllState_t *pDpll;
    OALMSG(OAL_FUNC, (L"+PrcmClockSetDpllAutoIdleState"
        L"(dpllId=%d, dpllAutoidleState=0x%08X)\r\n", 
        dpllId, dpllAutoidleState)
        );

    if (dpllId > kVDD5) goto cleanUp;
    
    Lock(Mutex_DeviceClock);
    pDpll = s_DpllTable[dpllId].pDpllInfo;
    pDpll->dpllAutoidleState = (dpllAutoidleState & DPLL_AUTOIDLE_MASK) >> DPLL_AUTOIDLE_SHIFT;
    rc = _ClockHwUpdateDpllState(dpllId, DPLL_UPDATE_AUTOIDLEMODE);    
    Unlock(Mutex_DeviceClock);
    
cleanUp:    
    
    OALMSG(OAL_FUNC, (L"+PrcmClockSetDpllAutoIdleState()=%d\r\n", rc));
    return rc;
}

//------------------------------------------------------------------------------
BOOL
PrcmClockSetExternalClockRequestMode(
    UINT extClkReqMode
    )
{    
    UINT val;
    
    // acquire sync handle   
    Lock(Mutex_Clock);

    extClkReqMode &= AUTOEXTCLKMODE_MASK;
    val = INREG32(&g_pPrcmPrm->pOMAP_GLOBAL_PRM->PRM_CLKSRC_CTRL) & ~AUTOEXTCLKMODE_MASK;
    val |= extClkReqMode;
    OUTREG32(&g_pPrcmPrm->pOMAP_GLOBAL_PRM->PRM_CLKSRC_CTRL, val);
    
    OUTREG32(&g_pPrcmRestore->PRM_CLKSRC_CTRL, val);
    
    // release sync handle    
    Unlock(Mutex_Clock);
    
    return TRUE;
}

//------------------------------------------------------------------------------
BOOL
PrcmClockSetSystemClockSetupTime(
    USHORT  setupTime
    )
{    
    UNREFERENCED_PARAMETER(setupTime);

    // acquire sync handle   
    Lock(Mutex_Clock);

    // release sync handle    
    Unlock(Mutex_Clock);
    
    return TRUE;
}

//-----------------------------------------------------------------------------
float 
PrcmClockGetSystemClockFrequency()
{
    static float _rgSysClkTbl[6] = {12.0f, 13.0f, 19.2f, 26.0f, 38.4f, 16.8f};
    return _rgSysClkTbl[INREG32(&g_pPrcmPrm->pOMAP_CLOCK_CONTROL_PRM->PRM_CLKSEL)] * 1000000.0f;
}

//-----------------------------------------------------------------------------
UINT 
PrcmClockGetDpllState(
    UINT dpllId
    )
{
    UINT val = 0; 
    OMAP_CM_REGS *pPrcmCm;
    OALMSG(OAL_FUNC, (L"+PrcmClockGetDpllState(dpllId=%d)\r\n", dpllId));

    switch (dpllId)
        {
        case kDPLL1:
            pPrcmCm = GetCmRegisterSet(POWERDOMAIN_MPU);
            break;

        default:
            goto cleanUp;
        }

    val = INREG32(&pPrcmCm->CM_IDLEST_PLL_xxx);

cleanUp:
    OALMSG(OAL_FUNC, (L"-PrcmClockGetDpllState()=0x%08X\r\n", val));
    return val;
}

//-----------------------------------------------------------------------------
VOID
PrcmClockRestoreDpllState(
    UINT dpll
    )
{
    // Set the DPLL frequency
    _ClockHwUpdateDpllFrequency(dpll);

    // Update the dpll state
    _ClockHwUpdateDpllState(dpll, DPLL_UPDATE_ALL);
}

//-----------------------------------------------------------------------------
UINT32 
PrcmClockGetClockRate(
    OMAP_CLOCKID clock_id
    )
{
    DWORD val;
    DWORD val2;
    float sys_clk, freq=0, core_clk;

    // get virtual address mapping to register
    val = INREG32(&g_pPrcmPrm->pOMAP_CLOCK_CONTROL_PRM->PRM_CLKSEL);
    switch (val)
    {
        case 0:
            sys_clk = 12.0f;
            break;

        case 1:
            sys_clk = 13.0f;
            break;

        case 2:
            sys_clk = 19.2f;
            break;

        case 3:
            sys_clk = 26.0f;
            break;

        case 4:
            sys_clk = 38.4f;
            break;

        case 5:
            sys_clk = 16.8f;
            break;

        default:
            goto cleanUp;                
    }
    
    switch(clock_id)
    {
        case SYS_CLK:
        freq = sys_clk;
        break;
        
     case MPU_CLK:
           // get mpu frequency
           val = INREG32(&g_pPrcmCm->pOMAP_MPU_CM->CM_CLKSEL1_PLL_MPU);
           val2 = INREG32(&g_pPrcmCm->pOMAP_MPU_CM->CM_CLKSEL2_PLL_MPU);
           freq = ((float)((val & DPLL_MULT_MASK)>>DPLL_MULT_SHIFT) * sys_clk) /
                        ((float)((val & DPLL_DIV_MASK) >> DPLL_DIV_SHIFT) + 1.0f);
           freq = freq / ((float)(val2 >> DPLL_MPU_CLKOUT_DIV_SHIFT));
        break;
        
        
     case CORE_CLK:
        // get core frequency
        val = INREG32(&g_pPrcmCm->pOMAP_CLOCK_CONTROL_CM->CM_CLKSEL1_PLL) >> 8;
        freq = ((float)((val & DPLL_MULT_MASK)>>DPLL_MULT_SHIFT) * sys_clk) /
                        ((float)((val & DPLL_DIV_MASK) >> DPLL_DIV_SHIFT) + 1.0f);
        freq = freq / ((float)(val >> (DPLL_CORE_CLKOUT_DIV_SHIFT - 8)));
        break;

     case SGX_CLK:
        val = INREG32(&g_pPrcmCm->pOMAP_CLOCK_CONTROL_CM->CM_CLKSEL1_PLL) >> 8;
        core_clk = ((float)((val & DPLL_MULT_MASK)>>DPLL_MULT_SHIFT) * sys_clk) /
                        ((float)((val & DPLL_DIV_MASK) >> DPLL_DIV_SHIFT) + 1.0f);
        core_clk = core_clk / ((float)(val >> (DPLL_CORE_CLKOUT_DIV_SHIFT - 8)));
        
        // get core frequency
        val = INREG32(&g_pPrcmCm->pOMAP_SGX_CM->CM_CLKSEL_SGX) & 0x7;
           switch(val){
        case 0: 
        /* CORE_CLK / 3*/
            freq = core_clk/3;
            break;
        case 1:
        /* CORE_CLK / 4 */
            freq = core_clk/4;
            break;
        case 2:
        /* CORE_CLK / 6 */
            freq = core_clk/6;
            break;
        case 3:
        /* CM_96M_FCLK */
            freq = 96;
            break;
        case 4:
        /* SGX_192M_FCLK */
            freq = 192;
            break;          
        case 5:
        /* CORE_CLK / 2 */
            freq = core_clk/2;
            break;
        case 6:
        /* CORE_CLK * 2 / 3 */
            freq = core_clk * 2 / 3;
            break;
        case 7:
        /* CORE_CLK * 2 / 5 */
            freq = core_clk * 2 /5;
            break;
            
            }
        break;
        
    default:
        freq = 0;
        break;
    }
    
cleanUp:    
    return (UINT32)freq;
}


#ifndef SHIP_BUILD

//-----------------------------------------------------------------------------

// vdd names
PTCHAR s_VddNameTable[] = {    
    L"kVDD1",
    L"kVDD2",
    L"kVDD3",
    L"kVDD4",
    L"kVDD5",
    L"kVDD_EXT",
    L"kVDDS",
    L"kVDDPLL",
    L"kVDDADAC",
    L"kMMC_VDDS"
};

// dpll names
PTCHAR s_DpllNameTable[] = {
    L"kDPLL1",
    L"kDPLL2",
    L"kDPLL3",
    L"kDPLL4",
    L"kDPLL5",
    L"kDPLL_EXT"
};

// dpll clkout names
PTCHAR s_DpllClkOutNameTable[] = {
    L"kEXT_32KHZ",
    L"kDPLL1_CLKOUT_M2X2",
    L"kDPLL2_CLKOUT_M2",
    L"kDPLL3_CLKOUT_M2",
    L"kDPLL3_CLKOUT_M2X2",
    L"kDPLL3_CLKOUT_M3X2",
    L"kDPLL4_CLKOUT_M2X2",
    L"kDPLL4_CLKOUT_M3X2",
    L"kDPLL4_CLKOUT_M4X2",
    L"kDPLL4_CLKOUT_M5X2",
    L"kDPLL4_CLKOUT_M6X2",
    L"kDPLL5_CLKOUT_M2",
    L"kEXT_SYS_CLK",
    L"kEXT_ALT",
    L"kEXT_MISC",
    L"kINT_OSC"
};

// source clock names
PTCHAR s_SrcClockNameTable[] = {
    L"kDPLL1_M2X2_CLK",
    L"kDPLL2_M2_CLK",
    L"kCORE_CLK",
    L"kCOREX2_FCLK",
    L"kEMUL_CORE_ALWON_CLK",
    L"kPRM_96M_ALWON_CLK",
    L"kDPLL4_M3X2_CLK",
    L"kDSS1_ALWON_FCLK",
    L"kCAM_MCLK",
    L"kEMUL_PER_ALWON_CLK ",
    L"k120M_FCLK",
    L"k32K_FCLK",
    L"kSYS_CLK",
    L"kSYS_ALTCLK",
    L"kSECURE_32K_FCLK",
    L"kMCBSP_CLKS",
    L"kUSBTLL_SAR_FCLK",
    L"kUSBHOST_SAR_FCLK",
    L"kEFUSE_ALWON_FCLK",
    L"kSR_ALWON_FCLK",
    L"kDPLL1_ALWON_FCLK",
    L"kDPLL2_ALWON_FCLK",
    L"kDPLL3_ALWON_FCLK",
    L"kDPLL4_ALWON_FCLK",
    L"kDPLL5_ALWON_FCLK",
    L"kCM_SYS_CLK",
    L"kDSS2_ALWON_FCLK",
    L"kWKUP_L4_ICLK",
    L"kCM_32K_CLK",
    L"kCORE_32K_FCLK",
    L"kWKUP_32K_FCLK",
    L"kPER_32K_ALWON_FCLK",
    L"kCORE_120M_FCLK",
    L"kCM_96M_FCLK",
    L"k96M_ALWON_FCLK",
    L"kL3_ICLK",
    L"kL4_ICLK",
    L"kUSB_L4_ICLK",
    L"kRM_ICLK",
    L"kDPLL1_FCLK",
    L"kDPLL2_FCLK",
    L"kCORE_L3_ICLK",
    L"kCORE_L4_ICLK",
    L"kSECURITY_L3_ICLK",
    L"kSECURITY_L4_ICLK1",
    L"kSECURITY_L4_ICLK2",
    L"kSGX_L3_ICLK",
    L"kSSI_L4_ICLK",
    L"kDSS_L3_ICLK",
    L"kDSS_L4_ICLK",
    L"kCAM_L3_ICLK",
    L"kCAM_L4_ICLK",
    L"kUSBHOST_L3_ICLK",
    L"kUSBHOST_L4_ICLK",
    L"kPER_L4_ICLK",
    L"kSR_L4_ICLK",
    L"kSSI_SSR_FCLK",
    L"kSSI_SST_FCLK",
    L"kCORE_96M_FCLK",
    L"kDSS_96M_FCLK",
    L"kCSI2_96M_FCLK",
    L"kCORE_48M_FCLK",
    L"kUSBHOST_48M_FCLK",
    L"kPER_48M_FCLK",
    L"k12M_FCLK",
    L"kCORE_12M_FCLK",
    L"kDSS_TV_FCLK",
    L"kUSBHOST_120M_FCLK",
    L"kCM_USIM_CLK",
    L"kUSIM_FCLK",
    L"k96M_FCLK",
    L"k48M_FCLK",
    L"k54M_FCLK",
    L"kSGX_FCLK",
    L"kGPT1_FCLK",
    L"kGPT2_ALWON_FCLK",
    L"kGPT3_ALWON_FCLK",
    L"kGPT4_ALWON_FCLK",
    L"kGPT5_ALWON_FCLK",
    L"kGPT6_ALWON_FCLK",
    L"kGPT7_ALWON_FCLK",
    L"kGPT8_ALWON_FCLK",
    L"kGPT9_ALWON_FCLK",
    L"kGPT10_FCLK",
    L"kGPT11_FCLK",
};

static VddRefCountTable s_SavedVddTable;
static DpllMap s_SavedDpllTable;
static DpllClkOutMap s_SavedDpllClkOutTable;
static SrcClockMap s_SavedSrcClockTable;

extern DWORD OALWakeupLatency_GetSuspendState();
extern DeviceLookupEntry s_rgDeviceLookupTable[OMAP_DEVICE_COUNT];
extern DomainMap s_DomainTable;

static DWORD SavedDeviceIClkRefCount[OMAP_DEVICE_COUNT];
static DWORD SavedDeviceFClkRefCount[OMAP_DEVICE_COUNT];

PTCHAR DeviceNames[] = {
    L"OMAP_DEVICE_SDRC", 
	L"OMAP_DEVICE_IPSS",
    L"OMAP_DEVICE_SCMCTRL",
    L"OMAP_DEVICE_MCBSP1",
    L"OMAP_DEVICE_MCBSP5",
    L"OMAP_DEVICE_GPTIMER10",
    L"OMAP_DEVICE_GPTIMER11",    
    L"OMAP_DEVICE_UART1",
    L"OMAP_DEVICE_UART2",
    L"OMAP_DEVICE_I2C1",
    L"OMAP_DEVICE_I2C2",
    L"OMAP_DEVICE_I2C3",
    L"OMAP_DEVICE_MCSPI1",
    L"OMAP_DEVICE_MCSPI2",
    L"OMAP_DEVICE_MCSPI3",
    L"OMAP_DEVICE_MCSPI4",
    L"OMAP_DEVICE_HDQ", 
    L"OMAP_DEVICE_MMC1", 
    L"OMAP_DEVICE_MMC2", 
    L"OMAP_DEVICE_MMC3",
    L"OMAP_DEVICE_DES2", 
    L"OMAP_DEVICE_SHA12", 
    L"OMAP_DEVICE_AES2", 
    L"OMAP_DEVICE_DES1",
    L"OMAP_DEVICE_SHA11", 
    L"OMAP_DEVICE_RNG", 
    L"OMAP_DEVICE_AES1", 
    L"OMAP_DEVICE_PKA",
    L"OMAP_DEVICE_USBTLL",
    L"OMAP_DEVICE_TS",
    L"OMAP_DEVICE_EFUSE",
    L"OMAP_DEVICE_GPTIMER1",
    L"OMAP_DEVICE_GPTIMER12",
    L"OMAP_DEVICE_32KSYNC",
    L"OMAP_DEVICE_WDT1",
    L"OMAP_DEVICE_WDT2",
    L"OMAP_DEVICE_GPIO1",
    L"OMAP_DEVICE_GPIO2", 
    L"OMAP_DEVICE_GPIO3",
    L"OMAP_DEVICE_GPIO4",
    L"OMAP_DEVICE_GPIO5", 
    L"OMAP_DEVICE_GPIO6", 
    L"OMAP_DEVICE_MCBSP2",          
    L"OMAP_DEVICE_MCBSP3",
    L"OMAP_DEVICE_MCBSP4",
    L"OMAP_DEVICE_GPTIMER2",
    L"OMAP_DEVICE_GPTIMER3",
    L"OMAP_DEVICE_GPTIMER4",
    L"OMAP_DEVICE_GPTIMER5",
    L"OMAP_DEVICE_GPTIMER6",
    L"OMAP_DEVICE_GPTIMER7",
    L"OMAP_DEVICE_GPTIMER8",
    L"OMAP_DEVICE_GPTIMER9",    
    L"OMAP_DEVICE_UART3",
    L"OMAP_DEVICE_WDT3",       
    L"OMAP_DEVICE_DSS",
    L"OMAP_DEVICE_DSS1",
    L"OMAP_DEVICE_DSS2",
    L"OMAP_DEVICE_TVOUT",
    L"OMAP_DEVICE_VPFE",
    L"OMAP_DEVICE_CSI2",
    L"OMAP_DEVICE_2D",
    L"OMAP_DEVICE_3D",
    L"OMAP_DEVICE_USBHOST1",
    L"OMAP_DEVICE_USBHOST2",
    L"OMAP_DEVICE_USBHOST3",
	L"OMAP_DEVICE_OHCI",
	L"OMAP_DEVICE_EHCI",
	L"OMAP_DEVICE_VRFB",
    L"OMAP_DEVICE_GENERIC",
    L"OMAP_DEVICE_STUB1",
    L"OMAP_DEVICE_STUB2",
    L"OMAP_DEVICE_SGX = 0x48",
    L"OMAP_DEVICE_COUNT",        
    L"OMAP_DEVICE_SDMA",
    L"OMAP_DEVICE_GPMC",
    L"OMAP_DEVICE_HECC",
    L"OMAP_DEVICE_CPGMAC",
    L"OMAP_DEVICE_HSOTGUSB"
};

static DWORD DomainRefCountSnapshot[POWERDOMAIN_COUNT];
static PTCHAR DomainNameTable[POWERDOMAIN_COUNT] = {
    L"POWERDOMAIN_WAKEUP",
    L"POWERDOMAIN_CORE",
    L"POWERDOMAIN_PERIPHERAL",
    L"POWERDOMAIN_USBHOST",
    L"POWERDOMAIN_EMULATION",
    L"POWERDOMAIN_MPU",
    L"POWERDOMAIN_DSS",
    L"POWERDOMAIN_NEON",
    L"POWERDOMAIN_SGX",
    L"POWERDOMAIN_EFUSE",
};

// dump current values
VOID PrcmDumpRefCounts()
{
    int i;
	
    // dump s_VddTable
    OALMSG(1, (L"\r\nVDD ref counts:\r\n"));
    for (i = 0; i < kVDD_COUNT; i++)
        if (s_VddTable[i])
		    OALMSG(1, (L"%s %d\r\n", s_VddNameTable[i], s_VddTable[i]));

	// dump s_DpllTable ref count field
    OALMSG(1, (L"\r\nDPLL ref counts:\r\n"));
    for (i = 0; i < kDPLL_COUNT; i++)
        if (s_DpllTable[i].refCount)
            OALMSG(1, (L"%s %d\r\n", s_DpllNameTable[i], s_DpllTable[i].refCount));

    // dump s_DpllClkOutTable ref count field
    OALMSG(1, (L"\r\nDPLL CLKOUT ref counts:\r\n"));
    for (i = 0; i < kDPLL_CLKOUT_COUNT; i++)
        if (s_DpllClkOutTable[i].refCount)
            OALMSG(1, (L"%s %d\r\n", s_DpllClkOutNameTable[i], s_DpllClkOutTable[i].refCount));

	// dump s_SrcClockTable ref count field
    OALMSG(1, (L"\r\nSRC CLOCK ref counts:\r\n"));
    for (i = 0; i < kSOURCE_CLOCK_COUNT; i++)
        if (s_SrcClockTable[i].refCount)
            OALMSG(1, (L"%s %d\r\n", s_SrcClockNameTable[i], s_SrcClockTable[i].refCount));

    // domain refCount
    for (i = 0; i < POWERDOMAIN_COUNT; i++)
        if (s_DomainTable[i].refCount)
            OALMSG(1, (L"%s %d\r\n", DomainNameTable[i], s_DomainTable[i].refCount));
}

// save values for later display
VOID PrcmSaveRefCounts()
{
    int i;
	
    // save s_VddTable
    for (i = 0; i < kVDD_COUNT; i++)
        s_SavedVddTable[i] = s_VddTable[i];

	// save s_DpllTable
    for (i = 0; i < kDPLL_COUNT; i++)
        s_SavedDpllTable[i] = s_DpllTable[i];

    // save s_DpllClkOutTable
    for (i = 0; i < kDPLL_CLKOUT_COUNT; i++)
        s_SavedDpllClkOutTable[i] = s_DpllClkOutTable[i];

	// save s_SrcClockTable
    for (i = 0; i < kSOURCE_CLOCK_COUNT; i++)
        s_SavedSrcClockTable[i] = s_SrcClockTable[i];

    for (i = 0; i < OMAP_DEVICE_COUNT; i++)
	{
        // prcm iclk ref count
        if (s_rgDeviceLookupTable[i].piclk)
            SavedDeviceIClkRefCount[i] = s_rgDeviceLookupTable[i].piclk->refCount;
		else
		    SavedDeviceIClkRefCount[i] = 0;
		// prcm fclk ref count
        if (s_rgDeviceLookupTable[i].pfclk)
            SavedDeviceFClkRefCount[i] = s_rgDeviceLookupTable[i].pfclk->refCount;
		else
            SavedDeviceFClkRefCount[i] = 0;
	}
	
    // domain refCount
    for (i = 0; i < POWERDOMAIN_COUNT; i++)
        DomainRefCountSnapshot[i] = s_DomainTable[i].refCount;
}

// dump saved values
VOID PrcmDumpSavedRefCounts()
{
    int i;
	
    OALMSG(1, (L"\r\nSaved Information Prior to Suspend::\r\n"));

    // dump s_VddTable
    OALMSG(1, (L"\r\nSaved VDD ref counts:\r\n"));
    for (i = 0; i < kVDD_COUNT; i++)
        if (s_SavedVddTable[i])
            OALMSG(1, (L"%s %d\r\n", s_VddNameTable[i], s_SavedVddTable[i]));

	// dump s_DpllTable ref count field
    OALMSG(1, (L"\r\nSaved DPLL ref counts:\r\n"));
    for (i = 0; i < kDPLL_COUNT; i++)
        if (s_SavedDpllTable[i].refCount)
            OALMSG(1, (L"%s %d\r\n", s_DpllNameTable[i], s_SavedDpllTable[i].refCount));

    // dump s_DpllClkOutTable ref count field
    OALMSG(1, (L"\r\nSaved DPLL CLKOUT ref counts:\r\n"));
    for (i = 0; i < kDPLL_CLKOUT_COUNT; i++)
        if (s_SavedDpllClkOutTable[i].refCount)
            OALMSG(1, (L"%s %d\r\n", s_DpllClkOutNameTable[i], s_SavedDpllClkOutTable[i].refCount));

	// dump s_SrcClockTable ref count field
    OALMSG(1, (L"\r\nSaved SRC CLOCK ref counts:\r\n"));
    for (i = 0; i < kSOURCE_CLOCK_COUNT; i++)
        if (s_SavedSrcClockTable[i].refCount)
            OALMSG(1, (L"%s %d\r\n", s_SrcClockNameTable[i], s_SavedSrcClockTable[i].refCount));

    // dump prcm device iclk ref count
    OALMSG(1, (L"\r\nSaved device ICLK ref counts:\r\n"));
    for (i = 0; i < OMAP_DEVICE_COUNT; i++)
        if (SavedDeviceIClkRefCount[i])
    	    OALMSG(1, (L"Saved PRCM Device ICLK RefCount %s = %d\r\n", DeviceNames[i], SavedDeviceIClkRefCount[i]));

	// dump prcm device fclk ref count
    OALMSG(1, (L"\r\nSaved device FCLK ref counts:\r\n"));
    for (i = 0; i < OMAP_DEVICE_COUNT; i++)
        if (SavedDeviceFClkRefCount[i])
    	    OALMSG(1, (L"Saved PRCM Device FCLK RefCount %s = %d\r\n", DeviceNames[i], SavedDeviceFClkRefCount[i]));

    // domain refCount
    OALMSG(1, (L"\r\nSaved Domain RefCounts:\r\n"));
    for (i = 0; i < POWERDOMAIN_COUNT; i++)
        if (DomainRefCountSnapshot[i])
            OALMSG(1, (L"Saved Domain RefCount %s = %d\r\n", DomainNameTable[i], DomainRefCountSnapshot[i]));
}

//----------------------------------------------------------------------------

typedef struct _DumpRegStruct {
    PTCHAR RegName;
    DWORD RegWidth;
	DWORD RegPA;
	DWORD RegVal;
} DumpRegStruct;
    
DumpRegStruct DumpRegArray[] = {
    {L"IVA2_CM", 0, 0, 0},
    {L"CM_FCLKEN_IVA2", 32, 0x48004000, 0},
    {L"CM_CLKEN_PLL_IVA2", 32, 0x48004004, 0},
    {L"CM_IDLEST_IVA2", 32, 0x48004020, 0},
    {L"CM_IDLEST_PLL_IVA2", 32, 0x48004024, 0},
    {L"CM_AUTOIDLE_PLL_IVA2", 32, 0x48004034, 0},
    {L"CM_CLKSEL1_PLL_IVA2", 32, 0x48004040, 0},
    {L"CM_CLKSEL2_PLL_IVA2", 32, 0x48004044, 0},
    {L"CM_CLKSTCTRL_IVA2", 32, 0x48004048, 0},
    {L"CM_CLKSTST_IVA2", 32, 0x4800404C, 0},
    {L"OCP_System_Reg_CM", 32, 0, 0},
    {L"CM_REVISION", 32, 0x48004800, 0},
    {L"CM_SYSCONFIG", 32, 0x48004810, 0},
    {L"MPU_CM", 0, 0, 0},
    {L"CM_CLKEN_PLL_MPU", 32, 0x48004904, 0},
    {L"CM_IDLEST_MPU", 32, 0x48004920, 0},
    {L"CM_IDLEST_PLL_MPU", 32, 0x48004924, 0},
    {L"CM_AUTOIDLE_PLL_MPU", 32, 0x48004934, 0},
    {L"CM_CLKSEL1_PLL_MPU", 32, 0x48004940, 0},
    {L"CM_CLKSEL2_PLL_MPU", 32, 0x48004944, 0},
    {L"CM_CLKSTCTRL_MPU", 32, 0x48004948, 0},
    {L"CM_CLKSTST_MPU", 32, 0x4800494C, 0},
    {L"CORE_CM", 0, 0, 0},
    {L"CM_FCLKEN1_CORE", 32, 0x48004A00, 0},
    {L"CM_FCLKEN3_CORE", 32, 0x48004A08, 0},
    {L"CM_ICLKEN1_CORE", 32, 0x48004A10, 0},
    {L"CM_ICLKEN2_CORE", 32, 0x48004A14, 0},
    {L"CM_ICLKEN3_CORE", 32, 0x48004A18, 0},
    {L"CM_IDLEST1_CORE", 32, 0x48004A20, 0},
    {L"CM_IDLEST2_CORE", 32, 0x48004A24, 0},
    {L"CM_IDLEST3_CORE", 32, 0x48004A28, 0},
    {L"CM_AUTOIDLE1_CORE", 32, 0x48004A30, 0},
    {L"CM_AUTOIDLE2_CORE", 32, 0x48004A34, 0},
    {L"CM_AUTOIDLE3_CORE", 32, 0x48004A38, 0},
    {L"CM_CLKSEL_CORE", 32, 0x48004A40, 0},
    {L"CM_CLKSTCTRL_CORE", 32, 0x48004A48, 0},
    {L"CM_CLKSTST_CORE", 32, 0x48004A4C, 0},
    {L"SGX_CM", 0, 0, 0},
    {L"CM_FCLKEN_SGX", 32, 0x48004B00, 0},
    {L"CM_ICLKEN_SGX", 32, 0x48004B10, 0},
    {L"CM_IDLEST_SGX", 32, 0x48004B20, 0},
    {L"CM_CLKSEL_SGX", 32, 0x48004B40, 0},
    {L"CM_SLEEPDEP_SGX", 32, 0x48004B44, 0},
    {L"CM_CLKSTCTRL_SGX", 32, 0x48004B48, 0},
    {L"CM_CLKSTST_SGX", 32, 0x48004B4C, 0},
    {L"WKUP_CM", 0, 0, 0},
    {L"CM_FCLKEN_WKUP", 32, 0x48004C00, 0},
    {L"CM_ICLKEN_WKUP", 32, 0x48004C10, 0},
    {L"CM_IDLEST_WKUP", 32, 0x48004C20, 0},
    {L"CM_AUTOIDLE_WKUP", 32, 0x48004C30, 0},
    {L"CM_CLKSEL_WKUP", 32, 0x48004C40, 0},
    {L"Clock_Control_Reg_CM", 32, 0, 0},
    {L"CM_CLKEN_PLL", 32, 0x48004D00, 0},
    {L"CM_CLKEN2_PLL", 32, 0x48004D04, 0},
    {L"CM_IDLEST_CKGEN", 32, 0x48004D20, 0},
    {L"CM_IDLEST2_CKGEN", 32, 0x48004D24, 0},
    {L"CM_AUTOIDLE_PLL", 32, 0x48004D30, 0},
    {L"CM_AUTOIDLE2_PLL", 32, 0x48004D34, 0},
    {L"CM_CLKSEL1_PLL", 32, 0x48004D40, 0},
    {L"CM_CLKSEL2_PLL", 32, 0x48004D44, 0},
    {L"CM_CLKSEL3_PLL", 32, 0x48004D48, 0},
    {L"CM_CLKSEL4_PLL", 32, 0x48004D4C, 0},
    {L"CM_CLKSEL5_PLL", 32, 0x48004D50, 0},
    {L"CM_CLKOUT_CTRL", 32, 0x48004D70, 0},
    {L"DSS_CM", 0, 0, 0},
    {L"CM_FCLKEN_DSS", 32, 0x48004E00, 0},
    {L"CM_ICLKEN_DSS", 32, 0x48004E10, 0},
    {L"CM_IDLEST_DSS", 32, 0x48004E20, 0},
    {L"CM_AUTOIDLE_DSS", 32, 0x48004E30, 0},
    {L"CM_CLKSEL_DSS", 32, 0x48004E40, 0},
    {L"CM_SLEEPDEP_DSS", 32, 0x48004E44, 0},
    {L"CM_CLKSTCTRL_DSS", 32, 0x48004E48, 0},
    {L"CM_CLKSTST_DSS", 32, 0x48004E4C, 0},
    {L"CAM_CM", 0, 0, 0},
    {L"CM_FCLKEN_CAM", 32, 0x48004F00, 0},
    {L"CM_ICLKEN_CAM", 32, 0x48004F10, 0},
    {L"CM_IDLEST_CAM", 32, 0x48004F20, 0},
    {L"CM_AUTOIDLE_CAM", 32, 0x48004F30, 0},
    {L"CM_CLKSEL_CAM", 32, 0x48004F40, 0},
    {L"CM_SLEEPDEP_CAM", 32, 0x48004F44, 0},
    {L"CM_CLKSTCTRL_CAM", 32, 0x48004F48, 0},
    {L"CM_CLKSTST_CAM", 32, 0x48004F4C, 0},
    {L"PER_CM", 0, 0, 0},
    {L"CM_FCLKEN_PER", 32, 0x48005000, 0},
    {L"CM_ICLKEN_PER", 32, 0x48005010, 0},
    {L"CM_IDLEST_PER", 32, 0x48005020, 0},
    {L"CM_AUTOIDLE_PER", 32, 0x48005030, 0},
    {L"CM_CLKSEL_PER", 32, 0x48005040, 0},
    {L"CM_SLEEPDEP_PER", 32, 0x48005044, 0},
    {L"CM_CLKSTCTRL_PER", 32, 0x48005048, 0},
    {L"CM_CLKSTST_PER", 32, 0x4800504C, 0},
    {L"EMU_CM", 0, 0, 0},
    {L"CM_CLKSEL1_EMU", 32, 0x48005140, 0},
    {L"CM_CLKSTCTRL_EMU", 32, 0x48005148, 0},
    {L"CM_CLKSTST_EMU", 32, 0x4800514C, 0},
    {L"CM_CLKSEL2_EMU", 32, 0x48005150, 0},
    {L"CM_CLKSEL3_EMU", 32, 0x48005154, 0},
    {L"Global_Reg_CM", 0, 0, 0},
    {L"CM_POLCTRL", 32, 0x4800529C, 0},
    {L"NEON_CM", 0, 0, 0},
    {L"CM_IDLEST_NEON", 32, 0x48005320, 0},
    {L"CM_CLKSTCTRL_NEON", 32, 0x48005348, 0},
    {L"USBHOST_CM", 0, 0, 0},
    {L"CM_FCLKEN_USBHOST", 32, 0x48005400, 0},
    {L"CM_ICLKEN_USBHOST", 32, 0x48005410, 0},
    {L"CM_IDLEST_USBHOST", 32, 0x48005420, 0},
    {L"CM_AUTOIDLE_USBHOST", 32, 0x48005430, 0},
    {L"CM_SLEEPDEP_USBHOST", 32, 0x48005444, 0},
    {L"CM_CLKSTCTRL_USBHOST", 32, 0x48005448, 0},
    {L"CM_CLKSTST_USBHOST", 32, 0x4800544C, 0},
    {L"IVA2_PRM", 0, 0, 0},
    {L"RM_RSTCTRL_IVA2", 32, 0x48306050, 0},
    {L"RM_RSTST_IVA2", 32, 0x48306058, 0},
    {L"PM_WKDEP_IVA2", 32, 0x483060C8, 0},
    {L"PM_PWSTCTRL_IVA2", 32, 0x483060E0, 0},
    {L"PM_PWSTST_IVA2", 32, 0x483060E4, 0},
    {L"PM_PREPWSTST_IVA2", 32, 0x483060E8, 0},
    {L"PRM_IRQSTATUS_IVA2", 32, 0x483060F8, 0},
    {L"PRM_IRQENABLE_IVA2", 32, 0x483060FC, 0},
    {L"OCP_System_Reg_PRM", 0, 0, 0},
    {L"PRM_REVISION", 32, 0x48306804, 0},
    {L"PRM_SYSCONFIG", 32, 0x48306814, 0},
    {L"PRM_IRQSTATUS_MPU", 32, 0x48306818, 0},
    {L"PRM_IRQENABLE_MPU", 32, 0x4830681C, 0},
    {L"MPU_PRM", 0, 0, 0},
    {L"RM_RSTST_MPU", 32, 0x48306958, 0},
    {L"PM_WKDEP_MPU", 32, 0x483069C8, 0},
    {L"PM_EVGENCTRL_MPU", 32, 0x483069D4, 0},
    {L"PM_EVGENONTIM_MPU", 32, 0x483069D8, 0},
    {L"PM_EVGENOFFTIM_MPU", 32, 0x483069DC, 0},
    {L"PM_PWSTCTRL_MPU", 32, 0x483069E0, 0},
    {L"PM_PWSTST_MPU", 32, 0x483069E4, 0},
    {L"PM_PREPWSTST_MPU", 32, 0x483069E8, 0},
    {L"CORE_PRM", 0, 0, 0},
    {L"RM_RSTST_CORE", 32, 0x48306A58, 0},
    {L"PM_WKEN1_CORE", 32, 0x48306AA0, 0},
    {L"PM_MPUGRPSEL1_CORE", 32, 0x48306AA4, 0},
    {L"PM_IVA2GRPSEL1_CORE", 32, 0x48306AA8, 0},
    {L"PM_WKST1_CORE", 32, 0x48306AB0, 0},
    {L"PM_WKST3_CORE", 32, 0x48306AB8, 0},
    {L"PM_PWSTCTRL_CORE", 32, 0x48306AE0, 0},
    {L"PM_PWSTST_CORE", 32, 0x48306AE4, 0},
    {L"PM_PREPWSTST_CORE", 32, 0x48306AE8, 0},
    {L"PM_WKEN3_CORE", 32, 0x48306AF0, 0},
    {L"PM_IVA2GRPSEL3_CORE", 32, 0x48306AF4, 0},
    {L"PM_MPUGRPSEL3_CORE", 32, 0x48306AF8, 0},
    {L"SGX_PRM", 0, 0, 0},
    {L"RM_RSTST_SGX", 32, 0x48306B58, 0},
    {L"PM_WKDEP_SGX", 32, 0x48306BC8, 0},
    {L"PM_PWSTCTRL_SGX", 32, 0x48306BE0, 0},
    {L"PM_PWSTST_SGX", 32, 0x48306BE4, 0},
    {L"PM_PREPWSTST_SGX", 32, 0x48306BE8, 0},
    {L"WKUP_PRM", 0, 0, 0},
    {L"PM_WKEN_WKUP", 32, 0x48306CA0, 0},
    {L"PM_MPUGRPSEL_WKUP", 32, 0x48306CA4, 0},
    {L"PM_IVA2GRPSEL_WKUP", 32, 0x48306CA8, 0},
    {L"PM_WKST_WKUP", 32, 0x48306CB0, 0},
    {L"Clock_Control_Reg_PRM", 0, 0, 0},
    {L"PRM_CLKSEL", 32, 0x48306D40, 0},
    {L"PRM_CLKOUT_CTRL", 32, 0x48306D70, 0},
    {L"DSS_PRM", 0, 0, 0},
    {L"RM_RSTST_DSS", 32, 0x48306E58, 0},
    {L"PM_WKEN_DSS", 32, 0x48306EA0, 0},
    {L"PM_WKDEP_DSS", 32, 0x48306EC8, 0},
    {L"PM_PWSTCTRL_DSS", 32, 0x48306EE0, 0},
    {L"PM_PWSTST_DSS", 32, 0x48306EE4, 0},
    {L"PM_PREPWSTST_DSS", 32, 0x48306EE8, 0},
    {L"CAM_PRM", 0, 0, 0},
    {L"RM_RSTST_CAM", 32, 0x48306F58, 0},
    {L"PM_WKDEP_CAM", 32, 0x48306FC8, 0},
    {L"PM_PWSTCTRL_CAM", 32, 0x48306FE0, 0},
    {L"PM_PWSTST_CAM", 32, 0x48306FE4, 0},
    {L"PM_PREPWSTST_CAM", 32, 0x48306FE8, 0},
    {L"PER_PRM", 0, 0, 0},
    {L"RM_RSTST_PER", 32, 0x48307058, 0},
    {L"PM_WKEN_PER", 32, 0x483070A0, 0},
    {L"PM_MPUGRPSEL_PER", 32, 0x483070A4, 0},
    {L"PM_IVA2GRPSEL_PER", 32, 0x483070A8, 0},
    {L"PM_WKST_PER", 32, 0x483070B0, 0},
    {L"PM_WKDEP_PER", 32, 0x483070C8, 0},
    {L"PM_PWSTCTRL_PER", 32, 0x483070E0, 0},
    {L"PM_PWSTST_PER", 32, 0x483070E4, 0},
    {L"PM_PREPWSTST_PER", 32, 0x483070E8, 0},
    {L"EMU_PRM", 0, 0, 0},
    {L"RM_RSTST_EMU", 32, 0x48307158, 0},
    {L"PM_PWSTST_EMU", 32, 0x483071E4, 0},
    {L"Global_Reg_PRM", 0, 0, 0},
    {L"PRM_VC_SMPS_SA", 32, 0x48307220, 0},
    {L"PRM_VC_SMPS_VOL_RA", 32, 0x48307224, 0},
    {L"PRM_VC_SMPS_CMD_RA", 32, 0x48307228, 0},
    {L"PRM_VC_CMD_VAL_0", 32, 0x4830722C, 0},
    {L"PRM_VC_CMD_VAL_1", 32, 0x48307230, 0},
    {L"PRM_VC_CH_CONF", 32, 0x48307234, 0},
    {L"PRM_VC_I2C_CFG", 32, 0x48307238, 0},
    {L"PRM_VC_BYPASS_VAL", 32, 0x4830723C, 0},
    {L"PRM_RSTCTRL", 32, 0x48307250, 0},
    {L"PRM_RSTTIME", 32, 0x48307254, 0},
    {L"PRM_RSTST", 32, 0x48307258, 0},
    {L"PRM_VOLTCTRL", 32, 0x48307260, 0},
    {L"PRM_SRAM_PCHARGE", 32, 0x48307264, 0},
    {L"PRM_CLKSRC_CTRL", 32, 0x48307270, 0},
    {L"PRM_OBS", 32, 0x48307280, 0},
    {L"PRM_VOLTSETUP1", 32, 0x48307290, 0},
    {L"PRM_VOLTOFFSET", 32, 0x48307294, 0},
    {L"PRM_CLKSETUP", 32, 0x48307298, 0},
    {L"PRM_POLCTRL", 32, 0x4830729C, 0},
    {L"PRM_VOLTSETUP2", 32, 0x483072A0, 0},
    {L"NEON_PRM", 0, 0, 0},
    {L"RM_RSTST_NEON", 32, 0x48307358, 0},
    {L"PM_WKDEP_NEON", 32, 0x483073C8, 0},
    {L"PM_PWSTCTRL_NEON", 32, 0x483073E0, 0},
    {L"PM_PWSTST_NEON", 32, 0x483073E4, 0},
    {L"PM_PREPWSTST_NEON", 32, 0x483073E8, 0},
    {L"USBHOST_PRM", 0, 0, 0},
    {L"RM_RSTST_USBHOST", 32, 0x48307458, 0},
    {L"PM_WKEN_USBHOST", 32, 0x483074A0, 0},
    {L"PM_MPUGRPSEL_USBHOST", 32, 0x483074A4, 0},
    {L"PM_IVA2GRPSEL_USBHOST", 32, 0x483074A8, 0},
    {L"PM_WKST_USBHOST", 32, 0x483074B0, 0},
    {L"PM_WKDEP_USBHOST", 32, 0x483074C8, 0},
    {L"PM_PWSTCTRL_USBHOST", 32, 0x483074E0, 0},
    {L"PM_PWSTST_USBHOST", 32, 0x483074E4, 0},
    {L"PM_PREPWSTST_USBHOST", 32, 0x483074E8, 0},
    {NULL, 0, 0, 0}
};

// take snapshot of prcm registers
void PrcmRegsSnapshot()
{
    int i = 0;
	
	while (DumpRegArray[i].RegName != NULL)
	{
        if (DumpRegArray[i].RegPA != 0)
		{
            if (DumpRegArray[i].RegWidth == 32)
                DumpRegArray[i].RegVal = INREG32(OALPAtoUA(DumpRegArray[i].RegPA));
            else if (DumpRegArray[i].RegWidth == 16)
                DumpRegArray[i].RegVal = INREG16(OALPAtoUA(DumpRegArray[i].RegPA));
            else if (DumpRegArray[i].RegWidth == 8)
                DumpRegArray[i].RegVal = INREG8(OALPAtoUA(DumpRegArray[i].RegPA));
		}
        i++;
	}
}

// display snapshot of prcm registers
void DumpPrcmRegsSnapshot()
{
    int i = 0;
	
	while (DumpRegArray[i].RegName != NULL)
	{
        if (DumpRegArray[i].RegPA == 0)
		{
		    OALMSG(1, (L"\r\n%s:\r\n", DumpRegArray[i].RegName));
		}
        else
		{
            if (DumpRegArray[i].RegWidth == 32)
    		    OALMSG(1, (L"  %-24s 0x%08x\r\n", DumpRegArray[i].RegName, DumpRegArray[i].RegVal));
            else if (DumpRegArray[i].RegWidth == 16)
    		    OALMSG(1, (L"  %-24s     0x%04x\r\n", DumpRegArray[i].RegName, DumpRegArray[i].RegVal));
            else if (DumpRegArray[i].RegWidth == 8)
    		    OALMSG(1, (L"  %-24s       0x%02x\r\n", DumpRegArray[i].RegName, DumpRegArray[i].RegVal));
            else
    		    OALMSG(1, (L"  %-24s <bad RegWidth>\r\n"));
		}
        i++;
	}
}

// display current prcm registers
void DumpPrcmRegs()
{
    int i = 0;
	
	while (DumpRegArray[i].RegName != NULL)
	{
        if (DumpRegArray[i].RegPA == 0)
		{
		    OALMSG(1, (L"\r\n%s:\r\n", DumpRegArray[i].RegName));
		}
        else
        {
            if (DumpRegArray[i].RegWidth == 32)
    		    OALMSG(1, (L"  %-24s 0x%08x\r\n", DumpRegArray[i].RegName, INREG32(OALPAtoUA(DumpRegArray[i].RegPA))));
            else if (DumpRegArray[i].RegWidth == 16)
    		    OALMSG(1, (L"  %-24s     0x%04x\r\n", DumpRegArray[i].RegName, INREG16(OALPAtoUA(DumpRegArray[i].RegPA))));
            else if (DumpRegArray[i].RegWidth == 8)
    		    OALMSG(1, (L"  %-24s       0x%02x\r\n", DumpRegArray[i].RegName, INREG8(OALPAtoUA(DumpRegArray[i].RegPA))));
            else
    		    OALMSG(1, (L"  %-24s <bad RegWidth>\r\n"));
        }
        i++;
	}
}

#else

void DumpPrcmRegs()
{
}

#endif
