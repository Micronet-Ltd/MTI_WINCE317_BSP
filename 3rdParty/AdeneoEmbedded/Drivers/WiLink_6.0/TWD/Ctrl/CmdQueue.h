/***************************************************************************
**+----------------------------------------------------------------------+**
**|                                ****                                  |**
**|                                ****                                  |**
**|                                ******o***                            |**
**|                          ********_///_****                           |**
**|                           ***** /_//_/ ****                          |**
**|                            ** ** (__/ ****                           |**
**|                                *********                             |**
**|                                 ****                                 |**
**|                                  ***                                 |**
**|                                                                      |**
**|     Copyright (c) 1998 - 2009 Texas Instruments Incorporated         |**
**|                        ALL RIGHTS RESERVED                           |**
**|                                                                      |**
**| Permission is hereby granted to licensees of Texas Instruments       |**
**| Incorporated (TI) products to use this computer program for the sole |**
**| purpose of implementing a licensee product based on TI products.     |**
**| No other rights to reproduce, use, or disseminate this computer      |**
**| program, whether in part or in whole, are granted.                   |**
**|                                                                      |**
**| TI makes no representation or warranties with respect to the         |**
**| performance of this computer program, and specifically disclaims     |**
**| any responsibility for any damages, special or consequential,        |**
**| connected with the use of this program.                              |**
**|                                                                      |**
**+----------------------------------------------------------------------+**
***************************************************************************/

/** \file CmdQueue.h
 *  \brief CmdQueue internal defenitions
 *
 *  \see CmdQueue.c
 */

#ifndef _CMDQUEUE_H_
#define _CMDQUEUE_H_




/*****************************************************************************
 **         Defines                                                         **
 *****************************************************************************/
#define CMDQUEUE_QUEUE_DEPTH          30
#define CMDQUEUE_HISTORY_DEPTH        5
#define CMDQUEUE_INFO_ELEM_HEADER_LEN 4


#define RC_CONVERT(rc) \
    (rc == TXN_STATUS_OK || rc == TXN_STATUS_COMPLETE || rc == TXN_STATUS_PENDING || rc == TI_OK) ? TI_OK : TI_NOK


/*****************************************************************************
 **         Enums                                                           **
 *****************************************************************************/
typedef enum 
{
    CMDQUEUE_EVENT_RUN,
    CMDQUEUE_EVENT_COMPLETE,
    CMDQUEUE_EVENT_NUM

} ECmdQueueSmEvents;


typedef enum 
{
    CMDQUEUE_STATE_IDLE,
    CMDQUEUE_STATE_WAIT_FOR_COMPLETION

} ECmdQueueSmStates;


/*****************************************************************************
 **         Structures                                                      **
 *****************************************************************************/
/*  CmdQueue Node */
typedef struct 
{
    /* Command Type Config/interrogat ... */
    Command_e               cmdType;        
    TI_UINT32               uParamsLen; 
    void*                   fCb;
    TI_HANDLE               hCb;
    /* Param for config */
    TI_UINT8                aParamsBuf[MAX_CMD_PARAMS]; 
    /* A returned value buffer */ 
    TI_UINT8*               pInterrogateBuf; 

} TCmdQueueNode;


/*  Saved CallBack Node In case of Recovery*/
typedef struct 
{
    void*                   fCb;
    TI_HANDLE               hCb;
    /* A returned value buffer */
    TI_UINT8*               pInterrogateBuf; 

} TCmdQueueRecoveryNode;


typedef void (*TCmdQueueCb) (TI_HANDLE handle, TI_UINT16 uMboxStatus);


typedef void (*TCmdQueueGenericCb)(TI_HANDLE handle, TI_UINT16 uCmdType, TI_UINT16 uCmdID, TI_UINT32 status);


/** \struct TCmdQueue
 * \brief CmdQueue structure
 * 
 * \par Description
 * 
 * \sa	
 */ 
typedef struct
{   
    /* Handles */
    TI_HANDLE               hOs;
    TI_HANDLE               hReport;
    TI_HANDLE               hCmdMBox;
    TI_HANDLE               hTwIf;

    /* SM */
    ECmdQueueSmStates       state;
    TCmdQueueGenericCb      fCmdCompleteCb;
    TI_HANDLE               hCmdCompleteCb;
    TCmdQueueCb             fFailureCb;
    TI_HANDLE               hFailureCb;

    /* Queues */
    TCmdQueueNode           aCmdQueue [CMDQUEUE_QUEUE_DEPTH]; 
    TCmdQueueRecoveryNode   aRecoveryQueue [CMDQUEUE_QUEUE_DEPTH];

    /* Indexes & counters */
    TI_UINT32               head;
    TI_UINT32               tail;
    TI_UINT32               uNumberOfCommandInQueue;
    TI_UINT32               uMaxNumberOfCommandInQueue;
    TI_UINT32               uNumberOfRecoveryNodes;  
    #ifdef TI_DBG
        TI_UINT32               uCmdSendCounter;
        TI_UINT32               uCmdCompltCounter;
    #endif

    /* Error handling */
    TI_BOOL                 bErrorFlag;
    /* Mbox status */
    TI_BOOL                 bMboxEnabled;
    /* Notify that we have already awaken the chip */
    TI_BOOL                 bAwake;

} TCmdQueue; 

#endif

