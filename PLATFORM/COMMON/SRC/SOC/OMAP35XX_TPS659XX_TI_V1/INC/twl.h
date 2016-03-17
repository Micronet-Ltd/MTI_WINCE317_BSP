/*
================================================================================
*             Texas Instruments OMAP(TM) Platform Software
* (c) Copyright Texas Instruments, Incorporated. All Rights Reserved.
*
* Use of this software is controlled by the terms and conditions found
* in the license agreement under which this software has been supplied.
*
================================================================================
*/
//
//  File: twl.h
//
#ifndef __TWL_H
#define __TWL_H

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
//
//  Define:  TWL_DEVICE_NAME
//
#define TWL_DEVICE_NAME         L"TWL1:"

//------------------------------------------------------------------------------
//
//  GUID:  DEVICE_IFC_TWL_GUID
//
// {DEF0A04B-B967-43db-959E-D9FC6225CDEB}
DEFINE_GUID(
    DEVICE_IFC_TWL_GUID, 0xdef0a04b, 0xb967, 0x43db, 
    0x95, 0x9e, 0xd9, 0xfc, 0x62, 0x25, 0xcd, 0xeb
    );


//------------------------------------------------------------------------------
//
//  Type:  DEVICE_IFC_TWL
//
//  This structure is used to obtain TWL interface funtion pointers used for
//  in-process calls via IOCTL_DDK_GET_DRIVER_IFC.
//
typedef struct {
    DWORD context;
    BOOL (*pfnReadRegs)(DWORD ctx, DWORD address, VOID *pBuffer, DWORD size);
    BOOL (*pfnWriteRegs)(DWORD ctx, DWORD address, const void *pBuffer, DWORD size);
    BOOL (*pfnInterruptInitialize)(DWORD ctx, DWORD intrId, HANDLE hEvent);
    BOOL (*pfnInterruptDisable)(DWORD ctx, DWORD intrId);
    BOOL (*pfnInterruptMask)(DWORD ctx, DWORD intrId, BOOL bEnable);
    BOOL (*pfnEnableWakeup)(DWORD ctx, DWORD intrId, BOOL bEnable);
} DEVICE_IFC_TWL;

//------------------------------------------------------------------------------
//
//  Type:  DEVICE_CONTEXT_TWL
//
//  This structure is used to store TWL device context
//
typedef struct {
    DEVICE_IFC_TWL ifc;
    HANDLE hDevice;
    DWORD seekAddress;
} DEVICE_CONTEXT_TWL;

//------------------------------------------------------------------------------

#define IOCTL_TWL_READREGS          \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0300, METHOD_BUFFERED, FILE_ANY_ACCESS)

typedef struct {
    DWORD address;
    DWORD size;
} IOCTL_TWL_READREGS_IN;

#define IOCTL_TWL_WRITEREGS         \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0301, METHOD_BUFFERED, FILE_ANY_ACCESS)

typedef struct {
    DWORD address;
    const void* pBuffer;
    DWORD size;
} IOCTL_TWL_WRITEREGS_IN;

#define IOCTL_TWL_SETINTREVENT      \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0302, METHOD_BUFFERED, FILE_ANY_ACCESS)

typedef struct {
    UINT   procId;
    DWORD  intrId;
    HANDLE hEvent;
} IOCTL_TWL_INTRINIT_IN;

typedef struct {
    UINT   procId;
    DWORD  intrId;
    BOOL   bEnable;
} IOCTL_TWL_INTRMASK_IN;

#define IOCTL_TWL_INTRINIT         \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0303, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_TWL_INTRDISABLE       \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0304, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_TWL_INTRMASK       \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0305, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_TWL_WAKEENABLE        \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0306, METHOD_BUFFERED, FILE_ANY_ACCESS)


typedef struct {
    DWORD intrId;
    BOOL bEnable;
} IOCTL_TWL_WAKEENABLE_IN;


//------------------------------------------------------------------------------
//
//  Functions: TWLxxx
//
__inline
HANDLE 
TWLOpen(
    )
{
    HANDLE hDevice;
    DEVICE_CONTEXT_TWL *pContext = NULL;

    hDevice = CreateFile(TWL_DEVICE_NAME, 0, 0, NULL, 0, 0, NULL);
    if (hDevice == INVALID_HANDLE_VALUE) goto cleanUp;

    // Allocate memory for our handler...
    pContext = (DEVICE_CONTEXT_TWL*)LocalAlloc(LPTR, sizeof(DEVICE_CONTEXT_TWL));
    if (pContext == NULL)
        {
        CloseHandle(hDevice);
        goto cleanUp;
        }

    memset(pContext, 0, sizeof(DEVICE_CONTEXT_TWL));

    // Get function pointers, fail when IOCTL isn't supported...
    DeviceIoControl(hDevice, IOCTL_DDK_GET_DRIVER_IFC, 
        (VOID*)&DEVICE_IFC_TWL_GUID, sizeof(DEVICE_IFC_TWL_GUID), 
        &pContext->ifc, sizeof(DEVICE_IFC_TWL), NULL, NULL
        );

    // Save device handle
    pContext->hDevice = hDevice;

cleanUp:
    return pContext;
}

