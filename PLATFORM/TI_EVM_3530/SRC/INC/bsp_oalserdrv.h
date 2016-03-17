//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this source code is subject to the terms of the Microsoft end-user
// license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
// If you did not accept the terms of the EULA, you are not authorized to use
// this source code. For a copy of the EULA, please see the LICENSE.RTF on your
// install media.
//
#ifndef __BSP_OALSERDRV_H
#define __BSP_OALSERDRV_H

#if __cplusplus
extern "C" {
#endif




BOOL OalUSBSerialInit(KITL_SERIAL_INFO *pInfo);
VOID OalUSBSerialDeInit();
UINT16 OalUSBSerialRecv(UINT8 *pData, UINT16 size);
UINT16 OalUSBSerialSend(UINT8 *pData, UINT16 size);
VOID OalUSBSerialSendComplete(UINT16 size);
VOID OalUSBSerialEnableInts();
VOID OalUSBSerialDisableInts();
VOID OalUSBSerialPowerOff();
VOID OalUSBSerialPowerOn();
VOID OalUSBSerialFlowControl(BOOL fOn);

#define OAL_USBSER_OMAP35XX    { \
    OalUSBSerialInit, \
    OalUSBSerialDeInit, \
    OalUSBSerialSend, \
    NULL, \
    OalUSBSerialRecv, \
    NULL, \
    NULL, \
    NULL, \
    NULL, \
    NULL, }
    
#if __cplusplus
}
#endif

#endif // __BSP_OALSERDRV_H
