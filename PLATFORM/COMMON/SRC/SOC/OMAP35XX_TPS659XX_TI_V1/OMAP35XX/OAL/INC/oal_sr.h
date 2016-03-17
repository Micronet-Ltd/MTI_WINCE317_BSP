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
//  File:  oal_sr.h
//
#ifndef _OAL_SR_H
#define _OAL_SR_H

#ifdef __cplusplus
extern "C" {
#endif

//-----------------------------------------------------------------------------
typedef enum {
    kSmartReflex_Channel1 = 0,
    kSmartReflex_Channel2,
    kSmartReflex_ChannelCount
}
SmartReflexChannel_e;

//-----------------------------------------------------------------------------
typedef struct {
    union {
        struct {
            UINT        vdd1_opp1_rnsenn:8;
            UINT        vdd1_opp1_rnsenp:8;
            UINT        vdd1_opp1_senngain:4;
            UINT        vdd1_opp1_senpgain:4;
            UINT        zzzReserved4:8;

            UINT        vdd1_opp2_rnsenn:8;
            UINT        vdd1_opp2_rnsenp:8;
            UINT        vdd1_opp2_senngain:4;
            UINT        vdd1_opp2_senpgain:4;
            UINT        zzzReserved5:8;

            UINT        vdd1_opp3_rnsenn:8;
            UINT        vdd1_opp3_rnsenp:8;
            UINT        vdd1_opp3_senngain:4;
            UINT        vdd1_opp3_senpgain:4;
            UINT        zzzReserved6:8;

            UINT        vdd1_opp4_rnsenn:8;
            UINT        vdd1_opp4_rnsenp:8;
            UINT        vdd1_opp4_senngain:4;
            UINT        vdd1_opp4_senpgain:4;
            UINT        zzzReserved7:8;

            UINT        vdd1_opp5_rnsenn:8;
            UINT        vdd1_opp5_rnsenp:8;
            UINT        vdd1_opp5_senngain:4;
            UINT        vdd1_opp5_senpgain:4;
            UINT        zzzReserved8:8;

            UINT        vdd2_opp1_rnsenn:8;
            UINT        vdd2_opp1_rnsenp:8;
            UINT        vdd2_opp1_senngain:4;
            UINT        vdd2_opp1_senpgain:4;
            UINT        zzzReserved1:8;

            UINT        vdd2_opp2_rnsenn:8;
            UINT        vdd2_opp2_rnsenp:8;
            UINT        vdd2_opp2_senngain:4;
            UINT        vdd2_opp2_senpgain:4;
            UINT        zzzReserved2:8;

            UINT        vdd2_opp3_rnsenn:8;
            UINT        vdd2_opp3_rnsenp:8;
            UINT        vdd2_opp3_senngain:4;
            UINT        vdd2_opp3_senpgain:4;
            UINT        zzzReserved3:8;
        };
        
        struct {
            UINT        Efuse_Vdd1_Opp_1;
            UINT        Efuse_Vdd1_Opp_2;
            UINT        Efuse_Vdd1_Opp_3;
            UINT        Efuse_Vdd1_Opp_4;
            UINT        Efuse_Vdd1_Opp_5;
            UINT        Efuse_Vdd2_Opp_1;
            UINT        Efuse_Vdd2_Opp_2;
            UINT        Efuse_Vdd2_Opp_3;
        };
    };
}
Efuse_SensorData_t;

//-----------------------------------------------------------------------------
typedef struct {
    union {
        struct {
            UINT        sr1_sennenable:2;
            UINT        sr1_senpenable:2;
            UINT        zzzReserved9:4;
            UINT        sr2_sennenable:2;
            UINT        sr2_senpenable:2;
            UINT        zzzReserved10:20;
            };
        UINT            Efuse_SR;
        };
}
SmartReflexSenorEnData_t;

//-----------------------------------------------------------------------------
typedef struct {
    SmartReflexChannel_e    channel;
    UINT                    srClkLengthT;
    UINT                    rnsenp;
    UINT                    rnsenn;
    UINT                    senpgain;
    UINT                    senngain;
}
SmartReflexSensorData_t;

typedef struct {
    union {
        struct {
            UINT   errMinLimit:8;
            UINT   errMaxLimit:8;
            UINT   errWeight:3;        
            UINT   zzzReserved1:13;
            };
        UINT   errConfig;
    };
}
SmartreflexConfiguration_t;

//-----------------------------------------------------------------------------
//
//  Function: SmartReflex functions
//
void
SmartReflex_InitializeChannel(
    SmartReflexChannel_e    channel,
    OMAP_SMARTREFLEX       *pSr
    );

void
SmartReflex_SetAccumulationSize(
    SmartReflexChannel_e    channel,
    UINT                    accumData
    );

void
SmartReflex_SetSensorMode(
    SmartReflexChannel_e    channel,
    UINT                    senNMode,
    UINT                    senPMode
    );

void 
SmartReflex_SetIdleMode(
    SmartReflexChannel_e    channel,
    UINT                    idleMode
    );

void
SmartReflex_SetSensorReferenceData(
    SmartReflexChannel_e     channel,
    SmartReflexSensorData_t *pSensorData    
    );

void
SmartReflex_SetErrorControl(
    SmartReflexChannel_e    channel,
    UINT                    errorWeight,
    UINT                    errorMax,
    UINT                    errorMin
    );

void
SmartReflex_EnableInterrupts(
    SmartReflexChannel_e    channel,
    UINT                    mask,
    BOOL                    bEnable    
    );

void
SmartReflex_EnableErrorGeneratorBlock(
    SmartReflexChannel_e    channel,
    BOOL                    bEnable
    );

void
SmartReflex_EnableMinMaxAvgBlock(
    SmartReflexChannel_e    channel,
    BOOL                    bEnable
    );

void
SmartReflex_EnableSensorBlock(
    SmartReflexChannel_e    channel,
    BOOL                    bEnable
    );

void
SmartReflex_Enable(
    SmartReflexChannel_e    channel,
    BOOL                    bEnable
    );

UINT32
SmartReflex_ClearInterruptStatus(
    SmartReflexChannel_e    channel,
    UINT                    mask
    );

UINT32
SmartReflex_GetStatus(
    SmartReflexChannel_e    channel
    );

void
SmartReflex_SetAvgWeight(
    SmartReflexChannel_e    channel,
    UINT                    senNWeight,
    UINT                    senPWeight
    );

//------------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif  // _OAL_SR_H

