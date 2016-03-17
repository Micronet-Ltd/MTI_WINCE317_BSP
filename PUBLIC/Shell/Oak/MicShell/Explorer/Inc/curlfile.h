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
#ifndef _H_CURLFILE_
#define _H_CURLFILE_

class CUrlFile
{
private:
    HANDLE m_hFile;

    static const BYTE UTF8_BOM1;
    static const BYTE UTF8_BOM2;
    static const BYTE UTF8_BOM3;

    static const WCHAR s_wszInternetShortCut[];
    static const WCHAR s_wszURLMark[];
    
    int GetLine(char *pszBuffer, int cchBuffer);

public:
    CUrlFile() { m_hFile = INVALID_HANDLE_VALUE; }
    ~CUrlFile() { CloseFile(); }
    HRESULT OpenFile(WCHAR *pwszPath, BOOL bCreate);
    void CloseFile();
    HRESULT ReadURL(WCHAR *pwszURL, int cchUrl);
    HRESULT WriteURL(WCHAR *pwszURL);
};

#endif
