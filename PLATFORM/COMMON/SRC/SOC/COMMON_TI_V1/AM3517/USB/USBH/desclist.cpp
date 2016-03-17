// All rights reserved ADENEO EMBEDDED 2010
//========================================================================
// Copyright (c) Texas Instruments Incorporated 2010. All Rights Reserved.
// Copyright (c) MPC Data Limited 2007.  All rights reserved.
//
//------------------------------------------------------------------------------
//
//  File:  desclist.cpp
//


#pragma warning(push)
#pragma warning(disable: 4201 4510 4512 4610)
#include <windows.h>
#include <Pkfuncs.h>
#include <Winbase.h>

#include "chw.hpp"
#include "am3517.h"
#include "cpipe.hpp"
#include "desclist.hpp"
#include "drvcommon.h"
#pragma warning(pop)

extern BOOL b_HighSpeed;
#define NO_ED   32
#define NO_TD   128

static USBED EDs[NO_ED], *pFreeED = NULL;
static USBTD TDs[NO_TD], *pFreeTD = NULL;
static UINT32 EDUsage = 0;
static UINT32 TDUsage = 0;

//========================================================================
//!  \fn AddElementToListEnd ()
//!  \brief Adds a given Element into the End of the specified List
//!  \param ListHead *pElement - List Element
//!         ListHead **ppQueueHead - List Head
//!  \return None
//========================================================================
static void
AddElementToListEnd(
    ListHead *pElement,
    ListHead **ppQueueHead
    )
{
    pElement->next = NULL;
    if(*ppQueueHead){
        ListHead *pTemp = *ppQueueHead;
        while(pTemp->next)   /*go to end of list*/
            pTemp = pTemp->next;

        pTemp->next = pElement;
    }
    else
        *ppQueueHead = pElement;
}

//========================================================================
//!  \fn AddElementToListStart ()
//!  \brief Adds a given Element to the Start of the List
//!  \param ListHead *pElement - List Element
//!         ListHead **ppQueueHead - List Head
//!  \return None
//========================================================================

static void
AddElementToListStart(
    ListHead *pElement,
    ListHead **ppQueueHead
    )
{
    pElement->next = *ppQueueHead;
    *ppQueueHead = pElement;
}

//========================================================================
//!  \fn RemoveElementFromListEnd ()
//!  \brief Removes an Element from the End ofthe specified List
//!  \param ListHead **ppQueueHead - List Head
//!  \return ListHead *
//========================================================================

ListHead *
RemoveElementFromListEnd(
    ListHead **ppQueueHead
    )
{
    ListHead *pTemp;
    ListHead *pElement;
    if(*ppQueueHead){
        pTemp = *ppQueueHead;
        if(!pTemp->next){
            /*only one element in list*/
            *ppQueueHead = pTemp->next;
            return pTemp;
        }
        else{
            while(pTemp->next->next)
                pTemp = pTemp->next;

            pElement = pTemp->next;
            pTemp->next = pElement->next;
            return pElement;
        }
    }
    else
        return NULL;
}

//========================================================================
//!  \fn RemoveElementFromListStart ()
//!  \brief Removes an Element from the Start of the specified List
//!  \param ListHead **ppQueueHead - List Head
//!  \return ListHead *
//========================================================================

ListHead *
RemoveElementFromListStart(
    ListHead **ppQueueHead
    )
{
    ListHead *pElement;
    if(*ppQueueHead){
        pElement = *ppQueueHead;
        *ppQueueHead = pElement->next;
        pElement->next = NULL;
        return pElement;
    }
    else
        return NULL;
}

//========================================================================
//!  \fn RemoveElementFromList ()
//!  \brief Removes the specified Element from the specified List
//!  \param ListHead **ppQueueHead - List Head
//!         ListHead *pElement - Element to be removed
//!  \return TRUE or FALSE
//========================================================================

BOOL RemoveElementFromList(
    ListHead **ppQueueHead,
    ListHead *pElement
    )
{
    ListHead *pTemp = *ppQueueHead;
    if(pTemp == pElement){
        *ppQueueHead = pTemp->next;
        return TRUE;
    }
    else{
        while(pTemp->next){
            if(pTemp->next == pElement){
                pTemp->next = pElement->next;
                return TRUE;
            }
            else
                pTemp = pTemp->next;
        }
        return FALSE;
    }
}
//========================================================================
//!  \fn CreateDescriptors ()
//!  \brief Function used to create the EndPoint and Transfer Descriptors.
//!  \param None
//!  \return None
//========================================================================

void CreateDescriptors(void){
    UINT32 index;
    for(index = 0; index < NO_ED; index++)
        AddElementToListStart((ListHead*)&EDs[index],(ListHead**)&pFreeED);

    EDUsage = 0;

    for(index = 0; index < NO_TD; index++)
        AddElementToListStart((ListHead*)&TDs[index],(ListHead**)&pFreeTD);

    TDUsage = 0;
}

//========================================================================
//!  \fn AllocateED ()
//!  \brief Allocates an EndPoint Descriptor and returns the same to
//!         calling routine.
//!  \param None
//!  \return USBED* - Pointer to EndPoint Descriptor
//========================================================================
USBED* AllocateED(void)
{
    USBED *pED = NULL;

    if ((EDUsage < NO_ED) && (TDUsage < NO_TD))
    {
        pED = (USBED*)RemoveElementFromListStart((ListHead**)&pFreeED);
        USBTD *pTD = (USBTD*)RemoveElementFromListStart((ListHead**)&pFreeTD);
        memset((PUCHAR) pED, 0, sizeof(USBED));
        memset((PUCHAR) pTD, 0, sizeof(USBTD));

        /*allocate NULL TD*/

        pED->HeadTD = (ListHead *)pTD;
        pED->TailTD = (ListHead *)pTD;

        EDUsage ++;
        TDUsage ++;
    }

    return pED;
}

//========================================================================
//!  \fn AllocateTD ()
//!  \brief Allocates a Transfer Descriptor and returns the same to the
//!         calling routine.
//!  \param None
//!  \return USBTD* - Pointer to the Transfer Descriptor
//========================================================================

USBTD* AllocateTD(void)
{
    USBTD* pTD = NULL;

    if (TDUsage < NO_TD)
    {
        pTD = (USBTD*)RemoveElementFromListStart((ListHead**)&pFreeTD);
        memset((PUCHAR) pTD, 0, sizeof(USBTD));

        TDUsage ++;
    }

    return pTD;
}

//========================================================================
//!  \fn FreeED ()
//!  \brief Frees an EndPoint Descriptor to the Desc Free Pool
//!  \param USBED *pElement - Pointer to the EndPoint Descriptor
//!  \return None
//========================================================================
void FreeED(USBED *pElement)
{
    if (EDUsage > 0)
    {
        AddElementToListEnd((ListHead*)pElement, (ListHead**)&pFreeED);
        EDUsage --;
    }
    else
    {
        DEBUGCHK(FALSE);
    }
}

//========================================================================
//!  \fn FreeTD ()
//!  \brief Frees a Transfer Descriptor to the Desc Free Pool
//!  \param USBTD *pElement - Pointer to the Transfer Descriptor
//!  \return None
//========================================================================
void FreeTD(USBTD *pElement)
{
    if (TDUsage > 0)
    {
        AddElementToListEnd((ListHead*)pElement, (ListHead**)&pFreeTD);
        TDUsage --;
    }
    else
    {
        DEBUGCHK(FALSE);
    }
}
