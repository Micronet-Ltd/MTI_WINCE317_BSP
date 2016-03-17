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
  jurisdictions.  Patents may be pending.

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

    This module contains functionality which is used to validate that 
    multithreaded synchronization mechanisms are operating properly.
    This API is specifically designed to validate synchronization 
    mechanisms for multiple-reader, or a single-writer style object.

    Typically this code would be used only in DEBUG mode, however it
    is constructed so that it can be used in RELEASE mode if needed.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dlsyncvalidate.c $
    Revision 1.1  2010/12/10 01:52:08Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <dcl.h>


/*  The counter token is a 32-bit value which consists of a concurrent
    reader count in the low 16-bits and a total write count in the
    high 16-bits.  Note the difference!  It is not expected that the
    concurrent reader count will ever approach the 16-bit limit, 
    however it is certainly possible (and likely) that the 16-bit
    write count will wrap.  

    If the total writes value is D_UINT16_MAX, that means that a 
    writer is currently active, therefore when the write count wraps, 
    D_UINT16_MAX is explicitly skipped.
*/    
#define TOTALWRITES(n)          ((n) >> 16)
#define CURRENTREADERS(n)       ((n) & D_UINT16_MAX)
#define CURRENTLYWRITING        D_UINT16_MAX
#define ISCURRENTLYWRITING(n)   (TOTALWRITES(n) == CURRENTLYWRITING)
#define WRITERTOKEN             (((D_UINT32)CURRENTLYWRITING) << 16)


static D_BOOL       CrcCheck(     DCLSYNCSTATE *pSync, D_BOOL fUpdate);
static const char * ProcessLocale(DCLSYNCSTATE *pSync, const char *pszLocale);


/*-------------------------------------------------------------------
    Protected: DclSyncValidateInitialize()

    Initialize the synchronization state data.

    This construct is designed to validate multithreaded synchronization
    protections in a "multiple-reader OR a single-writer" model.  This
    system does not <DO> multithreaded synchronization itself -- that is
    for higher level software.  However it does protect its internal 
    state data by using atomic operations.

    The system uses state data and tokens to ensure that entries to, and
    exits from, read and write operations are performed in a properly 
    synchronized fashion.  The system can optionally validate a memory
    range's integrity with a CRC.  

    Finally, within the DCLSYNCSTATE structure, an array of the last 
    DCL_SYNCVALIDATE_MAX_LOCALES users of the system is recorded in a
    circular buffer.  Should this system indicate a synchronization
    failure, the buffer can be examined to see who the most recent
    users were.

    Some requirements and limitations of this system:
    1) There is an inherent limit on the maximum number of concurrent
       readers of D_UINT16_MAX.
    2) The DCLSYNCSTATE structure <may> be included in the data structure
       identified by pData/nDataLen -- it will be excluded from the CRC.
    3) The DCLSYNCSTATE structure being used must be initialized to all
       zeros before the very first time a "ReadBegin" or "WriteBegin" is
       called, or DclSyncValidateInitialize() must be called with the
       fClearHistory flag set to TRUE.
    4) The memory location of the pData/nDataLen structure may not ever 
       be changed, unless the DCLSYNCSTATE structure is completely reset
       to zeros.  E.g., copying the whole pData/nDataLen structure to a
       new place in memory, requires that the DCLSYNCSTATE contents be
       reset to zeros.
    5) The content of the DCLSYNCSTATE structure must never be modified
       once it is used the first time, except in the case where it is
       completely initialized to zeros, or DclSyncValidateInitialize()
       is called.
 
    Parameters:
        pSyncState    - A pointer to a DCLSYNCSTATE structure to use.
        pData         - A pointer to the data to monitor.  May be NULL
                        if data CRCs are not to be performed.
        nDataLen      - The length of the data at pData.  Must be zero
                        if pData is NULL.
        fClearHistory - TRUE to clear the usage history array, or 
                        FALSE ot leave it unchanged.

    Return Value:
        None.
-------------------------------------------------------------------*/
void DclSyncValidateInitialize(
    DCLSYNCSTATE   *pSyncState,
    void           *pData,
    size_t          nDataLen,
    D_BOOL          fClearHistory)
{
    DclAssertWritePtr(pSyncState, sizeof(*pSyncState));

    if(fClearHistory)
    {
        DclMemSet(pSyncState, 0, sizeof(pSyncState));

        pSyncState->apAtomicLocale[0] = "SyncInit";
    }
    else
    {
        pSyncState->ulAtomicState = 0;
        pSyncState->ulAtomicCRC = 0;
    }
    
    if(pData)
    {
        DclProductionAssert(nDataLen);
        
        pSyncState->pData = pData;
        pSyncState->nDataLen = nDataLen;

        /*  Very first time, so set the CRC
        */
        CrcCheck(pSyncState, TRUE);
    }
    else
    {
        DclProductionAssert(!nDataLen);
    }

    return;
}


