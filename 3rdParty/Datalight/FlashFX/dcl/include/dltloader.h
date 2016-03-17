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

    This header contains prototypes for the Loader tests.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dltloader.h $
    Revision 1.6  2009/05/21 18:37:13Z  garyp
    Updated documentation.  Updated to use standard type names.  Cleaned
    up some code formatting.  No functional changes.
    Revision 1.5  2007/11/30 01:54:51Z  brandont
    Added the DclLoaderTestIOSequential prototype.
    Revision 1.4  2007/11/03 23:31:21Z  Garyp
    Added the standard module header.
    Revision 1.3  2007/10/29 18:14:54Z  jeremys
    Changed the prototypes for the test functions to handle the new
    test list parameter.  Added the new sLoaderTestParams structure
    for passing around lists of test parameters.
    Revision 1.2  2007/10/05 23:19:19Z  brandont
    Renamed DlLoaderTestIO to DclLoaderTestIO.  Renamed DlLoaderTestReader to
    DclLoaderTestReader.  Renamed DlLoaderTestLoader to DclLoaderTestLoader.
    Added DclLoaderTest prototype.  Updated prototype for DclLoaderTestReader.
    Added DCLFUNC macro for most of the prototypes in this module.
    Revision 1.1  2007/10/05 01:46:48Z  brandont
    Initial revision
---------------------------------------------------------------------------*/

#ifndef DLTLOADER_H_INCLUDED
#define DLTLOADER_H_INCLUDED


typedef struct
{
    char       *szFileName;
    D_UINT32    ulFileSize;
    D_UINT32    ulSerializationSeed;
    D_UINT32    ulBufferSize;
    D_UINT32    ulFileReadCount;
} LOADERTESTPARAMS;

#define DclLoaderTestIO         DCLFUNC(DclLoaderTestIO)
#define DclLoaderTestReader     DCLFUNC(DclLoaderTestReader)
#define DclLoaderTestLoader     DCLFUNC(DclLoaderTestLoader)
/*  Name masquerading is not performed on these APIs since an application
    can include this header directly.
*/
#if 0
#define DclLoaderTest           DCLFUNC(DclLoaderTest)
#endif


DCLSTATUS DclLoaderTest(            DCLREADERIO *psIoInstance, D_UINT32 ulTestCount, LOADERTESTPARAMS **psTestList);
DCLSTATUS DclLoaderTestReader(      DCLREADERIO *psIoInstance, LOADERTESTPARAMS **psTests, D_UINT32 ulTestCount);
DCLSTATUS DclLoaderTestLoader(      DCLREADERIO *psIoInstance, LOADERTESTPARAMS **psTests, D_UINT32 ulTestCount);
DCLSTATUS DclLoaderTestIO(          DCLREADERIO *psIoInstance);
DCLSTATUS DclLoaderTestIOSequential(DCLREADERIO *psIoInstance, D_UINT32 ulSeed);


#endif /* #ifndef DLTLOADER_H_INCLUDED */
