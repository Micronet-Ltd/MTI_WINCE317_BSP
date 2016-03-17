/*---------------------------------------------------------------------------
               ----> DO NOT REMOVE THE FOLLOWING NOTICE <----

                  Copyright (c) 1993 - 2008 Datalight, Inc.
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

    This header defines the API to the Datalight Common Libraries.

    Note: This header is intended to be included as part of dcl.h.
    Do not include this header directly.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dlend.h $
    Revision 1.6  2008/05/29 19:59:35Z  garyp
    Merged from the WinMobile branch.
    Revision 1.5.1.2  2008/05/29 19:59:35Z  garyp
    Modified the data type used in DclCopyLittleEndianToEmulated64() so 
    that calling the function does not require a typecast.
    Revision 1.5  2007/11/03 23:31:21Z  Garyp
    Added the standard module header.
    Revision 1.4  2006/10/05 23:05:00Z  Garyp
    Corrected the macros for dealing with emulated 64-bit types, which were
    broken for both big and little endian modes.  Added some shorthand
    macros.
    Revision 1.3  2006/10/02 05:54:15Z  Garyp
    Prototype updated.  Cleaned up for readability -- no functional changes.
    Revision 1.2  2005/12/27 22:43:35Z  Pauli
    Added DCLFUNC wrappers for byte order routines.
    Added requirement that DCL_BIG_ENDIAN be defined.
    Revision 1.1  2005/12/02 16:30:00Z  Pauli
    Initial revision
---------------------------------------------------------------------------*/

#ifndef DLEND_H_INCLUDED
#define DLEND_H_INCLUDED


#ifndef DCL_NATIVE_64BIT_SUPPORT
#error "DCL_NATIVE_64BIT_SUPPORT not defined"
#endif

#ifndef DCL_BIG_ENDIAN
#error "DCL_BIG_ENDIAN not defined"
#endif


/*-------------------------------------------------------------------
    Prototypes for low level routines used to swap byte order.
-------------------------------------------------------------------*/
#define DclReverseInPlace                       DCLFUNC(DclReverseInPlace)
#define DclCopyAndReverse                       DCLFUNC(DclCopyAndReverse)

void    DclReverseInPlace(void *pData, D_UINT16 uLength);
void    DclCopyAndReverse(void *pDest, const void *pSrc, D_UINT16 uLength);

#if !DCL_NATIVE_64BIT_SUPPORT
  #define DclCopyEmulated64ToLittleEndian       DCLFUNC(DclCopyEmulated64ToLittleEndian)
  #define DclCopyLittleEndianToEmulated64       DCLFUNC(DclCopyLittleEndianToEmulated64)
  #define DclInPlaceEmulated64ToLittleEndian    DCLFUNC(DclInPlaceEmulated64ToLittleEndian)
  #define DclInPlaceLittleEndianToEmulated64    DCLFUNC(DclInPlaceLittleEndianToEmulated64)

  void  DclCopyEmulated64ToLittleEndian(D_BUFFER *pBuff, const D_UINT64 *pullValue64);
  void  DclCopyLittleEndianToEmulated64(void *pDestEmul64, const D_BUFFER *pBuff);
  void  DclInPlaceEmulated64ToLittleEndian(void *pData);
  void  DclInPlaceLittleEndianToEmulated64(void *pData);
#endif


