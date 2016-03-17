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
#ifndef __COMMON_TYPES_H__
#define __COMMON_TYPES_H__

/************************************/
/*      System return values.       */
/************************************/

typedef enum
{
    PARAM_NOT_SUPPORTED         = 2,
    PARAM_VALUE_NOT_VALID       = 3,
    CONFIGURATION_NOT_VALID     = 4,
    NO_SITE_SELECTED_YET        = 5,
    EXTERNAL_SET_PARAM_DENIED   = 7,
    EXTERNAL_GET_PARAM_DENIED   = 8,
    PARAM_MODULE_NUMBER_INVALID = 9,
    STATION_IS_NOT_RUNNING      = 10,
    CARD_IS_NOT_INSTALLED       = 11,

    /* QoSMngr */
    NOT_CONNECTED,
    TRAFIC_ADM_PENDING,
    NO_QOS_AP,
    ADM_CTRL_DISABLE,
    AC_ALREADY_IN_USE,
    USER_PRIORITY_NOT_ADMITTED,


	COMMAND_PENDING,

    /* Rx Data Filters */
    RX_NO_AVAILABLE_FILTERS,
    RX_FILTER_ALREADY_EXISTS,
    RX_FILTER_DOES_NOT_EXIST,
    /* Soft Gemini */
    SG_REJECT_MEAS_SG_ACTIVE,
    PARAM_ALREADY_CONFIGURED

} systemStatus_e;

#endif /* __COMMON_TYPES_H__ */

