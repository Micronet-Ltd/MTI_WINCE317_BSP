// TransportNotify.h: interface for the CTransportNotify class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TRANSPORTNOTIFY_H__4A2E518E_AB70_4097_BDBE_4BA4EE6AA123__INCLUDED_)
#define AFX_TRANSPORTNOTIFY_H__4A2E518E_AB70_4097_BDBE_4BA4EE6AA123__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CTransportNotify  
{
public:
	CTransportNotify();
	virtual ~CTransportNotify();

	virtual void	OnRemove( HANDLE hPort )		{}
};

#endif // !defined(AFX_TRANSPORTNOTIFY_H__4A2E518E_AB70_4097_BDBE_4BA4EE6AA123__INCLUDED_)
