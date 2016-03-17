/*
================================================================================
*             Micronet CE500(TM) Platform Software
* (c) Copyright Micronet Ltd. All Rights Reserved.
*
* Use of this software is controlled by the terms and conditions found
* in the license agreement under which this software has been supplied.
*
================================================================================
*/
//
//  File: head_set.h
//

#ifndef __HEAD_SET_H_
#define __HEAD_SET_H_

#ifdef __cplusplus
extern "C" {
#endif
//------------------------------------------------------------------------------
//  Definition from devnotify.h used in smartphone
#define FILE_DEVICE_HEADSET                 32768
#define IOCTL_HEADSET_ADD_NOTIFY_HWND       CTL_CODE(FILE_DEVICE_HEADSET, 2048, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_HEADSET_REMOVE_NOTIFY_HWND    CTL_CODE(FILE_DEVICE_HEADSET, 2049, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_HEADSET_MIC_AMP_CTRL			CTL_CODE(FILE_DEVICE_HEADSET, 2050, METHOD_BUFFERED, FILE_ANY_ACCESS)

typedef enum
{
	HEADSET_MIC_UNSPEC,
	HEADSET_MIC_ON,
	HEADSET_MIC_OFF,
	HEADSET_MIC_INVALID
}HEADSET_MIC_AMP_CTRL;

typedef struct tag_DEVNOTIFY_EVENT {
    HWND hWndNotify;
    UINT Msg;
} DEVNOTIFY_EVENT, *PDEVNOTIFY_EVENT;

#ifdef __cplusplus
}
#endif

#endif //__HEAD_SET_H_
//-----------------------------------------------------------------------------

