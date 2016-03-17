/*---------------------------------------------------------------------------
               ----> DO NOT REMOVE THE FOLLOWING NOTICE <----

                  Copyright (c) 1993 - 2011 Datalight, Inc.
                       All Rights Reserved Worldwide.

  Datalight, Incorporated is a Washington State corporation located at:

        21520 30th Dr SE, Suite 110,      Tel:  425-951-8086
        Bothell, WA  98021                Fax:  425-951-8094
        USA                               Web:  http://www.datalight.com

  This software, including without limitation all source code and documen-
  tation, is the confidential, trade secret property of Datalight, Inc.
  and is protected under the copyright laws of the United States and other
  jurisdictions.  Patents may be pending.

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

    This module implements a filter which spys on file system function calls.
 ---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dlfilespy.c $
    Revision 1.1  2011/04/22 19:06:38Z  garyp
    Initial revision
---------------------------------------------------------------------------*/
 
#include <windows.h>
#include <excpt.h>
#include <fsdmgr.h>

#include <dcl.h>
 

typedef struct
{
    HDSK                hDsk;
    FILTERHOOK          FilterHook;
    DCLINSTANCEHANDLE   hDclInst;
    D_UINT32            ulFlags;
} SPYVOLINFO;

typedef struct
{
    DWORD               hObj;
    SPYVOLINFO         *pVolInfo;
} SPYHANDLEINFO;

#define DCLFILESPYFLAGS_ENABLED     (0x00000001)
#define DCLFILESPYFLAGS_QUERY       (D_UINT32_MAX)  /* Reserved for querying the flags value */

#define SPYPRINTF(flg, txt) (  ((!pVI) || (pVI->ulFlags & DCLFILESPYFLAGS_ENABLED)) ? (void)DclPrintf txt : ((void)0))



/*-------------------------------------------------------------------
    Protected:

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
BOOL WINAPI DllMain(
    HANDLE  hInstance,
    DWORD   dwReason,
    LPVOID  pReserved)
{
    switch(dwReason)
    {
        case DLL_PROCESS_ATTACH:
        {
            DclPrintf("DLFILESPY:DllMain() DLL_PROCESS_ATTACH hInst=%lX pReserved=%P\n", hInstance, pReserved);
            break;
        }
        
        case DLL_PROCESS_DETACH:
        {
            DclPrintf("DLFILESPY:DllMain() DLL_PROCESS_DETACH hInst=%lX pReserved=%P\n", hInstance, pReserved);
            break;
        }

        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:

            /*  Ignore these notifications since they happen all the time and
                we really don't care.
            */
            break;

        default:
        {
            DCLPRINTF(1, ("DLFILESPY:DllMain() hInst=%lX dwReason=%d pReserved=%P\n", hInstance, dwReason, pReserved));

            /*  Friendly reminder if we get some new notification.
            */
            DclError();
            break;
        }
    }

    return TRUE;
}


