/*
==============================================================
*             Texas Instruments OMAP(TM) Platform Software
* (c) Copyright Texas Instruments, Incorporated. All Rights Reserved.
*
* Use of this software is controlled by the terms and conditions found
* in the license agreement under which this software has been supplied.
*
==============================================================
*/
#include <windows.h>
#include "sdmapxclient.h"

SdmaPxClient::SdmaPxClient()
{
    m_dwClientID    = 0;
    m_dwDataLengthClient = 0;
    m_dwElementCount = 0;
    m_dwFrameCount = 0;
    m_bConfigured = FALSE;
}

SdmaPxClient::~SdmaPxClient()
{
}

BOOL SdmaPxClient::Init(DWORD dwClientID)
{
    BOOL RetVal = TRUE;
    
    // store id info
    m_dwClientID = dwClientID;
    
    // Init the sdma config
    memset(&m_dmaConfigInfoClient, 0, sizeof(DmaConfigInfo_t));

    return RetVal;
}

// Function stores the user DMA configuration information
BOOL SdmaPxClient::StoreConfig(DmaConfigInfo_t *pdmaDataConfig, DWORD dwDataLengthClient, DWORD dwElementCount, DWORD dwFrameCount)
{
    BOOL retVal = TRUE;
       
    if(pdmaDataConfig == NULL)
    {
        retVal = FALSE;
        goto CleanUp;
    }
    
    // Store a local copy of the user DMA configuration.
    // Not using memcpy_s to avoid performance degredation
    memcpy(&m_dmaConfigInfoClient, pdmaDataConfig, sizeof(DmaConfigInfo_t));
      
    // Store data length, element and frame count
    m_dwDataLengthClient = dwDataLengthClient;
    m_dwElementCount = dwElementCount;
    m_dwFrameCount = dwFrameCount;

    m_bConfigured = TRUE;
CleanUp:    
    return retVal;
}


