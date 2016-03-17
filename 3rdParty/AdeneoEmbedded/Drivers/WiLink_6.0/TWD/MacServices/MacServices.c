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
/** \file MacServices.c
 *  \brief This file include public definitions for the scan SRV module, comprising its API.
 *  \
 *  \date 6-Oct-2005
 */

#define __FILE_ID__  FILE_ID_109
#include "report.h"
#include "ScanSrv.h"
#include "MeasurementSrv.h"
#include "MacServices.h"
#include "PowerSrv_API.h"


/****************************************************************************************
 *                        MacServices_create                                                     *
 *****************************************************************************************
DESCRIPTION: Creates MacServices module
                                                                                                                               
INPUT:          hOS - handle to the OS object.
OUTPUT:     
RETURN:     handle to MacServices Object, NULL on failure .
****************************************************************************************/

TI_HANDLE MacServices_create( TI_HANDLE hOS) 
{
    MacServices_t *pMacServices = (MacServices_t*)os_memoryAlloc( hOS, sizeof(MacServices_t) );
    if ( NULL == pMacServices )
    {
        WLAN_OS_REPORT( ("ERROR: Failed to create Mac SRV module") );
        return NULL;
    }

    /* nullify all handles, so that only handles in existence will be released */
    pMacServices->hScanSRV = NULL;
    pMacServices->hPowerSrv = NULL;
 
    /* create the scanSRV handle */ 
    pMacServices->hScanSRV = MacServices_scanSRV_create(hOS);
    if ( NULL == pMacServices->hScanSRV )
    {
        MacServices_destroy( pMacServices );
        return NULL;
    }

/* create the measurment handle */
    pMacServices->hMeasurementSRV = MacServices_measurementSRV_create( hOS );
    if ( NULL == pMacServices->hMeasurementSRV )
    {
            MacServices_destroy(pMacServices);
        return NULL;
    }

    pMacServices->hPowerSrv = powerSrv_create(hOS);
    if (NULL == pMacServices->hPowerSrv  )
    {
        MacServices_destroy(pMacServices);
        return NULL;
     }

    /* store OS handle */
    pMacServices->hOS      = hOS;

    return pMacServices;
}


 
/****************************************************************************************
 *                        MacServices_destroy                                                    *
 *****************************************************************************************
DESCRIPTION: destroys MacServices module
                                                                                                                               
INPUT:          hMacServices - handle to the Mac Services object.
OUTPUT:     
RETURN:     
****************************************************************************************/
void MacServices_destroy( TI_HANDLE hMacServices ) 
{
    MacServices_t *pMacServices = (MacServices_t*)hMacServices;
    
    /* destroy all SRV modules */
    if ( NULL != pMacServices->hScanSRV )
    {
        MacServices_scanSRV_destroy( pMacServices->hScanSRV );
    }
    if ( NULL != pMacServices->hMeasurementSRV )
    {
        MacServices_measurementSRV_destroy( pMacServices->hMeasurementSRV );
    }

    if(pMacServices->hPowerSrv)
    powerSrv_destroy(pMacServices->hPowerSrv);

    /* free the Mac services allocated context */
    os_memoryFree( pMacServices->hOS, (TI_HANDLE)pMacServices , sizeof(MacServices_t) );
}


 /****************************************************************************************
 *                        MacServices_init                                                   *
 *****************************************************************************************
DESCRIPTION: Initializes the MacServices module
                                                                                                                               
INPUT:    hMacServices - handle to the Mac Services object.\n   
        hReport - handle to the report object.\n
        hHalCtrl - handle to the HAL ctrl object.\n
OUTPUT:     
RETURN:     
****************************************************************************************/
void MacServices_init (TI_HANDLE hMacServices, 
                       TI_HANDLE hReport, 
                       TI_HANDLE hTWD, 
                       TI_HANDLE hCmdBld, 
                       TI_HANDLE hEventMbox, 
                       TI_HANDLE hTimer) 
{
    MacServices_t *pMacServices = (MacServices_t*)hMacServices;

    MacServices_scanSRV_init (hMacServices, hReport, hTWD, hTimer, hEventMbox, hCmdBld);

    MacServices_measurementSRV_init (pMacServices->hMeasurementSRV, 
                                     hReport, 
                                     hCmdBld, 
                                     hEventMbox, 
                                     pMacServices->hPowerSrv,
                                     hTimer);
    
    if (powerSrv_init (pMacServices->hPowerSrv, 
                       hReport, 
                       hEventMbox, 
                       hCmdBld,
                       hTimer) != TI_OK)
    {
        WLAN_OS_REPORT(("\n.....PowerSRV_init configuration failure \n"));
        /*return TI_NOK;*/
    }
}


 /****************************************************************************************
 *                        MacServices_config                                                     *
 *****************************************************************************************
DESCRIPTION: config the MacServices moduleand sub modules
                                                                                                                               
INPUT:    hMacServices - handle to the Mac Services object.\n   
        pInitParams  - pointer to the init params
OUTPUT:     
RETURN:     
****************************************************************************************/
void MacServices_config( TI_HANDLE hMacServices, TTwdInitParams *pInitParams) 
{
    MacServices_t *pMacServices = (MacServices_t*)hMacServices;

    if (powerSrv_config(pMacServices->hPowerSrv,&pInitParams->tPowerSrv) != TI_OK)
    {
        WLAN_OS_REPORT(("\n.....PowerSRV_config failure \n"));
    }

    MacServices_scanSrv_config (pMacServices, &pInitParams->tScanSrv);
}

 /****************************************************************************************
 *                        MacServices_restart                                                     *
 *****************************************************************************************
DESCRIPTION: restart the MacServices moduleand sub modules upon recovery
                                                                                                                               
INPUT:    hMacServices - handle to the Mac Services object.\n   
OUTPUT:     
RETURN:     
****************************************************************************************/
void MacServices_restart (TI_HANDLE hMacServices) 
{
    MacServices_t *pMacServices = (MacServices_t*)hMacServices;

    scanSRV_restart (pMacServices->hScanSRV);
    measurementSRV_restart (pMacServices->hMeasurementSRV);
    powerSrv_restart (pMacServices->hPowerSrv);
}


