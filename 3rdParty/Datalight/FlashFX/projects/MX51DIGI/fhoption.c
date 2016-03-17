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

    This is the project hook for obtaining project-specific configuration
    information for a drive/device.

    To customize this for a particular project, copy this file to the
    project directory and add any additional parameter processing the
    project requires.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: fhoption.c $
    Revision 1.1  2010/10/23 01:11:58Z  jimmb
    Initial revision
    Revision 1.5  2009/07/17 19:39:58Z  garyp
    Merged from the v4.0 branch.  Eliminated use of an obsolete header.
    Revision 1.4  2009/03/23 21:08:57Z  keithg
    Fixed bug 2509, Removed call(s) to FfxDriverDeviceHandleValidate()
    to allow this module to be linked with either the driver or application
    libraries.  The API is not available through the external interface.
    Revision 1.3  2008/03/23 04:25:46Z  Garyp
    Modified to use some slightly relocated DEVICE and DISK configuration info
    Revision 1.2  2007/11/03 23:50:32Z  Garyp
    Updated to use the standard module header.
    Revision 1.1  2007/03/22 23:29:14Z  timothyj
    Initial revision
---------------------------------------------------------------------------*/

#include <windows.h>
/* ceddk.h is needed for PageSize, etc. */
#include <ceddk.h>

#include <diskio.h>

#include <flashfx.h>
#include <oecommon.h>
#include <fxdriver.h>
#include <ffxwce.h>


/*  Private helper functions for mapping and unmapping addresses
*/
static FFXSTATUS MapAddress( void **ppVirt, void *pPhys, D_UINT32 ulLength, D_BOOL fCacheable );
static void UnmapAddress(void * pVirt, D_UINT32 ulLength);

/*-------------------------------------------------------------------
    FfxHookOptionGet()

    Description
        Get a configuration option.

    Parameters
        opt       - The option identifier (FFXOPT_*).
        handle    - The FFXDEVHANDLE or FFXDISKHANDLE.
        pBuffer   - A pointer to object to receive the option value.
                    May be NULL.
        ulBuffLen - The size of object to receive the option value.
                    May be zero if pBuffer is NULL.

    Returns
        TRUE if the option identifier is valid, the option value is
        available and either pBuffer is NULL or ulBuffLen is the
        appropriate size for the option value, otherwise FALSE.
-------------------------------------------------------------------*/
D_BOOL FfxHookOptionGet(
    FFXOPTION       opt,
    void           *handle,
    void           *pBuffer,
    D_UINT32        ulBuffLen)
{
    D_BOOL          fResult = TRUE;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_CONFIG, 1, TRACEINDENT),
        "FfxHookOptionGet() handle=%P Type=%x pBuff=%P Len=%lU\n",
        handle, opt, pBuffer, ulBuffLen));

    switch(opt)
    {
        case FFXOPT_FLASH_START:
        {
            if(pBuffer)
            {
                FFXDEVINFO *pDev = *(FFXDEVHANDLE)handle;
                FFXSTATUS  ffxStat;

                DclAssert(ulBuffLen == sizeof(D_UINT32));

                ffxStat = MapAddress( (void **)pBuffer, pDev->Conf.DevSettings.pBaseFlashAddress, 0x00002000, FALSE );
                if (ffxStat == FFXSTAT_SUCCESS)
                {
                    fResult = TRUE;
                    break;
                }
            }
            fResult = FALSE;
            break;
        }
        default:

            /*  Other parameter codes may be recognized by the Driver Framework.
            */
            fResult = FfxDriverOptionGet(opt, handle, pBuffer, ulBuffLen);
            break;
    }

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_CONFIG, 1, TRACEUNDENT),
        "FfxHookOptionGet() returning %U\n", fResult));

    return fResult;
}

/*
 * VirtualCopy is provided in one of the standard DLLs on Windows CE, but the
 *   prototype is in the platform-specific header files.
 */
extern BOOL VirtualCopy(LPVOID lpvDest, LPVOID lpvSrc, DWORD cbSize, DWORD fdwProtect);

static FFXSTATUS MapAddress( void **ppVirt, void *pPhys, D_UINT32 ulLength, D_BOOL fCacheable )
{

    void *pVirt;
    D_UINT32 ulPhysAligned;
    D_UINT32 ulPhysOffset;
    D_UINT32 ulAlignedLength;
    D_UINT32 ulFlags;

    ulPhysAligned = (unsigned long)pPhys & ~(PAGE_SIZE - 1);
    ulPhysOffset = (unsigned long)pPhys & (PAGE_SIZE - 1);

    if (ulPhysOffset != 0)
    {
        ulLength += PAGE_SIZE;
    }

    /*  TBD: Put this in ALL implementations of MapAddress
    */
    if (ulLength < (D_UINT32)PAGE_SIZE)
    {
        ulLength = PAGE_SIZE;
    }

    ulAlignedLength = ulLength & ~(PAGE_SIZE - 1);

    /*  The flash address and length were both specified, map it and
        record the virtual address.
    */
    pVirt = VirtualAlloc(0, ulAlignedLength, MEM_RESERVE, PAGE_NOACCESS);
    if(!pVirt)
    {
        DWORD dwLastError = GetLastError();

        FFXPRINTF(1, ("FHMX35: VirtualAlloc() failed, LastError: 0x%08X (%u)\n",
            dwLastError, dwLastError));
        return FFXSTAT_PROJMAPMEMFAILED;
    }

    ulFlags = PAGE_READWRITE | PAGE_PHYSICAL;
    if (!fCacheable)
    {
        ulFlags |= PAGE_NOCACHE;
    }

    /*
     * Note: The following cast and shift assume the pointer of specific size.
     */
    if(!VirtualCopy(pVirt, (void *)(ulPhysAligned >> 8),
                ulAlignedLength, ulFlags))
    {
        DWORD dwLastError = GetLastError();

        FFXPRINTF(1, ("FHMX35: VirtualCopy() failed, LastError: 0x%08X (%u)\n",
            dwLastError, dwLastError));

        VirtualFree(pVirt, 0, MEM_RELEASE);

        return FFXSTAT_PROJMAPMEMFAILED;
    }

    (D_UINT32)pVirt += ulPhysOffset;

    *ppVirt = pVirt;

    return FFXSTAT_SUCCESS;
}

static void UnmapAddress(void * pVirt, D_UINT32 ulLength)
{
    VirtualFree(pVirt, 0, MEM_RELEASE);
}