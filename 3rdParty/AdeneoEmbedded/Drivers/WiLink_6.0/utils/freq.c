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
/** \file  freq.c
 *  \brief frequency to channel (and vice versa) conversion implementation
 *
 *  \see   freq.h
 */


#define __FILE_ID__  FILE_ID_126
#include "osTIType.h"

#define     CHAN_FREQ_TABLE_SIZE        (sizeof(ChanFreq) / sizeof(CHAN_FREQ))

typedef struct {
    TI_UINT8       chan;
    TI_UINT32      freq;
} CHAN_FREQ;

static CHAN_FREQ ChanFreq[] = {
    {1,  2412000},
    {2,  2417000},
    {3,  2422000},
    {4,  2427000},
    {5,  2432000},
    {6,  2437000},
    {7,  2442000},
    {8,  2447000},
    {9,  2452000},
    {10, 2457000},
    {11, 2462000},
    {12, 2467000},
    {13, 2472000}, 
    {14, 2484000}, 
    {34, 5170000},
    {36, 5180000},
    {38, 5190000},
    {40, 5200000},
    {42, 5210000},
    {44, 5220000},
    {46, 5230000}, 
    {48, 5240000},
    {52, 5260000},
    {56, 5280000},
    {60, 5300000},
    {64, 5320000},
    {100,5500000},
    {104,5520000},
    {108,5540000},
    {112,5560000},
    {116,5580000},
    {120,5600000},
    {124,5620000},
    {128,5640000},
    {132,5660000},
    {136,5680000},
    {140,5700000},
    {149,5745000},
    {153,5765000},
    {157,5785000},
    {161,5805000}
  };

TI_UINT8 Freq2Chan (TI_UINT32 freq)
{
    TI_UINT32   i;

    for (i = 0; i < CHAN_FREQ_TABLE_SIZE; i++)
    {
        if (ChanFreq[ i ].freq == freq)
        {
            return ChanFreq[ i ].chan;
        }
    }

    return 0;
}


TI_UINT32 Chan2Freq (TI_UINT8 chan)
{
    TI_UINT32   i;

    for (i = 0; i < CHAN_FREQ_TABLE_SIZE; i++)
    {
        if (ChanFreq[ i ].chan == chan)
        {
            return ChanFreq[ i ].freq;
        }
    }

    return 0;
}