/*-------------------------------------------------------------------
    The following macros translate between native byte ordering
    and the little endian.

    There are only 4 types of macros to perform Endian conversions.
    However, macros for 64-bit are implemented separately.  The
    implementation below seems complex because we may need 64-bit
    emulation.  Here are the 8 macros and the types they should be
    used with:

    DclMakeLittleEndian(ptr, len)
        for use with D_UINT16 and D_UINT32

    DclMakeNativeEndian(ptr, len)
        for use with D_UINT16 and D_UINT32

    DclCopyLittleToNativeEndian(dst, src, len)
        for use with D_UINT16 and D_UINT32

    DclCopyNativeToLittleEndian(dst, src, len)
        for use with D_UINT16 and D_UINT32

    DclMakeLittleEndian64(ptr)
        for use with D_UINT64

    DclMakeNativeEndian64(ptr)
        for use with D_UINT64

    DclCopyLittleToNativeEndian64(dst, src)
        for use with D_UINT64

    DclCopyNativeToLittleEndian64(dst, src)
        for use with D_UINT64

    The DclCopyLittleToNativeEndian64() and its opposite are
    implemented to copy to and from a byte array, as it relates to
    reading or writing the media.  Using these macros to copy from
    one D_UINT64 variable to another D_UINT64 variable could yield
    undesirable results when emulating 64-bits.  Thus, these two
    macros are for low-level core operations only.
-------------------------------------------------------------------*/
#if DCL_BIG_ENDIAN
    #define DclMakeLittleEndian(p, l)               DclReverseInPlace((p), (l))
    #define DclMakeNativeEndian(p, l)               DclReverseInPlace((p), (l))
    #define DclCopyLittleToNativeEndian(d, s, l)    DclCopyAndReverse((d), (s), (l))
    #define DclCopyNativeToLittleEndian(d, s, l)    DclCopyAndReverse((d), (s), (l))

  #if DCL_NATIVE_64BIT_SUPPORT
    #define DclMakeLittleEndian64(p)                DclReverseInPlace((p), sizeof(D_UINT64))
    #define DclMakeNativeEndian64(p)                DclReverseInPlace((p), sizeof(D_UINT64))
    #define DclCopyLittleToNativeEndian64(d, s)     DclCopyAndReverse((d), (s), sizeof(D_UINT64))
    #define DclCopyNativeToLittleEndian64(d, s)     DclCopyAndReverse((d), (s), sizeof(D_UINT64))
  #else
    #define DclMakeLittleEndian64(p)                DclInPlaceEmulated64ToLittleEndian((p))
    #define DclMakeNativeEndian64(p)                DclInPlaceLittleEndianToEmulated64((p))
    #define DclCopyLittleToNativeEndian64(d, p)     DclCopyLittleEndianToEmulated64((d), (p))
    #define DclCopyNativeToLittleEndian64(d, p)     DclCopyEmulated64ToLittleEndian((d), (p))
  #endif

#else

    #define DclMakeLittleEndian(p, l)               ((void) 0)
    #define DclMakeNativeEndian(p, l)               ((void) 0)
    #define DclCopyLittleToNativeEndian(d, s, l)    DclMemCpy((d), (s), (l))
    #define DclCopyNativeToLittleEndian(d, s, l)    DclMemCpy((d), (s), (l))

  #if DCL_NATIVE_64BIT_SUPPORT
    #define DclMakeLittleEndian64(p)                ((void) 0)
    #define DclMakeNativeEndian64(p)                ((void) 0)
    #define DclCopyLittleToNativeEndian64(d, s)     DclMemCpy((d), (s), sizeof(D_UINT64))
    #define DclCopyNativeToLittleEndian64(d, s)     DclMemCpy((d), (s), sizeof(D_UINT64))
  #else
    #define DclMakeLittleEndian64(p)                DclInPlaceEmulated64ToLittleEndian((p))
    #define DclMakeNativeEndian64(p)                DclInPlaceLittleEndianToEmulated64((p))
    #define DclCopyLittleToNativeEndian64(d, p)     DclCopyLittleEndianToEmulated64((d), (p))
    #define DclCopyNativeToLittleEndian64(d, p)     DclCopyEmulated64ToLittleEndian((d), (p))
  #endif
#endif


/*  Shorthand for readability
*/
#define DCLLITTLE(p, l)     DclMakeLittleEndian(p, l)
#define DCLNATIVE(p, l)     DclMakeNativeEndian(p, l)
#define DCLLE2NE(d, s, l)   DclCopyLittleToNativeEndian(d, s, l)
#define DCLNE2LE(d, s, l)   DclCopyNativeToLittleEndian(d, s, l)
#define DCLLITTLE64(p)      DclMakeLittleEndian64(p)
#define DCLNATIVE64(p)      DclMakeNativeEndian64(p)
#define DCLLE2NE64(d, s)    DclCopyLittleToNativeEndian64(d, s)
#define DCLNE2LE64(d, s)    DclCopyNativeToLittleEndian64(d, s)



#endif /* DLEND_H_INCLUDED */

