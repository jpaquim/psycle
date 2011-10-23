///\file
///\brief interface file for psycle::host::CKeyConfigDlg.
#pragma once
#include <psycle/host/detail/project.hpp>
#include "Psycle.hpp"
#include <afxwin.h>

namespace psycle { namespace host {

/// key config window.
class CSpecialKeys : public CDialog {
	public:
		CSpecialKeys(CWnd* pParent = 0); // standard constructor
		enum { IDD = IDD_SPECIALKEYS };

	protected:
		virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support
		virtual BOOL OnInitDialog();
		DECLARE_MESSAGE_MAP()

	public:
		afx_msg void OnBnClickedOk();
		int key;
		int mod;
};

}}
