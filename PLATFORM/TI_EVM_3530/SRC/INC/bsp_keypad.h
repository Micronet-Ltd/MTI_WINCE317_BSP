// Copyright (c) 2013 Micronet Ltd. All rights reserved.
// by Vladimir Zatulovsky

/*
================================================================================
*             Texas Instruments OMAP(TM) Platform Software
* (c) Copyright Texas Instruments, Incorporated. All Rights Reserved.
*
* Use of this software is controlled by the terms and conditions found
* in the license agreement under which this software has been supplied.
*
================================================================================
*/

#ifndef __BSP_KEYPAD_H
#define __BSP_KEYPAD_H
//------------------------------------------------------------------------------
//
//  Column 0 keys
#define KEY_S00 ((UINT64)1 << 0) // DOWN
#define KEY_S10 ((UINT64)1 << 1) // SPARE
#define KEY_S20 ((UINT64)1 << 2) // RIGHT
#define KEY_S30 ((UINT64)1 << 3) // LEFT
#define KEY_S40 ((UINT64)1 << 4) // ACCEPT
#define KEY_S50 ((UINT64)1 << 5) // DECLINE
#define KEY_S60 ((UINT64)1 << 6) // UP
#define KEY_S70	((UINT64)1 << 7) // PUSH
//
//  Column 1 keys
#define KEY_S01 ((UINT64)1 << 0) // DOWN
#define KEY_S11 ((UINT64)1 << 1) // SPARE
#define KEY_S21 ((UINT64)1 << 2) // RIGHT
#define KEY_S31 ((UINT64)1 << 3) // LEFT
#define KEY_S41 ((UINT64)1 << 4) // ACCEPT
#define KEY_S51 ((UINT64)1 << 5) // DECLINE
#define KEY_S61	((UINT64)1 << 6) // UP
#define KEY_S71	((UINT64)1 << 7) // PUSH
//
//  Column 2 keys
#define KEY_S02 ((UINT64)1 << 0) // DOWN
#define KEY_S12 ((UINT64)1 << 1) // SPARE
#define KEY_S22 ((UINT64)1 << 2) // RIGHT
#define KEY_S32 ((UINT64)1 << 3) // LEFT
#define KEY_S42 ((UINT64)1 << 4) // ACCEPT
#define KEY_S52 ((UINT64)1 << 5) // DECLINE
#define KEY_S62	((UINT64)1 << 6) // UP
#define KEY_S72	((UINT64)1 << 7) // PUSH
//
//  Column 3 keys
#define KEY_S03 ((UINT64)1 << 0) // DOWN
#define KEY_S13 ((UINT64)1 << 1) // SPARE
#define KEY_S23 ((UINT64)1 << 2) // RIGHT
#define KEY_S33 ((UINT64)1 << 3) // LEFT
#define KEY_S43 ((UINT64)1 << 4) // ACCEPT
#define KEY_S53	((UINT64)1 << 5) // DECLINE
#define KEY_S63	((UINT64)1 << 6) // UP
#define KEY_S73	((UINT64)1 << 7) // PUSH
//
//  Column 4 keys
#define KEY_S04 ((UINT64)1 << 0) // DOWN
#define KEY_S14 ((UINT64)1 << 1) // SPARE
#define KEY_S24 ((UINT64)1 << 2) // RIGHT
#define KEY_S34 ((UINT64)1 << 3) // LEFT
#define KEY_S44 ((UINT64)1 << 4) // ACCEPT
#define KEY_S54 ((UINT64)1 << 5) // DECLINE
#define KEY_S64	((UINT64)1 << 6) // UP
#define KEY_S74	((UINT64)1 << 7) // PUSH
//
//  Column 5 keys
#define KEY_S05 ((UINT64)1 << 0) // DOWN
#define KEY_S15 ((UINT64)1 << 1) // SPARE
#define KEY_S25 ((UINT64)1 << 2) // RIGHT
#define KEY_S35 ((UINT64)1 << 3) // LEFT
#define KEY_S45 ((UINT64)1 << 4) // ACCEPT
#define KEY_S55 ((UINT64)1 << 5) // DECLINE
#define KEY_S65	((UINT64)1 << 6) // UP
#define KEY_S75	((UINT64)1 << 7) // PUSH
//
//  Column 6 keys
#define KEY_S06 ((UINT64)1 << 0) // DOWN
#define KEY_S16 ((UINT64)1 << 1) // SPARE
#define KEY_S26 ((UINT64)1 << 2) // RIGHT
#define KEY_S36 ((UINT64)1 << 3) // LEFT
#define KEY_S46 ((UINT64)1 << 4) // ACCEPT
#define KEY_S56 ((UINT64)1 << 5) // DECLINE
#define KEY_S66	((UINT64)1 << 6) // UP
#define KEY_S76	((UINT64)1 << 7) // PUSH
//
//  Column 7 keys
#define KEY_S07 ((UINT64)1 << 0) // DOWN
#define KEY_S17 ((UINT64)1 << 1) // SPARE
#define KEY_S27 ((UINT64)1 << 2) // RIGHT
#define KEY_S37 ((UINT64)1 << 3) // LEFT
#define KEY_S47 ((UINT64)1 << 4) // ACCEPT
#define KEY_S57 ((UINT64)1 << 5) // DECLINE
#define KEY_S67	((UINT64)1 << 6) // UP
#define KEY_S77	((UINT64)1 << 7) // PUSH

UINT64 OALReadKeyPad();
//===menu===================================================================================
typedef struct
{
	UINT64	Key;
	WCHAR*	KeyName;
} KeypadKey;

extern KeypadKey KeyStr[];
extern int PUSH_INDEX;
//#define PUSH_INDEX ((sizeof(KeyStr)/sizeof(KeyStr[0])) - 2)
#define PUSH_KEY_INDEX(a) ((sizeof(a)/sizeof(a[0])) - 2)

#define OAL_MENU_MAX_DEVICES    8				
#define	MAX_INI_LEN				2048
#define MAX_LINE_LEN			256
#define	SHOW_DELAY				3000000

#define	StartX					10 
#define	StartY					10 


UINT32	GetKeyPressed();
BOOL	GetValStr(const CHAR* Name, CHAR* pTxt, CHAR* pDest, UINT32 Len);
CHAR*	TrimLeft( CHAR* pTxt, UINT32 MaxLen );
VOID	StartMenu();
VOID	Strn2Wstr(CHAR* pStr, WCHAR* pWstr, int len);
#endif // __BSP_KEYPAD_H
