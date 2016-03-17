/*---------------------------------------------------------------------------
               ----> DO NOT REMOVE THE FOLLOWING NOTICE <----

      Copyright (c) 1993 - 2007 Datalight, Inc.  All Rights Reserved.

  Datalight, Incorporated is a Washington State corporation located at:

        21520 30th Dr SE, Suite 110,      Tel:  425-951-8086
        Bothell, WA  98021                Fax:  425-951-8094
        USA                               Web:  http://www.datalight.com

  This software, including without limitation, all source code and documen-
  tation, is the confidential, trade secret property of Datalight, Inc., and
  is protected under the copyright laws of the United States and other juris-
  dictions.

  In addition to civil penalties for infringement of copyright under applic-
  able U.S. law, 17 U.S.C. 1204 provides criminal penalties for violation of
  (a) the restrictions on circumvention of copyright protection systems found
  in 17 U.S.C. 1201 and (b) the protections for the integrity of copyright
  management information found in 17 U.S.C. 1202.

  U.S. Government Restricted Rights:  Use, duplication, reproduction, or
  transfer of this commercial product and accompanying documentation is
  restricted in accordance with FAR 12.212 and DFARS 227.7202 and by a
  License Agreement.

  IN ADDITION TO COPYRIGHT AND PATENT LAW, THIS SOFTWARE IS PROTECTED UNDER A
  SOURCE CODE AGREEMENT, NON-DISCLOSURE AGREEMENT (NDA), OR SIMILAR BINDING
  CONTRACT BETWEEN DATALIGHT, INC. AND THE LICENSEE ("BINDING AGREEMENT").
  YOUR RIGHT, IF ANY, TO COPY, PUBLISH, MODIFY OR OTHERWISE USE THE SOFTWARE,
  IS SUBJECT TO THE TERMS AND CONDITIONS OF THE BINDING AGREEMENT.  BY USING
  THE SOFTWARE IN ANY MANNER, IN WHOLE OR IN PART, YOU AGREE TO BE BOUND BY
  THE TERMS OF THE BINDING AGREEMENT.  CONTACT DATALIGHT, INC. AT THE ADDRESS
  SET FORTH ABOVE IF YOU OBTAINED THIS SOFTWARE IN ERROR.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Description

    This header contains ToolSet specific data type definitions.
 ---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: gnurtos.h $
    Revision 1.1  2010/06/19 12:24:22Z  jimmb
    Initial revision
    Revision 1.1  2010/06/19 12:24:22Z  jimmb
    Initial revision
---------------------------------------------------------------------------*/

#ifndef RTOS_H_INCLUDED
#define RTOS_H_INCLUDED


/*-------------------------------------------------------------------
    Define the ToolSet name.
-------------------------------------------------------------------*/
#define DCL_TOOLSETNAME             "RTOS"


/*-------------------------------------------------------------------
    Specify the byte-order of the target CPU.

    TRUE    Big Endian
    FALSE   Little Endian
-------------------------------------------------------------------*/
#ifdef LITTLE_ENDIAN
#define DCL_BIG_ENDIAN        FALSE
#else
#define DCL_BIG_ENDIAN        TRUE
#endif


/*-------------------------------------------------------------------
    DCLPACK is used on some structure declarations to save memory.
    This feature may not exist in all ToolSet implementations, and
    therefore must not be used to force on-disk structures to match
    those in memory.
-------------------------------------------------------------------*/
#define DCLPACK


/*-------------------------------------------------------------------
    Define whether 64-bit types are natively supported or not.
-------------------------------------------------------------------*/
#define DCL_NATIVE_64BIT_SUPPORT TRUE


/*-------------------------------------------------------------------
    Define the basic fixed size data types, upon which the rest of
    the data types in the product are based.
-------------------------------------------------------------------*/
typedef signed char         D_INT8;         /* must be exactly 8 bits wide  */
typedef unsigned char       D_UINT8;        /* must be exactly 8 bits wide  */
typedef short               D_INT16;        /* must be exactly 16 bits wide */
typedef unsigned short      D_UINT16;       /* must be exactly 16 bits wide */
typedef long                D_INT32;        /* must be exactly 32 bits wide */
typedef unsigned long       D_UINT32;       /* must be exactly 32 bits wide */

#if DCL_NATIVE_64BIT_SUPPORT
typedef long long           D_INT64;        /* must be exactly 64 bits wide */
typedef unsigned long long  D_UINT64;       /* must be exactly 64 bits wide */

/*  Compiler specific suffixes for 64-bit literals
*/
#define UINT64SUFFIX( __value )     (__value ## ULL)
#define INT64SUFFIX( __value )      (__value ## LL)
#endif

/*  Integral types large enough to hold a pointer.
*/
typedef D_INT32             D_INTPTR;
typedef D_UINT32            D_UINTPTR;

#endif /* RTOS_H_INCLUDED */
