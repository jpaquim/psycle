///\file
///\brief interface file for psycle::host::CPatDlg.
#pragma once
#include "Psycle.hpp"


#if PSYCLE__CONFIGURATION__USE_PSYCORE
namespace psycle { 
			namespace core {
				class Pattern;
			}
		}
#endif

PSYCLE__MFC__NAMESPACE__BEGIN(psycle)
	PSYCLE__MFC__NAMESPACE__BEGIN(host)

		/// pattern window.
		class CPatDlg : public CDialog
		{
		public:
#if PSYCLE__CONFIGURATION__USE_PSYCORE
			CPatDlg(CWnd* pParent, psycle::core::Pattern* pattern);
#else
			CPatDlg(CWnd* pParent);
#endif
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
#if PSYCLE__CONFIGURATION__USE_PSYCORE
			psycle::core::Pattern* pattern_;
#endif
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

	PSYCLE__MFC__NAMESPACE__END
PSYCLE__MFC__NAMESPACE__END
