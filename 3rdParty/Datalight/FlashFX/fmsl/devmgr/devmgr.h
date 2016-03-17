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

    This header defines information used internally within the Device
    Manager.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: devmgr.h $
    Revision 1.31  2010/09/18 02:40:48Z  garyp
    Renamed the erase states for clarity.
    Revision 1.30  2010/06/19 04:19:07Z  garyp
    Removed an obsolete field.
    Revision 1.29  2009/07/31 23:48:41Z  garyp
    Merged from the v4.0 branch.  Headers updated.  Clarified some variable
    names.  Modified the DevMgr IO/Erase Prolog/Epilog functions to return
    an FFXSTATUS value rather than a D_BOOL.
    Revision 1.28  2009/04/08 17:57:39Z  keithg
    Changed include file name bbm_public is now fxbbm (conforming)
    Revision 1.27  2009/03/11 18:13:53Z  glenns
    Fix Bugzilla #2354: Made Dev Manager's event-reporting functions public.
    Revision 1.26  2009/02/12 07:10:21Z  keithg
    Fixed bug 2394 - The device manager now maintains a private member
    to describe reserved space and does not use the obsolete FIM/NTM
    reserved space attributes.
    Revision 1.25  2009/01/18 08:43:26Z  keithg
    Removed code and compile time condition for support of the old
    BBM which is now obsolete.
    Revision 1.24  2009/01/16 05:15:57Z  glenns
    Added elements to FFXFIMDEVICE to support the Error Manager and optionally
    the Event Manager.  Added optional declaration of a call to get the Event
    Manager instance.
    Revision 1.23  2008/12/31 06:29:40Z  keithg
    Added conditional inclusion of the BBM header and corrected the
    conditional logic of the new BBM elements.
    Revision 1.22  2008/12/18 07:04:26Z  keithg
    Updated the BbmMedia_t interface to use an opaque pointer to
    pass a context between the BBM and the user of the BBM.
    Revision 1.21  2008/12/18 00:51:48Z  keithg
    Removed duplicate prototypes, added RetireRawBlock prototype.
    Revision 1.20  2008/12/17 01:59:55Z  keithg
    Made teh status functions available unconditionally.
    Revision 1.19  2008/12/12 07:26:12Z  keithg
    Added prototype for setting the block status; Added page buffer pointer
    needed by the BBM.
    Revision 1.18  2008/12/09 00:48:57Z  keithg
    Clarified BBM specific structure element names.  Made the BBM list
    a opaque type.
    Revision 1.17  2008/10/24 04:38:10Z  keithg
    Moved BBM specific prototypes to devbbm.h
    Revision 1.16  2008/10/23 22:20:13Z  keithg
    Added BBM v5 prototypes and FIM device structure elements.
    Revision 1.15  2008/05/15 23:13:01Z  keithg
    Raw block status functionality is no longer dependant upon the
    inclusion of BBM functionality.
    Revision 1.14  2008/03/26 02:40:45Z  Garyp
    Prototype updates.
    Revision 1.13  2007/11/03 23:49:35Z  Garyp
    Updated to use the standard module header.
    Revision 1.12  2007/04/08 02:37:08Z  Garyp
    Updated so that cycling of the DEV mutex can be controlled at run-time
    whereas before it was done at compile time only.
    Revision 1.11  2007/02/22 19:05:07Z  timothyj
    Moved ulPagesPerBlock into the FimInfo structure as uPagesPerBlock, where
    it can be used by other modules to which FimInfo is public, rather than
    computing or storing an additional copy of the same value.  Modified call
    to FfxDevApplyArrayBounds() to pass and receive on return a return block
    index in lieu of byte offset, for NOR compatibility with NAND LFA changes.
    Revision 1.10  2007/02/01 02:39:58Z  Garyp
    Structure and prototype updates to support timing and latency configuration
    options.
    Revision 1.9  2006/08/23 02:39:23Z  Garyp
    Removed an unused field.
    Revision 1.8  2006/05/19 18:32:46Z  Garyp
    Added support for erasing NOR boot blocks.
    Revision 1.7  2006/03/21 04:22:03Z  billr
    Structure members used for thread synchronization must be declared volatile.
    Revision 1.6  2006/03/15 00:02:40Z  Garyp
    Fixed to not allocate the aligned I/O buffer on the stack.
    Revision 1.5  2006/03/06 00:59:35Z  Garyp
    Added RawPageRead/Write() support.
    Revision 1.4  2006/02/15 21:35:33Z  Garyp
    Added support for a raw block count.
    Revision 1.3  2006/02/15 04:23:05Z  Garyp
    Added reserved space support for NOR FIMs.
    Revision 1.2  2006/02/11 23:46:19Z  Garyp
    Remove dead code.
    Revision 1.1  2006/02/08 19:35:54Z  Garyp
    Initial revision
