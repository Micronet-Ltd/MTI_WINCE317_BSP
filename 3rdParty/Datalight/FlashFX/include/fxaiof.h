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

    This header contains structures, symbols, and prototypes necessary to
    use the All-In-One FML and VBF frameworks.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: fxaiof.h $
    Revision 1.6  2009/02/09 03:34:11Z  garyp
    Merged from the v4.0 branch.  Removed some now invalid preprocessor
    logic.
    Revision 1.5  2008/03/23 02:39:34Z  Garyp
    Made allocator/VBF code conditional on FFXCONF_ALLOCATORSUPPORT and
    FFXCONF_VBFSUPPORT, respectively.
    Revision 1.4  2007/11/03 23:49:38Z  Garyp
    Updated to use the standard module header.
    Revision 1.3  2006/03/15 22:26:31Z  Garyp
    Minor function renaming exercise.
    Revision 1.2  2006/02/06 20:05:16Z  Garyp
    Updated to use the FFXDEVHANDLE and FFXDISKHANDLE model.
    Revision 1.1  2005/10/02 02:11:02Z  Pauli
    Initial revision
    Revision 1.2  2004/12/30 23:08:25Z  GaryP
    Updated to use indents of 4, hard tabs of 8 (if used), and a standard
    comment style.
    Revision 1.1  2004/07/20 01:25:36Z  GaryP
    Initial revision
---------------------------------------------------------------------------*/
#ifndef FXAIOF_H_INCLUDED
#define FXAIOF_H_INCLUDED

/*  Function pointer for the TESTS and TOOLS All-In-One Framework
*/
typedef     D_INT16(*PFNFRAMEWORKCMD)(FFXTOOLPARAMS *pTP);

D_INT16     FfxDriverFmlAllInOneFramework(PFNFRAMEWORKCMD pfnFWCommand, FFXTOOLPARAMS *pTP, FFXDEVINITDATA *pDeviceData, FFXDISKINITDATA *pDiskData);
D_INT16     FfxDriverVbfAllInOneFramework(PFNFRAMEWORKCMD pfnFWCommand, FFXTOOLPARAMS *pTP, FFXDEVINITDATA *pDeviceData, FFXDISKINITDATA *pDiskData);


#endif  /* FXAIOF_H_INCLUDED */
