// All rights reserved ADENEO EMBEDDED 2010
//========================================================================
// Copyright (c) Texas Instruments Incorporated 2010. All Rights Reserved.
// Copyright (c) MPC Data Limited 2007.  All rights reserved.
//
//------------------------------------------------------------------------------
//
//  File:  desclist.hpp
//

typedef enum{
    TRANS_NO_ERROR=0,
    TRANS_ERROR,
    TRANS_NAK,
    TRANS_STALL,
}TransactionStatus;

#define MIN(a,b) ((a)<(b)?(a):(b))

void CreateDescriptors(void);
USBED* AllocateED(void);
USBTD* AllocateTD(void);
void FreeED(USBED *);
void FreeTD(USBTD *);

BOOL RemoveElementFromList(ListHead **, ListHead *);
ListHead *RemoveElementFromListEnd( ListHead **);
ListHead *RemoveElementFromListStart( ListHead **);


