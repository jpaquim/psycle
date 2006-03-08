#pragma once
///\file
///\brief interface file for psycle::host::CDirectoryDlg.
namespace psycle
{
	namespace host
	{
		/// directory config window.
		class CDirectoryDlg : public CPropertyPage
		{
			DECLARE_DYNCREATE(CDirectoryDlg)
		public:
			bool _instPathChanged;
			char _instPathBuf[MAX_PATH];
			bool _songPathChanged;
			char _songPathBuf[MAX_PATH];
			bool _pluginPathChanged;
			char _pluginPathBuf[MAX_PATH];
			bool _vstPathChanged;
			char _vstPathBuf[MAX_PATH];
			bool _skinPathChanged;
			char _skinPathBuf[MAX_PATH];
			bool initializingDlg;
			CDirectoryDlg();
			~CDirectoryDlg();
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
			bool BrowseForFolder(char *rpath);
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
	}
}
