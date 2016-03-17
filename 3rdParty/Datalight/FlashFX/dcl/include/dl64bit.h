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
    $Log: dl64bit.h $
    Revision 1.16  2009/03/02 18:58:53Z  keithg
    Removed unecessary cast of boolean operations to unsigned.
    Revision 1.15  2008/11/13 23:45:11Z  brandont
    Removed Linux specific macro defintions.  Conditioned DclUint64DivUint32
    and DclUint64ModUint32 so that they can be defined in dlosconf.h or dclconf.h.
    Revision 1.14  2008/11/05 12:23:03Z  jimmb
    Added 64 bit unsigned modulo operator
    DclUint64ModUint64
    Revision 1.13  2008/11/04 20:58:11Z  jimmb
    Added DclUint64DivUint64 default define for systems that directly
    support 64 bit divide
    Revision 1.12  2008/10/30 22:34:43Z  jimmb
    Replaced GNU MACRO extensions with the appropriate function
    calls.
    Revision 1.11  2007/11/03 23:31:21Z  Garyp
    Added the standard module header.
    Revision 1.10  2007/05/13 16:30:53Z  garyp
    Added typecasts so we build cleanly with MSVC6.
    Revision 1.9  2007/05/11 02:52:50Z  Garyp
    Fixed a bug in DclUint64LessUint32().  Renamed DclUint64AssignConst()
    to the more correct name DclUint64AssignHiLo().
    Revision 1.8  2007/04/25 23:35:26Z  timothyj
    Added parenthesis around macro parameters to avoid operator precedence
    problems.
    Revision 1.7  2006/12/13 03:44:18Z  Garyp
    Added the DclUint64IsEqualUint32() macro.
    Revision 1.6  2006/10/05 20:23:22Z  Garyp
    Added DclUint32CastUint64() and DclUint64AssignConst().  Deprecated
    the misnamed DclUint64CastUint32().  General cleanup for readability.
    Revision 1.5  2006/08/18 20:32:30Z  Garyp
    Moved the emulated D_UINT64 type declaration to dlenv.h.
    Revision 1.4  2006/07/07 03:51:04Z  Garyp
    Added asserts to the DIV and MOD macros to warn against devide-by-zero.
    Revision 1.3  2006/06/15 22:29:03Z  thomd
    Add code to handle 64 bit div and mod in __KERNEL__ mode
    Revision 1.2  2006/06/01 23:26:50Z  Garyp
    Added DclUint64ModUint32().   Added 64-bit greater and less than comparison
    macros (from Jeremy).
    Revision 1.1  2005/10/02 05:38:58Z  Pauli
    Initial revision
    Revision 1.2  2005/08/03 18:39:48Z  pauli
    Replaced DCL_FuncName with DclFuncName.
    Revision 1.1  2005/07/17 19:32:46Z  pauli
    Initial revision
---------------------------------------------------------------------------*/

#ifndef DL64BIT_H_INCLUDED
#define DL64BIT_H_INCLUDED


#ifndef DCL_NATIVE_64BIT_SUPPORT
#error "DCL_NATIVE_64BIT_SUPPORT not defined"
#endif

#if DCL_NATIVE_64BIT_SUPPORT

#define DclUint32CastUint64(    p64Src)             ((D_UINT32)(*(p64Src)))
#define DclUint64IsEqualUint32( p64Dst, u32)        (*(p64Dst) ==  (u32))
#define DclUint64IsEqualUint64( p64Dst, p64Src)     (*(p64Dst) == *(p64Src))
#define DclUint64LessUint64(    p64Dst, p64Src)     ((*(p64Dst) <  *(p64Src)))
#define DclUint64LessUint32(    p64Dst, u32)        ((*(p64Dst) <   (u32)))
#define DclUint64GreaterUint64( p64Dst, p64Src)     ((*(p64Dst) >  *(p64Src)))
#define DclUint64GreaterUint32( p64Dst, u32)        ((*(p64Dst) >   (u32)))
#define DclUint64AddUint16(     p64Dst, u16)        (*(p64Dst) +=  (u16))
#define DclUint64AddUint32(     p64Dst, u32)        (*(p64Dst) +=  (u32))
#define DclUint64AddUint64(     p64Dst, p64Src)     (*(p64Dst) += *(p64Src))
#define DclUint64SubUint16(     p64Dst, u16)        (*(p64Dst) -=  (u16))
#define DclUint64SubUint32(     p64Dst, u32)        (*(p64Dst) -=  (u32))
#define DclUint64SubUint64(     p64Dst, p64Src)     (*(p64Dst) -= *(p64Src))
#define DclUint64MulUint16(     p64Dst, u16)        (*(p64Dst) *=  (u16))
#define DclUint64MulUint32(     p64Dst, u32)        (*(p64Dst) *=  (u32))
#define DclUint64AssignUint16(  p64Dst, u16)        (*(p64Dst) =   (u16))
#define DclUint64AssignUint32(  p64Dst, u32)        (*(p64Dst) =   (u32))
#define DclUint64AssignUint64(  p64Dst, p64Src)     (*(p64Dst) =  *(p64Src))

#ifndef DclUint64DivUint64
#define DclUint64DivUint64(     p64Dst, p64Src)     (*(p64Dst) / *(p64Src))
#endif

#ifndef DclUint64ModUint64
#define DclUint64ModUint64(     p64Dst, p64Src)     (*(p64Dst) % *(p64Src))
#endif


