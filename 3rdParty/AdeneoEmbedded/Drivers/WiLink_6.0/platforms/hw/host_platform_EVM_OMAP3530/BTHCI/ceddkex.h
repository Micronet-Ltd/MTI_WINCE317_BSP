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
//------------------------------------------------------------------------------
//
//  File:  ceddkex.h
//
//  This file contains OMAP2430 specific ceddk extensions.
//
#ifndef __CEDDKEX_H
#define __CEDDKEX_H

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------

#define dimof(x)            (sizeof(x)/sizeof(x[0]))
#define offset(s, f)        FIELD_OFFSET(s, f)
#define fieldsize(s, f)     sizeof(((s*)0)->f)

//------------------------------------------------------------------------------
//
//  Define:  IN/OUT/SET/CLRREG
//
#ifndef __OAL_IO_H
#define INREG8(x)           READ_REGISTER_UCHAR((UCHAR*)(x))
#define OUTREG8(x, y)       WRITE_REGISTER_UCHAR((UCHAR*)(x), (UCHAR)(y))
#define SETREG8(x, y)       OUTREG8(x, INREG8(x)|(y))
#define CLRREG8(x, y)       OUTREG8(x, INREG8(x)&~(y))
#define MASKREG8(x, y, z)   OUTREG8(x, (INREG8(x)&~(y))|(z))

#define INREG16(x)          READ_REGISTER_USHORT((USHORT*)(x))
#define OUTREG16(x, y)      WRITE_REGISTER_USHORT((USHORT*)(x),(USHORT)(y))
#define SETREG16(x, y)      OUTREG16(x, INREG16(x)|(y))
#define CLRREG16(x, y)      OUTREG16(x, INREG16(x)&~(y))
#define MASKREG16(x, y, z)  OUTREG16(x, (INREG16(x)&~(y))|(z))

#define INREG32(x)          READ_REGISTER_ULONG((ULONG*)(x))
#define OUTREG32(x, y)      WRITE_REGISTER_ULONG((ULONG*)(x), (ULONG)(y))
#define SETREG32(x, y)      OUTREG32(x, INREG32(x)|(y))
#define CLRREG32(x, y)      OUTREG32(x, INREG32(x)&~(y))
#define MASKREG32(x, y, z)  OUTREG32(x, (INREG32(x)&~(y))|(z))
#endif

//------------------------------------------------------------------------------
//
//  Define:  IOCTL_DDK_GET_DRIVER_IFC
//
//  This IOCTL code is used to obtain device driver interface pointers used for
//  in-process calls between drivers. The input driver can contain interface
//  GUID. The driver should fill output buffer with interface structure based
//  on interface GUID. When input buffer is empty driver should return base
//  interface specified by CEDDK_DRIVER_PFN (see bellow).
//
#define IOCTL_DDK_GET_DRIVER_IFC        \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0100, METHOD_BUFFERED, FILE_ANY_ACCESS)

//------------------------------------------------------------------------------
//
//  Type:  CEDDK_DRIVER_PFN
//
//  This structure is used to obtain device driver funtion pointers used for
//  in-process calls via IOCTL_DDK_GET_DRIVER_PFN.
//
typedef struct {
    DWORD context;
    DWORD (*pfnRead)(DWORD, VOID*, DWORD);
    DWORD (*pfnWrite)(DWORD, VOID*, DWORD);
    BOOL  (*pfnIOControl)(DWORD, DWORD, UCHAR*, DWORD, UCHAR*, DWORD, DWORD*);
} CEDDK_DRIVER_PFN;

//------------------------------------------------------------------------------
//
//  Type:  CEDDK_DRIVER_CONTEXT
//
//  This structure is used internaly to store CEDDK_DRIVER_PFN and other
//  context needed for operation implementation.
//
typedef struct {
    CEDDK_DRIVER_PFN ifc;
    HANDLE hDevice;
} CEDDK_DRIVER_CONTEXT;

