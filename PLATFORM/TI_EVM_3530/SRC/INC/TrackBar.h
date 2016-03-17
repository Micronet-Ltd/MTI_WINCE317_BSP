#ifndef	__TRACKBAR_H
#define	__TRACKBAR_H

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>

class CTrackbar
{
public:
	CTrackbar():	m_hwnd(0), m_Id(0), m_pos(0),
					MIN_SETTING(0),		
					MAX_SETTING(100),		
					TICK_INTERVAL(1),	
					TICK_FREQ(1),		
					LINE_SIZE(1),		
					PAGE_SIZE(1)		
	{}

	CTrackbar(	DWORD	min_setting,
				DWORD	max_setting,
				DWORD	tick_interval,
				DWORD	tick_freq,
				DWORD	line_size,
				DWORD	page_size		) : m_hwnd(0), m_Id(0), m_pos(0) 
	{
		InitBase(min_setting,
				 max_setting,
				 tick_interval,
				 tick_freq,
				 line_size,
				 page_size		);

	}
	void InitBase(	DWORD	min_setting,
					DWORD	max_setting,
					DWORD	tick_interval,
					DWORD	tick_freq,
					DWORD	line_size,
					DWORD	page_size		)
	{
		MIN_SETTING		= min_setting;
		MAX_SETTING		= max_setting;
		TICK_INTERVAL	= tick_interval;
		TICK_FREQ		= tick_freq;
		LINE_SIZE		= line_size;
		PAGE_SIZE		= page_size;		
	}

	void Init( HWND hParent, DWORD Id, DWORD pos ) 
	{
		if( hParent )
		{
			m_hwnd = GetDlgItem(hParent, Id);
			if( m_hwnd )
			{
				m_Id = Id;
				InitSlider( m_hwnd, pos );
			}
			RETAILMSG(0,(_T("Init: hwnd %x, Id = %d pos = %d\r\n"), m_hwnd, m_Id, m_pos ));

		}
	}
	void UpdatePos( DWORD MinPos, DWORD MaxPos )
	{
		m_pos = SendMessage(m_hwnd, TBM_GETPOS, 0, 0 );
		
		if( m_pos > MaxPos )
		{
			SendMessage( m_hwnd, TBM_SETPOS, TRUE, MaxPos );
			m_pos = MaxPos;
		}
		else if( m_pos < MinPos )
		{
			SendMessage( m_hwnd, TBM_SETPOS, TRUE, MinPos );
			m_pos = MinPos;
		}
	}

	DWORD	GetPos( BOOL fUpdate =  0 )				
	{
		if( fUpdate )
			m_pos = SendMessage(m_hwnd, TBM_GETPOS, 0, 0 );

		return m_pos; 
	};
	
	BOOL	operator==( HWND hwnd ) { return m_hwnd == hwnd; }
	DWORD	Id()					{ return m_Id;	};

void InitSlider(HWND hwndSlider, DWORD Level )
{
	if( hwndSlider )
	{
		if(Level < MIN_SETTING)
			Level = MIN_SETTING;
		else if(Level > MAX_SETTING)
			Level = MAX_SETTING;

		SendMessage(hwndSlider, TBM_SETRANGE, TRUE, MAKELONG(MIN_SETTING/TICK_INTERVAL, MAX_SETTING/TICK_INTERVAL));
		SendMessage(hwndSlider, TBM_SETTICFREQ, TICK_FREQ, 0L);
		SendMessage(hwndSlider, TBM_SETLINESIZE, 0L, LINE_SIZE);
		SendMessage(hwndSlider, TBM_SETPAGESIZE, 0L, PAGE_SIZE);

		SendMessage(hwndSlider, TBM_SETPOS, TRUE, (Level / TICK_INTERVAL));
	
		m_pos = Level / TICK_INTERVAL;
	}
}
private:

	HWND	m_hwnd;
	DWORD	m_Id;
	DWORD	m_pos;

//working params
	DWORD	MIN_SETTING;
	DWORD	MAX_SETTING;
	DWORD	TICK_INTERVAL;
	DWORD	TICK_FREQ;
	DWORD	LINE_SIZE;
	DWORD	PAGE_SIZE;
};




#endif //	__TRACKBAR_H