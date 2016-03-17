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
  jurisdictions. 

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

  Notwithstanding the foregoing, Licensee acknowledges that the software may
  be distributed as part of a package containing, and/or in conjunction with
  other source code files, licensed under so-called "open source" software 
  licenses.  If so, the following license will apply in lieu of the terms set
  forth above:

  Redistribution and use of this software in source and binary forms, with or
  without modification, are permitted provided that the following conditions
  are met:
  1. Redistributions of source code must retain the above copyright notice,
     this list of conditions, and the following disclaimer.
  2. Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions, and the following disclaimer in the
     documentation and/or other materials provided with the distribution.
  
  THIS SOFTWARE IS PROVIDED BY DATALIGHT "AS IS" AND ANY EXPRESS OR IMPLIED
  WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MER-
  CHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, OR NON-INFRINGEMENT, ARE
  DISCLAIMED.  IN NO EVENT SHALL DATALIGHT BE LIABLE FOR ANY DIRECT, INDIRECT,
  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
  OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEG-
  LIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Description

    This header contains ToolSet specific data type definitions for a
    generic GNU C compiler environment.  It should work with any gcc
    version 3 or later.

    It depends only on the predefined macros provided by the compiler.
    These may be seen by invoking the compiler with the -dM switch,
    for example:

        echo | gcc -E -dM -
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: gcc3x.h $
    Revision 1.17  2010/02/23 23:58:19Z  billr
    Support PowerPC big-endian.
    Revision 1.16  2010/02/13 20:37:42Z  garyp
    Updated to use the correct license wording.
    Revision 1.15  2010/02/10 23:38:54Z  billr
    Changed the headers to reflect the shared or public license. [jimmb]
    Revision 1.14  2009/12/19 01:57:29Z  billr
    Refactor headers for better separation between 0S-dependent and
    OS-independent code.
    Revision 1.13  2009/12/02 17:54:45Z  billr
    Correct comment, no functional changes.
    Revision 1.12  2009/11/25 22:33:22Z  keithg
    Corrected Bug 2894, removed obsolete DCLPACK definition.
    Revision 1.11  2009/11/03 00:09:27Z  billr
    Define DCL standard types and byte order based on gcc predefined macros.
    Revision 1.10  2007/11/03 23:31:20Z  Garyp
    Added the standard module header.
    Revision 1.9  2007/08/02 17:38:03Z  jimmb
    Modified to enable Linux User mode as well as the Kernel mode
    Revision 1.8  2007/01/28 00:45:35Z  Garyp
    Updated to define the ToolSet name.
    Revision 1.7  2006/09/18 19:30:28Z  billr
    64-bit types should always be supported.
    Revision 1.6  2006/06/16 20:25:54Z  thomd
    Always compile linux kernel in 64 bit mode
    Revision 1.5  2006/06/08 20:31:26Z  jeremys
    Added checks to enable or disable native 64-bit support depending on
    native type size.
    Revision 1.4  2006/06/06 18:58:46Z  jeremys
    Disabled native 64 bit support for gcc3 platforms.
    Revision 1.3  2006/03/01 01:04:58Z  billr
    Get byte order and word size from OS build environment instead of toolset.
    Revision 1.2  2006/01/07 01:42:30Z  billr
    Toolset now must specify byte order.
    Revision 1.1  2005/10/21 01:59:04Z  Pauli
    Initial revision
    Revision 1.2  2005/10/21 02:59:04Z  garyp
    Documentation cleanup.
    Revision 1.1  2005/06/21 21:30:28Z  Garyp
    Initial revision
    ---------------------
    Bill Roman 2005-06-21
---------------------------------------------------------------------------*/

#ifndef DCL_GCC3X_H_INCLUDED
#define DCL_GCC3X_H_INCLUDED

#ifndef __GNUC__
#error Wrong toolset!  This requires gcc.
#endif

/*-------------------------------------------------------------------
    Define the ToolSet name.
-------------------------------------------------------------------*/
#define DCL_TOOLSETNAME             "GCC3X"


/*-------------------------------------------------------------------
    Define whether 64-bit types are natively supported or not.
-------------------------------------------------------------------*/

#define DCL_NATIVE_64BIT_SUPPORT TRUE


