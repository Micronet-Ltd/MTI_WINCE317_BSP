// EbootLoaderDlg.h : header file
//

#pragma once
#include "afxwin.h"


// CEbootLoaderDlg dialog
class CEbootLoaderDlg : public CDialog
{
	DECLARE_DYNAMIC(CEbootLoaderDlg)
// Construction
public:
	CEbootLoaderDlg(CWnd* pParent = NULL);	// standard constructor
	virtual ~CEbootLoaderDlg();

// Dialog Data
	enum { IDD = IDD_EBOOTLOADER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;
	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButton3();
	afx_msg void OnBnClickedButton2();
	CStatic m_Status;
	CEdit m_catalog;
	CEdit m_devid;
	CEdit m_ser;
	afx_msg void OnBnClickedButton4();
	afx_msg void OnBnClickedButton5();
	CEdit m_mac1;
	afx_msg void OnBnClickedButton6();
	CEdit m_mac2;
	CEdit m_mac3;
	CEdit m_mac4;
	CEdit m_mac5;
	CEdit m_mac6;
	afx_msg void OnBnClickedButton7();
	afx_msg void OnBnClickedButton8();
	afx_msg void OnBnClickedButton9();
	CEdit m_catalog2;
	CEdit m_devid2;
	CEdit m_ser2;
	CStatic m_ImgVer;
	CStatic m_EbootVer;
	CStatic m_XldrVer;
	CStatic m_HwVer;
	afx_msg void OnBnClickedButton10();
	CEdit m_SwRights;
	afx_msg void OnBnClickedButton11();
	afx_msg void OnBnClickedButton12();
	afx_msg void OnBnClickedButton13();
	afx_msg void OnBnClickedButton14();
	CComboBox m_Com;
	afx_msg void OnBnClickedButton15();
	CComboBox m_NetType;
	afx_msg void OnBnClickedButton16();
	afx_msg void OnBnClickedButton17();
	afx_msg void OnBnClickedButton18();
	CComboBox m_FileType;
	afx_msg void OnBnClickedButton19();
	afx_msg void OnBnClickedButton20();
	CComboBox m_ErType;
	CComboBox m_MaxFreq;
	CButton m_Erase;
	afx_msg void OnBnClickedButton22();
	afx_msg void OnBnClickedButton23();
	CStatic m_NandInfo;
};

