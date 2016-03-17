/*---------------------------------------------------------------------------
               ----> DO NOT REMOVE THE FOLLOWING NOTICE <----

                  Copyright (c) 1993 - 2011 Datalight, Inc.
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

    This module contains general functions for manipulating log files and
    log data.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dllogservice.c $
    Revision 1.12  2011/05/04 16:42:51Z  daniel.lewis
    Corrected erroneous use of DclMemFree() on a mutex.
    Revision 1.11  2011/04/18 02:10:29Z  daniel.lewis
    Eliminated mutex leaks. Updated to function without a log file.
    Revision 1.10  2011/04/14 21:34:37Z  daniel.lewis
    Implemented an atomic algorithm for writing to the log.
    Revision 1.9  2010/11/02 20:06:57Z  garyp
    Added a typecast to fix some D_BUFFER to char translation.
    Revision 1.8  2010/11/01 16:59:51Z  garyp
    Minor parameter type update.
    Revision 1.7  2010/11/01 06:24:16Z  garyp
    Fixed a logic flaw in the previous revision.
    Revision 1.6  2010/11/01 03:24:22Z  garyp
    Modified DclLogOpen() to take a flags parameter rather than a whole
    bunch of bools.  Updated to support binary logs.  Renamed DclLogWrite()
    to DclLogWriteString() to avoid confusion with the new logging capability
    of handling binary logs.
    Revision 1.5  2010/05/19 16:46:16Z  garyp
    Modified so that for private logs, if the fShadow option is used, the
    output will be shadowed using the standard output function (previously
    the combination was disallowed).
    Revision 1.4  2009/12/31 17:24:18Z  billr
    Refactor headers for better separation between 0S-dependent and
    OS-independent code.
    Revision 1.3  2009/09/10 18:25:50Z  garyp
    Added a missing typecast.
    Revision 1.2  2009/07/01 21:37:44Z  garyp
    Fixed to properly initialize a local variable.  Updated debug code.
    Revision 1.1  2009/06/23 22:08:06Z  garyp
    Initial revision
    Revision 1.12.1.6  2009/03/30 02:58:26Z  garyp
    Documentation fixes -- no functional changes.
    Revision 1.12.1.5  2009/03/19 02:58:11Z  garyp
    Documentation updates only -- no functional changes.
    Revision 1.12.1.4  2009/03/05 15:20:25Z  garyp
    Corrected the logic used to prevent being re-entered.
    Revision 1.12.1.3  2008/12/12 23:56:20Z  garyp
    Fixed a problem with the previous rev.
    Revision 1.12.1.2  2008/12/10 22:57:31Z  garyp
    Updated to implement logging as a dynamically created output service.
    Revision 1.12  2008/05/01 21:13:45Z  garyp
    Updated to build cleanly when output is disabled.
    Revision 1.11  2008/04/07 22:03:39Z  brandont
    Updated to use the new DCL file system services.
    Revision 1.10  2008/04/03 23:55:33Z  brandont
    Updated all defines and structures used by the DCL file system
    services to use the DCLFS prefix.
    Revision 1.9  2007/12/18 04:51:50Z  brandont
    Updated function headers.
    Revision 1.8  2007/11/03 05:00:59Z  Garyp
    Documentation corrections.
    Revision 1.7  2007/10/06 01:38:16Z  brandont
    Added the assignment of the DclLogClose function pointer that will
    only be set if DclLogOpen is called.
    Revision 1.6  2007/04/06 01:34:27Z  Garyp
    Added the DclLogWrite() function.
    Revision 1.5  2007/03/06 03:46:51Z  Garyp
    Updated to use a renamed setting.
    Revision 1.4  2006/12/28 02:30:41Z  Garyp
    Updated to support instantiating a private log instance, which is not hooked
    into the standard output mechanisms.  The only way to write to a private
    log is using the new functions DclLogPrintf() and DclLogVPrintf().
    Revision 1.3  2006/07/26 04:37:37Z  brandont
    Conditioned "flushing log file" debug message for debug level 2+.
    Revision 1.2  2006/07/21 20:04:20Z  brandont
    Corrected assignment of pDLI before hLog is initialized.  Most noticable
    when logging is enabled and an assert happens.
    Revision 1.1  2005/12/05 20:17:14Z  Pauli
    Initial revision
    Revision 1.5  2005/12/05 20:17:13Z  Garyp
    Updated to contain protections against being re-entered.
    Revision 1.4  2005/10/07 17:42:29Z  garyp
    Documentation fixes.
    Revision 1.3  2005/10/07 03:08:24Z  Garyp
    Modified to no longer use standard C file I/O, but rather use the DCL
    abstraction.
    Revision 1.2  2005/10/06 18:57:20Z  pauli
    Fixed memory corruption issues when using a 0 size buffer or when
    the buffer becomes full.
    Revision 1.1  2005/10/05 22:41:12Z  Garyp
    Initial revision
    Revision 1.2  2005/08/03 19:13:58Z  pauli
    Replaced DCL_FuncName with DclFuncName.
    Revision 1.1  2005/04/10 18:58:14Z  pauli
    Initial revision
---------------------------------------------------------------------------*/

#include <dcl.h>

#if DCLCONF_OUTPUT_ENABLED

#include <dllog.h>
#include <dlservice.h>
#include <dlinstance.h>
#include <dlprintf.h>


#define LOG_FLUSH_CRC_CHECK     (FALSE) /* Should be FALSE for checkin */


static const char       szOverflow[] = "DCL: LOG DATA LOST -- OVERFLOWED BY ";
#define OVERMSGLEN      (sizeof(szOverflow)+20)


