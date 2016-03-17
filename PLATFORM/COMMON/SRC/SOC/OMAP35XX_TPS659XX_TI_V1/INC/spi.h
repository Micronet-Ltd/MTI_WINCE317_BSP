// Copyright (c) 2007, 2008 BSQUARE Corporation. All rights reserved.

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
//  File:  spi.h
//
#ifndef __SPI_H
#define __SPI_H

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
//
//  Define:  SPI_DEVICE_NAME
//
#define SPI1_DEVICE_NAME         L"SPI1:"
#define SPI2_DEVICE_NAME         L"SPI2:"
#define SPI3_DEVICE_NAME         L"SPI3:"
#define SPI4_DEVICE_NAME         L"SPI4:"  // ##### Michael, 17-Feb-2010

//------------------------------------------------------------------------------
//
//  GUID:  DEVICE_IFC_SPI_GUID
//
// {2E559225-C95E-4300-86E9-6A5CBC07328F}
DEFINE_GUID(
    DEVICE_IFC_SPI_GUID, 0x2e559225, 0xc95e, 0x4300, 
    0x86, 0xe9, 0x6a, 0x5c, 0xbc, 0x7, 0x32, 0x8f
    );


//------------------------------------------------------------------------------
//
//  Type:  DEVICE_IFC_SPI
//
//  This structure is used to obtain SPI interface funtion pointers used for
//  in-process calls via IOCTL_DDK_GET_DRIVER_IFC.
//
typedef struct {
    DWORD context;
    BOOL  (*pfnConfigure)(DWORD context, DWORD address, DWORD config);
    DWORD (*pfnRead)(DWORD context, VOID *pBuffer, DWORD size);
    DWORD (*pfnWrite)(DWORD context, VOID *pBuffer, DWORD size);
    DWORD (*pfnWriteRead)(DWORD context, DWORD size, VOID *pInBuffer, VOID *pOutBuffer);
	DWORD (*pfnWriteReadCS)(DWORD context, DWORD inSize, UINT8 *pInBuffer, DWORD outSize, const UINT8 *pOutBuffer);
    BOOL  (*pfnLockController)(DWORD context, DWORD timeout);
    BOOL  (*pfnUnlockController)(DWORD context);
    BOOL  (*pfnEnableChannel)(DWORD context);
    BOOL  (*pfnDisableChannel)(DWORD context);
    DWORD (*pfnAsyncWriteRead)(DWORD context, DWORD size, VOID *pInBuffer, VOID *pOutBuffer);
    DWORD (*pfnWaitForAsyncWriteReadComplete)(DWORD context, DWORD, VOID*);
    DWORD (*pfnSetSlaveMode)(DWORD context);
} DEVICE_IFC_SPI;

//------------------------------------------------------------------------------
//
//  Type:  DEVICE_CONTEXT_SPI
//
//  This structure is used to store SPI device context.
//
typedef struct {
    DEVICE_IFC_SPI ifc;
    HANDLE hDevice;
} DEVICE_CONTEXT_SPI;

//------------------------------------------------------------------------------

#define IOCTL_SPI_CONFIGURE     \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0200, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_SPI_WRITEREAD     \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0201, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_SPI_ASYNC_WRITEREAD     \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0202, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_SPI_ASYNC_WRITEREAD_COMPLETE     \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0203, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_SPI_SET_SLAVEMODE     \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0204, METHOD_BUFFERED, FILE_ANY_ACCESS)
// ##### Michael
#define IOCTL_SPI_WRITEREAD_CS     \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0205, METHOD_BUFFERED, FILE_ANY_ACCESS)

typedef struct {
    DWORD address;
	// See OMAP35XX_TPS659XX_TI_V1\omap35xx\INC\omap35xx_mcspi.h (CHCONF section) for config bit field values.
	// See OMAP35xx Technical Reference Manual for bit field explanations.
    DWORD config;
} IOCTL_SPI_CONFIGURE_IN;


