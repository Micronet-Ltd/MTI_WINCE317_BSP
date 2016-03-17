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

#ifndef __OSDEBUG_H_
#define __OSDEBUG_H_

#ifdef TI_DBG
#ifndef _WINDOWS
#include "windows_types.h"
#endif /*_WINDOWS*/


#define IF_TIDEBUG(f)  if (!((TiDebugFlag & (f))^(f))) 
extern unsigned long TiDebugFlag;

#define PRINT(F, A)		IF_TIDEBUG( F ) { os_printf(A); }
#define PRINTF(F, A)	IF_TIDEBUG( F ) { os_printf A; }

#define DBG_INIT				0x0001
#define DBG_REGISTRY			0x0002
#define DBG_NDIS_CALLS			0x0004
#define DBG_NDIS_OIDS			0x0008
#define DBG_PCI_RES				0x0010
#define DBG_INTERRUPT			0x0020
#define DBG_IOCTL				0x0040
#define DBG_RECV				0x0080
#define DBG_SEND				0x0100

#define DBG_SEV_INFO			0x0001
#define DBG_SEV_LOUD			0x0002
#define DBG_SEV_VERY_LOUD		0x0004
#define DBG_SEV_WARNING			0x0008
#define DBG_SEV_ERROR			0x0010
#define DBG_SEV_FATAL_ERROR		0x0020


#define DBG_INIT_INFO				((DBG_INIT << 16) | DBG_SEV_INFO)
#define DBG_INIT_LOUD				((DBG_INIT << 16) | DBG_SEV_LOUD)
#define DBG_INIT_VERY_LOUD			((DBG_INIT << 16) | DBG_SEV_VERY_LOUD)
#define DBG_INIT_WARNING			((DBG_INIT << 16) | DBG_SEV_WARNING)
#define DBG_INIT_ERROR				((DBG_INIT << 16) | DBG_SEV_ERROR)
#define DBG_INIT_FATAL_ERROR		((DBG_INIT << 16) | DBG_SEV_FATAL_ERROR)

#define DBG_REGISTRY_INFO			((DBG_REGISTRY << 16) | DBG_SEV_INFO)
#define DBG_REGISTRY_LOUD			((DBG_REGISTRY << 16) | DBG_SEV_LOUD)
#define DBG_REGISTRY_VERY_LOUD		((DBG_REGISTRY << 16) | DBG_SEV_VERY_LOUD)
#define DBG_REGISTRY_WARNING		((DBG_REGISTRY << 16) | DBG_SEV_WARNING)
#define DBG_REGISTRY_ERROR			((DBG_REGISTRY << 16) | DBG_SEV_ERROR)
#define DBG_REGISTRY_FATAL_ERROR	((DBG_REGISTRY << 16) | DBG_SEV_FATAL_ERROR)

#define DBG_NDIS_CALLS_INFO			((DBG_NDIS_CALLS << 16) | DBG_SEV_INFO)
#define DBG_NDIS_CALLS_LOUD			((DBG_NDIS_CALLS << 16) | DBG_SEV_LOUD)
#define DBG_NDIS_CALLS_VERY_LOUD	((DBG_NDIS_CALLS << 16) | DBG_SEV_VERY_LOUD)
#define DBG_NDIS_CALLS_WARNING		((DBG_NDIS_CALLS << 16) | DBG_SEV_WARNING)
#define DBG_NDIS_CALLS_ERROR		((DBG_NDIS_CALLS << 16) | DBG_SEV_ERROR)
#define DBG_NDIS_CALLS_FATAL_ERROR	((DBG_NDIS_CALLS << 16) | DBG_SEV_FATAL_ERROR)

#define DBG_NDIS_OIDS_INFO			((DBG_NDIS_OIDS << 16) | DBG_SEV_INFO)
#define DBG_NDIS_OIDS_LOUD			((DBG_NDIS_OIDS << 16) | DBG_SEV_LOUD)
#define DBG_NDIS_OIDS_VERY_LOUD		((DBG_NDIS_OIDS << 16) | DBG_SEV_VERY_LOUD)
#define DBG_NDIS_OIDS_WARNING		((DBG_NDIS_OIDS << 16) | DBG_SEV_WARNING)
#define DBG_NDIS_OIDS_ERROR			((DBG_NDIS_OIDS << 16) | DBG_SEV_ERROR)
#define DBG_NDIS_OIDS_FATAL_ERROR	((DBG_NDIS_OIDS << 16) | DBG_SEV_FATAL_ERROR)

