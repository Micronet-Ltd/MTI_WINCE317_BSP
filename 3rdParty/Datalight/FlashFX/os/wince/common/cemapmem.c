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

    This module contains common CE code for mapping flash into virtual
    address space.  It is typically called from FfxHookDiskCreate().
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: cemapmem.c $
    Revision 1.3  2007/11/03 23:50:09Z  Garyp
    Updated to use the standard module header.
    Revision 1.2  2006/02/07 03:16:01Z  Garyp
    Updated debugging code.
    Revision 1.1  2005/10/06 05:48:56Z  Pauli
    Initial revision
    Revision 1.4  2005/03/29 03:36:46Z  GaryP
    Minor code formatting cleanup.
    Revision 1.3  2004/12/30 17:33:24Z  GaryP
    Updated to use indents of 4, hard tabs of 8 (if used), and a standard
    comment style.
    Revision 1.2  2004/12/27 01:56:54Z  GaryP
    Modified FfxCeUmnapFlash() to skip trying to unmap if the pointer is NULL.
    Revision 1.1  2004/08/23 19:26:18Z  GaryP
    Initial revision
---------------------------------------------------------------------------*/

#include <windows.h>

#include <flashfx.h>
#include <oecommon.h>


/*-------------------------------------------------------------------
    FfxCeMapFlash()

    Description
        The physical address of the flash to be mapped is specified
        by dwFlashAddress and dwFlashLength.  If they are equal to
        BAD_FLASH_ADDRESS and BAD_FLASH_LENGTH respectively, no
        mapping is performed, and the mapping operation is deemed
        to have succeeded; the address is set to NULL.  This
        accommodates NAND flash, for which the address and length
        are (generally) irrelevant.

    Parameters
        dwFlashAddress - The flash address in physical address space
        dwFlashLength  - The flash length

    Return Value
        A pointer to the mapped flash if successful, else NULL.
-------------------------------------------------------------------*/
BYTE * FfxCeMapFlash(
    D_BUFFER       *pFlashAddress,
    DWORD           dwFlashLength)
{
    DWORD           dwFlashAddress = (DWORD)pFlashAddress;
    BYTE *          pbFlashMedia;

    /*  If either the address nor the length was specified...
    */
    if(dwFlashAddress == BAD_FLASH_ADDRESS ||
       dwFlashLength == BAD_FLASH_LENGTH)
    {
        return NULL;
    }

    /*  The flash address and length were both specified, map it and
        record the virtual address.
    */
    pbFlashMedia = VirtualAlloc(0, dwFlashLength, MEM_RESERVE, PAGE_NOACCESS);
    if(!pbFlashMedia)
    {
        FFXPRINTF(1, ("VirtualAlloc() failed, Address=%lX Length=%lX\n", dwFlashAddress, dwFlashLength));
        return NULL;
    }

    if(!VirtualCopy((LPVOID)pbFlashMedia, (LPVOID)(dwFlashAddress >> 8),
                dwFlashLength, PAGE_READWRITE | PAGE_NOCACHE | PAGE_PHYSICAL))
    {
        FFXPRINTF(1, ("VirtualCopy() failed, Address=%lX Length=%lX\n", dwFlashAddress, dwFlashLength));

        VirtualFree(pbFlashMedia, 0, MEM_RELEASE);

        return NULL;
    }

    return pbFlashMedia;
}


/*-------------------------------------------------------------------
    FfxCeUnmapFlash()

    Description
        Release resources (in particular, the virtual address range)
        allocated by MapFlash().

    Parameters
        pbFlashMedia - The address created with FfxCeMapFlash()

    Return Value
        None
-------------------------------------------------------------------*/
void FfxCeUnmapFlash(
    BYTE * pbFlashMedia)
{
    if(pbFlashMedia)
        VirtualFree(pbFlashMedia, 0, MEM_RELEASE);
}