typedef struct sLOGINFO
{
    struct sLOGINFO    *pNext;
    DCLFSFILEHANDLE     hLogFile;
    DCLINSTANCEHANDLE   hDclInst;
    DCLSERVICEHANDLE    hOutputService;
    DCLOUTPUTINFO       oi;
    PDCLMUTEX           pFlushMutex;    /* log flush mutex */
    D_ATOMIC32          ulUserCount;    /* threads in log code */
    D_ATOMIC32          ulWaiterCount;  /* threads waiting on a flush */
    D_ATOMIC32          ulBuffPos;      /* current buffer offset */
    D_ATOMIC32          ulOverflowCount;
    D_UINT32            ulBuffLen;
    unsigned volatile   nCurrRec;       /* number of flushes */
    unsigned volatile   fFlushing : 1;  /* flush in progress */
    unsigned            fShadow   : 1;
    unsigned            fPrivate  : 1;
    unsigned            fBinary   : 1;
    DCLALIGNEDBUFFER    (aligned, abBuffer, 1);
} LOGINFO;

typedef struct
{
    DCLINSTANCEHANDLE   hDclInst;
    LOGINFO            *pChainHead;
} LOGSERVICEDATA;

static DCLSTATUS    ServiceIoctl(DCLSERVICEHANDLE hService, DCLREQ_SERVICE *pReq, void *pPriv);
static DCLSTATUS    Open(       LOGSERVICEDATA *pLSD, const char *pszFile, unsigned nBufferKB, D_UINT32 ulFlags);
static DCLSTATUS    WriteString(LOGSERVICEDATA *pLSD, LOGINFO *pLI, const char *pszString);
static DCLSTATUS    Flush(      LOGSERVICEDATA *pLSD, LOGINFO *pLI);
static DCLSTATUS    Close(      LOGSERVICEDATA *pLSD, LOGINFO *pLI);
static DCLLOGMODE   Mode(       LOGSERVICEDATA *pLSD, LOGINFO *pLI, DCLLOGMODE nMode);
static void         OutputString(LOGINFO *pLI, const char *pszMsg);
static DCLSTATUS    OutputData(  LOGINFO *pLI, const char *pData, size_t nDataLen);
static DCLSTATUS    FlushLog(    LOGINFO *pLI);
static DCLSTATUS    WriteLogData(LOGINFO *pLI);
static LOGINFO *    FindFirstNonPrivateLog(LOGSERVICEDATA *pLSD);

#endif


/*  The algorithm used in the logging code is explained below:

    This construct is designed to allow multiple threads to atomically record
    data in a shared memory buffer, which must be periodically flushed.  The
    simplistic solution would be to just use a mutex, however that would
    artificially serialize everything, meaning that the logging would change
    the behavior of the system it is attempting to monitor.  This algorithm
    allows the vast majority of the updates to occur without using a mutex --
    using only atomically updated condition variables.  A mutex is used only
    when an actual flush has to occur.

    BOOL    IsFlushing  // TRUE/FALSE flag indicating a flush is in progress
    ATOMIC  UserCount   // # of threads in the code
    ATOMIC  WaiterCount // # of threads blocked on flush (includes flusher)
    ATOMIC  BufferSpace // Available space remaining in the memory buffer

    while(TRUE)
    {
        AtomicInc(UserCount)

        //  If we're flushing, don't let any new threads into the code

        if(!IsFlushing)
            break

        AtomicDec(UserCount)
        AcquireMutex()
        ReleaseMutex()
    }

    while(TRUE)
    {
        while(BufferSpace >= SpaceNeeded)
        {
            if(AtomicAdjust(BufferSpace -= SpaceNeeded))
            {
                RecordData
                AtomicDec(UserCount)

                // Successful completion!
                return
            }
        }

        SavedRec = CurrentRec   // NOTE: CurrentRec is declared "volatile"
        AtomicInc(WaiterCount)
        AcquireMutex()

        //  Got the mutex, however if CurrentRec no longer equals SavedRec
        //  then someone else just finished a flush operation, so we can
        //  just continue on and try again.  If they are equal then we
        //  know that we're the thread to do the flush.

        if(SavedRec == CurrentRec)
        {
            IsFlushing = TRUE

            //  If UserCount > WaiterCount, then we must wait until any
            //  active theads have either exited the code entirely, or
            //  have also become "Waiters".

            while(UserCount > WaiterCount)
                Sleep(0)

            //  At this point any other threads in the code are blocked on
            //  the mutex, and no new threads can get in, due to IsFlushing
            //  being TRUE, so we're safe to flush the record.

            FlushRecordToMedia()
            CurrentRec++
            Atomic(reset BufferSpace)
            IsFlushing = FALSE
        }

        AtomicDec(WaiterCount)
        ReleaseMutex()

        //  Continue the while() loop and try to record the data again
    }
*/