---------------------------------------------------------------------------*/

#ifndef DEVMGR_H_INCLUDED
#define DEVMGR_H_INCLUDED

#include <fxdriverfwapi.h>

/*  Have access to Error Manager so each device has its own instance
    of error management:
*/
#include <errmanager.h>
#include <eventmgr.h>

#if FFXCONF_BBMSUPPORT
#include <fxbbm.h>
#endif

/*  The deviceList is maintained in devflash.c and defines
    the FIMs for each supported device.
*/
extern FIMDEVICE **ppFIMDevList[];


typedef struct tagFFXFIMDEVICE
{
    FFXDEVHANDLE    hDev;               /* FFX device handle                        */
    FIMHANDLE       hInst;              /* FIM device handle                        */
    FIMDEVICE      *pFimEntry;          /* FIM device function pointers             */
    const FIMINFO  *pFimInfo;           /* FIM device information                   */
    ERRMANAGERINSTANCE *errMgrInstance; /* Error Manager Instance                   */
  #if FFX_USE_EVENT_MANAGER
    EVENTMGRINSTANCE *eventMgrInstance; /* Event Manager Instance                   */
  #endif
    PDCLMUTEX       pMutex;             /* This mutex protects all flash access     */
    D_UINT32        ulRawBlocks;        /* Total blocks including BBM (not including reserved) */
    D_UINT32        ulRawTotalPages;    /* Total pages  including BBM (not including reserved) */
    D_UINT32        ulTotalBlocks;      /* Total blocks visible to higher layers (not including reserved) */
    D_UINT32        ulTotalPages;       /* Total pages (not including reserved)     */
    D_UINT32        ulReservedBlocks;   /* Number of blocks excluded from FlashFX use */
    D_UINT32        ulMaxReadCount;     /* Max pages to read in one operation       */
    D_UINT32        ulMaxWriteCount;    /* Max pages to write in one operation      */
    D_UINT32        ulMaxEraseCount;    /* Max blocks to erase in one operation     */
    D_UINT16        uDeviceNum;
  #if FFXCONF_LATENCYREDUCTIONENABLED
    D_UINT16        uErasePollInterval; /* MS to sleep while polling for erase completion */
    FFXDEVLATENCY   Latency;
    D_BOOL          fMutexCycle;        /* TRUE to cycle the DEV mutex during I/O   */
  #if FFXCONF_LATENCYAUTOTUNE
    unsigned        fReadAutoTune  : 1; /* TRUE to autotune read latency settings   */
    unsigned        fWriteAutoTune : 1; /* TRUE to autotune write latency settings  */
    unsigned        fEraseAutoTune : 1; /* TRUE to autotune erase latency settings  */
    unsigned        fReduceOnly    : 1; /* TRUE if ONLY reduction is allowed due to the artificial 64K size limit */
    D_UINT32        ulReadCount;        /* Number of reads timed so far             */
    D_UINT32        ulReadTimeMS;       /* Time spent doing that reading            */
    D_UINT32        ulWriteCount;       /* Number of writes timed so far            */
    D_UINT32        ulWriteTimeMS;      /* Time spent doing that writing            */
    D_UINT32        ulEraseCount;       /* Number of erases timed so far            */
    D_UINT32        ulEraseTimeMS;      /* Time spent doing that erasing            */
  #endif
  #endif
  #if FFXCONF_NORSUPPORT || FFXCONF_ISWFSUPPORT
    D_UINT32        ulBootBlockSize;     /* Size of each boot block                 */
    D_UINT32        ulLowBootBlockCount; /* Boot blocks at the low end (per chip)   */
    D_UINT32        ulHighBootBlockStart;/* Start boot block the high end (per chip)*/
    D_UINT32        ulBBsPerBlock;       /* Boot blocks per full block              */
  #endif
  #if FFXCONF_FORCEALIGNEDIO
    D_BUFFER       *pAlignedIOBuffer;   /* A 1 page buffer if aligned I/O is required */
  #endif
    volatile enum
    {
        ERASESTATE_IDLE, ERASESTATE_ERASING, ERASESTATE_SUSPENDED
    }                   nEraseState;
    volatile unsigned   nEraseSuspend;
  #if FFXCONF_BBMSUPPORT
    BbmDisk_t      *pBbm;               /* BBM management information used by FfxDevBbmXXX */
    D_UINT32        ulBbmChips;
    D_UINT32        ulBbmDataBlocks;
    D_UINT32        ulBbmReservedBlocks;
    D_UINT8        *rgbPageBuffer;
  #endif
}FFXFIMDEVICE;


