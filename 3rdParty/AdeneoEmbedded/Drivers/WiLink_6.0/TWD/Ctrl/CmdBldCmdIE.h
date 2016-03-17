/***************************************************************************
**+----------------------------------------------------------------------+**
**|                                ****                                  |**
**|                                ****                                  |**
**|                                ******o***                            |**
**|                          ********_///_****                           |**
**|                           ***** /_//_/ ****                          |**
**|                            ** ** (__/ ****                           |**
**|                                *********                             |**
**|                                 ****                                 |**
**|                                  ***                                 |**
**|                                                                      |**
**|     Copyright (c) 1998 - 2009 Texas Instruments Incorporated         |**
**|                        ALL RIGHTS RESERVED                           |**
**|                                                                      |**
**| Permission is hereby granted to licensees of Texas Instruments       |**
**| Incorporated (TI) products to use this computer program for the sole |**
**| purpose of implementing a licensee product based on TI products.     |**
**| No other rights to reproduce, use, or disseminate this computer      |**
**| program, whether in part or in whole, are granted.                   |**
**|                                                                      |**
**| TI makes no representation or warranties with respect to the         |**
**| performance of this computer program, and specifically disclaims     |**
**| any responsibility for any damages, special or consequential,        |**
**| connected with the use of this program.                              |**
**|                                                                      |**
**+----------------------------------------------------------------------+**
***************************************************************************/

/** \file  CmdBldCmdIE.h 
 *  \brief Command builder. Command information elements
 *
 *  \see   CmdBld.c 
 */

#ifndef CMDBLDCMDIE_H
#define CMDBLDCMDIE_H


TI_STATUS cmdBld_CmdIeStartBss          (TI_HANDLE hCmdBld, BSS_e BssType, void *fJoinCompleteCB, TI_HANDLE hCb);
TI_STATUS cmdBld_CmdIeEnableRx          (TI_HANDLE hCmdBld, void *fCb, TI_HANDLE hCb);
TI_STATUS cmdBld_CmdIeEnableTx          (TI_HANDLE hCmdBld, TI_UINT8 channel, void *fCb, TI_HANDLE hCb);
TI_STATUS cmdBld_CmdIeDisableRx         (TI_HANDLE hCmdBld, void *fCb, TI_HANDLE hCb);
TI_STATUS cmdBld_CmdIeDisableTx         (TI_HANDLE hCmdBld, void *fCb, TI_HANDLE hCb);
TI_STATUS cmdBld_CmdIeInitMemory        (TI_HANDLE hCmdBld, void *fCb, TI_HANDLE hCb);
TI_STATUS cmdBld_CmdIeConfigureTemplateFrame (TI_HANDLE hCmdBld, TTemplateParams *pTemplate, TI_UINT16 uFrameSize, TemplateType_e templateType, TI_UINT8 uIndex, void *fCb, TI_HANDLE hCb);
TI_STATUS cmdBld_CmdIeStartScan         (TI_HANDLE hCmdBld, ScanParameters_t* pScanParams, void* fScanCommandResponseCB, TI_HANDLE hCb);
TI_STATUS cmdBld_CmdIeStartSPSScan      (TI_HANDLE hCmdBld, ScheduledScanParameters_t* pScanParams, void* fScanCommandResponseCB, TI_HANDLE hCb);
TI_STATUS cmdBld_CmdIeStopScan          (TI_HANDLE hCmdBld, void *fScanCommandResponseCB, TI_HANDLE hCb);
TI_STATUS cmdBld_CmdIeStopSPSScan       (TI_HANDLE hCmdBld, void *fScanCommandResponseCB, TI_HANDLE hCb);
TI_STATUS cmdBld_CmdIeSetSplitScanTimeOut (TI_HANDLE hCmdBld, TI_UINT32 uTimeOut, void *fCB, TI_HANDLE hCb);
TI_STATUS cmdBld_CmdIeScanSsidList      (TI_HANDLE hCmdBld, ConnScanSSIDList_t *pSsidList, void* fScanResponseCB, TI_HANDLE hCb);
TI_STATUS cmdBld_CmdIePeriodicScanParams(TI_HANDLE hCmdBld, ConnScanParameters_t *pPeriodicScanParams, void* fScanResponseCB, TI_HANDLE hCb);
TI_STATUS cmdBld_CmdIeStartPeriodicScan (TI_HANDLE hCmdBld, PeriodicScanTag* pPeriodicScanStart, void* fScanResponseCB, TI_HANDLE hCb);
TI_STATUS cmdBld_CmdIeStopPeriodicScan  (TI_HANDLE hCmdBld, PeriodicScanTag* pPeriodicScanStop, void* fScanResponseCB, TI_HANDLE hCb);
TI_STATUS cmdBld_CmdIeNoiseHistogram    (TI_HANDLE hCmdBld, TNoiseHistogram *pNoiseHistParams, void *fCb, TI_HANDLE hCb);
TI_STATUS cmdBld_CmdIeSwitchChannel     (TI_HANDLE hCmdBld, TSwitchChannelParams *pSwitchChannelCmd, void *fCb, TI_HANDLE hCb);
TI_STATUS cmdBld_CmdIeSwitchChannelCancel (TI_HANDLE hCmdBld, void *fCb, TI_HANDLE hCb);
TI_STATUS cmdBld_CmdIeSetKey            (TI_HANDLE hCmdBld, TI_UINT32 uAction, TI_UINT8 *pMacAddr, TI_UINT32 uKeySize, TI_UINT32 uKeyType, TI_UINT32 uKeyId, TI_UINT8 *pKey, TI_UINT32 uSecuritySeqNumLow, TI_UINT32 SecuritySeqNumHigh, void *fCb, TI_HANDLE hCb);
TI_STATUS cmdBld_CmdIeSetPsMode         (TI_HANDLE hCmdBld, TPowerSaveParams *pPowerSaveParams, void *fCb, TI_HANDLE hCb);
TI_STATUS cmdBld_CmdIeStartNewScan      (TI_HANDLE hCmdBld, TScanParams *pScanParams, void *fCb, TI_HANDLE hCb);
TI_STATUS cmdBld_CmdIeFwDisconnect      (TI_HANDLE hCmdBld, TI_UINT32 uConfigOptions, TI_UINT32 uFilterOptions, DisconnectType_e uDisconType, TI_UINT16 uDisconReason, void *fCb, TI_HANDLE hCb);
TI_STATUS cmdBld_CmdIeMeasurement       (TI_HANDLE hCmdBld, TMeasurementParams *pMeasurementParams, void* fMeasureCommandResponseCb, TI_HANDLE hCb);
TI_STATUS cmdBld_CmdIeMeasurementStop   (TI_HANDLE hCmdBld, void *fMeasureCommandResponseCb, TI_HANDLE hCb);
TI_STATUS cmdBld_CmdIeApDiscovery       (TI_HANDLE hCmdBld, TApDiscoveryParams* pMeasurementParams, void *fCb, TI_HANDLE hCb);
TI_STATUS cmdBld_CmdIeApDiscoveryStop   (TI_HANDLE hCmdBld, void *fCb, TI_HANDLE hCb);
TI_STATUS cmdBld_CmdIeHealthCheck       (TI_HANDLE hCmdBld, void *fCb, TI_HANDLE hCb);
TI_STATUS cmdBld_CmdIeSetStaState       (TI_HANDLE hCmdBld, TI_UINT8 staState, void *fCb, TI_HANDLE hCb);

TI_STATUS cmdBld_CmdIeTest              (TI_HANDLE hCmdBld, void *fCb, TI_HANDLE hCb, TTestCmd* pTestCmdBuf);


#endif
