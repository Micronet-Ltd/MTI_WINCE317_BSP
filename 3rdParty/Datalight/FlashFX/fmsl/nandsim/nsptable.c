/*---------------------------------------------------------------------------
               ----> DO NOT REMOVE THE FOLLOWING NOTICE <----

                  Copyright (c) 1993 - 2010 Datalight, Inc.
                       All Rights Reserved Worldwide.

  Datalight, Incorporated is a Washington State corporation currently located
  at:
        21520 30th Dr SE, Suite 110,      Tel:  425-951-8086
        Bothell, WA  98021                Fax:  425-951-8094
        USA                               Web:  http://www.datalight.com

  This software, including without limitation all source code and documen-
  tation, is a trade secret and the confidential property of Datalight, 
  Inc., protected under the copyright laws of the United States and other
  jurisdictions.  The software may be subject to one or more US patents, 
  including without limitation US Patents: US#5860082, US#6260156.  Patents
  may be pending.

  U.S. Government Restricted Rights:  Use, duplication, reproduction, or
  transfer of this commercial product and accompanying documentation is
  restricted in accordance with FAR 12.212 and DFARS 227.7202 and by a
  License Agreement.

  IN ADDITION TO COPYRIGHT AND PATENT LAW, THIS SOFTWARE IS PROTECTED UNDER
  CONTRACT(S) BETWEEN DATALIGHT, INC. AND THE LICENSEE ("BINDING AGREEMENTS").
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

    This module implements the NAND Simulator Table-Driven Error Policy.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: nsptable.c $
    Revision 1.19  2010/04/29 00:04:24Z  garyp
    Updated to build cleanly with the TICCS ToolSet abstraction.
    Revision 1.18  2009/03/03 09:15:13Z  keithg
    Added table entries to describe spare read and spare write error
    lists seperately from page reads and page writes.  See bug 2480.
    Revision 1.17  2009/02/19 03:52:58Z  keithg
    Changed the behavior of the table based policy to limit the number of
    errors thown for a given element.  See NsOperationElement.
    Revision 1.16  2009/02/06 01:37:39Z  keithg
    Updated to reflect new location of NAND header files and macros,.
    Revision 1.15  2009/01/30 16:30:41  billr
    Really fix the compiler warning.
    Revision 1.14  2009/01/31 00:26:38Z  billr
    Fix compiler warning about using uninitialized structure members.
    Revision 1.13  2009/01/29 22:31:01Z  keithg
    Moved policy data structure to NSPTABLE.H so it can be shared.
    Added support to initialize bad blocks based on a given block list.
    Revision 1.12  2009/01/22 19:01:10  keithg
    Updated to support NULL tables.
    Revision 1.11  2009/01/21 10:54:20  keithg
    Added example tables for reference purpses.
    Revision 1.10  2009/01/18 21:56:27  keithg
    Fixed to operate without a given table.
    Revision 1.9  2009/01/19 05:20:19Z  keithg
    Table is now provided as a project option.  Mode table information
    to the include file.
    Revision 1.8  2008/07/23 18:11:35Z  keithg
    Clarified comments with professional customer facing terms.
    Revision 1.7  2008/02/03 04:28:06Z  keithg
    comment updates to support autodoc
    Revision 1.6  2007/12/27 00:16:41Z  Garyp
    Eliminated the use of FFXCONF_NAND_SIMULATOR.  The NAND simulator
    code is now conditional on FFXCONF_NANDSUPPORT only.
    Revision 1.5  2007/11/03 23:49:37Z  Garyp
    Updated to use the standard module header.
    Revision 1.4  2007/08/22 19:35:45Z  pauli
    Added a name field to the error policy interface.
    Revision 1.3  2007/08/01 17:47:05Z  timothyj
    Converted some 64-bit types back to 32-bit and used 64-bit DCL
    macros for all 64-bit operations, to facilitate use in environments
    where the emulated 64-bit types and environment-specific 64-bit
    libraries are used. (BZ #512 and BZ #803)
    Revision 1.2  2006/12/06 22:10:07Z  DeanG
    Add new table-driven error policy module
---------------------------------------------------------------------------*/

