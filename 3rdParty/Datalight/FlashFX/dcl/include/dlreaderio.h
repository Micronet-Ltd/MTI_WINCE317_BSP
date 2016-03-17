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
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dlreaderio.h $
    Revision 1.7  2009/05/21 18:37:13Z  garyp
    Updated documentation.  Updated to use standard type names.  Cleaned
    up some code formatting.  No functional changes.
    Revision 1.6  2007/11/03 23:31:21Z  Garyp
    Added the standard module header.
    Revision 1.5  2007/10/10 03:41:43Z  brandont
    Removed #if 0 code blocks.
    Revision 1.4  2007/10/06 01:49:39Z  brandont
    Changed DCL_DEFINED_sDlReaderIo to not use TRUE.  Disabled the misplaced
    and possibly unused DCLSTAT_LOADER_xxx, DCLSTAT_FATRDR_xxx, and
    DCLSTAT_RELRDR_xxx defines.  Disabled the misplaced FxReaderIoCreate
    protoytype.  Removed the FileReaderIoCreate prototype.
    Revision 1.3  2007/10/03 22:00:22Z  brandont
    Changed typedef for sDlReaderIo to only be defined once.  Changed
    references to FFXLOCALE_LOADER to DCLLOCALE_LOADER.
    Revision 1.2  2007/09/27 22:35:24Z  jeremys
    Added a prototype for FileReaderIoCreate to dlreaderio.h.
    Revision 1.1  2007/09/27 01:26:54Z  jeremys
    Initial revision
---------------------------------------------------------------------------*/

#ifndef DLREADERIO_H_INCLUDED
#define DLREADERIO_H_INCLUDED

#ifndef DCL_DEFINED_sDlReaderIo
typedef struct sDCLREADERIO DCLREADERIO;
/* Note: this cannot be defined as TRUE since an application can include
         this header directly.
*/
#define DCL_DEFINED_sDlReaderIo     1
#endif

typedef DCLSTATUS (*tDlReaderIoGetParameters)(struct sDCLREADERIO *psReaderDevice, D_UINT32 *pulBlockSize, D_UINT32 *pulTotalBlocks);
typedef DCLSTATUS (*tDlReaderIoRead)         (struct sDCLREADERIO *psReaderDevice, D_UINT32 ulBlockStart, D_UINT32 ulBlockLength, void *pBuffer);
typedef DCLSTATUS (*tDlReaderIoDestroy)      (struct sDCLREADERIO *psReaderDevice);


struct sDCLREADERIO
{
    tDlReaderIoGetParameters    fnIoGetParameters;
    tDlReaderIoRead             fnIoRead;
    tDlReaderIoDestroy          fnIoDestroy;
    void                       *pDeviceSpecificInfo;
};


#endif /* #ifndef DLREADERIO_H_INCLUDED */
