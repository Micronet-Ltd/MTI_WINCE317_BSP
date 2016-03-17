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

/** \file CmdMBox.h
 *  \brief CmdMbox internal defenitions
 *
 *  \see CmdMBox.c
 */

#ifndef _CMDMBOX_H_
#define _CMDMBOX_H_

#include "TwIf.h"

/*****************************************************************************
 **          Defines                                                        **
 *****************************************************************************/
 /* wait for a Mail box command to complete, ms */
#define CMDMBOX_WAIT_TIMEOUT            15000
#define CMDMBOX_HEADER_LEN              4
#define MAX_CMD_MBOX_CONSECUTIVE_TXN    5



typedef struct
{   
    TTxnStruct          tTxnStruct;
    Command_t           tCmdMbox;
}TCmdTxn;

typedef struct
{   
    TTxnStruct          tTxnStruct;
    TI_UINT32           uRegister;
}TRegTxn;


/*****************************************************************************
 **         Structures                                                      **
 *****************************************************************************/

/** \struct TCmdMbox
 * \brief CmdMbox structure
 * 
 * \par Description
 * 
 * \sa	
 */ 
typedef struct
{   
    /* handles */
    TI_HANDLE           hOs;
    TI_HANDLE           hReport;
    TI_HANDLE           hTwIf;
    TI_HANDLE           hCmdQueue;
    TI_HANDLE           hCmdMboxTimer;
    fnotify_t           fCb;
    TI_HANDLE           hCb;
    TCmdMboxErrorCb     fErrorCb;
    
    /* HW params */
    /* use a struct to read buffers from the bus - used for extra bytes reserving */

    TCmdTxn             aCmdTxn[2];
    TRegTxn             aRegTxn[2];

    /* Holds the module state */
    TI_BOOL             bCmdInProgress;
    TI_UINT32           uFwAddr;
    TI_UINT32           uWriteLen;
    TI_UINT32           uReadLen;

} TCmdMbox;

#endif


