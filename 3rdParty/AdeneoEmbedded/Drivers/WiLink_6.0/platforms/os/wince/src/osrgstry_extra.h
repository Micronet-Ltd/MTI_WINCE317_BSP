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

/** \file  osRgstry_extra.h 
 *  \brief 
 *
 *  \see 
 */

#ifndef __OSRGSTRY_EXTRA_H_
#define __OSRGSTRY_EXTRA_H_


VOID
regReadIntegerParameter(
				 TWlanDrvIfObjPtr 		pAdapter,
				 PNDIS_STRING			pParameterName,
				 ULONG					defaultValue,
				 ULONG					minValue,
				 ULONG					maxValue,
				 UCHAR					parameterSize,
				 PUCHAR					pParameter
				 );

VOID
regReadStringParameter(
				 TWlanDrvIfObjPtr 		pAdapter,
				 PNDIS_STRING			pParameterName,
				 PCHAR					pDefaultValue,
				 USHORT					defaultLen,
				 PUCHAR					pParameter,
				 void*					pParameterSize
				 );

VOID
regReadUnicodeStringParameter(
				 TWlanDrvIfObjPtr 		pAdapter,
				 PNDIS_STRING			pParameterName,
				 PCHAR					pDefaultValue,
				 UCHAR					defaultLen,
				 PUCHAR					pParameter,
				 PUCHAR					pParameterSize
				 );


VOID
regReadWepKeyParameter(
				 TWlanDrvIfObjPtr 		pAdapter,
				 PUCHAR					pKeysStructure,
				 UINT8					defaultKeyId
				 );

VOID
regReadNetworkAddress(TWlanDrvIfObjPtr pAdapter);


/* registry callbacks */
void regSetupRegistryNotifyCB(TWlanDrvIfObjPtr pAdapter);
void regRemoveRegistryNotifyCB(TWlanDrvIfObjPtr pAdapter);

#endif

