// SerialBuffer.h: interface for the CSerialBuffer class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SerialBuffer_H__FD164A5F_9F09_451B_8E42_180999B4DB9A__INCLUDED_)
#define AFX_SerialBuffer_H__FD164A5F_9F09_451B_8E42_180999B4DB9A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include <string>


class CSerialBuffer  
{
private:
	void	Init();
	void	ClearAndReset(HANDLE& hEventToReset);
public:
	inline void LockBuffer()	{ EnterCriticalSection(&m_csLock ); }
	inline void UnLockBuffer()	{ LeaveCriticalSection(&m_csLock ); } 
	 
	
	CSerialBuffer( );
	virtual ~CSerialBuffer();

	//---- public interface --
	void AddData( char ch );
	void AddData( std::string& szData );
	void AddData( std::string& szData, DWORD iLen );
	void AddData( char *strData, DWORD iLen );
	std::string GetData()	{ return m_szInternalBuffer; }

	void		Clear();
	DWORD		Read_N(std::string &szData, DWORD alCount, HANDLE& hEventToReset);
	inline size_t GetSize() {	return m_szInternalBuffer.size();	}
	inline BOOL IsEmpty()	{	return m_szInternalBuffer.size() == 0;	}

//	BOOL		Read_Upto(std::string &szData, char chTerm, DWORD  &alBytesRead, HANDLE &hEventToReset);
//	BOOL		Read_Available(std::string &szData, HANDLE &hEventToReset);
//	BOOL Read_Upto_FIX( std::string &szData, char chTerm, DWORD  &alBytesRead, HANDLE &hEventToReset);

private:
	std::string		m_szInternalBuffer;
	BOOL			m_abLockAlways;
	DWORD			m_iCurPos;
	DWORD			m_alBytesUnRead;
	CRITICAL_SECTION	m_csLock;
};

#endif // !defined(AFX_SerialBuffer_H__FD164A5F_9F09_451B_8E42_180999B4DB9A__INCLUDED_)
