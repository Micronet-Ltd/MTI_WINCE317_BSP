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
#ifndef __SDMAPROXY_H__
#define __SDMAPROXY_H__

//
// Pound Defines for I/O controls
// To be used by Applications
//
#define DEVICE_SDMA_PROXY                   0x40230
#define DEVICE_SDMA_PROXY_CONFIG_FUNCTION   0x2048
#define DEVICE_SDMA_PROXY_COPY_FUNCTION     0x2049

#define IOCTL_SDMAPROXY_CONFIG \
    CTL_CODE(DEVICE_SDMA_PROXY, DEVICE_SDMA_PROXY_CONFIG_FUNCTION, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_SDMAPROXY_COPY \
    CTL_CODE(DEVICE_SDMA_PROXY, DEVICE_SDMA_PROXY_COPY_FUNCTION, METHOD_BUFFERED, FILE_ANY_ACCESS)


//
// SDMA Config Struct
// To be used by Applications
//
typedef struct SdmaConfig_t {
    DWORD elementWidth; // 8, 16, 32 bit mode
    DWORD syncMode;     // element, frame or block
    DWORD elementCount;
    DWORD frameCount;
    DWORD srcElementIdx;
    DWORD srcFrameIdx;
    DWORD srcAddrMode;
    DWORD dstElementIdx;
    DWORD dstFrameIdx;
    DWORD dstAddrMode;
} SdmaConfig_t;

#endif /* sdmaproxy.h */
