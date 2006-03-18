#if !defined(AFX_CONTROLDESK_H__92D8AD60_EA85_11D3_8913_EDB0A03AE064__INCLUDED_)
#define AFX_CONTROLDESK_H__92D8AD60_EA85_11D3_8913_EDB0A03AE064__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ControlDesk.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CControlDesk dialog

class CControlDesk : public CDialog
{
// Construction
public:
	CControlDesk(CWnd* pParent = NULL);   // standard constructor
	psySong *songRef;
	void UpdateLoops();
	void DoUpdate();	
	CChildView* ParentWnd;
	
// Dialog Data
	//{{AFX_DATA(CControlDesk)
	enum { IDD = IDD_CONTROLDESK };
	CStatic	m_loopend;
	CStatic	m_loopbegin;
	CButton	m_ploop;
	CStatic	m_playdisplay;
	CStatic	m_timeline;
	CStatic	m_bpmlabel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CControlDesk)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CControlDesk)
	afx_msg void OnButton1();
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeInstcombo();
	afx_msg void OnInstedit();
	afx_msg void OnPlaylooping();
	afx_msg void OnButton2();
	afx_msg void OnButton4();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CONTROLDESK_H__92D8AD60_EA85_11D3_8913_EDB0A03AE064__INCLUDED_)
