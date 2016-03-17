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
//  File:  opp_map.c
//
#include <windows.h>
#include <bsp.h>
#include <oal_prcm.h>
#include <oal_sr.h>
#include <dvfs.h>
#include <bsp_opp_map.h>

#define MAX_VOLT_DOMAINS        (2)

#define VP_VOLTS(x) (((x) * 125 + 6000) / 10000)
#define VP_MILLIVOLTS(x) (((x) * 125 + 6000) % 10000)

//-----------------------------------------------------------------------------
static SmartReflexStateInfo_t   _rgSmartReflexInfo[2] = {
    {
        FALSE, FALSE
    }, {
        FALSE, FALSE
    }
};

//-----------------------------------------------------------------------------
#if (BSP_OPM_SELECT == A) || (BSP_CDS == 1)
static UINT _rgOppVdd[2] = {INITIAL_VDD1_OPP2, INITIAL_VDD2_OPP};
#else
static UINT _rgOppVdd[2] = {INITIAL_VDD1_OPP, INITIAL_VDD2_OPP};
#endif

//-----------------------------------------------------------------------------
static UINT _rgInitialRetentionVoltages[2] = {
    VC_CMD_0_VOLT_RET >> SMPS_RET_SHIFT,
    VC_CMD_1_VOLT_RET >> SMPS_RET_SHIFT
};

//-----------------------------------------------------------------------------
// Error Gain and Error Offset configuration 
static VoltageProcessorErrConfig_t _rgVp1ErrConfig [5] = {
                                 { VDD1_OPP1_ERRORGAIN, VDD_NO_ERROFFSET },
                                 { VDD1_OPP2_ERRORGAIN, VDD_NO_ERROFFSET },
                                 { VDD1_OPP3_ERRORGAIN, VDD_NO_ERROFFSET },
                                 { VDD1_OPP4_ERRORGAIN, VDD_NO_ERROFFSET },
                                 { VDD1_OPP5_ERRORGAIN, VDD_NO_ERROFFSET },
                                 };

static VoltageProcessorErrConfig_t _rgVp2ErrConfig [2] = {
                                 { VDD2_OPP1_ERRORGAIN, VDD_NO_ERROFFSET },
                                 { VDD2_OPP2_ERRORGAIN, VDD_NO_ERROFFSET },
                                 };

SmartreflexConfiguration_t      _rgSr1ErrConfig [5] = {
                                    {     SR1_OPP1_ERRMINLIMIT, 
                                          BSP_SR1_ERRMAXLIMIT_INIT, 
                                          BSP_SR1_ERRWEIGHT_INIT
                                    },
                                    {     SR1_OPP2_ERRMINLIMIT, 
                                          BSP_SR1_ERRMAXLIMIT_INIT, 
                                          BSP_SR1_ERRWEIGHT_INIT
                                    },
                                    {     SR1_OPP3_ERRMINLIMIT, 
                                          BSP_SR1_ERRMAXLIMIT_INIT, 
                                          BSP_SR1_ERRWEIGHT_INIT
                                    },
                                    {     SR1_OPP4_ERRMINLIMIT, 
                                          BSP_SR1_ERRMAXLIMIT_INIT, 
                                          BSP_SR1_ERRWEIGHT_INIT
                                    },
                                    {     SR1_OPP5_ERRMINLIMIT, 
                                          BSP_SR1_ERRMAXLIMIT_INIT, 
                                          BSP_SR1_ERRWEIGHT_INIT
                                    }
                                };

SmartreflexConfiguration_t      _rgSr2ErrConfig [2] = {
                                    {     SR2_OPP1_ERRMINLIMIT, 
                                          BSP_SR1_ERRMAXLIMIT_INIT, 
                                          BSP_SR1_ERRWEIGHT_INIT
                                    },
                                    {     SR2_OPP2_ERRMINLIMIT, 
                                          BSP_SR1_ERRMAXLIMIT_INIT, 
                                          BSP_SR1_ERRWEIGHT_INIT
                                    }
                                };

