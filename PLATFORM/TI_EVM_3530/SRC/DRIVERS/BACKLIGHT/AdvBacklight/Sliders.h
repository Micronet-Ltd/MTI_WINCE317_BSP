//Sliders.h

#ifndef __SLIDERS_H
#define __SLIDERS_H
#include "AdvBacklight.h"
#include <TrackBar.h>

class CTrackbarsArr
{
public:
	
	CTrackbarsArr();
	void Init( HWND hDlg, DWORD Id, DWORD Level );
	void UpdatePos();
	void UpdatePos0( HWND hDlg1 );
	void PosChanged( HWND hwnd );
	DWORD GetBlValue( DWORD Id );
	void SetMinPos(DWORD MinPos);
private:
	CTrackbar	arr[10];
	DWORD		m_minPos;
};

#endif //__SLIDERS_H
