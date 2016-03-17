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
  jurisdictions.  Patents may be pending.

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

    This header contains the prototypes, structures, and types necessary
    to use the Reliance reader API.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dlrelread.h $
    Revision 1.5  2009/05/21 18:37:10Z  garyp
    Updated documentation.  Updated to use standard type names.  Cleaned
    up some code formatting.  No functional changes.
    Revision 1.4  2007/11/03 23:31:11Z  Garyp
    Added the standard module header.
    Revision 1.3  2007/10/05 19:27:55Z  brandont
    Removed conditional for DCLCONF_RELIANCEREADERSUPPORT.
    Revision 1.2  2007/10/03 01:46:52Z  brandont
    Updated prototype formatting.
    Revision 1.1  2007/09/27 20:23:14Z  brandont
    Initial revision
---------------------------------------------------------------------------*/

#ifndef DLRELREAD_H_INCLUDED
#define DLRELREAD_H_INCLUDED


/*-------------------------------------------------------------------
---------------------------------------------------------------------
-----------------  Prototypes for Public Functions  -----------------
---------------------------------------------------------------------
-------------------------------------------------------------------*/

#define DclRelReaderDiskOpen    DCLFUNC(DclRelReaderDiskOpen)
#define DclRelReaderDiskClose   DCLFUNC(DclRelReaderDiskClose)
#define DclRelReaderFileOpen    DCLFUNC(DclRelReaderFileOpen)
#define DclRelReaderFileClose   DCLFUNC(DclRelReaderFileClose)
#define DclRelReaderFileRead    DCLFUNC(DclRelReaderFileRead)
#define DclRelReaderFileState   DCLFUNC(DclRelReaderFileState)

DCLSTATUS           DclRelReaderDiskOpen(DCLREADERIO *psReaderDevice, DCLREADERINSTANCE *ppDiskHandle);
void                DclRelReaderDiskClose(DCLREADERINSTANCE hDisk);
DCLREADERFILEHANDLE DclRelReaderFileOpen(DCLREADERINSTANCE hDisk, const char *pszFileName);
void                DclRelReaderFileClose(DCLREADERFILEHANDLE hFile);
D_UINT32            DclRelReaderFileRead(DCLREADERFILEHANDLE hFile, D_BUFFER *pBuffer, D_UINT32 ulLength);
D_UINT16            DclRelReaderFileState(DCLREADERFILEHANDLE hFile);


#endif /* DLRELREAD_H_INCLUDED */