//-----------------------------------------------------------------------------
// static : _sensorEnData
//      
//      Saves the sensor enable data from EFuse
// 
static SmartReflexSenorEnData_t     _sensorEnData;

//-----------------------------------------------------------------------------
// static : _bSmartreflexCapable
//      
//      Saves the hardware SmartReflex capability state
// 
static BOOL _bSmartreflexCapable;

//-----------------------------------------------------------------------------
// Prototype
//
void SmartReflex_VoltageFlush(VoltageProcessor_e vp);

//-----------------------------------------------------------------------------
__inline
void 
SmartReflexLock(UINT channel)
{
    if (_rgSmartReflexInfo[channel].bInitialized && INTERRUPTS_STATUS()) 
        {
        EnterCriticalSection(&_rgSmartReflexInfo[channel].cs);
        }
}

//-----------------------------------------------------------------------------
__inline
void 
SmartReflexUnlock(UINT channel)
{
    if (_rgSmartReflexInfo[channel].bInitialized && INTERRUPTS_STATUS()) 
        {
        LeaveCriticalSection(&_rgSmartReflexInfo[channel].cs);
        }
}

//-----------------------------------------------------------------------------
BOOL 
IsSmartReflexMonitoringEnabled(UINT channel)
{   
    return _rgSmartReflexInfo[channel].bEnabled;
}

