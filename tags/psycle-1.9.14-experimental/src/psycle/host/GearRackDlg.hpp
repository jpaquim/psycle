///\file
///\brief interface file for psycle::host::CGearRackDlg.
#pragma once
#include <psycle/host/ChildView.hpp>
UNIVERSALIS__COMPILER__NAMESPACE__BEGIN(psycle)
	UNIVERSALIS__COMPILER__NAMESPACE__BEGIN(host)
		/// gear rack window.
		class CGearRackDlg : public CDialog
		{
		public:
			void ExchangeIns(Instrument::id_type, Instrument::id_type);
			void ExchangeMacs(Machine::id_type, Machine::id_type);
			CGearRackDlg(CChildView* pParent, CMainFrame* pMain);
			CChildView* m_pParent;
			CMainFrame* pParentMain;
			void RedrawList();
			BOOL Create();
			afx_msg void OnCancel();
			static int DisplayMode;
		// Dialog Data
			//{{AFX_DATA(CGearRackDlg)
			enum { IDD = IDD_GEAR_RACK };
			CButton	m_props;
			CButton	m_radio_ins;
			CButton	m_radio_gen;
			CButton	m_radio_efx;
			CButton	m_text;
			CListBox	m_list;
			//}}AFX_DATA
		// Overrides
			// ClassWizard generated virtual function overrides
			//{{AFX_VIRTUAL(CGearRackDlg)
			protected:
			virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
			virtual BOOL OnInitDialog();
			virtual BOOL PreTranslateMessage(MSG* pMsg);
			//}}AFX_VIRTUAL
		// Implementation
		protected:
			// Generated message map functions
			//{{AFX_MSG(CGearRackDlg)
			afx_msg void OnCreate();
			afx_msg void OnDelete();
			afx_msg void OnDblclkGearlist();
			afx_msg void OnProperties();
			afx_msg void OnParameters();
			afx_msg void OnSelchangeGearlist();
			afx_msg void OnRadioEfx();
			afx_msg void OnRadioGen();
			afx_msg void OnRadioIns();
			afx_msg void OnExchange();
			afx_msg void OnClonemachine();
			//}}AFX_MSG
			DECLARE_MESSAGE_MAP()
		};

		//{{AFX_INSERT_LOCATION}}
		// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
	UNIVERSALIS__COMPILER__NAMESPACE__END
UNIVERSALIS__COMPILER__NAMESPACE__END