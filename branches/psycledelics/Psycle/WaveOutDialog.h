#if !defined(AFX_ADRIVERDIALOG_H__FE0D36F1_E90A_11D3_8913_9F3AED8AB763__INCLUDED_)
#define AFX_ADRIVERDIALOG_H__FE0D36F1_E90A_11D3_8913_9F3AED8AB763__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// Adriver header
//

/////////////////////////////////////////////////////////////////////////////
// CWaveOutDialog dialog

class CWaveOutDialog : public CDialog
{
// Construction
public:
	CWaveOutDialog(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CWaveOutDialog)
	enum { IDD = IDD_WAVEOUTCONFIG };
	CStatic	m_Latency;
	CSpinButtonCtrl	m_BufSizeSpin;
	CSpinButtonCtrl	m_BufNumSpin;
	CEdit	m_BufSizeEdit;
	CEdit	m_BufNumEdit;
	CButton	m_DitherCheck;
	CComboBox	m_DeviceList;
	CComboBox	m_SampleRateBox;
	int		m_BufNum;
	int		m_BufSize;
	int		m_Device;
	BOOL	m_Dither;
	//}}AFX_DATA

	int m_SampleRate;


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWaveOutDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CWaveOutDialog)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnChangeConfigBufnum();
	afx_msg void OnChangeConfigBufsize();
	afx_msg void OnSelendokConfigSamplerate();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	void RecalcLatency();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CONFIGDLG_H__8E063812_97DE_11D1_A298_2CC769000000__INCLUDED_)
