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
/**********************************************************************************************************************

  FILENAME:       public_host_int.h

  DESCRIPTION:    Contains the host interface fw interrupt structure in use.



***********************************************************************************************************************/
#ifndef PUBLIC_HOST_INT_H
#define PUBLIC_HOST_INT_H

#include "public_types.h"

                
/*************************************************************************

    Host Interrupt Register (WiLink -> Host)
 
**************************************************************************/
#define ACX_INTR_WATCHDOG           BIT_0   /* HW Initiated interrupt Watchdog timer expiration */
#define ACX_INTR_INIT_COMPLETE      BIT_1   /* Init sequence is done (masked interrupt, detection through polling only ) */
    #define ACX_INTR_EVENT_A            BIT_2   /* Event was entered to Event MBOX #A*/
    #define ACX_INTR_EVENT_B            BIT_3   /* Event was entered to Event MBOX #B*/
#define ACX_INTR_CMD_COMPLETE       BIT_4   /* Command processing completion*/
    #define ACX_INTR_HW_AVAILABLE       BIT_5   /* Signaling the host on HW wakeup */
    #define ACX_INTR_DATA               BIT_6   /* The MISC bit is used for aggregation of RX, TxComplete and TX rate update */
    #define ACX_INTR_TRACE_A	        BIT_7   /* Trace meassge on MBOX #A */
    #define ACX_INTR_TRACE_B            BIT_8   /* Trace meassge on MBOX #B */

#define ACX_INTR_ALL                0xFFFFFFFF


/*************************************************************************

    Interrupt Trigger Register (Host -> WiLink)              
  
**************************************************************************/

/******** Hardware to Embedded CPU Interrupts - first 32-bit register set ********/ 
   
#define INTR_TRIG_CMD       BIT_0   /* Host Command Interrupt. Setting this bit masks*/
                                    /* the interrupt that the host issues to inform*/
                                    /* the FW that it has sent a command*/
                                    /* to the Wlan hardware Command Mailbox.*/
    
#define INTR_TRIG_EVENT_ACK BIT_1   /* Host Event Acknowlegde Interrupt. The host */
                                    /* sets this bit to acknowledge that it received*/
                                    /* the unsolicited information from the event*/
                                    /* mailbox.*/
                                       
#define INTR_TRIG_TX_PROC0  BIT_2   /* OBSOLETE (automatic end-of-transaction interrupt is used instead) */
                                       
#define INTR_TRIG_RX_PROC   BIT_3   /* The host sets this bit to inform the FW that */
                                    /* it read a packet from the RX cyclic buffer */

#define INTR_TRIG_DEBUG_ACK BIT_4 
    
#define INTR_TRIG_STATE_CHANGED BIT_5
        

/******** Hardware to Embedded CPU Interrupts - second 32-bit register set ********/

#define INTR_TRIG_RX_PROC1  BIT_17  /* OBSOLETE */

#define INTR_TRIG_TX_PROC1  BIT_18  /* OBSOLETE */


/*************************************************************************

    FW status registers (Host reads from FW upon interrupt from FW)              
                                                        
**************************************************************************/
#define NUM_TX_QUEUES	    4   /* Number of Tx HW Queues (same as ACs). */
#define NUM_RX_PKT_DESC     8   /* Number of Rx packets short descriptors in the W status */

/* Get field from FwStatus_t->rxPktsDesc[i] */
#define RX_DESC_GET_MEM_BLK(desc)            ( (desc & 0x000000FF) >>  0 )  /* The first mem-block of the Rx packet */
#define RX_DESC_GET_LENGTH(desc)             ( (desc & 0x000FFF00) >>  8 )  /* The length of the packet in words */
#define RX_DESC_GET_UNALIGNED(desc)          ( (desc & 0x00100000) >> 20 )  /* If set, the payload is not 4 bytes aligned */
#define RX_DESC_GET_PACKET_CLASS_TAG(desc)   ( (desc & 0xFF000000) >> 24 )  /* Get the RX packet class tag */


/* Set field in FwStatus_t->rxPktsDesc[i] */
#define RX_DESC_SET_MEM_BLK(desc, value)     ( desc = (desc & ~0x000000FF) | (value <<  0 ) )
#define RX_DESC_SET_LENGTH(desc, value)      ( desc = (desc & ~0x000FFF00) | (value <<  8 ) )
#define RX_DESC_SET_UNALIGNED(desc, value)   ( desc = (desc & ~0x00100000) | (value << 20 ) )

/* The content of the "counters" field in FwStatus_t - see below */
typedef struct 
{
    uint8       fwRxCntr;       /* Incremented by FW upon adding pending Rx entry to aRxPktsDesc */
    uint8       drvRxCntr;      /* Incremented by FW upon RX host slave interrupt (for debug) */
    uint8       reserved;        
    uint8       txResultsCntr;  /* Incremented by FW upon adding Tx-Result to the TxResultQueue */
} FwStatCntrs_t;

/* The FW status registers structure read by the host upon interrupt from the FW */
typedef struct 
{
    uint32      intrStatus;     /* HINT register content (will be cleared upon the read) */
    uint32      counters;       /* The counters defined in FwStatCntrs_t - see above */
    uint32      rxPktsDesc[NUM_RX_PKT_DESC];   /* Array of Rx packets short descriptors (see RX_DESC_SET/GET...) */
	uint32      txReleasedBlks[NUM_TX_QUEUES]; /* Per queue released blocks count since FW-reset */
    uint32      fwLocalTime;    /* FW time in usec, used for clock synchronization with the host */
    uint32      spare[2];       /* Total structure size is 68 bytes */
} FwStatus_t;

#endif


