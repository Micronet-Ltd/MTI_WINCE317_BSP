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
    Project Directory and add any additional parameter processing the
    project requires.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: fhoption.c $
    Revision 1.6  2009/07/17 19:11:05Z  garyp
    Merged from the v4.0 branch.  Eliminated use of an obsolete header.
    Revision 1.5  2009/04/08 19:44:21Z  davidh
    Function headers updated for AutoDoc.
    Revision 1.4  2009/03/23 21:08:50Z  keithg
    Fixed bug 2509, Removed call(s) to FfxDriverDeviceHandleValidate()
    to allow this module to be linked with either the driver or application
    libraries.  The API is not available through the external interface.
    Revision 1.3  2008/03/23 03:58:56Z  Garyp
    Modified to use some slightly relocated DEVICE and DISK configuration info.
    Revision 1.2  2007/11/03 23:49:54Z  Garyp
    Updated to use the standard module header.
    Revision 1.1  2006/10/04 00:05:06Z  brandont
    Initial revision
    Revision 1.7  2006/10/04 00:05:05Z  Garyp
    Updated to use the new style printf macros and functions.
    Revision 1.6  2006/03/10 21:52:37Z  Pauli
    Updated a typecast to the correct type.
    Revision 1.5  2006/03/06 21:21:14Z  Garyp
    Eliminated the use of obsolete options.
    Revision 1.4  2006/02/14 01:13:24Z  Pauli
    Updated to use device/disk handles rather than the EXTMEDIAINFO structure.
    Revision 1.3  2006/02/10 02:48:08Z  Garyp
    Updated to use device/disk handles rather than the EXTMEDIAINFO structure.
    Revision 1.2  2006/02/09 19:10:26Z  Pauli
    Added FFXOPT_FIM_FILENAME.
    Revision 1.1  2005/10/06 05:51:00Z  Pauli
    Initial revision
---------------------------------------------------------------------------*/

#include <flashfx.h>
#include <fxdriver.h>
#include <ffxdrv.h>


/*-------------------------------------------------------------------
    Public: FfxHookOptionGet()

    Get a configuration option.

    Parameters:
        opt       - The option identifier (FFXOPT_*).
        handle    - The FFXDEVHANDLE or FFXDISKHANDLE.
        pBuffer   - A pointer to object to receive the option value.
                    May be NULL.
        ulBuffLen - The size of object to receive the option value.
                    May be zero if pBuffer is NULL.

    Return Value:
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

                DclAssert(ulBuffLen == sizeof(D_UINT32));
                *(D_UINT32 *) pBuffer =
                        (D_UINT32) pDev->Conf.DevSettings.pBaseFlashAddress;
            }
            fResult = TRUE;

            break;
        }

        case FFXOPT_FIM_FILENAME:
        {
            if(pBuffer)
            {
                static char szFilename[16];
                FFXDEVINFO *pDev = *(FFXDEVHANDLE)handle;

                DclAssert(ulBuffLen == sizeof(const D_UCHAR *));
                DclSNPrintf(szFilename, sizeof(szFilename), "FFXDEV%u.DAT", pDev->Conf.nDevNum);
                *(char **)pBuffer = szFilename;
            }
            fResult = TRUE;
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
