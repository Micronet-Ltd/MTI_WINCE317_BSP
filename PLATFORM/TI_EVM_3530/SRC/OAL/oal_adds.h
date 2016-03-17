
//
//  This file contains OMAP specific oal extensions.
//
#ifndef __OAL_ADDS_H
#define __OAL_ADDS_H

#ifdef __cplusplus
extern "C" {
#endif

//-----------------------------------------------------------------------------
//
//  Define: IOCTL_HAL_SW_UPDATE_FLAG
//
//  This code sw updating flags
//
//#define IOCTL_HAL_SW_UPDATE_FLAG  CTL_CODE(FILE_DEVICE_HAL, 2100, METHOD_BUFFERED, FILE_ANY_ACCESS)

BOOL OALIoCtlHalSwUpdateFlag(UINT32 code, VOID *pInpBuffer, UINT32 inpSize, VOID *pOutBuffer, UINT32 outSize, UINT32 *pOutSize);
//-----------------------------------------------------------------------------
//
//  Define: IOCTL_HAL_PARAM_BLOCK
//
//  This code parameters block
//
//#define IOCTL_HAL_PARAM_BLOCK  CTL_CODE(FILE_DEVICE_HAL, 2101, METHOD_BUFFERED, FILE_ANY_ACCESS)

BOOL OALIoCtlHalParamBlock(UINT32 code, VOID *pInpBuffer, UINT32 inpSize, VOID *pOutBuffer, UINT32 outSize, UINT32 *pOutSize);

//#define IOCTL_HAL_GET_HW_BOOT_STAT CTL_CODE(FILE_DEVICE_HAL, 2102, METHOD_BUFFERED, FILE_ANY_ACCESS)

BOOL OALIoCtlHalGetHwBootStat(UINT32 code, VOID *pInBuffer, UINT32 inSize, VOID *pOutBuffer, UINT32 outSize, UINT32 *pOutSize);

BOOL OALIoCtlHalGetHwBootStatEx(UINT32 code, VOID *pInBuffer, UINT32 inSize, VOID *pOutBuffer, UINT32 outSize, UINT32 *pOutSize);

#ifdef __cplusplus
}
#endif

#endif
