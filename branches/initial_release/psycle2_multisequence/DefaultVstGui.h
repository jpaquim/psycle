#if !defined(AFX_DEFAULTVSTGUI_H__44F25597_ACD9_11D4_937A_BEE48B868538__INCLUDED_)
#define AFX_DEFAULTVSTGUI_H__44F25597_ACD9_11D4_937A_BEE48B868538__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DefaultVstGui.h : header file
//

#include "VSTHost.h"
#include "ChildView.h"
/////////////////////////////////////////////////////////////////////////////
// CDefaultVstGui form view

#ifndef __AFXEXT_H__
#include <afxext.h>
#endif

class CDefaultVstGui : public CFormView
{
protected:
	CDefaultVstGui();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CDefaultVstGui)

// Form Data
public:
	void Init();
	void UpdateOne();
	void UpdateParList();
	void UpdateText(float value);
	void UpdateNew(int par,float value);

	int MachineIndex;
	int nPar;
	int previousProg;
	VSTPlugin* _pMachine;
	bool updatingvalue;
	CWnd* mainView;
	CChildView* childView;
	
	//{{AFX_DATA(CDefaultVstGui)
	enum { IDD = IDD_VSTRACK };
	CComboBox	m_program;
	CComboBox	m_combo;
	CSliderCtrl	m_slider;
	CStatic	m_text;
	CListBox	m_parlist;
	//}}AFX_DATA

// Attributes
public:

// Operations
public:
	void InitializePrograms(void);
	virtual ~CDefaultVstGui();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDefaultVstGui)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
	//{{AFX_MSG(CDefaultVstGui)
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnSelchangeList1();
	afx_msg void OnReleasedcaptureSlider1(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSelchangeCombo1();
	afx_msg void OnCloseupCombo1();
	afx_msg void OnDeltaposSpin1(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DEFAULTVSTGUI_H__44F25597_ACD9_11D4_937A_BEE48B868538__INCLUDED_)
