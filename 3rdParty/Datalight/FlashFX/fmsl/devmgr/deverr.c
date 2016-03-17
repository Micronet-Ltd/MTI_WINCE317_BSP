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

    This module contains the FlashFX Error Injection Test.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: deverr.c $
    Revision 1.12  2010/07/31 19:47:15Z  garyp
    Updated to work around the "VA64BUG" issue.
    Revision 1.11  2010/07/08 20:19:02Z  garyp
    Enhanced the debugging and diagnostics messages -- no functional
    changes.
    Revision 1.10  2010/07/08 03:58:25Z  garyp
    Added the ability to change back and forth between manual and
    random injection of errors.
    Revision 1.9  2009/04/03 20:18:55Z  davidh
    Function header error corrected for AutoDoc.
    Revision 1.8  2009/04/01 17:18:13Z  davidh
    Function Headers updated for AutoDoc.
    Revision 1.7  2008/01/13 07:27:59Z  keithg
    Function header updates to support autodoc.
    Revision 1.6  2007/11/05 18:47:29Z  Garyp
    Refactored the preprocessor conditionals to allow the read/write/erase
    tests to be individually enabled or disabled.
    Revision 1.5  2007/11/03 23:49:35Z  Garyp
    Updated to use the standard module header.
    Revision 1.4  2007/03/03 01:39:51Z  timothyj
    Modified interface to use page numbers in lieu of linear byte
    offsets, for large flash array support.
    Revision 1.3  2006/10/04 00:35:59Z  Garyp
    Updated to use DclMemAllocZero().
    Revision 1.2  2006/02/24 04:26:16Z  Garyp
    Updated to work in the new Devices and Disks model.
    Revision 1.1  2006/02/10 09:41:12Z  Garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <flashfx.h>

#if FFXCONF_ERRORINJECTIONTESTS

#include <deverr.h>
#include <fxiosys.h>

typedef struct FFXERRORINJECT
{
    FFXDEVHANDLE    hDev;
    D_UINT64        ullTotalReads;
    D_UINT64        ullTotalWrites;
    D_UINT64        ullTotalErases;
    D_UINT32        ulSeed;
    D_UINT32        ulReadCounter;          /* operations until the next error  */
    D_UINT32        ulWriteCounter;         /* operations until the next error  */
    D_UINT32        ulEraseCounter;         /* operations until the next error  */
    D_UINT32        ulReadErrorsInjected;   /* count of errors injected         */
    D_UINT32        ulWriteErrorsInjected;  /* count of errors injected         */
    D_UINT32        ulEraseErrorsInjected;  /* count of errors injected         */
    unsigned        fManualReadBitErrors : 1;
    unsigned        fManualWriteIOErrors : 1;
    unsigned        fManualEraseIOErrors : 1;
} FFXERRORINJECT;


/*-------------------------------------------------------------------
    Protected: FfxDevErrorInjectCreate()

    This function initializes the error injection test for the
    specified Device.

    Parameters:
        hDev   - The FFXDEVHANDLE to use
        ulSeed - The random seed to start with

    Return Value:
        Returns an ERRINJHANDLE if successful, otherwise NULL.
-------------------------------------------------------------------*/
ERRINJHANDLE FfxDevErrorInjectCreate(
    FFXDEVHANDLE    hDev,
    D_UINT32        ulSeed)
{
    FFXERRORINJECT *pEI = NULL;

    DclAssert(hDev);

    if(!ulSeed)
        ulSeed++;

    pEI = DclMemAllocZero(sizeof *pEI);
    if(!pEI)
        return NULL;

    pEI->ulSeed = ulSeed;
    pEI->hDev = hDev;

    /*  Calculate the initial countdown values until the next error
        is generated, for each class of I/O.

        The goal here is to average I/O errors at the specified rate, not
        the have an I/O error some exact number of operations.
    */

  #if FFX_ERRINJRATE_READ
    pEI->ulReadCounter = (DclRand(&pEI->ulSeed) % ((1000000UL/FFX_ERRINJRATE_READ)*2)) + 1;
  #else
    pEI->ulReadCounter = 0;
  #endif
  #if FFX_ERRINJRATE_WRITE
    pEI->ulWriteCounter = (DclRand(&pEI->ulSeed) % ((1000000UL/FFX_ERRINJRATE_WRITE)*2)) + 1;
  #else
    pEI->ulWriteCounter = 0;
  #endif
  #if FFX_ERRINJRATE_ERASE
    pEI->ulEraseCounter = (DclRand(&pEI->ulSeed) % ((1000000UL/FFX_ERRINJRATE_ERASE)*2)) + 1;
  #else
    pEI->ulEraseCounter = 0;
  #endif

    DclPrintf("Error Injection System initialized for hDev=%P, Seed=%lX Read=%lU Write=%lU Erase=%lU\n",
        hDev, ulSeed, pEI->ulReadCounter, pEI->ulWriteCounter, pEI->ulEraseCounter);

    return (ERRINJHANDLE)pEI;
}


