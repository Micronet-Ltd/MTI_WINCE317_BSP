//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this sample source code is subject to the terms of the Microsoft
// license agreement under which you licensed this sample source code. If
// you did not accept the terms of the license agreement, you are not
// authorized to use this sample source code. For the terms of the license,
// please see the license agreement between you and Microsoft or, if applicable,
// see the LICENSE.RTF on your install media or the root of your tools installation.
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES OR INDEMNITIES.
//

#ifndef _WAVERTGCTRL_H_
#define _WAVERTGCTRL_H_

#if (_MSC_VER >= 1000)
#pragma once
#endif

#include <basetyps.h>

#ifdef __cplusplus
extern "C" {
#endif

//
// Device topology.
//

// {9FEFFBD7-E178-4db3-81A3-1262347C96B3}
DEFINE_GUID(MM_PROPSET_DEVTOPOLOGY, 
    0x9feffbd7, 0xe178, 0x4db3, 0x81, 0xa3, 0x12, 0x62, 0x34, 0x7c, 0x96, 0xb3);

// Prop get only - Returns the device descriptor.
//      pvPropData - Pointer to DTP_DEVICE_DESCRIPTOR.
#define MM_PROP_DEVTOPOLOGY_DEVICE_DESCRIPTOR       1

// Prop get only - Returns the virutal endpoint descriptor.
//      pvPropParams - Pointer to DWORD for the endpoint index.
//      pvPropData - Pointer to DTP_ENDPOINT_DESCRIPTOR.
#define MM_PROP_DEVTOPOLOGY_ENDPOINT_DESCRIPTOR    2

// Prop set only - Registers a message queue to receive device topology messages.
//      pvPropData - Pointer to message queue handle returned from CreateMsgQueue.
#define MM_PROP_DEVTOPOLOGY_EVTMSG_REGISTER         3

// Prop set only - Unregisters a message queue from receiving device topology messages.
//      pvPropData - Pointer to a message queue handle previously registered.
#define MM_PROP_DEVTOPOLOGY_EVTMSG_UNREGISTER       4

// Device descriptor.
typedef struct _DTP_DEVICE_DESCRIPTOR
{
    GUID guidDevClass;                      // Device class GUID.
    DWORD cEndpoints;                       // Number of endpoints on the device.
} DTP_DEVICE_DESCRIPTOR, *PDTP_DEVICE_DESCRIPTOR;

// Endpoint descriptor.
typedef struct _DTP_ENDPOINT_DESCRIPTOR
{
    DWORD dwIndex;                          // Endpoint index starting at zero.
    GUID guidEndpoint;                      // Endpoint GUID.
    BOOL fRemovable;                        // Endpoint is removable.
    DWORD fAttached;                        // Endpoint attached state.
} DTP_ENDPOINT_DESCRIPTOR, *PDTP_ENDPOINT_DESCRIPTOR;

// Device topology event types.
#define DTP_EVT_ENDPOINT_CHANGE         0x00000001  // Endpoint change event.

// Device topology message header.  This structure is located at the 
// beginning of all device topology messages.
typedef struct _DTP_EVTMSG
{
    DWORD dwEvtType;                        // Event type.
    DWORD cbMsg;                            // Size in bytes of the containing message structure.
} DTP_EVTMSG, *PDTP_EVTMSG;

// Endpoint change message.
typedef struct _DTP_EVTMSG_ENDPOINT_CHANGE
{
    DTP_EVTMSG hdr;
    DWORD dwIndex;                          // Index of the endpoint that changed.
    BOOL fAttached;                         // Endpoint is attached?
} DTP_EVTMSG_ENDPOINT_CHANGE, *PDTP_EVTMSG_ENDPOINT_CHANGE;

//
// Routing control.
//

// {C19F94C4-8448-4c9b-997C-4EAE0B25500C}
DEFINE_GUID(MM_PROPSET_RTGCTRL, 
    0xc19f94c4, 0x8448, 0x4c9b, 0x99, 0x7c, 0x4e, 0xae, 0xb, 0x25, 0x50, 0xc);

// Prop set only - Request for system audio routing.
//      pvPropData - Pointer to RTGCTRL_ENDPOINT_SELECT.
#define MM_PROP_RTGCTRL_ENDPOINT_SYSTEM    1

// Prop set only - Request for cellular audio routing.
//      pvPropData - Pointer to RTGCTRL_CELLULAR_AUDIO.
#define MM_PROP_RTGCTRL_ENDPOINT_CELLULAR  2

// Maximum number of simultaneous selectable endpoints.
#define ENDPOINT_SELECT_MAX                4

// Selects the endpoints for audio routing.
typedef struct _RTGCTRL_ENDPOINT_SELECT
{
    DWORD cEndpoints;                       // Number of endpoints.
    DWORD rgdwIndex[ENDPOINT_SELECT_MAX];   // Index of endpoints to use for routing.
} RTGCTRL_ENDPOINT_SELECT, *PRTGCTRL_ENDPOINT_SELECT;

// Cellular audio routing.
typedef struct _RTGCTRL_CELLULAR_AUDIO
{
    BOOL fEnable;                           // Enable cellular audio routing.
    RTGCTRL_ENDPOINT_SELECT EndpointSelect; // Endpoints to use for cellular audio routing.
} RTGCTRL_CELLULAR_AUDIO, *PRTGCTRL_CELLULAR_AUDIO;

// Endpoint flags.
#define EPFLAGS_CELLULAR_RX             0x00000001  // Endpoint can be used for cellular RX.
#define EPFLAGS_CELLULAR_TX             0x00000002  // Endpoint can be used for cellular TX.
#define EPFLAGS_CELLULAR                (EPFLAGS_CELLULAR_RX | EPFLAGS_CELLULAR_TX)
#define EPFLAGS_GENVOICECOMM_RX         0x00000004  // Endpoint can be used for general non-cellular voice comm RX.
#define EPFLAGS_GENVOICECOMM_TX         0x00000008  // Endpoint can be used for general non-cellular voice comm TX.
#define EPFLAGS_GENVOICECOMM            (EPFLAGS_GENVOICECOMM_RX | EPFLAGS_GENVOICECOMM_TX)
#define EPFLAGS_NOAUTOENABLE            0x00008000  // Endpoint should not be auto-enabled when attached.
#define EPFLAGS_MASK                    0x0000FFFF

#ifdef __cplusplus
}
#endif

#endif // _WAVERTGCTRL_H_