/*-------------------------------------------------------------------
    Protected: DclLogServiceInit()

    Initialize the LOG service.

    Parameters:
        hDclInst - The DCL instance handle.

    Return Value:
        Returns a DCLSTATUS code indicating the results.
-------------------------------------------------------------------*/
DCLSTATUS DclLogServiceInit(
    DCLINSTANCEHANDLE       hDclInst)
{
  #if DCLCONF_OUTPUT_ENABLED
    static DCLSERVICEHEADER SrvLog[DCL_MAX_INSTANCES];
    static LOGSERVICEDATA   LSD[DCL_MAX_INSTANCES];
    DCLSERVICEHEADER       *pSrv;
    DCLSTATUS               dclStat;
    unsigned                nInstNum;

    /*  REMINDER! Because some environments are capable of restarting,
                  without reinitializing static data, this function must
                  explicitly ensure that any static data is manually
                  zero'd, rather than just expecting it to already be in
                  that state.
    */

    if(!hDclInst)
        return DCLSTAT_SERVICE_BADHANDLE;

    nInstNum = DclInstanceNumber(hDclInst);
    DclMemSet(&LSD[nInstNum], 0, sizeof(LSD[0]));

    LSD[nInstNum].hDclInst = hDclInst;

    pSrv = &SrvLog[nInstNum];
    dclStat = DclServiceHeaderInit(&pSrv, "DLLOG", DCLSERVICE_LOG, ServiceIoctl, &LSD[nInstNum], DCLSERVICEFLAG_THREADSAFE);
    if(dclStat == DCLSTAT_SUCCESS)
    {
        dclStat = DclServiceCreate(hDclInst, pSrv);
    }

    return dclStat;

  #else

    /*  Even if a feature is disabled at compile-time, a given project may
        still be using a default set of services which tries to initialize
        it.  Therefore, to avoid link errors, this "Init" function must
        exist.  Simply politely decline to initialize if this is the case.
        The DCLSTAT_FEATUREDISABLED status code is explicitly accommodated
        in the project creation process, and will not abort the process.
    */
    (void)hDclInst;
    return DCLSTAT_FEATUREDISABLED;

  #endif
}


#if DCLCONF_OUTPUT_ENABLED

                /*---------------------------------*\
                 *                                 *
                 *     Static Local Functions      *
                 *                                 *
                \*---------------------------------*/



/*-------------------------------------------------------------------
    Local: ServiceIoctl()

    Dispatch LOG service IOCTL requests.

    Parameters:
        hService - The service handle
        pReq     - A pointer to the DCLREQ_SERVICE structure
        pPriv    - A pointer to any private data to pass

    Return Value:
        Returns a DCLSTATUS code indicating the results.
-------------------------------------------------------------------*/
static DCLSTATUS ServiceIoctl(
    DCLSERVICEHANDLE    hService,
    DCLREQ_SERVICE     *pReq,
    void               *pPriv)
{
    LOGSERVICEDATA     *pLSD = pPriv;
    DCLSTATUS           dclStat;

    DclAssert(hService);
    DclAssertWritePtr(pReq, sizeof(*pReq));
    DclAssertWritePtr(pLSD, sizeof(*pLSD));

    if(pReq->ior.ulReqLen != sizeof(*pReq))
        return DCLSTAT_BADSTRUCLEN;

    switch(pReq->ior.ioFunc)
    {
        case DCLIOFUNC_SERVICE_CREATE:
        case DCLIOFUNC_SERVICE_REGISTER:
        case DCLIOFUNC_SERVICE_DEREGISTER:
            DclAssert(pReq->pSubRequest == NULL);

            /*  Nothing to do
            */
            return DCLSTAT_SUCCESS;

        case DCLIOFUNC_SERVICE_DESTROY:
        {
            LOGINFO *pCurrent = pLSD->pChainHead;
            LOGINFO *pNext;

            while(pCurrent)
            {
                /*  Save the next before we destroy this instance
                */
                pNext = pCurrent->pNext;

                DCLPRINTF(1, ("DCL: Destroying log instance %P\n", pCurrent));

                dclStat = Close(pLSD, pCurrent);
                if(dclStat != DCLSTAT_SUCCESS)
                {
                    DclPrintf("DCL: Unable to destroy log instance %P, Status=%lX, ignoring...\n", pCurrent, dclStat);
                }

                pCurrent = pNext;
            }

            /*  Even if all the log instances could not be destroyed, we're
                ignoring that and going to kill this service anyway.
            */
            return DCLSTAT_SUCCESS;
        }

        case DCLIOFUNC_SERVICE_DISPATCH:
        {
            if(pReq->pSubRequest == NULL)
                return DCLSTAT_SERVICE_BADSUBREQUEST;

            if(pReq->pSubRequest->ulReqLen < sizeof(pReq->pSubRequest))
                return DCLSTAT_BADSTRUCLEN;

            switch(pReq->pSubRequest->ioFunc)
            {
                case DCLIOFUNC_LOG_OPEN:
                {
                    DCLREQ_LOG_OPEN *pSubReq = (DCLREQ_LOG_OPEN*)pReq->pSubRequest;

                    if(pSubReq->ior.ulReqLen != sizeof(*pSubReq))
                        return DCLSTAT_BADSTRUCLEN;

                    dclStat = Open(pLSD, pSubReq->pszFile, pSubReq->nBufferKB, pSubReq->ulFlags);
                    if(dclStat == DCLSTAT_SUCCESS)
                        pSubReq->hLog = pLSD->pChainHead;
                    else
                        pSubReq->hLog = NULL;

                    return DCLSTAT_SUCCESS;
                }

                case DCLIOFUNC_LOG_WRITE:
                {
                    DCLREQ_LOG_WRITE *pSubReq = (DCLREQ_LOG_WRITE*)pReq->pSubRequest;

                    if(pSubReq->ior.ulReqLen != sizeof(*pSubReq))
                        return DCLSTAT_BADSTRUCLEN;

                    if(!pSubReq->hLog)
                    {
                        DclError();
                        pSubReq->dclStat = DCLSTAT_LOG_BADHANDLE;
                        return DCLSTAT_SUCCESS;
                    }

                    if(!pSubReq->pData)
                    {
                        DclError();
                        pSubReq->dclStat = DCLSTAT_LOG_NOWRITEDATA;
                        return DCLSTAT_SUCCESS;
                    }

                    if(!pSubReq->nDataLen)
                    {
                        DclError();
                        pSubReq->dclStat = DCLSTAT_LOG_NOWRITELENGTH;
                        return DCLSTAT_SUCCESS;
                    }

                    pSubReq->dclStat = OutputData(pSubReq->hLog, (const char*)pSubReq->pData, pSubReq->nDataLen);

                    return DCLSTAT_SUCCESS;
                }

                case DCLIOFUNC_LOG_WRITESTRING:
                {
                    DCLREQ_LOG_WRITESTRING *pSubReq = (DCLREQ_LOG_WRITESTRING*)pReq->pSubRequest;

                    if(pSubReq->ior.ulReqLen != sizeof(*pSubReq))
                        return DCLSTAT_BADSTRUCLEN;

                    pSubReq->dclStat = WriteString(pLSD, pSubReq->hLog, pSubReq->pszString);

                    return DCLSTAT_SUCCESS;
                }

                case DCLIOFUNC_LOG_FLUSH:
                {
                    DCLREQ_LOG_FLUSH   *pSubReq = (DCLREQ_LOG_FLUSH*)pReq->pSubRequest;

                    if(pSubReq->ior.ulReqLen != sizeof(*pSubReq))
                        return DCLSTAT_BADSTRUCLEN;

                    pSubReq->dclStat = Flush(pLSD, pSubReq->hLog);

                    return DCLSTAT_SUCCESS;
                }

                case DCLIOFUNC_LOG_CLOSE:
                {
                    DCLREQ_LOG_CLOSE   *pSubReq = (DCLREQ_LOG_CLOSE*)pReq->pSubRequest;

                    if(pSubReq->ior.ulReqLen != sizeof(*pSubReq))
                        return DCLSTAT_BADSTRUCLEN;

                    if(!pSubReq->hLog)
                        return DCLSTAT_LOG_BADHANDLE;

                    pSubReq->dclStat = Close(pLSD, pSubReq->hLog);

                    return DCLSTAT_SUCCESS;
                }

                case DCLIOFUNC_LOG_MODE:
                {
                    DCLREQ_LOG_MODE   *pSubReq = (DCLREQ_LOG_MODE*)pReq->pSubRequest;

                    if(pSubReq->ior.ulReqLen != sizeof(*pSubReq))
                        return DCLSTAT_BADSTRUCLEN;

                    pSubReq->nOldMode = Mode(pLSD, pSubReq->hLog, pSubReq->nMode);

                    return DCLSTAT_SUCCESS;
                }

                default:
                    DCLPRINTF(1, ("Log:ServiceIoctl() Unsupported subfunction %x\n", pReq->pSubRequest->ioFunc));
                    return DCLSTAT_SERVICE_UNSUPPORTEDREQUEST;
            }
        }

        default:
            DCLPRINTF(1, ("Log:ServiceIoctl() Unhandled request %x\n", pReq->ior.ioFunc));
            return DCLSTAT_SERVICE_UNHANDLEDREQUEST;
    }
}


