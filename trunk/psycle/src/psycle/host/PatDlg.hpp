///\file
///\brief interface file for psycle::host::CPatDlg.
#pragma once
#include "Psycle.hpp"

namespace psycle { 
namespace core {
	class Pattern;
}
namespace host {

		/// pattern window.
		class CPatDlg : public CDialog
		{
		public:
			CPatDlg(CWnd* pParent, Pattern* pattern);
			int patLines;
			char patName[32];
		// Dialog Data
			//{{AFX_DATA(CPatDlg)
			enum { IDD = IDD_PATDLG };
			CButton	m_adaptsizeCheck;
			CEdit	m_numlines;
			CEdit	m_patname;
			CSpinButtonCtrl	m_spinlines;
			BOOL	m_adaptsize;
			CStatic m_text;
			//}}AFX_DATA
		// Overrides
			// ClassWizard generated virtual function overrides
			//{{AFX_VIRTUAL(CPatDlg)
			protected:
			virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
			//}}AFX_VIRTUAL
		// Implementation
		protected:
			BOOL bInit;
			Pattern* pattern_;
			// Generated message map functions
			//{{AFX_MSG(CPatDlg)
			virtual BOOL OnInitDialog();
			virtual void OnOK();
			afx_msg void OnCheck1();
			afx_msg void OnUpdateNumLines();
			//}}AFX_MSG
			DECLARE_MESSAGE_MAP()
		};

		//{{AFX_INSERT_LOCATION}}
		// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
}}