/*-------------------------------------------------------------------
    Public: FfxDevErrorInjectDestroy()

    This function display error injection tests statistics and
    destroys the specified instance of the error injection test.

    Parameters:
        hEI - The ERRINJHANDLE to use

    Return Value:
        None
 -------------------------------------------------------------------*/
void FfxDevErrorInjectDestroy(
    ERRINJHANDLE    hEI)
{
    DclAssert(hEI);

    DclPrintf("Error Injection system uninitialized for hDev=%P, injected %lU read, %lU write, %lU erase errors\n",
        hEI->hDev, hEI->ulReadErrorsInjected, hEI->ulWriteErrorsInjected, hEI->ulEraseErrorsInjected);

    DclMemFree(hEI);

    return;
}


/*-------------------------------------------------------------------
    Public: FfxDevErrorInjectParamSet()

    Set a FlashFX system parameter.  If the parameter ID is not
    recognized for this layer, the request will be passed to the
    next lower software layer.

    Parameters:
        hEI       - The Error Injection instance handle
        id        - The parameter identifier (FFXPARAM_*).
        pBuffer   - A pointer to the parameter data.
        ulBuffLen - The parameter data length.

    Return Value:
        Returns an FFXSTATUS code indicating success or failure.
-------------------------------------------------------------------*/
FFXSTATUS FfxDevErrorInjectParamSet(
    ERRINJHANDLE    hEI,
    FFXPARAM        id,
    const void     *pBuffer,
    D_UINT32        ulBuffLen)
{
    FFXSTATUS       ffxStat = FFXSTAT_BADPARAMETER;

    DclAssert(hEI);
    
    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_CONFIG, 1, TRACEINDENT),
        "FfxDevErrorInjectParamSet() hEI=%P hDev=%P ID=%x pBuff=%P Len=%lU\n",
        hEI, hEI->hDev, id, pBuffer, ulBuffLen));

    DclAssert(id >= FFXPARAM_FIM_ERRINJECT_MIN && id <= FFXPARAM_FIM_ERRINJECT_MAX);

    switch(id)
    {
      #if FFXCONF_ERRORINJECTIONTESTS
        case FFXPARAM_FIM_ERRINJECT_DISABLE:
            hEI->fManualReadBitErrors = TRUE;
            hEI->fManualWriteIOErrors = TRUE;
            hEI->fManualEraseIOErrors = TRUE;
            hEI->ulReadCounter = 0;
            hEI->ulWriteCounter = 0;
            hEI->ulEraseCounter = 0;

            DclPrintf("FfxDevErrorInjectParamSet() RANDOM error injection is disabled\n");
            ffxStat = FFXSTAT_SUCCESS;
            break;
            
        case FFXPARAM_FIM_ERRINJECT_READIO:
            DclAssert(ulBuffLen == sizeof(hEI->ulReadCounter));
            hEI->ulReadCounter = *(D_UINT32*)pBuffer;
            if(hEI->ulReadCounter == D_UINT32_MAX)
            {
              #if FFX_ERRINJRATE_READ
                hEI->fManualReadBitErrors = FALSE;
                hEI->ulReadCounter = (DclRand(&hEI->ulSeed) % ((1000000UL/FFX_ERRINJRATE_READ)*2)) + 1;
                DclPrintf("FfxDevErrorInjectParamSet() Switching to RANDOM read error injection\n");
              #else
                hEI->fManualReadBitErrors = TRUE;
                hEI->ulReadCounter = 0;
              #endif
            }
            else
            {
                if(!hEI->fManualReadBitErrors)
                {
                    hEI->fManualReadBitErrors = TRUE;
                    DclPrintf("FfxDevErrorInjectParamSet() Switching to EXPLICIT read error injection\n");
                }
            }
            
            ffxStat = FFXSTAT_SUCCESS;
            break;

        case FFXPARAM_FIM_ERRINJECT_WRITEIO:
            DclAssert(ulBuffLen == sizeof(hEI->ulWriteCounter));
            hEI->ulWriteCounter = *(D_UINT32*)pBuffer;
            if(hEI->ulWriteCounter == D_UINT32_MAX)
            {
              #if FFX_ERRINJRATE_WRITE
                hEI->fManualWriteIOErrors = FALSE;
                hEI->ulWriteCounter = (DclRand(&hEI->ulSeed) % ((1000000UL/FFX_ERRINJRATE_WRITE)*2)) + 1;
                DclPrintf("FfxDevErrorInjectParamSet() Switching to RANDOM write error injection\n");
              #else
                hEI->fManualWriteIOErrors = TRUE;
                hEI->ulWriteCounter = 0;
              #endif
            }
            else
            {
                if(!hEI->fManualWriteIOErrors)
                {
                    hEI->fManualWriteIOErrors = TRUE;
                    DclPrintf("FfxDevErrorInjectParamSet() Switching to EXPLICIT write error injection\n");
                }
            }
            
            ffxStat = FFXSTAT_SUCCESS;
            break;

        case FFXPARAM_FIM_ERRINJECT_ERASEIO:
            DclAssert(ulBuffLen == sizeof(hEI->ulEraseCounter));
            hEI->ulEraseCounter = *(D_UINT32*)pBuffer;
            if(hEI->ulEraseCounter == D_UINT32_MAX)
            {
              #if FFX_ERRINJRATE_ERASE
                hEI->fManualEraseIOErrors = FALSE;
                hEI->ulEraseCounter = (DclRand(&hEI->ulSeed) % ((1000000UL/FFX_ERRINJRATE_ERASE)*2)) + 1;
                DclPrintf("FfxDevErrorInjectParamSet() Switching to RANDOM erase error injection\n");
              #else
                hEI->fManualEraseIOErrors = TRUE;
                hEI->ulEraseCounter = 0;
              #endif
            }
            else
            {
                if(!hEI->fManualEraseIOErrors)
                {
                    hEI->fManualEraseIOErrors = TRUE;
                    DclPrintf("FfxDevErrorInjectParamSet() Switching to EXPLICIT erase error injection\n");
                }
            }
            
            ffxStat = FFXSTAT_SUCCESS;
            break;

      #endif
            
        default:
            /*  Not a parameter ID we recognize, and no place else to pass it
            */
            FFXPRINTF(1, ("FfxDevErrorInjectParamSet() unhandled parameter ID=%x\n", id));
            break;
    }

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_CONFIG, 1, TRACEUNDENT),
        "FfxDevErrorInjectParamSet() returning status=%lX\n", ffxStat));

    return ffxStat;
}