/*-------------------------------------------------------------------
    Local: Open()

    Open a log for capturing output.  See the DclLogOpen() function
    for a detailed description.

    Parameters:
        pLSD      - A pointer to the LOGSERVICEDATA structure.
        pszFile   - A pointer to the null terminated filename.  If
                    this value is NULL, output will be queued in the
                    buffer (so long as nBufferKB is non-zero).
        nBufferKB - The buffer size in KB.  This value may be zero
                    only in the event that pszFile is NULL, in which
                    case, logging is disabled.
        ulFlags   - The open flags to use.  May be a combination of the
                    following values:
          DCLLOGFLAGS_APPEND - Indicates that an existing file should
                    be appended to, rather than deleted.
          DCLLOGFLAGS_SHADOW - Indicates that output should be shadowed
                    to the original output device.  If this flag is used
                    with a private log, the output will be shadowed using
                    the high level output functions.
          DCLLOGFLAGS_PRIVATE - Indicates that this log instance should
                    be private in that it is NOT hooked into the standard
                    output chain managed by the DclOutputFunction() logic.
                    If this option is used, the only way to write to the
                    log is with DclLogPrintf().
          DCLLOGFLAGS_BINARY - Indicates that the log output is binary and
                    should be written exactly as received.  Binary logs
                    should be DCLLOGFLAGS_PRIVATE as well, and should not
                    be used with DCLLOGFLAGS_SHADOW.

    Return Value:
        Returns a DCLSTATUS code indicating the results.
-------------------------------------------------------------------*/
static DCLSTATUS Open(
    LOGSERVICEDATA     *pLSD,
    const char         *pszFile,
    unsigned            nBufferKB,
    D_UINT32            ulFlags)
{
    LOGINFO            *pLI;
    DCLSTATUS           dclStat = DCLSTAT_SUCCESS;

    DclAssertWritePtr(pLSD, sizeof(*pLSD));

    /*  The buffer size may only be zero if a file is NOT specified.
    */
    if(nBufferKB == 0 && pszFile)
        return DCLSTAT_LOG_BADPARAMS;

    /*  Allocate enough memory for our control structure and the
        data buffer tacked onto the tail end.
    */
    DCLPRINTF(1, ("LogService:Open() using a %lU KB buffer + workarea\n", nBufferKB));

    pLI = DclMemAlloc(sizeof(*pLI) + (nBufferKB * 1024UL));
    if(!pLI)
        return DCLSTAT_MEMALLOCFAILED;

    /*  Only need to clear the area used for our tracking structure
    */
    DclMemSet(pLI, 0, sizeof(*pLI));

    pLI->pFlushMutex = DclMutexCreate("DLLOG");
    if(!pLI->pFlushMutex)
    {
        DclMemFree(pLI);
        return DCLSTAT_MUTEXCREATEFAILED;
    }

    pLI->hDclInst = pLSD->hDclInst;
    pLI->ulBuffLen = nBufferKB * 1024UL;
    pLI->fShadow  = (ulFlags & DCLLOGFLAGS_SHADOW)  ? TRUE : FALSE;
    pLI->fPrivate = (ulFlags & DCLLOGFLAGS_PRIVATE) ? TRUE : FALSE;
    pLI->fBinary  = (ulFlags & DCLLOGFLAGS_BINARY)  ? TRUE : FALSE;

    if(pszFile && *pszFile)
    {
        /*  Open or create the file as specified
        */
        dclStat = DclFsFileOpen(pszFile, (ulFlags & DCLLOGFLAGS_APPEND) ? "a+b" : "w+b", &pLI->hLogFile);
        if(dclStat != DCLSTAT_SUCCESS)
        {
            DclPrintf("LogService:Open() Unable to create/open log file '%s', Status=%lX\n", pszFile, dclStat);

            DclMutexDestroy(pLI->pFlushMutex);
            DclMemFree(pLI);

            return dclStat;
        }
    }

    /*  Do this only for normal log instances which are hooked into the normal
        output stream.  Private logs do not use the standard output stream,
        nor are they maintained as part of the "hLogCurrent/hLogPrevious"
        chain.
    */
    if(!pLI->fPrivate)
    {
        DCLSERVICEHEADER   *pOutputSrv = NULL;

        pLI->oi.pContext = pLI;
        pLI->oi.pfnOutputString = (PFNDCLOUTPUTSTRING)OutputString;

        dclStat = DclServiceHeaderInit(&pOutputSrv, "DLLOGOUTPUT", DCLSERVICE_OUTPUT,
                                       NULL, &pLI->oi, DCLSERVICEFLAG_SLAVE);
        if(dclStat == DCLSTAT_SUCCESS)
        {
            dclStat = DclServiceCreate(pLSD->hDclInst, pOutputSrv);
            if(dclStat == DCLSTAT_SUCCESS)
                pLI->hOutputService = pOutputSrv;
            else
                DclMemFree(pOutputSrv);
        }
    }

    if(dclStat != DCLSTAT_SUCCESS)
    {
        if(pLI->hLogFile)
            DclFsFileClose(pLI->hLogFile);

        DclMutexDestroy(pLI->pFlushMutex);
        DclMemFree(pLI);

        return dclStat;
    }

    /*  Hook the new service in at the head of the chain
    */
    pLI->pNext = pLSD->pChainHead;
    pLSD->pChainHead = pLI;

    /*  All good...
    */
    return DCLSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    Local: WriteString()

    This function writes a null-terminated string to the log.

    Parameters:
        pLSD      - A pointer to the LOGSERVICEDATA structure.
        pLI       - A pointer to the LOGINFO structure.  If
                    this value is NULL, the the string will be
                    written to the default output device.
        pszString - A pointer to the null-terminated format string

    Return Value:
        Returns a DCLSTATUS code indicating the results.
-------------------------------------------------------------------*/
static DCLSTATUS WriteString(
    LOGSERVICEDATA *pLSD,
    LOGINFO        *pLI,
    const char     *pszString)
{
    DCLSTATUS       dclStat = DCLSTAT_SUCCESS;

    DclAssertWritePtr(pLSD, sizeof(*pLSD));

    if(!pszString)
    {
        DclError();
        return DCLSTAT_LOG_NOWRITEDATA;
    }

    if(!pLI)
    {
        pLI = FindFirstNonPrivateLog(pLSD);

        /*  Even if the above call fails, pLI is allowed
            to be NULL to cause the output to go to the
            standard output service.
        */
    }

    if(pLI)
        OutputString(pLI, pszString);
    else
        dclStat = DclOutputString(NULL, pszString);

    return dclStat;
}


/*-------------------------------------------------------------------
    Local: Flush()

    Flushes log data which was opened with Open().

    Parameters:
        pLSD      - A pointer to the LOGSERVICEDATA structure.
        pLI       - A pointer to the LOGINFO structure.  If
                    this value is NULL, then the most recently
                    opened NON-private log file will be flushed.

    Return Value:
        Returns a DCLSTATUS code indicating the results.
-------------------------------------------------------------------*/
static DCLSTATUS Flush(
    LOGSERVICEDATA *pLSD,
    LOGINFO        *pLI)
{
    DCLSTATUS       dclStat;

    DclAssertWritePtr(pLSD, sizeof(*pLSD));

    if(!pLI)
    {
        pLI = FindFirstNonPrivateLog(pLSD);
        if(!pLI)
            return DCLSTAT_LOG_NOTFOUND;
    }

    /*  If there is no log file, there is nothing to flush to.
    */
    if(!pLI->hLogFile)
        return DCLSTAT_SUCCESS;

    /*  Perform the flush, updating the reference count.
    */
    DclOsAtomic32Increment(&pLI->ulUserCount);
    dclStat = FlushLog(pLI);
    DclOsAtomic32Decrement(&pLI->ulUserCount);

    return dclStat;
}


/*-------------------------------------------------------------------
    Local: Close()

    Close a log file which was opened with DclLogOpen().

    Note that nested calls to DclLogOpen() should be matched
    with corollary calls to DclLogClose() in reverse order.

    Parameters:
        pLSD      - A pointer to the LOGSERVICEDATA structure.
        pLI       - A pointer to the LOGINFO structure.

    Return Value:
        Returns a DCLSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
static DCLSTATUS Close(
    LOGSERVICEDATA *pLSD,
    LOGINFO        *pLI)
{
    DCLSTATUS       dclStat;

    DclAssertWritePtr(pLSD, sizeof(*pLSD));
    DclAssertWritePtr(pLI, sizeof(*pLI));
    DclAssert(pLSD->pChainHead);

    if(pLI->hOutputService)
    {
        /*  Destroy the output service
        */
        dclStat = DclServiceDestroy(pLI->hOutputService);
        if(dclStat != DCLSTAT_SUCCESS)
            return dclStat;
    }

    if(pLI->hLogFile)
    {
        /*  Write out anything pending in the buffer
        */
        dclStat = Flush(pLSD, pLI);
        if(dclStat != DCLSTAT_SUCCESS)
            return dclStat;

        /*  Close the file and delete resources
        */
        dclStat = DclFsFileClose(pLI->hLogFile);
        if(dclStat != DCLSTAT_SUCCESS)
            return dclStat;
    }
    else if(pLI->ulBuffPos)
    {
        D_UINT32    ulLen = pLI->ulBuffPos;
        char       *pStr = (char*)&pLI->aligned.abBuffer[0];

        /*  If there is no log file, but something in the buffer,
            redirect the queued data to the original device.
        */
        while(ulLen)
        {
            size_t  nStrLen;

            nStrLen = DclStrLen(pStr);
            if(nStrLen)
            {
                dclStat = DclOutputString(pLSD->hDclInst, pStr);
                if(dclStat != DCLSTAT_SUCCESS)
                    return dclStat;
            }

            nStrLen++;
            pStr += nStrLen;

            DclAssert(ulLen >= nStrLen);

            ulLen -= nStrLen;
        }

        if(pLI->ulOverflowCount)
        {
            char    achTemp[OVERMSGLEN];

            DclSNPrintf(achTemp, OVERMSGLEN, "%s%lU BYTES!\n", szOverflow, pLI->ulOverflowCount);

            dclStat = DclOutputString(pLSD->hDclInst, achTemp);
            if(dclStat != DCLSTAT_SUCCESS)
                return dclStat;
        }
    }

    /*  Remove this entry from the list
    */
    if(pLSD->pChainHead == pLI)
    {
        /*  This will be the typical case -- removing the most recently
            added log, which will be at the head of the list.
        */
        pLSD->pChainHead = pLI->pNext;
    }
    else
    {
        LOGINFO *pCurrent = pLSD->pChainHead;

        while(pCurrent)
        {
            if(pCurrent->pNext == pLI)
            {
                pCurrent->pNext = pLI->pNext;
                break;
            }

            pCurrent = pLI->pNext;
        }

        /*  Something really bad is going on if the entry is not
            in the list.
        */
        if(!pCurrent)
        {
            DclError();
            return DCLSTAT_LOG_BADHANDLE;
        }
    }

    (void)DclMutexDestroy(pLI->pFlushMutex);

    return DclMemFree(pLI);
}


