#if !defined(AFX_VSTEDITORDLG_H__44F25588_ACD9_11D4_937A_BEE48B868538__INCLUDED_)
#define AFX_VSTEDITORDLG_H__44F25588_ACD9_11D4_937A_BEE48B868538__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// VstEditorDlg.h : header file
//

#include "VSTHost.h"
#include "DefaultVstGui.h"


/////////////////////////////////////////////////////////////////////////////
// CVstEditorDlg frame

class CVstEditorDlg : public CFrameWnd
{
public:
	DECLARE_DYNCREATE(CVstEditorDlg)
	CVstEditorDlg(int n) {};
protected:
	CVstEditorDlg();           // protected constructor used by dynamic creation

// Attributes
public:
	CWnd *pGui;
	CDefaultVstGui *pParamGui;
	VSTPlugin* _pMachine;
	CChildView* wndView;
	int index;
	bool *_editorActive;

// Operations
public:
	void Resize(int w,int h);
	void Refresh();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CVstEditorDlg)
	protected:
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);
	//}}AFX_VIRTUAL

// Implementation
protected:
	bool editorgui;
	CSplitterWnd _splitter;
	virtual ~CVstEditorDlg();

	// Generated message map functions
	//{{AFX_MSG(CVstEditorDlg)
	afx_msg void OnParametersRandomparameters();
	afx_msg void OnParametersShowpreset();
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VSTEDITORDLG_H__44F25588_ACD9_11D4_937A_BEE48B868538__INCLUDED_)