//------------------------------------------------------------------------------
//
//  Function:  CreateDevice
//
//  This function is similar to CreateFile function. It will call CreateFile
//  and DeviceIoControl with code IOCTL_DDK_GET_DRIVER_PFN. If both calls 
//  succeed function will create internal structure and it returns its pointer
//  as handler.
//
__inline HANDLE CreateDevice(
    LPCTSTR deviceName, DWORD desiredAccess, DWORD shareMode, 
    LPSECURITY_ATTRIBUTES pSecurityAttributes, DWORD creationDisposition, 
    DWORD flagsAndAttributes, HANDLE hTemplateFile
) {
    HANDLE hDevice;
    CEDDK_DRIVER_CONTEXT *pContext = NULL;

    if ((hDevice = CreateFile(
        deviceName, desiredAccess, shareMode, pSecurityAttributes, 
        creationDisposition, flagsAndAttributes, hTemplateFile
    )) == INVALID_HANDLE_VALUE) goto cleanUp;

    // Allocate memory for our handler...
    if ((pContext = (CEDDK_DRIVER_CONTEXT*)LocalAlloc(
        LPTR, sizeof(CEDDK_DRIVER_CONTEXT)
    )) == NULL) {
        CloseHandle(hDevice);
        goto cleanUp;
    }

    // Get function pointers, fail when IOCTL isn't supported...
    // Get function pointers, fail when IOCTL isn't supported...
    if (!DeviceIoControl(
        hDevice, IOCTL_DDK_GET_DRIVER_IFC, NULL, 0, &pContext->ifc, 
        sizeof(CEDDK_DRIVER_PFN), NULL, NULL
    )) {
        CloseHandle(hDevice);
        LocalFree(pContext);
        pContext = NULL;
        goto cleanUp;
    }

    // Save device handle
    pContext->hDevice = hDevice;

cleanUp:
    return (pContext != NULL)?(HANDLE)pContext:INVALID_HANDLE_VALUE;
}

//------------------------------------------------------------------------------
//
//  Function:  CloseDevice
//
//  This function will call CloseHandle and deallocate internal structure.
//
__inline BOOL CloseDevice(HANDLE hDevice)
{
    BOOL rc;
    CEDDK_DRIVER_CONTEXT *pContext = (CEDDK_DRIVER_CONTEXT*)hDevice;

    rc = CloseHandle(pContext->hDevice);
    LocalFree(pContext);
    return rc;
}

//------------------------------------------------------------------------------
//
//  Function:  WriteDevice
//
//  This function will call directly XXX_Write function based on pointer and
//  context obtained by CreateDevice.
//
__inline BOOL WriteDevice(
    HANDLE hDevice, LPCVOID pBuffer, DWORD bytesToWrite, LPDWORD pBytesWritten, 
    LPOVERLAPPED pOverlapped
) {
    CEDDK_DRIVER_CONTEXT *pContext = (CEDDK_DRIVER_CONTEXT*)hDevice;
    DWORD rc;

    rc = pContext->ifc.pfnWrite(
        pContext->ifc.context, (VOID*)pBuffer, bytesToWrite
    );
    if (pBytesWritten != NULL) *pBytesWritten = rc;
    return (rc == bytesToWrite);
}

//------------------------------------------------------------------------------
//
//  Function:  ReadDevice
//
//  This function will call directly XXX_Read function based on pointer and
//  context obtained by CreateDevice.
//
__inline BOOL ReadDevice( 
    HANDLE hDevice, LPVOID pBuffer, DWORD bytesToRead, LPDWORD pBytesRead, 
    LPOVERLAPPED pOverlapped
) {
    CEDDK_DRIVER_CONTEXT *pContext = (CEDDK_DRIVER_CONTEXT*)hDevice;
    DWORD rc;

    rc = pContext->ifc.pfnRead(pContext->ifc.context, pBuffer, bytesToRead);
    if (pBytesRead != NULL) *pBytesRead = rc;
    return (rc > 0);
}

