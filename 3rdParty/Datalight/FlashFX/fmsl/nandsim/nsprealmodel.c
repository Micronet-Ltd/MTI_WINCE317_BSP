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

    This module implements the NAND Simulator Real Model Error Policy.  This
    policy will cause errors to occur on the simulated NAND flash device in
    a fashion similar to real NAND devices.

    TBD: Persist pBlockInfo array somewhere, perhaps tightly coupled
    with the storage model, such that when the storage is
    re-initialized, so is this corresponding persistent data

---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: nsprealmodel.c $
    Revision 1.1  2009/02/06 23:35:38Z  billr
    Initial revision
    Based loosely on projects/WinNSim/nsprealmodel.c revision 1.7
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

NsPolicyInterfaces NANDSIM_RealModelPolicy =
{
    Create,
    Destroy,
    BlockInit,
    ReadPage,
    WritePage,
    ReadSpare,
    WriteSpare,
    EraseBlock,
    "Real Model"
};

#define ONE_MILLION (1000UL * 1000)
typedef struct
{
    D_UINT32  ulErrorThreshold;  /* errors per million operations */
    D_UINT16  iPercentTimeoutError;
    D_UINT16  iPercentIoError;
    D_UINT16  iPercentPageBitError;
    D_UINT16  iPercentBlockBitError;
} NsOperationData;


/*  Structure to hold all per-block information
*/
typedef struct
{
    unsigned nReads : 31;    /* reads in the block since last erase */
    unsigned fFailed : 1;    /* persistent failure in block */
} NSPBLOCKINFO;

#define MAX_BIT_ERRORS 12
struct tagNsPolicyData
{
    D_UINT32          ulRandom;
    unsigned int      nMaxBitErrors;
    unsigned          fIsPersistent : 1;
    NsDevDimensions * pDevDim;
    NSPBLOCKINFO     *pBlockInfo;
    NsOperationData   ReadDisturb;
    NsOperationData   Write;
    NsOperationData   Erase;
    NsPolicyBitError  aBitErrors[MAX_BIT_ERRORS];
};


/*-------------------------------------------------------------------
    Default settings.
-------------------------------------------------------------------*/

static NsPolicyData DefaultPolicy =
{
    653589793,   /* ulRandom - default seed */
    4,           /* nMaxBitErrors - simulate MLC */
    FALSE        /* fIsPersistent */
};


/* ulErrorThreshold is set dynamically at run time.  Generate only
   single-bit errors spread across the entire erase block.
*/
static NsOperationData DefaultReadDisturb =
{
    0,   /* ulErrorThreshold - set dynamically at run time */
    0,   /* iPercentTimeoutError - none */
    0,   /* iPercentIoError - none */
    0,   /* iPercentPageBitError - none */
    100  /* iPercentBlockBitError - all */
};

static NsOperationData DefaultWrite =
{
    1,   /* ulErrorThreshold (per million) */
    0,   /* iPercentTimeoutError - none */
    1,   /* iPercentIoError - none */
    9,   /* iPercentPageBitError */
    90   /* iPercentBlockBitError */
};

static NsOperationData DefaultErase =
{
    1,   /* ulErrorThreshold (per million) */
    0,   /* iPercentTimeoutError - none */
    100, /* iPercentIoError - all */
    0,   /* iPercentPageBitError - none */
    0    /* iPercentBlockBitError - none */
};