/*-------------------------------------------------------------------
    Public: FfxDevErrorInjectRead()

    Parameters:

    Return Value:
        None
-------------------------------------------------------------------*/
void FfxDevErrorInjectRead(
    ERRINJHANDLE    hEI,
    const char     *pszOpName,
    D_UINT32        ulPage,
    D_BUFFER       *pBuff,
    D_UINT16        uBuffLen,
    FFXIOSTATUS    *pIOStat)
{
    DclAssert(hEI);
    DclAssert(pszOpName);
    DclAssert(pBuff);
    DclAssert(uBuffLen);
    DclAssert(pIOStat);

    /*  If the original operation was not successful, don't
        try injecting anything...
    */
    if(pIOStat->ffxStat != FFXSTAT_SUCCESS)
        return;

    hEI->ullTotalReads++;

    if(hEI->ulReadCounter == 0)
        return;

    if(hEI->ulReadCounter == 1)
    {
        D_UINT32        ulByte;
        D_UINT32        ulBit;
        D_UINT8         bOld;

        ulBit = DclRand(&hEI->ulSeed) % (uBuffLen * 8);
        ulByte = ulBit / 8;
        ulBit %= 8;

        bOld = pBuff[ulByte];

        pBuff[ulByte] = bOld ^ (1 << ulBit);

        /*  If in manual mode, decrementing the value sets it to zero which
            will disable these errors until somebody resets the counter.  If
            in RANDOM mode, then calculate the number of operations until the
            next failure is injected (can never be zero).
        */
      #if FFX_ERRINJRATE_READ
        if(!hEI->fManualReadBitErrors)
            hEI->ulReadCounter = (DclRand(&hEI->ulSeed) % ((1000000UL/FFX_ERRINJRATE_READ)*2)) + 1;
        else
      #endif
            hEI->ulReadCounter--;
      
        hEI->ulReadErrorsInjected++;

        DclPrintf("##############################################################################\n");
        DclPrintf("##                    ERROR INJECTION for hDev=%P\n", hEI->hDev);
        DclPrintf("## Op='%s' Page=%lX[%03X] from %02X to %02X\n", pszOpName, ulPage, ulByte, bOld, pBuff[ulByte]);
        DclPrintf("##   Totals:  Reads=%9llU ErrorsInjected=%9lU NextError=%9lU\n", VA64BUG(hEI->ullTotalReads),  hEI->ulReadErrorsInjected,  hEI->ulReadCounter);
        DclPrintf("##           Writes=%9llU ErrorsInjected=%9lU NextError=%9lU\n", VA64BUG(hEI->ullTotalWrites), hEI->ulWriteErrorsInjected, hEI->ulWriteCounter);
        DclPrintf("##           Erases=%9llU ErrorsInjected=%9lU NextError=%9lU\n", VA64BUG(hEI->ullTotalErases), hEI->ulEraseErrorsInjected, hEI->ulEraseCounter);
        DclPrintf("##############################################################################\n");
    }
    else
    {
        hEI->ulReadCounter--;
    }

    return;
}


/*-------------------------------------------------------------------
    Public: FfxDevErrorInjectWrite()

    Parameters:

    Return Value:
        None
-------------------------------------------------------------------*/
void FfxDevErrorInjectWrite(
    ERRINJHANDLE    hEI,
    const char     *pszOpName,
    D_UINT32        ulPage,
    FFXIOSTATUS    *pIOStat)
{
    DclAssert(hEI);
    DclAssert(pszOpName);
    DclAssert(pIOStat);

    /*  If the original operation was not successful, don't
        try injecting anything...
    */
    if(pIOStat->ffxStat != FFXSTAT_SUCCESS)
        return;

    hEI->ullTotalWrites++;

    /*  If the operation was returning successfully, the count of elements
        must always be at least one.
    */
    DclAssert(pIOStat->ulCount);

    if(hEI->ulWriteCounter == 0)
        return;

    if(hEI->ulWriteCounter == 1)
    {
        pIOStat->ffxStat = FFXSTAT_FIMIOERROR;

        /*  By reducing this count by one, we are indicating that the last
            element of whatever we were writing failed.
        */
        pIOStat->ulCount--;

        /*  If in manual mode, decrementing the value sets it to zero which
            will disable these errors until somebody resets the counter.  If
            in RANDOM mode, then calculate the number of operations until the
            next failure is injected (can never be zero).
        */
      #if FFX_ERRINJRATE_WRITE
        if(!hEI->fManualWriteIOErrors)
            hEI->ulWriteCounter = (DclRand(&hEI->ulSeed) % ((1000000UL/FFX_ERRINJRATE_WRITE)*2)) + 1;
        else
      #endif
            hEI->ulWriteCounter--;

        hEI->ulWriteErrorsInjected++;

        DclPrintf("##############################################################################\n");
        DclPrintf("##                    ERROR INJECTION for hDev=%P\n", hEI->hDev);
        DclPrintf("## Op='%s' Page=%lX, I/O error injected for element %lU\n", pszOpName, ulPage, pIOStat->ulCount);
        DclPrintf("##   Totals:  Reads=%9llU ErrorsInjected=%9lU NextError=%9lU\n", VA64BUG(hEI->ullTotalReads),  hEI->ulReadErrorsInjected,  hEI->ulReadCounter);
        DclPrintf("##           Writes=%9llU ErrorsInjected=%9lU NextError=%9lU\n", VA64BUG(hEI->ullTotalWrites), hEI->ulWriteErrorsInjected, hEI->ulWriteCounter);
        DclPrintf("##           Erases=%9llU ErrorsInjected=%9lU NextError=%9lU\n", VA64BUG(hEI->ullTotalErases), hEI->ulEraseErrorsInjected, hEI->ulEraseCounter);
        DclPrintf("##############################################################################\n");
    }
    else
    {
        hEI->ulWriteCounter--;
    }

    return;
}


