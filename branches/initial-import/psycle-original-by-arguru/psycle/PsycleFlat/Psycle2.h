// Psycle2.h : main header file for the PSYCLE2 application
//

#if !defined(AFX_PSYCLE2_H__FE0D36E4_E90A_11D3_8913_9F3AED8AB763__INCLUDED_)
#define AFX_PSYCLE2_H__FE0D36E4_E90A_11D3_8913_9F3AED8AB763__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CPsycle2App:
// See Psycle2.cpp for the implementation of this class
//

class CPsycle2App : public CWinApp
{
public:
	CPsycle2App();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPsycle2App)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

public:
	//{{AFX_MSG(CPsycle2App)
	afx_msg void OnAboutpsycle();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PSYCLE2_H__FE0D36E4_E90A_11D3_8913_9F3AED8AB763__INCLUDED_)