/*-------------------------------------------------------------------
    Random()

    Description
        This function returns a random number between 0 and ulScale -1.

    Parameters
        pAction  - a pointer to the action to be updated.
        ulScale  - the scale factor to be applied.

    Return Value
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
    Error()

    Description
        This function generates an error of random type.

    Parameters
        hPolicy  - a handle to the policy layer.
        pOperation - a pointer to the operation-specific data.
        pAction  - a pointer to the action to be updated.
        ulSize   - the size of the area for bit errors, in bytes.

    Return Value
        (none)
-------------------------------------------------------------------*/
static void Error(
    NsPolicyHandle    hPolicy,
    NsOperationData * pOperation,
    NsPolicyAction  * pAction,
    D_UINT32          ulSize )
{
    D_INT16 iTypeFactor;

    DclAssert( hPolicy );
    pAction->ulNumBitErrors = 0; /* safety */
    pAction->pBitErrors     = hPolicy->aBitErrors; /* common */

    iTypeFactor = (D_INT16) Random( &hPolicy->ulRandom, 100 );
    if ( (iTypeFactor -= pOperation->iPercentTimeoutError) < 0 )
    {
        pAction->error = NsPolicyError_Timeout;
    }
    else if ( (iTypeFactor -= pOperation->iPercentIoError) < 0 )
    {
        pAction->error = NsPolicyError_IO;
    }
    else
    {
        if ( (iTypeFactor -= pOperation->iPercentPageBitError) < 0 )
        {
            pAction->error = NsPolicyError_PageBit;
            pAction->ulNumBitErrors = 1;
            hPolicy->aBitErrors[0].state = Random(&hPolicy->ulRandom, NsPolicyBitState_Count);
            hPolicy->aBitErrors[0].ulOffset = Random(&hPolicy->ulRandom, ulSize * CHAR_BIT);
        }
        else if ( (iTypeFactor -= pOperation->iPercentBlockBitError) < 0 )
        {
            /*  Create multiple bit errors within a single page in the
                block.
            */
            unsigned int i;
            D_UINT32 ulPageOffset =
                Random(&hPolicy->ulRandom, hPolicy->pDevDim->pagesPerBlock) * ulSize * CHAR_BIT;

            pAction->error = NsPolicyError_BlockBit;
            pAction->ulNumBitErrors = Random(&hPolicy->ulRandom, hPolicy->nMaxBitErrors) + 1;
            for (i = 0; i < pAction->ulNumBitErrors; ++i)
            {
                /*  Note that the generated errors are all confined to either
                    the main or spare area, as represented by ulSize.
                */
                hPolicy->aBitErrors[i].ulOffset =
                    ulPageOffset + Random(&hPolicy->ulRandom, ulSize * CHAR_BIT);
                hPolicy->aBitErrors[i].state = Random(&hPolicy->ulRandom, NsPolicyBitState_Count);
            }
        }
    }
}


/*-------------------------------------------------------------------
    ReadDisturbThreshold()

    Description
        This function determines the number of errors in
        one million reads, given the number of reads in
        this block since the last erase operation.

    Parameters
        ulReadsInBlockSinceLastErase  - Number of reads in the
            block since the last read operation.

    Return Value
        Number of times in a million that a read error is
        expected to occur.
-------------------------------------------------------------------*/
static D_UINT32 ReadDisturbThreshold(
    D_UINT32 ulReadsInBlockSinceLastErase )
{
    D_UINT32    ulErrorThreshold;

    /*  OK, here's the statement we're basing this policy on:
        Performing a high number (hundreds of thousands or millions) of READ
        operations on individual pages before an ERASE command for the block
        containing those pages can exacerbate this error.

        I'm interpreting this as:
        - The rate is non-linear
        - There is a non-zero chance of this occuring even on the first READ after ERASE
        - At 100000, the rate is higher
        - At 1000000, the rate is much higher

        Using that admittedly quite subjective interpretation, I'm just going to make
        up my own freqency distribution.  Hopefully someday someone will give me a
        valid one, and we can plug it in here:

        Number of Reads   Probability of Occurance
                          of Single-Bit Error Due
                          To Read Disturb Effect
              0                    0.00000001 (since that's the smallest non-zero number we
                                               can represent using the code currently in this
                                               module)
           100000                  0.0001     (I made this up.  It's less than the probability
                                               at one million, and more than zero, and results
                                               in a non-linear distribution.)
          1000000                  0.5        (I also made this up, same criteria.)

          Someone could also put this into a table.  Or MUCH better, find the actual curve
          (using something based in the actual physics or statistical analysis) and
          compute this at run-time using a formula.

    */
    if (ulReadsInBlockSinceLastErase > 1000000L)
    {
        ulErrorThreshold = 5000000L;
    } else if (ulReadsInBlockSinceLastErase > 100000L)
    {
        ulErrorThreshold = 1000;
    } else
    {
        ulErrorThreshold = 1;
    }
    return ulErrorThreshold;
}


/*-------------------------------------------------------------------
    Create()

    Description
        This will be called by the NAND Simulator once for each device
        that is configured to utilize this specific error policy.
        This function should perform whater initialization the policy
        requires to prepare it for use.  It must create and return
        a handle that will uniquely identify this instance of the
        policy.

    Parameters
        hDev      - An opaque pointer to a device on which this
                    module is to begin operating.  This module uses
                    this handle to acquire per-device configuration.
        pDevDim   - A pointer to the device dimensions.

    Return Value
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
    pPolicyData = DclMemAlloc(sizeof *pPolicyData);
    if (pPolicyData)
    {
        /*  Set defaults for the error policy.
         */
        *pPolicyData = DefaultPolicy;
        DclAssert(pPolicyData->nMaxBitErrors <= DCLDIMENSIONOF(pPolicyData->aBitErrors));
        pPolicyData->ReadDisturb = DefaultReadDisturb;
        pPolicyData->Write = DefaultWrite;
        pPolicyData->Erase = DefaultErase;

        /*  TODO: query options to override each of the defaults.
            Don't forget to range-check nMaxBitErrors.
        */

        pPolicyData->pDevDim = pDevDim;
        pPolicyData->pBlockInfo =
            DclMemAllocZero(pDevDim->totalBlocks * sizeof *pPolicyData->pBlockInfo);

        if (pPolicyData->pBlockInfo)
            return pPolicyData;

        DclMemFree(pPolicyData);
    }

    return NULL;
}