/****************************************************************************************
 *                        MacServices_registerFailureEventCB                                                     *
 *****************************************************************************************
DESCRIPTION: register the centeral error function from the health monitor to the MacService's sub modules
                                                                                                                               
INPUT:    hMacServices      - handle to the Mac Services object.    
        failureEventCB  - pointer ro the call back
        hFailureEventObj    -handle of the Callback Object
OUTPUT:     
RETURN:     
****************************************************************************************/

void MacServices_registerFailureEventCB (TI_HANDLE hMacServices, 
                                         void * failureEventCB, 
                                         TI_HANDLE hFailureEventObj)
{
    MacServices_t *pMacServices = (MacServices_t*)hMacServices;

    powerSrvRegisterFailureEventCB (pMacServices->hPowerSrv,
                                    failureEventCB,
                                    hFailureEventObj);

    measurementSRVRegisterFailureEventCB (pMacServices->hMeasurementSRV,
                                          failureEventCB,
                                          hFailureEventObj);

    scanSRV_registerFailureEventCB (pMacServices->hScanSRV,
                                    failureEventCB,
                                    hFailureEventObj);

}

/****************************************************************************************
 *                        MacServices_powerSrv_SetPsMode                                                            *
 ****************************************************************************************
DESCRIPTION: This function is a wrapper for the power server's powerSrv_SetPsMode function
                                                                                                                   
INPUT:      - hMacServices          - handle to the Mac services object.        
            - psMode                    - Power save/Active request
            - sendNullDataOnExit        - 
            - powerSaveCBObject     - handle to the Callback function module.
            - powerSaveCompleteCB   - Callback function - for success/faild notification.
OUTPUT: 
RETURN:    TI_STATUS - TI_OK / PENDING / TI_NOK.
****************************************************************************************/

TI_STATUS MacServices_powerSrv_SetPsMode (TI_HANDLE                   hMacServices,
                                          E80211PsMode                psMode,
                                          TI_BOOL                        sendNullDataOnExit,
                                          void *                      powerSaveCompleteCBObject,
                                          TPowerSaveCompleteCb        powerSaveCompleteCB,
                                          TPowerSaveResponseCb        powerSavecmdResponseCB)
{
    MacServices_t *pMacServices = (MacServices_t*)hMacServices;

    return powerSrv_SetPsMode (pMacServices->hPowerSrv,
                               psMode,
                               sendNullDataOnExit,
                               powerSaveCompleteCBObject,
                               powerSaveCompleteCB,
                               powerSavecmdResponseCB);
}


/****************************************************************************************
 *                        MacServices_powerSrv_ReservePS                                                        *
 ****************************************************************************************
DESCRIPTION: This function is a wrapper for the power server's powerSrv_ReservePS function
                                                                                                                   
INPUT:      - hMacServices                  - handle to the Mac services object.        
            - psMode                            - Power save/Active request
            - sendNullDataOnExit                - 
            - powerSaveCBObject             - handle to the Callback function module.
            - powerSaveCompleteCB           - Callback function - for success/faild notification.
OUTPUT: 
RETURN:    TI_STATUS - TI_OK / PENDING / TI_NOK.
****************************************************************************************/
TI_STATUS MacServices_powerSrv_ReservePS(   TI_HANDLE               hMacServices,
                                            E80211PsMode psMode,
                                            TI_BOOL                    sendNullDataOnExit,
                                            void *                  powerSaveCBObject,
                                            TPowerSaveCompleteCb    powerSaveCompleteCB)
{
    MacServices_t *pMacServices = (MacServices_t*)hMacServices;

    return powerSrv_ReservePS(pMacServices->hPowerSrv,psMode,sendNullDataOnExit,powerSaveCBObject,powerSaveCompleteCB);
}


/****************************************************************************************
 *                        MacServices_powerSrv_ReleasePS                                                        *
 ****************************************************************************************
DESCRIPTION: This function is a wrapper for the power server's powerSrv_ReleasePS function
                                                                                                                   
INPUT:      - hPowerSrv                         - handle to the PowerSrv object.        
            - sendNullDataOnExit                - 
            - powerSaveCBObject     - handle to the Callback function module.
            - powerSaveCompleteCB           - Callback function - for success/faild notification.
OUTPUT: 
RETURN:    TI_STATUS - TI_OK / PENDING / TI_NOK.
****************************************************************************************/

TI_STATUS MacServices_powerSrv_ReleasePS(   TI_HANDLE                   hMacServices,
                                    TI_BOOL                        sendNullDataOnExit,
                                    void *                          powerSaveCBObject,
                                    TPowerSaveCompleteCb              powerSaveCompleteCB)
{
    MacServices_t *pMacServices = (MacServices_t*)hMacServices;

    return powerSrv_ReleasePS(pMacServices->hPowerSrv,sendNullDataOnExit,powerSaveCBObject,powerSaveCompleteCB);
}


/****************************************************************************************
 *                        MacServices_powerSrv_getPsStatus                                                       *
 *****************************************************************************************
DESCRIPTION: This function is a wrapper for the power server's powerSrv_getPsStatus function
                                                                                                                                                                       
INPUT:      - hPowerSrv                         - handle to the PowerSrv object.        
            
OUTPUT: 
RETURN:    TI_BOOL - true if the SM is in PS state -  false otherwise
****************************************************************************************/
TI_BOOL MacServices_powerSrv_getPsStatus(TI_HANDLE hMacServices)
{
    MacServices_t *pMacServices = (MacServices_t*)hMacServices;

    return powerSrv_getPsStatus( pMacServices->hPowerSrv);
}


 /****************************************************************************************
 *                        MacServices_powerSrv_SetRateModulation                                                         *
 *****************************************************************************************
DESCRIPTION: This function is a wrapper for the power server's powerSrv_SetRateModulation function
                                                                                                                                                                       
INPUT:      - hPowerSrv                         - handle to the PowerSrv object.
            - dot11mode_e - The current radio mode (A or G)
            
OUTPUT: 
RETURN:    TI_BOOL - true if the SM is in PS state -  false otherwise
****************************************************************************************/
void MacServices_powerSrv_SetRateModulation(TI_HANDLE hMacServices, TI_UINT16  rate)
{
    MacServices_t *pMacServices = (MacServices_t*)hMacServices;

    powerSrv_SetRateModulation( pMacServices->hPowerSrv,   rate);
}


TI_UINT32 MacServices_powerSrv_GetRateModulation(TI_HANDLE hMacServices)
{
    MacServices_t *pMacServices = (MacServices_t*)hMacServices;

    return powerSrv_GetRateModulation( pMacServices->hPowerSrv);
}






