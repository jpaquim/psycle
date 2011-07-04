#if !defined(AFX_WAVEEDAMPLIFYDIALOG_H__6A36C2F3_B9B5_4246_9B1E_5B0395AB70C9__INCLUDED_)
#define AFX_WAVEEDAMPLIFYDIALOG_H__6A36C2F3_B9B5_4246_9B1E_5B0395AB70C9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define AMP_DIALOG_CANCEL -10000

class CWaveEdAmplifyDialog : public CDialog
{
// Construction
public:
	CWaveEdAmplifyDialog(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CWaveEdAmplifyDialog)
	enum { IDD = IDD_WAVED_AMPLIFY };
	CEdit	m_dbedit;
	CEdit	m_edit;
	CSliderCtrl	m_slider;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWaveEdAmplifyDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CWaveEdAmplifyDialog)
	virtual BOOL OnInitDialog();
	afx_msg void OnCustomdrawSlider(NMHDR* pNMHDR, LRESULT* pResult);
	virtual void OnOK();
	virtual void OnCancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_WAVEEDAMPLIFYDIALOG_H__6A36C2F3_B9B5_4246_9B1E_5B0395AB70C9__INCLUDED_)