/*-------------------------------------------------------------------
    Destroy()

    Description
        This function is called by the NAND Simulator when the device
        is being destroyed.  This function should perform whatever
        cleanup this policy layer needs.

    Parameters
        hPolicy  - a handle to the policy layer.

    Return Value
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
    D_UINT32       blockNumber;
    D_UINT32       ulErrorThreshold;
    D_UINT32       ulChance;

    DclAssert(hPolicy);
    DclAssert(pParams);

    /*  Account for read disturb effect, IGNORING THAT READS IN THE
        SAME PAGE DON'T CONTRIBUTE TO THE READ DISTURB EFFECT!!!!
    */
    blockNumber = (D_UINT32)(pParams->ulPageNumber / pParams->uPagesPerBlock);
    hPolicy->pBlockInfo[blockNumber].nReads++;

    ulErrorThreshold = ReadDisturbThreshold(hPolicy->pBlockInfo[blockNumber].nReads);
    ulChance = Random(&hPolicy->ulRandom, ONE_MILLION);

    if ( ulChance < ulErrorThreshold )
    {
        Error(hPolicy, &hPolicy->ReadDisturb, &action, pParams->ulSize);
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

    if ( hPolicy->pBlockInfo[blockNumber].fFailed
         || Random(&hPolicy->ulRandom, ONE_MILLION)
            < hPolicy->Write.ulErrorThreshold )
    {
        Error(hPolicy, &hPolicy->Write, &action, pParams->ulSize);
        hPolicy->pBlockInfo[blockNumber].fFailed = hPolicy->fIsPersistent;
    }

    return action;
}


/*-------------------------------------------------------------------
    Public: ReadSpare

    This function is called to specify an action when a spare area is
    read.

    Parameters
        hPolicy  - a handle to the policy layer.
        pParams  - a pointer to the parameters to be used.

    Return Value
        NsPolicyAction - a structure containing the action requested of
        the caller.
-------------------------------------------------------------------*/
static NsPolicyAction ReadSpare(
    NsPolicyHandle hPolicy,
    NsPolicyPageParam const * pParams )
{
    NsPolicyAction action = { NsPolicyError_None };
    D_UINT32       blockNumber;
    D_UINT32       ulErrorThreshold;
    D_UINT32       ulChance;

    DclAssert(hPolicy);
    DclAssert(pParams);

    /*  Account for read disturb effect IGNORING THAT READS IN THE
        SAME PAGE DON'T CONTRIBUTE TO THE READ DISTURB EFFECT!!!!
    */
    blockNumber = (D_UINT32)(pParams->ulPageNumber / pParams->uPagesPerBlock);
    hPolicy->pBlockInfo[blockNumber].nReads++;


    ulErrorThreshold = ReadDisturbThreshold(hPolicy->pBlockInfo[blockNumber].nReads);
    ulChance = Random(&hPolicy->ulRandom, ONE_MILLION);

    if ( ulChance < ulErrorThreshold )
    {
        Error(hPolicy, &hPolicy->ReadDisturb, &action, pParams->ulSize);
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

    if (Random(&hPolicy->ulRandom, ONE_MILLION)
        < hPolicy->Write.ulErrorThreshold)
    {
        Error(hPolicy, &hPolicy->Write, &action, pParams->ulSize);
    }

    return action;
}


/*-------------------------------------------------------------------
    Public: EraseBlock

    Description
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

    /*  Reset the counter we use to simulate-ish the Read Disturb Effect
    */
    hPolicy->pBlockInfo[blockNumber].nReads = 0;

    if ( hPolicy->pBlockInfo[blockNumber].fFailed
         || Random(&hPolicy->ulRandom, ONE_MILLION)
            < hPolicy->Erase.ulErrorThreshold )
    {
        int iTypeFactor = (int) Random(&hPolicy->ulRandom, 100);

        if ((iTypeFactor -= hPolicy->Erase.iPercentTimeoutError) < 0)
        {
            action.error = NsPolicyError_Timeout;
        }
        else if ((iTypeFactor -= hPolicy->Erase.iPercentIoError) < 0)
        {
            action.error = NsPolicyError_IO; /* Bug 2235 */
        }

        hPolicy->pBlockInfo[blockNumber].fFailed = hPolicy->fIsPersistent;
    }

    return action;
}

#endif  /* FFXCONF_NANDSUPPORT */
