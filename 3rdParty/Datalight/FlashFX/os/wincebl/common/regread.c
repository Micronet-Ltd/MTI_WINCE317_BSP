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

    This module implements the FlashFX version of the Windows CE
    ReadRegistryFromOEM() function.  This function allows OEMs to read a
    WinCE registry file, previously saved with FXWriteRegistryToOEM, off
    of a FlashFX disk using the FlashFX Loader.  The name of the registry
    file to read is specified in the REG_FILE_NAME constant in FXREG.H.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: regread.c $
    Revision 1.6  2009/07/24 00:27:43Z  garyp
    Commented out an obsolete header and added appropriate comments.
    Revision 1.5  2008/01/25 07:34:46Z  keithg
    Comment style updates to support autodoc.
    Revision 1.4  2007/11/03 23:50:17Z  Garyp
    Updated to use the standard module header.
    Revision 1.3  2006/03/15 03:02:41Z  Garyp
    Updated to build cleanly.
    Revision 1.2  2006/03/09 20:52:10Z  Garyp
    Minor cleanup -- no functional changes.
    Revision 1.1  2005/10/01 11:25:06Z  Pauli
    Initial revision
    Revision 1.2  2004/12/30 23:43:18Z  GaryP
    Updated to use indents of 4, hard tabs of 8 (if used), and a standard
    comment style.
    Revision 1.1  2004/08/17 18:26:24Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <windows.h>
#include <pwinreg.h>
#include <diskio.h>

#include <ffxldr.h>
/* The old FXLOADER interface has been eliminated and replaced with the
   DCL Loader.  This code has not been updated to use the new interface.
   See Bug 2810
#include <fxloader.h>
*/

#include "fxreg.h"

#define REG_READ_FAILURE   (-1) /* MS defined */
#define REG_READ_EOF       (0)  /* MS defined */