//------------------------------------------------------------------------------
//
//  Function:  DeviceIoCtrl
//
//  This function will call directly XXX_IOControl function based on pointer
//  and context obtained by CreateDevice.
//
__inline BOOL DeviceIoCtrl(
  HANDLE hDevice, DWORD code, LPVOID pInBuffer, DWORD inSize, LPVOID pOutBuffer,
  DWORD outSize, DWORD *pOutSize, LPOVERLAPPED pOverlapped
) {
    CEDDK_DRIVER_CONTEXT *pContext = (CEDDK_DRIVER_CONTEXT*)hDevice;

    return pContext->ifc.pfnIOControl(
        pContext->ifc.context, code, (UCHAR*)pInBuffer, inSize, 
        (UCHAR*)pOutBuffer, outSize, pOutSize
    );        
}

//------------------------------------------------------------------------------
//
//  Function:  ReadRegistryParams
//
//  This function initializes driver default settings from registry based on
//  table passed as argument.
//
#define PARAM_DWORD             1
#define PARAM_STRING            2
#define PARAM_MULTIDWORD        3
#define PARAM_BIN               4

typedef struct {
    LPTSTR name;
    DWORD  type;
    BOOL   required;
    DWORD  offset;
    DWORD  size;
    PVOID  pDefault;
} DEVICE_REGISTRY_PARAM;

DWORD GetDeviceRegistryParams(
    LPCWSTR szContext, VOID *pBase, DWORD count, 
    const DEVICE_REGISTRY_PARAM params[]
);

//------------------------------------------------------------------------------
//
//  Define:  IOCTL_DISPLAY_SETDEVICEPOWERSTATE
//
//  This IOCTL code is used by display driver to call root bus driver to 
//  put hardware to given power state (= enable/disable LCD controller clock).
//
#define IOCTL_DISPLAY_SETDEVICEPOWERSTATE       \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0101, METHOD_BUFFERED, FILE_ANY_ACCESS)

//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
//
//  Define:  IOCTL_KERNELI2C_SUBMIT
//
//  This IOCTL code is used by I2C driver to submit an i2c transaction to the
//  kernel for processing.
//
#define IOCTL_KERNELI2C_SUBMIT       \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0102, METHOD_BUFFERED, FILE_ANY_ACCESS)

//------------------------------------------------------------------------------



//------------------------------------------------------------------------------
//
//  Define:  IOCTL_ICLK_ENB,IOCTL_ICLK_DIS
//
//  This IOCTL code is used by drivers to enable and disable the interface clocks
//  in the PRCM register.
//
#define IOCTL_ICLK1_ENB       \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0103, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_ICLK1_DIS       \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0104, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_ICLK2_ENB       \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0105, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_ICLK2_DIS       \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0106, METHOD_BUFFERED, FILE_ANY_ACCESS)

//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
//
//  Define:  IOCTL_FCLK_ENB,IOCTL_FCLK_DIS
//
//  This IOCTL code is used by drivers to enable and disable the functional clocks
//  in the PRCM register.
//
#define IOCTL_FCLK1_ENB       \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0107, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_FCLK1_DIS       \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0108, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_FCLK2_ENB       \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0109, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_FCLK2_DIS       \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x010A, METHOD_BUFFERED, FILE_ANY_ACCESS)

//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
//
//  Define:  IOCTL_AIDLE_ENB,IOCTL_AIDLE_DIS
//
//  This IOCTL code is used by drivers to enable and disable auto-idle for the
//  PRCM register.
//
#define IOCTL_AIDLE1_ENB       \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x010B, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_AIDLE1_DIS       \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x010C, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_AIDLE2_ENB       \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x010D, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_AIDLE2_DIS       \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x010E, METHOD_BUFFERED, FILE_ANY_ACCESS)

//------------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif
