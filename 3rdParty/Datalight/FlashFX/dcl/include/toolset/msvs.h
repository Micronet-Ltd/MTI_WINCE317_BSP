/*---------------------------------------------------------------------------
               ----> DO NOT REMOVE THE FOLLOWING NOTICE <----

                  Copyright (c) 1993 - 2010 Datalight, Inc.
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
    $Log: msvs.h $
    Revision 1.16  2010/01/27 03:21:54Z  glenns
    Fix typo in list of compile warnings to be disabled.
    Revision 1.15  2010/01/19 17:33:22Z  keithg
    Enabled warning of loss of data in type conversions.
    Revision 1.14  2009/11/25 22:34:10Z  keithg
    Corrected Bug 2894, removed obsolete DCLPACK definition.
    Revision 1.13  2009/09/15 01:08:20Z  garyp
    Fixed bug #2845 -- INT64SUFFIX/UINT64SUFFIX is incorrectly defined.
    Revision 1.12  2009/07/02 16:32:23Z  garyp
    Temporarily disabled warning 4701.
    Revision 1.11  2009/07/01 16:59:29Z  keithg
    Increased warning level to include signed/unsigned mismatches and
    potentially uninitialized variables.
    Revision 1.10  2009/05/17 23:43:30Z  keithg
    Added checks around DCL_BIG_ENDIAN define to allow the default
    to be overridden by the toolset.
    Revision 1.9  2009/02/18 19:27:31Z  keithg
    Temporarily removed unused formal parameter from warning list.
    Revision 1.8  2009/02/17 22:29:53Z  keithg
    Enabled more warnings regarding prototype checking, use of undefined
    macros, and unused formal parameters.
    Revision 1.7  2009/01/20 01:37:17Z  keithg
    Backed out previously enabled warnings.
    Revision 1.6  2009/01/18 08:23:34Z  keithg
    Enabled warnings regarding functions without prototypes; differences
    between prototypes and declaration; and differences between scope
    inidicated in the prototype and declaration.
    Revision 1.5  2009/01/05 04:16:38Z  keithg
    Corrected warning number to ignore about formal parameter differences.
    Revision 1.4  2009/01/02 19:14:01Z  keithg
    Quieted a few more warnings to enable builds to pass.
    Revision 1.3  2009/01/01 00:13:16Z  keithg
    Added #pragmas to enable and disable select warnings.
    Revision 1.2  2007/11/03 23:31:20Z  Garyp
    Added the standard module header.
    Revision 1.1  2007/03/18 18:05:56Z  Garyp
    Initial revision
    Adapted from the MSVC6 ToolSet
---------------------------------------------------------------------------*/

#ifndef MSVS_H_INCLUDED
#define MSVS_H_INCLUDED


/*-------------------------------------------------------------------
    Define the ToolSet name.
-------------------------------------------------------------------*/
#define DCL_TOOLSETNAME             "MSVS"


/*-------------------------------------------------------------------
    Specify the byte-order of the target CPU.

    TRUE    Big Endian
    FALSE   Little Endian
-------------------------------------------------------------------*/
#ifndef DCL_BIG_ENDIAN
#define DCL_BIG_ENDIAN              FALSE
#endif

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
#define UINT64SUFFIX( __value )     (__value ## ULL)
#define INT64SUFFIX( __value )      (__value ## LL)
#endif

/*  Integral types large enough to hold a pointer.
*/
typedef D_INT32             D_INTPTR;
typedef D_UINT32            D_UINTPTR;


/*  The following warning modifications assume level 4 is enabled
    Warning level 4 is as high as possible without introducing some
    really useless, but interesting warnings. The list was compiled
    via an audit of the warnings at level 4 and review of the MSVS
    list of disabled warnings in December 2008 at:
        http://msdn.microsoft.com/en-us/library/23k5d385.aspx
*/
#pragma warning(3:4206) /* nonstandard empty translation unit */
#pragma warning(3:4254) /* possible loss of data between bit fields */
#pragma warning(3:4287) /* unsigned compared against signed */
#pragma warning(3:4289) /* auto in for() expression used outside scope */
#pragma warning(3:4305) /* pointer typecase truncation */
#pragma warning(3:4431) /* missing type specifier */
#pragma warning(3:4536) /* type name exceeds meta-data limit */
#pragma warning(3:4545) /* expression evaluates to function w/o arguments */
#pragma warning(3:4546) /* expression evaluates to function w/o arguments */
#pragma warning(3:4547) /* operator before comma has no effect */
#pragma warning(3:4549) /* boolean operator before comma has no effect */
#pragma warning(3:4555) /* statement has no effect */
#pragma warning(3:4028) /* formal parameter different from decl */
#pragma warning(3:4255) /* no function prototype */
#pragma warning(3:4668) /* undefined macros referenced */
#pragma warning(3:4221) /* nonstandard initializer */
#pragma warning(3:4204) /* nonstandard aggregate initializer */
#pragma warning(3:4211) /* nonstandard redefinition (extern to static) */
#pragma warning(3:4189) /* unused initialized local */
#pragma warning(3:4245) /* signed/unsigned mismatch */
#pragma warning(3:4389) /* signed/unsigned mismatch compare */
#pragma warning(3:4242) /* conversion loss of data */

/*  The following list includes those warnings that are either on or
    need to be turned on but which are not yet addressed in the code.
*/
#ifdef D_MAXWARN
#pragma warning(3:4701) /* potentially uninitialized local */
#pragma warning(3:4057) /* initialization differs in indirection */
#pragma warning(3:4242) /* conversion loss of data */
#pragma warning(3:4244) /* conversion from int to ... */
#pragma warning(3:4310) /* truncating a constant */
#pragma warning(3:4548) /* operator before comma has no effect */

#pragma warning(3:4702) /* unreachable code */
#pragma warning(3:4100) /* unreferenced formal parameter */

#else
#pragma warning(disable:4701) /* potentially uninitialized local */
#pragma warning(disable:4057) /* initialization differs in indirection */
#pragma warning(disable:4244) /* conversion from int to ... */
#pragma warning(disable:4310) /* truncating a constant */
#pragma warning(disable:4548) /* operator before comma has no effect */

#pragma warning(disable:4702) /* unreachable code */
#pragma warning(disable:4100) /* unreferenced formal parameter */
#endif

/*  Warnings that must be ignored!
*/

/*  Unforunately, enabling this warning will cause problems compiling
    some code that has conditionals for other compilers, including some
    MS header files.
*/
#pragma warning(disable:4619) /* warn about unknown warning pragmas */

/*  Padding is normal and expected behavior of the compiler.  The
    following warning is a bit interesting for some diagnostic cases
    where packing is necessary and structures are manually aligned.
*/
#pragma warning(disable:4820) /* padding added to structure */

/*  The warning about expressions always false and conditional
    expressions being constant are disabled only because there are a few
    instances where the constructs are very useful.  One such example is
    our debug assert which uses the do { ... } while(0) construct. These
    particular warnings will be better detected with a lint tool.
*/
#pragma warning(disable:4296) /* expression is always false */
#pragma warning(disable:4127) /* conditional expression is constant */


#endif /* MSVS_H_INCLUDED */


