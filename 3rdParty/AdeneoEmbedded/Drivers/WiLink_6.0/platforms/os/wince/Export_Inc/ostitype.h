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

/** \file  ostitype.h 
 *  \brief 
 *
 *  \see   
 */

#ifndef __OSTITYPE_H__
#define __OSTITYPE_H__


typedef char                TI_CHAR;
typedef signed char         TI_INT8;
typedef unsigned char       TI_UINT8;
typedef signed short        TI_INT16;
typedef unsigned short      TI_UINT16;
typedef signed int          TI_INT32;
typedef unsigned int        TI_UINT32;
typedef signed long long    TI_INT64;
typedef unsigned long long  TI_UINT64;

typedef TI_UINT32           TI_BOOL;
typedef TI_UINT8			TI_BOOLEAN;

#define TI_CONST64(x)       (x##LL)

#define TI_LIKELY           
#define TI_UNLIKELY       
  
#define inline __inline
#define INLINE              inline

typedef TI_INT32            NDIS_STATUS, *PNDIS_STATUS;

#endif /* __OSTITYPE_H__*/


