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
    $Log: gnucs.h $
    Revision 1.4  2010/08/04 00:10:32Z  garyp
    Updated to work around the "VA64BUG" issue.
    Revision 1.3  2009/11/25 22:37:44Z  keithg
    Corrected Bug 2894, removed obsolete DCLPACK definition.
    Revision 1.2  2007/11/03 23:31:20Z  Garyp
    Added the standard module header.
    Revision 1.1  2007/06/15 23:01:38Z  pauli
    Initial revision
---------------------------------------------------------------------------*/

#ifndef GNUCS_H_INCLUDED
#define GNUCS_H_INCLUDED


/*-------------------------------------------------------------------
    Define the ToolSet name.
-------------------------------------------------------------------*/
#define DCL_TOOLSETNAME             "GNUCS"


/*  Specify the Endian-ness of the target CPU

    1  Big Endian
    0  Little Endian
*/
#ifdef LITTLE_ENDIAN
#define DCL_BIG_ENDIAN        0
#else
#define DCL_BIG_ENDIAN        1
#endif


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

#define VAARG64_BROKEN      (TRUE)

/*  Integral types large enough to hold a pointer.
*/
typedef D_INT32             D_INTPTR;
typedef D_UINT32            D_UINTPTR;

#endif /* GNUCS_H_INCLUDED */
