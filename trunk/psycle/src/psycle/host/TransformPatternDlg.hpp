// -*- mode:c++; indent-tabs-mode:t -*-
#pragma once
#include "resources/resources.hpp"
#include "mfc_namespace.hpp"
#include "ChildView.hpp"
#include "Song.hpp"
#include "Helpers.hpp"

// CTransformPatternDlg dialog
PSYCLE__MFC__NAMESPACE__BEGIN(psycle)
	PSYCLE__MFC__NAMESPACE__BEGIN(host)
		class CTransformPatternDlg : public CDialog
		{
			DECLARE_DYNAMIC(CTransformPatternDlg)

		private:
			CChildView* _pChildView;

		public:
			CTransformPatternDlg(CChildView* pChildView, CWnd* pParent = NULL);   // standard constructor
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
			int	m_applyto;
			CButton	m_applytosong;
			CButton	m_applytopattern;
			CButton	m_applytoblock;
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
			virtual BOOL OnInitDialog();
		//}}AFX_MSG
			DECLARE_MESSAGE_MAP()
		public:
			afx_msg void OnBnClickedApply();

		};


		//{{AFX_INSERT_LOCATION}}
		// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
	PSYCLE__MFC__NAMESPACE__END
PSYCLE__MFC__NAMESPACE__END