#define DBG_PCI_RES_INFO			((DBG_PCI_RES << 16) | DBG_SEV_INFO)
#define DBG_PCI_RES_LOUD			((DBG_PCI_RES << 16) | DBG_SEV_LOUD)
#define DBG_PCI_RES_VERY_LOUD		((DBG_PCI_RES << 16) | DBG_SEV_VERY_LOUD)
#define DBG_PCI_RES_WARNING			((DBG_PCI_RES << 16) | DBG_SEV_WARNING)
#define DBG_PCI_RES_ERROR			((DBG_PCI_RES << 16) | DBG_SEV_ERROR)
#define DBG_PCI_RES_FATAL_ERROR		((DBG_PCI_RES << 16) | DBG_SEV_FATAL_ERROR)

#define DBG_INTERRUPT_INFO			((DBG_INTERRUPT << 16) | DBG_SEV_INFO)
#define DBG_INTERRUPT_LOUD			((DBG_INTERRUPT << 16) | DBG_SEV_LOUD)
#define DBG_INTERRUPT_VERY_LOUD		((DBG_INTERRUPT << 16) | DBG_SEV_VERY_LOUD)
#define DBG_INTERRUPT_WARNING		((DBG_INTERRUPT << 16) | DBG_SEV_WARNING)
#define DBG_INTERRUPT_ERROR			((DBG_INTERRUPT << 16) | DBG_SEV_ERROR)
#define DBG_INTERRUPT_FATAL_ERROR	((DBG_INTERRUPT << 16) | DBG_SEV_FATAL_ERROR)

#define DBG_IOCTL_INFO				((DBG_IOCTL << 16) | DBG_SEV_INFO)
#define DBG_IOCTL_LOUD				((DBG_IOCTL << 16) | DBG_SEV_LOUD)
#define DBG_IOCTL_VERY_LOUD			((DBG_IOCTL << 16) | DBG_SEV_VERY_LOUD)
#define DBG_IOCTL_WARNING			((DBG_IOCTL << 16) | DBG_SEV_WARNING)
#define DBG_IOCTL_ERROR				((DBG_IOCTL << 16) | DBG_SEV_ERROR)
#define DBG_IOCTL_FATAL_ERROR		((DBG_IOCTL << 16) | DBG_SEV_FATAL_ERROR)

#define DBG_RECV_INFO				((DBG_RECV << 16) | DBG_SEV_INFO)
#define DBG_RECV_LOUD				((DBG_RECV << 16) | DBG_SEV_LOUD)
#define DBG_RECV_VERY_LOUD			((DBG_RECV << 16) | DBG_SEV_VERY_LOUD)
#define DBG_RECV_WARNING			((DBG_RECV << 16) | DBG_SEV_WARNING)
#define DBG_RECV_ERROR				((DBG_RECV << 16) | DBG_SEV_ERROR)
#define DBG_RECV_FATAL_ERROR		((DBG_RECV << 16) | DBG_SEV_FATAL_ERROR)

#define DBG_SEND_INFO				((DBG_SEND << 16) | DBG_SEV_INFO)
#define DBG_SEND_LOUD				((DBG_SEND << 16) | DBG_SEV_LOUD)
#define DBG_SEND_VERY_LOUD			((DBG_SEND << 16) | DBG_SEV_VERY_LOUD)
#define DBG_SEND_WARNING			((DBG_SEND << 16) | DBG_SEV_WARNING)
#define DBG_SEND_ERROR				((DBG_SEND << 16) | DBG_SEV_ERROR)
#define DBG_SEND_FATAL_ERROR		((DBG_SEND << 16) | DBG_SEV_FATAL_ERROR)


#else

#define PRINT(F, A)
#define PRINTF(F, A)

#endif


#endif /* __OSDEBUG_H_*/

