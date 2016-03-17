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
#ifndef _CMD_INTERPRET_OID_H_
#define _CMD_INTERPRET_OID_H_
/*  Command interpreter header file */

#include "cu_event.h"

#define WLAN_PROTOCOL_NAME    "IEEE 802.11ABG"

#define TKIP_KEY_LENGTH		32
#define AES_KEY_LENGTH      16
#define WEP_KEY_LENGTH_40   5
#define WEP_KEY_LENGTH_104   13

#define MAX_THROUGHPUT 5500000


#define GEN_MAXIMUM_EXTRA_TOTAL_SIZE 14
#define TRANSMIT_BUFFER_SPACE 64*1514
#define RECEIVE_BUFFER_SPACE 64*1514
#define MAXIMUM_SEND_PACKETS 1
#define VENDOR_DESCRIPTION "TNET1271"
#define VENDOR_DESCRIPTION_LENGTH 9
#define VENDOR_ID_LENGTH 4
#define VENDOR_DRIVER_VERSION 0x05000200
#define LINK_SPEED 540000
#define NUMBER_OF_ANTENNAS 1

#define OID_TI_WILINK_IOCTL         0xFF010200 

#define CHECK_PENDING_RESULT(x,y)                   if (x == COMMAND_PENDING) { os_printf ("Unexpected COMMAND PENDING result (cmd = 0x%x)\n",y.paramType);  break; }

#define OS_OID_SET_FLAG 0x00000001
#define OS_OID_GET_FLAG	0x00000002

#define BIT_TO_BYTE_FACTOR  8
#define MAX_DATA_FILTER_MASK_LENGTH         8
#define MAX_DATA_FILTER_PATTERN_LENGTH      64
#define IW_ESSID_MAX_SIZE 32

typedef struct {
	TI_UINT16	id;
	TI_UINT8	events[IPC_EVENT_MAX];
} CmdAppEvents, *pCmdAppEvents;

typedef struct 
{
    TI_HANDLE               hOs;            				/* Pointer to the adapter object */
	TI_HANDLE               hReport;            			/* Pointer to the Report handle */
	TI_UINT8				nickName[IW_ESSID_MAX_SIZE + 1];/* Interface nickname */
    TI_HANDLE               hEvHandler;     				/* Event-handler module handle */
	TI_HANDLE				hCmdHndlr;						/* Handle to the Command-Handler */
	TI_HANDLE				hCmdDispatch;					/* Handle to the Command-Dispatcher */
	TI_HANDLE				hEvents[IPC_EVENT_MAX];			/* Contains handlers of events registered to */
	CmdAppEvents			appEvents[TI_MAX_APPLICATIONS];	/* application events */
    TConfigCommand         *pAsyncCmd;       				/* Pointer to the command currently being processed */
} cmdInterpret_t;


