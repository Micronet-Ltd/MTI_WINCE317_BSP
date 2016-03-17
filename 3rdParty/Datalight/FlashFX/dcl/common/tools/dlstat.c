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

---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dlstat.c $
    Revision 1.10  2010/04/18 21:12:16Z  garyp
    Did some refactoring of the DCL system info functions to make them
    more useful.
    Revision 1.9  2009/11/04 18:50:48Z  garyp
    Updated to gather detailed stats for mutexes, semaphores, and read/
    write semaphores.
    Revision 1.8  2009/06/28 00:40:34Z  garyp
    Modified to use a Service IOCTL to query the stats from the MemTracking
    system.
    Revision 1.7  2007/12/18 20:33:52Z  brandont
    Updated function headers.
    Revision 1.6  2007/11/03 23:31:13Z  Garyp
    Added the standard module header.
    Revision 1.5  2007/05/15 03:52:47Z  garyp
    Clarified a message.
    Revision 1.4  2007/04/08 01:43:43Z  Garyp
    Fixed a broken help message.
    Revision 1.3  2007/03/31 19:08:50Z  Garyp
    Modified the syntax message to match other tools.
    Revision 1.2  2007/03/30 18:49:33Z  Garyp
    Updated to support display mutex and read/write semaphore statistics.
    Revision 1.1  2006/07/06 16:49:40Z  Pauli
    Initial revision
---------------------------------------------------------------------------*/

#include <dcl.h>
#include <dltools.h>
#include <dlstats.h>
#include <dlservice.h>

#define MAX_MUTEXES        (256)
#define MAX_SEMAPHORES      (64)
#define MAX_RWSEMAPHORES    (64)

typedef struct
{
    D_BOOL  fSystem;
    D_BOOL  fMemory;
    D_BOOL  fMutex;
    D_BOOL  fSemaphore;
    D_BOOL  fRWSemaphore;
    D_BOOL  fVerbose;
    D_BOOL  fReset;
} STATSINFO;

static DCLSTATUS    ProcessParameters(DCLTOOLPARAMS *pTP, STATSINFO *pSI);