#include <flashfx.h>
#include <nandsim.h>
#include <nsptable.h>


#if FFXCONF_NANDSUPPORT

static NsPolicyCreate     Create;
static NsPolicyDestroy    Destroy;
static NsPolicyBlockInit  BlockInit;
static NsPolicyReadPage   ReadPage;
static NsPolicyWritePage  WritePage;
static NsPolicyReadSpare  ReadSpare;
static NsPolicyWriteSpare WriteSpare;
static NsPolicyEraseBlock EraseBlock;

NsPolicyInterfaces NANDSIM_TablePolicy =
{
    Create,
    Destroy,
    BlockInit,
    ReadPage,
    WritePage,
    ReadSpare,
    WriteSpare,
    EraseBlock,
    "Table Driven"
};

/*  Example tables added for reference purposes.
*/
#if 0
static NsOperationElement readList[] =
{
    { "Read tracking",  NsPolicyError_None, -1, 500 }  /* dump every 500 accesses */
};

static NsOperationElement writeList[] =
{
    { "Write tracking",   NsPolicyError_None, -1, 200 }, /* dump every 200 accesses */
    { "Write failure #1", NsPolicyError_IO, 0, 2 }       /* error on 3rd access */
};

static NsOperationElement eraseList[] =
{
    { "Erase tracking",  NsPolicyError_None, -1, 100 }, /* dump every 100 accesses */
    { "Spare block bad", NsPolicyError_IO, 0xFD, 0 }    /* error on 1st access */
};

static NsOperationList const read
    = { "Read",  "Page",  readList,  sizeof readList  / sizeof readList[ 0 ] };

static NsOperationList const write
    = { "Write", "Page",  writeList, sizeof writeList / sizeof writeList[ 0 ] };

static NsOperationList const erase
    = { "Erase", "Block", eraseList, sizeof eraseList / sizeof eraseList[ 0 ] };
#endif


