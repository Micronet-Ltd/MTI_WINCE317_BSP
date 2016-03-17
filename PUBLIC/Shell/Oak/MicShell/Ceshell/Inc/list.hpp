//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this sample source code is subject to the terms of the Microsoft
// license agreement under which you licensed this sample source code. If
// you did not accept the terms of the license agreement, you are not
// authorized to use this sample source code. For the terms of the license,
// please see the license agreement between you and Microsoft or, if applicable,
// see the LICENSE.RTF on your install media or the root of your tools installation.
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES.
//
/*---------------------------------------------------------------------------*\
 *
 * (c) Copyright Microsoft Corp. 1994 All Rights Reserved
 *
 *  module: list.hpp
 *  date:
 *  author: tonykit
 *
 *  purpose: 
 *
\*---------------------------------------------------------------------------*/
#ifndef __LIST_H__
#define __LIST_H__

#pragma warning ( disable : 4710 )

/////////////////////////////////////////////////////////////////////////////

// Internal List Structures

typedef struct tagLISTOBJ {
	struct tagLISTOBJ *lpprev;
	struct tagLISTOBJ *lpnext;
} LISTOBJ, FAR* LPLISTOBJ;

typedef struct tagLIST {
    struct tagLISTOBJ *lphead;
	struct tagLISTOBJ *lptail;
	int ncount;
	CRITICAL_SECTION cs;
} LIST, FAR* LPLIST;


class CList : public LIST
{

public:
	CList() {
		InitializeCriticalSection(&cs);

		lphead = NULL;
		lptail = NULL;
		ncount = NULL;
	}
	
	~CList() { DeleteCriticalSection(&cs); }

	int GetCount(VOID)   { return ncount; }
	LPLISTOBJ Head(VOID) { return lphead; }
	LPLISTOBJ Tail(VOID) { return lptail; }
	
	VOID Lock(BOOL fLock) {

		EnterCriticalSection(&cs);	

		if (fLock) { EnterCriticalSection(&cs);
		}else{ LeaveCriticalSection(&cs); }

		LeaveCriticalSection(&cs);	
	} /* Lock() */


	LPLISTOBJ IsObjectValid(LPLISTOBJ pobject) {
		LPLISTOBJ p;

		if (pobject == NULL) return NULL;

		EnterCriticalSection(&cs);	
		p = lphead;
		
		while (p != NULL) {
			if (p == pobject) {
				LeaveCriticalSection(&cs);	
				return pobject;
			}
			p = p->lpnext;
		}
		
		LeaveCriticalSection(&cs);	
		return NULL;   
	} /* IsObjectValid() */

	
	LPLISTOBJ AddHead(LPLISTOBJ pobject) {
		if (pobject == NULL) return NULL;

		EnterCriticalSection(&cs);	

		pobject->lpnext = NULL;
		pobject->lpprev = NULL;
		
		if (ncount == 0) {
			lphead = pobject;
			lptail = pobject;
			
		}else{
			pobject->lpnext = lphead;
			lphead->lpprev = pobject;
			lphead = pobject;
		}
		
		ncount ++;
		LeaveCriticalSection(&cs);			
		return pobject;

	} /* AddHead() */

	
	LPLISTOBJ AddTail(LPLISTOBJ pobject) {
		if (pobject == NULL) return NULL;

		EnterCriticalSection(&cs);	

		pobject->lpnext = NULL;
		pobject->lpprev = NULL;
		
		if (ncount == 0) {
			lphead = pobject;
			lptail = pobject;
			
		}else{
			pobject->lpprev = lptail;
			lptail->lpnext = pobject;
			lptail = pobject;
		}
		
		ncount ++;
		LeaveCriticalSection(&cs);		
		return pobject;

	} /* AddTail() */

	
	LPLISTOBJ Disconnect(LPLISTOBJ pobject) {
		LPLISTOBJ pprev, pnext; 

		if (pobject == NULL) return NULL;

		EnterCriticalSection(&cs);	
		
		if (--ncount == 0) {
			lphead = NULL;
			lptail = NULL; 
			
		}else{
			pprev = pobject->lpprev;
			pnext = pobject->lpnext;
			
			if (pprev == NULL) {
				pnext->lpprev = NULL;
				lphead = pnext;
				
			}else if (pnext == NULL) {
				pprev->lpnext = pnext;
				lptail = pprev;
				
			}else{
				pnext->lpprev = pprev;
				pprev->lpnext = pnext;
			}
		}
		
		LeaveCriticalSection(&cs);	
		return pobject;

	} /* Disconnect() */

	
	LPLISTOBJ MoveToHead(LPLISTOBJ pobject) {
		if (Disconnect(pobject)) {
			return AddHead(pobject);
		}
		return NULL;
	} /* MoveToHead() */

	
	LPLISTOBJ MoveToTail(LPLISTOBJ pobject) {
		if (Disconnect(pobject)) {
			return AddTail(pobject);
		}
		return NULL;
	} /* MoveToTail() */
	