//-----------------------------------------------------------------------------
void SmartReflex_Initialize()
{
    SmartReflexSensorData_t *pSRInfo;
    Efuse_SensorData_t      sensorData;
    OMAP_SYSC_GENERAL_REGS  *pSysc = OALPAtoVA(OMAP_SYSC_GENERAL_REGS_PA, FALSE);
#if (BSP_OPM_SELECT == A) || (BSP_CDS == 1)
	UINT32 *pOpm = OALPAtoUA(IMAGE_SHARE_OPM);
	UINT32 opm;

	opm = *pOpm;

	if(1 == opm)
		_rgOppVdd[0] = INITIAL_VDD1_OPP3; // 800 Mhz
	else if(2 == opm)
		_rgOppVdd[0] = INITIAL_VDD1_OPP4; // 1000 Mhz
	else
		_rgOppVdd[0] = INITIAL_VDD1_OPP2; // 600 MHz

	_rgOppVdd[1] = INITIAL_VDD2_OPP;
#endif
    // Initialize the voltage processor with the current OPP voltage.
    SmartReflex_VoltageFlush(kVoltageProcessor1);
    SmartReflex_VoltageFlush(kVoltageProcessor2);
	 
    // EFuse data for Vdd1
    sensorData.Efuse_Vdd1_Opp_1 = INREG32(&pSysc->CONTROL_FUSE_OPP1_VDD1);
    sensorData.Efuse_Vdd1_Opp_2 = INREG32(&pSysc->CONTROL_FUSE_OPP2_VDD1);
    sensorData.Efuse_Vdd1_Opp_3 = INREG32(&pSysc->CONTROL_FUSE_OPP3_VDD1);
    sensorData.Efuse_Vdd1_Opp_4 = INREG32(&pSysc->CONTROL_FUSE_OPP4_VDD1);
    sensorData.Efuse_Vdd1_Opp_5 = INREG32(&pSysc->CONTROL_FUSE_OPP5_VDD1);

    // EFuse data for Vdd2
    sensorData.Efuse_Vdd2_Opp_1 = INREG32(&pSysc->CONTROL_FUSE_OPP1_VDD2);
    sensorData.Efuse_Vdd2_Opp_2 = INREG32(&pSysc->CONTROL_FUSE_OPP2_VDD2);
    sensorData.Efuse_Vdd2_Opp_3 = INREG32(&pSysc->CONTROL_FUSE_OPP3_VDD2);

    // SmartReflex data
    _sensorEnData.Efuse_SR       = INREG32(&pSysc->CONTROL_FUSE_SR);

    if (sensorData.Efuse_Vdd1_Opp_1 == 0x00 &&
         sensorData.Efuse_Vdd1_Opp_2 == 0x00 &&
         sensorData.Efuse_Vdd1_Opp_4 == 0x00 &&
         sensorData.Efuse_Vdd1_Opp_5 == 0x00 &&
         sensorData.Efuse_Vdd2_Opp_1 == 0x00 &&
         sensorData.Efuse_Vdd2_Opp_2 == 0x00 &&
         sensorData.Efuse_Vdd2_Opp_3 == 0x00 &&
         _sensorEnData.Efuse_SR  == 0x00)
        {
        _bSmartreflexCapable = FALSE;
        return;
        }
    else
        {
         _bSmartreflexCapable = TRUE;
        }
    
    // Opp1, Vdd1
    pSRInfo = &_rgVdd1OppMap[kOpp1]->smartReflexInfo;
    pSRInfo->rnsenn = sensorData.vdd1_opp1_rnsenn;
    pSRInfo->rnsenp = sensorData.vdd1_opp1_rnsenp;
    pSRInfo->senngain = sensorData.vdd1_opp1_senngain;
    pSRInfo->senpgain = sensorData.vdd1_opp1_senpgain;

    // Opp2, Vdd1
    pSRInfo = &_rgVdd1OppMap[kOpp2]->smartReflexInfo;
    pSRInfo->rnsenn = sensorData.vdd1_opp2_rnsenn;
    pSRInfo->rnsenp = sensorData.vdd1_opp2_rnsenp;
    pSRInfo->senngain = sensorData.vdd1_opp2_senngain;
    pSRInfo->senpgain = sensorData.vdd1_opp2_senpgain;

    // Opp3, Vdd1
    pSRInfo = &_rgVdd1OppMap[kOpp3]->smartReflexInfo;
    pSRInfo->rnsenn = sensorData.vdd1_opp3_rnsenn;
    pSRInfo->rnsenp = sensorData.vdd1_opp3_rnsenp;
    pSRInfo->senngain = sensorData.vdd1_opp3_senngain;
    pSRInfo->senpgain = sensorData.vdd1_opp3_senpgain;

    // Opp4, Vdd1
    pSRInfo = &_rgVdd1OppMap[kOpp4]->smartReflexInfo;
    pSRInfo->rnsenn = sensorData.vdd1_opp4_rnsenn;
    pSRInfo->rnsenp = sensorData.vdd1_opp4_rnsenp;
    pSRInfo->senngain = sensorData.vdd1_opp4_senngain;
    pSRInfo->senpgain = sensorData.vdd1_opp4_senpgain;

    // Opp5, Vdd1
    pSRInfo = &_rgVdd1OppMap[kOpp5]->smartReflexInfo;
    pSRInfo->rnsenn = sensorData.vdd1_opp5_rnsenn;
    pSRInfo->rnsenp = sensorData.vdd1_opp5_rnsenp;
    pSRInfo->senngain = sensorData.vdd1_opp5_senngain;
    pSRInfo->senpgain = sensorData.vdd1_opp5_senpgain;

    // Opp6, Vdd1 (duplicate of Opp5 info)
    pSRInfo = &_rgVdd1OppMap[kOpp6]->smartReflexInfo;
    pSRInfo->rnsenn = sensorData.vdd1_opp5_rnsenn;
    pSRInfo->rnsenp = sensorData.vdd1_opp5_rnsenp;
    pSRInfo->senngain = sensorData.vdd1_opp5_senngain;
    pSRInfo->senpgain = sensorData.vdd1_opp5_senpgain;

    // Opp1, Vdd2
    // VDD2 OPP1 corresponds to EFUSE VDD2 OPP2
    pSRInfo = &_rgVdd2OppMap[kOpp1]->smartReflexInfo;
    pSRInfo->rnsenn = sensorData.vdd2_opp2_rnsenn;
    pSRInfo->rnsenp = sensorData.vdd2_opp2_rnsenp;
    pSRInfo->senngain = sensorData.vdd2_opp2_senngain;
    pSRInfo->senpgain = sensorData.vdd2_opp2_senpgain;

    // Opp2, Vdd2
    // VDD2 OPP2 corresponds to EFUSE VDD2 OPP3
    pSRInfo = &_rgVdd2OppMap[kOpp2]->smartReflexInfo;
    pSRInfo->rnsenn = sensorData.vdd2_opp3_rnsenn;
    pSRInfo->rnsenp = sensorData.vdd2_opp3_rnsenp;
    pSRInfo->senngain = sensorData.vdd2_opp3_senngain;
    pSRInfo->senpgain = sensorData.vdd2_opp3_senpgain;    
    
    // initialize smartreflex library
    SmartReflex_InitializeChannel(kSmartReflex_Channel1, 
        OALPAtoVA(OMAP_SMARTREFLEX1_PA, FALSE)
        );
    SmartReflex_InitializeChannel(kSmartReflex_Channel2, 
        OALPAtoVA(OMAP_SMARTREFLEX2_PA, FALSE)
        );
}

