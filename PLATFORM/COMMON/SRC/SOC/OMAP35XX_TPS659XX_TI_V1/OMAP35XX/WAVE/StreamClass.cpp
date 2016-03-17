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

#include <windows.h>
#include <mmsystem.h>
#include <mmddk.h>
#include "StreamClass.h"

StreamClassTable::StreamClassTable() :
    m_pTable(NULL),
    m_cEntries(0)
{

}

StreamClassTable::~StreamClassTable()
{
    if (m_pTable)
    {
        delete [] m_pTable;
    }
}

// Initializes the stream class table by creating a default entry.
BOOL
StreamClassTable::Initialize()
{
    DEBUGCHK((NULL == m_pTable) && (0 == m_cEntries));

    // Create default entry.  It is expected for StreamClassTable::Reinitialize
    // to be called afterwards.
    m_pTable = new STREAMCLASSCONFIG[1];
    if (m_pTable != NULL)
    {
        m_pTable[0].dwClassId = c_dwClassIdDefault;
        m_pTable[0].dwClassGain = c_dwClassGainDefault;
        m_pTable[0].fBypassDeviceGain = c_fBypassDeviceGainDefault;
        m_cEntries = 1;
    }

    return (m_pTable != NULL);
}

// Reinitializes the stream class table with new entries.  Old table is
// discarded.
BOOL
StreamClassTable::Reinitialize(
    STREAMCLASSCONFIG *pTable,
    DWORD cEntries
    )
{
    if ((NULL == pTable) || (0 == cEntries))
    {
        return FALSE;
    }

    BOOL fSuccess = FALSE;

    // Alloc new table.
    STREAMCLASSCONFIG *pNewTable = new STREAMCLASSCONFIG[cEntries];
    if (pNewTable != NULL)
    {
        // Copy to new table.
        memcpy(pNewTable, pTable, cEntries * sizeof(pNewTable[0]));

        // Wipe old table.
        if (m_pTable)
        {
            delete [] m_pTable;
        }

        // Save new table.
        m_pTable = pNewTable;
        m_cEntries = cEntries;

        fSuccess = TRUE;
    }

    return fSuccess;
}

// Finds an entry with matching class ID.
STREAMCLASSCONFIG *
StreamClassTable::FindEntry(
    DWORD dwClassId
    )
{
    if ((NULL == m_pTable) || (0 == m_cEntries))
    {
        return NULL;
    }

    STREAMCLASSCONFIG *pEntry = NULL;

    for (DWORD iEntry = 0; iEntry < m_cEntries; iEntry++)
    {
        if (m_pTable[iEntry].dwClassId == dwClassId)
        {
            pEntry = &m_pTable[iEntry];
            break;
        }
    }

    return pEntry;
}
