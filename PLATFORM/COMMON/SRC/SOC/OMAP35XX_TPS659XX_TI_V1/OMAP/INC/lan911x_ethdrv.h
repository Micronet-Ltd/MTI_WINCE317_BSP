/*****************************************************************************
   
    Copyright (c) 2005, SMSC. All rights reserved.

    Use of this source code is subject to the terms of the SMSC Software
    License Agreement (SLA) under which you licensed this software product.  
    If you did not accept the terms of the SLA, you are not authorized to use
    this source code. 

      This code and information is provided as is without warranty of any kind, 
      either expressed or implied, including but not limited to the implied
      warranties of merchantability and/or fitness for a particular purpose.
     
    File name   : lan911x.c 
    Description : LAN911x EBOOT driver

    History     :
*****************************************************************************/

//------------------------------------------------------------------------------
// This file comes from WINCE500\PLATFORM\COMMON\SRC\INC\oal_ethdrv.h
//
#if __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// Include ethdbg.h until we remove prototypes from it...

#include <halether.h>

//------------------------------------------------------------------------------
// Prototypes for SMSC LAN911x

BOOL   LAN911xInit(UINT8 *pAddress, UINT32 offset, UINT16 mac[3]);
UINT16 LAN911xSendFrame(UINT8 *pBuffer, UINT32 length);
UINT16 LAN911xGetFrame(UINT8 *pBuffer, UINT16 *pLength);
VOID   LAN911xEnableInts(void);
VOID   LAN911xDisableInts(void);
VOID   LAN911xCurrentPacketFilter(UINT32 filter);
BOOL   LAN911xMulticastList(UINT8 *pAddresses, UINT32 count);

#define OAL_ETHDRV_LAN911x  { \
    LAN911xInit, NULL, NULL, LAN911xSendFrame, LAN911xGetFrame, \
    LAN911xEnableInts, LAN911xDisableInts, \
    NULL, NULL,  LAN911xCurrentPacketFilter, LAN911xMulticastList \
}

#if __cplusplus
}
#endif