//-----------------------------------------------------------------------------
void 
SmartReflex_PostInitialize()
{
    InitializeCriticalSection(&_rgSmartReflexInfo[kSmartReflex_Channel1].cs);
    InitializeCriticalSection(&_rgSmartReflexInfo[kSmartReflex_Channel2].cs);

    _rgSmartReflexInfo[kSmartReflex_Channel1].bInitialized = TRUE;
    _rgSmartReflexInfo[kSmartReflex_Channel2].bInitialized = TRUE;
}

//-----------------------------------------------------------------------------
void 
SmartReflex_Configure(UINT channel)
{
    if (channel == kSmartReflex_Channel1)
        {
        // Configure SR1 Parameters
        SmartReflex_SetErrorControl(kSmartReflex_Channel1, 
                                    _rgSr1ErrConfig[_rgOppVdd[channel]].errWeight,
                                    _rgSr1ErrConfig[_rgOppVdd[channel]].errMaxLimit,
                                    _rgSr1ErrConfig[_rgOppVdd[channel]].errMinLimit
                                    );

        SmartReflex_SetSensorMode(kSmartReflex_Channel1, 
                                    _sensorEnData.sr1_sennenable, 
                                    _sensorEnData.sr1_senpenable
                                    );
        
        SmartReflex_SetAvgWeight(kSmartReflex_Channel1, 
                                    BSP_SR1_SENN_AVGWEIGHT, 
                                    BSP_SR1_SENP_AVGWEIGHT
                                    );
        }
    else
        {
        // Configure SR2 Parameters
        SmartReflex_SetErrorControl(kSmartReflex_Channel2, 
                                    _rgSr2ErrConfig[_rgOppVdd[channel]].errWeight,
                                    _rgSr2ErrConfig[_rgOppVdd[channel]].errMaxLimit,
                                    _rgSr2ErrConfig[_rgOppVdd[channel]].errMinLimit
                                    );

        SmartReflex_SetSensorMode(kSmartReflex_Channel2, 
                                    _sensorEnData.sr2_sennenable, 
                                    _sensorEnData.sr2_senpenable
                                    );

        SmartReflex_SetAvgWeight(kSmartReflex_Channel2, 
                                    BSP_SR2_SENN_AVGWEIGHT,
                                    BSP_SR2_SENP_AVGWEIGHT
                                    );
        }

    // Enable Smartreflex Sensor Block
    SmartReflex_EnableSensorBlock(channel, TRUE);

    // Enable Smartreflex ErrorGenerator Block
    SmartReflex_EnableErrorGeneratorBlock(channel, TRUE);

}

//------------------------------------------------------------------------------
//
//  Function:  SmartReflex_Deconfigure
//
//  Description : Deconfigures SR module
// 
void 
SmartReflex_Deconfigure(UINT channel)
{
    // Disable Smartreflex Sensor Block
    SmartReflex_EnableSensorBlock(channel, FALSE);

    // Disable Smartreflex ErrorGenerator Block
    SmartReflex_EnableErrorGeneratorBlock(channel, FALSE);

    // Disable Smartreflex MinMaxAvg Block
    SmartReflex_EnableMinMaxAvgBlock(channel, FALSE);
}

