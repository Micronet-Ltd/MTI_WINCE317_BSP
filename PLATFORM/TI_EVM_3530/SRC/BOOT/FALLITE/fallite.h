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
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES.
//

#ifndef __FALLITE_H__
#define __FALLITE_H__

#include <fmd.h>
#include <fls.h>

#ifdef __cplusplus
extern "C" {
#endif	// __cplusplus
typedef DWORD (*pfnFMD_ReadSectorNotify)();

PVOID FAL_Init (PVOID hFMD, LPBYTE buffer);
VOID FAL_Deinit(PVOID hFAL);
BOOL FAL_ReadSectors(PVOID hFAL, SECTOR_ADDR sectorLogical, LPBYTE pSectorBuff, DWORD dwSectorCount);
BOOL FAL_WriteSectors(PVOID hFAL, SECTOR_ADDR startSectorAddr, LPBYTE pSectorBuff, DWORD dwSectorCount);
BOOL FAL_GetInfo (PVOID hFAL, DISK_INFO *pdi);
BOOL FAL_LockFlashRegion (REGION_TYPE regionType);
BOOL FAL_UnlockFlashRegion (REGION_TYPE regionType);
void SetFalNotify(pfnFMD_ReadSectorNotify fnFal_ReadSectorNotify);

#ifdef __cplusplus
}
#endif	// __cplusplus

#endif __FALLITE_H__

