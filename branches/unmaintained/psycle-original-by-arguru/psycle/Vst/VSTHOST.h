// VSTHOST.h : main header file for the VSTHOST application
//

#if !defined(AFX_VSTHOST_H__AEA9F7C5_5BE5_11D4_A87E_00C026101D9A__INCLUDED_)
#define AFX_VSTHOST_H__AEA9F7C5_5BE5_11D4_A87E_00C026101D9A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CVSTHOSTApp:
// See VSTHOST.cpp for the implementation of this class
//



class CVSTHOSTApp : public CWinApp
{
public:
	CVSTHOSTApp();


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CVSTHOSTApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CVSTHOSTApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VSTHOST_H__AEA9F7C5_5BE5_11D4_A87E_00C026101D9A__INCLUDED_)
