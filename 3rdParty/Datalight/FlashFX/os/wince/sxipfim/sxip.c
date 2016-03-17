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
  jurisdictions.  The software may be subject to one or more of these US
  patents: US#5860082, US#6260156.  Patents may be pending.

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

    Provides a generic Flash Interface Module (FIM) for software XIP
    (eXecute In Place) on Windows CE.net.

    Code that programs and erases flash must not depend on any code or
    data in flash.  This is achieved in Windows CE by putting that code
    in a separate DLL and loading it into statically mapped RAM with
    LoadIntChainHandler().  Static mapping is important on ARM
    processors (at least), as the way Windows CE 4.0 uses the MMU
    causes frequent data abort (and probably prefetch abort) exceptions
    when accessing virtual addresses that are not statically mapped.

    Calls into the DLL are all achieved using KernelLibIoControl().  This
    is probably not strictly necessary; it was done this way because there
    was a belief that it would cause the DLL to be entered with the stack
    in statically mapped memory.  This turns out not to be the case.


    For SXIP, the flash must be statically mapped, (at least on ARM).
    Code similar to the follow would need to be used in FfxHookDiskCreate():

    pbFlashMedia = CreateStaticMapping(dwFlashAddress >> 8, dwFlashLength);
    if(!pbFlashMedia)
    {
       FFXPRINTF(1, ("CreateStaticMapping() FAILED!\n"));
       return NULL;
    }


    Assumptions:

    - All flash to be accessed is linearly mapped into virtual address
      and is accessible by offsets up to WindowSize() relative to the
      address returned by WindowMap(0).

---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: sxip.c $
    Revision 1.4  2007/11/03 23:50:12Z  Garyp
    Updated to use the standard module header.
    Revision 1.3  2006/10/04 00:14:38Z  Garyp
    Updated to use the new style printf macros and functions.
    Revision 1.2  2006/02/07 03:35:55Z  Garyp
    Header updates.
    Revision 1.1  2005/10/06 05:49:34Z  Pauli
    Initial revision
    Revision 1.2  2005/08/03 19:34:20Z  pauli
    Replaced DCL_FuncName with DclFuncName.
    Revision 1.1  2005/05/03 01:31:12Z  pauli
    Initial revision
    Revision 1.5  2005/05/03 01:31:12Z  garyp
    Removed references to some obsolete fields.
    Revision 1.4  2004/12/30 23:48:17Z  GaryP
    Updated to use indents of 4, hard tabs of 8 (if used), and a standard
    comment style.
    Revision 1.3  2004/08/23 19:26:15Z  GaryP
    Added documentation about using CreateStaticMapping().
    Revision 1.2  2003/04/21 02:40:40Z  garyp
    Modified to load FIM.H from the FIM directory.
    Revision 1.1  2002/11/14 00:31:06Z  qa
    Initial revision
---------------------------------------------------------------------------*/

#include <windows.h>

#include <flashfx.h>

#if defined __SXIP

#include <sxip.h>
#include "..\..\..\fmsl\fim\fim.h"


/*  An integer type that can hold a pointer (as would be found in stdint.h).
*/
typedef D_UINT32 uintptr_t;

/*  prototypes
*/
static D_BOOL SXIPMount(
    void);
static D_BOOL SXIPWrite(
    D_UINT32,
    D_UINT16,
    void *);
static D_BOOL SXIPErase(
    D_UINT32,
    D_UINT32);
static D_BOOL EraseBlock(
    void *);
static D_BOOL WriteBytes(
    void *,
    void *,
    D_UINT32);

Device          SXIP = { SXIPMount, ROMUnMount, ROMRead, FailRead,
    SXIPWrite, FailWrite, SXIPErase
};

/*  Handle for the SXIP "installable ISR" kernel DLL.
*/
static HANDLE   hSxipDll = NULL;


/*------------------------------------------------------------------------
    Parameters:
       nothing
    Notes:
       Determines if the media in the socket is indeed supported.
       If so the MediaInfo structure is updated to reflect it.

       Chips that are a pair in which both chips are the same and are
       on the list of supported manufacturer/device IDs are accepted.
       The first pair must begin at offset 0.  Multiple pairs are
       detected and accepted if they are contiguous.  Address aliasing
       is detected.

    Returns:
       TRUE - If the media is supported; the full structure pointed to
              by lpThisMedia is updated.
       FALSE - If the media is NOT supported; the structure pointed to
               by lpThisMedia is not modified.
------------------------------------------------------------------------*/
D_BOOL SXIPMount(
    void)
{
    PROBE_INPUT     probeInput;
    D_BOOL          found;

    DclProductionAssert(lpThisMedia != NULL);

    /*  If the kernel DLL for SXIP flash access hasn't been loaded yet,
        try to load it.
    */

    if(hSxipDll == NULL)
    {
        hSxipDll = LoadIntChainHandler(TEXT("fxsxip.dll"), NULL, 0);

        /*  Note: documentation shipped with CE.net 4.0 says the failure
            return is INVALID_HANDLEVALUE (which is actually spelled
            INVALID_HANDLE_VALUE).  MSDN says, and experience confirms,
            the failure return is NULL.
        */

        if(hSxipDll == NULL)
        {
            RETAILMSG(1,
                      (TEXT("Failed to load DLL: error 0x%08lx\r\n"),
                       GetLastError()));
            return FALSE;
        }
    }

    probeInput.pFlashBase = WindowMap(0);
    probeInput.windowSize = WindowSize();
    found = KernelLibIoControl(hSxipDll, KERNEL_IOCTL_SXIP_PROBE,
                               &probeInput, sizeof probeInput,
                               lpThisMedia, sizeof *lpThisMedia, NULL);
    if(found)
    {
        RETAILMSG(1, (TEXT("Found size %lu (0x%08lx)\r\n"),
                      (unsigned long)lpThisMedia->ulTotalSize,
                      (unsigned long)lpThisMedia->ulTotalSize));
    }
    else
    {
        RETAILMSG(1,
                  (TEXT("Mount: probe failed, error 0x%08lx\r\n"),
                   GetLastError()));
    }

    return found;
}




