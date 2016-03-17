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

    This module implements the NAND Simulator Random Error Policy.  This
    policy will cause no errors to occur on the simulated NAND flash device.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: nsprand.c $
    Revision 1.15  2009/02/06 08:36:12Z  keithg
    Updated to reflect new location of NAND header files and macros,.
    Revision 1.14  2008/11/20 12:00:44  billr
    Fix compiler warning about type conversion with possible loss of data.
    Revision 1.13  2008/11/19 21:04:26Z  billr
    Resolve bug 2233: Random() function only returns 0 or 1.  Resolve
    bug 2234: Random bit errors are generated outside of intended
    erase block.  Correctly handle memory allocation failure in
    Create().  Fix memory leak in Destroy().  Use less memory to
    record persistent failure flags.
    Revision 1.12  2008/02/03 04:25:51Z  keithg
    comment updates to support autodoc
    Revision 1.11  2007/12/27 00:28:11Z  Garyp
    Eliminated the use of FFXCONF_NAND_SIMULATOR.  The NAND simulator
    code is now conditional on FFXCONF_NANDSUPPORT only.
    Revision 1.10  2007/11/03 23:49:37Z  Garyp
    Updated to use the standard module header.
    Revision 1.9  2007/08/30 17:16:35Z  thomd
    Fix to use 64bit Macros where necessary
    Revision 1.8  2007/08/22 19:35:46Z  pauli
    Added a name field to the error policy interface.
    Revision 1.7  2007/08/01 17:51:34Z  timothyj
    Converted some 64-bit types back to 32-bit and used 64-bit DCL macros for
    all 64-bit operations, to facilitate use in environments where the emulated
    64-bit types and environment-specific 64-bit libraries are used. (BZ #512
    and BZ #803)
    Revision 1.6  2007/07/25 03:34:29Z  timothyj
    Manually aligned structures for ARM compatibility (see BZ #150).
    Revision 1.5  2007/03/01 22:02:05Z  timothyj
    Changed pages per block computations to be D_UINT16, to avoid warnings or
    casts for working with the representations of this value in the NTMINFO and
    FIMINFO structs.
    Revision 1.4  2006/10/20 22:36:53Z  DeanG
    Forward-port simulator persistence enhancement
    Revision 1.3.1.2  2006/10/20 22:36:53Z  DeanG
    Add basic error persistence capability
    Revision 1.3  2006/08/26 00:30:03Z  Pauli
    Added compile time setting to allow enabling/disabling the NAND simulator.
    Revision 1.2  2006/07/29 01:31:59Z  DeanG
    Changes to accomodate enhanced random policy module
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

NsPolicyInterfaces NANDSIM_RandomPolicy =
{
    Create,
    Destroy,
    BlockInit,
    ReadPage,
    WritePage,
    ReadSpare,
    WriteSpare,
    EraseBlock,
    "Random"
};

typedef struct
{
    D_UINT32  ulCount;
    D_UINT32  ulRandom;
    D_UINT32  ulErrorThreshold;  /* errors per million operations */
} NsOperationData;

struct tagNsPolicyData
{
    NsDevDimensions * pDevDim;
    NsOperationData   Read;
    NsOperationData   Write;
    NsOperationData   Erase;
    D_UINT32          ulRandomErrorType;
    D_UINT16          iPercentTimeoutError;
    D_UINT16          iPercentIoError;
    D_UINT16          iPercentPageBitError;
    D_UINT16          iPercentBlockBitError;
    NsPolicyBitError  bitError;
    /*  TBD: These are at the end of the structure to manually align it.
        This generally should not be necessary if we allow the compiler
        to align the structures for the environment.
    */
    D_BOOL            isPersistent;
    D_UCHAR          *pIsBlockFailed;
};


/*-------------------------------------------------------------------
    Public: Random

    This function returns a random number between 0 and ulScale -1.

    Parameters:
        pAction  - a pointer to the action to be updated.
        ulScale  - the scale factor to be applied.

    Return:
        D_UINT32 - the scaled random number.
-------------------------------------------------------------------*/
static D_UINT32 Random(
    D_UINT32 * pRandom,
    D_UINT32   ulScale )
{
    D_UINT64 ullReturnValue;

    DclAssert( pRandom );
    DclUint64AssignUint32(&ullReturnValue, DclRand(pRandom));
    DclUint64MulUint32(&ullReturnValue, ulScale);
    DclUint64DivUint32(&ullReturnValue, ((D_UINT32) 1 << 31));
    return DclUint32CastUint64(&ullReturnValue);
}


/*-------------------------------------------------------------------
    Public: Error

    This function generates an error of random type.

    Parameters:
        hPolicy  - a handle to the policy layer.
        pAction  - a pointer to the action to be updated.
        ulSize   - the size in bits of the area for bit errors.

    Return:
        (none)
-------------------------------------------------------------------*/
static void Error(
    NsPolicyHandle   hPolicy,
    NsPolicyAction * pAction,
    D_UINT32         ulSize )
{
    D_INT16 iTypeFactor;

    DclAssert( hPolicy );
    pAction->ulNumBitErrors = 0; /* safety */
    pAction->pBitErrors     = &hPolicy->bitError; /* common */

    iTypeFactor = (D_INT16) Random( &hPolicy->ulRandomErrorType, 100 );
    if ( (iTypeFactor -= hPolicy->iPercentTimeoutError) < 0 )
    {
        pAction->error = NsPolicyError_Timeout;
    }
    else if ( (iTypeFactor -= hPolicy->iPercentIoError) < 0 )
    {
        pAction->error = NsPolicyError_IO;
    }
    else
    {
        pAction->ulNumBitErrors = 1;
        hPolicy->bitError.state
            = Random( &hPolicy->ulRandomErrorType, NsPolicyBitState_Count );
        hPolicy->bitError.ulOffset
            = Random( &hPolicy->ulRandomErrorType, ulSize );
        if ( (iTypeFactor -= hPolicy->iPercentPageBitError) < 0 )
        {
            pAction->error = NsPolicyError_PageBit;
        }
        else if ( (iTypeFactor -= hPolicy->iPercentBlockBitError) < 0 )
        {
            pAction->error = NsPolicyError_BlockBit;
        }
    }
}


/*-------------------------------------------------------------------
    Public: Create

    This will be called by the NAND Simulator once for each device
    that is configured to utilize this specific error policy.
    This function should perform whatever initialization the policy
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
    pPolicyData = DclMemAllocZero(sizeof *pPolicyData);
    if (pPolicyData)
    {
        pPolicyData->pIsBlockFailed = DclMemAllocZero(pDevDim->totalBlocks
                                                      * sizeof *pPolicyData->pIsBlockFailed);
        if (pPolicyData->pIsBlockFailed)
        {
            pPolicyData->pDevDim        = pDevDim;
            pPolicyData->isPersistent   = FALSE;
            pPolicyData->Read.ulCount   = 0;
            pPolicyData->Write.ulCount  = 0;
            pPolicyData->Erase.ulCount  = 0;
            pPolicyData->Read.ulRandom  = 123456789L;
            pPolicyData->Write.ulRandom = 345678901L;
            pPolicyData->Erase.ulRandom = 567890123L;
            pPolicyData->Read.ulErrorThreshold  =   10L;  /* errors per million ops */
            pPolicyData->Write.ulErrorThreshold = 1000L;  /* errors per million ops */
            pPolicyData->Erase.ulErrorThreshold =  100L;  /* errors per million ops */
            pPolicyData->ulRandomErrorType      = 789012345L;
            pPolicyData->iPercentTimeoutError   = 10;
            pPolicyData->iPercentIoError        = 60;
            pPolicyData->iPercentPageBitError   = 20;
            pPolicyData->iPercentBlockBitError  = 10;
        }
        else
        {
            DclMemFree(pPolicyData);
            pPolicyData = NULL;
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
    DclMemFree(hPolicy->pIsBlockFailed);
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
    NsPolicyHandle hPolicy,
    NsPolicyBlockParam const * pParams )
{
    NsPolicyAction  action = { NsPolicyError_None };

    DclAssert(hPolicy);
    DclAssert(pParams);

    (void) hPolicy;
    (void) pParams;

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

    hPolicy->Read.ulCount++;
    if ( Random( &hPolicy->Read.ulRandom, 1000000L )
         < hPolicy->Read.ulErrorThreshold )
    {
        Error(hPolicy, &action, pParams->ulSize * CHAR_BIT);
    }

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
    D_UINT32       blockNumber;
    NsPolicyAction action = { NsPolicyError_None };

    DclAssert(hPolicy);
    DclAssert(pParams);
    DclAssert(pParams->uPagesPerBlock);

    blockNumber = (D_UINT32)(pParams->ulPageNumber / pParams->uPagesPerBlock);

    hPolicy->Write.ulCount++;
    if ( hPolicy->pIsBlockFailed[ blockNumber ]
         || Random( &hPolicy->Write.ulRandom, 1000000L )
            < hPolicy->Write.ulErrorThreshold )
    {
        Error(hPolicy, &action, pParams->ulSize * CHAR_BIT);
        hPolicy->pIsBlockFailed[ blockNumber ] = (hPolicy->isPersistent != 0);
    }

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

    if ( Random( &hPolicy->Read.ulRandom, 1000000L )
         < hPolicy->Read.ulErrorThreshold )
    {
        Error(hPolicy, &action, pParams->ulSize * CHAR_BIT );
    }

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

    if ( Random( &hPolicy->Write.ulRandom, 1000000L )
         < hPolicy->Write.ulErrorThreshold )
    {
        Error(hPolicy, &action, pParams->ulSize * CHAR_BIT );
    }

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
    D_UINT32       blockNumber;
    NsPolicyAction action = { NsPolicyError_None };

    DclAssert(hPolicy);
    DclAssert(pParams);

    blockNumber = (D_UINT32)pParams->ulBlockNumber;

    hPolicy->Erase.ulCount++;
    if ( hPolicy->pIsBlockFailed[ blockNumber ]
         || Random( &hPolicy->Erase.ulRandom, 1000000L )
            < hPolicy->Erase.ulErrorThreshold )
    {
        int iTypeFactor = (int) Random(&hPolicy->ulRandomErrorType, 100);

        if ((iTypeFactor -= hPolicy->iPercentTimeoutError) < 0)
        {
            action.error = NsPolicyError_Timeout;
        }
        else if ((iTypeFactor -= hPolicy->iPercentIoError) < 0)
        {
            action.error = NsPolicyError_IO; /* Bug 2235 */
        }

        hPolicy->pIsBlockFailed[ blockNumber ] = (hPolicy->isPersistent != 0);
    }

    return action;
}

#endif  /* FFXCONF_NANDSUPPORT */