/*-------------------------------------------------------------------
    Local: Mode()

    Get/set logging mode flags.  This API is constructed to allow
    multiple logging mode flags to be independently set or cleared
    without requiring manipulation of, or even knowledge of other
    flags.  For now this API only controls the "shadowing" feature.

    Parameters:
        pLSD      - A pointer to the LOGSERVICEDATA structure.
        pLI       - A pointer to the LOGINFO structure.  If
                    this value is NULL, then the most recently
                    opened NON-private log file will be flushed.

    Return Value:
        Returns a DCLSTATUS code indicating the results.
-------------------------------------------------------------------*/
static DCLLOGMODE Mode(
    LOGSERVICEDATA *pLSD,
    LOGINFO        *pLI,
    DCLLOGMODE      nMode)
{
    DCLLOGMODE      nOldMode;

    DclAssertWritePtr(pLSD, sizeof(*pLSD));

    if(!pLI)
    {
        pLI = FindFirstNonPrivateLog(pLSD);
        if(!pLI)
            return DCLLOGMODE_FAILED;
    }

    DclAssert(nMode > DCLLOGMODE_LOWLIMIT && nMode < DCLLOGMODE_HIGHLIMIT);
    DclAssert(nMode != DCLLOGMODE_FAILED);

    /* Set the default return value -- assuming no failure.
    */
    nOldMode = pLI->fShadow ? DCLLOGMODE_SHADOW : DCLLOGMODE_NOSHADOW;

    /*  For now this logic is pretty simple because there is only one
        characteristic (shadowing) being controlled.
    */
    switch(nMode)
    {
        case DCLLOGMODE_QUERY:
            break;

        case DCLLOGMODE_SHADOW:
            pLI->fShadow = TRUE;
            break;

        case DCLLOGMODE_NOSHADOW:
            pLI->fShadow = FALSE;
            break;

        default:
            DCLPRINTF(1, ("DclLog:Mode() invalid request %x\n", nMode));
            nOldMode = DCLLOGMODE_FAILED;
            break;
    }

    return nOldMode;
}


