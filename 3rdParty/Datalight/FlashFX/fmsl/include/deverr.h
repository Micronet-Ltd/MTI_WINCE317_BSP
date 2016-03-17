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

---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: deverr.h $
    Revision 1.5  2010/07/08 03:58:26Z  garyp
    Added the ability to change back and forth between manual and
    random injection of errors.
    Revision 1.4  2007/11/05 16:45:41Z  Garyp
    Refactored the preprocessor conditionals to allow the read/write/erase
    tests to be individually enabled or disabled.
    Revision 1.3  2007/11/03 23:49:36Z  Garyp
    Updated to use the standard module header.
    Revision 1.2  2007/03/03 01:41:57Z  timothyj
    Modified interface to use page numbers in lieu of linear byte
    offsets, for large flash array support.
    Revision 1.1  2006/02/24 04:25:08Z  Garyp
    Initial revision
    Revision 1.1  2005/11/30 22:39:30Z  Pauli
    Initial revision
---------------------------------------------------------------------------*/

typedef struct FFXERRORINJECT *ERRINJHANDLE;

#if FFXCONF_ERRORINJECTIONTESTS

    ERRINJHANDLE  FfxDevErrorInjectCreate(FFXDEVHANDLE hDev, D_UINT32 ulSeed);
    void          FfxDevErrorInjectDestroy( ERRINJHANDLE hEI);
    FFXSTATUS     FfxDevErrorInjectParamSet(ERRINJHANDLE hEI, FFXPARAM id, const void *pBuffer, D_UINT32 ulBuffLen);
    void          FfxDevErrorInjectRead(    ERRINJHANDLE hEI, const char *pszOpName, D_UINT32 ulPage, D_BUFFER *pBuff, D_UINT16 uBuffLen, FFXIOSTATUS *pIOStat);
    void          FfxDevErrorInjectWrite(   ERRINJHANDLE hEI, const char *pszOpName, D_UINT32 ulPage, FFXIOSTATUS *pIOStat);
    void          FfxDevErrorInjectErase(   ERRINJHANDLE hEI, const char *pszOpName, D_UINT32 ulBlock, FFXIOSTATUS *pIOStat);

    #define FFXERRORINJECT_CREATE(hDev, ulSeed)              FfxDevErrorInjectCreate(hDev, ulSeed);
    #define FFXERRORINJECT_DESTROY()                         FfxDevErrorInjectDestroy(hNTM->hEI);
    #define FFXERRORINJECT_READ(nam, page, pBuff, len, stat) FfxDevErrorInjectRead(   hNTM->hEI, nam, page, pBuff, len, stat);
    #define FFXERRORINJECT_WRITE(nam, page, stat)            FfxDevErrorInjectWrite(  hNTM->hEI, nam, page, stat);
    #define FFXERRORINJECT_ERASE(nam, block, stat)           FfxDevErrorInjectErase(  hNTM->hEI, nam, block, stat);

#else

    #define FFXERRORINJECT_CREATE(hDev, ulSeed)
    #define FFXERRORINJECT_DESTROY()
    #define FFXERRORINJECT_READ(nam, pag, pBuff, buffLen, stat)
    #define FFXERRORINJECT_WRITE(nam, pag, stat)
    #define FFXERRORINJECT_ERASE(nam, pag, stat)

#endif
