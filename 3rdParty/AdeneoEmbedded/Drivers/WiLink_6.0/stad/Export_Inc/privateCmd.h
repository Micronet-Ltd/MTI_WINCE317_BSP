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

/****************************************************************************/
/*																			*/
/*    MODULE:   privateCmd.h													*/
/*    PURPOSE:																*/
/*																			*/
/****************************************************************************/
#ifndef _PVT_CMD_H_
#define _PVT_CMD_H_


/** \file  privateCmd.h 
 * \brief Private Command	\n
 * This file contains all the data structures and definitions which are needed for Private Command
 * \n\n
 *
 * \n\n
 */

#include "osTIType.h"

/***********/
/* defines */
/***********/

/** \def PRIVATE_CMD_SET_FLAG
 * \brief Bitmaks of bit which indicates that the Command is SET Command
 */
#define PRIVATE_CMD_SET_FLAG	0x00000001
/** \def PRIVATE_CMD_GET_FLAG
 * \brief Bitmaks of bit which indicates that the Command is GET Command
 */
#define PRIVATE_CMD_GET_FLAG	0x00000002


/*********/
/* types */
/*********/

/** \struct ti_private_cmd_t
 * \brief TI Private Command
 * 
 * \par Description
 * This Struct defines the Parameters needed for performing TI Private Command 
 * 
 * \sa
 */
typedef struct
{
	TI_UINT32 	cmd;   			/**< Number of command to execute - configMgr parameter name			*/
	TI_UINT32 	flags; 			/**< Command action type (PRIVATE_CMD_SET_FLAG | PRIVATE_CMD_GET_FLAG) 	*/
	void* 		in_buffer; 		/**< Pointer to Input Buffer											*/
	TI_UINT32	in_buffer_len; 	/**< Input buffer length 												*/
	void* 		out_buffer;		/**< Pointer to Output buffer											*/
	TI_UINT32	out_buffer_len;	/**< Output buffer length 												*/
} ti_private_cmd_t; 


/*************/
/* functions */
/*************/

#endif  /* _PVT_CMD_H_*/
        
