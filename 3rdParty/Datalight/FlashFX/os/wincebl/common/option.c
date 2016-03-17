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
    Revision 1.9  2008/03/26 02:27:55Z  Garyp
    Header updated.
    Revision 1.8  2008/01/25 07:34:46Z  keithg
    Comment style updates to support autodoc.
    Revision 1.7  2007/11/03 23:50:17Z  Garyp
    Updated to use the standard module header.
    Revision 1.6  2006/10/13 02:02:02Z  Garyp
    Eliminated obsolete options.
    Revision 1.5  2006/05/17 18:32:13Z  Garyp
    Fixed to honor the FFX_USEMBR setting.
    Revision 1.4  2006/02/15 05:21:07Z  Garyp
    Removed obsolete settings.
    Revision 1.3  2006/02/12 00:03:58Z  Garyp
    Continuing updates to the new Device and Disk model.
    Revision 1.2  2006/02/07 02:50:55Z  Garyp
    Started the process of factoring things into Devices and Disks.
    Work-in-progress, not completed yet.
    Revision 1.1  2005/10/06 05:51:14Z  Pauli
    Initial revision
    Revision 1.4  2005/04/06 22:40:07Z  GaryP
    Combined a duplicate case to be consistent with the rest of our code.
    Revision 1.3  2004/12/30 23:29:57Z  GaryP
    Updated to use indents of 4, hard tabs of 8 (if used), and a standard
    comment style.
    Revision 1.2  2004/09/08 01:56:24Z  GaryP
    Eliminated the FFXOPT_FATFORMAT setting.
    Revision 1.1  2004/08/17 16:39:16Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <flashfx.h>

#include <oecommon.h>
#include <fxdriver.h>
#include <ffxldr.h>


/*-------------------------------------------------------------------
    Public: FfxOsOptionGet

    Get a configuration option.  This function is normally only
    called from FfxDriverOptionGet() to obtain an option that is
    standard for all projects using a given OS.

    Parameters:
        opt       - The option identifier (FFXOPT_*).
        handle    - The FFXDEVHANDLE or FFXDISKHANDLE.
        pBuffer   - A pointer to object to receive the option value.
                    May be NULL.
        ulBuffLen - The size of object to receive the option value.
                    May be zero if pBuffer is NULL.

    Return:
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
