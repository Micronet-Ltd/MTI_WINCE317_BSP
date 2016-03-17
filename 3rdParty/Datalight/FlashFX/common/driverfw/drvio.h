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

    This header contains prototypes for interfacing to the functions in
    drvio.c
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: drvio.h $
    Revision 1.6  2009/07/21 21:23:02Z  garyp
    Merged from the v4.0 branch.  Conditioned the prototypes based on the
    presence or absence of allocator support.
    Revision 1.5  2007/11/02 03:21:26Z  Garyp
    Modified FfxDriverReadData() to return a sector count rather than a Boolean
    TRUE/FALSE.
    Revision 1.4  2006/02/20 04:37:37Z  Garyp
    Minor type changes.
    Revision 1.3  2006/02/06 19:37:09Z  Garyp
    Updated to use new device/disk handle model.
    Revision 1.2  2006/01/11 02:34:14Z  Garyp
    Commented out a prototype for a function which is now static.
    Revision 1.1  2005/10/02 01:58:24Z  Pauli
    Initial revision
    Revision 1.2  2005/08/14 18:52:42Z  garyp
    Fixed FfxDriverReadData() to take a D_UINT32 length parameter
    as the function is structured to take.
    Revision 1.1  2005/08/03 02:50:18Z  pauli
    Initial revision
    Revision 1.2  2004/12/30 23:03:13Z  GaryP
    Updated to use indents of 4, hard tabs of 8 (if used), and a standard
    comment style.
    Revision 1.1  2004/02/01 18:59:12Z  garyp
    Initial revision
---------------------------------------------------------------------------*/


#if FFXCONF_ALLOCATORSUPPORT && FFXCONF_VBFSUPPORT
D_UINT32 FfxDriverReadData(FFXDISKHANDLE hDisk, D_UINT32 ulStartSector, D_UINT32 ulSectorCount, D_BUFFER *pBuffer);
#endif