#define DclUint64AssignHiLo(    p64Dst, hi, lo)                     \
    do                                                              \
    {                                                               \
        *(p64Dst) = (hi);                                           \
        *(p64Dst) <<= 32;                                           \
        *(p64Dst) |= (lo);                                          \
    }                                                               \
    while(FALSE);

#ifndef DclUint64DivUint32
#define DclUint64DivUint32(     p64Dst,  u32 )                  \
    do                                                          \
    {                                                           \
        DclAssert(u32);                                         \
        (*(p64Dst) /= (u32));                                   \
    } while(0)
#endif

#ifndef DclUint64ModUint32
#define DclUint64ModUint32(     p64Dst,  u32 )                  \
    do                                                          \
    {                                                           \
        DclAssert(u32);                                         \
        (*(p64Dst) %= (u32));                                   \
    } while(0)
#endif


#else /* !DCL_NATIVE_64BIT_SUPPORT */

#define DclUint32CastUint64(p64Src) ((p64Src)->ulLowDword)

#define DclUint64IsEqualUint32(p64Dst, u32)                         \
    (((p64Dst)->ulLowDword  == (u32)) &&                            \
     ((p64Dst)->ulHighDword == 0))

#define DclUint64IsEqualUint64(p64Dst, p64Src)                      \
    (((p64Dst)->ulLowDword  == (p64Src)->ulLowDword) &&             \
     ((p64Dst)->ulHighDword == (p64Src)->ulHighDword))

#define DclUint64LessUint64(p64Dst, p64Src)                         \
    (((p64Dst)->ulHighDword < (p64Src)->ulHighDword) ? TRUE :       \
     ((p64Dst)->ulHighDword > (p64Src)->ulHighDword) ? FALSE :      \
     ((p64Dst)->ulLowDword  < (p64Src)->ulLowDword) ? TRUE : FALSE)

#define DclUint64LessUint32(p64Dst, u32)                            \
    (((p64Dst)->ulHighDword > 0) ? FALSE :                          \
    (((p64Dst)->ulLowDword  >= (u32)) ? FALSE : TRUE))

#define DclUint64GreaterUint64(p64Dst, p64Src)                      \
    DclUint64LessUint64(p64Src, p64Dst)

#define DclUint64GreaterUint32(p64Dst, u32)                         \
    (((p64Dst)->ulHighDword > 0) ? TRUE :                           \
     ((p64Dst)->ulLowDword  > (u32)) ? TRUE : FALSE)

#define DclUint64AssignUint16(p64Dst, u16)                          \
    (p64Dst)->ulHighDword = 0;                                      \
    (p64Dst)->ulLowDword  = (u16)

#define DclUint64AssignUint32(p64Dst, u32)                          \
    (p64Dst)->ulHighDword = 0;                                      \
    (p64Dst)->ulLowDword  = (u32)

#define DclUint64AssignUint64(p64Dst, p64Src)                       \
    (p64Dst)->ulHighDword = (p64Src)->ulHighDword;                  \
    (p64Dst)->ulLowDword  = (p64Src)->ulLowDword

#define DclUint64AssignHiLo(p64, h, l)                              \
    (p64)->ulHighDword = (h);                                       \
    (p64)->ulLowDword  = (l);



/*  Function prototypes for the helper functions used to handle
    emulated 64-bit operations.
*/
#define DclUint64AddUint16    DCLFUNC(DclUint64AddUint16)
#define DclUint64AddUint32    DCLFUNC(DclUint64AddUint32)
#define DclUint64AddUint64    DCLFUNC(DclUint64AddUint64)
#define DclUint64SubUint16    DCLFUNC(DclUint64SubUint16)
#define DclUint64SubUint32    DCLFUNC(DclUint64SubUint32)
#define DclUint64SubUint64    DCLFUNC(DclUint64SubUint64)
#define DclUint64MulUint16    DCLFUNC(DclUint64MulUint16)
#define DclUint64MulUint32    DCLFUNC(DclUint64MulUint32)
#define DclUint64DivUint32    DCLFUNC(DclUint64DivUint32)
#define DclUint64ModUint32    DCLFUNC(DclUint64ModUint32)

void DclUint64AddUint16(D_UINT64 *pullValue64, D_UINT16 uValue16);
void DclUint64AddUint32(D_UINT64 *pullValue64, D_UINT32 ulValue32);
void DclUint64AddUint64(D_UINT64 *pullValue1,  D_UINT64 *pullValue2);
void DclUint64SubUint16(D_UINT64 *pullValue64, D_UINT16 uValue16);
void DclUint64SubUint32(D_UINT64 *pullValue64, D_UINT32 ulValue32);
void DclUint64SubUint64(D_UINT64 *pullValue1,  D_UINT64 *pullValue2);
void DclUint64MulUint16(D_UINT64 *pullValue64, D_UINT16 uValue16);
void DclUint64MulUint32(D_UINT64 *pullValue64, D_UINT32 ulValue32);
void DclUint64DivUint32(D_UINT64 *pullValue64, D_UINT32 ulValue32);
void DclUint64ModUint32(D_UINT64 *pullValue64, D_UINT32 ulValue32);


#endif /* #if DCL_NATIVE_64BIT_SUPPORT */


/*  This macro is misnamed, and now deprectated, use
    DclUint32CastUint64() instead!
*/
#define DclUint64CastUint32(    p64Dst)          DclUint32CastUint64(p64Dst)


#endif /* DL64BIT_H_INCLUDED */


