///\file
///\brief interface file for psycle::host::CGearTracker.
#pragma once
#include "Sampler.hpp"
#include "constants.hpp"
#include "resources/resources.hpp"
#include "mfc_namespace.hpp"
PSYCLE__MFC__NAMESPACE__BEGIN(psycle)
	PSYCLE__MFC__NAMESPACE__BEGIN(host)

		class CChildView;

		/// gear tracker window.
		class CGearTracker : public CDialog
		{
		public:
			CGearTracker(CChildView* pParent = 0);
			Sampler* _pMachine;
			BOOL Create();
			afx_msg void OnCancel();
		// Dialog Data
			//{{AFX_DATA(CGearTracker)
			enum { IDD = IDD_GEAR_TRACKER };
			CComboBox	m_interpol;
			CSliderCtrl	m_polyslider;
			CStatic	m_polylabel;
			//}}AFX_DATA
		// Overrides
			// ClassWizard generated virtual function overrides
			//{{AFX_VIRTUAL(CGearTracker)
			protected:
			virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
			//}}AFX_VIRTUAL
		// Implementation
		protected:
			CChildView* m_pParent;
			// Generated message map functions
			//{{AFX_MSG(CGearTracker)
			virtual BOOL OnInitDialog();
			afx_msg void OnCustomdrawTrackslider(NMHDR* pNMHDR, LRESULT* pResult);
			afx_msg void OnCustomdrawTrackslider2(NMHDR* pNMHDR, LRESULT* pResult);
			afx_msg void OnSelchangeCombo1();
			//}}AFX_MSG
			DECLARE_MESSAGE_MAP()
		};

		//{{AFX_INSERT_LOCATION}}
		// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

	PSYCLE__MFC__NAMESPACE__END
PSYCLE__MFC__NAMESPACE__END