/*------------------------------------------------------------------------
    Parameters:
       ulStart - Starting offset in bytes to begin the access
       uLength - Number of bytes to transfer
       lpBuffer - Pointer to client supplied transfer area
    Notes:
       Writes a given number of bytes of data out to the media.
       It does not return until the data is programmed and verified.
    Returns:
       TRUE - If the access was successful
       FALSE - If the access failed
------------------------------------------------------------------------*/
D_BOOL SXIPWrite(
    D_UINT32 ulStart,
    D_UINT16 uLength,
    void * lpBuffer)
{
    D_UINT32       ulLength;
    char           *lpData;

    /*  FIM API requires addresses to be aligned to 32 bits, and length
        to be a multiple of 32 bits.
    */

    DclProductionAssert(ulStart % sizeof(D_UINT32) == 0);
    DclProductionAssert(uLength % sizeof(D_UINT32) == 0);
    DclProductionAssert((uintptr_t) lpBuffer % sizeof(D_UINT32) == 0);

    DclProductionAssert(lpThisMedia->ulWindowSize != 0);
    DclProductionAssert(lpThisMedia->ulWindowSize % sizeof(D_UINT32) == 0);

    /*  Verify user address and length parameters within the media
        boundaries.
    */
    DclAssert(ulStart <= lpThisMedia->ulTotalSize);
    DclAssert(uLength <= lpThisMedia->ulTotalSize - ulStart);

    /*  Buffer pointer should not be NULL.  FIMs traditionally don't
        accept a zero length, although this is not specified anywhere in
        the API documentation and it could be handled correctly.
    */

    DclAssert(lpBuffer != NULL);
    DclAssert(uLength);

    /*  Fail if parameters were bad (if DclAssert was compiled out).
    */

    if(ulStart > lpThisMedia->ulTotalSize
       || uLength > lpThisMedia->ulTotalSize - ulStart
       || lpBuffer == NULL || uLength == 0)
    {
        return FALSE;
    }

    lpData = lpBuffer;
    ulLength = uLength;
    while(ulLength)
    {
        WRITE_INPUT     writeInput;
        D_UINT32       written;
        BOOL            status;

        writeInput.pFlash = WindowMap(ulStart);
        writeInput.pData = lpData;
        writeInput.length = ulLength;
        status = KernelLibIoControl(hSxipDll, KERNEL_IOCTL_SXIP_WRITE,
                                    &writeInput, sizeof writeInput,
                                    &written, sizeof written, NULL);
        if(!status)
        {
            /*  Unlocking locked blocks is not supported.  Just fail.
            */
            break;
        }

        ulStart += written;
        lpData += written;
        ulLength -= written;
    }

    /*  The write succeeded if all bytes were transferred.
    */

    return (ulLength == 0);
}




/*------------------------------------------------------------------------
    Parameters:
       ulStart - Starting offset in bytes to begin the erase
       ulLength - Number of bytes to erase
    Notes:
       Erases the given number of bytes in the media from the
       specified offset.  It does not return until the zones
       are fully erased.
    Returns:
       TRUE - If the erase was successful
       FALSE - If the erase failed
------------------------------------------------------------------------*/
static D_BOOL SXIPErase(
    D_UINT32 ulStart,
    D_UINT32 ulLength)
{
    /*  erase must start on an erase zone boundary
    */

    DclProductionAssert(lpThisMedia->ulEraseZoneSize != 0);
    DclProductionAssert(ulStart % lpThisMedia->ulEraseZoneSize == 0);

    /*  erase length must be nonzero and a multiple of erase zone size
    */

    DclProductionAssert(ulLength != 0);
    DclProductionAssert(ulLength % lpThisMedia->ulEraseZoneSize == 0);

    /*  Verify user address and length parameters within the media
        boundaries.
    */
    DclAssert(ulStart <= lpThisMedia->ulTotalSize);
    DclAssert(ulLength <= lpThisMedia->ulTotalSize - ulStart);

    /*  Fail if parameters were bad (if DclAssert was compiled out).
    */

    if(ulStart > lpThisMedia->ulTotalSize
       || ulLength > lpThisMedia->ulTotalSize - ulStart)
    {
        return FALSE;
    }

    /*  Erase as many blocks as necessary.  If a failure occurs erasing
        any single block, give up at that point and return a failure
        status for the whole erase operation.
    */

    while(ulLength)
    {
        ERASE_INPUT     eraseInput;
        D_BOOL          status;


        eraseInput.pFlash = WindowMap(ulStart);
        status = KernelLibIoControl(hSxipDll, KERNEL_IOCTL_SXIP_ERASE,
                                    &eraseInput, sizeof eraseInput,
                                    NULL, 0, NULL);
        if(!status)
        {
            /*  Unlocking locked blocks is not supported.  Just fail.
            */
            break;
        }

        ulLength -= lpThisMedia->ulEraseZoneSize;
        ulStart += lpThisMedia->ulEraseZoneSize;
    }                           /* end while (ulLength) */

    /*  If all blocks were erased, the whole erase was successful.
    */

    return (ulLength == 0);
}



#endif /* ifdef __SXIP */
