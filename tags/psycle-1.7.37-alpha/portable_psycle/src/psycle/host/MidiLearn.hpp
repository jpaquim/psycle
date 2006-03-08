///\file
///\brief interface file for psycle::host::CMidiLearn.
#pragma once
NAMESPACE__BEGIN(psycle)
	NAMESPACE__BEGIN(host)
		/// midi learn.
		class CMidiLearn : public CDialog
		{
		// Construction
		public:
			CMidiLearn(CWnd* pParent = 0);
			int Message;
			void __cdecl MidiCallback_Test( HMIDIIN handle, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2 );
		// Dialog Data
			//{{AFX_DATA(CMidiLearn)
			enum { IDD = IDD_MIDI_LEARN };
			CStatic		m_Text;
				// NOTE: the ClassWizard will add data members here
			//}}AFX_DATA
		// Overrides
			// ClassWizard generated virtual function overrides
			//{{AFX_VIRTUAL(CMidiLearn)
			protected:
			virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
			//}}AFX_VIRTUAL
		// Implementation
		protected:
			// Generated message map functions
			//{{AFX_MSG(CMidiLearn)
			virtual void OnCancel();
			//}}AFX_MSG
			DECLARE_MESSAGE_MAP()
		};

		//{{AFX_INSERT_LOCATION}}
		// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
	NAMESPACE__END
NAMESPACE__END
