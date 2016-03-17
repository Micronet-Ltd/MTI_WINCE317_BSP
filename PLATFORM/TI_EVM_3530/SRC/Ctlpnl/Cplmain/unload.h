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

// Unload Defines
enum tagErrors {
	ERR_ILLEGAL_APP_NAME = 1000,	// illegal App name
	ERR_REGISTRY_APP_NAME,			// can't find AppName in regsitry
	ERR_REGISTRY_INSTALLED_DIR,		// can't open find Install dir in registry
   	ERR_UNLOAD_OPEN,				// can't open unload script file
	ERR_UNLOAD_TEMP_OPEN,			// can't open unload temporary script file
	ERR_WRITE_TEMP_UNLOAD,			// error write to temporary unload script file		
	ERR_UNLOAD_COMMAND, 			// illegal unload script file comand		
	ERR_UNLOAD_NO_COMMAND,          // no command found in unload script file 
	ERR_FILE_IN_USE,				// can't delete,  file in use
	ERR_REG_COMMAND,				// Illegal registry command
	//ERR_REGISTRY_SHARED_FILES,		// can't find 'SharedFiles' entry in regsitry
	//ERR_REGISTRY_SET_VALUE,			// can't set entry in regsitry
	ERR_FILE_NOT_FOUND,				// One or more files could not be deleted
	ERR_OUT_OF_MEMORY,				// out of memory
	ERR_INTERNAL,					// internal errors
};

// Unload script file commands
enum tagCommands {
	REMOVE_FILE = 0,             // remove file
	REMOVE_FILE_SHARED,          // remove file, w/ ref counting
	REMOVE_DIR,                  // remove dir              
	REMOVE_REG_VALUE,            // remove registry value
	REMOVE_REG_KEY,              // remove registry key
	EXECUTE_FILE,  		         // Execute a exe file
    EXECUTE_DLL_FUNCTION,        // Execute a function in a dll.
    REMOVE_REG_VALUE_EX,         // remove registry value. Allows key names with spaces.
	REMOVE_LAST_CMD,             // last remove cmd                
};

typedef void (*DLLFUNC)(void);

// This is the message sent to the CPL when unload is done: 
// msg=WM_COMMAND wp=IDC_REMOVE_FLAG lp=errcode(above)
#define IDC_REMOVE_FLAG                 5555

