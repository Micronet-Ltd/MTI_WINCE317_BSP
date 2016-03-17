
#include "Sliders.h"

CTrackbarsArr::CTrackbarsArr()
{
	m_minPos = BKL_MIN_SETTING;
	for( int i = 0; i < 10; ++ i )
	{
		arr[i].InitBase(	BKL_MIN_SETTING,
							BKL_MAX_SETTING,
							BKL_TICK_INTERVAL,
							10,	//TICK_FREQ;
							1,	//LINE_SIZE;
							1	//PAGE_SIZE;
						);
	}
}
void CTrackbarsArr::SetMinPos(DWORD MinPos)
{
	m_minPos = MinPos;
}

void CTrackbarsArr::Init( HWND hDlg, DWORD Id, DWORD Level )
{
	switch(Id)
	{
		case IDC_TRACKBAR_BT_ON     :
			arr[0].Init( hDlg, Id, Level );
		break;
		case IDC_TRACKBAR_BT_OFF    :
			arr[1].Init( hDlg, Id, Level );
		break;
		case IDC_TRACKBAR_AC_ON     :
			arr[2].Init( hDlg, Id, Level );
		break;
		case IDC_TRACKBAR_AC_OFF    :
			arr[3].Init( hDlg, Id, Level );
		break;
		case IDC_TRACKBAR_BATT_HIGH	:	
			arr[4].Init( hDlg, Id, Level );
		break;
		case IDC_TRACKBAR_BATT_NORM	:
			arr[5].Init( hDlg, Id, Level );
		break;
		case IDC_TRACKBAR_BATT_LOW	:
			arr[6].Init( hDlg, Id, Level );
		break;
		case IDC_TRACKBAR_AC_HIGH	:
			arr[7].Init( hDlg, Id, Level );
		break;
		case IDC_TRACKBAR_AC_NORM	:
			arr[8].Init( hDlg, Id, Level );
		break;
		case IDC_TRACKBAR_AC_LOW	:	
			arr[9].Init( hDlg, Id, Level );
		break;
			
		default:
			break;
	}
}
void CTrackbarsArr::UpdatePos()
{
	arr[4].UpdatePos( arr[0].GetPos(), arr[0].GetPos() );//=
	arr[5].UpdatePos( arr[1].GetPos(), arr[0].GetPos() );
	arr[6].UpdatePos( arr[1].GetPos(), arr[0].GetPos() );
	
	arr[7].UpdatePos( arr[2].GetPos(), arr[2].GetPos() );//=
	arr[8].UpdatePos( arr[3].GetPos(), arr[2].GetPos() );
	arr[9].UpdatePos( arr[3].GetPos(), arr[2].GetPos() );

}

void CTrackbarsArr::UpdatePos0( HWND hDlg1 )
{
	arr[0].UpdatePos( arr[1].GetPos(), ( hDlg1 ? arr[4].GetPos() : BKL_MAX_SETTING ) );
	arr[2].UpdatePos( arr[3].GetPos(), ( hDlg1 ? arr[7].GetPos() : BKL_MAX_SETTING ) );
}

void CTrackbarsArr::PosChanged( HWND hwnd )
{
	if( !hwnd )
		return;
	DWORD j = 0;

	for( DWORD i = 0; i < 10; ++i )
	{
		if( arr[i] == hwnd  )
			break;
	}
	if( i > 9 )
		return;
	RETAILMSG(0,(_T("PosChanged: hwnd %x, i = %d\r\n"), hwnd, i ));
	switch(arr[i].Id())
	{
		case IDC_TRACKBAR_BT_ON     :
			arr[i].UpdatePos( arr[1].GetPos(), BKL_MAX_SETTING );
		break;
		case IDC_TRACKBAR_BT_OFF    :
			arr[i].UpdatePos( m_minPos, arr[0].GetPos() );
		break;
		case IDC_TRACKBAR_AC_ON     :
			arr[i].UpdatePos( arr[3].GetPos(), BKL_MAX_SETTING );
		break;
		case IDC_TRACKBAR_AC_OFF    :
			arr[i].UpdatePos( m_minPos, arr[2].GetPos() );
		break;
		case IDC_TRACKBAR_BATT_HIGH	:
			arr[i].UpdatePos( arr[5].GetPos(), arr[0].GetPos() );
		break;
		case IDC_TRACKBAR_BATT_NORM	:
			arr[i].UpdatePos( arr[6].GetPos(), arr[4].GetPos() );
		break;
		case IDC_TRACKBAR_BATT_LOW	:
			arr[i].UpdatePos( arr[1].GetPos(), arr[5].GetPos() );
		break;
		case IDC_TRACKBAR_AC_HIGH	:
			arr[i].UpdatePos( arr[8].GetPos(), arr[2].GetPos() );
		break;
		case IDC_TRACKBAR_AC_NORM	:
			arr[i].UpdatePos( arr[9].GetPos(), arr[7].GetPos() );
		break;
		case IDC_TRACKBAR_AC_LOW	:	
			arr[i].UpdatePos( arr[3].GetPos(), arr[8].GetPos() );
		break;

		default:
			break;
	}
}
DWORD CTrackbarsArr::GetBlValue( DWORD Id )
{
	DWORD dwLevel = BKL_MIN_SETTING;

	for( DWORD i = 0; i < 10; ++i )
	{
		if( Id == arr[i].Id() )
				break;
		}
		if( i > 9 )
			return dwLevel;

	dwLevel = BKL_TICK_INTERVAL * arr[i].GetPos();
    
	if(dwLevel < m_minPos)
		dwLevel = m_minPos;
	else if(dwLevel > BKL_MAX_SETTING)
		dwLevel = BKL_MAX_SETTING;
	
	RETAILMSG( 0, (_T("Level: %d\r\n"), dwLevel) ); 
	return dwLevel;
}


