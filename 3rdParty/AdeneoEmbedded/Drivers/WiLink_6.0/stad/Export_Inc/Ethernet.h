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

/***************************************************************************/
/*                                                                         */
/*    MODULE:   Ethernet.h                                                     */
/*    PURPOSE:                                                             */
/*                                                                         */
/***************************************************************************/
#ifndef _ETHERNET_H_
#define _ETHERNET_H_


typedef struct
{
    TMacAddr        dst;
    TMacAddr        src;
    TI_UINT16       type;

} TEthernetHeader;
 

typedef struct
{
    TMacAddr        dst;
    TMacAddr        src;
    TI_UINT16       length;
    TI_UINT8        dsap;
    TI_UINT8        ssap;
    TI_UINT8        control;
    TI_UINT8        oui[3];
    TI_UINT16       type;

} TLlcSnapHeader;


#define ETHERTYPE_802_1D                        0x8100
#define ETHERTYPE_EAPOL                         0x888e
#define ETHERTYPE_PREAUTH_EAPOL                 0x88c7
#define ETHERTYPE_IP                            0x0800
#define ETHERTYPE_APPLE_AARP                    0x80f3
#define ETHERTYPE_DIX_II_IPX                    0x8137

#define ETHERNET_HDR_LEN                        14
#define IEEE802_3_HDR_LEN                       14 
#define LLC_SNAP_HDR_LEN                        20

#define SNAP_CHANNEL_ID                         0xAA
#define LLC_CONTROL_UNNUMBERED_INFORMATION      0x03
#define ETHERNET_MAX_PAYLOAD_SIZE               1500

#define SNAP_OUI_802_1H_BYTE0                   0x00
#define SNAP_OUI_802_1H_BYTE1                   0x00
#define SNAP_OUI_802_1H_BYTE2                   0xf8
#define SNAP_OUI_802_1H_BYTES  { SNAP_OUI_802_1H_BYTE0, SNAP_OUI_802_1H_BYTE1, SNAP_OUI_802_1H_BYTE2 }

#define SNAP_OUI_RFC1042_BYTE0                  0x00
#define SNAP_OUI_RFC1042_BYTE1                  0x00
#define SNAP_OUI_RFC1042_BYTE2                  0x00
#define SNAP_OUI_RFC1042_LEN                    3
#define SNAP_OUI_RFC1042_BYTES { SNAP_OUI_RFC1042_BYTE0, SNAP_OUI_RFC1042_BYTE1, SNAP_OUI_RFC1042_BYTE2 }


#endif
