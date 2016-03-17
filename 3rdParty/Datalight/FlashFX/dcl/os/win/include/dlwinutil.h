/*---------------------------------------------------------------------------
               ----> DO NOT REMOVE THE FOLLOWING NOTICE <----

                  Copyright (c) 1993 - 2010 Datalight, Inc.
                       All Rights Reserved Worldwide.

  Datalight, Incorporated is a Washington State corporation currently located
  at:
        21520 30th Dr SE, Suite 110,      Tel:  425-951-8086
        Bothell, WA  98021                Fax:  425-951-8094
        USA                               Web:  http://www.datalight.com

  This software, including without limitation all source code and documen-
  tation, is a trade secret and the confidential property of Datalight, 
  Inc., protected under the copyright laws of the United States and other
  jurisdictions.  Patents may be pending.

  U.S. Government Restricted Rights:  Use, duplication, reproduction, or
  transfer of this commercial product and accompanying documentation is
  restricted in accordance with FAR 12.212 and DFARS 227.7202 and by a
  License Agreement.

  IN ADDITION TO COPYRIGHT AND PATENT LAW, THIS SOFTWARE IS PROTECTED UNDER
  CONTRACT(S) BETWEEN DATALIGHT, INC. AND THE LICENSEE ("BINDING AGREEMENTS").
  IF YOU ARE A LICENSEE, YOUR RIGHT, IF ANY, TO COPY, PUBLISH, MODIFY, OR
  OTHERWISE USE THE SOFTWARE, IS SUBJECT TO THE TERMS AND CONDITIONS OF THE
  BINDING AGREEMENTS.  BY USING THE SOFTWARE IN ANY MANNER, IN WHOLE OR IN
  PART, YOU AGREE TO BE BOUND BY THE TERMS OF THE BINDING AGREEMENTS.

  IF YOU ARE NOT A DATALIGHT LICENSEE, ANY USE MAY RESULT IN CIVIL AND
  CRIMINAL ACTION AGAINST YOU.  CONTACT DATALIGHT, INC. AT THE ADDRESS
  SET FORTH ABOVE IF YOU OBTAINED THIS SOFTWARE IN ERROR.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Description

    This module defines the interface to the generic utility functions in
    the "win" OS abstraction.

    This code must be fully compilable and usable under both normal Win32
    user mode, as well as Windows CE.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dlwinutil.h $
    Revision 1.5  2010/11/01 04:11:46Z  garyp
    Added and updated prototypes.
    Revision 1.4  2009/07/15 00:54:15Z  garyp
    Added the DCLWINSETTINGS structure, along with various prototypes.
    Revision 1.3  2009/06/27 23:06:13Z  garyp
    Added GUI stuff.
    Revision 1.2  2009/02/08 05:08:14Z  garyp
    Removed GUI stuff.
---------------------------------------------------------------------------*/

#ifndef DLWINUTIL_H_INCLUDED
#define DLWINUTIL_H_INCLUDED

typedef struct
{
    BOOL            fEscKeyToExit;
    LONG            lFontHeight;
    LONG            lFontWeight;
    BOOL            fFontItalic;
    BYTE            bFontPitchAndFamily;
    D_WCHAR         wzFontFaceName[LF_FACESIZE];
} DCLWINSETTINGS;

typedef struct
{
    const D_WCHAR  *pwzCaption;
    const D_WCHAR  *pwzPrompt;
    D_WCHAR        *pwzBuffer;
    unsigned        nBufferLen;
} DCLWINDLGEDITDATA;

HANDLE      DclWinCommOpen(const D_WCHAR *pwzCommName);
DCLSTATUS   DclWinCommClose(HANDLE hComm);
DCLSTATUS   DclWinCommParamInit(HANDLE hComm, D_UINT32 ulBaud, D_BYTE bParity, D_BYTE bDataBits, D_BYTE bStopBits);
void        DclWinCommParamDump(const DCB *pDCB, unsigned fVerbose);

HMODULE     DclWinCommonControlsLoad(DWORD dwICC);
D_BOOL      DclWinCommonControlsUnload(HMODULE hModCommCtrl);
int         DclWinDialogEdit(HINSTANCE hInstance, DCLWINDLGEDITDATA *pEditData);
D_BOOL      DclWinDialogSetPosition(HWND hDlg, unsigned nLevel);
unsigned    DclWinDisplayProcessInfo(void);
D_BOOL      DclWinHelpAbout(HWND hWnd, const D_WCHAR *pwzPrefix, const char *pszVersion, const D_WCHAR *pwzSuffix);
HANDLE      DclWinLogOpen(const D_WCHAR *pwzName, D_BOOL fAppend);
DCLSTATUS   DclWinLogClose(HANDLE hLogFile);
int         DclWinLogPrintf(HANDLE hLogFile, const char *pszFmtStr, ...);
const char *DclWinMessageName(UINT nMsg);
char *      DclWinMessageFormat(char *pBuffer, unsigned nBufferLen, HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam);
char *      DclWinMsgFormat(char *pBuffer, unsigned nBufferLen, const MSG *pMsg);
DCLSTATUS   DclWinShellAddCommands(DCLSHELLHANDLE hShell, const char *pszClassName, const char *pszPrefix);

DCLSTATUS   DclWinSettingsLoad(const D_WCHAR *pwzAppName, const DCLWINSETTINGS *pDefaultSettings, DCLWINSETTINGS *pSettingsData);
DCLSTATUS   DclWinSettingsSave(const D_WCHAR *pwzAppName, const DCLWINSETTINGS *pSettingsData);
int         DclWinSettingsEdit(HINSTANCE hInstance, HWND hWndParent, DCLWINSETTINGS *pSettings);
DCLSTATUS   DclWinSettingsDelete(const D_WCHAR *pwzAppName);



#if D_DEBUG && DCLCONF_OUTPUT_ENABLED

#define DCLWINMESSAGEPRINT(lev, txt, h, msg, wp, lp)                        \
    {                                                                       \
        char    abBuff[DCLOUTPUT_BUFFERLEN];                                \
        DCLPRINTF((lev), ((txt),                                            \
            DclWinMessageFormat(abBuff, sizeof abBuff,                      \
            (h), (msg), (wp), (lp))));                                      \
    }

#define DCLWINMESSAGETRACEPRINT(flg, txt, h, msg, wp, lp)                   \
    {                                                                       \
        char    abBuff[DCLOUTPUT_BUFFERLEN];                                \
        DclWinMessageFormat(abBuff, sizeof abBuff,                          \
            (h), (msg), (wp), (lp));                                        \
        DCLTRACEPRINTF(((flg), (txt), abBuff));                             \
    }

#define DCLWINMSGPRINT(lev, txt, msg)                                       \
    {                                                                       \
        char    abBuff[DCLOUTPUT_BUFFERLEN];                                \
        DCLPRINTF((lev), ((txt),                                            \
            DclWinMsgFormat(abBuff, sizeof abBuff, (msg))));                \
    }

#define DCLWINMSGTRACEPRINT(flg, txt, msg)                                  \
    {                                                                       \
        char    abBuff[DCLOUTPUT_BUFFERLEN];                                \
        DclWinMsgFormat(abBuff, sizeof abBuff, (msg));                      \
        DCLTRACEPRINTF(((flg), (txt), abBuff));                             \
    }

#else

#define DCLWINMESSAGEPRINT(lev, txt, h, msg, wp, lp)
#define DCLWINMESSAGETRACEPRINT(flg, txt, h, msg, wp, lp)
#define DCLWINMSGPRINT(lev, txt, msg)
#define DCLWINMSGTRACEPRINT(flg, txt, msg)

#endif

#endif  /* DLWINUTIL_H_INCLUDED */


