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
    $Log: msvc6.h $
    Revision 1.6  2009/11/25 22:37:45Z  keithg
    Corrected Bug 2894, removed obsolete DCLPACK definition.
    Revision 1.5  2007/11/03 23:31:20Z  Garyp
    Added the standard module header.
    Revision 1.4  2007/01/28 00:45:37Z  Garyp
    Updated to define the ToolSet name.
    Revision 1.3  2006/08/18 20:30:20Z  Garyp
    Documentation cleanup.
    Revision 1.2  2006/04/26 22:53:59Z  billr
    Support for pointers wider than 32 bits.
    Revision 1.1  2005/12/02 16:45:22Z  Pauli
    Initial revision
    Revision 1.4  2005/12/02 16:45:22Z  Pauli
    Merge with 2.0 product line, build 173.
    Revision 1.3  2005/11/09 18:50:13Z  Garyp
    Modified INT/UINT64SUFFIX to resolve to do nothing.
    Revision 1.2  2005/10/21 02:59:01Z  garyp
    Updated to ensure that D_INT8 is really a signed value.
    Revision 1.1  2005/06/20 21:00:20Z  Garyp
    Initial revision
    Revision 1.6  2005/09/22 00:49:12Z  brandont
    Added define for DCL_BIG_ENDIAN.
    Revision 1.5  2005/06/20 21:00:19Z  Pauli
    Updated 64-bit types.
    Revision 1.4  2005/06/14 18:13:31Z  PaulI
    Updated 64-bit type definitions.
    Revision 1.3  2005/04/09 23:27:33Z  PaulI
    XP Merge
    Revision 1.2.1.2  2005/04/09 23:27:33Z  garyp
    Enabled the new standard data types.
    Revision 1.2  2005/03/24 23:09:33Z  GaryP
    Added the DCLPACK directive.  Commented out the as-of-yet unused types.
    Revision 1.1  2005/02/26 02:13:22Z  GaryP
    Initial revision
---------------------------------------------------------------------------*/

#ifndef MSVC6_H_INCLUDED
#define MSVC6_H_INCLUDED


/*-------------------------------------------------------------------
    Define the ToolSet name.
-------------------------------------------------------------------*/
#define DCL_TOOLSETNAME             "MSVC6"


/*-------------------------------------------------------------------
    Specify the byte-order of the target CPU.

    TRUE    Big Endian
    FALSE   Little Endian
-------------------------------------------------------------------*/
#define DCL_BIG_ENDIAN              FALSE


/*-------------------------------------------------------------------
    Define whether 64-bit types are natively supported or not.
-------------------------------------------------------------------*/
#define DCL_NATIVE_64BIT_SUPPORT    TRUE


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
typedef __int64             D_INT64;        /* must be exactly 64 bits wide */
typedef unsigned __int64    D_UINT64;       /* must be exactly 64 bits wide */

/*  Compiler specific suffixes for 64-bit literals
*/
#define UINT64SUFFIX( __value )     (__value)
#define INT64SUFFIX( __value )      (__value)
#endif

/*  Integral types large enough to hold a pointer.
*/
typedef D_INT32             D_INTPTR;
typedef D_UINT32            D_UINTPTR;


#endif /* MSVC6_H_INCLUDED */