/*-------------------------------------------------------------------
    Define the basic fixed size data types, upon which the rest of
    the data types in the product are based.
-------------------------------------------------------------------*/

/*  8-bit integral types.  These depend on char being 8 bits.
*/
#if __CHAR_BIT__ == 8

typedef signed char         D_INT8;         /* must be exactly 8 bits wide  */
typedef unsigned char       D_UINT8;        /* must be exactly 8 bits wide  */

#else
#error No 8-bit integral type.  What is this, a PDP-10?
#endif


/*  16-bit integral types.  short must be no fewer than 16 bits, if it's
    more, there's trouble.
*/
#if __SHRT_MAX__ == 32767

typedef short               D_INT16;        /* must be exactly 16 bits wide */
typedef unsigned short      D_UINT16;       /* must be exactly 16 bits wide */

#else
#error No 16-bit integral type.  It appears that neither short int nor int is 16 bits.
#endif

#if __INT_MAX__ == 2147483647

typedef int                 D_INT32;        /* must be exactly 32 bits wide */
typedef unsigned int        D_UINT32;       /* must be exactly 32 bits wide */

#elif __LONG_MAX__ == 2147483647

typedef long                D_INT32;        /* must be exactly 32 bits wide */
typedef unsigned long       D_UINT32;       /* must be exactly 32 bits wide */

#else
#error No 32-bit integral type.  It appears that neither int nor long is 32 bits.
#endif

#if __LONG_MAX__ == 9223372036854775807

typedef long                D_INT64;        /* must be exactly 64 bits wide */
typedef unsigned long       D_UINT64;       /* must be exactly 64 bits wide */

#define UINT64SUFFIX(v)     (v ## UL)
#define INT64SUFFIX(v)      (v ## L)

#elif defined(__LONG_LONG_MAX__) && __LONG_LONG_MAX__ == 9223372036854775807

__extension__
typedef long long           D_INT64;        /* must be exactly 64 bits wide */
__extension__
typedef unsigned long long  D_UINT64;       /* must be exactly 64 bits wide */

#define UINT64SUFFIX(v)     (v ## ULL)
#define INT64SUFFIX(v)      (v ## LL)

#else
#error No 64-bit integral type.  It appears that neither long nor long long is 64 bits.
#endif

/*  An integral type that can hold a pointer.  Note that this only
    works for "reasonable" architectures (those with nice, flat
    address spaces, segmented architectures need not apply).
*/
typedef __PTRDIFF_TYPE__             D_INTPTR;
typedef unsigned __PTRDIFF_TYPE__    D_UINTPTR;


/*  Identify byte order.  Unfortunately, gcc doesn't seem to provide
    a consistent way to do this that works for all CPU types.
*/
#if defined(__amd64__)
#define DCL_CPU_ALIGNSIZE (8) /* 4 might work, at lower performance */
#define DCL_BIG_ENDIAN FALSE

#elif defined(__i386__)
#define DCL_CPU_ALIGNSIZE (4)
#define DCL_BIG_ENDIAN FALSE

#elif defined(__arm__)
#define DCL_CPU_ALIGNSIZE (4)
#if defined(__ARMEB__)
#define DCL_BIG_ENDIAN TRUE
#elif defined(__ARMEL__)
#define DCL_BIG_ENDIAN FALSE
#else
#error Cannot determine ARM processor byte order.  Please contact support@datalight.com.
#endif

#elif defined(__mips__)
#define DCL_CPU_ALIGNSIZE (4)
#if defined(__MIPSEB__)
#define DCL_BIG_ENDIAN TRUE
#elif defined(__MIPSEL__)
#define DCL_BIG_ENDIAN FALSE
#else
#error Cannot determine MIPS processor byte order.  Please contact support@datalight.com.
#endif

#elif defined(__PPC__)
#define DCL_CPU_ALIGNSIZE (4)
#if defined(__BIG_ENDIAN__) && __BIG_ENDIAN__
#define DCL_BIG_ENDIAN TRUE
#else
#error Cannot determine PPC processor byte order.  Please contact support@datalight.com.
#endif

#else
#error Please contact support@datalight.com for help with your compiler.

#endif /* various processor types */

#endif /* DCL_GCC3X_H_INCLUDED */
