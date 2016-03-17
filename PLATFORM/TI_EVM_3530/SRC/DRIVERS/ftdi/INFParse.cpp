#include "MultiPlatformFile.h"
#include <string.h>
#include <stdio.h>
#include "serial.h"

#define NUMBER_SECTIONS			3
#define MAX_SECTION_STRING_SIZE	50
#define MAX_STRING_LENGTH		4096	// way over the top

//
// States for the parsing
//
enum 
{
	FIND_EQUALS,
	FIND_COMMA, 
	FIND_AMPERSAND, 
	GET_VID, 
	GET_PID, 
	GOT_ALL
};

const char acSections[NUMBER_SECTIONS][MAX_SECTION_STRING_SIZE] = {
    "[Manufacturer]",
    ".NT.HW.AddReg]",
    "[Strings]"
};

static char gcPortName[1024];

/*
	Internal helper function
*/
BOOL FindVIDPIDInLine(MultiPlatformFile * mpf, DWORD * pVID, DWORD * pPID)
{
	char cBuffer[MAX_STRING_LENGTH];
	char *pcBuffer;
	DWORD dwVID = -1;
	DWORD dwPID = -1;
	int state;

	//
	// We have been through at least once - now look for the next VID and PID
	//
	pcBuffer = cBuffer;	// reset the buffer
	state = FIND_EQUALS;

	while(	(mpf->Read(pcBuffer++, 1, 1) == 1) && 
			(state != GOT_ALL)) 
	{
		if( (*(pcBuffer - 1) == '\n') && (state != GET_PID) ) {
			//
			// Our terminating condition on an invalid line
			//
			return FALSE;
		}

		//
		// Crawl along the text to find what you are looking for
		//
		switch(state) {
		case FIND_EQUALS:
			if(*(pcBuffer - 1) == '=') {
				pcBuffer = cBuffer;	// reset the buffer
				state = FIND_COMMA;	// move on state
			}
			break;

		case FIND_COMMA:
			if(*(pcBuffer - 1) == ',') {
				*(pcBuffer - 1) = (char)NULL;
				strcpy(gcPortName, cBuffer);
				pcBuffer = cBuffer;		// reset the buffer
				state = FIND_AMPERSAND; // move on state
			}
			break;

		case FIND_AMPERSAND:
			if(*(pcBuffer - 1) == '&') {
				pcBuffer = cBuffer;	// reset the buffer
				state = GET_VID;	// move on state
			}
			break;

		case GET_VID:
			if(*(pcBuffer - 1) == '&') {
				if(sscanf(cBuffer, "VID_%lX", &dwVID) != 1) {
					dwVID = -1;
				}
				pcBuffer = cBuffer;	// reset the buffer
				state = GET_PID;	// move on state
			}
			break;

		case GET_PID:
			if(*(pcBuffer - 1) == '\n') {
				if(sscanf(cBuffer, "PID_%lX", &dwPID) != 1) {
					dwPID = -1;
				}
				pcBuffer = cBuffer;	// reset the buffer
				state = GOT_ALL;	// move on state
			}
			break;
		default:
			//
			// should NOT get here
			//
			break;
		};
	}

	if((dwVID != -1) && (dwPID != -1)) {
		*pVID = dwVID;
		*pPID = dwPID;
		return TRUE;
	}
	return FALSE;
}


BOOL FindSection(MultiPlatformFile * mpf, const char * cSectionName)
{
	BOOL bFound = FALSE;
	char cBuffer[MAX_STRING_LENGTH];
	char *pcBuffer;

	pcBuffer = cBuffer;

	while(!bFound && (mpf->Read(pcBuffer++, 1, 1) == 1)) {
		if(*(pcBuffer-1) == '\n') {
			*(pcBuffer-2) = (char)NULL;
			if(strcmp(cSectionName, cBuffer) == 0) {
				bFound = TRUE;
			}
			else {
				pcBuffer = cBuffer;
			}
		}
	}
	return bFound;
}