	LPLISTOBJ Next(LPLISTOBJ pobject) {
		if (pobject == NULL) return NULL;
		return pobject->lpnext;
	} /* Next() */

	
	LPLISTOBJ Prev(LPLISTOBJ pobject) {
		if (pobject == NULL) return NULL;
		return pobject->lpprev;
	} /* Prev() */

	
	int GetObjectIndex(LPLISTOBJ pobject) {
		LPLISTOBJ p = lphead;
		int index = 0;

		if (pobject == NULL)
			return 0;

		EnterCriticalSection(&cs);

		while ((NULL != p) && (pobject != p))
			index++;

		LeaveCriticalSection(&cs);

		return index;
	} /* GetObjectIndex() */

	
	LPLISTOBJ GetAtIndex(int iIndex) {
		LPLISTOBJ p = lphead;
		int index = 0;

		if (iIndex >= ncount)
			return NULL;

		EnterCriticalSection(&cs);

		while ((NULL != p) && (index < iIndex))
		{
			p = p->lpnext;
			index++;
		}

		LeaveCriticalSection(&cs);

		return p;
	} /* GetAtIndex() */

	
	LPLISTOBJ DisconnectAtIndex(int iIndex) {
		LPLISTOBJ p;

		EnterCriticalSection(&cs);	
		if (p = GetAtIndex(iIndex)) {
			p = Disconnect(p);
		}
		LeaveCriticalSection(&cs);	
		return p;
		
	} /* DisconnectAtIndex() */

	
	LPLISTOBJ InsertBeforeIndex(LPLISTOBJ pobject, int iIndex) {
		LPLISTOBJ pnext = GetAtIndex(iIndex);

		if (!pobject || (iIndex < 0) ||	(iIndex > ncount)) return NULL;

		if ((iIndex == 0) || (pnext == NULL)) {
			return AddHead(pobject);
			
		}else if (iIndex == ncount) {
			return AddTail(pobject);
			
		}else{
			LPLISTOBJ pprev;
			
			EnterCriticalSection(&cs);	

			pobject->lpnext = NULL;
			pobject->lpprev = NULL;
			
			pprev           = pnext->lpprev;
			pobject->lpnext = pnext;
			pobject->lpprev = pnext->lpprev;
			
			pnext->lpprev = pobject;
			if (pprev != NULL) {
				pprev->lpnext = pobject;
			}
			
			ncount ++;
			LeaveCriticalSection(&cs);	
		}
		return pobject;
		
	} /* InsertBeforeIndex() */

	
	LPLISTOBJ InsertAfterIndex(LPLISTOBJ pobject, LONG iIndex) {
		LPLISTOBJ pprev = GetAtIndex(iIndex);

		if (!pobject || (iIndex < -1) || (iIndex >= ncount)) return NULL;

		if ((iIndex == -1) || (pprev == NULL)) {
			return AddHead(pobject);
			
		}else if (iIndex == (ncount-1)) {
			return AddTail(pobject);
			
		}else{
			LPLISTOBJ pnext;

			EnterCriticalSection(&cs);	
			
			pobject->lpnext = NULL;
			pobject->lpprev = NULL;
			
			pnext           = pprev->lpnext;
			pobject->lpprev = pprev;
			pobject->lpnext = pprev->lpnext;
			
			pprev->lpnext = pobject;
			if (pnext != NULL) {
				pnext->lpprev = pobject;
			}
			
			ncount ++;
			LeaveCriticalSection(&cs);	
		}
		return pobject;
		
	} /* InsertAfterIndex() */

	
	LPLISTOBJ InsertAfterObject(LPLISTOBJ pobject, LPLISTOBJ pobjectAfter) {

		if (pobject == NULL) return NULL;

		if (pobjectAfter == NULL) {
			return AddHead(pobject);

		}else if (pobjectAfter->lpnext == NULL) {
			return AddTail(pobject);
			
		}else{
			LPLISTOBJ pobjectBefore;
			
			EnterCriticalSection(&cs);	
			
			pobject->lpnext = NULL;
			pobject->lpprev = NULL;
			
			pobjectBefore   = pobjectAfter->lpnext;
			pobject->lpprev = pobjectAfter;
			pobject->lpnext = pobjectAfter->lpnext;
			
			pobjectAfter->lpnext = pobject;
			if (pobjectBefore != NULL) {
				pobjectBefore->lpprev = pobject;
			}
			
			ncount++;
			LeaveCriticalSection(&cs);	
		}
		return pobject;
	
	} /* InsertAfterObject() */

	
	BOOL Swap(LPLISTOBJ pobject1, LPLISTOBJ pobject2) {
		int cbIndex1, cbIndex2;

		if (!pobject1 || !pobject2) return FALSE;

		EnterCriticalSection(&cs);	
		
		cbIndex1 = GetObjectIndex(pobject1);
		cbIndex2 = GetObjectIndex(pobject2);
		
		Disconnect(pobject1);
		Disconnect(pobject2);

		if (cbIndex1 < cbIndex2) {
			InsertAfterIndex(pobject2, cbIndex1 - 1);
			InsertAfterIndex(pobject1, cbIndex2 - 1);
		}else{
			InsertAfterIndex(pobject1, cbIndex2 - 1);
			InsertAfterIndex(pobject2, cbIndex1 - 1);
		}
		LeaveCriticalSection(&cs);	

		return TRUE;
		
	} /* Swap() */
		
}; /* CList */

/////////////////////////////////////////////////////////////////////////////

#endif /* __LIST_H__ */
