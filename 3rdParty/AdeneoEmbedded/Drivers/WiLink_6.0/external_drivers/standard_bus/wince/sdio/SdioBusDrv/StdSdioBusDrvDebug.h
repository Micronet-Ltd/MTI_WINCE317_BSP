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

#ifndef __STD_SDIO_BUS_DRV_DEBUG_H
#define __STD_SDIO_BUS_DRV_DEBUG_H

#include <windows.h>


/* Extern global variable */
extern int g_tisdbus_debug_level;


/* Define for printing debug */
/* #define TISDBUS_DEBUG */

typedef enum{
  TISDBUS_DEBUGLEVEL_EMERG=1,
  TISDBUS_DEBUGLEVEL_ALERT,
  TISDBUS_DEBUGLEVEL_CRIT,
  TISDBUS_DEBUGLEVEL_ERR=4,
  TISDBUS_DEBUGLEVEL_WARNING,
  TISDBUS_DEBUGLEVEL_NOTICE,
  TISDBUS_DEBUGLEVEL_INFO,
  TISDBUS_DEBUGLEVEL_DEBUG=8
}sdt_debuglevel;

#ifdef TISDBUS_DEBUG

#define PDEBUG(fmt)if(g_tisdbus_debug_level >= TISDBUS_DEBUGLEVEL_DEBUG)  printf(fmt)
#define PDEBUG1(fmt,p1) if(g_tisdbus_debug_level >= TISDBUS_DEBUGLEVEL_DEBUG) printf(fmt,p1)
#define PDEBUG2(fmt,p1,p2) if(g_tisdbus_debug_level >= TISDBUS_DEBUGLEVEL_DEBUG) printf(fmt,p1,p2)
#define PDEBUG3(fmt,p1,p2,p3) if(g_tisdbus_debug_level >= TISDBUS_DEBUGLEVEL_DEBUG) printf(fmt,p1,p2,p3)
#define PDEBUG4(fmt,p1,p2,p3,p4) if(g_tisdbus_debug_level >= TISDBUS_DEBUGLEVEL_DEBUG) printf(fmt,p1,p2,p3,p4)
#define PDEBUG5(fmt,p1,p2,p3,p4,p5) if(g_tisdbus_debug_level >= TISDBUS_DEBUGLEVEL_DEBUG) printf(fmt,p1,p2,p3,p4,p5)


#else

#define PDEBUG(fmt)
#define PDEBUG1(fmt,p1) 
#define PDEBUG2(fmt,p1,p2) 
#define PDEBUG3(fmt,p1,p2,p3) 
#define PDEBUG4(fmt,p1,p2,p3,p4) 
#define PDEBUG5(fmt,p1,p2,p3,p4,p5) 

#endif

#define PERR(fmt)if(g_tisdbus_debug_level >= TISDBUS_DEBUGLEVEL_ERR)  printf(fmt)
#define PERR1(fmt,p1) if(g_tisdbus_debug_level >= TISDBUS_DEBUGLEVEL_ERR) printf(fmt,p1)
#define PERR2(fmt,p1,p2) if(g_tisdbus_debug_level >= TISDBUS_DEBUGLEVEL_ERR) printf(fmt,p1,p2)
#define PERR3(fmt,p1,p2,p3) if(g_tisdbus_debug_level >= TISDBUS_DEBUGLEVEL_ERR) printf(fmt,p1,p2,p3)
#define PERR4(fmt,p1,p2,p3,p4) if(g_tisdbus_debug_level >= TISDBUS_DEBUGLEVEL_ERR) printf(fmt,p1,p2,p3,p4)
#define PERR5(fmt,p1,p2,p3,p4,p5) if(g_tisdbus_debug_level >= TISDBUS_DEBUGLEVEL_ERR) printf(fmt,p1,p2,p3,p4,p5)

#endif/* __STD_SDIO_BUS_DRV_DEBUG_H */