FFXSTATUS   FfxDevLatencyInit(              FFXFIMDEVICE *pFIM);
FFXSTATUS   FfxDevIOProlog(                 FFXFIMDEVHANDLE hFimDev, D_UINT16 uType);
FFXSTATUS   FfxDevIOEpilog(                 FFXFIMDEVHANDLE hFimDev, D_UINT16 uType);
FFXSTATUS   FfxDevEraseProlog(              FFXFIMDEVHANDLE hFimDev);
FFXSTATUS   FfxDevEraseEpilog(              FFXFIMDEVHANDLE hFimDev);
FFXIOSTATUS FfxDevErasePoll(                FFXFIMDEVHANDLE hFimDev);
#if FFX_USE_EVENT_MANAGER
EVENTMGRINSTANCE *FfxDevGetEventMgrInstance(FFXFIMDEVHANDLE hFimDev);
#endif


#if FFXCONF_NORSUPPORT || FFXCONF_ISWFSUPPORT
void        FfxDevBootBlockInit(            FFXFIMDEVICE *pFIM);
FFXIOSTATUS FfxDevReadControlData(          FFXFIMDEVHANDLE hFimDev, FFXIOR_DEV_READ_CONTROLDATA *pDevReq);
FFXIOSTATUS FfxDevWriteControlData(         FFXFIMDEVHANDLE hFimDev, FFXIOR_DEV_WRITE_CONTROLDATA *pDevReq);
FFXIOSTATUS FfxDevEraseBootBlocks(          FFXFIMDEVHANDLE hFimDev, FFXIOR_DEV_ERASE_BOOT_BLOCKS *pDevReq);
FFXIOSTATUS FfxDevEraseBootBlockSequence(   FFXFIMDEVHANDLE hFimDev, FFXIOR_FIM_ERASE_BOOT_BLOCK_START *pReq);
D_BOOL      FfxDevIsInBootBlocks(           FFXFIMDEVHANDLE hFimDev, D_UINT32 ulStartBlock);
#endif

#if FFXCONF_NANDSUPPORT
#if FFXCONF_BBMSUPPORT
FFXIOSTATUS FfxDevNandGetBlockInfo(         FFXFIMDEVHANDLE hFimDev, FFXIOR_DEV_GET_BLOCK_INFO *pDevReq);
FFXIOSTATUS FfxDevNandGetRawBlockInfo(      FFXFIMDEVHANDLE hFimDev, FFXIOR_DEV_GET_RAW_BLOCK_INFO *pDevReq);
FFXIOSTATUS FfxDevNandRetireRawBlock(       FFXFIMDEVHANDLE hFimDev, FFXIOR_DEV_RETIRE_RAW_BLOCK *pDevReq);
#endif
FFXIOSTATUS FfxDevNandGetBlockStatus(       FFXFIMDEVHANDLE hFimDev, FFXIOR_DEV_GET_BLOCK_STATUS *pDevReq);
FFXIOSTATUS FfxDevNandGetRawBlockStatus(    FFXFIMDEVHANDLE hFimDev, FFXIOR_DEV_GET_RAW_BLOCK_STATUS *pDevReq);
FFXIOSTATUS FfxDevNandSetRawBlockStatus(    FFXFIMDEVHANDLE hFimDev, FFXIOR_DEV_SET_RAW_BLOCK_STATUS *pDevReq);
FFXIOSTATUS FfxDevNandGetRawPageStatus(     FFXFIMDEVHANDLE hFimDev, FFXIOR_DEV_GET_RAW_PAGE_STATUS *pDevReq);
#endif

#if FFXCONF_LATENCYREDUCTIONENABLED
D_UINT32    FfxDevCalcMaxReadCount(         FFXFIMDEVHANDLE hFimDev, D_UINT32 ulReadTimeUS);
D_UINT32    FfxDevCalcMaxWriteCount(        FFXFIMDEVHANDLE hFimDev, D_UINT32 ulWriteTimeUS);
D_UINT32    FfxDevCalcMaxEraseCount(        FFXFIMDEVHANDLE hFimDev, D_UINT32 ulEraseTimeUS);
#endif

void        FfxDevReportRead(               FFXFIMDEVHANDLE hFimDev, D_UINT32 ulStartPage, D_UINT32 ulPageCount);
void        FfxDevReportWrite(              FFXFIMDEVHANDLE hFimDev, D_UINT32 ulStartPage, D_UINT32 ulPageCount);
void        FfxDevReportErase(              FFXFIMDEVHANDLE hFimDev, D_UINT32 ulBlockNum);
void        FfxDevReportSuspend(            FFXFIMDEVHANDLE hFimDev);
void        FfxDevReportResume(             FFXFIMDEVHANDLE hFimDev);

#endif  /* DEVMGR_H_INCLUDED */

