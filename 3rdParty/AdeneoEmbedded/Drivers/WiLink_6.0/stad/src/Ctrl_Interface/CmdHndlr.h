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

#ifndef CMD_H
#define CMD_H

#include "queue.h"
#include "DrvMainModules.h"
#include "WlanDrvIf.h"

/* The configuration commands structure */
typedef struct 
{
    TQueNodeHdr tQueNodeHdr;                            /* The header used for queueing the command */
    TI_UINT32   cmd;
    TI_UINT32   flags;
    void       *buffer1;
    TI_UINT32   buffer1_len;
    void       *buffer2;
    TI_UINT32   buffer2_len;
    TI_UINT32  *param3;
    TI_UINT32  *param4;
    void       *pSignalObject;                          /* use to save handle to complete mechanism per OS */
    void       *local_buffer;
    TI_UINT32   local_buffer_len;
    TI_UINT32   return_code;
    TI_STATUS	eCmdStatus;                             /* (PEND / COMPLETE) */
    TI_BOOL	    bWaitFlag; 	                            /* (TRUE / FALSE) */
    /*
     * TCmdRespUnion is defined for each OS:
     * For Linx and WM that defined is empty.
     * For OSE the new typedef includes all "Done" typedefs in union from EMP code (H files).
     */
    TI_UINT8	CmdRespBuffer[sizeof(TCmdRespUnion)];   
} TConfigCommand;


TI_HANDLE cmdHndlr_Create (TI_HANDLE hOs, TI_HANDLE hEvHandler);
TI_STATUS cmdHndlr_Destroy (TI_HANDLE hCmdHndlr, TI_HANDLE hEvHandler);
void      cmdHndlr_ClearQueue (TI_HANDLE hCmdHndlr);
void      cmdHndlr_Init (TStadHandlesList *pStadHandles);
TI_STATUS cmdHndlr_InsertCommand (TI_HANDLE     hCmdHndlr,
                                  TI_UINT32     cmd,
                                  TI_UINT32     flags,
                                  void         *buffer1,
                                  TI_UINT32     buffer1_len,
                                  void         *buffer2,
                                  TI_UINT32     buffer2_len,
                                  TI_UINT32    *param3,
                                  TI_UINT32    *param4);
void      cmdHndlr_HandleCommands (TI_HANDLE hCmdHndlr);
void      cmdHndlr_Complete (TI_HANDLE hCmdHndlr);
void     *cmdHndlr_GetStat (TI_HANDLE hCmdHndlr);
void      cmdHndlr_Enable  (TI_HANDLE hCmdHndlr);
void      cmdHndlr_Disable (TI_HANDLE hCmdHndlr);

#endif
