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
    $Log: windebug.c $
    Revision 1.1  2008/09/30 16:31:48Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <windows.h>
#include <tlhelp32.h>

#include <dcl.h>
#include <dlwinutil.h>


/*-------------------------------------------------------------------
    DclWinDisplayProcessInfo()

    This function displays detailed process information using the
    ToolHelp API.

    Parameters:
        None.

    Returns:
        Returns the number of processes processed.
-------------------------------------------------------------------*/
unsigned DclWinDisplayProcessInfo(void)
{
    HANDLE          hSnap;
    PROCESSENTRY32  pe;
    unsigned        nCount = 0;

    hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if(hSnap == INVALID_HANDLE_VALUE)
    {
        DCLPRINTF(1, ("DclWinDisplayProcessInfo() failed to get process snapshot\n"));
        return nCount;
    }

    memset(&pe, 0, sizeof(pe));
    pe.dwSize = sizeof(pe);

    if(Process32First(hSnap, &pe))
    {
        DclPrintf("Process Information:\n");

        do
        {
            DclPrintf("  Usage=%lU ProcID=%lX HeapID=%lX ModID=%lX Threads=%lU ParProcID=%lX PriClassBase=%lD Flags=%lX Base=%lX Key=%lX File=\"%W\"\n",
                pe.cntUsage,       pe.th32ProcessID,       pe.th32DefaultHeapID, pe.th32ModuleID,
                pe.cntThreads,     pe.th32ParentProcessID, pe.pcPriClassBase,    pe.dwFlags,
              #ifdef _WIN32_WCE
                pe.th32MemoryBase, pe.th32AccessKey,
              #else
                0,                 0,
              #endif
                pe.szExeFile);

            nCount++;
        } while(Process32Next(hSnap, &pe));
    }

  #ifdef _WIN32_WCE
    CloseToolhelp32Snapshot(hSnap);
  #else
    CloseHandle(hSnap);
  #endif

    return nCount;
}


