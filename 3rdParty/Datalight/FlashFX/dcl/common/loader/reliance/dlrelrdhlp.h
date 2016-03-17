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

---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dlrelrdhlp.h $
    Revision 1.3  2007/11/03 23:31:11Z  Garyp
    Added the standard module header.
    Revision 1.2  2007/10/03 01:53:00Z  brandont
    Removed the unused function RelReaderGetMemoryRequirements.
    Revision 1.1  2007/09/27 01:21:12Z  jeremys
    Initial revision
---------------------------------------------------------------------------*/


/*  Definitions and macros used by the Reliance reader.
*/
typedef struct RELFILEINFO *PRELFILEINFO;
typedef struct RELDISKINST *PRELDISKINST;

typedef unsigned long (
    *PFNDEVICEREADSECTORS) (
    void *pBlockDev,
    unsigned long ulStartSector,
    unsigned long ulCount,
    void *pBuffer);


unsigned short  RelReaderVolumeMount(
    PRELDISKINST * pDisk,
    PFNDEVICEREADSECTORS pfnDevRead,
    unsigned long ulBytesPerSector,
    void *pDevInfo);
void            RelReaderVolumeDismount(
    PRELDISKINST pDisk);
PRELFILEINFO    RelReaderFileOpen(
    PRELDISKINST pDisk,
    const char *pszFileName);
void            RelReaderFileClose(
    PRELFILEINFO pFile);
unsigned long   RelReaderFileRead(
    PRELFILEINFO pFile,
    unsigned long ulLength,
    void *pBuffer);
unsigned short  RelReaderFileState(
    PRELFILEINFO pFile);
