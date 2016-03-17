
#include "Sliders.h"
#include "keypadcpl.h"

CTrackbarsArr::CTrackbarsArr()
{
	RETAILMSG(0, (_T("CTrackbarsArr() %d, %d, %d\r\n"),LED_MIN_SETTING, LED_MAX_SETTING, LED_TICK_INTERVAL));

	for( int i = 0; i < NUM_OF_SLIDERS; ++ i )
	{
		arr[i].InitBase(	LED_MIN_SETTING,
							LED_MAX_SETTING,
							LED_TICK_INTERVAL,
							10,	//TICK_FREQ;
							1,	//LINE_SIZE;
							1	//PAGE_SIZE;
						);
	}
}
void CTrackbarsArr::Init( HWND hDlg, DWORD Id, DWORD Level )
{
	switch(Id)
	{
		case IDC_TRACKBAR_KEYPAD_HIGH	:	
			arr[0].Init( hDlg, Id, Level );
		break;
		case IDC_TRACKBAR_KEYPAD_NORM	:
			arr[1].Init( hDlg, Id, Level );
		break;
		case IDC_TRACKBAR_KEYPAD_LOW	:
			arr[2].Init( hDlg, Id, Level );
		break;
			
		default:
			break;
	}
}
//void CTrackbarsArr::UpdatePos()
//{
//	arr[4].UpdatePos( arr[0].GetPos(), arr[0].GetPos() );//=
//	arr[5].UpdatePos( arr[1].GetPos(), arr[0].GetPos() );
//	arr[6].UpdatePos( arr[1].GetPos(), arr[0].GetPos() );
//	
//	arr[7].UpdatePos( arr[2].GetPos(), arr[2].GetPos() );//=
//	arr[8].UpdatePos( arr[3].GetPos(), arr[2].GetPos() );
//	arr[9].UpdatePos( arr[3].GetPos(), arr[2].GetPos() );
//
//}
//
//void CTrackbarsArr::UpdatePos0( HWND hDlg1 )
//{
//	arr[0].UpdatePos( arr[1].GetPos(), ( hDlg1 ? arr[4].GetPos() : LED_MAX_SETTING ) );
//	arr[2].UpdatePos( arr[3].GetPos(), ( hDlg1 ? arr[7].GetPos() : LED_MAX_SETTING ) );
//}
//
void CTrackbarsArr::PosChanged( HWND hwnd )
{
	if( !hwnd )
		return;
	DWORD j = 0;

	for( DWORD i = 0; i < NUM_OF_SLIDERS; ++i )
	{
		if( arr[i] == hwnd  )
			break;
	}
	if( i >= NUM_OF_SLIDERS )
		return;
	RETAILMSG(0,(_T("PosChanged: hwnd %x, i = %d\r\n"), hwnd, i ));
	switch(arr[i].Id())
	{
		case IDC_TRACKBAR_KEYPAD_HIGH	:
			arr[0].UpdatePos( LED_MIN_SETTING, arr[1].GetPos() );
		break;
		case IDC_TRACKBAR_KEYPAD_NORM	:
			arr[1].UpdatePos( arr[0].GetPos(), arr[2].GetPos() );
		break;
		case IDC_TRACKBAR_KEYPAD_LOW	:
			arr[2].UpdatePos( arr[1].GetPos(), LED_MAX_SETTING );
		break;

		default:
			break;
	}
}
DWORD CTrackbarsArr::GetBlValue( DWORD Id )
{
	DWORD dwLevel = LED_MIN_SETTING;

	for( DWORD i = 0; i < 10; ++i )
	{
		if( Id == arr[i].Id() )
				break;
		}
		if( i > 9 )
			return dwLevel;

	dwLevel = LED_TICK_INTERVAL * arr[i].GetPos();
    
	if(dwLevel < LED_MIN_SETTING)
		dwLevel = LED_MIN_SETTING;
	else if(dwLevel > LED_MAX_SETTING)
		dwLevel = LED_MAX_SETTING;
	
	RETAILMSG( 0, (_T("Level: %d\r\n"), dwLevel) ); 
	return dwLevel;
}


