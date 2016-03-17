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

    This header contains prototypes for the abstractiosn in dltloaderutil.h.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dltloaderutil.h $
    Revision 1.2  2007/11/03 23:31:11Z  Garyp
    Added the standard module header.
    Revision 1.1  2007/10/31 00:47:06Z  jeremys
    Initial revision
    Revision 1.3  2007/10/05 22:20:18Z  brandont
    Changed RdrTstDiskOpen to return a bool when Reliance is 
    detected instead of taking an argument for whether Reliance is
    expected.
    Revision 1.2  2007/10/05 19:29:08Z  brandont
    Removed condition for DCLCONF_FATREADERSUPPORT
    and DCLCONF_RELIANCEREADERSUPPORT.  Updated
    prototype formatting.
    Revision 1.1  2007/10/03 22:37:14Z  brandont
    Initial revision
---------------------------------------------------------------------------*/


#ifndef DLTLOADERUTIL_H_INCLUDED
#define DLTLOADERUTIL_H_INCLUDED     1


DCLSTATUS DlLoaderTestCheckSeed(
    D_BUFFER *  pBuffer,
    D_UINT32    ulBufferSize,
    D_UINT32    ulBufferOffset,
    D_UINT32    ulSerializationSeed);



#endif /* #ifndef DLTLOADERUTIL_H_INCLUDED */

