///\file
///\brief interface file for psycle::host::CDefaultVstGui.
#pragma once
#include <psycle/engine/VSTHost.hpp>
#include <psycle/host/ChildView.hpp>
#include <afxext.h>
UNIVERSALIS__COMPILER__NAMESPACE__BEGIN(psycle)
	UNIVERSALIS__COMPILER__NAMESPACE__BEGIN(host)
/// default vst gui window.
		class CDefaultVstGui : public CFormView
		{
		protected:
			CDefaultVstGui(); // protected constructor used by dynamic creation
			DECLARE_DYNCREATE(CDefaultVstGui)
		// Form Data
		public:
			void Init();
			void UpdateOne();
			void UpdateParList();
			void UpdateText(float value);
			void UpdateNew(int par,float value);

			int MachineIndex;
			int nPar;
			int previousProg;
			vst::plugin * _pMachine;
			bool updatingvalue;
			CWnd* mainView;
			CChildView* childView;
			//{{AFX_DATA(CDefaultVstGui)
			enum { IDD = IDD_VSTRACK };
			CComboBox	m_program;
			CComboBox	m_combo;
			CSliderCtrl	m_slider;
			CStatic	m_text;
			CListBox	m_parlist;
			//}}AFX_DATA
		// Attributes
		public:
		// Operations
		public:
			void InitializePrograms(void);
			virtual ~CDefaultVstGui();
		// Overrides
			// ClassWizard generated virtual function overrides
			//{{AFX_VIRTUAL(CDefaultVstGui)
			protected:
			virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
			//}}AFX_VIRTUAL
		// Implementation
		protected:
		#if !defined NDEBUG
			virtual void AssertValid() const;
			virtual void Dump(CDumpContext& dc) const;
		#endif
			// Generated message map functions
			//{{AFX_MSG(CDefaultVstGui)
			afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
			afx_msg void OnSelchangeList1();
			afx_msg void OnReleasedcaptureSlider1(NMHDR* pNMHDR, LRESULT* pResult);
			afx_msg void OnSelchangeCombo1();
			afx_msg void OnCloseupCombo1();
			afx_msg void OnDeltaposSpin1(NMHDR* pNMHDR, LRESULT* pResult);
			afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
			//}}AFX_MSG
			DECLARE_MESSAGE_MAP()
		};

		//{{AFX_INSERT_LOCATION}}
		// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
	UNIVERSALIS__COMPILER__NAMESPACE__END
UNIVERSALIS__COMPILER__NAMESPACE__END