void ParseManufacturer(MultiPlatformFile * mpf, DWORD * pVID, DWORD * pPID)
{
	BOOL bDone = FALSE;
	BOOL bFound = FALSE;
	char cBuffer[MAX_STRING_LENGTH];
	char *pcBuffer;
	char cModel[1024];
	DWORD dwVID = -1, dwPID = -1;

	//
	// Get the name of the Manufacturer (from the %FTDI%=FtdiHw)
	// This is the section where we get the VIDs and PIDs from
	//
	pcBuffer = cBuffer;

	while((mpf->Read(pcBuffer++, 1, 1) == 1) && !bDone) {
		if(*(pcBuffer - 1) == '=') {
			pcBuffer = cBuffer;
		}
		if(*(pcBuffer - 1) == '\n') {
			*(pcBuffer - 2) = (char)NULL;
			strcpy(cModel, cBuffer);
			bDone = TRUE;
		}
	}
	
	if(!bDone)
		return;

/*
	// THIS CODE NOT RELIABLE ON WINCE 6.0!!!
	// Replace to use FindSection - already exists, so use it.
	pcBuffer = cBuffer;

	// CRASH IS IN THIS WHILE LOOP
	while(!bFound && (mpf->Read(pcBuffer++, 1, 1) == 1)) {
		if(*(pcBuffer-1) == '\n') {
			*(pcBuffer-2) = (char)NULL;
			pcBuffer = cBuffer;
			pcBuffer++;
			if (strncmp(cModel, pcBuffer, strlen(cModel)) == 0) {
				bFound = TRUE;
			}
			else {
				pcBuffer = cBuffer;
			}
		}
	}

	*/

	char manbuf[1024];
	manbuf[0] = NULL;

	strcat(manbuf,"[");
	strcat(manbuf,cModel);
	strcat(manbuf,"]\0");

	FindSection(mpf,manbuf);

	bDone = FALSE;

	FindVIDPIDInLine(mpf, pVID, pPID);

	return;
}

