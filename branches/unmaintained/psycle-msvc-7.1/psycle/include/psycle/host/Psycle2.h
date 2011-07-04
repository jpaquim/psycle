// Psycle.h : main header file for the Psycle application
//

#if !defined(AFX_Psycle_H__FE0D36E4_E90A_11D3_8913_9F3AED8AB763__INCLUDED_)
#define AFX_Psycle_H__FE0D36E4_E90A_11D3_8913_9F3AED8AB763__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CPsycleApp:
// See Psycle.cpp for the implementation of this class
//

class CMainFrame;

class CPsycleApp : public CWinApp
{
public:
	CPsycleApp();
	virtual ~CPsycleApp() throw();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPsycleApp)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	//}}AFX_VIRTUAL

// Implementation
private:
	void ProcessCmdLine(CMainFrame*);
	void LoadRecent(CMainFrame*);
public:
	void SaveRecent(CMainFrame*);	//Called from CMainFrm::OnClose()!
									//That's why it's public :(
		
public:
	Global _global;

	//{{AFX_MSG(CPsycleApp)
	afx_msg void OnAboutpsycle();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};




/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_Psycle_H__FE0D36E4_E90A_11D3_8913_9F3AED8AB763__INCLUDED_)
