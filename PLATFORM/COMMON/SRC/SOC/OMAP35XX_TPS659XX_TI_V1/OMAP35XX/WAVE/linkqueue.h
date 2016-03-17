//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this source code is subject to the terms of the Microsoft end-user
// license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
// If you did not accept the terms of the EULA, you are not authorized to use
// this source code. For a copy of the EULA, please see the LICENSE.RTF on your
// install media.
//
// Portions Copyright (c) Texas Instruments.  All rights reserved.
//
//------------------------------------------------------------------------------
//

#ifndef __LINKQUEUE_H__
#define __LINKQUEUE_H__

// defines a singly linked list
//

typedef struct _tagQUEUE_ENTRY {
    struct _tagQUEUE_ENTRY FAR * Blink;
} QUEUE_ENTRY, *PQUEUE_ENTRY;

#define InsertHead(_QueueHead, _Entry) do {\
      (_Entry)->Blink = _QueueHead; \
      _QueueHead = _Entry; \
    } while (0);

#define InsertTail(_QueueHead, _Entry) do {\
    PQUEUE_ENTRY _EX_Queue = _QueueHead; \
    _Entry->Blink = NULL;\
    if (_EX_Queue) { \
        while (_EX_Queue->Blink) {_EX_Queue = _EX_Queue->Blink;} \
        _EX_Queue->Blink = _Entry; \
    } \
    else \
        _QueueHead = _Entry; \
    } while (0);


#endif //__LINKQUEUE_H__

