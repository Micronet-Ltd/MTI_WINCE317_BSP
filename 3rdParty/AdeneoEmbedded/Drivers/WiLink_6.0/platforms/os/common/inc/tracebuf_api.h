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
/*
 * Benchmark tracing utility
 */

#ifndef TB_TRACE_API_H

#define TB_TRACE_API_H

#ifdef TI_TRACE_BUF

#define TB_NUM_ENTRIES 256
#define MAX_TB_MSG 128

#define TB_OPTION_NONE      0x00000000
#define TB_OPTION_AUTOPRINT 0x00000001
#define TB_OPTION_PRINTONCE 0x00000002
#define TB_OPTION_STOP      0x80000000

/* Initialization */
int  tb_init(unsigned long options);
int  tb_sprintf(const char *format ,...);
int  tb_trace(int loc, unsigned long p1, unsigned long p2);
void tb_destroy(void);
void tb_scan(void);
void tb_dump(void);
void tb_printf(void);
void tb_set_option(unsigned long option);
void tb_reset_option(unsigned long option);

#define tb_min(x,y) (((x)<(y)) ? (x) : (y)) 

#else  /* #ifdef TI_TRACE_BUF */

#define tb_init(options)
#define tb_sprintf(format ,...)
#define tb_trace(loc, p1, p2)
#define tb_destroy()
#define tb_scan()
#define tb_dump()
#define tb_printf()
#define tb_set_option(option)
#define tb_reset_option(option)

#endif /* #ifdef TI_TRACE_BUF */

#endif
