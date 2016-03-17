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
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Module Name:
//    globals.hpp
//
// Abstract:  This header contains UHCD data that all classes
//            should have access to
//
// Notes:
//

#ifndef _UHCD_GLOBALS_HPP_
#define _UHCD_GLOBALS_HPP_

// There are four warnings that I like from Warning level 4.  Since we build
// at warning level 3, I'm setting these four down to level 3 so I still get
// them.
// C4100 unrefrenced formal parameter
// C4101 unrefrenced local variable
// C4705 statement has no effect
// C4706 assignment in conditional
#pragma warning (3 : 4100 4101 4705 4706)

#include <windows.h>
#include <ceddk.h>
#include <hcdi.h>
#include "uhcdddsi.h"


#define IN
#define OUT
#define IN_OUT

typedef volatile PUCHAR REGISTER;

enum HCD_REQUEST_STATUS {
    requestFailed = 0,
    requestOK,
    requestIgnored
};

#ifdef DEBUG
    // in debug mode, structures are inited and sometimes written with
    // garbage during execution. This "GARBAGE" can be set to anything
    // and should not affect program execution. Changing this value once
    // in a while may find hidden bugs (i.e. if code is accessing items
    // that it shouldn't be...)
    #define GARBAGE int(0xAA)

    #define DEBUG_ONLY(x) x
    #define DEBUG_PARAM(x) x,
#else
    #define DEBUG_ONLY(x)
    #define DEBUG_PARAM(x)
#endif

#define USBPAGESIZE DWORD(4096)    // Spec'ed to be 4kB
#define USBPAGEMASK DWORD(USBPAGESIZE - 1)

// USB addresses are 7 bits long, hence between 0 and 127
#define USB_MAX_ADDRESS                     UCHAR(127)

// minimum packet size of control pipe to endpoint 0
#define ENDPOINT_ZERO_MIN_MAXPACKET_SIZE USHORT(8)

//
// USBD function typedefs
//

typedef BOOL (* LPUSBD_HCD_ATTACH_PROC)(LPVOID lpvHcd, LPCHCD_FUNCS lpHcdFuncs,
                                        LPLPVOID lppvContext);
typedef BOOL (* LPUSBD_HCD_DETACH_PROC)(LPVOID lpvContext);
typedef BOOL (* LPUSBD_ATTACH_PROC)(LPVOID lpvContext, UINT address,
                                    UINT iEndpointZero,
                                    LPCUSB_DEVICE lpDeviceInfo,
                                    LPLPVOID lppvDeviceDetach);
typedef BOOL (* LPUSBD_DETACH_PROC)(LPVOID lpvDeviceDetach);
typedef BOOL (* LPUSBD_SELECT_CONFIGURATION_PROC)(LPCUSB_DEVICE lpDeviceInfo, LPBYTE );
typedef BOOL (* LPUSBD_SUSPEND_RESUME_PROC)(LPVOID lpvDeviceDetach, BOOL fResumed);

//
// These structures MUST have exactly the same
// data types as the USB_* structues in usbtypes.h
//

typedef struct _NON_CONST_USB_ENDPOINT {
    DWORD                                   dwCount;

    USB_ENDPOINT_DESCRIPTOR                 Descriptor;
    LPBYTE                                  lpbExtended;
    DWORD                                   dwExtendedSize;
} NON_CONST_USB_ENDPOINT, * LPNON_CONST_USB_ENDPOINT;

typedef struct _NON_CONST_USB_INTERFACE {
    DWORD                                   dwCount;

    USB_INTERFACE_DESCRIPTOR                Descriptor;
    LPBYTE                                  lpbExtended;
    LPNON_CONST_USB_ENDPOINT                lpEndpoints;
    DWORD                                   dwExtendedSize;
} NON_CONST_USB_INTERFACE, * LPNON_CONST_USB_INTERFACE;

