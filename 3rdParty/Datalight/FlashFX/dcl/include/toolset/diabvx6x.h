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
    $Log: diabvx6x.h $
    Revision 1.18  2010/06/09 17:10:15Z  jimmb
    Updated VxWorks build files to support VxWorks 6.9
    Revision 1.17  2009/11/25 22:34:46Z  keithg
    Corrected Bug 2894, removed obsolete DCLPACK definition.
    Revision 1.16  2009/08/20 21:52:46Z  jimmb
    Updated VxWorks build files to support VxWorks 6.8
    Revision 1.15  2008/08/25 19:55:27Z  jimmb
    Updated for VxWorks 6.7
    Revision 1.14  2008/06/03 21:43:09Z  keithg
    Clarified comments with professional customer facing terms.
    Revision 1.13  2007/11/07 17:25:25Z  pauli
    Made #error messages strings to prevent macro expansion.
    Revision 1.12  2007/11/03 23:31:20Z  Garyp
    Added the standard module header.
    Revision 1.11  2007/09/05 19:49:26Z  jimmb
    Added VxWorks 6.6 to the diab configuration
    Revision 1.10  2007/01/28 00:45:35Z  Garyp
    Updated to define the ToolSet name.
    Revision 1.9  2007/01/18 18:41:31Z  peterb
    (Per gp) updated to work with VxWorks 6.5.
    Revision 1.8  2006/08/28 02:47:34Z  Garyp
    Added VxWorks 6.4 support.
    Revision 1.7  2006/05/26 20:14:00Z  tonyq
    Updated with VxWorks 6.3 support
    Revision 1.6  2006/05/17 17:32:30Z  Garyp
    Corrected the previous the checkin.
    Revision 1.5  2006/05/06 23:43:49Z  Garyp
    Turned 64-bit support back on (accidently disabled in previous rev).
    Revision 1.4  2006/05/05 03:19:09Z  Garyp
    Modified to define D_(U)INTPTR in fixed 32-bit terms since the (u)intptr_t
    types appear to not be available.
    Revision 1.3  2006/04/26 21:59:50Z  billr
    Support for pointers wider than 32 bits.
    Revision 1.2  2006/01/20 23:23:34Z  brandont
    Added defines for DCL_OS_VERSION.
    Revision 1.1  2005/12/02 16:46:18Z  Pauli
    Initial revision
    Revision 1.3  2005/12/02 16:46:18Z  Pauli
    Merge with 2.0 product line, build 173.
    Revision 1.2  2005/10/21 02:59:04Z  garyp
    Documentation cleanup.
    Revision 1.1  2005/06/20 22:32:02Z  Garyp
    Initial revision
    Revision 1.5  2005/10/03 20:47:36Z  brandont
    Added define for DCL_BIG_ENDIAN based on the _BYTE_ORDER define in the
    VxWorks header files.
    Revision 1.4  2005/06/20 22:32:02Z  PaulI
    Updated to use vxworks types.
    Revision 1.3  2005/06/20 21:05:21Z  Pauli
    Updated 64-bit types.
    Revision 1.2  2005/06/11 04:20:20Z  PaulI
    Enabled DCL types.
    Revision 1.1  2005/03/25 00:09:34Z  GaryP
    Initial revision
---------------------------------------------------------------------------*/

#ifndef DIABVX6X_H_INCLUDED
#define DIABVX6X_H_INCLUDED


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
  #elif (_WRS_VXWORKS_MINOR == 8)
    #define DCL_OS_VERSION   0x0680
  #elif (_WRS_VXWORKS_MINOR == 7)
    #define DCL_OS_VERSION   0x0670
  #elif (_WRS_VXWORKS_MINOR == 6)
    #define DCL_OS_VERSION   0x0660
  #elif (_WRS_VXWORKS_MINOR == 5)
    #define DCL_OS_VERSION   0x0650
  #elif (_WRS_VXWORKS_MINOR == 4)
    #define DCL_OS_VERSION   0x0640
  #elif (_WRS_VXWORKS_MINOR == 3)
    #define DCL_OS_VERSION   0x0630
  #elif (_WRS_VXWORKS_MINOR == 2)
    #define DCL_OS_VERSION   0x0620
  #elif (_WRS_VXWORKS_MINOR == 1)
    #define DCL_OS_VERSION   0x0610
  #elif (_WRS_VXWORKS_MINOR == 0)
    #define DCL_OS_VERSION   0x0600
  #else
    #error "Unknown VxWorks version"
  #endif
#else
  #error "Unknown VxWorks version"
#endif


/*-------------------------------------------------------------------
    Define the ToolSet name.
-------------------------------------------------------------------*/
#define DCL_TOOLSETNAME             "DIABVX6X"


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
#define DCL_NATIVE_64BIT_SUPPORT    TRUE


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



#endif /* DIABVX6X_H_INCLUDED */
