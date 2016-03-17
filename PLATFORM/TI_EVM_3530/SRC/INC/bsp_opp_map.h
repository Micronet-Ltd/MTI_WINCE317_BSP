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
//  File:  bsp_opp_map.h
//

#ifndef __BSP_OPP_MAP_H
#define __BSP_OPP_MAP_H

//-----------------------------------------------------------------------------
typedef struct {
    SmartReflexSensorData_t     smartReflexInfo;
    VoltageProcessorSetting_t   vpInfo;
    int                         dpllCount;
    DpllFrequencySetting_t      rgFrequencySettings[];
} VddOppSetting_t;

//-----------------------------------------------------------------------------
typedef struct {
    BOOL                        bInitialized;
    BOOL                        bEnabled;
    CRITICAL_SECTION            cs;
} SmartReflexStateInfo_t;

#define VDD1_OPP_COUNT      4
#define VDD2_OPP_COUNT      2
#if (BSP_OPM_SELECT == A) || (BSP_CDS == 1)
    // MPU[1000Mhz @ 1.3125V], IVA2[875Mhz @ 1.3125V]
    #define INITIAL_VDD1_OPP4	(kOpp4)
    // MPU[800Mhz @ 1.2625V], IVA2[660Mhz @ 1.2625V]
    #define INITIAL_VDD1_OPP3	(kOpp3)
    // MPU[600Mhz @ 1.1000V], IVA2[520Mhz @ 1.1000V]
    #define INITIAL_VDD1_OPP2	(kOpp2)
#elif (BSP_OPM_SELECT == 4)
    // MPU[1000Mhz @ 1.3125V], IVA2[875Mhz @ 1.3125V]
    #define INITIAL_VDD1_OPP    (kOpp4)
#elif (BSP_OPM_SELECT == 3)
    // MPU[800Mhz @ 1.2625V], IVA2[660Mhz @ 1.2625V]
    #define INITIAL_VDD1_OPP    (kOpp3)
#elif BSP_OPM_SELECT == 2
    // MPU[600Mhz @ 1.1000V], IVA2[520Mhz @ 1.1000V]
    #define INITIAL_VDD1_OPP    (kOpp2)
#elif BSP_OPM_SELECT == 1
    // MPU[300Mhz @ 0.9375V], IVA2[260Mhz @ 0.9375V]
    #define INITIAL_VDD1_OPP    (kOpp1)
#else
    #error Unsupported value for BSP_OPM_SELECT, use 1..4
#endif

#define INITIAL_VDD2_OPP    (kOpp2)
#define MAX_VDD1_OPP        (kOpp4)
#define MAX_VDD2_OPP        (kOpp2)

//-----------------------------------------------------------------------------

// For TPS659XX use the following equations to calculate the value from volt and vice versa.
// val = (volt - .6)/.0125, volt = val * .0125 + .6

// (just a placeholder)
static VddOppSetting_t vdd1Opp0Info = {
    {
        kSmartReflex_Channel1, BSP_SRCLKLEN_INIT        // smartreflex info
    }, {
        kVoltageProcessor1, 0, 0                        // voltage processor info
    }, 
        2,
    {                                                   // vdd1 has 2 dpll's
       {
            kDPLL1, 0,  -1, 0,  0, 1                    // dpll1 (MPU)
       }, {
            kDPLL2, 0,  -1, 0,  0, 1                    // dpll2 (DSP)
       }
    }
};

// MPU[300Mhz @ 0.9375V], IVA2[260Mhz @ 0.9375V]
static VddOppSetting_t vdd1Opp1Info = {
    {
        kSmartReflex_Channel1, BSP_SRCLKLEN_INIT        // smartreflex info
    }, {
        kVoltageProcessor1, 0x1B,   0x1B                // voltage processor info
    }, 
        2,
    {                                                   // vdd1 has 2 dpll's
       {
            kDPLL1, 150, 300, 12, 7, 4                  // dpll1 (MPU)
       }, {
            kDPLL2, 130, 260, 12, 7, 4                  // dpll2 (DSP)
       }
    }
};

