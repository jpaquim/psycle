///\file
///\brief interface file for psycle::host::CSongpDlg.
#pragma once
NAMESPACE__BEGIN(psycle)
	NAMESPACE__BEGIN(host)
		class Song; // forward declaration

		/// song dialog window.
		class CSongpDlg : public CDialog
		{
		public:
			/// mfc compliant constructor.
			CSongpDlg(Song *song, CWnd* pParent = 0);
			Song* _pSong;
		// Dialog Data
			enum { IDD = IDD_SONGPROP };
			CEdit	m_songcomments;
			CEdit	m_songcredits;
			CEdit	m_songtitle;
			bool	readonlystate;
		// Overrides
		protected:
			virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
		// Implementation
		public:
			void SetReadOnly();
		protected:
			// Generated message map functions
			virtual BOOL OnInitDialog();
			afx_msg void OnOk();
			DECLARE_MESSAGE_MAP()
		};

	NAMESPACE__END
NAMESPACE__END