void ParseRegSettings(MultiPlatformFile * mpf, TCHAR * pgszDriverPrefix, TCHAR * pszStreamDriverKey)
{
	BOOL bDone = FALSE;
	BOOL bReadingBinaryData = FALSE;
	int Stage = 0;	/* in the parsing of the line */
	int iBinaryPos = 0;
	unsigned char ucBinaryBuffer[1024];		// +++ is this big enough?
	char cBuffer[MAX_STRING_LENGTH];
	char *pcBuffer;
	char cRegKeyValueName[1024];
	char cRegKeyStringValue[1024];
	DWORD dwKeyType = 0;
	DWORD dwKeyDWORDValue = 0;

	pcBuffer = cBuffer;

	while((mpf->Read(pcBuffer++, 1, 1) == 1) && !bDone) {	// read 1 byte at a time
		if(cBuffer[0] == '\n') {	/* we have reached the end of the section */
			bDone = TRUE;
			continue;
		}
		if(Stage == 4) {		// where we extract the value from the file
			if((*(pcBuffer - 1) == '\n')) {
				switch(dwKeyType) {
				case 0x00010000:
					// remove all the quotes
					*(pcBuffer-3) = (char)NULL;
					pcBuffer = cBuffer;
					pcBuffer++;
					// now copy it to another buffer
					strcpy(cRegKeyStringValue, pcBuffer);

					break;
				case 0x00010001:
					if(sscanf(cBuffer, "%ld", &dwKeyDWORDValue) != 1) {
						Stage = 0;
						pcBuffer = cBuffer;
					}

					break;				
				case 0x00000001:	// config data - ignore it
					{
						unsigned char ucVal;
						int iPos = 0;
						int iRet;
						char * pcBuf;

						pcBuf = cBuffer;
						// read all of the data into a buffer - comma delimited
						if((iRet = sscanf(pcBuf, "%X", &ucVal))) {
							if(iPos < 1024) {
								ucBinaryBuffer[iPos++] = ucVal;
							}
						}
						if(iRet == 0)
							iRet = 1;
						else
							iRet = 2;
						pcBuf+= iRet;

						while(*pcBuf != 0x0D) {	// check for the LF
							if((iRet = sscanf(pcBuf, "%X", &ucVal))) {
								// only here if it is not a comma!
								if(iPos < 1024) {
									ucBinaryBuffer[iPos++] = ucVal;
								}
							}
							if(iRet == 0)
								iRet = 1;
							else
								iRet = 2;

							pcBuf+= iRet;
						}
						iBinaryPos = iPos;
					}
					break;
				};
				Stage = 5;
				pcBuffer = cBuffer;
			}
		}

		if(*(pcBuffer - 1) == ',') {
			switch(Stage) {
			case 0:	// reg root string
				if(strncmp(cBuffer, "HKR", 3) == 0) {
					Stage = 1;
					pcBuffer = cBuffer;
				}
				else {
					Stage = 0;
					pcBuffer = cBuffer;
				}
				break;
			case 1:	// subkey
				if(strncmp(cBuffer, ",", 1) == 0) {
					Stage = 2;
					pcBuffer = cBuffer;
				}
				else {
					Stage = 0;
					pcBuffer = cBuffer;
				}
				break;
			case 2:	// value name
				// remove all the quotes
				*(pcBuffer-2) = (char)NULL;
				pcBuffer = cBuffer;
				pcBuffer++;
				// now copy it to another buffer
				strcpy(cRegKeyValueName, pcBuffer);
				Stage = 3;
				pcBuffer = cBuffer;
				memset(cBuffer, 0x00, MAX_STRING_LENGTH);
				break;
			case 3:	// flag
				*(pcBuffer-1) = (char)NULL;
				if(sscanf(cBuffer, "%lX", &dwKeyType) != 1) {
					Stage = 0;
					pcBuffer = cBuffer;
				}
				else {
					Stage = 4;
					pcBuffer = cBuffer;
				}
				break;
			};
		}

		if(Stage == 5) {
		    HKEY hKey = NULL;
	        DWORD dwDisp;
		    DWORD dwStatus;
			TCHAR tcTemp[1024];
			TCHAR tcTemp1[1024];

			// save the settings to the registry
			Stage = 0;
			pcBuffer = cBuffer;
			dwStatus = RegCreateKeyEx(
					 HKEY_LOCAL_MACHINE,
					 pszStreamDriverKey,
					 0,
					 NULL,
					 0,
					 0,
					 NULL,
					 &hKey,
					 &dwDisp);

			if (dwStatus == ERROR_SUCCESS) {
				if(dwKeyType == 0x00010000) {
					// its a string
					StringToWchar(tcTemp, cRegKeyValueName, 1024);
					StringToWchar(tcTemp1, cRegKeyStringValue, 1024);
					if(wcsncmp(L"Prefix", tcTemp, 6) == 0) {
						if(lstrlen(tcTemp1) == 3) {
							dwStatus = RegSetValueEx(hKey,tcTemp,0,REG_SZ,(const BYTE *)tcTemp1,(lstrlen(tcTemp1)+1)*sizeof(WCHAR));
							wcsncpy(pgszDriverPrefix, tcTemp1, 3);
						}
					}
					else {
						dwStatus = RegSetValueEx(hKey,tcTemp,0,REG_SZ,(const BYTE *)tcTemp1,(lstrlen(tcTemp1)+1)*sizeof(WCHAR));
					}
				}
				if(dwKeyType == 0x00010001) {
					// its a DWORD
					StringToWchar(tcTemp, cRegKeyValueName, 1024);
					dwStatus = RegSetValueEx(
							 hKey,
							 tcTemp,
							 0,
							 REG_DWORD,
							 (const BYTE *) &dwKeyDWORDValue,
							 sizeof(DWORD)
							 ); 
				}
				if(dwKeyType == 0x00000001) {
					// Free form binary
					// needs to be some sort of array of chars to go in properly.

					//
					// Came accross a problem with the AMD hardware that was sent in (PIC)
					// when we arrived here (even if we were first in theINF) this function would 
					// immediatly return after calling the RegSetValueEx for REG_BINARY! This is
					// pretty wierd. The solution was to leave functionality in and take the ConfigData 
					// out of the INF file.
					//
					StringToWchar(tcTemp, cRegKeyValueName, 1024);
					dwStatus = RegSetValueEx(
							 hKey,
							 tcTemp,
							 0,
							 REG_BINARY,
							 (const BYTE *) &ucBinaryBuffer, //pointer to the beginning of array
							 (DWORD)(iBinaryPos)// size of the array
							 ); 		
				}

			} // if (dwStatus == ERROR_SUCCESS && dwDisp == REG_CREATED_NEW_KEY) 

			if (hKey)
				RegCloseKey(hKey);
		}
	}
	return;
}


/*
	GetVIDPID needs top be called before RegisterINFValues so that gcPortName is populated
*/
extern "C"
BOOL GetVIDPID(char * filename, DWORD * pdwVID, DWORD * pdwPID)
{
	MultiPlatformFile mpf;

	memset(gcPortName, 0x00, 1024);

	if(mpf.Open(filename, 0) == 0) {

		if(FindSection(&mpf, "[Manufacturer]") == TRUE) {
			ParseManufacturer(&mpf, pdwVID, pdwPID);
		}

		mpf.Close();

		if((*pdwVID != -1) && (*pdwPID != -1)) {
			return TRUE;
		}
	}
	return FALSE;
}