/*-------------------------------------------------------------------
    Protected: DclSyncValidateReadBegin()

    Perform synchronization validation for an object which is about to
    be read.  When this function returns a non-zero value, it must be 
    paired with a call to DclSyncValidateReadEnd().  If this function 
    returns zero, then there is a synchronization error, operations must 
    be aborted, and no call to the "end" function may be performed.

    See DclSyncValidateInitialize() for a complete description of this
    system.
  
    Parameters:
        pSyncState - A pointer to a DCLSYNCSTATE structure to use.
        pszLocale  - A pointer to a signature string of the caller.
                     May be NULL to skip recording the locale for
                     this event.

    Return Value:
        Returns a non-zero synchronization token if successful, or 
        0 if a writer is active.
-------------------------------------------------------------------*/
D_UINT32 DclSyncValidateReadBegin(
    DCLSYNCSTATE   *pSyncState,
    const char     *pszLocale)
{
    D_UINT32        ulToken;
    const char     *pszOldLocale;

    DclAssertWritePtr(pSyncState, sizeof(*pSyncState));

    /*  Get the old locale and store the new one (if specified)
    */
    pszOldLocale = ProcessLocale(pSyncState, pszLocale);

    /*  Increment the concurrent readers count
    */
    ulToken = DclOsAtomic32Increment(&pSyncState->ulAtomicState);

    /*  Concurrent readers should never wrap a 16-bit value.
    */
    DclProductionAssert(CURRENTREADERS(ulToken) != 0);

    if(ISCURRENTLYWRITING(ulToken))
    {
        /*  SYCHRONIZATION ERROR!  Decrement the counter and
            return a failure indicator.
        */    

        DclPrintf("SyncValidation error RB1: Reader \"%s\" interrupted writer \"%s\", State=%lX\n", 
            pszLocale, pszOldLocale, ulToken);
        
        DclOsAtomic32Decrement(&pSyncState->ulAtomicState);
        return 0;
    }

    if(pSyncState->pData)
    {
        /*  If this is level 1 reader, and there has never been a writer,
            stuff the data pointer and length into our DCLSYNCSTATE strucure.
            Technically we could have synchronization problems updating these
            variables, HOWEVER, that could only occur if there is a higher
            level violation, so don't bother using atomic operations to set
            these values.
        */        
        if(ulToken == 1)
        {
            /*  Very first time, so set the CRC
            */
            CrcCheck(pSyncState, TRUE);
        }
        else
        {
            if(!CrcCheck(pSyncState, FALSE))
            {
                /*  SYCHRONIZATION ERROR!  Decrement the counter and
                    return a failure indicator.
                */    
                DclPrintf("SyncValidation error RB2: Reader \"%s\" CRC invalid, previous locale was \"%s\"\n", 
                    pszLocale, pszOldLocale);
                
                DclOsAtomic32Decrement(&pSyncState->ulAtomicState);
                return 0;
            }
        }        
    }
    
    DCLPRINTF(2, ("SyncValidator RB: Old locale was \"%s\", new locale is \"%s\"\n", pszOldLocale, pszLocale));

    /*  Already confirmed via the checks above, but just for 
        clarity's sake...
    */        
    DclProductionAssert(ulToken != 0);

    return ulToken;
}