/*-------------------------------------------------------------------
    Protected:

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
PVOLUME DLFILESPY_HookVolume(
    HDSK                hDsk,
    FILTERHOOK         *pHook)
{
    SPYVOLINFO         *pVI = NULL;
    DCLINSTANCEHANDLE   hDclInst;
    DCLSTATUS           dclStat;

    if(!hDsk)
    {
        RETAILMSG(1, (TEXT(__FUNCTION__) TEXT(": Bad handle, hDsk=%P\r\n"), hDsk));
        return 0;
    }

    if(!pHook)
    {
        RETAILMSG(1, (TEXT(__FUNCTION__) TEXT(": Bad pointer, pHook=%P\r\n"), pHook));
        return 0;
    }

    dclStat = DclInstanceCreate(0, DCLFLAG_DRIVER, &hDclInst);
    if(dclStat != DCLSTAT_SUCCESS)
    {
        RETAILMSG(1, (TEXT(__FUNCTION__) TEXT(": DCL instance creation failed with error %08X\r\n"), dclStat));
        return 0;
    }

    DclPrintf(__FUNCTION__": hDsk=%P pHook=%P\n", hDsk, pHook);

    pVI = DclMemAllocZero(sizeof(*pVI));
    if(!pVI)
    {
        DclPrintf(__FUNCTION__": Out of memory\n");
        DclInstanceDestroy(hDclInst);
        return 0;
    }

    pVI->hDclInst = hDclInst;
    pVI->hDsk = hDsk;
    DclMemCpy(&pVI->FilterHook, pHook, sizeof(*pHook));

    return (PVOLUME)pVI;
}


/*-------------------------------------------------------------------
    Protected:

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
BOOL DLFILESPY_UnhookVolume(
    PVOLUME     pVol)
{
    BOOL        fSuccess = FALSE;
    SPYVOLINFO *pVI = (SPYVOLINFO*)pVol;

    if(!pVol)
    {
        DclPrintf(__FUNCTION__": Bad pointer, pVol=%P\n", pVol);
    }
    else
    {
        DCLSTATUS dclStat = DclInstanceDestroy(pVI->hDclInst);
        if(dclStat != DCLSTAT_SUCCESS)
        {
            RETAILMSG(1, (TEXT(__FUNCTION__) TEXT(": DCL instance destruction failed with status %08X\r\n"), dclStat));
        }
        
        RETAILMSG(1, (TEXT(__FUNCTION__) TEXT(": pVol=%08X hDsk=%P\r\n"), pVol, pVI->hDsk));

        DclMemFree((void*)pVol);
        pVol = (DWORD)NULL;

        fSuccess = TRUE;
    }

    return fSuccess;
}


/*-------------------------------------------------------------------
    Protected:

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
BOOL DLFILESPY_CloseFile(
    PFILE           pFH)
{
    SPYHANDLEINFO  *pHI = (SPYHANDLEINFO*)pFH;
    SPYVOLINFO     *pVI = (SPYVOLINFO*)(pHI ? pHI->pVolInfo : NULL);
    BOOL            fSuccess = FALSE;

    if(!pFH)
    {
        SPYPRINTF(0, (__FUNCTION__": Bad handle, pFile=%P\n", pFH));
        DclProductionError();
        return FALSE;
    }

    SPYPRINTF(0, (__FUNCTION__": hDsk=%P hFile=%P\n", pVI->hDsk, pFH));

    __try
    {
        fSuccess = pVI->FilterHook.pCloseFile(pHI->hObj);
        if(fSuccess)
        {
            SPYPRINTF(0, (__FUNCTION__": hDsk=%P OK\n", pVI->hDsk));

            DclMemFree(pHI);
            pHI = NULL;
        }
        else
        {
            SPYPRINTF(0, (__FUNCTION__": hDsk=%P Failed, LastError=%lU\n", pVI->hDsk, GetLastError()));
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        SPYPRINTF(0, (__FUNCTION__": hDsk=%P Exception in call!\n", pVI->hDsk));
    }

    return fSuccess;
}


/*-------------------------------------------------------------------
    Protected:

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
BOOL DLFILESPY_CreateDirectory(
    PVOLUME                 pVol,
    PCWSTR                  pwzPath,
    SECURITY_ATTRIBUTES    *pSecurityAttribs)
{
    SPYVOLINFO             *pVI = (SPYVOLINFO*)pVol;
    BOOL                    fSuccess = FALSE;

    if(!pVI)
    {
        SPYPRINTF(0, (__FUNCTION__": Bad pointer, pVol=%P\n", pVol));
        DclProductionError();
        return FALSE;
    }

    SPYPRINTF(0, (__FUNCTION__": hDsk=%P PathName=\"%W\"\n", pVI->hDsk, pwzPath));

    __try
    {
        fSuccess = pVI->FilterHook.pCreateDirectoryW(pVI->FilterHook.hVolume,
                                                     pwzPath,
                                                     pSecurityAttribs);

        if(fSuccess)
            SPYPRINTF(0, (__FUNCTION__": hDsk=%P OK\n", pVI->hDsk));
        else
            SPYPRINTF(0, (__FUNCTION__": hDsk=%P Failed, LastError=%lU\n", pVI->hDsk, GetLastError()));
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        SPYPRINTF(0, (__FUNCTION__": hDsk=%P Exception in call!\n", pVI->hDsk));
    }

    return fSuccess;
}


/*-------------------------------------------------------------------
    Protected:

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
HANDLE DLFILESPY_CreateFile(
    PVOLUME                 pVol,
    HANDLE                  hProc,
    LPCWSTR                 pwzFileName,
    DWORD                   dwAccess,
    DWORD                   dwShareMode,
    SECURITY_ATTRIBUTES    *pSecurityAttribs,
    DWORD                   dwCreate,
    DWORD                   dwFlagsAndAttribs,
    HANDLE                  hTemplate)
{
    SPYVOLINFO             *pVI = (SPYVOLINFO*)pVol;
    HANDLE                  hFile = INVALID_HANDLE_VALUE;

    if(!pVI)
    {
        SPYPRINTF(0, (__FUNCTION__": Bad pointer, pVol=%P\n", pVol));
        DclProductionError();
        return FALSE;
    }

    SPYPRINTF(0, (__FUNCTION__": hDsk=%P hProc=%P FileName=\"%W\" Access=%lX ShareMode=%lX Disposition=%lX FlagsAndAttribs=%lX\n",
        pVI->hDsk, hProc, pwzFileName, dwAccess, dwShareMode, dwCreate, dwFlagsAndAttribs));

    __try
    {
        hFile = pVI->FilterHook.pCreateFileW(pVI->FilterHook.hVolume,
                                             hProc,
                                             pwzFileName,
                                             dwAccess,
                                             dwShareMode,
                                             pSecurityAttribs,
                                             dwCreate,
                                             dwFlagsAndAttribs,
                                             hTemplate);

        if(hFile != INVALID_HANDLE_VALUE)
        {
            SPYHANDLEINFO *pHI = DclMemAllocZero(sizeof(*pHI));
            if(pHI)
            {
                pHI->pVolInfo = pVI;
                pHI->hObj = (DWORD)hFile;
 
                SPYPRINTF(0, (__FUNCTION__": hDsk=%P Succeeded with handle %P returning %P\n",
                    pVI->hDsk, hFile, pHI));

                hFile = (HANDLE)pHI;
            }
            else
            {
                SPYPRINTF(0, (__FUNCTION__": hDsk=%P Failed to allocate handle\n", pVI->hDsk));

                hFile = INVALID_HANDLE_VALUE;
            }
        }
        else
        {
            SPYPRINTF(0, (__FUNCTION__": hDsk=%P Failed, LastError=%lU\n", pVI->hDsk, GetLastError()));
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        SPYPRINTF(0, (__FUNCTION__": hDsk=%P Exception in call!\n", pVI->hDsk));
    }

    return hFile;
}


/*-------------------------------------------------------------------
    Protected:

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
BOOL DLFILESPY_DeleteAndRenameFile(
    PVOLUME     pVol,
    PCWSTR      pwzOldFile,
    PCWSTR      pwzNewFile)
{
    SPYVOLINFO *pVI = (SPYVOLINFO*)pVol;
    BOOL        fSuccess = TRUE;

    if(!pVI)
    {
        SPYPRINTF(0, (__FUNCTION__": Bad pointer, pVol=%P\n", pVol));
        DclProductionError();
        return FALSE;
    }

    SPYPRINTF(0, (__FUNCTION__": hDsk=%P DestFile=\"%W\" SourceFile=\"%W\"\n",
        pVI->hDsk, pwzOldFile, pwzNewFile));

    __try
    {
        fSuccess = pVI->FilterHook.pDeleteAndRenameFileW(pVI->FilterHook.hVolume,
                                                         pwzOldFile,
                                                         pwzNewFile);

        if(fSuccess)
            SPYPRINTF(0, (__FUNCTION__": hDsk=%P OK\n", pVI->hDsk));
        else
            SPYPRINTF(0, (__FUNCTION__": hDsk=%P Failed, LastError=%lU\n", pVI->hDsk, GetLastError()));
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        SPYPRINTF(0, (__FUNCTION__": hDsk=%P Exception in call!\n", pVI->hDsk));
    }

    return fSuccess;
}


/*-------------------------------------------------------------------
    Protected:

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
BOOL DLFILESPY_DeleteFile(
    PVOLUME     pVol,
    PCWSTR      pwzFileName)
{
    SPYVOLINFO *pVI = (SPYVOLINFO*)pVol;
    BOOL        fSuccess = TRUE;

    if(!pVI)
    {
        SPYPRINTF(0, (__FUNCTION__": Bad pointer, pVol=%P\n", pVol));
        DclProductionError();
        return FALSE;
    }

    SPYPRINTF(0, (__FUNCTION__": hDsk=%P pwzFileName=\"%W\"\n", pVI->hDsk, pwzFileName));

    __try
    {
        fSuccess = pVI->FilterHook.pDeleteFileW(pVI->FilterHook.hVolume, pwzFileName);

        if(fSuccess)
            SPYPRINTF(0, (__FUNCTION__": hDsk=%P OK\n", pVI->hDsk));
        else
            SPYPRINTF(0, (__FUNCTION__": hDsk=%P Failed, LastError=%lU\n", pVI->hDsk, GetLastError()));
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        SPYPRINTF(0, (__FUNCTION__": hDsk=%P Exception in call!\n", pVI->hDsk));
    }

    return fSuccess;
}


/*-------------------------------------------------------------------
    Protected:

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
BOOL DLFILESPY_FindClose(
    PSEARCH         pSearch)
{
    SPYHANDLEINFO  *pHI = (SPYHANDLEINFO*)pSearch;
    SPYVOLINFO     *pVI = (SPYVOLINFO*)(pHI ? pHI->pVolInfo : NULL);
    BOOL            fSuccess = TRUE;

    SPYPRINTF(0, (__FUNCTION__": hDsk=%P hFindFile=%P\n", pVI->hDsk, pSearch));

    __try
    {
        fSuccess = pVI->FilterHook.pFindClose(pHI->hObj);

        if(fSuccess)
        {
            SPYPRINTF(0, (__FUNCTION__": hDsk=%P OK\n", pVI->hDsk));

            DclMemFree(pHI);
            pHI = NULL;
        }
        else
        {
            SPYPRINTF(0, (__FUNCTION__": hDsk=%P Failed, LastError=%lU\n", pVI->hDsk, GetLastError()));
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        SPYPRINTF(0, (__FUNCTION__": hDsk=%P Exception in call!\n", pVI->hDsk));
    }

    return fSuccess;
}


/*-------------------------------------------------------------------
    Protected:

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
HANDLE DLFILESPY_FindFirstFile(
    PVOLUME             pVol,
    HANDLE              hProc,
    PCWSTR              pwzFileSpec,
    WIN32_FIND_DATAW   *pFD)
{
    SPYVOLINFO         *pVI = (SPYVOLINFO*)pVol;
    HANDLE              hSearch = INVALID_HANDLE_VALUE;

    if(!pVI)
    {
        SPYPRINTF(0, (__FUNCTION__": Bad pointer, pVol=%P\n", pVol));
        DclProductionError();
        return hSearch;
    }
    
    SPYPRINTF(0, (__FUNCTION__": hDsk=%P hProc=%P FileName=\"%W\" pFindFileData=%P\n",
        pVI->hDsk, hProc, pwzFileSpec, pFD));

    __try
    {
        hSearch = pVI->FilterHook.pFindFirstFileW(pVI->FilterHook.hVolume,
                                                  hProc,
                                                  pwzFileSpec,
                                                  pFD);

        if(hSearch != INVALID_HANDLE_VALUE)
        {
            SPYHANDLEINFO *pHI = DclMemAllocZero(sizeof(*pHI));
            if(pHI)
            {
                pHI->pVolInfo = pVI;
                pHI->hObj = (DWORD)hSearch;
                hSearch = (HANDLE)pHI;
            }
            else
            {
                SPYPRINTF(0, (__FUNCTION__": hDsk=%P Failed to allocate handle\n", pVI->hDsk));

                pVI->FilterHook.pFindClose((DWORD)hSearch);
                hSearch = INVALID_HANDLE_VALUE;
            }
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        SPYPRINTF(0, (__FUNCTION__": hDsk=%P Exception in call!\n", pVI->hDsk));

        hSearch = INVALID_HANDLE_VALUE;
    }

    return hSearch;
}


/*-------------------------------------------------------------------
    Protected:

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
BOOL DLFILESPY_FindNextFile(
    PSEARCH             pSearch,
    WIN32_FIND_DATAW   *pFD)
{   
    SPYHANDLEINFO      *pHI = (SPYHANDLEINFO*)pSearch;
    SPYVOLINFO         *pVI = (SPYVOLINFO*)(pHI ? pHI->pVolInfo : NULL);
    BOOL                fSuccess = FALSE;

    if(!pHI)
    {
        SPYPRINTF(0, (__FUNCTION__": Bad handle, pSearch=%P\n", pSearch));
        DclProductionError();
        return FALSE;
    }

    SPYPRINTF(0, (__FUNCTION__": hDsk=%P hFindFile=%P pFindFileData=%P\n",
        pVI->hDsk, pSearch, pFD));

    __try
    {
        fSuccess = pVI->FilterHook.pFindNextFileW(pHI->hObj, pFD);

        if(fSuccess)
            SPYPRINTF(0, (__FUNCTION__": hDsk=%P OK\n", pVI->hDsk));
        else
            SPYPRINTF(0, (__FUNCTION__": hDsk=%P Failed, LastError=%lU\n", pVI->hDsk, GetLastError()));
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        SPYPRINTF(0, (__FUNCTION__": hDsk=%P Exception in call!\n", pVI->hDsk));
    }

    return fSuccess;
}


/*-------------------------------------------------------------------
    Protected:

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
BOOL DLFILESPY_FlushFileBuffers(
    PFILE           pFH)
{
    SPYHANDLEINFO  *pHI = (SPYHANDLEINFO*)pFH;
    SPYVOLINFO     *pVI = (SPYVOLINFO*)(pHI ? pHI->pVolInfo : NULL);
    BOOL            fSuccess = FALSE;

    if(!pFH)
    {
        SPYPRINTF(0, (__FUNCTION__": Bad handle, pFile=%P\n", pFH));
        DclProductionError();
        return FALSE;
    }

    SPYPRINTF(0, (__FUNCTION__": hDsk=%P hFile=%P\n", pVI->hDsk, pFH));

    __try
    {
        fSuccess = pVI->FilterHook.pFlushFileBuffers(pHI->hObj);

        if(fSuccess)
            SPYPRINTF(0, (__FUNCTION__": hDsk=%P OK\n", pVI->hDsk));
        else
            SPYPRINTF(0, (__FUNCTION__": hDsk=%P Failed, LastError=%lU\n", pVI->hDsk, GetLastError()));
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        SPYPRINTF(0, (__FUNCTION__": hDsk=%P Exception in call!\n", pVI->hDsk));
    }

    return fSuccess;
}


/*-------------------------------------------------------------------
    Protected:

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
BOOL DLFILESPY_DeviceIoControl(
    PFILE           pFH,
    DWORD           dwIoControlCode,
    void           *pInBuf,
    DWORD           dwInBufSize,
    void           *pOutBuf,
    DWORD           dwOutBufSize,
    DWORD          *pdwBytesReturned,
    OVERLAPPED     *pOverlapped)
{
    SPYHANDLEINFO  *pHI = (SPYHANDLEINFO*)pFH;
    SPYVOLINFO     *pVI = (SPYVOLINFO*)(pHI ? pHI->pVolInfo : NULL);
    BOOL            fSuccess = FALSE;

    if(!pFH)
    {
        SPYPRINTF(0, (__FUNCTION__": Bad handle, pFile=%P\n", pFH));
        DclProductionError();
        return FALSE;
    }

    SPYPRINTF(0, (__FUNCTION__": hDsk=%P hFile=%P IoControlCode=%lX pInBuffer=%P InBufferSize=%lU pOutBuffer=%P OutBufferSize=%lU pdwBytesReturned=%P\n",
        pVI->hDsk, pFH, dwIoControlCode, pInBuf, dwInBufSize, pOutBuf, pdwBytesReturned));

    __try
    {
        if(dwIoControlCode == DCL_IOCTL_FILESYS)
        {
            DCLIOREQUEST   *pReqHdr = (DCLIOREQUEST*)pInBuf;
            
            DclAssert(pInBuf == pOutBuf);
            DclAssert(dwInBufSize == dwOutBufSize);

            switch(pReqHdr->ioFunc)
            {
                case DCLIOFUNC_FILESPY_TRACEMASK:
                {
                    DCLREQ_FILESPY_TRACEMASK *pReq = (DCLREQ_FILESPY_TRACEMASK*)pReqHdr;

                    if(pReq->ior.ulReqLen != sizeof(*pReq))
                    {
                        DclError();
                    }
                    else
                    {
                        DclPrintf(__FUNCTION__": DCLIOFUNC_FILESPY_TRACEMASK hDsk=%P OldMask=%lX NewMask=%lX\n", 
                            pVI->hDsk, pVI->ulFlags, pReq->ulNewTraceMask);

                        pReq->ulOldTraceMask = pVI->ulFlags;

                        if(pReq->ulNewTraceMask != DCLFILESPYFLAGS_QUERY)
                            pVI->ulFlags = pReq->ulNewTraceMask;

                        pReq->dclStat = DCLSTAT_SUCCESS;

                        *pdwBytesReturned = sizeof(*pReq);

                        fSuccess = TRUE;
                    }
                    
                    break;                        
                }
            }
        }

        if(!fSuccess)
        {
            fSuccess = pVI->FilterHook.pDeviceIoControl(pHI->hObj,
                                                        dwIoControlCode,
                                                        pInBuf,
                                                        dwInBufSize,
                                                        pOutBuf,
                                                        dwOutBufSize,
                                                        pdwBytesReturned,
                                                        pOverlapped);

            if(fSuccess)
                SPYPRINTF(0, (__FUNCTION__": hDsk=%P OK\n", pVI->hDsk));
            else
                SPYPRINTF(0, (__FUNCTION__": hDsk=%P Failed, LastError=%lU\n", pVI->hDsk, GetLastError()));
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        SPYPRINTF(0, (__FUNCTION__": hDsk=%P Exception in call!\n", pVI->hDsk));
    }

    return fSuccess;
}


/*-------------------------------------------------------------------
    Protected:

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
BOOL DLFILESPY_FsIoControl(
    PVOLUME         pVol,
    DWORD           dwIoControlCode,
    void           *pInBuf,
    DWORD           dwInBufSize,
    void           *pOutBuf,
    DWORD           dwOutBufSize,
    DWORD          *pdwBytesReturned,
    OVERLAPPED     *pOverlapped)
{
    SPYVOLINFO     *pVI = (SPYVOLINFO*)pVol;
    BOOL            fSuccess = FALSE;

    if(!pVI)
    {
        SPYPRINTF(0, (__FUNCTION__": Bad pointer, pVol=%P\n", pVol));
        DclProductionError();
        return FALSE;
    }

    SPYPRINTF(0, (__FUNCTION__": hDsk=%P IoControlCode=%lX pInBuf=%P InBufSize=%lU pOutBuf=%P OutBufSize=%lU pdwBytesReturned=%P\n",
        pVI->hDsk, dwIoControlCode, pInBuf, dwInBufSize, pOutBuf, dwOutBufSize, pdwBytesReturned));

    __try
    {
        fSuccess = pVI->FilterHook.pFsIoControl(pVI->FilterHook.hVolume,
                                                dwIoControlCode,
                                                pInBuf,
                                                dwInBufSize,
                                                pOutBuf,
                                                dwOutBufSize,
                                                pdwBytesReturned,
                                                pOverlapped);

        if(fSuccess)
            SPYPRINTF(0, (__FUNCTION__": hDsk=%P OK\n", pVI->hDsk));
        else
            SPYPRINTF(0, (__FUNCTION__": hDsk=%P Failed, LastError=%lU\n", pVI->hDsk, GetLastError()));
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        SPYPRINTF(0, (__FUNCTION__": hDsk=%P Exception in call!\n", pVI->hDsk));
    }

    return fSuccess;
}


/*-------------------------------------------------------------------
    Protected:

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
BOOL DLFILESPY_GetDiskFreeSpace(
    PVOLUME         pVol,
    PCWSTR          pwzPath,
    DWORD          *pdwSectorsPerCluster,
    DWORD          *pdwBytesPerSector,
    DWORD          *pdwFreeClusters,
    DWORD          *pdwClusters)
{
    SPYVOLINFO     *pVI = (SPYVOLINFO*)pVol;
    BOOL            fSuccess = TRUE;

    if(!pVI)
    {
        SPYPRINTF(0, (__FUNCTION__": Bad pointer, pVol=%P\n", pVol));
        DclProductionError();
        return FALSE;
    }

    SPYPRINTF(0, (__FUNCTION__": hDsk=%P Path=\"%W\"\n", pVI->hDsk, pwzPath));

    __try
    {
        fSuccess = pVI->FilterHook.pGetDiskFreeSpaceW(pVI->FilterHook.hVolume,
                                                      pwzPath,
                                                      pdwSectorsPerCluster,
                                                      pdwBytesPerSector,
                                                      pdwFreeClusters,
                                                      pdwClusters);

        if(fSuccess)
            SPYPRINTF(0, (__FUNCTION__": hDsk=%P OK\n", pVI->hDsk));
        else
            SPYPRINTF(0, (__FUNCTION__": hDsk=%P Failed, LastError=%lU\n", pVI->hDsk, GetLastError()));
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        SPYPRINTF(0, (__FUNCTION__": hDsk=%P Exception in call!\n", pVI->hDsk));
    }

    return fSuccess;
}


/*-------------------------------------------------------------------
    Protected:

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
DWORD DLFILESPY_GetFileAttributes(
    PVOLUME     pVol,
    PCWSTR      pwzFileName)
{
    SPYVOLINFO *pVI = (SPYVOLINFO*)pVol;
    DWORD       dwReturn = 0;

    if(!pVI)
    {
        SPYPRINTF(0, (__FUNCTION__": Bad pointer, pVol=%P\n", pVol));
        DclProductionError();
        return FALSE;
    }

    SPYPRINTF(0, (__FUNCTION__": hDsk=%P FileName=\"%W\"\n", pVI->hDsk, pwzFileName));

    __try
    {
        dwReturn = pVI->FilterHook.pGetFileAttributesW(pVI->FilterHook.hVolume, pwzFileName);

        if(0xFFFFFFFF != dwReturn)
        {
            SPYPRINTF(0, (__FUNCTION__": hDsk=%P returning %lX\n", pVI->hDsk, dwReturn));
        }
        else
        {
            SPYPRINTF(0, (__FUNCTION__": hDsk=%P Failed, LastError=%lU\n", pVI->hDsk, GetLastError()));
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        SPYPRINTF(0, (__FUNCTION__": hDsk=%P Exception in call!\n", pVI->hDsk));
    }

    return dwReturn;
}


/*-------------------------------------------------------------------
    Protected:

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
BOOL DLFILESPY_GetFileInfoByHandle(
    PFILE                           pFH,
    BY_HANDLE_FILE_INFORMATION     *pFileInfo)
{
    SPYHANDLEINFO                  *pHI = (SPYHANDLEINFO*)pFH;
    SPYVOLINFO                     *pVI = (SPYVOLINFO*)(pHI ? pHI->pVolInfo : NULL);
    BOOL                            fSuccess = FALSE;

    if(!pFH)
    {
        SPYPRINTF(0, (__FUNCTION__": Bad handle, pFile=%P\n", pFH));
        DclProductionError();
        return FALSE;
    }

    SPYPRINTF(0, (__FUNCTION__": hDsk=%P hFile=%P pFileInfo=%P\n", pVI->hDsk, pFH, pFileInfo));

    __try
    {
        fSuccess = pVI->FilterHook.pGetFileInformationByHandle(pHI->hObj, pFileInfo);

        if(fSuccess)
        {
            SPYPRINTF(0, (__FUNCTION__": hDsk=%P returning dwAttributes=%lX\n",
                pVI->hDsk, pFileInfo->dwFileAttributes));
        }
        else
        {
            SPYPRINTF(0, (__FUNCTION__": hDsk=%P Failed, LastError=%lU\n", pVI->hDsk, GetLastError()));
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        SPYPRINTF(0, (__FUNCTION__": hDsk=%P Exception in call!\n", pVI->hDsk));
    }

    return fSuccess;
}


/*-------------------------------------------------------------------
    Protected:

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
DWORD DLFILESPY_GetFileSize(
    PFILE           pFH,
    DWORD          *pdwFileSizeHigh)
{
    SPYHANDLEINFO  *pHI = (SPYHANDLEINFO*)pFH;
    SPYVOLINFO     *pVI = (SPYVOLINFO*)(pHI ? pHI->pVolInfo : NULL);
    DWORD           dwReturn = 0;

    if(!pFH)
    {
        SPYPRINTF(0, (__FUNCTION__": Bad handle, pFile=%P\n", pFH));
        DclProductionError();
        return dwReturn;
    }

    SPYPRINTF(0, (__FUNCTION__": hDsk=%P hFile=%P pdwFileSizeHigh=%P\n",
        pVI->hDsk, pFH, pdwFileSizeHigh));

    __try
    {
        dwReturn = pVI->FilterHook.pGetFileSize(pHI->hObj, pdwFileSizeHigh);

        SPYPRINTF(0, (__FUNCTION__": hDsk=%P Returning %lX\n", pVI->hDsk, dwReturn));
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        SPYPRINTF(0, (__FUNCTION__": hDsk=%P Exception in call!\n", pVI->hDsk));
    }

    return dwReturn;
}


/*-------------------------------------------------------------------
    Protected:

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
BOOL DLFILESPY_GetFileTime(
    PFILE           pFH,
    FILETIME       *pftCreation,
    FILETIME       *pftLastAccess,
    FILETIME       *pftLastWrite)
{
    SPYHANDLEINFO  *pHI = (SPYHANDLEINFO*)pFH;
    SPYVOLINFO     *pVI = (SPYVOLINFO*)(pHI ? pHI->pVolInfo : NULL);
    BOOL            fSuccess = FALSE;

    if(!pFH)
    {
        SPYPRINTF(0, (__FUNCTION__": Bad handle, pFile=%P\n", pFH));
        DclProductionError();
        return FALSE;
    }

    SPYPRINTF(0, (__FUNCTION__": hDsk=%P hFile=%P pftCreationTime=%P pftLastAccessTime=%P pftLastWriteTime=%P\n",
        pVI->hDsk, pFH, pftCreation, pftLastAccess, pftLastWrite));

    __try
    {
        fSuccess = pVI->FilterHook.pGetFileTime(pHI->hObj,
                                                pftCreation,
                                                pftLastAccess,
                                                pftLastWrite);

        if(fSuccess)
            SPYPRINTF(0, (__FUNCTION__": hDsk=%P OK\n", pVI->hDsk));
        else
            SPYPRINTF(0, (__FUNCTION__": hDsk=%P Failed, LastError=%lU\n", pVI->hDsk, GetLastError()));
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        SPYPRINTF(0, (__FUNCTION__": hDsk=%P Exception in call!\n", pVI->hDsk));
    }

    return fSuccess;
}


/*-------------------------------------------------------------------
    Protected:

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
BOOL DLFILESPY_GetVolumeInfo(
    PVOLUME             pVol,
    FSD_VOLUME_INFO    *pInfo)
{
    SPYVOLINFO         *pVI = (SPYVOLINFO*)pVol;
    BOOL                fSuccess = FALSE;

    if(!pVI)
    {
        SPYPRINTF(0, (__FUNCTION__": Bad pointer, pVol=%P\n", pVol));
        DclProductionError();
        return FALSE;
    }

    SPYPRINTF(0, (__FUNCTION__": hDsk=%P pInfo=%P\n", pVI->hDsk, pInfo));

    __try
    {
        fSuccess = pVI->FilterHook.pGetVolumeInfo(pVI->FilterHook.hVolume, pInfo);

        if(fSuccess)
            SPYPRINTF(0, (__FUNCTION__": hDsk=%P OK\n", pVI->hDsk));
        else
            SPYPRINTF(0, (__FUNCTION__": hDsk=%P Failed, LastError=%lU\n", pVI->hDsk, GetLastError()));
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        SPYPRINTF(0, (__FUNCTION__": hDsk=%P Exception in call!\n", pVI->hDsk));
    }

    return fSuccess;
}


/*-------------------------------------------------------------------
    Protected:

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
BOOL DLFILESPY_MoveFile(
    PVOLUME     pVol,
    PCWSTR      pwzOldFileName,
    PCWSTR      pwzNewFileName)
{
    SPYVOLINFO *pVI = (SPYVOLINFO*)pVol;
    BOOL        fSuccess = TRUE;

    if(!pVI)
    {
        SPYPRINTF(0, (__FUNCTION__": Bad pointer, pVol=%P\n", pVol));
        DclProductionError();
        return FALSE;
    }

    SPYPRINTF(0, (__FUNCTION__": hDsk=%P OldFileName=\"%W\" NewFileName=\"%W\"\n",
        pVI->hDsk, pwzOldFileName, pwzNewFileName));

    __try
    {
        fSuccess = pVI->FilterHook.pMoveFileW(pVI->FilterHook.hVolume, pwzOldFileName, pwzNewFileName);

        if(fSuccess)
            SPYPRINTF(0, (__FUNCTION__": hDsk=%P OK\n", pVI->hDsk));
        else
            SPYPRINTF(0, (__FUNCTION__": hDsk=%P Failed, LastError=%lU\n", pVI->hDsk, GetLastError()));
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        SPYPRINTF(0, (__FUNCTION__": hDsk=%P Exception in call!\n", pVI->hDsk));
    }

    return fSuccess;
}


/*-------------------------------------------------------------------
    Protected:

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
VOID DLFILESPY_Notify(
    PVOLUME         pVol,
    DWORD           dwFlags)
{
    SPYVOLINFO     *pVI = (SPYVOLINFO*)pVol;

    if(!pVI)
    {
        SPYPRINTF(0, (__FUNCTION__": Bad pointer, pVol=%P\n", pVol));
        DclProductionError();
        return;
    }

    SPYPRINTF(0, (__FUNCTION__": hDsk=%P dwFlags=%lX\n", pVI->hDsk, dwFlags));

    __try
    {
        pVI->FilterHook.pNotify(pVI->FilterHook.hVolume, dwFlags);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        SPYPRINTF(0, (__FUNCTION__": hDsk=%P Exception in call!\n", pVI->hDsk));
    }

    return;
}


/*-------------------------------------------------------------------
    Protected:

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
BOOL DLFILESPY_ReadFile(
    PFILE           pFH,
    void           *pBuffer,
    DWORD           dwBytesToRead,
    DWORD          *pdwBytesRead,
    OVERLAPPED     *pOverlapped)
{
    SPYHANDLEINFO  *pHI = (SPYHANDLEINFO*)pFH;
    SPYVOLINFO     *pVI = (SPYVOLINFO*)(pHI ? pHI->pVolInfo : NULL);
    BOOL            fSuccess = FALSE;

    if(!pFH)
    {
        SPYPRINTF(0, (__FUNCTION__": Bad handle, pFile=%P\n", pFH));
        DclProductionError();
        return FALSE;
    }

    SPYPRINTF(0, (__FUNCTION__": hDsk=%P hFile=%P pBuffer=%P BytesToRead=%lU pBytesRead=%P\n",
        pVI->hDsk, pFH, pBuffer, dwBytesToRead, pdwBytesRead));

    __try
    {
        fSuccess = pVI->FilterHook.pReadFile(pHI->hObj,
                                             pBuffer,
                                             dwBytesToRead,
                                             pdwBytesRead,
                                             pOverlapped);

        if(fSuccess)
        {
            SPYPRINTF(0, (__FUNCTION__": hDsk=%P returning %lU bytes\n",
                pVI->hDsk, pdwBytesRead ? *pdwBytesRead : 0));
        }
        else
        {
            SPYPRINTF(0, (__FUNCTION__": hDsk=%P Failed, LastError=%lU\n", pVI->hDsk, GetLastError()));
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        SPYPRINTF(0, (__FUNCTION__": hDsk=%P Exception in call!\n", pVI->hDsk));
    }

    return fSuccess;
}


/*-------------------------------------------------------------------
    Protected:

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
BOOL DLFILESPY_ReadFileScatter(
    PFILE                   pFH,
    FILE_SEGMENT_ELEMENT    aSegments[],
    DWORD                   dwBytesToRead,
    DWORD                  *pReserved,
    OVERLAPPED             *pOverlapped)
{
    SPYHANDLEINFO          *pHI = (SPYHANDLEINFO*)pFH;
    SPYVOLINFO             *pVI = (SPYVOLINFO*)(pHI ? pHI->pVolInfo : NULL);
    BOOL                    fSuccess = FALSE;

    if(!pFH)
    {
        SPYPRINTF(0, (__FUNCTION__": Bad handle, pFile=%P\n", pFH));
        DclProductionError();
        return FALSE;
    }

    SPYPRINTF(0, (__FUNCTION__": hDsk=%P hFile=%P aSegments=%P BytesToRead=%lU pReserved=%P\n",
        pVI->hDsk, pFH, aSegments, dwBytesToRead, pReserved));

    __try
    {
        fSuccess = pVI->FilterHook.pReadFileScatter(pHI->hObj,
                                                    aSegments,
                                                    dwBytesToRead,
                                                    pReserved,
                                                    pOverlapped);

        if(fSuccess)
        {
            SPYPRINTF(0, (__FUNCTION__": hDsk=%P returning %lU bytes\n",
                pVI->hDsk, dwBytesToRead));
        }
        else
        {
            SPYPRINTF(0, (__FUNCTION__": hDsk=%P Failed, LastError=%lU\n", pVI->hDsk, GetLastError()));
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        SPYPRINTF(0, (__FUNCTION__": hDsk=%P Exception in call!\n", pVI->hDsk));
    }

    return fSuccess;
}


/*-------------------------------------------------------------------
    Protected:

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
BOOL DLFILESPY_ReadFileWithSeek(
    PFILE           pFH,
    void           *pBuffer,
    DWORD           dwBytesToRead,
    DWORD          *pdwBytesRead,
    OVERLAPPED     *pOverlapped,
    DWORD           dwLowOffset,
    DWORD           dwHighOffset)
{
    SPYHANDLEINFO  *pHI = (SPYHANDLEINFO*)pFH;
    SPYVOLINFO     *pVI = (SPYVOLINFO*)(pHI ? pHI->pVolInfo : NULL);
    BOOL            fSuccess = FALSE;

    if(!pFH)
    {
        SPYPRINTF(0, (__FUNCTION__": Bad handle, pFile=%P\n", pFH));
        DclProductionError();
        return FALSE;
    }

    SPYPRINTF(0, (__FUNCTION__": hDsk=%P hFile=%P pBuffer=%P BytesToRead=%lU pBytesRead=%P LowOffset=%lU HighOffset=%lU\n",
        pVI->hDsk, pFH, pBuffer, dwBytesToRead, pdwBytesRead, dwLowOffset, dwHighOffset));

    __try
    {
        fSuccess = pVI->FilterHook.pReadFileWithSeek(pHI->hObj,
                                                     pBuffer,
                                                     dwBytesToRead,
                                                     pdwBytesRead,
                                                     pOverlapped,
                                                     dwLowOffset,
                                                     dwHighOffset);

        if(fSuccess)
        {
            SPYPRINTF(0, (__FUNCTION__": hDsk=%P returning %lU bytes\n",
                pVI->hDsk, pdwBytesRead ? *pdwBytesRead : 0));
        }
        else
        {
            SPYPRINTF(0, (__FUNCTION__": hDsk=%P Failed, LastError=%lU\n", pVI->hDsk, GetLastError()));
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        SPYPRINTF(0, (__FUNCTION__": hDsk=%P Exception in call!\n", pVI->hDsk));
    }

    return fSuccess;
}


/*-------------------------------------------------------------------
    Protected:

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
BOOL DLFILESPY_RegisterFileSysFunc(
    PVOLUME                 pVol,
    SHELLFILECHANGEFUNC_t   pfn)
{
    SPYVOLINFO             *pVI = (SPYVOLINFO*)pVol;
    BOOL                    fSuccess = TRUE;

    if(!pVI)
    {
        SPYPRINTF(0, (__FUNCTION__": Bad pointer, pVol=%P\n", pVol));
        DclProductionError();
        return FALSE;
    }

    SPYPRINTF(0, (__FUNCTION__": hDsk=%P pFN=%P\n", pVI->hDsk, pfn));

    __try
    {
        fSuccess = pVI->FilterHook.pRegisterFileSystemFunction(pVI->FilterHook.hVolume, pfn);

        if(fSuccess)
            SPYPRINTF(0, (__FUNCTION__": hDsk=%P OK\n", pVI->hDsk));
        else
            SPYPRINTF(0, (__FUNCTION__": hDsk=%P Failed, LastError=%lU\n", pVI->hDsk, GetLastError()));
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        SPYPRINTF(0, (__FUNCTION__": hDsk=%P Exception in call!\n", pVI->hDsk));
    }

    return fSuccess;
}


/*-------------------------------------------------------------------
    Protected:

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
BOOL DLFILESPY_RemoveDirectory(
    PVOLUME     pVol,
    PCWSTR      pwzPath)
{
    SPYVOLINFO *pVI = (SPYVOLINFO*)pVol;
    BOOL        fSuccess = FALSE;

    if(!pVI)
    {
        SPYPRINTF(0, (__FUNCTION__": Bad pointer, pVol=%P\n", pVol));
        DclProductionError();
        return FALSE;
    }

    SPYPRINTF(0, (__FUNCTION__": hDsk=%P PathName=\"%W\"\n", pVI->hDsk, pwzPath));

    __try
    {
        fSuccess = pVI->FilterHook.pRemoveDirectoryW(pVI->FilterHook.hVolume, pwzPath);

        if(fSuccess)
            SPYPRINTF(0, (__FUNCTION__": hDsk=%P OK\n", pVI->hDsk));
        else
            SPYPRINTF(0, (__FUNCTION__": hDsk=%P Failed, LastError=%lU\n", pVI->hDsk, GetLastError()));
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        SPYPRINTF(0, (__FUNCTION__": hDsk=%P Exception in call!\n", pVI->hDsk));
    }

    return fSuccess;
}


/*-------------------------------------------------------------------
    Protected:

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
BOOL DLFILESPY_SetEndOfFile(
    PFILE           pFH)
{
    SPYHANDLEINFO  *pHI = (SPYHANDLEINFO*)pFH;
    SPYVOLINFO     *pVI = (SPYVOLINFO*)(pHI ? pHI->pVolInfo : NULL);
    BOOL            fSuccess = FALSE;

    if(!pFH)
    {
        SPYPRINTF(0, (__FUNCTION__": Bad handle, pFile=%P\n", pFH));
        DclProductionError();
        return FALSE;
    }

    SPYPRINTF(0, (__FUNCTION__": hDsk=%P hFile=%P\n", pVI->hDsk, pFH));

    __try
    {
        fSuccess = pVI->FilterHook.pSetEndOfFile(pHI->hObj);

        if(fSuccess)
            SPYPRINTF(0, (__FUNCTION__": hDsk=%P OK\n", pVI->hDsk));
        else
            SPYPRINTF(0, (__FUNCTION__": hDsk=%P Failed, LastError=%lU\n", pVI->hDsk, GetLastError()));
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        SPYPRINTF(0, (__FUNCTION__": hDsk=%P Exception in call!\n", pVI->hDsk));
    }

    return fSuccess;
}


/*-------------------------------------------------------------------
    Protected:

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
BOOL DLFILESPY_SetFileAttributes(
    PVOLUME     pVol,
    PCWSTR      pwzFileName,
    DWORD       dwAttributes)
{
    SPYVOLINFO *pVI = (SPYVOLINFO*)pVol;
    BOOL        fSuccess = TRUE;

    if(!pVI)
    {
        SPYPRINTF(0, (__FUNCTION__": Bad pointer, pVol=%P\n", pVol));
        DclProductionError();
        return FALSE;
    }

    SPYPRINTF(0, (__FUNCTION__": hDsk=%P FileName=\"%W\" Attribs=%lX\n",
        pVI->hDsk, pwzFileName, dwAttributes));

    __try
    {
        fSuccess = pVI->FilterHook.pSetFileAttributesW(pVI->FilterHook.hVolume,
                                                       pwzFileName,
                                                       dwAttributes);

        if(fSuccess)
            SPYPRINTF(0, (__FUNCTION__": hDsk=%P OK\n", pVI->hDsk));
        else
            SPYPRINTF(0, (__FUNCTION__": hDsk=%P Failed, LastError=%lU\n", pVI->hDsk, GetLastError()));
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        SPYPRINTF(0, (__FUNCTION__": hDsk=%P Exception in call!\n", pVI->hDsk));
    }

    return fSuccess;
}


/*-------------------------------------------------------------------
    Protected:

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
DWORD DLFILESPY_SetFilePointer(
    PFILE           pFH,
    LONG            lDistanceToMove,
    LONG           *plDistanceToMoveHigh,
    DWORD           dwMoveMethod)
{
    SPYHANDLEINFO  *pHI = (SPYHANDLEINFO*)pFH;
    SPYVOLINFO     *pVI = (SPYVOLINFO*)(pHI ? pHI->pVolInfo : NULL);
    DWORD           dwReturn = 0;

    if(!pFH)
    {
        SPYPRINTF(0, (__FUNCTION__": Bad handle, pFile=%P\n", pFH));
        DclProductionError();
        return FALSE;
    }

    SPYPRINTF(0, (__FUNCTION__": hDsk=%P hFile=%P lDistanceToMove=%lD plDistanceToMoveHigh=%P dwMoveMethod=%lU\n",
        pVI->hDsk, pFH, lDistanceToMove, plDistanceToMoveHigh, dwMoveMethod));

    __try
    {
        dwReturn = pVI->FilterHook.pSetFilePointer(pHI->hObj,
                                                   lDistanceToMove,
                                                   plDistanceToMoveHigh,
                                                   dwMoveMethod);

        if(dwReturn != -1)
        {
            SPYPRINTF(0, (__FUNCTION__": hDsk=%P returning %lU\n", pVI->hDsk, dwReturn));
        }
        else
        {
            SPYPRINTF(0, (__FUNCTION__": hDsk=%P Failed, LastError=%lU\n", pVI->hDsk, GetLastError()));
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        SPYPRINTF(0, (__FUNCTION__": hDsk=%P Exception in call!\n", pVI->hDsk));
    }

    return dwReturn;
}


/*-------------------------------------------------------------------
    Protected:

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
BOOL DLFILESPY_SetFileTime(
    PFILE           pFH,
    const FILETIME *pftCreation,
    const FILETIME *pftLastAccess,
    const FILETIME *pftLastWrite)
{
    SPYHANDLEINFO  *pHI = (SPYHANDLEINFO*)pFH;
    SPYVOLINFO     *pVI = (SPYVOLINFO*)(pHI ? pHI->pVolInfo : NULL);
    BOOL            fSuccess = FALSE;

    if(!pFH)
    {
        SPYPRINTF(0, (__FUNCTION__": Bad handle, pFile=%P\n", pFH));
        DclProductionError();
        return FALSE;
    }

    SPYPRINTF(0, (__FUNCTION__": hDsk=%P hFile=%P pftCreationTime=%P pftLastAccessTime=%P pftLastWriteTime=%P\n",
        pVI->hDsk, pFH, pftCreation, pftLastAccess, pftLastWrite));

    __try
    {
        fSuccess = pVI->FilterHook.pSetFileTime(pHI->hObj,
                                                pftCreation,
                                                pftLastAccess,
                                                pftLastWrite);

        if(fSuccess)
            SPYPRINTF(0, (__FUNCTION__": hDsk=%P OK\n", pVI->hDsk));
        else
            SPYPRINTF(0, (__FUNCTION__": hDsk=%P Failed, LastError=%lU\n", pVI->hDsk, GetLastError()));
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        SPYPRINTF(0, (__FUNCTION__": hDsk=%P Exception in call!\n", pVI->hDsk));
    }

    return fSuccess;
}


/*-------------------------------------------------------------------
    Protected:

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
BOOL DLFILESPY_WriteFile(
    PFILE           pFH,
    const void     *pBuffer,
    DWORD           dwBytesToWrite,
    DWORD          *pdwBytesWritten,
    OVERLAPPED     *pOverlapped)
{
    SPYHANDLEINFO  *pHI = (SPYHANDLEINFO*)pFH;
    SPYVOLINFO     *pVI = (SPYVOLINFO*)(pHI ? pHI->pVolInfo : NULL);
    BOOL            fSuccess = FALSE;

    if(!pFH)
    {
        SPYPRINTF(0, (__FUNCTION__": Bad handle, pFile=%P\n", pFH));
        DclProductionError();
        return FALSE;
    }

    SPYPRINTF(0, (__FUNCTION__": hDsk=%P hFile=%P pBuffer=%P BytesToWrite=%lU pdwBytesWritten=%P\n",
        pVI->hDsk, pFH, pBuffer, dwBytesToWrite, pdwBytesWritten));

    __try
    {
        fSuccess = pVI->FilterHook.pWriteFile(pHI->hObj,
                                              pBuffer,
                                              dwBytesToWrite,
                                              pdwBytesWritten,
                                              pOverlapped);

        if(fSuccess)
        {
            SPYPRINTF(0, (__FUNCTION__": hDsk=%P wrote %lU bytes\n",
                pVI->hDsk, pdwBytesWritten ? *pdwBytesWritten : 0));
        }
        else
        {
            SPYPRINTF(0, (__FUNCTION__": hDsk=%P Failed, LastError=%lU\n", pVI->hDsk, GetLastError()));
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        SPYPRINTF(0, (__FUNCTION__": hDsk=%P Exception in call!\n", pVI->hDsk));
    }

    return fSuccess;
}


/*-------------------------------------------------------------------
    Protected:

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
BOOL DLFILESPY_WriteFileWithSeek(
    PFILE           pFH,
    const void     *pBuffer,
    DWORD           dwBytesToWrite,
    DWORD          *pdwBytesWritten,
    OVERLAPPED     *pOverlapped,
    DWORD           dwLowOffset,
    DWORD           dwHighOffset)
{
    SPYHANDLEINFO  *pHI = (SPYHANDLEINFO*)pFH;
    SPYVOLINFO     *pVI = (SPYVOLINFO*)(pHI ? pHI->pVolInfo : NULL);
    BOOL            fSuccess = FALSE;

    if(!pHI)
    {
        SPYPRINTF(0, (__FUNCTION__": Bad handle, pFile=%P\n", pFH));
        DclProductionError();
        return FALSE;
    }

    SPYPRINTF(0, (__FUNCTION__": hDsk=%P hFile=%P pBuffer=%P BytesToWrite=%lU pdwBytesWritten=%P LowOffset=%lU HighOffset=%lU\n",
        pVI->hDsk, pFH, pBuffer, dwBytesToWrite, pdwBytesWritten, dwLowOffset, dwHighOffset));

    __try
    {
        fSuccess = pVI->FilterHook.pWriteFileWithSeek(pHI->hObj,
                                                      pBuffer,
                                                      dwBytesToWrite,
                                                      pdwBytesWritten,
                                                      pOverlapped,
                                                      dwLowOffset,
                                                      dwHighOffset);

        if(fSuccess)
        {
            SPYPRINTF(0, (__FUNCTION__": hDsk=%P wrote %lU bytes\n",
                pVI->hDsk, pdwBytesWritten ? *pdwBytesWritten : 0));
        }
        else
        {
            SPYPRINTF(0, (__FUNCTION__": hDsk=%P Failed, LastError=%lU\n", pVI->hDsk, GetLastError()));
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        SPYPRINTF(0, (__FUNCTION__": hDsk=%P Exception in call!\n", pVI->hDsk));
    }

    return fSuccess;
}


/*-------------------------------------------------------------------
    Protected:

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
BOOL DLFILESPY_WriteFileGather(
    PFILE                   pFH,
    FILE_SEGMENT_ELEMENT    aSegments[],
    DWORD                   dwBytesToWrite,
    DWORD                  *pReserved,
    OVERLAPPED             *pOverlapped)
{
    SPYHANDLEINFO          *pHI = (SPYHANDLEINFO*)pFH;
    SPYVOLINFO             *pVI = (SPYVOLINFO*)(pHI ? pHI->pVolInfo : NULL);
    BOOL                    fSuccess = FALSE;

    if(!pHI)
    {
        SPYPRINTF(0, (__FUNCTION__": Bad handle, pFile=%P\n", pFH));
        DclProductionError();
        return FALSE;
    }

    SPYPRINTF(0, (__FUNCTION__": hDsk=%P hFile=%P aSegments=%P BytesToWrite=%lU pReserved=%P\n",
        pVI->hDsk, pFH, aSegments, dwBytesToWrite, pReserved));

    __try
    {
        fSuccess = pVI->FilterHook.pWriteFileGather(pHI->hObj,
                                                    aSegments,
                                                    dwBytesToWrite,
                                                    pReserved,
                                                    pOverlapped);

        if(fSuccess)
        {
            SPYPRINTF(0, (__FUNCTION__": hDsk=%P returning %lU bytes\n",
                pVI->hDsk, dwBytesToWrite));
        }
        else
        {
            SPYPRINTF(0, (__FUNCTION__": hDsk=%P Failed, LastError=%lU\n", pVI->hDsk, GetLastError()));
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        SPYPRINTF(0, (__FUNCTION__": hDsk=%P Exception in call!\n", pVI->hDsk));
    }

    return fSuccess;
}


/*-------------------------------------------------------------------
    Protected:

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
BOOL DLFILESPY_LockFileEx(
    PFILE           pFH,
    DWORD           dwFlags,
    DWORD           dwReserved,
    DWORD           dwBytesToLockLow,
    DWORD           dwBytesToLockHigh,
    OVERLAPPED     *pOverlapped)
{
    SPYHANDLEINFO  *pHI = (SPYHANDLEINFO*)pFH;
    SPYVOLINFO     *pVI = (SPYVOLINFO*)(pHI ? pHI->pVolInfo : NULL);
    BOOL            fSuccess = FALSE;

    if(!pHI)
    {
        SPYPRINTF(0, (__FUNCTION__": Bad handle, pFile=%P\n", pFH));
        DclProductionError();
        return FALSE;
    }

    SPYPRINTF(0, (__FUNCTION__": hDsk=%P hFile=%P Flags=%lX Reserved=%lU BytesToLockLow=%lU BytesToLockHigh=%lU\n",
        pVI->hDsk, pFH, dwFlags, dwReserved, dwBytesToLockLow, dwBytesToLockHigh));

    __try
    {
        fSuccess = pVI->FilterHook.pLockFileEx(pHI->hObj,
                                               dwFlags,
                                               dwReserved,
                                               dwBytesToLockLow,
                                               dwBytesToLockHigh,
                                               pOverlapped);

        if(fSuccess)
            SPYPRINTF(0, (__FUNCTION__": hDsk=%P OK\n", pVI->hDsk));
        else
            SPYPRINTF(0, (__FUNCTION__": hDsk=%P Failed, LastError=%lU\n", pVI->hDsk, GetLastError()));
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        SPYPRINTF(0, (__FUNCTION__": hDsk=%P Exception in call!\n", pVI->hDsk));
    }

    return fSuccess;
}


/*-------------------------------------------------------------------
    Protected:

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
BOOL DLFILESPY_UnlockFileEx(
    PFILE           pFH,
    DWORD           dwReserved,
    DWORD           dwBytesToUnlockLow,
    DWORD           dwBytesToUnlockHigh,
    OVERLAPPED     *pOverlapped)
{
    SPYHANDLEINFO  *pHI = (SPYHANDLEINFO*)pFH;
    SPYVOLINFO     *pVI = (SPYVOLINFO*)(pHI ? pHI->pVolInfo : NULL);
    BOOL            fSuccess = FALSE;

    if(!pHI)
    {
        SPYPRINTF(0, (__FUNCTION__": Bad handle, pFile=%P\n", pFH));
        DclProductionError();
        return FALSE;
    }

    SPYPRINTF(0, (__FUNCTION__": hDsk=%P hFile=%P Reserved=%lU BytesToUnlockLow=%lU BytesToUnlockHigh=%lU\n",
        pVI->hDsk, pFH, dwReserved, dwBytesToUnlockLow, dwBytesToUnlockHigh));

    __try
    {
        fSuccess = pVI->FilterHook.pUnlockFileEx(pHI->hObj,
                                                 dwReserved,
                                                 dwBytesToUnlockLow,
                                                 dwBytesToUnlockHigh,
                                                 pOverlapped);

        if(fSuccess)
            SPYPRINTF(0, (__FUNCTION__": hDsk=%P OK\n", pVI->hDsk));
        else
            SPYPRINTF(0, (__FUNCTION__": hDsk=%P Failed, LastError=%lU\n", pVI->hDsk, GetLastError()));
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        SPYPRINTF(0, (__FUNCTION__": hDsk=%P Exception in call!\n", pVI->hDsk));
    }

    return fSuccess;
}


