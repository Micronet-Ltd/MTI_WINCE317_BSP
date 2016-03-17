
#include "PVRScope.h"
#include "PVRScopeGraph.h"

/****************************************************************************
** Defines
****************************************************************************/

/****************************************************************************
** Structures
****************************************************************************/

/****************************************************************************
** Constants
****************************************************************************/

/****************************************************************************
** Code
****************************************************************************/
CPVRScopeGraph::CPVRScopeGraph()
{
	m_pPVRScopeData = PVRScopeInit();
	if(m_pPVRScopeData)
	{
		PVRScopeGetPerfCounters(m_pPVRScopeData, &m_nCounterNum, &m_pCounters, &m_pnCounterValues);
		m_pPVRScopeAPIData = APIInit();

		m_pnValueCB = 0;
		position(320, 240, 0, 0, 320, 240);
		select(0);
	}
}

CPVRScopeGraph::~CPVRScopeGraph()
{
	if(m_pPVRScopeData)
	{
		APIShutdown(&m_pPVRScopeAPIData);

		PVRScopeDeInit(&m_pPVRScopeData, &m_pCounters, &m_pnCounterValues);

		delete [] m_pnValueCB;
		m_pnValueCB = 0;
	}
}

void CPVRScopeGraph::Ping()
{
	if(m_pPVRScopeData)
	{
		unsigned int nGroupSelect;

		if(m_nActiveGroup == m_pCounters[m_nActiveCounter].nGroup)
		{
			nGroupSelect = 0xffffffff;
		}
		else
		{
			nGroupSelect = m_pCounters[m_nActiveCounter].nGroup;
		}

		unsigned int ui32ValueCnt;

		if(PVRScopeReadPerfCountersThenSetGroup(m_pPVRScopeData, &ui32ValueCnt, m_pnCounterValues, m_nCounterNum, nGroupSelect))
		{
			// The active group is always returned as counter 0
			m_nActiveGroup = m_pnCounterValues[0];

			// Write the counter value to the buffer
			if(m_nWritePosCB >= m_nSizeCB)
			{
				m_nWritePosCB = 0;
			}
			m_pnValueCB[m_nWritePosCB++] = m_pnCounterValues[m_nActiveCounterValueIdx];
		}

		APIRender(m_pPVRScopeAPIData);
	}
}

void CPVRScopeGraph::select(const unsigned int nCounter)
{
	unsigned int i;

	m_nActiveCounter			= nCounter;
	m_nActiveCounterValueIdx	= 0;

	for(i = 0; i < m_nActiveCounter; ++i)
	{
		if(m_pCounters[i].nGroup == m_pCounters[m_nActiveCounter].nGroup || m_pCounters[i].nGroup == 0xffffffff)
		{
			++m_nActiveCounterValueIdx;
		}
	}
}

const char *CPVRScopeGraph::GetCounterName(const unsigned int i) const
{
	if(i > m_nCounterNum)
		return "";
	return m_pCounters[i].pszName;
}

void CPVRScopeGraph::position(
	const unsigned int nViewportW, const unsigned int nViewportH,
	const unsigned int nGraphX, const unsigned int nGraphY,
	const unsigned int nGraphW, const unsigned int nGraphH)
{
	m_nSizeCB		= nGraphW;

	delete [] m_pnValueCB;
	m_pnValueCB		= new unsigned int[m_nSizeCB];

	m_nWritePosCB	= 0;

	m_fPixelW = 2 * 1.0f / nViewportW;
	m_fGraphH = 2 * (float)nGraphH / nViewportH;
	m_fX = 2 * ((float)nGraphX / nViewportW) - 1;
	m_fY = 2 * ((float)nGraphY / nViewportH) - 1;

	APISize(m_pPVRScopeAPIData, nViewportW, nViewportH);
}

/*****************************************************************************
 End of file (PVRScopeGraph.cpp)
*****************************************************************************/
