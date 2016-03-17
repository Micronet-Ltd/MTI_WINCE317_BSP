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
/*--------------------------------------------------------------------------*\
   Include File:  loc.h

   Include file for the Comm Control Panel Applet
      
\*--------------------------------------------------------------------------*/

#ifndef  PH_LOC
#define  PH_LOC

#define  CPL_MAX_STRING             132      // biggest allowed string
#define  MAX_LOCATION_NAME          30       // max length for a location name
#define  MAX_CALL_WAIT_NUM          8        // max length for a CW number
#define  MAX_LOCAL_NUM          	64       // max length for Local number
#define  MAX_LONG_DIST_NUM         	64       // max length for Long Dist.  number
#define  MAX_INTL_NUM         		64       // max length for International number
#define  MAX_AREA_CODE_NUM     		10       // max length for area code
#define  MAX_CNTRYRGN_CODE_NUM 		10       // max length for country/region  code

//--------
// LOCEDIT
//--------
typedef struct tagLOCEDIT
{
   DWORD    dwPermLocationID;  // Stored in registry as decimal string, but really a DWORD
   TCHAR    szName[MAX_LOCATION_NAME+1];
   TCHAR    szLocPrefix[MAX_LOCAL_NUM+1];
   TCHAR    szLDPrefix[MAX_LONG_DIST_NUM+1];
   TCHAR    szIntlPrefix[MAX_INTL_NUM+1];
   TCHAR    szAreaCode[MAX_AREA_CODE_NUM+1];
   TCHAR    szCancelCW[MAX_CALL_WAIT_NUM+1];
   TCHAR    szCntryRgnCode[MAX_CNTRYRGN_CODE_NUM+1];
   DWORD    dwOptions;
   DWORD 	dwStatus;    				 // no-change, deleted, modified...
}  LOCEDIT, *PLOCEDIT,  FAR *LPLOCEDIT;


typedef struct tagLOCATION
   {
   LOCEDIT LocEdit;
   }  LOCATION, *PLOCATION, FAR *LPLOCATION;

//--------------------
// Function Prototypes
//--------------------
UINT   LocationClearList( VOID );
void   ParseLocation(LPTSTR lpLocIDPtr, LPTSTR lpLocPtr, LOCEDIT *Location);

#endif   // PH_LOC

