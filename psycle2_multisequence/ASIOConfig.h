#if !defined(AFX_ASIOCONFIG_H__C1ED2948_8437_11D4_937A_E78B9D175439__INCLUDED_)
#define AFX_ASIOCONFIG_H__C1ED2948_8437_11D4_937A_E78B9D175439__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ASIOConfig.h : header file
//
#include "ASIOInterface.h"

/////////////////////////////////////////////////////////////////////////////
// CASIOConfig dialog

class CASIOConfig : public CDialog
{
// Construction
public:
	int m_sampleRate;
	int	m_bufferSize;
	CASIOConfig(CWnd* pParent = NULL);   // standard constructor
	ASIOInterface* pASIO;

// Dialog Data
	//{{AFX_DATA(CASIOConfig)
	enum { IDD = IDD_ASIO_CONFIG };
	CComboBox	m_driverComboBox;
	CStatic	m_latency;
	CComboBox	m_sampleRateCombo;
	CComboBox	m_bufferSizeCombo;
	int		m_driverIndex;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CASIOConfig)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	
	void RecalcLatency();
	void FillBufferBox();
	// Generated message map functions
	//{{AFX_MSG(CASIOConfig)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnSelendokSamplerate();
	afx_msg void OnSelendokBuffersize();
	afx_msg void OnDestroy();
	afx_msg void OnExclusive();
	afx_msg void OnControlPanel();
	afx_msg void OnSelchangeAsioDriver();
	afx_msg void OnBnClickedOk();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
};
//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ASIOCONFIG_H__C1ED2948_8437_11D4_937A_E78B9D175439__INCLUDED_)