/*
	GetNextVIDPID - 
	Purpose: This is the function that will loop around the INF file retrieving the next VID and PID
			(need to call this continually until fail to allow the INF file to close)
*/
extern "C"
BOOL GetNextVIDPID(char * filename, DWORD * pdwVID, DWORD * pdwPID)
{
	static MultiPlatformFile * pmpf = NULL;

	if(pmpf == NULL) {
		pmpf = new MultiPlatformFile;
	}

	if(pmpf->IsOpen() == FALSE) {
		//
		// Just do this as the previous one did
		//
		memset(gcPortName, 0x00, 1024);
		if(pmpf->Open(filename, 0) == 0) {
			//
			// Initial reading of the file
			//
			if(FindSection(pmpf, "[Manufacturer]") == TRUE) {
				ParseManufacturer(pmpf, pdwVID, pdwPID);
			}
			if((*pdwVID != -1) && (*pdwPID != -1)) {
				return TRUE;
			}
		}
	}
	else {
		if(FindVIDPIDInLine(pmpf, pdwVID, pdwPID) == FALSE) {
			pmpf->Close();
			delete pmpf;
			return FALSE;
		}
		else {
			return TRUE;
		}
	}

	return FALSE;
}

extern "C"
void RegisterINFValues(char * filename, TCHAR * pgszDriverPrefix, TCHAR * pszStreamDriverKey)
{
	char cRegSettings[1024];	// this can change so keep it dynamic
	MultiPlatformFile mpf;

	sprintf(cRegSettings, "[%s%s", gcPortName, &acSections[1][0]);

	if(mpf.Open(filename, 0) == 0) {
		if(FindSection(&mpf, cRegSettings) == TRUE) {
			ParseRegSettings(&mpf, pgszDriverPrefix, pszStreamDriverKey);
		}
		mpf.Close();
	}
}

extern "C"
BOOL GetStreamDriverKey(TCHAR * pszStreamDriverKey)
{
	swprintf(pszStreamDriverKey, TEXT("Drivers\\USB\\ClientDrivers\\FTDI_DEVICE"));
	return TRUE;
}
/*BOOL GetStreamDriverKey(TCHAR * pszStreamDriverKey, int size, int PID, BOOL bGotPID)
{
	if(bGotPID) {
		swprintf(pszStreamDriverKey, TEXT("Drivers\\USB\\FTDI_DEVICE_%04X"), PID);
	}
	else {
		wcscpy(pszStreamDriverKey, TEXT("Drivers\\USB\\FTDI_DEVICE"));
	}
	return TRUE;
}*/

/*
	This is for ActivateDevice - need to see if the key is there in the first place if not go 
	for the default
*/
/*extern "C"
BOOL GetCurrentStreamDriverKey(TCHAR * pszStreamDriverKey, int PID)
{
	HKEY hKey;
	DWORD dwErr;
	DWORD dwLastErr;

	swprintf(pszStreamDriverKey, TEXT("Drivers\\USB\\FTDI_DEVICE_%04X"), PID);

	// Test if the key is present
	dwErr = RegOpenKeyEx(HKEY_LOCAL_MACHINE, pszStreamDriverKey, 0, 0, &hKey);

	if(dwErr != ERROR_SUCCESS) {
		dwLastErr = GetLastError();
		// Use the default
		wcscpy(pszStreamDriverKey, TEXT("Drivers\\USB\\FTDI_DEVICE"));
	}
	if(hKey)
		RegCloseKey(hKey);

	return TRUE;
}*/

/*
	This is for ActivateDevice 

	Unfortuately this will not work as Windows CE core makes up the function pointers to Init, Read, Write etc from
	the Prefix + the name - so unless I have self modifying code that changed the name of the functions (which would 
	lead to alot of problems on multi platforms) then it will need to be a recompile for customers who want 10 ports.
	NO THIS DOES WORK

*/
extern "C"
BOOL GetDevicePrefix(TCHAR * pszStreamDriverKey, TCHAR * pszDriverPrefix, int size)
{
	HKEY hKey;
	DWORD dwType, dwSize;
	DWORD dwErr;
	DWORD dwLastErr;

	// Test if the key is present
	dwErr = RegOpenKeyEx(HKEY_LOCAL_MACHINE, pszStreamDriverKey, 0, 0, &hKey);

	if(dwErr == ERROR_SUCCESS) {
		dwSize = size * sizeof(TCHAR);
		dwErr = RegQueryValueEx(hKey, TEXT("Prefix"), 0, &dwType, (LPBYTE)pszDriverPrefix, &dwSize);
		if (dwErr != ERROR_SUCCESS) {
			wcscpy(pszDriverPrefix, DEFAULT_SERIAL_NAME);
		}
		dwLastErr = GetLastError();
	}
	if(hKey)
		RegCloseKey(hKey);

	return TRUE;
}