//------------------------------------------------------------------------------
//
//  Function:  SmartReflex_VoltageFlush
//
//  Description : Flushes the Normal voltage for the current OPP
// 
void 
SmartReflex_VoltageFlush(VoltageProcessor_e vp)
{
    UINT32 interruptMask;
    UINT32 tcrr;
    UINT rampDelay = 0;

    if (vp == kVoltageProcessor1)
        {
        interruptMask = PRM_IRQENABLE_VP1_TRANXDONE_EN;
        }
    else
        {
        interruptMask = PRM_IRQENABLE_VP2_TRANXDONE_EN;
        }
    
    PrcmVoltEnableVp(vp, TRUE);
    
    // Get the delay required 
    rampDelay = PrcmVoltGetVoltageRampDelay(vp);

    // clear prcm interrupts
    PrcmInterruptClearStatus(interruptMask);

    // force SMPS voltage update through voltage processor
    PrcmVoltFlushVoltageLevels(vp);
        
    tcrr = OALTimerGetCount();
    while ((PrcmInterruptClearStatus(interruptMask) & interruptMask) == 0 &&
        (OALTimerGetCount() - tcrr) < BSP_ONE_MILLISECOND_TICKS);

    // wait for voltage change complete
    if (rampDelay)
        {
        OALStall(rampDelay);
        }
    
    PrcmVoltEnableVp(vp, FALSE);
}


//------------------------------------------------------------------------------
//
//  Function:  SmartReflex_EnableMonitor
//
//  Description : Enable/Disable Smart reflex
// 
BOOL 
SmartReflex_EnableMonitor(
    UINT                    channel,
    BOOL                    bEnable
    )
{
    VddOppSetting_t   **ppVoltDomain;
    UINT32 tcrr;
    VoltageProcessor_e  vp;
    UINT interruptMask;

    if(!_bSmartreflexCapable) goto cleanUp;

    if (_rgSmartReflexInfo[channel].bEnabled == bEnable ) goto cleanUp;
    
    SmartReflexLock(channel);

    if (channel == kSmartReflex_Channel1)
        {
        ppVoltDomain = _rgVdd1OppMap;    
        interruptMask = PRM_IRQENABLE_VP1_TRANXDONE_EN;
        vp = kVoltageProcessor1;
        }
    else
        {
        ppVoltDomain = _rgVdd2OppMap;    
        interruptMask = PRM_IRQENABLE_VP2_TRANXDONE_EN;
        vp = kVoltageProcessor2;
        }
    
    if (bEnable)
        {
        // Disable Auto Retention and Sleep
        PrcmVoltSetAutoControl(AUTO_OFF_ENABLED | AUTO_SLEEP_DISABLED | AUTO_RET_DISABLED, 
                                AUTO_OFF | AUTO_SLEEP | AUTO_RET);

        PrcmVoltEnableVp(vp, TRUE);
        
        // Enable Smartreflex
        PrcmDeviceEnableClocks(
            channel == kSmartReflex_Channel1 ? OMAP_DEVICE_SR1 : OMAP_DEVICE_SR2, 
            TRUE
            );

        // Set Interface Clk gating on Idle mode
        SmartReflex_SetIdleMode(channel, SR_CLKACTIVITY_NOIDLE);
        
        // Disable SmartReflex before updating sensor ref values
        SmartReflex_Enable(channel, FALSE);

        SmartReflex_Configure(channel);

        // enable smartreflex
        SmartReflex_SetSensorReferenceData(channel, 
            &(ppVoltDomain[_rgOppVdd[channel]]->smartReflexInfo));
        
        SmartReflex_ClearInterruptStatus(channel,        
                                         ERRCONFIG_VP_BOUNDINT_ST
                                         );

        SmartReflex_EnableInterrupts(channel,
                                        ERRCONFIG_VP_BOUNDINT_EN,
                                        TRUE
                                        );
        
        // Enable Voltage Processor and Smart Reflex
        SmartReflex_Enable(channel, TRUE);
        }
    else
        {
        // Disable Voltage Processor
        PrcmVoltEnableVp(vp, FALSE);

        // Wait till the Voltage processor is Idle
        tcrr = OALTimerGetCount();
        while(!PrcmVoltIdleCheck(vp) &&
                ((OALTimerGetCount() - tcrr) < BSP_TEN_MILLISECOND_TICKS)); 

        SmartReflex_EnableInterrupts(channel,
                                        ERRCONFIG_MCU_DISACKINT_EN,
                                        TRUE
                                        );

        // Disable SmartReflex
        SmartReflex_Enable(channel, FALSE);

        SmartReflex_EnableInterrupts(channel,
                                        ERRCONFIG_VP_BOUNDINT_EN,
                                        FALSE
                                        );

        SmartReflex_ClearInterruptStatus(channel,        
                                         ERRCONFIG_VP_BOUNDINT_ST
                                         );

        SmartReflex_Deconfigure(channel);

        // Wait till SR is disabled
        tcrr = OALTimerGetCount();
        while(((SmartReflex_ClearInterruptStatus(channel, 
                    ERRCONFIG_MCU_DISACKINT_ST) & ERRCONFIG_MCU_DISACKINT_ST) == 0) &&
                    ((OALTimerGetCount() - tcrr) < BSP_ONE_MILLISECOND_TICKS)); 

        SmartReflex_EnableInterrupts(channel,
                                        ERRCONFIG_MCU_DISACKINT_EN,
                                        FALSE
                                        );

        // Set Interface and functional Clk gating on Idle mode
        SmartReflex_SetIdleMode(channel, SR_CLKACTIVITY_IDLE);

        PrcmDeviceEnableClocks(
            channel == kSmartReflex_Channel1 ? OMAP_DEVICE_SR1 : OMAP_DEVICE_SR2, 
            FALSE
            );

        SmartReflex_VoltageFlush(vp);
        }

    _rgSmartReflexInfo[channel].bEnabled = bEnable;

    SmartReflexUnlock(channel);

cleanUp:
    return TRUE;
}

