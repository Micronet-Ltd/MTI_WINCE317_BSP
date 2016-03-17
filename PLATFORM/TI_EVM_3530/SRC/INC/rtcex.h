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
//  File: rtcex.h
//

#ifndef __RTCEX_H_
#define __RTCEX_H_

#define RTCDEV	L"RTC1:"

//------------------------------------------------------------------------------

#define IOCTL_RTC_GETTIME CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0800, METHOD_BUFFERED, FILE_ANY_ACCESS)

#endif //__RTCEX_H_
//-----------------------------------------------------------------------------

