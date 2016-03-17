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

    This module contains the default functionality for configuring OESL
    specific options.  Options that are commonly configured in typical
    projects are handled in fhoption.c, while general FlashFX options are
    handled by fxoption.c.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: option.c $
    Revision 1.8  2009/07/17 20:47:55Z  garyp
    Merged from the v4.0 branch.  Eliminated use of an obsolete header.
    Revision 1.7  2007/11/03 23:50:09Z  Garyp
    Updated to use the standard module header.
    Revision 1.6  2006/10/13 02:01:44Z  Garyp
    Eliminated obsolete options.
    Revision 1.5  2006/05/17 18:32:13Z  Garyp
    Fixed to honor the FFX_USEMBR setting.
    Revision 1.4  2006/03/06 21:21:11Z  Garyp
    Eliminated the use of obsolete options.
    Revision 1.3  2006/02/15 05:21:10Z  Garyp
    Removed obsolete settings.
    Revision 1.2  2006/02/12 00:04:00Z  Garyp
    Continuing updates to the new Device and Disk model.
    Revision 1.1  2005/10/06 05:51:16Z  Pauli
    Initial revision
    Revision 1.10  2005/03/29 05:44:40Z  GaryP
    Minor code formatting cleanup.
    Revision 1.9  2004/12/30 23:29:56Z  GaryP
    Updated to use indents of 4, hard tabs of 8 (if used), and a standard
    comment style.
    Revision 1.8  2004/09/08 01:56:23Z  GaryP
    Eliminated the FFXOPT_FATFORMAT setting.
    Revision 1.7  2004/08/24 06:05:38Z  GaryP
    Added the handlers for standard options.
    Revision 1.6  2004/08/17 16:39:12Z  garyp
    Modified FfxOptionGet() to no longer take an hDrive parameter.
    Revision 1.5  2004/08/13 22:21:44Z  GaryP
    Added a typecast to force enumerated items to be displayed properly when
    a 16-bit development environment is used.
    Revision 1.4  2004/08/06 23:34:19Z  GaryP
    Added support for FFXOPT_USEFATMONITOR.
    Revision 1.3  2004/07/22 04:02:03Z  GaryP
    Eliminated the uFormatBBMState support which is implemented elsewhere.
    Revision 1.2  2004/07/21 21:00:14Z  GaryP
    Added support for FFXOPT_FORMATBBMSTATE.  Updated to properly return
    TRUE for those option cases that are supported, but the supplied buffer
    is NULL.
    Revision 1.1  2004/07/07 03:23:38Z  GaryP
    Initial revision
---------------------------------------------------------------------------*/

#include <windows.h>
#include <diskio.h>

#include <flashfx.h>
#include <oecommon.h>
#include <fxdriver.h>
#include <ffxwce.h>


/*-------------------------------------------------------------------
    FfxOsOptionGet()

    Description
        Get a configuration option.  This function is normally only
        called from FfxDriverOptionGet() to obtain an option that is
        standard for all projects using a given OS.

    Parameters
        opt       - The option identifier (FFXOPT_*).
        handle    - The FFXDEVHANDLE or FFXDISKHANDLE.
        pBuffer   - A pointer to object to receive the option value.
                    May be NULL.
        ulBuffLen - The size of object to receive the option value.
                    May be zero if pBuffer is NULL.

    Return Value
        TRUE if the option identifier is valid, the option value is
        available and either pBuffer is NULL or ulBuffLen is the
        appropriate size for the option value, otherwise FALSE.
-------------------------------------------------------------------*/
D_BOOL FfxOsOptionGet(
    FFXOPTION       opt,
    void           *handle,
    void           *pBuffer,
    D_UINT32        ulBuffLen)
{
    D_BOOL          fResult = FALSE;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_CONFIG, 1, TRACEINDENT),
        "FfxOsOptionGet() handle=%P Type=%x pBuff=%P Len=%lU\n",
        handle, opt, pBuffer, ulBuffLen));

    switch (opt)
    {
#if 0
        case FFXOPT_FLASH_START:

            /*  Note that the original supplied parameter is the physical
                address dwFlashAddress, but the flash starting address
                that's used at run time is the virtual address pbFlashMedia.
            */
            if(hDrive->dwFlashAddress != BAD_FLASH_ADDRESS)
            {
                if(pBuffer)
                {
                    DclAssert(ulBuffLen == sizeof(BYTE *));
                    *(BYTE **) pBuffer = hDrive->pbFlashMedia;
                }
                fResult = TRUE;
            }
            break;

#endif
#if FFXCONF_FATSUPPORT
#if FFXCONF_FATFORMATSUPPORT
        case FFXOPT_FATROOTENTRIES:
            if(pBuffer)
            {
                DclAssert(ulBuffLen == sizeof(D_UINT16));
                *(D_UINT16 *) pBuffer = ROOT_DIR_SIZE;
            }
            fResult = TRUE;
            break;

        case FFXOPT_FATCOUNT:
            if(pBuffer)
            {
                DclAssert(ulBuffLen == sizeof(D_UINT16));
                *(D_UINT16 *) pBuffer = 2;
            }
            fResult = TRUE;
            break;
#endif
#if FFXCONF_FATMONITORSUPPORT
        case FFXOPT_USEFATMONITOR:
            if(pBuffer)
            {
                DclAssert(ulBuffLen == sizeof(D_BOOL));
                *(D_BOOL *) pBuffer = dd.fUseFatMon;
            }
            fResult = TRUE;
            break;
#endif
#endif

        default:
            /*  Other parameter codes may be recognized by FlashFX.
            */
            fResult = FfxOptionGet(opt, pBuffer, ulBuffLen);
            break;
    }

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_CONFIG, 1, TRACEUNDENT),
        "FfxOsOptionGet() returning %U\n", fResult));

    return fResult;
}
