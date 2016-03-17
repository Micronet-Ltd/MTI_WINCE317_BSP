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

#include <windows.h>
#include <netui.h>

/**
 *  This method is called exclusively by the "LineTranslateDialogExt" function below.
 *  "LineTranslateDialogExt" creates a thread and blocks on it so that TLS methods are 
 *  not called by the current thread.
 */
DWORD WINAPI LineTranslateWrapperThread(LPVOID lpParam){
	//cast the parameters
	LineTranslateParametersStruct * parameters = (LineTranslateParametersStruct*)lpParam;
	//call the method
	LineTranslateDialog(parameters->hParent, parameters->phDlg);
	//Indicate call completed to the blocked parent thread
	SetEvent(parameters->LineTranslateEvent);
	return 0;
}

EXTERN_C BOOL LineTranslateDialogExt(LPVOID lpInBuffer, 
									 DWORD nInBufferSize, 
									 LPVOID lpOutBuffer, 
									 DWORD nOutBufferSize, 
									 PDWORD pBytesReturned)
{
	LineTranslateParametersStruct ThreadParameters;
    PLineTranslateDialogArgs pLineTranslateDialogArgs = NULL;
    PLineTranslateDialogArgs pReturnArgs = NULL;

    if(nInBufferSize < sizeof(LineTranslateDialogArgs)) {
	 pReturnArgs->retCode = ERROR_INVALID_PARAMETER;	
       return FALSE;
    }

    *pBytesReturned = nOutBufferSize;
    pReturnArgs = (PLineTranslateDialogArgs) lpOutBuffer;
    pLineTranslateDialogArgs = (PLineTranslateDialogArgs) lpInBuffer;
    ThreadParameters.hParent = pLineTranslateDialogArgs->hParent;
    ThreadParameters.phDlg = &(pLineTranslateDialogArgs->hDlg);
 
    pReturnArgs->retCode = ERROR_SUCCESS;
    SetLastError(ERROR_SUCCESS);

	//create thread and event
	ThreadParameters.LineTranslateEvent = CreateEvent(
				NULL,		//default security
				FALSE,		//auto reset after waiting thread is released
				FALSE,		//initial state nonsignaled
				L"NetUI transdialog wrapper event");
	
	CreateThread(NULL,						//default security attributes
				 0,							//use default stack size
				 LineTranslateWrapperThread,//thread function
				 (LPVOID)&ThreadParameters,	//event argument for the thread				 
				 //0,					    //no thread argument
				 0,							//default creation flags
				 NULL);						//ignore thread identifier

	//wait for completion of translate dialog thread
	WaitForSingleObject(ThreadParameters.LineTranslateEvent, INFINITE);

	//close the event handle (event should be destroyed)
	CloseHandle(ThreadParameters.LineTranslateEvent);
	
    return TRUE;
}