typedef struct _NON_CONST_USB_CONFIGURATION {
    DWORD                                   dwCount;

    USB_CONFIGURATION_DESCRIPTOR            Descriptor;
    LPBYTE                                  lpbExtended;
    // Total number of interfaces (including alternates)
    DWORD                                   dwNumInterfaces;
    LPNON_CONST_USB_INTERFACE               lpInterfaces;
    DWORD                                   dwExtendedSize;
} NON_CONST_USB_CONFIGURATION, * LPNON_CONST_USB_CONFIGURATION;

typedef struct _USB_DEVICE_INFO {
    DWORD                                   dwCount;

    USB_DEVICE_DESCRIPTOR                   Descriptor;
    LPNON_CONST_USB_CONFIGURATION           lpConfigs;
    LPNON_CONST_USB_CONFIGURATION           lpActiveConfig;
} USB_DEVICE_INFO, * LPUSB_DEVICE_INFO;


//
// USB Hub definitions
//
#define UHCD_NUM_ROOT_HUB_PORTS             UCHAR(2)

// There can be at most 5 hubs linked in a row, according to USB spec
// 1.1, section 7.1.19 (Figure 7-31)
#define USB_MAXIMUM_HUB_TIER UCHAR(5)
//
// USB spec(1.1) 11.15.2.1 - hub descriptor type is 29h
#define USB_HUB_DESCRIPTOR_TYPE               UCHAR(0x29)
// If the hub has <= 7 ports, the descriptor will be
// 9 bytes long. Else, it will be larger.
#define USB_HUB_DESCRIPTOR_MINIMUM_SIZE     UCHAR(0x9)

// device set and clear feature recipients
#define USB_DEVICE_RECIPIENT                UCHAR(0x00)
#define USB_INTERFACE_RECIPIENT             UCHAR(0x01)
#define USB_ENDPOINT_RECIPIENT              UCHAR(0x02)

// Standard device feature selectors
#define USB_DEVICE_REMOTE_WAKEUP            UCHAR(0x01)
#define USB_FEATURE_ENDPOINT_HALT           UCHAR(0x00)

// USB spec(1.1) Table 11-12 : Hub Class Feature Selectors
#define USB_HUB_FEATURE_C_HUB_LOCAL_POWER   UCHAR(0x00)
#define USB_HUB_FEATURE_C_HUB_OVER_CURRENT  UCHAR(0x01)
#define USB_HUB_FEATURE_PORT_CONNECTION     UCHAR(0x00)
#define USB_HUB_FEATURE_PORT_ENABLE         UCHAR(0x01)
#define USB_HUB_FEATURE_PORT_SUSPEND        UCHAR(0x02)
#define USB_HUB_FEATURE_PORT_OVER_CURRENT   UCHAR(0x03)
#define USB_HUB_FEATURE_PORT_RESET          UCHAR(0x04)
// 0x5 - 0x7 are reserved
#define USB_HUB_FEATURE_PORT_POWER            UCHAR(0x08)
#define USB_HUB_FEATURE_PORT_LOW_SPEED      UCHAR(0x09)
// 0xA - 0xF are reserved
#define USB_HUB_FEATURE_C_PORT_CONNECTION   UCHAR(USB_HUB_FEATURE_PORT_CONNECTION | 0x10)
#define USB_HUB_FEATURE_C_PORT_ENABLE         UCHAR(USB_HUB_FEATURE_PORT_ENABLE | 0x10)
#define USB_HUB_FEATURE_C_PORT_SUSPEND    UCHAR(USB_HUB_FEATURE_PORT_SUSPEND | 0x10)
#define USB_HUB_FEATURE_C_PORT_OVER_CURRENT UCHAR(USB_HUB_FEATURE_PORT_OVER_CURRENT | 0x10)
#define USB_HUB_FEATURE_C_PORT_RESET          UCHAR(USB_HUB_FEATURE_PORT_RESET | 0x10)

#define USB_HUB_FEATURE_PORT_INDICATOR      UCHAR(0x16) /*22*/

#define USB_HUB_FEATURE_INVALID             UCHAR(0xFF)

