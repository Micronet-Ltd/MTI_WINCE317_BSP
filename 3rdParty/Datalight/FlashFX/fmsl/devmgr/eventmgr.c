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

    This source file provides the function definitions and support functions
    needed for the FlashFX Event Manager.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: eventmgr.c $
    Revision 1.9  2011/10/20 16:59:50Z  johnb
    Corrected invalid comparison on exit condition
    Revision 1.8  2009/12/31 17:24:42Z  billr
    Declare local functions static.
    Revision 1.7  2009/04/01 20:15:10Z  davidh
    Function headers updated for AutoDoc.
    Revision 1.6  2009/01/28 05:03:30Z  glenns
    - Reorganized header file inclusion to relieve some obfuscation
      difficulty.
    Revision 1.5  2009/01/23 22:58:13Z  glenns
    - Removed extra revision history block.
    - Updated function names to Datalight coding standards.
    - Added function header commentary to accomodate auto-document
      software.
    - Re-engineered to eliminate dynamic memory allocation outside of
      create time.
    - Added semaphore to protect event manager RegisterHandler
      procedure.
    Revision 1.4  2009/01/20 21:52:50Z  glenns
    - Added revision history block.
    Revision 1.3  2009/01/19 04:18:30Z  keithg
    Added explicit void to FfxEventMgrCreate function declaration.
 ---------------------------------------------------------------------------*/

#include <flashfx.h>
#include <dcl.h>
#include "eventmgr.h"

#if FFX_USE_EVENT_MANAGER

#define MAX_HANDLERS_PER_EVENT_TYPE 8

typedef pfvEventHandler *HANDLER_ARRAY;

struct tagEventMgrInstance
{
    HANDLER_ARRAY aHandlerArray;
    PDCLMUTEX     pMutex;
};

/*-------------------------------------------------------------------
    Private: AddToHandlerList()

    Adds an event handler to the handler array.

    Parameters:

    Return Value:
-------------------------------------------------------------------*/

static D_BOOL AddToHandlerList(
    pfvEventHandler *aHandlerArray,
    pfvEventHandler pfvHandler)
{
D_UINT32 index;

    for (index=0; index<MAX_HANDLERS_PER_EVENT_TYPE; index++)
    {
        if (aHandlerArray[index] == NULL)
        {
            aHandlerArray[index] = pfvHandler;
            break;
        }
    }
    if (index < MAX_HANDLERS_PER_EVENT_TYPE)
        return TRUE;
    return FALSE;
}

/*-------------------------------------------------------------------
    Private: RemoveFromHandlerList()

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
static D_BOOL RemoveFromHandlerList(
    pfvEventHandler *aHandlerArray,
    pfvEventHandler pfvHandler)
{
D_UINT32 index;

    for (index=0; index<MAX_HANDLERS_PER_EVENT_TYPE; index++)
    {
        if (aHandlerArray[index] == pfvHandler)
        {
            aHandlerArray[index] = NULL;
            break;
        }
    }
    if (index < MAX_HANDLERS_PER_EVENT_TYPE)
        return TRUE;
    return FALSE;
}

/*-------------------------------------------------------------------
    Public: FfxEventMgrRegisterEventHandler()

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
D_BOOL FfxEventMgrRegisterEventHandler(
    EVENTMGRINSTANCE *instance,
    FFX_EVENTTYPE eventtype,
    pfvEventHandler pfvHandler)
{
D_BOOL fResult;

    DclMutexAcquire(instance->pMutex);
    DclAssert(instance);
    DclAssert(pfvHandler);
    fResult = AddToHandlerList(&(instance->aHandlerArray[eventtype*FFX_EVENT_NUMEVENTS]), pfvHandler);
    if (!fResult)
    {
        FFXPRINTF(2, ("Warning: event handler could not be registered\n"));
    }
    DclMutexRelease(instance->pMutex);
    return fResult;
}

/*-------------------------------------------------------------------
    Public: FfxEventMgrUnregisterEventHandler()

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
D_BOOL FfxEventMgrUnregisterEventHandler(
    EVENTMGRINSTANCE *instance,
    FFX_EVENTTYPE eventtype,
    pfvEventHandler pfvHandler)
{
    DclAssert(instance);
    DclAssert(pfvHandler);
    return RemoveFromHandlerList(&(instance->aHandlerArray[eventtype*FFX_EVENT_NUMEVENTS]), pfvHandler);
}


/*-------------------------------------------------------------------
    Public: FfxEventMgrPostEvent()

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
void FfxEventMgrPostEvent(EVENTMGRINSTANCE *instance, FFX_EVENT *event)
{
    D_UINT32 index;

    /*  Note that this procedure is not protected by the mutex. This is to
        prevent the case where one of the handlers causes another event to
        be posted through some secondary effect, which would then cause a
        deadlock.

        In multi-threaded systems this could lead to some strange effects
        should an event get posted from a thread while event processing is
        ongoing in another one. It is up to the clients of the Event
        Manager to see to it that such effects are avoided.
    */
    DclAssert(instance);
    DclAssert(event);
    for (index=0; index<MAX_HANDLERS_PER_EVENT_TYPE; index++)
    {
        if (instance->aHandlerArray[event->event*FFX_EVENT_NUMEVENTS + index])
            (*instance->aHandlerArray[event->event*FFX_EVENT_NUMEVENTS + index])(event);
    }
}

/*-------------------------------------------------------------------
    Public: FfxEventMgrCreate()

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
EVENTMGRINSTANCE *FfxEventMgrCreate(void)
{
EVENTMGRINSTANCE *eventMgr;
D_UINT32 ulHandlerArraySize;
char achMutexName[DCL_MUTEXNAMELEN];

    /*  Get instance:
    */
    eventMgr = DclMemAlloc(sizeof(EVENTMGRINSTANCE));
    if (!eventMgr)
        return NULL;

    /*  Get handler array storage:
    */
    ulHandlerArraySize = sizeof(pfvEventHandler)*FFX_EVENT_NUMEVENTS*MAX_HANDLERS_PER_EVENT_TYPE;
    eventMgr->aHandlerArray = DclMemAllocZero(ulHandlerArraySize);
    if (!eventMgr->aHandlerArray)
    {
        DclMemFree(eventMgr);
        return NULL;
    }

    /*  Get mutex:
    */
    DclAssert(DCL_MUTEXNAMELEN >= 8);
    DclSNPrintf(achMutexName, sizeof(achMutexName), "EVNTMGR");
    eventMgr->pMutex = DclMutexCreate(achMutexName);
    if(!eventMgr->pMutex)
    {
        FFXPRINTF(2,("Warning: cannot acquire mutex\n"));
        DclMemFree(eventMgr->aHandlerArray);
        DclMemFree(eventMgr);
        return NULL;
    }
    return eventMgr;
}

/*-------------------------------------------------------------------
    Public: FfxEventMgrDestroy()

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
void FfxEventMgrDestroy(EVENTMGRINSTANCE *instance)
{

    if (instance)
    {
        if (instance->pMutex)
            DclMutexDestroy(instance->pMutex);
        if (instance->aHandlerArray)
            DclMemFree(instance->aHandlerArray);
        DclMemFree(instance);
    }
}

#endif /* #if FFX_USE_EVENT_MANAGER */

