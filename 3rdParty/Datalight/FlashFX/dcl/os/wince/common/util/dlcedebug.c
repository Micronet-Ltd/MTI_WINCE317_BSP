/*---------------------------------------------------------------------------
               ----> DO NOT REMOVE THE FOLLOWING NOTICE <----

                  Copyright (c) 1993 - 2007 Datalight, Inc.
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

---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dlcedebug.c $
    Revision 1.1  2008/09/29 21:28:52Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <windows.h>
#include <tlhelp32.h>

#include <dcl.h>
#include <dlceutil.h>

#define INVALID_ZONE    (0xFFFFFFFF)


/*-------------------------------------------------------------------
    DclCeDebugZoneProcess()

    Description


    Parameters

    Return Value
-------------------------------------------------------------------*/
DWORD DclCeDebugZoneProcess(
    const TCHAR    *ptzProcessName,
    DWORD           dwNewZone)
{
    HANDLE          hSnap;
    PROCESSENTRY32  pe;
    DWORD           dwOldZone = INVALID_ZONE;

    DclAssert(ptzProcessName);

    hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS | TH32CS_SNAPNOHEAPS, 0);
    if(hSnap == INVALID_HANDLE_VALUE)
    {
        DCLPRINTF(1, ("DclCeDebugZoneProcess() failed to get process snapshot\n"));
        return INVALID_ZONE;
    }

    memset(&pe, 0, sizeof(pe));
    pe.dwSize = sizeof(pe);

    if(Process32First(hSnap, &pe))
    {
        do
        {
            DCLPRINTF(2, ("DclCeDebugZoneProcess() Usage=%lU ProcID=%lX ModID=%lX Threads=%lU ParentProcID=%lX PriClassBase=%lD Flags=%lX Base=%lX Key=%lX File=\"%W\"\n",
                pe.cntUsage,       pe.th32ProcessID,       pe.th32ModuleID,
                pe.cntThreads,     pe.th32ParentProcessID, pe.pcPriClassBase, pe.dwFlags,
                pe.th32MemoryBase, pe.th32AccessKey,       pe.szExeFile));

            if(wcsstr(pe.szExeFile, ptzProcessName))
            {
                DBGPARAM    dp;

                /*  Query the current debug zone setting
                */
                if(SetDbgZone(pe.th32ProcessID, 0, NULL, INVALID_ZONE, &dp))
                {
                    unsigned    nn;

                    dwOldZone = dp.ulZoneMask;

                    /*  Note: lpszName is a misnamed field, it really is an
                        array not a pointer to an array.
                    */
                    DCLPRINTF(1, ("\"%W\" -- CurrentZoneMask=%lX\n", dp.lpszName, dwOldZone));

                    for(nn=0; nn<16; nn++)
                    {
                        DCLPRINTF(1, ("    %u - \"%W\"\n", nn, &dp.rglpszZones[nn][0]));
                    }

                    DclAssert(dwOldZone != INVALID_ZONE);
                }
                else
                {
                    DCLPRINTF(1, ("DclCeDebugZoneProcess() failed to get zone information\n"));
                    break;
                }

                if(dwNewZone != INVALID_ZONE)
                {
                    if(SetDbgZone(pe.th32ProcessID, 0, NULL, dwNewZone, &dp))
                    {
                        DCLPRINTF(1, ("NewZoneMask=%lX\n", dp.ulZoneMask));
                    }
                    else
                    {
                        DCLPRINTF(1, ("DclCeDebugZoneProcess() failed to set zone information\n"));
                        dwOldZone = INVALID_ZONE;
                    }
                }

                break;
            }
        } while(Process32Next(hSnap, &pe));
    }

    CloseToolhelp32Snapshot(hSnap);

    return dwOldZone;
}



