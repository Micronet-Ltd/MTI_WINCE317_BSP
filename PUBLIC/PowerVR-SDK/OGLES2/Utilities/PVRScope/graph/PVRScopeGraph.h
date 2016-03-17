
#ifndef _PVRSCOPEGRAPH_H_
#define _PVRSCOPEGRAPH_H_


/****************************************************************************
** Structures
****************************************************************************/

// Internal implementation data
struct SPVRScopeData;
struct SPVRScopeCounter;
struct SPVRScopeAPIData;

/****************************************************************************
** Class: CPVRScopeGraph
****************************************************************************/
class CPVRScopeGraph
{
protected:
	SPVRScopeData		*m_pPVRScopeData;

	unsigned int		m_nCounterNum;
	SPVRScopeCounter	*m_pCounters;
	unsigned int		*m_pnCounterValues;
	unsigned int		m_nActiveGroup;

	unsigned int		*m_pnValueCB;	// Circular buffer of counter values
	unsigned int		m_nWritePosCB;	// Current write position of circular buffer
	unsigned int		m_nSizeCB;
	unsigned int		m_nActiveCounter;
	unsigned int		m_nActiveCounterValueIdx;

	SPVRScopeAPIData	*m_pPVRScopeAPIData;

	float				m_fX, m_fY, m_fPixelW, m_fGraphH;

public:
	CPVRScopeGraph();
	~CPVRScopeGraph();
	void Ping();

	void select(const unsigned int nCounter);
	unsigned int GetActiveCounter() const { return m_nActiveCounter; }

	unsigned int GetCounterNum() const { return m_nCounterNum; }

	const char *GetCounterName(const unsigned int i) const;

	void position(
		const unsigned int nViewportW, const unsigned int nViewportH,
		const unsigned int nGraphX, const unsigned int nGraphY,
		const unsigned int nGraphW, const unsigned int nGraphH);

protected:
	SPVRScopeAPIData *APIInit();
	void APISize(SPVRScopeAPIData * const pData, const unsigned int nViewportW, const unsigned int nViewportH);
	void APIShutdown(SPVRScopeAPIData ** const ppData);
	void APIRender(SPVRScopeAPIData * const pData);
};


/****************************************************************************
** Declarations
****************************************************************************/


#endif /* _PVRSCOPEGRAPH_H_ */

/*****************************************************************************
 End of file (PVRScopeGraph.h)
*****************************************************************************/
