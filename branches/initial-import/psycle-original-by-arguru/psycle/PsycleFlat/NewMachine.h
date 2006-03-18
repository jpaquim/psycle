#if !defined(AFX_NEWMACHINE_H__92D8AD66_EA85_11D3_8913_EDB0A03AE064__INCLUDED_)
#define AFX_NEWMACHINE_H__92D8AD66_EA85_11D3_8913_EDB0A03AE064__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// NewMachine.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CNewMachine dialog

class CNewMachine : public CDialog
{
// Construction
public:
	CNewMachine(CWnd* pParent = NULL);   // standard constructor
	psySong* songRef;
	int xLoc;
	int yLoc;
	HTREEITEM hGen;
	HTREEITEM hSam;
	HTREEITEM hVstI;
	HTREEITEM hVstE;
	
	HTREEITEM hEfx[16];
	HTREEITEM hMis[MAX_BUZZFX_PLUGS];
	
	int Outputmachine;
	int Outputdll;
	int OutBus;

// Dialog Data
	//{{AFX_DATA(CNewMachine)
	enum { IDD = IDD_NEWMACHINE };
	CStatic	m_verslabel;
	CStatic	m_desclabel;
	CStatic	m_namelabel;
	CTreeCtrl	m_mactree;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNewMachine)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CNewMachine)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangedTree1(NMHDR* pNMHDR, LRESULT* pResult);
	virtual void OnOK();
	afx_msg void OnDblclkTree1(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NEWMACHINE_H__92D8AD66_EA85_11D3_8913_EDB0A03AE064__INCLUDED_)
