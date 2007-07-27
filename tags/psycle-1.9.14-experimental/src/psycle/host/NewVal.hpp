///\file
///\brief interface file for psycle::host::CNewVal.
#pragma once
UNIVERSALIS__COMPILER__NAMESPACE__BEGIN(psycle)
	UNIVERSALIS__COMPILER__NAMESPACE__BEGIN(host)
		/// parameter value window.
		class CNewVal : public CDialog
		{
		public:
			CNewVal(CWnd* pParent = 0);
			int m_Value;
			char title[1024];
			int min;
			int max;
			int macindex;
			int paramindex;
		// Dialog Data
			//{{AFX_DATA(CNewVal)
			enum { IDD = IDD_NEWVAL };
			CEdit	m_value;
			CStatic m_text;
			//}}AFX_DATA
		// Overrides
			// ClassWizard generated virtual function overrides
			//{{AFX_VIRTUAL(CNewVal)
			protected:
			virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
			//}}AFX_VIRTUAL
		// Implementation
		protected:
			// Generated message map functions
			//{{AFX_MSG(CNewVal)
			afx_msg void OnChangeEdit1();
			virtual BOOL OnInitDialog();
			virtual void OnOK();
			afx_msg void OnSetfocusEdit1();
			afx_msg void OnUpdateEdit1();
			//}}AFX_MSG
			DECLARE_MESSAGE_MAP()
		};

		//{{AFX_INSERT_LOCATION}}
		// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
	UNIVERSALIS__COMPILER__NAMESPACE__END
UNIVERSALIS__COMPILER__NAMESPACE__END