/*-------------------------------------------------------------------
    Protected: DclSyncValidateReadEnd()

    Perform synchronization validation for an object for which read
    access has been completed.  The non-zero token returned by
    DclSyncValidateReadBegin() must be passed as ulToken.
  
    Parameters:
        pAtomicSync - A pointer to a D_ATOMIC32 counter field.
        ulToken     - The original token value returned by
                      DclSyncValidateReadBegin().

    Return Value:
        Returns a DCLSTATUS code indicating the operation result.
-------------------------------------------------------------------*/
DCLSTATUS DclSyncValidateReadEnd(
    DCLSYNCSTATE   *pSyncState,
    D_UINT32        ulToken)
{
    D_UINT32        ulNewCount;

    DclAssertWritePtr(pSyncState, sizeof(*pSyncState));
    DclProductionAssert(ulToken);
    DclProductionAssert(CURRENTREADERS(ulToken) > 0);
    
    if(pSyncState->pData && !CrcCheck(pSyncState, FALSE))
    {
        /*  SYCHRONIZATION ERROR!  Decrement the counter and
            return a failure indicator.
        */    
        DclPrintf("SyncValidation error RE1: CRC invalid, previous locale was \"%s\"\n", 
            ProcessLocale(pSyncState, NULL));
        
        return DCLSTAT_SYNC_READENDCRCINVALID;
    }

    ulNewCount = DclOsAtomic32Decrement(&pSyncState->ulAtomicState);

    /*  Better not have wrapped...
    */
    DclProductionAssert(CURRENTREADERS(ulNewCount) != D_UINT16_MAX);

    /*  SYCHRONIZATION ERROR!  If the total write operations value
                               is different now than when the read
                               operation started, then there must 
                               be a synchronization problem, even
                               though the CRC was correct.
    */  
    if(TOTALWRITES(ulNewCount) != TOTALWRITES(ulToken))
        return DCLSTAT_SYNC_WRITEINTERRUPTEDREAD;

    return DCLSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    Protected: DclSyncValidateWriteBegin()

    Perform synchronization validation for an object which is 
    about to be written.  When this function returns a non-zero,
    and non-D_UINT32_MAX value, it must be paired with a call to 
    DclSyncValidateWritedEnd().  If either of those two values is
    returned, then there is a synchronization error, operations
    should be aborted, and no call to the "end" function may be
    performed.
 
    See DclSyncValidateInitialize() for a complete description of
    this system.
  
    Parameters:
        pSyncState - A pointer to a DCLSYNCSTATE structure to use.
        pszLocale  - A pointer to a signature string of the caller.
                     May be NULL to skip recording the locale for
                     this event.

    Return Value:
        Returns a non-zero, non-D_UINT32_MAX synchronization token if
        successful.  Returns 0 if a reader is active, or D_UINT32_MAX
        if another writer is active.
-------------------------------------------------------------------*/
D_UINT32 DclSyncValidateWriteBegin(
    DCLSYNCSTATE   *pSyncState,
    const char     *pszLocale)
{
    D_UINT32        ulCurrentCount;
    D_UINT32        ulOriginalCount;
    D_UINT32        ulNewCount = WRITERTOKEN;
    const char     *pszOldLocale;
    
    DclAssertWritePtr(pSyncState, sizeof(*pSyncState));

    /*  Get the old locale and store the new one (if specified)
    */
    pszOldLocale = ProcessLocale(pSyncState, pszLocale);

    ulCurrentCount = DclOsAtomic32Retrieve(&pSyncState->ulAtomicState);

    /*  SYCHRONIZATION ERROR!  Should not be starting a write
                               if someone is reading.
    */                               
    if(CURRENTREADERS(ulCurrentCount) > 0)
    {
        DclPrintf("SyncValidation error WB1: Writer \"%s\" interrupted reader \"%s\", State=%lX\n", 
            pszLocale, pszOldLocale, ulCurrentCount);
        
        return 0;
    }
    
    /*  SYCHRONIZATION ERROR!  Should not be starting a write
                               if someone is already writing.
    */                               
    if(ulCurrentCount == ulNewCount)
    {
        DclPrintf("SyncValidation error WB2: Writer \"%s\" interrupted writer \"%s\"\n", 
            pszLocale, pszOldLocale);
        
        return D_UINT32_MAX;
    }
    
    /*  Reality check -- better not be setting this to either of our
        special return values!
    */        
    DclProductionAssert(ulNewCount != 0);
    DclProductionAssert(ulNewCount != D_UINT32_MAX);

    ulOriginalCount = DclOsAtomic32CompareExchange(&pSyncState->ulAtomicState, ulCurrentCount, ulNewCount);
    if(ulOriginalCount != ulCurrentCount)
    {
        DclPrintf("SyncValidation error WB3: Expected %lX, but got %lX (OldLocale=%s NewLocale=%s)\n", 
            ulCurrentCount, ulOriginalCount, pszOldLocale, pszLocale);

        /*  SYCHRONIZATION ERROR!
        */                               
        if(CURRENTREADERS(ulOriginalCount) > 0)
            return 0;
        else
            return D_UINT32_MAX;
    }

    /*  If this is not the very first write, make sure the previous CRC 
        was valid.
    */        
    if(pSyncState->pData && (TOTALWRITES(ulOriginalCount) != 0))
    {
        if(!CrcCheck(pSyncState, FALSE))
        {
            /*  SYCHRONIZATION ERROR!  Decrement the counter and
                return a failure indicator.
            */    
            DclPrintf("SyncValidation error WB4: Writer \"%s\" CRC invalid, previous locale was \"%s\"\n", 
                pszLocale, pszOldLocale);

            /*  Restore the original count
            */
            DclOsAtomic32Exchange(&pSyncState->ulAtomicState, ulOriginalCount);

            return 0;
        }
    }
    
    DCLPRINTF(2, ("SyncValidator WB: Old locale was \"%s\", new locale is \"%s\"\n", pszOldLocale, pszLocale));

    /*  Ensure that incrementing the total write count does not ever
        end up using the reserved CURRENTLYWRITING value.  Just start
        over at 1 if so.
    */        
    if(TOTALWRITES(ulCurrentCount) + 1 == CURRENTLYWRITING)
        ulNewCount = ((D_UINT32)1) << 16;
    else
        ulNewCount = ((D_UINT32)(TOTALWRITES(ulCurrentCount) + 1)) << 16;

    DclProductionAssert(ulNewCount != 0);
    DclProductionAssert(ulNewCount != D_UINT32_MAX);

    return ulNewCount;
}


/*-------------------------------------------------------------------
    Protected: DclSyncValidateWriteEnd()

    Perform synchronization validation for an object for which write
    access has been completed.  The valid token which was returned by
    DclSyncValidateWriteBegin() must be passed as ulToken.
 
    Parameters:
        pAtomicSync - A pointer to a D_ATOMIC32 counter field.
        ulToken     - The original token value returned by
                      DclSyncValidateWriteBegin().

    Return Value:
        Returns a DCLSTATUS code indicating the operation result.
-------------------------------------------------------------------*/
DCLSTATUS DclSyncValidateWriteEnd(
    DCLSYNCSTATE   *pSyncState,
    D_UINT32        ulToken)
{
    D_UINT32        ulCurrentCount;
    D_UINT32        ulNewCount;

    DclAssertWritePtr(pSyncState, sizeof(*pSyncState));
    DclProductionAssert(ulToken != 0);
    DclProductionAssert(ulToken != D_UINT32_MAX);
    DclProductionAssert(CURRENTREADERS(ulToken) == 0);

    ulCurrentCount = DclOsAtomic32Retrieve(&pSyncState->ulAtomicState);

    /*  SYCHRONIZATION ERROR!
    */                               
    if(ulCurrentCount != WRITERTOKEN)
    {
        DclPrintf("SyncValidation error WE1: Expected %lX, but got %lX, Last locale was \"%s\"\n",
            WRITERTOKEN, ulCurrentCount, ProcessLocale(pSyncState, NULL));

        if(CURRENTREADERS(ulCurrentCount) > 0)
            return DCLSTAT_SYNC_READINTERRUPTEDWRITE;
        else
            return DCLSTAT_SYNC_WRITEINTERRUPTEDWRITE;
    }

    /*  Update the CRC for the newly written data
    */
    if(pSyncState->pData)
        CrcCheck(pSyncState, TRUE);

    ulNewCount = DclOsAtomic32CompareExchange(&pSyncState->ulAtomicState, ulCurrentCount, ulToken);
    if(ulNewCount == ulCurrentCount)
        return DCLSTAT_SUCCESS;

    DclPrintf("SyncValidation error WE2: Expected %lX, but got %lX, Last locale was \"%s\"\n",
        ulCurrentCount, ulNewCount, ProcessLocale(pSyncState, NULL));

    /*  SYCHRONIZATION ERROR!
    */                               
    if(CURRENTREADERS(ulNewCount) > 0)
        return DCLSTAT_SYNC_READINTERRUPTEDWRITE;
    else
        return DCLSTAT_SYNC_WRITEINTERRUPTEDWRITE;
}


/*-------------------------------------------------------------------
    Protected: DclSyncValidateReadToWrite()

    Transition from a reader to a writer.  When this function
    returns a non-zero value, it must be paired with a call to
    DclSyncValidateWriteEnd() (or transition back to a reader).
 
    Parameters:
        pAtomicSync - A pointer to a D_ATOMIC32 counter field.
        ulToken     - The token returned by DclSyncValidateReadBegin()
        pszLocale   - A pointer to a signature string of the caller.
                      May be NULL to skip recording the locale for
                      this event.

    Return Value:
        Returns a non-zero, non-D_UINT32_MAX synchronization token if
        successful.  Returns 0 if the read-end failed, or D_UINT32_MAX
        if another writer is active.
-------------------------------------------------------------------*/
D_UINT32 DclSyncValidateReadToWrite(
    DCLSYNCSTATE   *pSyncState,
    D_UINT32        ulToken,
    const char     *pszLocale)
{
    DCLSTATUS       dclStat;
    
    DclAssertWritePtr(pSyncState, sizeof(*pSyncState));

    dclStat = DclSyncValidateReadEnd(pSyncState, ulToken);
    if(dclStat != DCLSTAT_SUCCESS)
        return 0;

    return DclSyncValidateWriteBegin(pSyncState, pszLocale);
}


/*-------------------------------------------------------------------
    Protected: DclSyncValidateWriteToRead()

    Transition from a writer to a reader.  When this function
    returns a non-zero value, it must be paired with a call to
    DclSyncValidateReadEnd() (or transition back to a writer).
 
    Parameters:
        pAtomicSync - A pointer to a D_ATOMIC32 counter field.
        ulToken     - The token returned by DclSyncValidateWriteBegin()
        pszLocale   - A pointer to a signature string of the caller.
                      May be NULL to skip recording the locale for
                      this event.

    Return Value:
        Returns a non-zero synchronization token if successful, or 
        0 if the transition failed.
-------------------------------------------------------------------*/
D_UINT32 DclSyncValidateWriteToRead(
    DCLSYNCSTATE   *pSyncState,
    D_UINT32        ulToken,
    const char     *pszLocale)
{
    DCLSTATUS       dclStat;
    
    DclAssertWritePtr(pSyncState, sizeof(*pSyncState));

    dclStat = DclSyncValidateWriteEnd(pSyncState, ulToken);
    if(dclStat != DCLSTAT_SUCCESS)
        return 0;

    return DclSyncValidateReadBegin(pSyncState, pszLocale);
}


/*-------------------------------------------------------------------
    Local: CrcCheck()

    Get the CRC for the pData/nDataLen memory range.  If fUpdate is
    TRUE, then the CRC will be recorded in the pSyncState structure.
    If fUpdate is FALSE, the the CRC will be validated against the
    original CRC.

    Parameters:
        pAtomicSync - A pointer to a D_ATOMIC32 counter field.
        fUpdate     - TRUE to update, FALSE to validate.

    Return Value:
        Returns TRUE if everything is A-OK, or FALSE if the CRC
        validation fails.
-------------------------------------------------------------------*/
static D_BOOL CrcCheck(
    DCLSYNCSTATE   *pSyncState,
    D_BOOL          fUpdate)
{
    D_UINT32        ulCRC = 0;
    const D_BUFFER *pSync;
    const D_BUFFER *pStart;
    size_t          nLen;

    DclAssertWritePtr(pSyncState, sizeof(*pSyncState));
    DclProductionAssert(pSyncState->nDataLen);
    DclAssertReadPtr(pSyncState->pData, pSyncState->nDataLen);

    pSync = (const D_BUFFER*)pSyncState;
    pStart = (const D_BUFFER*)pSyncState->pData;
    nLen =  pSyncState->nDataLen;

    DclProductionAssert(nLen);

    /*  Handle the case where the DCLSYNCSTATE structure is encapsulated
        in pData/nDataLen -- we don't want to CRC the DCLSYNCSTATE data.
    */        
    if((pStart < pSync) && (pStart + nLen > pSync))
    {
        size_t nDiff = pSync - pStart;
            
        ulCRC = DclCrc32Update(ulCRC, pStart, nDiff);

        nDiff += sizeof(*pSyncState);

        if(nDiff < nLen)
        {
            pStart += nDiff;
            nLen = nLen - nDiff;
        }
        else
        {
            nLen = 0;
        }
    }

    /*  Update the CRC for any remaining data
    */
    if(nLen)
        ulCRC = DclCrc32Update(ulCRC, pStart, nLen);

    if(fUpdate)
    {
        DclOsAtomic32Exchange(&pSyncState->ulAtomicCRC, ulCRC);
    }
    else
    {
        if(DclOsAtomic32Retrieve(&pSyncState->ulAtomicCRC) != ulCRC)
            return FALSE;
    }
    
    return TRUE;
}


/*-------------------------------------------------------------------
    Local: ProcessLocale()

    Get the original locale value and store the new one if specified.

    Parameters:
        pSync     - A pointer to the DCLSYNCSTATE structure to use.
        pszLocale - The new locale value.  May be NULL to skip storing
                    an new value.

    Return Value:
        Returns the previous locale value.
-------------------------------------------------------------------*/
static const char * ProcessLocale(
    DCLSYNCSTATE   *pSync,
    const char     *pszLocale)
{
    D_UINT32        ulIndex;
    const char     *pszOldLocale;

    DclAssertWritePtr(pSync, sizeof(*pSync));

    while(TRUE)
    {
        /*  Set a D_UINT32_MAX flag indicating "busy"
        */
        ulIndex = DclOsAtomic32Exchange(&pSync->ulAtomicLocaleIndex, D_UINT32_MAX);
        if(ulIndex != D_UINT32_MAX)
            break;
    }

    /*  Grab the current locale value
    */
    pszOldLocale = DclOsAtomicPtrRetrieve(&pSync->apAtomicLocale[ulIndex]);

    /*  If we are setting a new locale increment the index, wrapping
        as needed.
    */        
    if(pszLocale)
    {
        ulIndex++;
        if(ulIndex == DCLDIMENSIONOF(pSync->apAtomicLocale))
            ulIndex = 0;

        /*  Store the new locale
        */
        DclOsAtomicPtrExchange(&pSync->apAtomicLocale[ulIndex], (void*)pszLocale);
    }

    /*  Remove the "busy" flag, and store the (possibly) updated index
    */
    DclOsAtomic32Exchange(&pSync->ulAtomicLocaleIndex, ulIndex);

    return pszOldLocale;
}


