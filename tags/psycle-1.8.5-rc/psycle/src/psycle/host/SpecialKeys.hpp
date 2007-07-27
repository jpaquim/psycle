///\file
///\brief interface file for psycle::host::CKeyConfigDlg.
#pragma once
#include <afxwin.h>
#include "resources/resources.hpp"
#include "mfc_namespace.hpp"
PSYCLE__MFC__NAMESPACE__BEGIN(psycle)
PSYCLE__MFC__NAMESPACE__BEGIN(host)

/// key config window.
class CSpecialKeys : public CDialog
{
public:
	CSpecialKeys(CWnd* pParent = NULL);   // standard constructor
	enum { IDD = IDD_SPECIALKEYS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnBnClickedOk();
	int key;
	int mod;
};

PSYCLE__MFC__NAMESPACE__END
PSYCLE__MFC__NAMESPACE__END
