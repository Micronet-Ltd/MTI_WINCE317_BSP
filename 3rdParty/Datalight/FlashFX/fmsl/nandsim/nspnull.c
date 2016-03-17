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

    This module implements the NAND Simulator Null Error Policy.  This policy
    will cause no errors to occur on the simulated NAND flash device.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: nspnull.c $
    Revision 1.9  2009/02/06 08:36:12Z  keithg
    Updated to reflect new location of NAND header files and macros,.
    Revision 1.8  2008/02/02 20:23:22  keithg
    comment updates to support autodoc
    Revision 1.7  2007/12/27 00:16:42Z  Garyp
    Eliminated the use of FFXCONF_NAND_SIMULATOR.  The NAND simulator
    code is now conditional on FFXCONF_NANDSUPPORT only.
    Revision 1.6  2007/11/03 23:49:37Z  Garyp
    Updated to use the standard module header.
    Revision 1.5  2007/08/22 19:52:41Z  pauli
    Added a name field to the error policy interface.
    Revision 1.4  2006/10/20 00:50:28Z  DeanG
    Forward-port simulator persistence enhancement
    Revision 1.3.1.2  2006/10/20 00:50:28Z  DeanG
    Separate out device dimensions into separate structure.
    Revision 1.3  2006/08/26 00:30:03Z  Pauli
    Added compile time setting to allow enabling/disabling the NAND simulator.
    Revision 1.2  2006/07/28 03:06:21Z  DeanG
    Changes to add "random" policy module
    Revision 1.1  2006/07/20 23:27:40Z  Pauli
    Initial revision
---------------------------------------------------------------------------*/

#include <flashfx.h>
#include <nandsim.h>


#if FFXCONF_NANDSUPPORT

static NsPolicyCreate     Create;
static NsPolicyDestroy    Destroy;
static NsPolicyBlockInit  BlockInit;
static NsPolicyReadPage   ReadPage;
static NsPolicyWritePage  WritePage;
static NsPolicyReadSpare  ReadSpare;
static NsPolicyWriteSpare WriteSpare;
static NsPolicyEraseBlock EraseBlock;

NsPolicyInterfaces NANDSIM_NullPolicy =
{
    Create,
    Destroy,
    BlockInit,
    ReadPage,
    WritePage,
    ReadSpare,
    WriteSpare,
    EraseBlock,
    "Null (No Errors)"
};


struct tagNsPolicyData
{
    NsDevDimensions * pDevDim;
    D_UINT32          ulReadCount;
    D_UINT32          ulWriteCount;
    D_UINT32          ulEraseCount;
};


/*-------------------------------------------------------------------
    Public: Create

    This will be called by the NAND Simulator once for each device
    that is configured to utilize this specific error policy.
    This function should perform whater initialization the policy
    requires to prepare it for use.  It must create and return
    a handle that will uniquely identify this instance of the
    policy.

    Parameters:
        hDev      - An opaque pointer to a device on which this
                    module is to begin operating.  This module uses
                    this handle to acquire per-device configuration.
        pDevDim   - A pointer to the device dimensions.

    Return:
        Returns a handle to the policy that is used for subsequent
        policy layer calls.
-------------------------------------------------------------------*/
static NsPolicyHandle Create(
    FFXDEVHANDLE      hDev,
    NsDevDimensions * pDevDim)
{
    NsPolicyData   *pPolicyData;

    DclAssert(hDev);

    (void) hDev;

    /*  Allocate our private data.
    */
    pPolicyData = DclMemAlloc(sizeof(*pPolicyData));
    if(!pPolicyData)
    {
        return NULL;
    }

    pPolicyData->pDevDim      = pDevDim;
    pPolicyData->ulReadCount  = 0;
    pPolicyData->ulWriteCount = 0;
    pPolicyData->ulEraseCount = 0;

    return pPolicyData;
}


/*-------------------------------------------------------------------
    Public: Destroy

    This function is called by the NAND Simulator when the device
    is being destroyed.  This function should perform whatever
    cleanup this policy layer needs.

    Parameters:
        hPolicy  - a handle to the policy layer.

    Return:
        (none)
-------------------------------------------------------------------*/
static void Destroy(
    NsPolicyHandle hPolicy )
{
    DclAssert(hPolicy);
    if(hPolicy)
    {
        DclMemFree(hPolicy);
    }
}


