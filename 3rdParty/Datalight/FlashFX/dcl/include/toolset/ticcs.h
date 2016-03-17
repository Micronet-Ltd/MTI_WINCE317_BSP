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
    $Log: ticcs.h $
    Revision 1.2  2010/05/10 20:11:10Z  garyp
    Define D_UINTPTR in terms of uintptr_t from stdint.h.  Removed test code.
    Revision 1.1  2010/04/28 23:09:28Z  garyp
    Initial revision
 ---------------------------------------------------------------------------*/

#ifndef TICCS_H_INCLUDED
#define TICCS_H_INCLUDED

#include <stdint.h>     /* for intptr_t and uintptr_t */

 
/*-------------------------------------------------------------------
    Define the ToolSet name.
-------------------------------------------------------------------*/
#define DCL_TOOLSETNAME             "TICCS"


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
typedef int                 D_INT32;        /* must be exactly 32 bits wide */
typedef unsigned int        D_UINT32;       /* must be exactly 32 bits wide */

/*  WARNING -- "unsigned long" is apparently 40 bits -- don't use it
*/

#if DCL_NATIVE_64BIT_SUPPORT
typedef long long           D_INT64;        /* must be exactly 64 bits wide */
typedef unsigned long long  D_UINT64;       /* must be exactly 64 bits wide */

/*  Compiler specific suffixes for 64-bit literals
*/
#define UINT64SUFFIX( __value )     (__value ## ULL)
#define INT64SUFFIX( __value )      (__value ## LL)
#endif


/*-------------------------------------------------------------------
    Integral types large enough to hold a pointer. 
-------------------------------------------------------------------*/
typedef intptr_t            D_INTPTR;
typedef uintptr_t           D_UINTPTR;



#endif /* TICCS_H_INCLUDED */