/*
 * Note:  Order is important.  The Supported list requires these to be
 * returned in increasing order....
*/
static NDIS_OID SupportedOids[] = 
{
    OID_GEN_SUPPORTED_LIST,                /* 0x00010101 */
    OID_GEN_HARDWARE_STATUS,               /* 0x00010102 */
    OID_GEN_MEDIA_SUPPORTED,               /* 0x00010103 */
    OID_GEN_MEDIA_IN_USE,                  /* 0x00010104 */
    OID_GEN_MAXIMUM_LOOKAHEAD,             /* 0x00010105 */
    OID_GEN_MAXIMUM_FRAME_SIZE,            /* 0x00010106 */
    OID_GEN_LINK_SPEED,                    /* 0x00010107 */
    OID_GEN_TRANSMIT_BUFFER_SPACE,         /* 0x00010108 */
    OID_GEN_RECEIVE_BUFFER_SPACE,          /* 0x00010109 */
    OID_GEN_TRANSMIT_BLOCK_SIZE,           /* 0x0001010A */
    OID_GEN_RECEIVE_BLOCK_SIZE,            /* 0x0001010B */
    OID_GEN_VENDOR_ID,                     /* 0x0001010C */
    OID_GEN_VENDOR_DESCRIPTION,            /* 0x0001010D */
    OID_GEN_CURRENT_PACKET_FILTER,         /* 0x0001010E */
    OID_GEN_CURRENT_LOOKAHEAD,             /* 0x0001010F */
    OID_GEN_DRIVER_VERSION,                /* 0x00010110 */
    OID_GEN_MAXIMUM_TOTAL_SIZE,            /* 0x00010111 */
    OID_GEN_PROTOCOL_OPTIONS,              /* 0x00010112 */
    OID_GEN_MAC_OPTIONS,                   /* 0x00010113 */
    OID_GEN_MEDIA_CONNECT_STATUS,          /* 0x00010114 */
    OID_GEN_MAXIMUM_SEND_PACKETS,          /* 0x00010115 */
    OID_GEN_VENDOR_DRIVER_VERSION,         /* 0x00010116 */
    
    OID_GEN_MEDIA_CAPABILITIES,            /* 0x00010201 */
    OID_GEN_PHYSICAL_MEDIUM,               /* 0x00010202 */
    OID_GEN_XMIT_OK,                       
    OID_GEN_RCV_OK,                        
    OID_GEN_XMIT_ERROR,                    
    OID_GEN_RCV_ERROR,                     
    OID_GEN_RCV_NO_BUFFER,                 
    OID_GEN_DIRECTED_BYTES_XMIT,           
    OID_GEN_DIRECTED_FRAMES_XMIT,          
    OID_GEN_MULTICAST_BYTES_XMIT,          
    OID_GEN_MULTICAST_FRAMES_XMIT,         
    OID_GEN_BROADCAST_BYTES_XMIT,          
    OID_GEN_BROADCAST_FRAMES_XMIT,         
    OID_GEN_DIRECTED_BYTES_RCV,            
    OID_GEN_DIRECTED_FRAMES_RCV,           
    OID_GEN_MULTICAST_BYTES_RCV,           
    OID_GEN_MULTICAST_FRAMES_RCV,          
    OID_GEN_BROADCAST_BYTES_RCV,           
    OID_GEN_BROADCAST_FRAMES_RCV,          
    OID_GEN_RCV_CRC_ERROR,                 
    OID_GEN_INIT_TIME_MS,                  /* 0x00020213 */
    OID_GEN_MEDIA_SENSE_COUNTS,            /* 0x00020215 */

    OID_802_3_PERMANENT_ADDRESS,
    OID_802_3_CURRENT_ADDRESS,             
    OID_802_3_MULTICAST_LIST,              
    OID_802_3_MAXIMUM_LIST_SIZE,           
    
    OID_802_3_RCV_ERROR_ALIGNMENT,
    OID_802_3_XMIT_ONE_COLLISION,
    OID_802_3_XMIT_MORE_COLLISIONS,
    
    OID_802_11_BSSID,
    OID_802_11_SSID,
    OID_802_11_NETWORK_TYPES_SUPPORTED,
    OID_802_11_NETWORK_TYPE_IN_USE,
    OID_802_11_TX_POWER_LEVEL,
    OID_802_11_RSSI,
    OID_802_11_RSSI_TRIGGER,
    OID_802_11_INFRASTRUCTURE_MODE,
    OID_802_11_FRAGMENTATION_THRESHOLD,
    OID_802_11_RTS_THRESHOLD,
    OID_802_11_NUMBER_OF_ANTENNAS,
    OID_802_11_RX_ANTENNA_SELECTED,
    OID_802_11_TX_ANTENNA_SELECTED,
    OID_802_11_SUPPORTED_RATES,
    OID_802_11_DESIRED_RATES,
    OID_802_11_CONFIGURATION,
    OID_802_11_STATISTICS,
    OID_802_11_ADD_WEP,
    OID_802_11_REMOVE_WEP,
    OID_802_11_ADD_KEY,
    OID_802_11_REMOVE_KEY,
    OID_802_11_ASSOCIATION_INFORMATION,
    OID_802_11_DISASSOCIATE,
    OID_802_11_POWER_MODE,
    OID_802_11_BSSID_LIST,
    OID_802_11_AUTHENTICATION_MODE,
    OID_802_11_PRIVACY_FILTER,
    OID_802_11_BSSID_LIST_SCAN,
    OID_802_11_WEP_STATUS,
    OID_802_11_RELOAD_DEFAULTS,
    OID_802_11_TEST,
    OID_802_11_CAPABILITY,
    OID_802_11_PMKID,
    OID_FSW_802_11_AVAILABLE_OPTIONS,    /* propriatary OID for FUNK supplicant*/
    OID_FSW_802_11_OPTIONS,              /* propriatary OID for FUNK supplicant*/
    OID_TI_WILINK_IOCTL,                 
    
    /* Power Management OIDs*/
    OID_PNP_CAPABILITIES,
    OID_PNP_SET_POWER,
    OID_PNP_QUERY_POWER,
    OID_PNP_ADD_WAKE_UP_PATTERN,
    OID_PNP_REMOVE_WAKE_UP_PATTERN,
    OID_PNP_ENABLE_WAKE_UP 			/* begin change by removing comma */
    
    /* non-standard, do not support */
    /*  OID_TI_802_11_REG_DOMAIN24, */
    /*  OID_TI_802_11_REG_DOMAIN50 */
};


static NDIS_GUID SupportedGuids[] =
{
    /* NDIS_GUID ng = {CGUID_FSW_CCX_CONFIGURATION, OID_FSW_CCX_CONFIGURATION, 4, fNDIS_GUID_TO_OID};*/
    /*
    {CGUID_FSW_802_11_AVAILABLE_OPTIONS, OID_FSW_802_11_AVAILABLE_OPTIONS, sizeof(TI_UINT32),
        fNDIS_GUID_TO_OID | fNDIS_GUID_ALLOW_READ},

    {CGUID_FSW_802_11_OPTIONS, OID_FSW_802_11_OPTIONS, sizeof(TI_UINT32),
	fNDIS_GUID_TO_OID | fNDIS_GUID_ALLOW_READ | fNDIS_GUID_ALLOW_WRITE}
     */
    {CGUID_FSW_802_11_AVAILABLE_OPTIONS, OID_FSW_802_11_AVAILABLE_OPTIONS, sizeof(UINT32),fNDIS_GUID_TO_OID},

    {CGUID_FSW_802_11_OPTIONS, OID_FSW_802_11_OPTIONS, sizeof(UINT32),fNDIS_GUID_TO_OID}
};


#endif /* _CMD_INTERPRET_OID_H_ */