/*-------------------------------------------------------------------
    Public: BlockInit

    This function is called to specify an action as each block is
    initialized.  Blocks are initialized only once when the NAND
    Simulator creates a new simulated flash array.  This is
    typically be used to indicate factory bad blocks on the flash.

    Parameters:
        hPolicy  - a handle to the policy layer.
        pParams  - a pointer to the parameters to be used.

    Return:
        NsPolicyAction - a structure containing the action requested of
        the caller.
-------------------------------------------------------------------*/
static NsPolicyAction BlockInit(
    NsPolicyHandle hPolicy,
    NsPolicyBlockParam const * pParams )
{
    NsPolicyAction  action = { NsPolicyError_None };

    DclAssert(hPolicy);
    DclAssert(pParams);

    (void) pParams;
    (void) hPolicy;

    return action;
}


/*-------------------------------------------------------------------
    Public: ReadPage

    This function is called to specify an action when a page is read.

    Parameters:
        hPolicy  - a handle to the policy layer.
        pParams  - a pointer to the parameters to be used.

    Return:
        NsPolicyAction - a structure containing the action requested of
        the caller.
-------------------------------------------------------------------*/
static NsPolicyAction ReadPage(
    NsPolicyHandle hPolicy,
    NsPolicyPageParam const * pParams )
{
    NsPolicyAction action = { NsPolicyError_None };

    DclAssert(hPolicy);
    DclAssert(pParams);

    (void) pParams;

    hPolicy->ulReadCount++;

    return action;
}


/*-------------------------------------------------------------------
    Public: WritePage

    This function is called to specify an action when a page is written.

    Parameters:
        hPolicy  - a handle to the policy layer.
        pParams  - a pointer to the parameters to be used.

    Return:
        NsPolicyAction - a structure containing the action requested of
        the caller.
-------------------------------------------------------------------*/
static NsPolicyAction WritePage(
    NsPolicyHandle hPolicy,
    NsPolicyPageParam const * pParams )
{
    NsPolicyAction action = { NsPolicyError_None };

    DclAssert(hPolicy);
    DclAssert(pParams);

    (void) pParams;

    hPolicy->ulWriteCount++;

    return action;
}


/*-------------------------------------------------------------------
    Public: ReadSpare

    This function is called to specify an action when a spare area is
    read.

    Parameters:
        hPolicy  - a handle to the policy layer.
        pParams  - a pointer to the parameters to be used.

    Return:
        NsPolicyAction - a structure containing the action requested of
        the caller.
-------------------------------------------------------------------*/
static NsPolicyAction ReadSpare(
    NsPolicyHandle hPolicy,
    NsPolicyPageParam const * pParams )
{
    NsPolicyAction action = { NsPolicyError_None };

    DclAssert(hPolicy);
    DclAssert(pParams);

    (void) pParams;
    (void) hPolicy;

    return action;
}


/*-------------------------------------------------------------------
    Public: WriteSpare

    This function is called to specify an action when a spare area is
    written.

    Parameters:
        hPolicy  - a handle to the policy layer.
        pParams  - a pointer to the parameters to be used.

    Return:
        NsPolicyAction - a structure containing the action requested of
        the caller.
-------------------------------------------------------------------*/
static NsPolicyAction WriteSpare(
    NsPolicyHandle hPolicy,
    NsPolicyPageParam const * pParams )
{
    NsPolicyAction action = { NsPolicyError_None };

    DclAssert(hPolicy);
    DclAssert(pParams);

    (void) pParams;
    (void) hPolicy;

    return action;
}


/*-------------------------------------------------------------------
    Public: EraseBlock

    This function is called to specify an action when a block is erased.

    Parameters:
        hPolicy  - a handle to the policy layer.
        pParams  - a pointer to the parameters to be used.

    Return:
        NsPolicyAction - a structure containing the action requested of
        the caller.
-------------------------------------------------------------------*/
static NsPolicyAction EraseBlock(
    NsPolicyHandle hPolicy,
    NsPolicyBlockParam const * pParams )
{
    NsPolicyAction action = { NsPolicyError_None };

    DclAssert(hPolicy);
    DclAssert(pParams);

    (void) pParams;

    hPolicy->ulEraseCount++;

    return action;
}

#endif  /* FFXCONF_NANDSUPPORT */

