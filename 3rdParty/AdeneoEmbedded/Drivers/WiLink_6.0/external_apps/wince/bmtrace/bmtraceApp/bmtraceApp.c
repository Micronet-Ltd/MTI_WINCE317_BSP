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
**|     Copyright (c) 1998-2009 Texas Instruments Incorporated           |**
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
 
/** \file   bmtraceApp.c 
 *  \brief  The WinMobile bmtrace performance tracing commnads application 
 *
 *  \see    bmtrace.c, bmtrace_api.h 
 */

#include <windows.h>
#include <ceddk.h>
#include <stdio.h>
#include <stdlib.h>
#include <oal_io.h>
#include "bmtrace_api.h"

void bm_test_all(void);

typedef enum _e_bmAct_t
{
	e_bmNone,
	e_bmPrint,
	e_bmStart,
	e_bmStop,
    e_bmAllMeasurements,
    e_bmDumpRegs,
    e_bmSetRegs

} e_bmAct_t;

#define PRCM_BASE_PA 0x49006000
/*--------------------------------------------------------------------------------------*/

static void usage(void)
{
    printf("Usage:\n");
    printf("\tbmtraceApp [-h] [-Print] [-Start] [-stOp] [-All] \n");
    printf("\tcompiled on %s at %s \\n",__DATE__,__TIME__);

} /* usage() */

/*--------------------------------------------------------------------------------------*/

int argproc(int argc, char **argv, e_bmAct_t *bmAct)
{
    int rc = 0;

    while (--argc)
    {
        if ((*++argv)[0] != '-')
        {
            rc = -1;
            break;;
        }
        else
        {
            switch (toupper((*argv)[1])) 
            {
                case 'H':
                    usage();
                    break;

                case 'P':
					*bmAct = e_bmPrint;
                    break;

                case 'S':
					*bmAct = e_bmStart;
                    break;

                case 'O':
					*bmAct = e_bmStop;
                    break;

                case 'A':
                    *bmAct = e_bmAllMeasurements;
                    break;

                case 'D':
                    *bmAct = e_bmDumpRegs;
                    break;

                case 'T':
                    *bmAct = e_bmSetRegs;
                    break;

                default:
                    rc = -1;
                    break;
            }
        }
    }
    if (rc == -1)
    {
        printf("bmtraceApp: invalid option %s !!!\n",*argv);
        usage();
    }

    return rc;

} /* argproc()*/

/*--------------------------------------------------------------------------------------*/

int main(int argc, char ** argv)
{
    int			rc=0;
	e_bmAct_t	bmAct=e_bmNone;

    if (rc = argproc (argc, argv, &bmAct) == 0)
    {
        switch (bmAct)
        {
        case e_bmPrint:
            CL_TRACE_PRINT(NULL);
            break;
    
        case e_bmStart:
            CL_TRACE_ENABLE();
            break;
    
        case e_bmStop:
            CL_TRACE_DISABLE();
            break;

        case e_bmAllMeasurements:
            bm_test_all();
            break;

       }
    }

    return rc;

} /* main() */

/*--------------------------------------------------------------------------------------*/
