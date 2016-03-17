// All rights reserved ADENEO EMBEDDED 2010
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
// Portions Copyright (c) Texas Instruments.  All rights reserved.
//
//  This is to be exported to SDK
//------------------------------------------------------------------------------
#ifndef MUSBIOCTL_H
#define MUSBIOCTL_H

#ifdef __cplusplus
extern "C" {
#endif

#define IOCTL_USBOTG_REQUEST_SESSION_ENABLE \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0800, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_USBOTG_REQUEST_SESSION_DISABLE \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0801, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_USBOTG_REQUEST_SESSION_QUERY \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0802, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_USBOTG_DUMP_ULPI_REG  \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0803, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_USBOTG_ULPI_SET_LOW_POWER_MODE  \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0804, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_USBOTG_REQUEST_HOST_MODE \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0805, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_USBOTG_SUSPEND_HOST \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0806, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_USBOTG_RESUME_HOST \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0807, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_USBOTG_REQUEST_START \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0808, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_USBOTG_REQUEST_STOP   \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0809, METHOD_BUFFERED, FILE_ANY_ACCESS)

#ifdef __cplusplus
}
#endif

#endif