//------------------------------------------------------------------------------
//
//  Functions: SPIxxx
//
__inline HANDLE SPIOpen(LPCTSTR pSpiName)
{
    HANDLE hDevice;
    DEVICE_CONTEXT_SPI *pContext = NULL;

    hDevice = CreateFile(pSpiName, 0, 0, NULL, 0, 0, NULL);
    if (hDevice == INVALID_HANDLE_VALUE) goto clean;

    // Allocate memory for our handler...
    if ((pContext = (DEVICE_CONTEXT_SPI *)LocalAlloc(
        LPTR, sizeof(DEVICE_CONTEXT_SPI)
    )) == NULL) {
        CloseHandle(hDevice);
        goto clean;
    }

    // Get function pointers.  If not possible (b/c of cross process calls), use IOCTLs instead
    if (!DeviceIoControl(
        hDevice, IOCTL_DDK_GET_DRIVER_IFC, (VOID*)&DEVICE_IFC_SPI_GUID,
        sizeof(DEVICE_IFC_SPI_GUID), &pContext->ifc, sizeof(DEVICE_IFC_SPI),
        NULL, NULL
    )) {
        //  Need to use IOCTLs instead of direct function ptrs
        pContext->ifc.context = 0;
    }

    // Save device handle
    pContext->hDevice = hDevice;

clean:
    return pContext;
}

__inline VOID SPIClose(HANDLE hContext)
{
    DEVICE_CONTEXT_SPI *pContext = (DEVICE_CONTEXT_SPI *)hContext;
    CloseHandle(pContext->hDevice);
    LocalFree(pContext);
}

__inline BOOL SPILockController(HANDLE hContext, DWORD dwTimeout)
{
    DEVICE_CONTEXT_SPI *pContext = (DEVICE_CONTEXT_SPI *)hContext;
    return pContext->ifc.pfnLockController(pContext->ifc.context, dwTimeout);
}

__inline BOOL SPIUnlockController(HANDLE hContext)
{
    DEVICE_CONTEXT_SPI *pContext = (DEVICE_CONTEXT_SPI *)hContext;
    return pContext->ifc.pfnUnlockController(pContext->ifc.context);
}

__inline BOOL SPIEnableChannel(HANDLE hContext)
{
    DEVICE_CONTEXT_SPI *pContext = (DEVICE_CONTEXT_SPI *)hContext;
    return pContext->ifc.pfnEnableChannel(pContext->ifc.context);
}

__inline BOOL SPIDisableChannel(HANDLE hContext)
{
    DEVICE_CONTEXT_SPI *pContext = (DEVICE_CONTEXT_SPI *)hContext;
    return pContext->ifc.pfnDisableChannel(pContext->ifc.context);
}

// See OMAP35XX_TPS659XX_TI_V1\omap35xx\INC\omap35xx_mcspi.h (CHCONF section) for config bit field values.
// See OMAP35xx Technical Reference Manual for bit field explanations.
__inline BOOL SPIConfigure(HANDLE hContext, DWORD address, DWORD config)
{
    DEVICE_CONTEXT_SPI *pContext = (DEVICE_CONTEXT_SPI *)hContext;
    if( pContext->ifc.context )
    {
        return pContext->ifc.pfnConfigure(pContext->ifc.context, address, config);
    }
    else
    {
        IOCTL_SPI_CONFIGURE_IN  dwIn;

        dwIn.address = address;
        dwIn.config = config;
        
        return DeviceIoControl(pContext->hDevice, 
                        IOCTL_SPI_CONFIGURE, 
                        &dwIn,
                        sizeof(dwIn),
                        NULL,
                        0,
                        NULL,
                        NULL );
    }
}

__inline BOOL SPISetSlaveMode(HANDLE hContext)
{
    DEVICE_CONTEXT_SPI *pContext = (DEVICE_CONTEXT_SPI *)hContext;
    if( pContext->ifc.context )
    {
        return pContext->ifc.pfnSetSlaveMode(pContext->ifc.context);
    }
    else
    {
        return DeviceIoControl(pContext->hDevice, 
                        IOCTL_SPI_SET_SLAVEMODE, 
                        NULL,
                        0,
                        NULL,
                        0,
                        NULL,
                        NULL );
    }
}

