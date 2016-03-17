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
#ifndef __TIWLNIF_H__
#define __TIWLNIF_H__

#include "osDot11.h"
#ifndef TIWLNAPI_EXPORTS
#include "tidef.h"
#endif
#include "TWDriver.h"

#include "STADExternalIf.h"

typedef enum _TIWLN_DOT11_STATUS
{
    eDot11Idle           = 0,
    eDot11Scaning        = 1,
    eDot11Connecting     = 2,
    eDot11Associated     = 3,
    eDot11Disassociated  = 4,
    eDot11RadioDisabled  = 5,
  eDot11Error          = 1000
} TIWLN_DOT11_STATUS;

typedef enum _TIWLN_SECURITY_STATE  /* Values are compatible with 802.1x'S IMSTATE*/
{
    eSecurityStateHalted = 0,           /* Security state machine halted*/
    eSecurityStateStarting,         /* state machine is starting*/
    eSecurityStateInitializing,     /* state machine is initializing*/
    eSecurityStateDisabled,         /* state machine is disabled*/
    eSecurityStateNotAuthenticated, /* Not authenticated state*/
    eSecurityStateAuthenticating,       /* Authentication request is sent*/
    eSecurityStateAuthenticated     /* Authenticated state*/
} TIWLN_SECURITY_STATE;

typedef struct _TIWLN_STATISTICS
{
    /**/
    /* config info*/
    /**/
    TI_UINT32                        dot11CurrentTxRate;
    TI_UINT32                        dot11CurrentChannel;
    TMacAddr                         currentMACAddress;
    OS_802_11_SSID                   dot11DesiredSSID; 
    OS_802_11_NETWORK_MODE           dot11BSSType;
    OS_802_11_AUTHENTICATION_MODE    AuthenticationMode;
    TI_BOOL                          bShortPreambleUsed;
    TI_UINT32                        RTSThreshold;
    TI_UINT32                        FragmentationThreshold;
    TI_BOOL                          bDefaultWEPKeyDefined;
    OS_802_11_WEP_STATUS             WEPStatus;
    TI_UINT32                        TxAntenna;
    TI_UINT32                        RxAntenna;
    TI_UINT32                        TxPowerDbm;
    TI_UINT32                        PowerMode;
    TI_INT32                         RxLevel;

    /**/
    /* status & AP info*/
    /**/
    TIWLN_DOT11_STATUS              dot11State;
    OS_802_11_BSSID                 targetAP; 

    /**/
    /* network layer statistics (except Tx statistics which are handled sparately)*/
    /**/
    TIWLN_COUNTERS                  tiCounters;

    /**/
    /* other statistics*/
    /**/
    TI_UINT32  dwSecuritySuit;           /* Security suit bitmask (see defines)*/
    TI_UINT32  dwSecurityState;          /* 802.1x security protocol state*/
    TI_UINT32  dwSecurityAuthStatus;     /* Security suit authentication status*/
    TI_UINT32  dwFeatureSuit;            /* Additional features suit bitmask (see defines)*/

}  TIWLN_STATISTICS;


/* Statistics security suit bitmasks*/
#define TIWLN_STAT_SECURITY_RESERVE_1   0x0001

typedef struct _RADIO_RX_QUALITY
{
    TI_INT32             Snr;
    TI_INT32             Rssi;
}TIWLN_RADIO_RX_QUALITY;

#define MAX_NUM_DATA_FILTERS                4

#endif /* __TIWLNIF_H__*/
