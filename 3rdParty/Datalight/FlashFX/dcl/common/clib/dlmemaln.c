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

    The functions in this module are used internally by Datalight products.
    Typically these functions are used rather than the similarly named,
    general Standard C library functions.  Typically this is done for
    compatibility, portability, and code size reasons.

    These functions are NOT intended to be complete, 100% ANSI C compatible
    implementations, but rather are designed address specific needs.

    See the specific function headers for more information about departures
    from the ANSI standard, and missing or extended functionality.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dlmemaln.c $
    Revision 1.7  2009/06/24 21:40:34Z  garyp
    Updated to use new asserts.
    Revision 1.6  2009/04/09 14:13:18Z  davidh
    Function headers updated for AutoDoc.
    Revision 1.5  2007/12/18 03:32:57Z  brandont
    Updated function headers.
    Revision 1.4  2007/11/03 23:31:09Z  Garyp
    Added the standard module header.
    Revision 1.3  2006/05/04 22:45:08Z  Garyp
    Fixed DclMemSetAligned() to work properly (was not in use anywhere).
    Revision 1.2  2006/04/26 20:31:13Z  billr
    Support for pointers wider than 32 bits.
    Revision 1.1  2005/10/07 02:45:26Z  Pauli
    Initial revision
    Revision 1.2  2005/10/07 03:45:25Z  Garyp
    Reverted back to using size_t types now that we're properly including
    stddef.h elsewhere.
    Revision 1.1  2005/10/05 23:57:12Z  Garyp
    Initial revision
    Revision 1.5  2005/08/04 04:47:52Z  Garyp
    Modified so that the DCL functions compile with their default names even
    if the OS or project code has redefined the macros to use an alternate
    implementation.
    Revision 1.4  2005/08/03 19:17:48Z  pauli
    Replaced DCL_FuncName with DclFuncName.
    Revision 1.2  2005/07/31 03:12:30Z  Garyp
    Added DclMemSetAligned() -- not yet tested.
    Revision 1.1  2005/07/25 21:11:08Z  pauli
    Initial revision
---------------------------------------------------------------------------*/

#include <dcl.h>

/*  Redefine this to the default value just in case it was defined
    in dlosconf.h.  If that is the case, this function will still
    compile as expected, it just won't be called.
*/
#undef  DclMemCpyAligned
#define DclMemCpyAligned     DCLFUNC(DclMemCpyAligned)
#undef  DclMemSetAligned
#define DclMemSetAligned     DCLFUNC(DclMemSetAligned)


/*-------------------------------------------------------------------
    Public: DclMemCpyAligned()

    Copy an aligned memory buffer from one location to another.
    This is similar to the standard C memcpy() function.

    Note that not only must the source and destination buffers
    be aligned on DCL_ALIGNTYPE boundaries, but the nLen
    parameter as well must be evenly divisible by DCL_ALIGNSIZE.

    Parameters:
        pDst    - A pointer to the destination buffer
        pSrc    - A pointer to the source buffer
        nLen    - The length to copy

    Return Value:
        Returns the original destination pointer.
-------------------------------------------------------------------*/
void * DclMemCpyAligned(
    void                   *pDst,
    const void             *pSrc,
    size_t                  nLen)
{
    DCL_ALIGNTYPE          *paDst = (DCL_ALIGNTYPE*)pDst;
    const DCL_ALIGNTYPE    *paSrc = (const DCL_ALIGNTYPE*)pSrc;

    DclProfilerEnter("DclMemCpyAligned", DCLPROF_TOPLEVEL, nLen);

    DclAssertWritePtr(pDst, nLen);
    DclAssertReadPtr(pSrc, nLen);
    DclAssert(((D_UINTPTR) pDst & (DCL_ALIGNSIZE-1)) == 0);
    DclAssert(((D_UINTPTR) pSrc & (DCL_ALIGNSIZE-1)) == 0);
    DclAssert((nLen & (DCL_ALIGNSIZE-1)) == 0);

    /*  copy in chunks, since we can't copy all at once
    */
    while(nLen)
    {
        *paDst = *paSrc;
        paDst++;
        paSrc++;
        nLen -= DCL_ALIGNSIZE;
    }

    DclProfilerLeave(0UL);

    return pDst;
}


/*-------------------------------------------------------------------
    Public: DclMemSetAligned()

    Initialize an aligned memory buffer.
    This is similar to the standard C memset() function.

    Note that not only must the destination buffer be aligned on
    DCL_ALIGNTYPE boundaries, but the nLen parameter as well must
    be evenly divisible by DCL_ALIGNSIZE.

    Parameters:
        pDst    - A pointer to the destination buffer
        pSrc    - A pointer to the source buffer
        nLen    - The length to copy

    Return Value:
        Returns the original destination pointer.
-------------------------------------------------------------------*/

/*  There must be a more efficient way of doing this...
*/
#define SETCHARPOS(chr, pos) ( ( ((DCL_ALIGNTYPE)(chr)) & 0xFF) << (pos*8) )

#if DCL_ALIGNSIZE == 1
#define MAKEINITVAL(chr)  (SETCHARPOS(chr, 0))
#elif DCL_ALIGNSIZE == 2
#define MAKEINITVAL(chr)  (SETCHARPOS(chr, 0) | SETCHARPOS(chr, 1))
#elif DCL_ALIGNSIZE == 4
#define MAKEINITVAL(chr)  (SETCHARPOS(chr, 0) | SETCHARPOS(chr, 1) |  \
                           SETCHARPOS(chr, 2) | SETCHARPOS(chr, 3))
#elif DCL_ALIGNSIZE == 8
#define MAKEINITVAL(chr)  (SETCHARPOS(chr, 0) | SETCHARPOS(chr, 1) |  \
                           SETCHARPOS(chr, 2) | SETCHARPOS(chr, 3) |  \
                           SETCHARPOS(chr, 4) | SETCHARPOS(chr, 5) |  \
                           SETCHARPOS(chr, 6) | SETCHARPOS(chr, 7))
#else
#error "DCL: Unsupported DCL_ALIGNTYPE width"
#endif

void * DclMemSetAligned(
    void           *pDst,
    int             chr,
    size_t          nLen)
{
    DCL_ALIGNTYPE  *paDst = (DCL_ALIGNTYPE*)pDst;
    DCL_ALIGNTYPE   initValue = 0;

    DclProfilerEnter("DclMemSetAligned", DCLPROF_TOPLEVEL, nLen);

    DclAssertWritePtr(pDst, nLen);
    DclAssert(((D_UINTPTR) pDst & (DCL_ALIGNSIZE-1)) == 0);
    DclAssert((nLen & (DCL_ALIGNSIZE-1)) == 0);

    if(chr & 0xFF)
        initValue = MAKEINITVAL(chr);

    while(nLen)
    {
        *paDst = initValue;
        paDst++;
        nLen -= DCL_ALIGNSIZE;
    }

    DclProfilerLeave(0UL);

    return pDst;
}