//-----------------------------------------------------------------------------
void 
UpdateRetentionVoltages(IOCTL_RETENTION_VOLTAGES *pData)
{
    BOOL    rc = FALSE;
    BOOL    bEnableSmartReflex = FALSE;
    UINT32  tcrr;
    UINT    vp;
    UINT    interruptMask[kVoltageProcessorCount] = {
                            PRM_IRQENABLE_VP1_TRANXDONE_EN,
                            PRM_IRQENABLE_VP2_TRANXDONE_EN
                            };

    if(!_bSmartreflexCapable) goto cleanUp;

    for (vp = kVoltageProcessor1 ; vp < kVoltageProcessorCount ; vp++)
        {

        OALMSG(1, (L"UpdateRetentionVoltages: VDD%d %d.%04dV \r\n", vp + 1, VP_VOLTS(pData->retentionVoltage[vp]), VP_MILLIVOLTS(pData->retentionVoltage[vp])));

        //check if SmartReflex was enabled
        if (IsSmartReflexMonitoringEnabled(vp))
            {
            bEnableSmartReflex = TRUE;
            SmartReflex_EnableMonitor(vp, FALSE);
            }
    
        // disable vp
        PrcmVoltEnableVp(vp, FALSE);
    
        // enable voltage processor timeout
        PrcmVoltEnableTimeout(vp, TRUE);
    
        // update retention value
        PrcmVoltSetVoltageLevel(
            vp, 
            pData->retentionVoltage[vp], 
            SMPS_RET_MASK
            );
        
        PrcmVoltSetVoltageLevel(
            vp, 
            pData->retentionVoltage[vp], 
            SMPS_ON_MASK
            );
    
        PrcmVoltSetVoltageLevel(
            vp, 
            pData->retentionVoltage[vp], 
            SMPS_ONLP_MASK
            );
    
        PrcmVoltSetInitVddLevel(
            vp, 
            pData->retentionVoltage[vp]
            );
    
        // enable voltage processor
        PrcmVoltEnableVp(vp, TRUE);
            
        // force SMPS voltage update through voltage processor
        PrcmVoltFlushVoltageLevels(vp);
    
        // wait for voltage change complete
        tcrr = OALTimerGetCount();
        while ((PrcmInterruptClearStatus(interruptMask[vp]) & interruptMask[vp]) == 0 &&
            (OALTimerGetCount() - tcrr) < BSP_MAX_VOLTTRANSITION_TIME);
    
    
        // disable the voltage processor sub-chip
        PrcmVoltEnableVp(vp, FALSE);
    
        //restore SmartReflex state	
        if (bEnableSmartReflex == TRUE) 
            {
            SmartReflex_EnableMonitor(vp, TRUE);
            }
        }
    
cleanUp:
    return;
}

