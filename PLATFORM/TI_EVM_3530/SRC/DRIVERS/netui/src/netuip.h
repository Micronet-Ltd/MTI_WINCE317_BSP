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
/*++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Module Name:  

netuip.h

Abstract:  

Functions:


Notes: 


--*/
#ifndef _NETUIP_H_
#define _NETUIP_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifdef DEBUG
// Debug ZONE defines
#define ZONE_INIT		DEBUGZONE(0)
#define ZONE_USERPWD	DEBUGZONE(1)
#define ZONE_GETIP  	DEBUGZONE(2)
#define ZONE_LINECONFIG DEBUGZONE(3)
#define ZONE_ADDCONN    DEBUGZONE(4)
#define ZONE_DISCONN    DEBUGZONE(5)
#define ZONE_NETWORK	DEBUGZONE(6)
#define ZONE_DRIVERNAME	DEBUGZONE(7)
#define ZONE_ETHMAN		DEBUGZONE(8)

#define ZONE_INTERFACE	DEBUGZONE(10)
#define ZONE_MISC		DEBUGZONE(11)
#define ZONE_ALLOC		DEBUGZONE(12)
#define ZONE_FUNCTION	DEBUGZONE(13)
#define ZONE_WARN		DEBUGZONE(14)
#define ZONE_ERROR		DEBUGZONE(15)

#endif

#define  ARRAYSIZE(a) (sizeof(a)/ sizeof(a[0]))

// Location of help exe
#define PEGHELP_EXE				TEXT("\\Windows\\peghelp.exe")

// Name of help file.
#define HELP_FILE				TEXT("\\Windows\\rnetw.htm")

// ----------------------------------------------------------------
//
// Global Data
//
// ----------------------------------------------------------------

extern HINSTANCE v_hInst;

// ----------------------------------------------------------------
//
// IPAddress Control info
//
// ----------------------------------------------------------------
/*
    defines and Typedefs.
*/

#define	MAX_IPNOMEMSTRING	30
#define MAX_IPCAPTION		30
#define MAX_IPRES		256

//#define MAX_NAME	63
#define NUM_FIELDS	4	// number of fields in an IP Address
#define MIN_FIELD1	1	// min allowed value for field 1
#define MAX_FIELD1	223	// max allowed value for field 1
#define MIN_FIELD2	0	// min for field 2
#define MAX_FIELD2	255	// max for field 2
#define MIN_FIELD3	0	// min for field 3
#define MAX_FIELD3	254	// max for field 3
#define MIN_FIELD4      1       // 0 is reserved for broadcast

#define MAX_SCOPE_ID	47	// maximum number of characters in scope id
#define MAX_SESSIONS	22	// maximum number of sessions
#define	SESSION_DIGITS   2	// maximum length of sessions as a string
#define DEFAULT_SESSIONS 6	// default number of sessions
#define DEFAULT_SOCKETS  4	// default number of sockets
//#define MAX_NAME	63	// maximum length of hostname or domain

#define MAX_MBSTRING	400	// Maximum length of string for message box.
#define MAX_KEY		32	// Maximum length of a key field in a .INI file
#define	MAX_NOMEMSTRING	128	// Maximum length of out of memory string
#define MAX_CAPTION	80	// Maximum length of caption

#define IPADDRESS_CLASS            TEXT("RNA_IPAddress")

/* The character that is displayed between address fields. */
#define	FILLER		TEXT('.')
#define	SZFILLER	TEXT(".")
#define SPACE		TEXT(' ')
#define BACK_SPACE	8

/* Min, max values */
#define NUM_FIELDS	4
#define CHARS_PER_FIELD	3
#define HEAD_ROOM	2	/* space at top of control */
#define LEAD_ROOM	2	/* space at front of control */
#define MIN_FIELD_VALUE	0	/* default minimum allowable field value */
#define MAX_FIELD_VALUE	255	/* default maximum allowable field value */


/* All the information unique to one control is stuffed in one of these
   structures in global memory and the handle to the memory is stored in the 
   Windows extra space.
 */  

typedef struct tagFIELD {
    HANDLE	hWnd;
    FARPROC	lpfnWndProc;
    BYTE	byLow;	/* lowest allowed value for this field. */
    BYTE	byHigh;	/* Highest allowed value for this field.*/
    DWORD	dwOrigVal;
} FIELD;

typedef struct tagCONTROL {
    HWND	hwndParent;
    UINT	uiFieldWidth;
    UINT	uiFillerWidth;
    DWORD	dwStyle;	/* Style bits for the control*/
    BOOL	fEnabled;
    BOOL	fPainted;
    BOOL	fInMessageBox;	/* Set when a message box is displayed so that
                                   we don't send a EN_KILLFOCUS message when
				   we receive the EN_KILLFOCUS message for the
				   current field. */
    BOOL	fHaveFocus;	/* TRUE if we have the focus, FALSE otherwise.*/
    FIELD	Children[NUM_FIELDS];
} CONTROL;


/* The following macros extract and store the CONTROL structure for a control.*/
#define    IPADDRESS_EXTRA            sizeof(HANDLE)

#define	GET_CONTROL_HANDLE(hWnd)	((HANDLE)(GetWindowLong((hWnd), 0)))
#define SAVE_CONTROL_HANDLE(hWnd,x)	(SetWindowLong((hWnd), 0, (DWORD)x))


#define CheckDlgButton(hDlg, idCtl, bChk) SendDlgItemMessage(hDlg, idCtl, BM_SETCHECK, bChk,  0)
#define IsDlgButtonChecked(hDlg, idCtl)	  SendDlgItemMessage(hDlg, idCtl, BM_GETCHECK, 0,  0)


/* internal IPAddress function prototypes */
LONG CALLBACK IPAddrDlgProc(HWND, UINT, WPARAM, LPARAM );
LONG CALLBACK IPAddressWndFn( HWND, UINT, WPARAM, LPARAM );
LONG CALLBACK IPAddressFieldProc(HWND, UINT, WPARAM, LPARAM);
BOOL SwitchFields(CONTROL FAR *, int, int, DWORD, DWORD);
void EnterField(FIELD FAR *, DWORD, DWORD);
BOOL ExitField(CONTROL FAR *, int iField);
void PadField(CONTROL FAR *, int iField);
int GetFieldValue(FIELD FAR *);
DWORD CheckAddress(DWORD ip);


// ----------------------------------------------------------------
//
// Local Structure definitions
//
// ----------------------------------------------------------------

// ----------------------------------------------------------------
// externs
//
// ----------------------------------------------------------------

extern CRITICAL_SECTION g_csNetUI;

#ifdef __cplusplus
}
#endif

#endif // _NETUIP_H_
