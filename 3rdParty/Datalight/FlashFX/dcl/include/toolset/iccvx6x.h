/*---------------------------------------------------------------------------
               ----> DO NOT REMOVE THE FOLLOWING NOTICE <----

                  Copyright (c) 1993 - 2010 Datalight, Inc.
                       All Rights Reserved Worldwide.

  Datalight, Incorporated is a Washington State corporation currently located
  at:
        21520 30th Dr SE, Suite 110,      Tel:  425-951-8086
        Bothell, WA  98021                Fax:  425-951-8094
        USA                               Web:  http://www.datalight.com

  This software, including without limitation all source code and documen-
  tation, is a trade secret and the confidential property of Datalight, 
  Inc., protected under the copyright laws of the United States and other
  jurisdictions.  Patents may be pending.

  U.S. Government Restricted Rights:  Use, duplication, reproduction, or
  transfer of this commercial product and accompanying documentation is
  restricted in accordance with FAR 12.212 and DFARS 227.7202 and by a
  License Agreement.

  IN ADDITION TO COPYRIGHT AND PATENT LAW, THIS SOFTWARE IS PROTECTED UNDER
  CONTRACT(S) BETWEEN DATALIGHT, INC. AND THE LICENSEE ("BINDING AGREEMENTS").
  IF YOU ARE A LICENSEE, YOUR RIGHT, IF ANY, TO COPY, PUBLISH, MODIFY, OR
  OTHERWISE USE THE SOFTWARE, IS SUBJECT TO THE TERMS AND CONDITIONS OF THE
  BINDING AGREEMENTS.  BY USING THE SOFTWARE IN ANY MANNER, IN WHOLE OR IN
  PART, YOU AGREE TO BE BOUND BY THE TERMS OF THE BINDING AGREEMENTS.

  IF YOU ARE NOT A DATALIGHT LICENSEE, ANY USE MAY RESULT IN CIVIL AND
  CRIMINAL ACTION AGAINST YOU.  CONTACT DATALIGHT, INC. AT THE ADDRESS
  SET FORTH ABOVE IF YOU OBTAINED THIS SOFTWARE IN ERROR.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Description

    This header contains ToolSet specific data type definitions.
 ---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: iccvx6x.h $
    Revision 1.1  2010/06/18 14:58:30Z  jimmb
    Initial revision
---------------------------------------------------------------------------*/

#ifndef ICCVX6X_H_INCLUDED
#define ICCVX6X_H_INCLUDED


/*  VxWorks architecture types
*/
#include <vxWorks.h>
#include <types.h>
#include <types/vxTypesBase.h>


/*  Determine the VxWorks version
*/
#if (_WRS_VXWORKS_MAJOR == 6)
  #if (_WRS_VXWORKS_MINOR == 9)
    #define DCL_OS_VERSION   0x0690
  #else  
    #error "UnSupported VxWorks version"
  #endif
#else
  #error "Unknown/UnSupported VxWorks version"
#endif


/*-------------------------------------------------------------------
    Define the ToolSet name.
-------------------------------------------------------------------*/
#define DCL_TOOLSETNAME             "ICCVX6X"


/*-------------------------------------------------------------------
    Specify the byte-order of the target CPU.

    TRUE    Big Endian
    FALSE   Little Endian
-------------------------------------------------------------------*/
#if _BYTE_ORDER == _BIG_ENDIAN
    #define DCL_BIG_ENDIAN          TRUE
#else
    #define DCL_BIG_ENDIAN          FALSE
#endif


/*-------------------------------------------------------------------
    Define whether 64-bit types are natively supported or not.
-------------------------------------------------------------------*/
#define DCL_NATIVE_64BIT_SUPPORT TRUE


/*-------------------------------------------------------------------
    Define the basic fixed size data types, upon which the rest of
    the data types in the product are based.
-------------------------------------------------------------------*/
typedef int8_t              D_INT8;         /* must be exactly 8 bits wide  */
typedef uint8_t             D_UINT8;        /* must be exactly 8 bits wide  */
typedef int16_t             D_INT16;        /* must be exactly 16 bits wide */
typedef uint16_t            D_UINT16;       /* must be exactly 16 bits wide */
typedef int32_t             D_INT32;        /* must be exactly 32 bits wide */
typedef uint32_t            D_UINT32;       /* must be exactly 32 bits wide */

#if DCL_NATIVE_64BIT_SUPPORT
typedef long long           D_INT64;        /* must be exactly 64 bits wide */
typedef unsigned long long  D_UINT64;       /* must be exactly 64 bits wide */

/*  Compiler specific suffixes for 64-bit literals
*/
#define UINT64SUFFIX( __value )     (__value ## ULL)
#define INT64SUFFIX( __value )      (__value ## LL)
#endif


/*-------------------------------------------------------------------
    Integral types large enough to hold a pointer.  Typically these
    would be defined in terms of intptr_t and uintptr_t, however
    these types don't appear to be available in the standard VxWorks
    headers.  Experimentation has shown that even when using a 64-bit
    BSP, an "int" as well as pointers are still 32-bits.
-------------------------------------------------------------------*/
typedef D_INT32             D_INTPTR;
typedef D_UINT32            D_UINTPTR;


#endif /* ICCVX6X_H_INCLUDED */