/*-------------------------------------------------------------------
    Public: DclStats()

    DCL Statistics information.  This function invokes displays the
    Datalight Common Libraries Statistics information about various
    sub-systems.

    Parameters:
        pTP - A pointer to the DCLTOOLPARAMS structure to use

    Return Value:
        Returns a DCLSTATUS value indicating success or failure
-------------------------------------------------------------------*/
DCLSTATUS DclStats(
    DCLTOOLPARAMS      *pTP)
{
    DCLSTATUS           dclStat;
    STATSINFO           si;

    DclMemSet(&si, 0, sizeof(si));

    DclPrintf("DCL Statistics\n");
    DclSignOn(FALSE);
    DclPrintf("\n");

    dclStat = ProcessParameters(pTP, &si);
    if(dclStat == DCLSTAT_HELPREQUEST)
        return DCLSTAT_SUCCESS;

    if(dclStat != DCLSTAT_SUCCESS)
        return dclStat;

    /*  Display system information
    */
    if(si.fSystem)
    {
        DCLSYSTEMINFO SysInfo = {0};

        SysInfo.nStrucLen = sizeof(SysInfo);
        dclStat = DclSystemInfoQuery(&SysInfo);
        if(dclStat == DCLSTAT_SUCCESS)
        {
            dclStat = DclSystemInfoDisplay(&SysInfo);
            if(dclStat != DCLSTAT_SUCCESS)
            {
                DclPrintf("Error displaying system statistics: %lX.\n", dclStat);
            }
        }
        else
        {
            DclPrintf("Error retrieving system statistics: %lX.\n", dclStat);
        }
    }

    /*  Display memory information
    */
    if(si.fMemory)
    {
        DCLDECLAREREQUESTPACKET (MEMTRACK, STATS, stats);   /* DCLREQ_MEMTRACK_STATS */

        stats.dms.uStrucLen = sizeof(stats.dms);
        stats.fVerbose = si.fVerbose;
        stats.fReset = si.fReset;

        dclStat = DclServiceIoctl(pTP->hDclInst, DCLSERVICE_MEMTRACK, &stats.ior);
        if(dclStat == DCLSTAT_SUCCESS)
        {
            dclStat = DclMemStatsDisplay(&stats.dms);
            if(dclStat != DCLSTAT_SUCCESS)
            {
                DclPrintf("Error displaying memory statistics: %lX\n", dclStat);
            }
        }
        else if(dclStat == DCLSTAT_SERVICE_NOTREGISTERED)
        {
            DclPrintf("Memory tracking disabled.\n");
        }
        else
        {
            DclPrintf("Error retrieving memory statistics: %lX\n", dclStat);
        }
    }

    /*  Display mutex information
    */
    if(si.fMutex)
    {
        DCLMUTEXSTATS   MutStats = {0};

        MutStats.nStrucLen = sizeof(MutStats);

        MutStats.pDMI = DclMemAlloc(MAX_MUTEXES * sizeof(*MutStats.pDMI));
        if(MutStats.pDMI)
            MutStats.nSupplied = MAX_MUTEXES;
        
        dclStat = DclMutexStatsQuery(&MutStats, si.fVerbose, si.fReset);
        if(dclStat == DCLSTAT_SUCCESS)
        {
            dclStat = DclMutexStatsDisplay(&MutStats);
            if(dclStat != DCLSTAT_SUCCESS)
            {
                DclPrintf("Error displaying mutex statistics: %lX\n", dclStat);
            }
        }
        else if(dclStat == DCLSTAT_MUTEXTRACKINGDISABLED)
        {
            DclPrintf("Mutex tracking disabled.\n");
        }
        else
        {
            DclPrintf("Error retrieving mutex statistics: %lX.\n", dclStat);
        }

        if(MutStats.pDMI)
            DclMemFree(MutStats.pDMI);
    }

    /*  Display semaphore information
    */
    if(si.fSemaphore)
    {
        DCLSEMAPHORESTATS   SemStats = {0};

        SemStats.nStrucLen = sizeof(SemStats);

        SemStats.pDSI = DclMemAlloc(MAX_SEMAPHORES * sizeof(*SemStats.pDSI));
        if(SemStats.pDSI)
            SemStats.nSupplied = MAX_SEMAPHORES;
         
        dclStat = DclSemaphoreStatsQuery(&SemStats, si.fVerbose, si.fReset);
        if(dclStat == DCLSTAT_SUCCESS)
        {
            dclStat = DclSemaphoreStatsDisplay(&SemStats);
            if(dclStat != DCLSTAT_SUCCESS)
            {
                DclPrintf("Error displaying semaphore statistics: %lX.\n", dclStat);
            }
        }
        else if(dclStat == DCLSTAT_SEMAPHORETRACKINGDISABLED)
        {
            DclPrintf("Semaphore tracking disabled.\n");
        }
        else
        {
            DclPrintf("Error retrieving semaphore statistics: %lX.\n", dclStat);
        }

        if(SemStats.pDSI)
            DclMemFree(SemStats.pDSI);
    }

    /*  Display read/write semaphore information
    */
    if(si.fRWSemaphore)
    {
        DCLRDWRSEMAPHORESTATS   RWSemStats = {0};

        RWSemStats.nStrucLen = sizeof(RWSemStats);

        RWSemStats.pRWI = DclMemAlloc(MAX_RWSEMAPHORES * sizeof(*RWSemStats.pRWI));
        if(RWSemStats.pRWI)
            RWSemStats.nSupplied = MAX_RWSEMAPHORES;
        
        dclStat = DclSemaphoreRdWrStatsQuery(&RWSemStats, si.fVerbose, si.fReset);
        if(dclStat == DCLSTAT_SUCCESS)
        {
            dclStat = DclSemaphoreRdWrStatsDisplay(&RWSemStats);
            if(dclStat != DCLSTAT_SUCCESS)
            {
                DclPrintf("Error displaying read/write semaphore statistics: %lX.\n", dclStat);
            }
        }
        else if(dclStat == DCLSTAT_SEMAPHORETRACKINGDISABLED)
        {
            DclPrintf("Read/write semaphore tracking disabled.\n");
        }
        else
        {
            DclPrintf("Error retrieving read/write semaphore statistics: %lX.\n", dclStat);
        }

        if(RWSemStats.pRWI)
            DclMemFree(RWSemStats.pRWI);
    }

    return DCLSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    Local: ProcessParameters()

    Parses the command line arguments.  Also shows usage
    information if needed.

    Parameters:
        pTP     - A pointer to the DCLTOOLPARAMS structure
        pSI     - A pointer to the STATSINFO structure

    Return Value:
        Error level non-zero if any problems are encountered.
        Zero if the command line arguments are valid and recorded.
-------------------------------------------------------------------*/
static DCLSTATUS ProcessParameters(
    DCLTOOLPARAMS  *pTP,
    STATSINFO      *pSI)
{
    #define         ARGBUFFLEN  (128)
    D_INT16         i, j;
    char            achArgBuff[ARGBUFFLEN];
    D_UINT16        argc;
    D_BOOL          fHelp = FALSE;

    argc = DclArgCount(pTP->pszCmdLine);

    for(i = 1; i <= argc; i++)
    {
        if(!DclArgRetrieve(pTP->pszCmdLine, i, ARGBUFFLEN, achArgBuff))
        {
            DclPrintf("Bad argument!\n");
            return DCLSTAT_BADPARAMETER;
        }

        if(DclStrICmp(achArgBuff, "/all") == 0)
        {
            pSI->fSystem = TRUE;
            pSI->fMemory = TRUE;
            pSI->fMutex = TRUE;
            pSI->fSemaphore = TRUE;
            pSI->fRWSemaphore = TRUE;
            continue;
        }

        if(DclStrICmp(achArgBuff, "/sys") == 0)
        {
            pSI->fSystem = TRUE;
            continue;
        }

        if(DclStrICmp(achArgBuff, "/mem") == 0)
        {
            pSI->fMemory = TRUE;
            continue;
        }

        if(DclStrICmp(achArgBuff, "/mut") == 0)
        {
            pSI->fMutex = TRUE;
            continue;
        }

        if(DclStrICmp(achArgBuff, "/sem") == 0)
        {
            pSI->fSemaphore = TRUE;
            continue;
        }

        if(DclStrICmp(achArgBuff, "/rwsem") == 0)
        {
            pSI->fRWSemaphore = TRUE;
            continue;
        }

        j = 0;

        /*  Allow for multiple switches in a string
        */
        while(*(achArgBuff + j))
        {
            if(*(achArgBuff + j) == '/')
            {
                /*  Go to the next character to test
                */
                ++j;

                /*  Information specfic for each switch
                */
                switch (*(achArgBuff + j))
                {
                    case '?':
                        fHelp = TRUE;
                        break;

                    case 'v':
                    case 'V':
                        pSI->fVerbose = TRUE;
                        break;

                    case 'r':
                    case 'R':
                        pSI->fReset = TRUE;
                        break;

                    /*  Unrecognized switch
                    */
                    default:
                        DclPrintf("Bad option: %s\n", (achArgBuff + j - 1));
                        return DCLSTAT_BADPARAMETER;
                }

                j++;
            }
            else
            {
                DclPrintf("Bad option: %s\n", (achArgBuff + j));
                return DCLSTAT_BADPARAMETER;
            }
        }
    }

    /*  Display usage information if necessary
    */
    if(argc == 0 || fHelp)
    {
        DclPrintf("Syntax:  %s [options]\n\n", pTP->pszCmdName);
        DclPrintf("Where [options] are:\n");
        DclPrintf("  /?     This help information\n");
        DclPrintf("  /ALL   Display all statistics information\n");
        DclPrintf("  /SYS   Display System statistics\n");
        DclPrintf("  /MEM   Display Memory statistics\n");
        DclPrintf("  /MUT   Display Mutex statistics\n");
        DclPrintf("  /SEM   Display Semaphore statistics\n");
        DclPrintf("  /RWSEM Display Read/Write Semaphore statistics\n");
        DclPrintf("  /R     Reset those statistics which are reset-able\n");
        DclPrintf("  /V     Display verbose statistics information, where available\n\n");

        if(pTP->pszExtraHelp)
            DclPrintf(pTP->pszExtraHelp);

        return DCLSTAT_HELPREQUEST;
    }

    /*  All worked fine!
    */
    return DCLSTAT_SUCCESS;
}


