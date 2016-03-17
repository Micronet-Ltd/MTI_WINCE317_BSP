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
    information for a Device or Disk.

    To customize this for a particular project, copy this file to the
    project directory and add any additional parameter processing the
    project requires.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: fhoption.c $
    Revision 1.1  2009/12/13 17:22:48Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <flashfx.h>
#include <fxdriver.h>


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
        case FFXOPT_DEVICE_SOFTLOCKFLAGS:
        {
            DclAssert(FfxDriverDeviceHandleValidate(handle));
            if(pBuffer)
            {
                DclAssert(ulBuffLen == sizeof(unsigned));

                *(unsigned*)pBuffer = (FFXLOCKFLAGS_LOCKALLBLOCKS | 
                                       FFXLOCKFLAGS_UNLOCKINVERTEDRANGE |
                                       FFXLOCKFLAGS_LOCKFREEZE);
            }
            fResult = TRUE;

            break;
        }
        
        case FFXOPT_DEVICE_SOFTLOCKSTATE:
        {
            DclAssert(FfxDriverDeviceHandleValidate(handle));
            if(pBuffer)
            {
                DclAssert(ulBuffLen == sizeof(D_BOOL));

                *(D_BOOL*)pBuffer = TRUE;    /* AKA "Locked" */
            }
            fResult = TRUE;

            break;
        }
        
        default:

            /*  Other option codes may be recognized by the Driver Framework
            */
            fResult = FfxDriverOptionGet(opt, handle, pBuffer, ulBuffLen);
            break;
    }

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_CONFIG, 1, TRACEUNDENT),
        "FfxHookOptionGet() returning %U\n", fResult));

    return fResult;
}

