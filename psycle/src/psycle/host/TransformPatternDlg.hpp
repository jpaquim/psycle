#pragma once
#include "resources/resources.hpp"
#include "mfc_namespace.hpp"

// CTransformPatternDlg dialog
PSYCLE__MFC__NAMESPACE__BEGIN(psycle)
	PSYCLE__MFC__NAMESPACE__BEGIN(host)
		class CTransformPatternDlg : public CDialog
		{
			DECLARE_DYNAMIC(CTransformPatternDlg)

		public:
			CTransformPatternDlg(CWnd* pParent = NULL);   // standard constructor
			virtual ~CTransformPatternDlg();

		// Dialog Data
			//{{AFX_DATA(CTransformPatternDlg)
			enum { IDD = IDD_TRANSFORMPATTERN };			
			CEdit	m_filternote;
			CEdit	m_filterins;
			CEdit	m_filtermac;
			CEdit	m_filtercmd;
			CEdit	m_replacenote;
			CEdit	m_replaceins;
			CEdit	m_replacemac;
			CEdit	m_replacecmd;
			BOOL	m_applytoblock;
			//}}AFX_DATA
		// Overrides
			// ClassWizard generated virtual function overrides
			//{{AFX_VIRTUAL(CTransformPatternDlg)
		protected:
			virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
			//}}AFX_VIRTUAL
		// Implementation
		protected:
		// Generated message map functions
		//{{AFX_MSG(CTransformPatternDlg)
			
		//}}AFX_MSG
			DECLARE_MESSAGE_MAP()
		public:
			afx_msg void OnBnClickedApply();
		};


		//{{AFX_INSERT_LOCATION}}
		// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
	PSYCLE__MFC__NAMESPACE__END
PSYCLE__MFC__NAMESPACE__END
