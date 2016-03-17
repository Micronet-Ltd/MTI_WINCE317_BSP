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

#ifndef __OSRGSTRY_H_
#define __OSRGSTRY_H_

typedef struct {

    TI_UINT8*                  ParameterName;
    NDIS_STRING             NdisParameterName;

    NDIS_PARAMETER_TYPE     ParameterType;

    TI_BOOL                 RangeCheck;

    TI_UINT32                   DefaultValue;
    TI_UINT32                   MinValue;
    TI_UINT32                   MaxValue;

    TI_UINT32                   FieldOffset;
    TI_UINT32                   FieldSize;

} REGISTRY_DATA, *PREGISTRY_DATA;


void
regFillInitTable(
    TWlanDrvIfObjPtr pAdapter,
    void* pInitTable
    );

#ifdef _WINDOWS
void regReadParameters (TWlanDrvIfObjPtr pAdapter);
#endif

#ifdef TI_DBG

void
regReadLastDbgState(
    TWlanDrvIfObjPtr pAdapter
    );


#endif

#endif      /* __OSRGSTRY_H_*/

