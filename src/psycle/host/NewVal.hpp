///\file
///\brief interface file for psycle::host::CNewVal.
#pragma once
NAMESPACE__BEGIN(psycle)
	NAMESPACE__BEGIN(host)
		/// parameter value window.
		class CNewVal : public CDialog
		{
		public:
			CNewVal(int mindex,int pindex, int vval, int vmin, int vmax,char *title,CWnd* pParent = 0);
			int m_Value;
		protected:
			int v_min;
			int v_max;
			int macindex;
			int paramindex;
			char dlgtitle[256];
			enum { IDD = IDD_NEWVAL };
			CEdit	m_value;
			CStatic m_text;
		// Overrides
		protected:
			virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
		// Implementation
		protected:
			virtual BOOL OnInitDialog();
			afx_msg void OnUpdateEdit1();
			DECLARE_MESSAGE_MAP()
		};

	NAMESPACE__END
NAMESPACE__END