__inline DWORD SPIRead(HANDLE hContext, DWORD size, VOID *pBuffer)
{
    DEVICE_CONTEXT_SPI *pContext = (DEVICE_CONTEXT_SPI *)hContext;
    if( pContext->ifc.context )
    {
        return pContext->ifc.pfnRead(pContext->ifc.context, pBuffer, size);
    }
    else
    {
        DWORD   dwCount = 0;
        BOOL    rc;
    
        rc = ReadFile( pContext->hDevice, pBuffer, size, &dwCount, NULL );
        return dwCount;
    }
}
    
__inline DWORD SPIWrite(HANDLE hContext, DWORD size, VOID *pBuffer)
{
    DEVICE_CONTEXT_SPI *pContext = (DEVICE_CONTEXT_SPI *)hContext;
    if( pContext->ifc.context )
    {
        return pContext->ifc.pfnWrite(pContext->ifc.context, pBuffer, size);
    }
    else
    {
        DWORD   dwCount = 0;
        BOOL    rc;
    
        rc = WriteFile( pContext->hDevice, pBuffer, size, &dwCount, NULL );
        return dwCount;
    }
}
    
__inline DWORD SPIWriteRead(HANDLE hContext, DWORD size, VOID *pOutBuffer, VOID *pInBuffer)
{
    DEVICE_CONTEXT_SPI *pContext = (DEVICE_CONTEXT_SPI *)hContext;
    if( pContext->ifc.context )
    {
        return pContext->ifc.pfnWriteRead(pContext->ifc.context, size, pOutBuffer, pInBuffer);
    }
    else
    {
        return DeviceIoControl(pContext->hDevice, 
                        IOCTL_SPI_WRITEREAD, 
                        pInBuffer,
                        size,
                        pOutBuffer,
                        size,
                        NULL,
                        NULL );
    }
}

// ##### Michael
__inline DWORD SPIWriteReadCS(HANDLE hContext, DWORD inSize, UINT8 *pInBuffer, DWORD outSize, const UINT8 *pOutBuffer )
{
    DEVICE_CONTEXT_SPI *pContext = (DEVICE_CONTEXT_SPI *)hContext;
    if( pContext->ifc.context )
    {
        return pContext->ifc.pfnWriteReadCS(pContext->ifc.context, inSize, pInBuffer, outSize, pOutBuffer);
    }
    else
    {
        return DeviceIoControl(pContext->hDevice, 
                        IOCTL_SPI_WRITEREAD_CS, 
                        pInBuffer,
                        inSize,
                        (LPVOID)pOutBuffer,
                        outSize,
                        NULL,
                        NULL );
    }
}


__inline DWORD SPIAsyncWriteRead(HANDLE hContext, DWORD size, VOID *pOutBuffer, VOID *pInBuffer)
{
    DEVICE_CONTEXT_SPI *pContext = (DEVICE_CONTEXT_SPI *)hContext;
    if( pContext->ifc.context )
    {
        return pContext->ifc.pfnAsyncWriteRead(pContext->ifc.context, size, pOutBuffer, pInBuffer);
    }
    else
    {
        return DeviceIoControl(pContext->hDevice, 
                        IOCTL_SPI_ASYNC_WRITEREAD, 
                        pInBuffer,
                        size,
                        pOutBuffer,
                        size,
                        NULL,
                        NULL );
    }
}    

__inline DWORD SPIWaitForAsyncWriteReadComplet(HANDLE hContext, DWORD size, VOID *pOutBuffer)
{
    DEVICE_CONTEXT_SPI *pContext = (DEVICE_CONTEXT_SPI *)hContext;
    if( pContext->ifc.context )
    {
        return pContext->ifc.pfnWaitForAsyncWriteReadComplete(pContext->ifc.context, size, pOutBuffer);
    }
    else
    {
        return DeviceIoControl(pContext->hDevice, 
                        IOCTL_SPI_ASYNC_WRITEREAD_COMPLETE, 
                        NULL,
                        0,
                        pOutBuffer,
                        size,
                        NULL,
                        NULL );
    }
}    

//------------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif
