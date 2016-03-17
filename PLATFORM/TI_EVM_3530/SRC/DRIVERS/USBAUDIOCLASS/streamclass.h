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
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES OR INDEMNITIES.
//
// -----------------------------------------------------------------------------
//
//      THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//      ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//      THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//      PARTICULAR PURPOSE.
//
// -----------------------------------------------------------------------------

#ifndef _STREAMCLASS_H_
#define _STREAMCLASS_H_

//#include <initguid.h>

#if (_MSC_VER >= 1000)
#pragma once
#endif

#if (_WINCEOSVER!=700)
#define WAVE_STREAMCLASS_CLASSGAIN_MAX  0xffff

#define MM_PROP_STREAMCLASS_CLASSID         1
#define MM_PROP_STREAMCLASS_CLASSGAIN       2
#define MM_PROP_STREAMCLASS_CONFIG          3

// {17C317CD-B02B-4fbd-88AC-C6C0CA78C815}
DEFINE_GUID(MM_PROPSET_STREAMCLASS, 
		0x17c317cd, 0xb02b, 0x4fbd, 0x88, 0xac, 0xc6, 0xc0, 0xca, 0x78, 0xc8, 0x15);

// Stream class config structure - Passed as an array in pvPropData field of WAVEPROPINFO
typedef struct _STREAMCLASSCONFIG
{
	DWORD dwClassId;
	DWORD dwClassGain;
	BOOL fBypassDeviceGain;
} STREAMCLASSCONFIG, *PSTREAMCLASSCONFIG;

// Stream class config property ID

#define MM_PROP_STREAMCLASSCONFIG           1

#define MM_PROP_STREAMCLASS_CLASSID         1
#define MM_PROP_STREAMCLASS_CLASSGAIN       2

#endif//~(_WINCEOSVER!=700)

const DWORD c_dwClassIdDefault = 0;
const DWORD c_dwClassGainDefault = WAVE_STREAMCLASS_CLASSGAIN_MAX;
const BOOL c_fBypassDeviceGainDefault = FALSE;

class StreamClassTable
{
public:

    StreamClassTable();
    virtual ~StreamClassTable();

    BOOL Initialize();
    BOOL Reinitialize(STREAMCLASSCONFIG *pTable, DWORD cEntries);

    STREAMCLASSCONFIG *FindEntry(DWORD dwClassId);
    DWORD GetNumEntries() { return m_cEntries; }

protected:

    STREAMCLASSCONFIG *m_pTable;
    DWORD m_cEntries;
};

#endif // _STREAMCLASS_H_