/*-------------------------------------------------------------------
    Local: OutputString()

    Write a string to the log.

    Parameters:
        pLI      - The LOGINFO structure to use.
        pszMsg   - The null-terminated string to write.

    Return Value:
        None.
-------------------------------------------------------------------*/
static void OutputString(
    LOGINFO        *pLI,
    const char     *pszMsg)
{
    DclAssertWritePtr(pLI, sizeof(*pLI));
    DclAssertReadPtr(pszMsg, 0);

    if(!pLI)
        return;

    (void)OutputData(pLI, pszMsg, 0);
}


/*-------------------------------------------------------------------
    Local: OutputData()

    Write data to the log.

    Parameters:
        pLI      - The LOGINFO structure to use.
        pData    - The data to write.
        nDataLen - The length of pData, or 0 if it is null-terminated.

    Return Value:
        Returns a DCLSTATUS code indicating the results.
-------------------------------------------------------------------*/
static DCLSTATUS OutputData(
    LOGINFO        *pLI,
    const char     *pData,
    size_t          nDataLen)
{
    DCLSTATUS       dclStat = DCLSTAT_SUCCESS;

    DclAssertWritePtr(pLI, sizeof(*pLI));
    DclAssertReadPtr(pData, nDataLen);

    if(!nDataLen)
        nDataLen = DclStrLen(pData) + 1;

    /*  No attempt is made to write a chunk of data larger than the log buffer.
    */
    if(nDataLen > pLI->ulBuffLen)
        goto Shadow;

    while(TRUE)
    {
        DclOsAtomic32Increment(&pLI->ulUserCount);

        /*  New threads can enter the code if we are not flushing.
        */
        if(!pLI->fFlushing)
            break;

        /*  Wait for the current flush operation to complete and try again.
        */
        {
            DclOsAtomic32Decrement(&pLI->ulUserCount);

            if(!DclMutexAcquire(pLI->pFlushMutex))
                return DCLSTAT_MUTEXACQUIREFAILED;

            if(!DclMutexRelease(pLI->pFlushMutex))
                return DCLSTAT_MUTEXRELEASEFAILED;
        }
    }

    while(TRUE)
    {
        while(TRUE)
        {
            D_UINT32 ulOrigPos = DclOsAtomic32Retrieve(&pLI->ulBuffPos);
            D_UINT32 ulNewPos  = ulOrigPos + nDataLen;

            /*  If there is insufficient room, wait for a flush.
            */
            if(ulNewPos > pLI->ulBuffLen)
                break;

            /*  Attempt to secure a position for this data. If unsuccessful,
                reevaluate the situation.
            */
            if(DclOsAtomic32CompareExchange(&pLI->ulBuffPos, ulOrigPos, ulNewPos) != ulOrigPos)
                continue;

            /*  Copy in the data; write complete
            */
            DclMemCpy(&pLI->aligned.abBuffer[ulOrigPos], pData, nDataLen);
            DclOsAtomic32Decrement(&pLI->ulUserCount);
            goto Shadow;
        }

        if(pLI->hLogFile)
        {
            /*  Flush the buffer and retry.
            */
            dclStat = FlushLog(pLI);
            if(dclStat != DCLSTAT_SUCCESS)
            {
                DclOsAtomic32Decrement(&pLI->ulUserCount);
                return dclStat;
            }
        }
        else
        {
            /*  Without a log file, nothing can be flushed; record the lost
                data.
            */
            DclOsAtomic32ExchangeAdd(&pLI->ulOverflowCount, (D_UINT32)nDataLen);
            DclOsAtomic32Decrement(&pLI->ulUserCount);
            break;
        }
    }

    /*  If shadowing is enabled, write the string to the previously configured
        output device. (NOTE: Not currently thread-safe.)
    */
  Shadow:
    if(pLI->fShadow && pLI->fPrivate)
    {
        /*  For private logs we are not a part of the standard output service
            chain, so there is no "previous" service.  Instead, if shadowing,
            just use the high level output function.
        */
        DclOutputString(pLI->hDclInst, pData);
    }
    else if(pLI->fShadow)
    {
        DCLOUTPUTINFO  *pOut;

        dclStat = DclServiceDataPrevious(pLI->hOutputService, (void**)&pOut);
        if((dclStat == DCLSTAT_SUCCESS) && pOut)
        {
            (*pOut->pfnOutputString)(pOut->pContext, pData);
        }
    }

    return dclStat;
}


