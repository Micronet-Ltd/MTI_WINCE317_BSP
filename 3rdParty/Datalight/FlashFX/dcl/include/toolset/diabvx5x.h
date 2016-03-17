/*---------------------------------------------------------------------------
               ----> DO NOT REMOVE THE FOLLOWING NOTICE <----

                  Copyright (c) 1993 - 2009 Datalight, Inc.
                       All Rights Reserved Worldwide.

  Datalight, Incorporated is a Washington State corporation located at:

        21520 30th Dr SE, Suite 110,      Tel:  425-951-8086
        Bothell, WA  98021                Fax:  425-951-8094
        USA                               Web:  http://www.datalight.com

  This software, including without limitation all source code and documen-
  tation, is the confidential, trade secret property of Datalight, Inc.
  and is protected under the copyright laws of the United States and other
  jurisdictions.  Patents may be pending.

  In addition to civil penalties for infringement of copyright under appli-
  cable U.S. law, 17 U.S.C. 1204 provides criminal penalties for violation
  of (a) the restrictions on circumvention of copyright protection systems
  found in 17 U.S.C. 1201 and (b) the protections for the integrity of
  copyright management information found in 17 U.S.C. 1202.

  U.S. Government Restricted Rights:  Use, duplication, reproduction, or
  transfer of this commercial product and accompanying documentation is
  restricted in accordance with FAR 12.212 and DFARS 227.7202 and by a
  License Agreement.

  IN ADDITION TO COPYRIGHT AND PATENT LAW, THIS SOFTWARE IS PROTECTED UNDER
  A SOURCE CODE AGREEMENT, NON-DISCLOSURE AGREEMENT, AND/OR SIMILAR BINDING
  CONTRACT BETWEEN DATALIGHT, INC. AND THE LICENSEE ("BINDING AGREEMENTS").
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
    $Log: diabvx5x.h $
    Revision 1.7  2009/11/25 22:37:43Z  keithg
    Corrected Bug 2894, removed obsolete DCLPACK definition.
    Revision 1.6  2007/11/03 23:31:20Z  Garyp
    Added the standard module header.
    Revision 1.5  2007/01/28 00:45:36Z  Garyp
    Updated to define the ToolSet name.
    Revision 1.4  2006/04/28 22:21:00Z  michaelm
    changed intptr_t and uintptr_t to long and unsigned long respectively
    Revision 1.3  2006/04/26 23:04:54Z  billr
    Support for pointers wider than 32 bits.
    Revision 1.2  2006/01/20 23:23:00Z  brandont
    Added defines for DCL_OS_VERSION.
    Revision 1.1  2005/12/02 16:46:26Z  Pauli
    Initial revision
    Revision 1.3  2005/12/02 16:46:25Z  Pauli
    Merge with 2.0 product line, build 173.
    Revision 1.2  2005/10/21 02:59:05Z  garyp
    Documentation cleanup.
    Revision 1.1  2005/06/20 22:32:04Z  Garyp
    Initial revision
    Revision 1.1  2005/06/20 22:32:04Z  pauli
    Initial revision
    Revision 1.5  2005/10/03 20:47:28Z  brandont
    Added define for DCL_BIG_ENDIAN based on the _BYTE_ORDER
    define in the VxWorks header files.
    Revision 1.4  2005/06/20 22:32:03Z  PaulI
    Updated to use vxworks types.  Added condition for native 64-bit support.
    Revision 1.3  2005/06/20 21:05:04Z  Pauli
    Updated 64-bit types.
    Revision 1.2  2005/06/11 04:20:20Z  PaulI
    Enabled DCL types.
    Revision 1.1  2005/03/25 00:09:34Z  GaryP
    Initial revision
---------------------------------------------------------------------------*/

#ifndef DIABVX5X_H_INCLUDED
#define DIABVX5X_H_INCLUDED


/*  VxWorks architecture types
*/
#include "vxWorks.h"
#include "types.h"
#include "types/vxTypesBase.h"


/*  Determine the VxWorks version
*/
#ifdef _WRS_VXWORKS_5_X
#define DCL_OS_VERSION   0x0550
#else
#define DCL_OS_VERSION   0x0540
#endif


/*-------------------------------------------------------------------
    Define the ToolSet name.
-------------------------------------------------------------------*/
#define DCL_TOOLSETNAME             "DIABVX5X"


/*-------------------------------------------------------------------
    Specify the byte-order of the target CPU.

    TRUE    Big Endian
    FALSE   Little Endian
-------------------------------------------------------------------*/
#if(_BYTE_ORDER == _BIG_ENDIAN)
#define DCL_BIG_ENDIAN          TRUE
#else
#define DCL_BIG_ENDIAN          FALSE
#endif


/*-------------------------------------------------------------------
    Define whether 64-bit types are natively supported or not.
-------------------------------------------------------------------*/
#ifndef _WRS_VXWORKS_5_X
#define DCL_NATIVE_64BIT_SUPPORT       FALSE
#else
#define DCL_NATIVE_64BIT_SUPPORT       TRUE
#endif


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

/*  Integral types large enough to hold a pointer.
*/
typedef long            D_INTPTR;
typedef unsigned long   D_UINTPTR;

#endif /* DIABVX5X_H_INCLUDED */