/*-------------------------------------------------------------------
    Public: FXReadRegistryFromOEM

    This function reads the registry file stored on a FlashFX
    disk using the Windows CE OAL WriteRegistryToOEM function.
    The name of the file is specified in the REG_FILE_NAME
    constant in FXREG.H.

    Parameters:
        dwFlags - Read options specified by the OS.  The only
                  supported open mode is REG_READ_BYTES_START that
                  indicates reading must start from the beginning of
                  the registry file. Any other non-zero value causes
                  this routine to fail.
        lpData  - Pointer to the buffer allocated by the OS that is
                  to receive the registry data.
        cbData  - Number of bytes to read from the registry file.

    Return:
        Returns the number of bytes read into lpData, REG_READ_EOF
        if the end of the file has been reached or REG_READ_FAILURE
        on error.
-------------------------------------------------------------------*/
DWORD FXReadRegistryFromOEM(
    DWORD                       dwFlags,
    LPBYTE                      lpData,
    DWORD                       cbData)
{
    static REGSTATEMACHINE      stateMachine = SM_DORMANT;
    static FXLOADERHANDLE       hLoader = NULL;
    static FXLOADERFILEHANDLE   hFile = NULL;
    static DWORD                dwTotalRead;
    DWORD                       dwReturnValue;
    DWORD                       ulTotalJustRead;

    /*  The registry reads from the FlashFX disk are handled by this state
        machine.
    */
  NEXT_STATE:
    switch (stateMachine)
    {
            /*  Begin the registry restoration process by initializing FlashFX
            */
        case SM_DORMANT:
            RETAILMSG(1, (TEXT("\r\nReadRegistryFromOEM: Initializing FlashFX Loader.\r\n")));

            /*  Initialize the FlashFX loader.  Passing NULL pointers for
                the Disk and Device data structures will cause DEV0/DISK0
                to be used.  Specify partition 0 within the disk.
            */
            hLoader = FfxLoaderInitialize(NULL, NULL, 0);
            if(!hLoader)
            {
                RETAILMSG(1, (TEXT("\r\nReadRegistryFromOEM: FlashFX Loader Init Failed\r\n")));

                stateMachine = SM_BROKEN;
                goto NEXT_STATE;
            }

            stateMachine = SM_OPENING;

            /*  Fall through from successful init to open the file...
            */

        case SM_OPENING:
            if((dwFlags != REG_READ_BYTES_START) && (dwFlags != 0))
            {
                RETAILMSG(1, (TEXT("\r\nReadRegistryFromOEM: Unknown dwFlags argument: 0x%x\r\n"),
                           dwFlags));

                stateMachine = SM_BROKEN;
                goto NEXT_STATE;
            }
            DEBUGMSG(0, (TEXT("ReadRegistryFromOEM: Opening registry file.\r\n")));

            hFile = FfxLoaderFileOpen(hLoader, REG_FILE_NAME);
            if(!hFile)
            {
                RETAILMSG(1, (TEXT("\r\nReadRegistryFromOEM: Could not open registry file.\r\n")));

                stateMachine = SM_BROKEN;
                goto NEXT_STATE;
            }

            stateMachine = SM_ACCESSING;

            /*  We want to be able to catch the Registry Read routine telling
                us to rewind the file and start over.
            */
            dwFlags = 0;
            dwTotalRead = 0;

            /*  Fall through from successful open to start reading the file from the
                FlashFX disk.
            */

        case SM_ACCESSING:
            if(dwFlags != 0)
            {
                RETAILMSG(1, (TEXT("\r\nReadRegistryFromOEM: Non-zero dwFlags argument:in SM_ACCESSING: 0x%x\r\n"),
                           dwFlags));
                RETAILMSG(1, (TEXT("ReadRegistryFromOEM: Closing and re-opening registry file.\r\n")));

                FfxLoaderFileClose(hLoader, hFile);
                hFile = NULL;
                stateMachine = SM_OPENING;
                goto NEXT_STATE;
            }

            DEBUGMSG(0, (TEXT("ReadRegistryFromOEM: Reading 0x%x bytes from registry file from offset %08X.\r\n"),
                      cbData, dwTotalRead));
            if(cbData == 0)
            {
                DEBUGMSG(0, (TEXT("\r\nReadRegistryFromOEM: Got (cbData=0), closing registry file.\r\n")));

                stateMachine = SM_CLOSING;
                goto NEXT_STATE;
            }

            ulTotalJustRead = FfxLoaderFileRead(hLoader, hFile, cbData, lpData);

            dwReturnValue = ulTotalJustRead;

            DEBUGMSG(0, (TEXT("ReadRegistryFromOEM: Read 0x%x bytes from registry file: %02x %02x %02x %02x....\r\n"),
                ulTotalJustRead, *lpData, *(lpData+1), *(lpData+2), *(lpData+3)));

            if(ulTotalJustRead != cbData)
            {
                if(FfxLoaderFileState(hLoader, hFile) != FILESTATE_EOF)
                {
                    stateMachine = SM_BROKEN;
                    goto NEXT_STATE;
                }

                /*  Must have hit EOF, get out, returning as many bytes
                    as successfully read.
                */
            }

            break;

        case SM_CLOSING:
            DEBUGMSG(0, (TEXT("\r\nReadRegistryFromOEM: Registry close\r\n")));

            FfxLoaderFileClose(hLoader, hFile);
            hFile = NULL;

            FfxLoaderDeinitialize(hLoader);
            hLoader = NULL;

            stateMachine = SM_DORMANT;
            dwReturnValue = REG_READ_EOF;
            break;

        case SM_BROKEN:
        default:

            /*  Something is dreadfully wrong...
            */
            RETAILMSG(1, (TEXT("\r\nReadRegistryFromOEM: Registry read machine is broken\r\n")));

            if(hLoader)
            {
                if(hFile)
                {
                    FfxLoaderFileClose(hLoader, hFile);
                    hFile = NULL;
                }

                FfxLoaderDeinitialize(hLoader);
                hLoader = NULL;
            }

            stateMachine = SM_DORMANT;
            dwReturnValue = REG_READ_FAILURE;
            break;
    }

    return dwReturnValue;
}
