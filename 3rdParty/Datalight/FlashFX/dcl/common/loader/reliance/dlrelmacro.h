/*---------------------------------------------------------------------------
               ----> DO NOT REMOVE THE FOLLOWING NOTICE <----

                  Copyright (c) 1993 - 2007 Datalight, Inc.
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

    These are macros used by the Reliance Reader.
---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------
                                Revision History
    $Log: dlrelmacro.h $
    Revision 1.3  2007/11/03 23:31:11Z  Garyp
    Added the standard module header.
    Revision 1.2  2007/10/05 17:00:15Z  pauli
    Updated #error message to be more useful and accurate.
    Revision 1.1  2007/09/27 01:22:12Z  jeremys
    Initial revision
---------------------------------------------------------------------------*/

#ifndef TFSMACRO_H_INCLUDED
#define TFSMACRO_H_INCLUDED     1


/*  -------------------------------------------------------------------------
    The following macros handle unicode/ascii text strings
*/
#ifndef TFS_UNICODE
#error "TFS_UNICODE not defined before including dlrelmacro.h"
#endif

#if TFS_UNICODE

/*  unicode characters
*/
#define TfsStr(sz)   _T(sz)

#else

/*  ascii characters
*/
#define TfsStr(sz)   sz

#endif


/*  -------------------------------------------------------------------------
    The following macros are used to set common data format (CDF) field and
    retrieve CDF fields.
*/

#ifndef DCL_NATIVE_64BIT_SUPPORT
#error "DCL_NATIVE_64BIT_SUPPORT not defined before dlrelmacro.h"
#endif


/*  COPYFROMCDF

    This macro is used to copy a value from a CDF buffer.  The buffer
    pointer (__CDFsrc) will be updated for convience of any subsequent
    calls.  The size of the field is automatically determined by the
    sizeof __dst argument.

    Typical use: Copying from a buffer to an in-memory structure.
*/
#define COPYFROMCDF( __CDFsrc, __dst ) \
   DCLLE2NE( __dst, __CDFsrc, sizeof(*(__dst)) );  \
   __CDFsrc = DclPtrAddByte( __CDFsrc, sizeof(*(__dst)) )


#if DCL_NATIVE_64BIT_SUPPORT

/*  COPYFROMCDF64

    This macro is used to copy a 64-bit value from a CDF buffer.  The buffer
    pointer (__CDFsrc) will be updated for convience of any subsequent calls.

    This macro uses DCL_NATIVE_64BIT_SUPPORT to determine how to copy 64-bit
    values around.

    Typical use: Copying from a buffer to an in-memory structure.
*/
#define COPYFROMCDF64( __CDFsrc, __dst )  \
   DCLLE2NE64( __dst, __CDFsrc );   \
   __CDFsrc = DclPtrAddByte( __CDFsrc, 8 )
#else

/*  COPYFROMCDF64

    This macro is used to copy a 64-bit value from a CDF buffer.  The buffer
    pointer (__CDFsrc) will be updated for convience of any subsequent calls.

    This macro uses DCL_NATIVE_64BIT_SUPPORT to determine how to copy 64-bit
    values around.

    Typical use: Copying from a buffer to an in-memory structure.
*/
#define COPYFROMCDF64( __CDFsrc, __dst )  \
   DCLLE2NE( &(__dst)->ulLowDword, __CDFsrc, sizeof((__dst)->ulLowDword) );   \
   __CDFsrc = DclPtrAddByte( __CDFsrc, sizeof((__dst)->ulLowDword) );   \
   DCLLE2NE( &(__dst)->ulHighDword, __CDFsrc, sizeof((__dst)->ulHighDword) ); \
   __CDFsrc = DclPtrAddByte( __CDFsrc, sizeof((__dst)->ulHighDword) )
#endif


/*  COPYTOCDF

    This macro is used to copy a value to a CDF buffer.  The buffer
    pointer (__CDFdst) will be updated for convience of any subsequent calls.
    The size of the field is automatically determined by the sizeof __src
    argument.

    Typical use: Copying an in-memory structure to a buffer in preparation
    for writing the buffer to disk.
*/
#define COPYTOCDF( __CDFdst, __src )   \
   DCLNE2LE( __CDFdst, __src, sizeof(*(__src)) );  \
   __CDFdst = DclPtrAddByte( __CDFdst, sizeof(*(__src)) )


#if DCL_NATIVE_64BIT_SUPPORT

/*  COPYTOCDF64

    This macro is used to copy a 64-bit value to a CDF buffer.  The buffer
    pointer (__CDFdst) will be updated for convience of any subsequent calls.

    This macro uses DCL_NATIVE_64BIT_SUPPORT to determine how to copy 64-bit
    values around.

    Typical use: Copying an in-memory structure to a buffer in preparation
    for writing the buffer to disk.
*/
#define COPYTOCDF64( __CDFdst, __src ) \
   DCLNE2LE64( __CDFdst, __src );   \
   __CDFdst = DclPtrAddByte( __CDFdst, 8 )
#else

/*  COPYTOCDF64

    This macro is used to copy a 64-bit value to a CDF buffer.  The buffer
    pointer (__CDFdst) will be updated for convience of any subsequent calls.

    This macro uses DCL_NATIVE_64BIT_SUPPORT to determine how to copy 64-bit
    values around.

    Typical use: Copying an in-memory structure to a buffer in preparation
    for writing the buffer to disk.
*/
#define COPYTOCDF64( __CDFdst, __src ) \
   DCLNE2LE( __CDFdst, &(__src)->ulLowDword, sizeof((__src)->ulLowDword) );   \
   __CDFdst = DclPtrAddByte( __CDFdst, sizeof((__src)->ulLowDword) );   \
   DCLNE2LE( __CDFdst, &(__src)->ulHighDword, sizeof((__src)->ulHighDword) ); \
   __CDFdst = DclPtrAddByte( __CDFdst, sizeof((__src)->ulHighDword) )
#endif


/*  COPYFROMCDFTOTEMP

    This macro is used to copy a value from a a CDF buffer.  Unlike the
    COPYFROMCDF macro, this macro takes a third argument which it uses to
    compute the size of the field.  Debug asserts are included in the macro
    for validating the size of the destination against the sizeof the third
    argument.  The buffer pointer (__CDFsrc) will be updated for convience
    of any subsequent calls.

    Typical use: Retrieving the value from within a structure in a buffer.
*/
#define COPYFROMCDFTOTEMP( __CDFsrc, __dst, __sizeofvar )   \
   DclAssert( sizeof(*(__dst)) == (__sizeofvar) ); \
   DCLLE2NE( __dst, __CDFsrc, (__sizeofvar) );  \
   __CDFsrc = DclPtrAddByte( __CDFsrc, (__sizeofvar) )


#endif /* #ifndef TFSMACRO_H_INCLUDED */
