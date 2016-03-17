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

    This module contains MBR related helper routines.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: drvmbr.c $
    Revision 1.10  2009/07/18 00:53:09Z  garyp
    Eliminated use of an obsolete header.
    Revision 1.9  2009/03/31 19:19:59Z  davidh
    Function Headers updated for AutoDoc.
    Revision 1.8  2008/03/22 18:45:28Z  Garyp
    Made the code conditional on FFXCONF_VBFSUPPORT.  Minor data
    type changes.
    Revision 1.7  2008/01/13 07:26:30Z  keithg
    Function header updates to support autodoc.
    Revision 1.6  2007/11/03 23:49:31Z  Garyp
    Updated to use the standard module header.
    Revision 1.5  2006/10/16 20:32:11Z  Garyp
    Modified so MBR support can be stripped out of the product.
    Revision 1.4  2006/10/13 01:49:07Z  Garyp
    Updated to use some renamed symbols.
    Revision 1.3  2006/02/06 18:08:27Z  Garyp
    Updated to use new device/disk handle model.
    Revision 1.2  2006/01/11 01:52:47Z  Garyp
    Modified to dynamically allocate a sector buffer.  Added debugging code.
    Revision 1.1  2005/10/03 19:28:02Z  Pauli
    Initial revision
    Revision 1.15  2004/12/30 17:32:42Z  GaryP
    Updated to use indents of 4, hard tabs of 8 (if used), and a standard
    comment style.
    Revision 1.14  2004/12/18 23:23:38Z  GaryP
    Modified to accomodate MBRs that manage disks with sector sizes other than
    512 bytes.  Added a FlashFX proprietary field into the MBR to denote the
    sector size.
    Revision 1.13  2004/11/19 20:23:50Z  GaryP
    Updated to use refactored header files.
    Revision 1.12  2004/09/25 04:10:32Z  GaryP
    Added error handling to FfxCalculateCHS().
    Revision 1.11  2004/09/22 17:52:19Z  GaryP
    Updated to use DSK_SECTORLEN.
    Revision 1.10  2004/09/17 02:27:26Z  GaryP
    Minor function renaming exercise.
    Revision 1.9  2004/08/13 20:08:29Z  GaryP
    Modified to use the updated FfxDriverSectorWrite() function.
    Revision 1.8  2004/08/06 03:13:26Z  GaryP
    Modified to return the calculated heads, SPT, and cylinders values, as well
    as use the driver framework functions to write the MBR.
    Revision 1.7  2004/07/23 23:43:33Z  GaryP
    Added support for specifying a default file system.
    Revision 1.6  2004/04/30 02:27:40Z  garyp
    Updated to eliminate passing sector buffers around.
    Revision 1.5  2003/11/01 01:31:26Z  garys
    Merge from FlashFXMT
    Revision 1.2.1.2  2003/11/01 01:31:26  garyp
    Re-checked into variant sandbox.
    Revision 1.3  2003/11/01 01:31:26Z  garyp
    Updated to use the new FlashFX/MT interface.
    Revision 1.2  2003/06/03 22:54:12Z  garyp
    Fixed the file number to not conflict.
    Revision 1.1  2003/04/10 18:23:46Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <flashfx.h>

#if FFXCONF_ALLOCATORSUPPORT
#if FFXCONF_VBFSUPPORT

#include <fxdriver.h>


#if FFXCONF_MBRFORMAT

/*-------------------------------------------------------------------
    Protected: FfxDriverWriteMBR()

    This function derives the disk characteristics, based on the
    media size, and writes an MBR.

    Parameters:

    Return Value:
        Returns TRUE if successful, otherwise FALSE.
-------------------------------------------------------------------*/
D_BOOL FfxDriverWriteMBR(
    FFXDISKHANDLE   hDisk,
    D_UINT32        ulMediaSize,
    D_UINT32       *pulHiddenSecs,
    D_UINT16       *puSides,
    D_UINT16       *puSPT,
    D_UINT16       *puCylinders)
{
    FFXDISKINFO    *pDsk = *hDisk;
    D_UINT32        ulTotalSecs;
    D_UINT32        ulWritten;
    D_BOOL          fSuccess = FALSE;
    D_BUFFER       *pBuffer = NULL;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_DRIVERFW, 1, TRACEINDENT),
        "FfxDriverWriteMBR() Size=%lU\n", ulMediaSize));

    DclAssert(pDsk);
    DclAssert(pulHiddenSecs);
    DclAssert(puSides);
    DclAssert(puSPT);
    DclAssert(puCylinders);

    /*  If the emulated sector length is greater than, or equal to
        a standard disk sector (512 bytes) we are OK, but if it
        is less then, we can't build an MBR in it.
    */
    if(pDsk->ulSectorLength < DSK_SECTORLEN)
    {
        DclError();
        goto Cleanup;
    }

    pBuffer = DclMemAlloc(pDsk->ulSectorLength);
    if(!pBuffer)
    {
        DclError();
        goto Cleanup;
    }

    /*  Determine the client disk size
    */
    ulTotalSecs = ulMediaSize / pDsk->ulSectorLength;

    DclAssert(ulTotalSecs);

    /*  Determine the number of heads, SPT, sides, and cylinders.  This
        will update the number of sectors to fit into the calculated CHS.
    */
    if(!FfxCalculateCHS(&ulTotalSecs, puSides, puSPT, puCylinders))
        goto Cleanup;

    /*  Make sure there is enough space in the media
    */
    if(ulTotalSecs < 32)
    {
        DclError();
        goto Cleanup;
    }

    /*  Build the Master Boot Record (MBR)
    */
    *pulHiddenSecs = FfxMBRBuild(pBuffer, *puSides, *puSPT, *puCylinders,
            pDsk->ulSectorLength, pDsk->Conf.FormatSettings.uFileSystem);

    ulWritten = FfxDriverSectorWrite(hDisk, 0, 1, pBuffer);
    if(ulWritten == 1)
        fSuccess = TRUE;

  Cleanup:

    if(pBuffer)
        DclMemFree(pBuffer);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_DRIVERFW, 1, TRACEUNDENT),
        "FfxDriverWriteMBR() returning %U\n", fSuccess));

    return fSuccess;
}


#endif  /* FFXCONF_MBRFORMAT */

#endif  /* FFXCONF_VBFSUPPORT */
#endif  /* FFXCONF_ALLOCATORSUPPORT */




