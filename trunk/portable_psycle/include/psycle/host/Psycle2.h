#pragma once
// Psycle.h : main header file for the Psycle application
//

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