// MPU600[Mhz @ 1.100V], IVA2[520Mhz @ 1.100V]
static VddOppSetting_t vdd1Opp2Info = {
    {
        kSmartReflex_Channel1, BSP_SRCLKLEN_INIT        // smartreflex info
    }, {
        kVoltageProcessor1, 0x28,   0x28                // voltage processor info
    }, 
        2,
    {                                                   // vdd1 has 2 dpll's
       {
            kDPLL1, 600, 600, 12, 7, 2                  // dpll1 (MPU)
       }, {
            kDPLL2, 520, 520, 12, 7, 2                  // dpll2 (DSP)
       }
    }
};

// MPU[800Mhz @ 1.2625V], IVA2[660Mhz @ 1.2625V]
static VddOppSetting_t vdd1Opp3Info = {
    {
        kSmartReflex_Channel1, BSP_SRCLKLEN_INIT        // smartreflex info
    }, {
        kVoltageProcessor1, 0x35,   0x35                // voltage processor info
    }, 
        2,
    {                                                   // vdd1 has 2 dpll's
       {
            kDPLL1, 800, 400, 12, 7, 1                  // dpll1 (MPU)
       }, {
            kDPLL2, 660, 330, 12, 7, 1                  // dpll2 (DSP)
       }
    }
};

// MPU[1000Mhz @ 1.3125V], IVA2[875Mhz @ 1.3125V]
static VddOppSetting_t vdd1Opp4Info = {
    {
        kSmartReflex_Channel1, BSP_SRCLKLEN_INIT        // smartreflex info
    }, {
        kVoltageProcessor1, 0x39,   0x39                // voltage processor info
    }, 
        2,
    {                                                   // vdd1 has 2 dpll's
       {
            kDPLL1, 1000, 500, 12, 7, 1                 // dpll1 (MPU)
       }, {
            kDPLL2, 874, 437, 12, 7, 1                  // dpll2 (DSP)
       }
    }
};


//-----------------------------------------------------------------------------
// (just a placeholder)
static VddOppSetting_t vdd2Opp0Info = {
    {
        kSmartReflex_Channel2, BSP_SRCLKLEN_INIT        // smartreflex info
    }, {
        kVoltageProcessor2, 0, 0                        // voltage processor info
    }, 
        1,
    {                                                   // vdd2 has 1 dpll
       {
            kDPLL3,   0,  -1,  0, 0, 0                  // dpll3 (CORE)
       }
    }
};

//CORE[200Mhz @ 0.9375V]
static VddOppSetting_t vdd2Opp1Info = {
    {
        kSmartReflex_Channel2, BSP_SRCLKLEN_INIT        // smartreflex info
    }, {
        kVoltageProcessor2, 0x1B,    0x1B               // voltage processor info
    }, 
        1,
    {                                                   // vdd2 has 1 dpll
       {
            kDPLL3, 200, 200, 12, 7, 2                  // dpll3 (CORE)
       }
    }
};

// CORE[400Mhz @ 1.1375V]
static VddOppSetting_t vdd2Opp2Info = {
    {
        kSmartReflex_Channel2, BSP_SRCLKLEN_INIT        // smartreflex info
    }, {
        kVoltageProcessor2, 0x2B,    0x2B               // voltage processor info
    }, 
        1,
    {                                                   // vdd2 has 1 dpll
       {
            kDPLL3, 400, 200, 12, 7, 1                  // dpll3 (CORE)
       }
    }
};

//-----------------------------------------------------------------------------
static VddOppSetting_t  *_rgVdd1OppMap[VDD1_OPP_COUNT] = {
    &vdd1Opp1Info,      // kOpp1
    &vdd1Opp2Info,      // kOpp2
    &vdd1Opp3Info,      // kOpp3
    &vdd1Opp4Info,      // kOpp4
};

static VddOppSetting_t  *_rgVdd2OppMap[VDD2_OPP_COUNT] = {
    &vdd2Opp1Info,      // kOpp1
    &vdd2Opp2Info,      // kOpp2
};


//-----------------------------------------------------------------------------
#endif // __BSP_OPP_MAP_H

