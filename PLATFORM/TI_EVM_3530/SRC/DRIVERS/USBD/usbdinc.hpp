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
/*++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Module Name:  

usbdinc.hpp

Abstract:  

Constants for USBD driver.

Notes: 

--*/
#ifndef __USBDINC_HPP__
#define __USBDINC_HPP__

// USBD Version Information
const DWORD gcMajorUSBDVersion = 1;
const DWORD gcMinorUSBDVersion = 3;

const int gcMaxPipes = 31; // 1 bi (ep0) + 15 in + 15 out
const int gcTransferAllocateChunk = 4;
const int gcEndpointZero = 0;

const int gcMaxDriverString = 100;
const int gcMaxDriverIdSize = 256;
const int gcCancelGranularity = 20;



#endif