/*-------------------------------------------------------------------
    Local: FlushLog()

    Internal function for flushing a log. Code which calls this
    function should have already incremented the user count.

    Parameters:
        pLI - A pointer to the LOGINFO structure.

    Return Value:
        Returns a DCLSTATUS code indicating the results.
-------------------------------------------------------------------*/
static DCLSTATUS FlushLog(
    LOGINFO        *pLI)
{
    DCLSTATUS       dclStat = DCLSTAT_SUCCESS;
    unsigned        nSavedRec;

    DclAssertReadPtr(pLI, sizeof(*pLI));
    DclAssertReadPtr(pLI->hLogFile, 0);
    DclAssert(DclOsAtomic32Retrieve(&pLI->ulUserCount));

    /*  Save the current flush count and increment the number of threads
        waiting for a flush.
    */
    nSavedRec = pLI->nCurrRec;
    DclOsAtomic32Increment(&pLI->ulWaiterCount);

    /*  Acquire the flush mutex. This prevents two threads from trying to flush
        concurrently, and provides writing threads something to block on.
    */
    if(!DclMutexAcquire(pLI->pFlushMutex))
        return DCLSTAT_MUTEXACQUIREFAILED;

    /*  If nSavedRec is not equal to pLI->nCurrRec, then someone else
        already finished a flush operation. Otherwise, this thread performs
        the flush.
    */
    if((nSavedRec == pLI->nCurrRec) && DclOsAtomic32Retrieve(&pLI->ulBuffPos))
    {
        pLI->fFlushing = TRUE;

        /*  Wait until ulUserCount does not exceed ulWaiterCount, thereby
            resulting in all active threads having exited the code entirely,
            or started waiting for a flush.
        */
        while(DclOsAtomic32Retrieve(&pLI->ulUserCount) > DclOsAtomic32Retrieve(&pLI->ulWaiterCount))
            /* DclOsSleep(1) */ ;

        /*  At this point any other threads in the code are blocked on the
            mutex, and no new threads can get in, due to pLI->fFlushing
            being TRUE, so we are safe to flush the record.
        */
        dclStat = WriteLogData(pLI);
        if(dclStat == DCLSTAT_SUCCESS)
        {
            DclOsAtomic32Exchange(&pLI->ulBuffPos, 0);
            pLI->nCurrRec++;
        }

        pLI->fFlushing = FALSE;
    }

    /*  Release resources
    */
    DclOsAtomic32Decrement(&pLI->ulWaiterCount);
    if(!DclMutexRelease(pLI->pFlushMutex))
        return DCLSTAT_MUTEXRELEASEFAILED;

    return dclStat;
}


