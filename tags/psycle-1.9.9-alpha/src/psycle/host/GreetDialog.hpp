#pragma once
///\file
///\brief interface file for psycle::host::CGreetDialog.
UNIVERSALIS__COMPILER__NAMESPACE__BEGIN(psycle)
	UNIVERSALIS__COMPILER__NAMESPACE__BEGIN(host)
		/// greeting window.
		class CGreetDialog : public CDialog
		{
		// Construction
		public:
			CGreetDialog(CWnd* pParent = NULL);   // standard constructor

		// Dialog Data
			//{{AFX_DATA(CGreetDialog)
			enum { IDD = IDD_GREETS };
			CListBox	m_greetz;
			//}}AFX_DATA


		// Overrides
			// ClassWizard generated virtual function overrides
			//{{AFX_VIRTUAL(CGreetDialog)
			protected:
			virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
			//}}AFX_VIRTUAL

		// Implementation
		protected:

			// Generated message map functions
			//{{AFX_MSG(CGreetDialog)
			virtual BOOL OnInitDialog();
			//}}AFX_MSG
			DECLARE_MESSAGE_MAP()
		};
		
		//{{AFX_INSERT_LOCATION}}
		// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
	UNIVERSALIS__COMPILER__NAMESPACE__END
UNIVERSALIS__COMPILER__NAMESPACE__END
