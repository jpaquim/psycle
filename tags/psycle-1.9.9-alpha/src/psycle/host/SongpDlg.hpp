///\file
///\brief interface file for psycle::host::CSongpDlg.
#pragma once
UNIVERSALIS__COMPILER__NAMESPACE__BEGIN(psycle)
	UNIVERSALIS__COMPILER__NAMESPACE__BEGIN(host)
		class Song; // forward declaration

		/// song dialog window.
		class CSongpDlg : public CDialog
		{
		public:
			/// mfc compliant constructor.
			CSongpDlg(CWnd* pParent = 0);
			Song* _pSong;
		// Dialog Data
			//{{AFX_DATA(CSongpDlg)
			enum { IDD = IDD_SONGPROP };
			CEdit	m_songcomments;
			CEdit	m_songcredits;
			CEdit	m_songtitle;
			//}}AFX_DATA
		// Overrides
			// ClassWizard generated virtual function overrides
			//{{AFX_VIRTUAL(CSongpDlg)
			protected:
			virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
			//}}AFX_VIRTUAL
		// Implementation
		protected:
			// Generated message map functions
			//{{AFX_MSG(CSongpDlg)
			virtual BOOL OnInitDialog();
			afx_msg void OnChangeTitle();
			afx_msg void OnChangeAuthor();
			afx_msg void OnChangeComment();
			afx_msg void OnOk();
			//}}AFX_MSG
			DECLARE_MESSAGE_MAP()
		};

		//{{AFX_INSERT_LOCATION}}
		// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

	UNIVERSALIS__COMPILER__NAMESPACE__END
UNIVERSALIS__COMPILER__NAMESPACE__END
