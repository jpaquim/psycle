#if !defined(AFX_SWINGFILLDLG_H__4108AB3F_F019_484E_8177_E98B6637C612__INCLUDED_)
#define AFX_SWINGFILLDLG_H__4108AB3F_F019_484E_8177_E98B6637C612__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SwingFillDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSwingFillDlg dialog

class CSwingFillDlg : public CDialog
{
// Construction
public:
	CSwingFillDlg(CWnd* pParent = NULL);   // standard constructor
	BOOL bGo;
	int tempo;
	int width;
	float variance;
	float phase;

// Dialog Data
	//{{AFX_DATA(CSwingFillDlg)
	enum { IDD = IDD_SWINGFILL };
	CEdit	m_Tempo;
	CEdit	m_Width;
	CEdit	m_Variance;
	CEdit	m_Phase;
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSwingFillDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSwingFillDlg)
		// NOTE: the ClassWizard will add member functions here
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SWINGFILLDLG_H__4108AB3F_F019_484E_8177_E98B6637C612__INCLUDED_)