/*-------------------------------------------------------------------
    Public: ErrorCheck

    This function generates an error of random type.

    Parameters:
        pOpList  - a pointer to the operation list.
        ulMatch  - the address (page or block) to match.
        pAction  - a pointer to the action to be updated.

    Return:
        (none)
-------------------------------------------------------------------*/
static void ErrorCheck(
    NsOperationList const * pOpList,
    D_UINT32                ulMatch,
    NsPolicyAction        * pAction )
{
    D_UINT32             index;
    D_UINT32             ulMaxThreshold;
    NsOperationElement * pOpElement;

    DclAssert(pAction);

    /* NULL is now an aceptable parameter, nothing to do...
    */
    if(!pOpList)
        return;

    pAction->ulNumBitErrors = 0;  /* safety */
    for ( index = 0;  index < pOpList->ulCount;  index++ )
    {
        pOpElement = & pOpList->pOpElement[ index ];
        if (   pOpElement->ulMatch == ulMatch
            || pOpElement->ulMatch == D_UINT32_MAX )
        {
            pOpElement->ulCount++;
            switch ( pOpElement->errorType )
            {
            case NsPolicyError_None:
                /* Use "no error" to produce periodic debug output */
                DclAssert( pOpElement->ulThreshold );
                if ( pOpElement->ulCount % pOpElement->ulThreshold == 0 )
                {
                    DclPrintf( "FFX: NAND simulator: %s check '%s'; %s address = %08lX, count = %lU\n",
                               pOpList->pListName,
                               pOpElement->pTestLabel,
                               pOpList->pUnitName,
                               ulMatch,
                               pOpElement->ulCount );
                }
                if ( pOpElement->ulMatch == D_UINT32_MAX )
                {
                    continue;
                }
                break;
            default:
                /*  Allow for the threshold to be met prior to triggering any
                    errors.  Also, if the user had specified a maximum nubmer
                    of errors and that number has not been met, allow the error
                    to be reflected to the caller.
                */
                ulMaxThreshold = pOpElement->ulThreshold
                        + DCLMAX(1, pOpElement->ulMaxErrors);

                if ( pOpElement->ulCount > pOpElement->ulThreshold
                    && pOpElement->ulCount <= ulMaxThreshold)
                {
                    DclPrintf( "FFX: NAND simulator: %s error '%s'; %s address = %08lX, count = %lU\n",
                               pOpList->pListName,
                               pOpElement->pTestLabel,
                               pOpList->pUnitName,
                               ulMatch,
                               pOpElement->ulCount );
                    pAction->error          =   pOpElement->errorType;
                    pAction->pBitErrors     = & pOpElement->errorBits;
                    pAction->ulNumBitErrors =   1;
                }
                break;
            }
            return;
        }
    }
}


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
    NsPolicyData  table_policy;
    NsPolicyData   *pPolicyData;

    DclAssert(hDev);
    DclAssert(pDevDim);

    (void) pDevDim;

    /*  Allocate our private data.
    */
    pPolicyData = DclMemAllocZero(sizeof(*pPolicyData));
    if(pPolicyData)
    {
        /*  Setup the base file name and memory space for the full name.
        */
        if(FfxHookOptionGet(FFXOPT_NANDSIM_GET_TABLE, hDev,
                &table_policy, sizeof(table_policy)))
        {
            *pPolicyData = table_policy;
        }
    }

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
    DclMemFree(hPolicy);
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
    NsPolicyHandle             hPolicy,
    NsPolicyBlockParam const * pParams )
{
    NsPolicyAction action = { NsPolicyError_None };
    D_UINT32 ulIndex;

    DclAssert(hPolicy);
    DclAssert(pParams);

    if(hPolicy->rgBadBlocks && hPolicy->ulBadBlockCount)
    {
        /*  If the block is in our list indicate it is factory bad. */
        for(ulIndex = 0; ulIndex < hPolicy->ulBadBlockCount; ++ulIndex)
        {
            if(pParams->ulBlockNumber == hPolicy->rgBadBlocks[ulIndex])
            {
                action.error = NsPolicyError_FactoryBadBlock;
                break;
            }
        }
    }

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
    NsPolicyHandle            hPolicy,
    NsPolicyPageParam const * pParams )
{
    NsPolicyAction action = { NsPolicyError_None };

    DclAssert(pParams);

    if(hPolicy)
        ErrorCheck( hPolicy->pReadPage, pParams->ulPageNumber, &action );

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
    NsPolicyHandle            hPolicy,
    NsPolicyPageParam const * pParams )
{
    NsPolicyAction action = { NsPolicyError_None };

    DclAssert(pParams);

    if(hPolicy)
        ErrorCheck( hPolicy->pWritePage, pParams->ulPageNumber, &action );

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
    NsPolicyHandle            hPolicy,
    NsPolicyPageParam const * pParams )
{
    NsPolicyAction action = { NsPolicyError_None };

    DclAssert(pParams);

    if(hPolicy)
        ErrorCheck( hPolicy->pReadSpares, pParams->ulPageNumber, &action );

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
    NsPolicyHandle            hPolicy,
    NsPolicyPageParam const * pParams )
{
    NsPolicyAction action = { NsPolicyError_None };

    DclAssert(pParams);

    if(hPolicy)
        ErrorCheck( hPolicy->pWriteSpares, pParams->ulPageNumber, &action );

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
    NsPolicyHandle             hPolicy,
    NsPolicyBlockParam const * pParams )
{
    NsPolicyAction action = { NsPolicyError_None };

    DclAssert(pParams);

    if(hPolicy)
        ErrorCheck( hPolicy->pEraseBlock, pParams->ulBlockNumber, &action );

    return action;
}

#endif  /* FFXCONF_NANDSUPPORT */

