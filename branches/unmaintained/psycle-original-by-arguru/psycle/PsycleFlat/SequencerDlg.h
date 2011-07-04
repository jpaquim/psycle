#if !defined(AFX_SEQUENCERDLG_H__075CF2A0_4881_11D4_8913_D1DD5C2E1040__INCLUDED_)
#define AFX_SEQUENCERDLG_H__075CF2A0_4881_11D4_8913_D1DD5C2E1040__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SequencerDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSequencerDlg dialog

class CSequencerDlg : public CDialog
{
// Construction
public:
	UpdateSequencer();
	CSequencerDlg(CWnd* pParent = NULL);   // standard constructor
	psySong* songRef;
	
// Dialog Data
	//{{AFX_DATA(CSequencerDlg)
	enum { IDD = IDD_SEQUENCER };
	CStatic	m_seqview;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSequencerDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSequencerDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SEQUENCERDLG_H__075CF2A0_4881_11D4_8913_D1DD5C2E1040__INCLUDED_)
