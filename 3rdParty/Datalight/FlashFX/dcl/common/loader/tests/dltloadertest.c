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
    $Log: dltloadertest.c $
    Revision 1.4  2009/05/21 18:37:11Z  garyp
    Updated documentation.  Updated to use standard type names.  Cleaned
    up some code formatting.  No functional changes.
    Revision 1.3  2007/11/03 23:31:11Z  Garyp
    Added the standard module header.
    Revision 1.2  2007/10/29 18:13:06Z  jeremys
    Added additional parameters to this module.
    Revision 1.1  2007/10/06 01:26:40Z  brandont
    Initial revision
---------------------------------------------------------------------------*/

#include <dcl.h>
#include <dlreaderio.h>
#include <dltloader.h>


DCLSTATUS DclLoaderTest(
    DCLREADERIO            *psIoInstance,
    D_UINT32                ulTestCount,
    LOADERTESTPARAMS      **psTestList)
{
    DCLSTATUS               DclStatus;

    /*  Perform the I/O layer tests
    */
    DclStatus = DclLoaderTestIO(psIoInstance);
    if(DclStatus != DCLSTAT_SUCCESS)
    {
        goto Failure;
    }


    /*  Perform the reader level tests
    */
    DclStatus = DclLoaderTestReader(psIoInstance, psTestList, ulTestCount);
    if(DclStatus != DCLSTAT_SUCCESS)
    {
        goto Failure;
    }


    /*  Perform the loader level tests
    */
    DclStatus = DclLoaderTestLoader(psIoInstance, psTestList, ulTestCount);
    if(DclStatus != DCLSTAT_SUCCESS)
    {
        goto Failure;
    }


    /*  Destroy the IO instance.

        Note: This serves testing the the I/O layer destroy function
    */
    DclStatus = psIoInstance->fnIoDestroy(psIoInstance);
    if(DclStatus != DCLSTAT_SUCCESS)
    {
        DclPrintf("I/O layer destroy returned an error: %lU\n", DclStatus);
        goto Failure;
    }


    DclPrintf("Datalight Loader Test: PASSED\n");
    return DclStatus;

  Failure:

    DclPrintf("Datalight Loader Test: FAILED\n");
    return DclStatus;
}

