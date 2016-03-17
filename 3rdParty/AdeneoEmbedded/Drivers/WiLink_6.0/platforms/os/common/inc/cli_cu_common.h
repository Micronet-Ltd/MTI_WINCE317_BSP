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
#ifndef common_h
#define common_h

#include "tidef.h"

#ifdef _WINDOWS
#define DRV_NAME "\\\\.\\tiwlnpci"
#endif

#ifndef TIWLAN_DRV_NAME
#define TIWLAN_DRV_NAME  DRV_NAME
#endif

#define IN     /* input parameter          */
#define OUT    /* output parameter         */
#define INOUT  /* input & output parameter */

#ifndef TI_TRUE
#define TI_TRUE  1
#endif

#ifndef TI_FALSE
#define TI_FALSE 0
#endif

#ifndef TI_PENDING
#define TI_PENDING 2
#endif


#ifndef NULL
# define NULL 0L
#endif

#ifndef TI_VOIDCAST
#define TI_VOIDCAST(p) ((void)p)
#endif

#ifdef __LINUX__
#ifndef print
# define print(fmt, arg...) fprintf(stdout, fmt, ##arg)
#endif

#ifndef print_err
# define print_err(fmt, arg...) fprintf(stderr, fmt, ##arg)
#endif

#ifndef print_deb
# ifdef DEBUG_MESSAGES
#  define print_deb(fmt, arg...) fprintf(stdout, fmt, ##arg)
# else
#  define print_deb(fmt, arg...)
# endif	/* DEBUG_MESSAGES */
#endif	/* print_deb */

#endif /* __LINUX__ */

#ifndef SIZE_ARR
# define SIZE_ARR(a) (sizeof(a)/sizeof(a[0]) )
#endif

#ifndef min
# define min(a, b)	(((a)<(b)) ? (a) : (b))
#endif

#ifndef max
# define max(a, b)	(((a)>(b)) ? (a) : (b))
#endif

typedef struct
{
    TI_UINT32 value;
    char *name;
} named_value_t;

#define print_available_values(arr) \
        {   int i; for(i=0; i<SIZE_ARR(arr); i++) \
            print("%d - %s%s", arr[i].value, arr[i].name, (i>=SIZE_ARR(arr)-1) ? "\n" : ", " ); \
        }
        
void print_memory_dump(char *addr, int size );

#endif	/* common_h */

