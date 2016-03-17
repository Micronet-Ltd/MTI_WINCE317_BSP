/***************************************************************************
**+----------------------------------------------------------------------+**
**|                                ****                                  |**
**|                                ****                                  |**
**|                                ******o***                            |**
**|                          ********_///_****                           |**
**|                           ***** /_//_/ ****                          |**
**|                            ** ** (__/ ****                           |**
**|                                *********                             |**
**|                                 ****                                 |**
**|                                  ***                                 |**
**|                                                                      |**
**|     Copyright (c) 1998 - 2009 Texas Instruments Incorporated         |**
**|                        ALL RIGHTS RESERVED                           |**
**|                                                                      |**
**| Permission is hereby granted to licensees of Texas Instruments       |**
**| Incorporated (TI) products to use this computer program for the sole |**
**| purpose of implementing a licensee product based on TI products.     |**
**| No other rights to reproduce, use, or disseminate this computer      |**
**| program, whether in part or in whole, are granted.                   |**
**|                                                                      |**
**| TI makes no representation or warranties with respect to the         |**
**| performance of this computer program, and specifically disclaims     |**
**| any responsibility for any damages, special or consequential,        |**
**| connected with the use of this program.                              |**
**|                                                                      |**
**+----------------------------------------------------------------------+**
***************************************************************************/
#ifndef __PRIVCMD_H_
#define __PRIVCMD_H_

#define SHELLDRV_IOCTL_INDEX  				0x00800
#define WINDOWS_MOBILE_SPECIFIC_OFFSET  	0xb50

#define TIWLN_EVENTS_REGISTRATION	CTL_CODE(FILE_DEVICE_UNKNOWN, \
										SHELLDRV_IOCTL_INDEX + WINDOWS_MOBILE_SPECIFIC_OFFSET + 10, \
										METHOD_BUFFERED,          \
										FILE_ANY_ACCESS)

#define TIWLN_DISABLE_EVENT			CTL_CODE(FILE_DEVICE_UNKNOWN, \
										SHELLDRV_IOCTL_INDEX + WINDOWS_MOBILE_SPECIFIC_OFFSET + 11, \
										METHOD_BUFFERED,          \
										FILE_ANY_ACCESS)

#define TIWLN_ENABLE_EVENT			CTL_CODE(FILE_DEVICE_UNKNOWN, \
										SHELLDRV_IOCTL_INDEX + WINDOWS_MOBILE_SPECIFIC_OFFSET + 12, \
										METHOD_BUFFERED,          \
										FILE_ANY_ACCESS)

#define TIWLN_EVENTS_DEBUG_LEVEL	CTL_CODE(FILE_DEVICE_UNKNOWN, \
										SHELLDRV_IOCTL_INDEX + WINDOWS_MOBILE_SPECIFIC_OFFSET + 13, \
										METHOD_BUFFERED,          \
										FILE_ANY_ACCESS)

#define TIWLN_EVENTS_UNREGISTER		CTL_CODE(FILE_DEVICE_UNKNOWN, \
										SHELLDRV_IOCTL_INDEX + WINDOWS_MOBILE_SPECIFIC_OFFSET + 14, \
										METHOD_BUFFERED,          \
										FILE_ANY_ACCESS)

#define TIWLN_REDIR_TO_TICON		CTL_CODE(FILE_DEVICE_UNKNOWN, \
										SHELLDRV_IOCTL_INDEX + WINDOWS_MOBILE_SPECIFIC_OFFSET + 15, \
										METHOD_BUFFERED,          \
										FILE_ANY_ACCESS)										


#endif