/*-------------------------------------------------------------------
    Public: FfxDevErrorInjectErase()

    Parameters:

    Return Value:
        None
-------------------------------------------------------------------*/
void FfxDevErrorInjectErase(
    ERRINJHANDLE    hEI,
    const char     *pszOpName,
    D_UINT32        ulBlock,
    FFXIOSTATUS    *pIOStat)
{
    DclAssert(hEI);
    DclAssert(pszOpName);
    DclAssert(pIOStat);

    /*  If the original operation was not successful, don't
        try injecting anything...
    */
    if(pIOStat->ffxStat != FFXSTAT_SUCCESS)
        return;

    hEI->ullTotalErases++;

    /*  If the operation was returning successfully, the count of elements
        must always be at least one.
    */
    DclAssert(pIOStat->ulCount);

    if(hEI->ulEraseCounter == 0)
        return;

    if(hEI->ulEraseCounter == 1)
    {
        pIOStat->ffxStat = FFXSTAT_FIMIOERROR;

        /*  By reducing this count by one, we are indicating that the last
            element of whatever we were writing failed.
        */
        pIOStat->ulCount--;

        /*  If in manual mode, decrementing the value sets it to zero which
            will disable these errors until somebody resets the counter.  If
            in RANDOM mode, then calculate the number of operations until the
            next failure is injected (can never be zero).
        */
      #if FFX_ERRINJRATE_ERASE
        if(!hEI->fManualEraseIOErrors)
            hEI->ulEraseCounter = (DclRand(&hEI->ulSeed) % ((1000000UL/FFX_ERRINJRATE_ERASE)*2)) + 1;
        else
      #endif
            hEI->ulEraseCounter--;

        hEI->ulEraseErrorsInjected++;

        DclPrintf("##############################################################################\n");
        DclPrintf("##                    ERROR INJECTION for hDev=%P\n", hEI->hDev);
        DclPrintf("## Op='%s' Block=%lX, I/O error injected for element %lU\n", pszOpName, ulBlock, pIOStat->ulCount);
        DclPrintf("##   Totals:  Reads=%9llU ErrorsInjected=%9lU NextError=%9lU\n", VA64BUG(hEI->ullTotalReads),  hEI->ulReadErrorsInjected,  hEI->ulReadCounter);
        DclPrintf("##           Writes=%9llU ErrorsInjected=%9lU NextError=%9lU\n", VA64BUG(hEI->ullTotalWrites), hEI->ulWriteErrorsInjected, hEI->ulWriteCounter);
        DclPrintf("##           Erases=%9llU ErrorsInjected=%9lU NextError=%9lU\n", VA64BUG(hEI->ullTotalErases), hEI->ulEraseErrorsInjected, hEI->ulEraseCounter);
        DclPrintf("##############################################################################\n");
    }
    else
    {
        hEI->ulEraseCounter--;
    }

    return;
}


#endif  /* FFXCONF_ERRORINJECTIONTESTS */