//-----------------------------------------------------------------------------
BOOL
SetVoltageOppViaVoltageProcessor(
    VddOppSetting_t        *pVddOppSetting,
    UINT                   *retentionVoltages
    )
{
    UINT32 tcrr;
    BOOL rc = FALSE;        
    UINT interruptMask = (pVddOppSetting->vpInfo.vp == kVoltageProcessor1) ?
                            PRM_IRQENABLE_VP1_TRANXDONE_EN :
                            PRM_IRQENABLE_VP2_TRANXDONE_EN;

    // disable vp
    PrcmVoltEnableVp(pVddOppSetting->vpInfo.vp, FALSE);

    // enable voltage processor timeout
    PrcmVoltEnableTimeout(pVddOppSetting->vpInfo.vp, TRUE);

    OALMSG(OAL_INFO, (L"Vdd%d=0x%02X\r\n", pVddOppSetting->vpInfo.vp + 1,
        pVddOppSetting->vpInfo.initVolt)
        );

    // configure voltage processor parameters
    PrcmVoltSetVoltageLevel(
        pVddOppSetting->vpInfo.vp, 
        pVddOppSetting->vpInfo.initVolt, 
        SMPS_ON_MASK
        );

    OALMSG(1, (L"VDD%d %d.%04dV \r\n", pVddOppSetting->vpInfo.vp + 1, VP_VOLTS(pVddOppSetting->vpInfo.initVolt), VP_MILLIVOLTS(pVddOppSetting->vpInfo.initVolt)));

    PrcmVoltSetVoltageLevel(
        pVddOppSetting->vpInfo.vp, 
        pVddOppSetting->vpInfo.lpVolt, 
        SMPS_ONLP_MASK
        );
    
    PrcmVoltSetVoltageLevel(
        pVddOppSetting->vpInfo.vp, 
        retentionVoltages[pVddOppSetting->vpInfo.vp], 
        SMPS_RET_MASK
        );

    PrcmVoltSetInitVddLevel(
        pVddOppSetting->vpInfo.vp, 
        pVddOppSetting->vpInfo.initVolt
        );

    // clear prcm interrupts
    PrcmInterruptClearStatus(interruptMask);

    // enable voltage processor
    PrcmVoltEnableVp(pVddOppSetting->vpInfo.vp, TRUE);
        
    // force SMPS voltage update through voltage processor
    PrcmVoltFlushVoltageLevels(pVddOppSetting->vpInfo.vp);

    tcrr = OALTimerGetCount();
    while ((PrcmInterruptClearStatus(interruptMask) & interruptMask) == 0 &&
        (OALTimerGetCount() - tcrr) < BSP_ONE_MILLISECOND_TICKS);

    // disable the voltage processor sub-chip
    PrcmVoltEnableVp(pVddOppSetting->vpInfo.vp, FALSE);

    rc = TRUE;

    return rc;
}

