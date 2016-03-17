//Sliders.h

#ifndef __SLIDERS_H
#define __SLIDERS_H
//#include "KeypadCpl.h"
#include <TrackBar.h>

#define NUM_OF_SLIDERS	3 

class CTrackbarsArr
{
public:
	
	CTrackbarsArr();
	void Init( HWND hDlg, DWORD Id, DWORD Level );
//	void UpdatePos();
	void PosChanged( HWND hwnd );
	DWORD GetBlValue( DWORD Id );

private:
	CTrackbar	arr[NUM_OF_SLIDERS];
};

#endif //__SLIDERS_H