// bits for hub descriptor wHubCharacteristics field
// see USB spec (1.1) Table 11-8, offset 3
#define USB_HUB_CHARACTERISTIC_GANGED_POWER_SWITCHING           USHORT(0x0)
#define USB_HUB_CHARACTERISTIC_INDIVIDUAL_POWER_SWITCHING       USHORT(0x1)
#define USB_HUB_CHARACTERISTIC_NO_POWER_SWITCHING               USHORT(0x2) // warning can be 0x3
#define USB_HUB_CHARACTERISTIC_NOT_PART_OF_COMPOUND_DEVICE      USHORT(0 << 2)
#define USB_HUB_CHARACTERISTIC_PART_OF_COMPOUND_DEVICE          USHORT(1 << 2)
#define USB_HUB_CHARACTERISTIC_GLOBAL_OVER_CURRENT_PROTECTION   USHORT(0x0 << 3)
#define USB_HUB_CHARACTERISTIC_INDIVIDUAL_OVER_CURRENT_PROTECTION USHORT(0x1 << 3)
#define USB_HUB_CHARACTERISTIC_NO_OVER_CURRENT_PROTECTION       USHORT(0x2 << 3) // warning, can be (0x3 << 3)

// Data returned by GetHubStatus request is 4 bytes
// and structures as follows: (USB spec 1.1 - Table 11-13/14)
//
// Data returned by GetPortStatus request is 4 bytes
// and structured as follows: (USB spec 1.1 - Table 11-15/16)
typedef struct _USB_HUB_AND_PORT_STATUS
{
    union {
        struct {
            USHORT LocalPowerStatus:1;             // wHubStatus bit 0
            USHORT OverCurrentIndicator:1;         // wHubStatus bit 1
            USHORT Reserved:14;                    // wHubStatus bits 2-15
        }          hub;
        struct {
            USHORT PortConnected:1;                // wPortStatus bit 0
            USHORT PortEnabled:1;                  // wPortStatus bit 1
            USHORT PortSuspended:1;                // wPortStatus bit 2
            USHORT PortOverCurrent:1;              // wPortStatus bit 3
            USHORT PortReset:1;                    // wPortStatus bit 4
            USHORT Reserved:3;                     // wPortStatus bits 5-7
            USHORT PortPower:1;                    // wPortStatus bit 8
            USHORT DeviceIsLowSpeed:1;             // wPortStatus bit 9
            USHORT DeviceIsHighSpeed:1;
            USHORT PortTestMode:1;
            USHORT PortIndicatorControl:1;
            USHORT Reserved2:3;                    // wPortStatus bits 10-15
        }          port;
        USHORT     word;
    }                                              status;
    union {
        struct {
            USHORT LocalPowerChange:1;             // wHubChange bit 0
            USHORT OverCurrentIndicatorChange:1;   // wHubChange bit 1
            USHORT Reserved2:14;                   // wHubChange bits 2-15
        }          hub;
        struct {
            USHORT ConnectStatusChange:1;          // wPortChange bit 0
            USHORT PortEnableChange:1;             // wPortChange bit 1
            USHORT SuspendChange:1;                // wPortChange bit 2
            USHORT OverCurrentChange:1;            // wPortChange bit 3
            USHORT ResetChange:1;                  // wPortChange bit 4
            USHORT Reserved:11;                    // wPortChange bits 5-15
        }          port;
        USHORT     word;
    }                                              change;
} USB_HUB_AND_PORT_STATUS, *PUSB_HUB_AND_PORT_STATUS;

// this is the minimum packet size which can be safely scheduled
// for bandwidth reclamation
#define UHCD_MAX_RECLAMATION_PACKET_SIZE    USHORT(64)


#ifdef __cplusplus
extern "C" DWORD g_IstThreadPriority;
#endif

#define DEFAULT_UHCD_IST_PRIORITY 101   // UsbInterruptThread
#define RELATIVE_PRIO_ADJUST_FRAME 7    // UsbAdjustFrameLengthThread (low pri!)
#define RELATIVE_PRIO_STSCHG       5    // HubStatusChangeThread
#define RELATIVE_PRIO_DOWNSTREAM   3    // DetachDownstreamDeviceThread
#define RELATIVE_PRIO_CHECKDONE    (-1) // CheckForDoneTransfersThread

#endif //_UHCD_GLOBALS_HPP_