//-----------------------------------------------------------------------------
BOOL
SetFrequencyOpp(
    VddOppSetting_t        *pVddOppSetting
    )
{
    int i;
    BOOL rc = FALSE;

    // iterate through and set the dpll frequency settings    
    for (i = 0; i < pVddOppSetting->dpllCount; ++i)
        {
        //OALMSG(1, (L"DPLL%d m=%d, n=%d, freqSel=0x%x, outputDivisor=%d\r\n", 
		//    pVddOppSetting->rgFrequencySettings[i].dpllId,
        //    pVddOppSetting->rgFrequencySettings[i].m,
        //    pVddOppSetting->rgFrequencySettings[i].n,
        //    pVddOppSetting->rgFrequencySettings[i].freqSel,
        //    pVddOppSetting->rgFrequencySettings[i].outputDivisor
        //	));

        PrcmClockSetDpllFrequency(
            pVddOppSetting->rgFrequencySettings[i].dpllId,
            pVddOppSetting->rgFrequencySettings[i].m,
            pVddOppSetting->rgFrequencySettings[i].n,
            pVddOppSetting->rgFrequencySettings[i].freqSel,
            pVddOppSetting->rgFrequencySettings[i].outputDivisor
            );
        };

    rc = TRUE;

    return rc;
}

//-----------------------------------------------------------------------------
BOOL
SetVoltageOpp(
    VddOppSetting_t    *pVddOppSetting
    )
{
    return SetVoltageOppViaVoltageProcessor(pVddOppSetting,_rgInitialRetentionVoltages);
}

//-----------------------------------------------------------------------------
BOOL 
SetOpp(
    DWORD *rgDomains,
    DWORD *rgOpps,    
    DWORD  count
    )
{
    UINT                opp;
    UINT                i;
    int                 vdd;
    VddOppSetting_t   **ppVoltDomain;
    BOOL                bEnableSmartReflex = FALSE;

    // loop through and update all changing voltage domains
    //
    for (i = 0; i < count; ++i)
        {
        // select the Opp table to use
        switch (rgDomains[i])
            {
            case DVFS_MPU1_OPP:
                // validate parameters
                if (rgOpps[i] > MAX_VDD1_OPP) continue;
                
                vdd = kVDD1;
                ppVoltDomain = _rgVdd1OppMap;                
                break;
                
            case DVFS_CORE1_OPP:
                // validate parameters
                if (rgOpps[i] > MAX_VDD2_OPP) continue;
                
                vdd = kVDD2;
                ppVoltDomain = _rgVdd2OppMap;                
                break;
                
            default:
                continue;
            }

        // check if the operating point is actually changing
        opp = rgOpps[i];
        if (_rgOppVdd[vdd] == opp) continue;

        // disable smartreflex if it's enabled
        if (IsSmartReflexMonitoringEnabled(vdd))
            {
            bEnableSmartReflex = TRUE;
            SmartReflex_EnableMonitor(vdd, FALSE);
            }

        // depending on which way the transition is occurring change
        // the frequency and voltage levels in the proper order
        if (opp > _rgOppVdd[vdd])
            {
            // transitioning to higher performance, change voltage first
            SetVoltageOpp(ppVoltDomain[opp]);
            SetFrequencyOpp(ppVoltDomain[opp]);         
            }
        else
            {
            // transitioning to lower performance, change frequency first
            SetFrequencyOpp(ppVoltDomain[opp]); 
            SetVoltageOpp(ppVoltDomain[opp]);         
            }

        if (vdd == kVDD1)
            {
            PrcmVoltSetErrorConfiguration(kVoltageProcessor1, 
                                          _rgVp1ErrConfig[opp].errOffset, 
                                          _rgVp1ErrConfig[opp].errGain);
            }
        else
            {
            PrcmVoltSetErrorConfiguration(kVoltageProcessor2, 
                                          _rgVp2ErrConfig[opp].errOffset, 
                                          _rgVp2ErrConfig[opp].errGain);
            }
            
        // update opp for voltage domain
		OALMSG(1, (L"SetOpp(%d)\r\n", opp));
        _rgOppVdd[vdd] = opp;

        // re-enable smartreflex if previously enabled
        if (bEnableSmartReflex == TRUE) 
            {
            SmartReflex_EnableMonitor(vdd, TRUE);
            }
        }

    OALLED(LED_IDX_MPU_VDD, (_rgOppVdd[kVDD1] + 1));
    OALLED(LED_IDX_CORE_VDD, (_rgOppVdd[kVDD2] + 1));
                
    // update latency table
    OALWakeupLatency_UpdateOpp(rgDomains, rgOpps, count);
    
    return TRUE;    
}

//-----------------------------------------------------------------------------