/*-------------------------------------------------------------------
    Local: WriteLogData()
-------------------------------------------------------------------*/
static DCLSTATUS WriteLogData(
    LOGINFO        *pLI)
{
    #define         CHUNK_LEN (128)
    const char     *pSrc = (char*)&pLI->aligned.abBuffer[0];
    D_UINT32        ulWritten;
    DCLSTATUS       dclStat;
  #if LOG_FLUSH_CRC_CHECK
    D_UINT32        ulInitCrc = DclCrc32Update(0, pSrc, pLI->ulBuffLen);
    D_UINT32        ulEndCrc;
  #endif

    DclAssertWritePtr(pLI, sizeof(*pLI));
    DclAssert(pLI->hLogFile);

  #if D_DEBUG > 1
    if(pLI->fShadow)
    {
        if(pLI->fPrivate)
        {
            /*  For private logs we are not a part of the standard output
                service chain, so there is no "previous" service.  Instead,
                if shadowing, just use the high level output function.
            */
            DclOutputString(pLI->hDclInst, "Flushing log to disk\n");
        }
        else
        {
            DCLOUTPUTINFO  *pOut;

            dclStat = DclServiceDataPrevious(pLI->hOutputService, (void**)&pOut);
            if(dclStat == DCLSTAT_SUCCESS && pOut)
            {
                (*pOut->pfnOutputString)(pOut->pContext, "Flushing log to disk\n");
            }
        }
    }
  #endif

    if(pLI->fBinary)
    {
        dclStat = DclFsFileWrite(pLI->hLogFile, pSrc, pLI->ulBuffPos, &ulWritten);
        if(dclStat != DCLSTAT_SUCCESS)
            return dclStat;

        if(ulWritten != pLI->ulBuffPos)
            return DCLSTAT_FS_WRITEFAILED;
    }
    else
    {
        char            achBuffer[CHUNK_LEN];
        char           *pDest = &achBuffer[0];
        size_t          nThisLen = 0;
        D_UINT32        ulLength = pLI->ulBuffPos;

        /*  Build our output data in a temporary buffer, inserting a \r for
            every \n that is found, then write to disk when the temp buffer
            is full.
        */
        while(ulLength)
        {
            /*  If no space for at least two characters, (in case we find
                a \n next), write out what we have so far, and reset.
            */
            if(nThisLen >= CHUNK_LEN - 1)
            {
                dclStat = DclFsFileWrite(pLI->hLogFile, achBuffer, nThisLen, &ulWritten);
                if(dclStat != DCLSTAT_SUCCESS)
                    return dclStat;

                if(ulWritten != nThisLen)
                    return DCLSTAT_FS_WRITEFAILED;

                pDest = &achBuffer[0];
                nThisLen = 0;
            }

            /*  ToDo: This should be abstracted at the project level, to allow
                      the developer to define whether he wants Windows or Linux
                      style EOL characters.
            */

            /*  Add a \r for every \n that is found.
            */
            if(*pSrc == '\n')
            {
                *pDest++ = '\r';
                nThisLen++;
            }

            /*  Transfer any non-NULL character
            */
            if(*pSrc)
            {
                *pDest++ = *pSrc;
                nThisLen++;
            }

            pSrc++;
            ulLength--;
        }

        /*  Write anything left at the tail end...
        */
        if(nThisLen)
        {
            dclStat = DclFsFileWrite(pLI->hLogFile, achBuffer, nThisLen, &ulWritten);
            if(dclStat != DCLSTAT_SUCCESS)
                return dclStat;

            if(ulWritten != nThisLen)
                return DCLSTAT_FS_WRITEFAILED;
        }
    }

    dclStat = DclFsFileFlush(pLI->hLogFile);

    /*  If the top level code is doing its job, no one should be writing to
        the buffer while it is being flushed. Thus, the CRC should be the same
        now as it was when the function began.
    */
  #if LOG_FLUSH_CRC_CHECK
    ulEndCrc = DclCrc32Update(0, &pLI->aligned.abBuffer[0], pLI->ulBuffLen);
    DclAssert(ulEndCrc == ulInitCrc);
  #endif

    return dclStat;
}


/*-------------------------------------------------------------------
    Local: FindFirstNonPrivateLog()

    Loop through the various log instances for this service instance
    to find the most recent log instance which is not private.
-------------------------------------------------------------------*/
static LOGINFO * FindFirstNonPrivateLog(
    LOGSERVICEDATA *pLSD)
{
    LOGINFO        *pCurrent;

    DclAssertWritePtr(pLSD, sizeof(*pLSD));

    pCurrent = pLSD->pChainHead;
    while(pCurrent)
    {
        if(!pCurrent->fPrivate)
            return pCurrent;

        pCurrent = pCurrent->pNext;
    }

    return NULL;
}




#endif  /* DCLCONF_OUTPUT_ENABLED */