__inline
VOID
TWLClose(
    HANDLE hContext
    )
{
    DEVICE_CONTEXT_TWL *pContext = (DEVICE_CONTEXT_TWL*)hContext;
    CloseHandle(pContext->hDevice);
    LocalFree(pContext);
}

__inline
BOOL
TWLReadRegs(
    HANDLE hContext, 
    DWORD address,
    VOID *pBuffer,
    DWORD size
    )
{
    DEVICE_CONTEXT_TWL *pContext = (DEVICE_CONTEXT_TWL*)hContext;
    if (pContext->ifc.pfnReadRegs != NULL)
        {
        return pContext->ifc.pfnReadRegs(
            pContext->ifc.context, address, pBuffer, size
            );
        }
    else
        {
        DWORD dwCount = 0;
        if (pContext->seekAddress != address)
            {
            SetFilePointer(pContext->hDevice, address, NULL, FILE_CURRENT);
            pContext->seekAddress = address;
            }
        ReadFile(pContext->hDevice, pBuffer, size, &dwCount, NULL);
        return dwCount;
        }
}
    
__inline
BOOL 
TWLWriteRegs(
    HANDLE hContext, 
    DWORD address,
    const VOID *pBuffer,
    DWORD size
    )
{
    DEVICE_CONTEXT_TWL *pContext = (DEVICE_CONTEXT_TWL*)hContext;
    if (pContext->ifc.pfnWriteRegs != NULL)
        {
        return pContext->ifc.pfnWriteRegs(
            pContext->ifc.context, address, pBuffer, size
            );
        }
    else
        {
        DWORD dwCount = 0;
        if (pContext->seekAddress != address)
            {
            SetFilePointer(pContext->hDevice, address, NULL, FILE_CURRENT);
            pContext->seekAddress = address;
            }
        return WriteFile(pContext->hDevice, pBuffer, size, &dwCount, NULL);
        }
}

__inline
BOOL 
TWLInterruptInitialize(
    HANDLE hContext, 
    DWORD intrId,
    HANDLE hEvent
    )
{
    IOCTL_TWL_INTRINIT_IN inParam;
    DEVICE_CONTEXT_TWL *pContext = (DEVICE_CONTEXT_TWL*)hContext;

    if (pContext->ifc.pfnInterruptInitialize != NULL)
        {
        return pContext->ifc.pfnInterruptInitialize(
            pContext->ifc.context, intrId, hEvent
            );
        }
    else
        {
        inParam.intrId = intrId;
        inParam.hEvent = hEvent;
        inParam.procId = GetCurrentProcessId();
        return DeviceIoControl(pContext->hDevice,
                               IOCTL_TWL_INTRINIT,
                               &inParam,
                               sizeof(inParam),
                               NULL,
                               0,
                               NULL,
                               NULL 
                               );
        }
}

__inline
BOOL 
TWLInterruptMask(
    HANDLE hContext, 
    DWORD intrId,
    BOOL  bEnable
    )
{
    DEVICE_CONTEXT_TWL *pContext = (DEVICE_CONTEXT_TWL*)hContext;

    if (pContext->ifc.pfnInterruptMask != NULL)
        {
        return pContext->ifc.pfnInterruptMask(
            pContext->ifc.context, intrId, bEnable
            );
        }
    else
        {
        IOCTL_TWL_INTRMASK_IN inParam = {intrId, bEnable};

        return DeviceIoControl(pContext->hDevice,
                               IOCTL_TWL_INTRMASK,
                               &inParam,
                               sizeof(inParam),
                               NULL,
                               0,
                               NULL,
                               NULL 
                               );
        }
}

__inline
BOOL 
TWLInterruptDisable(
    HANDLE hContext, 
    DWORD intrId
    )
{
    DEVICE_CONTEXT_TWL *pContext = (DEVICE_CONTEXT_TWL*)hContext;

    if (pContext->ifc.pfnInterruptDisable != NULL)
        {
        return pContext->ifc.pfnInterruptDisable(
            pContext->ifc.context, intrId
            );
        }
    else
        {
        return DeviceIoControl(pContext->hDevice,
                               IOCTL_TWL_INTRDISABLE,
                               &intrId,
                               sizeof(DWORD),
                               NULL,
                               0,
                               NULL,
                               NULL 
                               );
        }
}

__inline
BOOL 
TWLWakeEnable(
    HANDLE hContext, 
    DWORD intrId,
    BOOL bEnable
    )
{
    IOCTL_TWL_WAKEENABLE_IN inParam;
    DEVICE_CONTEXT_TWL *pContext = (DEVICE_CONTEXT_TWL*)hContext;

    if (pContext->ifc.pfnEnableWakeup != NULL)
        {
        return pContext->ifc.pfnEnableWakeup(
            pContext->ifc.context, intrId, bEnable
            );
        }
    else
        {
        inParam.intrId = intrId;
        inParam.bEnable = bEnable;
        return DeviceIoControl(pContext->hDevice,
                               IOCTL_TWL_WAKEENABLE,
                               &inParam,
                               sizeof(inParam),
                               NULL,
                               0,
                               NULL,
                               NULL 
                               );
        }
}


//------------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif
