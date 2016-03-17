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
#define __FILE_ID__  FILE_ID_96
#include "osApi.h"
#include "report.h"
#include "CmdBld.h"
#include "CmdQueue_api.h"


TI_STATUS cmdBld_ItrIeMemoryMap (TI_HANDLE hCmdBld, MemoryMap_t *apMap, void *fCb, TI_HANDLE hCb)
{
    TCmdBld *pCmdBld = (TCmdBld *)hCmdBld;
   
    /* Set information element header */
    apMap->EleHdr.id  = ACX_MEM_MAP;
    apMap->EleHdr.len = sizeof(*apMap) - sizeof(EleHdrStruct);

    return cmdQueue_SendCommand (pCmdBld->hCmdQueue, CMD_INTERROGATE, apMap, sizeof(*apMap), fCb, hCb, apMap);
}


/****************************************************************************
 *                      cmdBld_ItrIeRoamimgStatisitics ()
 ****************************************************************************
 * DESCRIPTION: Get the ACX GWSI statistics 
 *
 * INPUTS:
 *
 * OUTPUT:  None
 *
 * RETURNS: TI_OK or TI_NOK
 ****************************************************************************/
TI_STATUS cmdBld_ItrIeRoamimgStatisitics (TI_HANDLE  hCmdBld, void *fCb, TI_HANDLE hCb, void *pCb)
{
    TCmdBld *pCmdBld = (TCmdBld *)hCmdBld;
    ACXRoamingStatisticsTable_t acx;
    ACXRoamingStatisticsTable_t * pCfg = &acx;

    /* 
     * Set information element header
     */
    pCfg->EleHdr.id  = ACX_ROAMING_STATISTICS_TBL;
    pCfg->EleHdr.len = sizeof(*pCfg) - sizeof(EleHdrStruct);

   
    return cmdQueue_SendCommand (pCmdBld->hCmdQueue, CMD_INTERROGATE, pCfg, sizeof(*pCfg), fCb, hCb, pCb);
}


/****************************************************************************
 *                      cmdBld_ItrIeErrorCnt ()
 ****************************************************************************
 * DESCRIPTION: Get the ACX GWSI counters 
 *
 * INPUTS:
 *
 * OUTPUT:  None
 *
 * RETURNS: TI_OK or TI_NOK
 ****************************************************************************/
TI_STATUS cmdBld_ItrIeErrorCnt (TI_HANDLE hCmdBld, void *fCb, TI_HANDLE hCb, void *pCb)
{
    TCmdBld *pCmdBld = (TCmdBld *)hCmdBld;
    ACXErrorCounters_t acx;
    ACXErrorCounters_t * pCfg = &acx;

    /* 
     * Set information element header
     */
    pCfg->EleHdr.id  = ACX_ERROR_CNT;
    pCfg->EleHdr.len = sizeof(*pCfg) - sizeof(EleHdrStruct);

     
    return cmdQueue_SendCommand (pCmdBld->hCmdQueue, CMD_INTERROGATE, pCfg, sizeof(*pCfg), fCb, hCb, pCb);
}


/****************************************************************************
 *                      cmdBld_ItrIeRSSI ()
 ****************************************************************************
 * DESCRIPTION: Configure/Interrogate StationId information element to/from
 *      the wlan hardware.
 *      This information element specifies the MAC Address assigned to the
 *      STATION or AP.
 *      This default value is the permanent MAC address that is stored in the
 *      adaptor's non-volatile memory.
 *
 * INPUTS:  None
 *
 * OUTPUT:  None
 *
 * RETURNS: TI_OK or TI_NOK
 ****************************************************************************/
TI_STATUS cmdBld_ItrIeRSSI (TI_HANDLE hCmdBld, void *fCb, TI_HANDLE hCb, TI_UINT8* pCb)
{
    TCmdBld *pCmdBld = (TCmdBld *)hCmdBld;
    ACXRoamingStatisticsTable_t AcxElm_GetAverageRSSI;
    ACXRoamingStatisticsTable_t* pCfg = &AcxElm_GetAverageRSSI;

    /* Set information element header */
    pCfg->EleHdr.id = ACX_ROAMING_STATISTICS_TBL;
    pCfg->EleHdr.len = sizeof(ACXRoamingStatisticsTable_t) - sizeof(EleHdrStruct);

      
    return cmdQueue_SendCommand (pCmdBld->hCmdQueue, CMD_INTERROGATE, pCfg, sizeof(ACXRoamingStatisticsTable_t), fCb, hCb, pCb);
}


/****************************************************************************
 *                      cmdBld_ItrIeSg()
 ****************************************************************************
 * DESCRIPTION: Get the BTH-WLAN co-exsistance parameters from the Fw   
 *
 *
 * OUTPUT:  None
 *
 * RETURNS: TI_OK or TI_NOK
 ****************************************************************************/
TI_STATUS cmdBld_ItrIeSg (TI_HANDLE hCmdBld, void *fCb, TI_HANDLE hCb, void* pCb)
{
    TCmdBld *pCmdBld = (TCmdBld *)hCmdBld;
    ACXBluetoothWlanCoParamsStruct  AcxElm_BluetoothWlanEnable;
    ACXBluetoothWlanCoParamsStruct* pCfg = &AcxElm_BluetoothWlanEnable;

    TRACE0(pCmdBld->hReport, REPORT_SEVERITY_INFORMATION, "cmdBld_ItrIeSg \n");

    /* Set information element header */
    pCfg->EleHdr.id = ACX_SG_CFG;
    pCfg->EleHdr.len = sizeof(*pCfg) - sizeof(EleHdrStruct);

   return cmdQueue_SendCommand (pCmdBld->hCmdQueue, CMD_INTERROGATE, pCfg, sizeof(ACXBluetoothWlanCoParamsStruct), fCb, hCb, pCb);
}
/****************************************************************************
 *                      cmdBld_ItrIeRateParams()
 ****************************************************************************
 * DESCRIPTION: Get the rate managment configuration  
 *
 *
 * OUTPUT:  None
 *
 * RETURNS: TI_OK or TI_NOK
 ****************************************************************************/

TI_STATUS cmdBld_ItrIeRateParams (TI_HANDLE hCmdBld, void *fCb, TI_HANDLE hCb, void* pCb)
{
    TCmdBld *pCmdBld = (TCmdBld *)hCmdBld;
	AcxRateMangeParams  RateParams;
    AcxRateMangeParams* pCfg = &RateParams;

    TRACE0(pCmdBld->hReport, REPORT_SEVERITY_INFORMATION, "cmdBld_ItrIeRateParams \n");

    /* Set information element header */
    pCfg->EleHdr.id = ACX_GET_RATE_MAMAGEMENT_PARAMS;
    pCfg->EleHdr.len = sizeof(*pCfg) - sizeof(EleHdrStruct);

    return cmdQueue_SendCommand (pCmdBld->hCmdQueue, CMD_INTERROGATE, pCfg, sizeof(AcxRateMangeParams), fCb, hCb, pCb);
}

/****************************************************************************
 *                      cmdBld_ItrIePowerConsumptionstat()
 ****************************************************************************
 * DESCRIPTION: Get the Power consumption statistic from the Fw   
 *
 *
 * OUTPUT:  None
 *
 * RETURNS: TI_OK or TI_NOK
 ****************************************************************************/
TI_STATUS cmdBld_ItrIePowerConsumptionstat(TI_HANDLE hCmdBld, void *fCb, TI_HANDLE hCb, void* pCb)
{
    TCmdBld *pCmdBld = (TCmdBld *)hCmdBld;
    ACXPowerConsumptionTimeStat_t  AcxPowerConsumptionStat;
    ACXPowerConsumptionTimeStat_t* pCfg = &AcxPowerConsumptionStat;

    TRACE0(pCmdBld->hReport, REPORT_SEVERITY_INFORMATION, "cmdBld_ItrIePowerConsumptionstat \n");

    /* Set information element header */
    pCfg->EleHdr.id = ACX_PWR_CONSUMPTION_STATISTICS;
    pCfg->EleHdr.len = sizeof(*pCfg) - sizeof(EleHdrStruct);
   
    return cmdQueue_SendCommand (pCmdBld->hCmdQueue, CMD_INTERROGATE, pCfg, sizeof(AcxPowerConsumptionStat), fCb, hCb, pCb);

}



/****************************************************************************
 *                      cmdBld_ItrIeStatistics ()
 ****************************************************************************
 * DESCRIPTION: Print the statistics from the input IE statistics
 *
 * INPUTS:
 *          ACXStatisticsStruct* pElem  The Statistics information element
 *                                      to be printed
 *
 * OUTPUT:  None
 *
 * RETURNS: TI_OK or TI_NOK
 ****************************************************************************/
TI_STATUS cmdBld_ItrIeStatistics (TI_HANDLE hCmdBld, void *fCb, TI_HANDLE hCb, void *pCb)
{
    TCmdBld *pCmdBld = (TCmdBld *)hCmdBld;
    ACXStatistics_t *pACXStatistics = (ACXStatistics_t *)pCb;

    /* Set information element header */
    pACXStatistics->EleHdr.id  = ACX_STATISTICS;
    pACXStatistics->EleHdr.len = sizeof(*pACXStatistics) - sizeof(EleHdrStruct);

    return cmdQueue_SendCommand (pCmdBld->hCmdQueue, CMD_INTERROGATE, pCb, sizeof(*pACXStatistics), fCb, hCb, pCb);
}


/****************************************************************************
 *                      cmdBld_ItrIeMediumOccupancy ()
 ****************************************************************************
 * DESCRIPTION: Get the Medium Occupancy.
 *
 * INPUTS:
 *
 * OUTPUT:  None
 *
 * RETURNS: TI_OK or TI_NOK
 ****************************************************************************/
TI_STATUS cmdBld_ItrIeMediumOccupancy (TI_HANDLE hCmdBld,
                                       TInterrogateCmdCbParams  mediumUsageCBParams)
{
    TCmdBld *pCmdBld = (TCmdBld *)hCmdBld;
    ACXMediumUsage_t    medium;
    ACXMediumUsage_t    *pCfg = &medium;

    /* Set information element header */
    pCfg->EleHdr.id  = ACX_MEDIUM_USAGE;
    pCfg->EleHdr.len = sizeof(*pCfg) - sizeof(EleHdrStruct);

    return cmdQueue_SendCommand (pCmdBld->hCmdQueue, 
                                 CMD_INTERROGATE, 
                                 pCfg, 
                                 sizeof(*pCfg), 
                                 mediumUsageCBParams.fCb, 
                                 mediumUsageCBParams.hCb, 
                                 mediumUsageCBParams.pCb);
}


/****************************************************************************
 *                      cmdBld_ItrIeTfsDtim ()
 ****************************************************************************
 * DESCRIPTION: Get the Tsf and Dtim counter from Fw
 *
 * INPUTS:
 *
 * OUTPUT:  None
 *
 * RETURNS: TI_OK or TI_NOK
 ****************************************************************************/
TI_STATUS cmdBld_ItrIeTfsDtim (TI_HANDLE hCmdBld,
                               TInterrogateCmdCbParams  mediumUsageCBParams)
{
    TCmdBld *pCmdBld = (TCmdBld *)hCmdBld;
    ACX_fwTSFInformation_t    fwTsfDtimMib;
    ACX_fwTSFInformation_t    *pCfg = &fwTsfDtimMib;

    /* Set information element header*/
    pCfg->EleHdr.id  = ACX_TSF_INFO;
    pCfg->EleHdr.len = sizeof(ACX_fwTSFInformation_t) - sizeof(EleHdrStruct);

    return cmdQueue_SendCommand (pCmdBld->hCmdQueue, 
                                 CMD_INTERROGATE, 
                                 pCfg, 
                                 sizeof(*pCfg), 
                                 mediumUsageCBParams.fCb, 
                                 mediumUsageCBParams.hCb, 
                                 mediumUsageCBParams.pCb);
}


/****************************************************************************
 *                      cmdBld_ItrIeNoiseHistogramResults()
 ****************************************************************************
 * DESCRIPTION: Get the Noise Histogram Measurement Results.
 *
 * INPUTS:
 *
 * OUTPUT:  None
 *
 * RETURNS: TI_OK or TI_NOK
 ****************************************************************************/
TI_STATUS cmdBld_ItrIeNoiseHistogramResults (TI_HANDLE hCmdBld,
                                             TInterrogateCmdCbParams noiseHistCBParams)
{
    TCmdBld *pCmdBld = (TCmdBld *)hCmdBld;
    NoiseHistResult_t   results;
    NoiseHistResult_t   *pCfg = &results;

    /* Set information element header*/
    pCfg->EleHdr.id  = ACX_NOISE_HIST;
    pCfg->EleHdr.len = sizeof(*pCfg) - sizeof(EleHdrStruct);

    return cmdQueue_SendCommand (pCmdBld->hCmdQueue, 
                                 CMD_INTERROGATE, 
                                 pCfg, 
                                 sizeof(*pCfg), 
                                 noiseHistCBParams.fCb, 
                                 noiseHistCBParams.hCb, 
                                 noiseHistCBParams.pCb);
} 

/****************************************************************************
 *                      cmdBld_ItrIeDataFilterStatistics()
 ****************************************************************************
 * DESCRIPTION: Get the ACX GWSI counters 
 *
 * INPUTS:
 *
 * OUTPUT:  None
 *
 * RETURNS: TI_OK or TI_NOK
 ****************************************************************************/
TI_STATUS cmdBld_ItrIeDataFilterStatistics (TI_HANDLE  hCmdBld, 
                                            void      *fCb, 
                                            TI_HANDLE  hCb, 
                                            void      *pCb)
{
	TCmdBld       *pCmdBld = (TCmdBld *)hCmdBld;
    ACXDataFilteringStatistics_t acx;
    ACXDataFilteringStatistics_t * pCfg = &acx;

    /* Set information element header */
    pCfg->EleHdr.id  = ACX_GET_DATA_FILTER_STATISTICS;
    pCfg->EleHdr.len = sizeof(*pCfg) - sizeof(EleHdrStruct);

    TRACE_INFO_HEX(pCmdBld->hReport, (TI_UINT8 *) pCfg, sizeof(ACXDataFilteringStatistics_t));

    /* Send the interrogation command */
    return cmdQueue_SendCommand (pCmdBld->hCmdQueue, CMD_INTERROGATE, pCfg, sizeof(*pCfg), fCb, hCb, pCb);
}


