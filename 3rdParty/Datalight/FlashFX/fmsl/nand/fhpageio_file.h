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

    This header defines the interface to the fhpageio_file.c functions.

    Note that these functions <must> have an identical interface (excepting
    the name) as the standard fhpageio.c functions.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: fhpageio_file.h $
    Revision 1.3  2007/11/03 23:49:37Z  Garyp
    Updated to use the standard module header.
    Revision 1.2  2007/02/13 20:02:38Z  timothyj
    Changed D_UINT16 uCount parameters to D_UIN32 ulCount, to
    allow the call tree all the way up through the IoRequest to avoid
    having to range check (and/or split) requests.  Removed
    corresponding casts.
    Revision 1.1  2006/03/12 04:15:16Z  Garyp
    Initial revision
---------------------------------------------------------------------------*/

#ifndef FHPAGEIO_FILE_H_INCLUDED
#define FHPAGEIO_FILE_H_INCLUDED


PNANDCTL    FfxHookNTPageFileCreate(       FFXDEVHANDLE hDev, NANDFLASHINFO *pNFI, FFXFIMBOUNDS *pBounds);
FFXSTATUS   FfxHookNTPageFileDestroy(      PNANDCTL pNC);
FFXIOSTATUS FfxHookNTPageFileRead(         PNANDCTL pNC, D_UINT32 ulStartPage, D_UINT32 ulCount, D_BUFFER *pPageBuff, D_BUFFER *pSpareBuff);
FFXSTATUS   FfxHookNTPageFileReadStart(    PNANDCTL pNC, D_UINT32 ulPage);
FFXIOSTATUS FfxHookNTPageFileReadComplete( PNANDCTL pNC, D_BUFFER *pPageBuff, D_BUFFER *pSpareBuff);
FFXIOSTATUS FfxHookNTPageFileReadHidden(   PNANDCTL pNC, D_UINT32 ulStartPage, D_UINT32 ulCount, D_UINT16 uScale, D_BUFFER *pBuffer);
FFXIOSTATUS FfxHookNTPageFileWrite(        PNANDCTL pNC, D_UINT32 ulStartPage, D_UINT32 ulCount, const D_BUFFER *pPageBuff, const D_BUFFER *pSpareBuff);
FFXSTATUS   FfxHookNTPageFileWriteStart(   PNANDCTL pNC, D_UINT32 ulPage, const D_BUFFER *pPageBuff, const D_BUFFER *pSpareBuff);
FFXIOSTATUS FfxHookNTPageFileWriteComplete(PNANDCTL pNC);
FFXIOSTATUS FfxHookNTPageFileEraseBlock(   PNANDCTL pNC, D_UINT32 ulBlock);



#endif  /* FHPAGEIO_FILE_H_INCLUDED */

