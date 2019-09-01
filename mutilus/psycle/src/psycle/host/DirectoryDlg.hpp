///\file
///\brief interface file for psycle::host::CDirectoryDlg.
#pragma once
#include "resources/resources.hpp"
#include "mfc_namespace.hpp"
PSYCLE__MFC__NAMESPACE__BEGIN(psycle)
	PSYCLE__MFC__NAMESPACE__BEGIN(host)

		/// directory config window.
		class CDirectoryDlg : public CPropertyPage
		{
			DECLARE_DYNCREATE(CDirectoryDlg)
		public:
			bool _instPathChanged;
			std::string _instPathBuf;
			bool _songPathChanged;
			std::string _songPathBuf;
			bool _pluginPathChanged;
			std::string _pluginPathBuf;
			bool _vstPathChanged;
			std::string _vstPathBuf;
			bool _skinPathChanged;
			std::string _skinPathBuf;
			bool initializingDlg;
			CDirectoryDlg();
			virtual ~CDirectoryDlg();
		// Dialog Data
			//{{AFX_DATA(CDirectoryDlg)
			enum { IDD = IDD_DIRECTORIES };
			CEdit	m_vstEdit;
			CEdit	m_pluginEdit;
			CEdit	m_songEdit;
			CEdit	m_instEdit;
			CEdit	m_skinEdit;
			//}}AFX_DATA
		// Overrides
			// ClassWizard generate virtual function overrides
			//{{AFX_VIRTUAL(CDirectoryDlg)
			protected:
			virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
			//}}AFX_VIRTUAL
		// Implementation
		protected:
			bool BrowseForFolder(std::string& rpath);
			// Generated message map functions
			//{{AFX_MSG(CDirectoryDlg)
			afx_msg void OnBrowseInst();
			afx_msg void OnBrowseSong();
			virtual BOOL OnInitDialog();
			afx_msg void OnBrowsePlugin();
			afx_msg void OnBrowseVst();
			afx_msg void OnChangeSongedit();
			afx_msg void OnChangeInstedit();
			afx_msg void OnChangePluginedit();
			afx_msg void OnChangeVstedit();
			afx_msg void OnBrowseSkin();
			afx_msg void OnChangeSkinedit();
			//}}AFX_MSG
			DECLARE_MESSAGE_MAP()
		};

		//{{AFX_INSERT_LOCATION}}
		// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

	PSYCLE__MFC__NAMESPACE__END
PSYCLE__MFC__NAMESPACE__END
