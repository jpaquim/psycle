#if !defined(AFX_CONFIGDLG_H__0F38D780_98AB_11D4_937A_B3D2C98D5E39__INCLUDED_)
#define AFX_CONFIGDLG_H__0F38D780_98AB_11D4_937A_B3D2C98D5E39__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ConfigDlg.h : header file
//

#include "DirectoryDlg.h"
#include "SkinDlg.h"
#include "OutputDlg.h"
#include "MidiInputDlg.h"
#include "Configuration.h"

/////////////////////////////////////////////////////////////////////////////
// CConfigDlg

class CConfigDlg : public CPropertySheet
{
	DECLARE_DYNAMIC(CConfigDlg)

// Construction
public:
	CConfigDlg(UINT nIDCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
	CConfigDlg(LPCTSTR pszCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);

// Attributes
public:
	CDirectoryDlg _dirDlg;
	CSkinDlg _skinDlg;
	COutputDlg _outputDlg;
	CMidiInputDlg _midiDlg;

// Operations
public:
	void Init(Configuration* pConfig);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CConfigDlg)
	public:
	virtual int DoModal();
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CConfigDlg();

	// Generated message map functions
protected:
	Configuration* _pConfig;
	//{{AFX_MSG(CConfigDlg)
 		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CONFIGDLG_H__0F38D780_98AB_11D4_937A_B3D2C98D5E39__INCLUDED_)
