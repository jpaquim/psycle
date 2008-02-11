// -*- mode:c++; indent-tabs-mode:t -*-
///\file
///\brief interface file for psycle::host::CMasterDlg.
#pragma once
#include "Machine.hpp"
#include "constants.hpp"
#include "resources/resources.hpp"
#include "mfc_namespace.hpp"
PSYCLE__MFC__NAMESPACE__BEGIN(psycle)
	PSYCLE__MFC__NAMESPACE__BEGIN(host)

		class CChildView;


		class CVolumeCtrl: public CSliderCtrl
		{
		public:
			CVolumeCtrl():CSliderCtrl(),editing(false){}

			DECLARE_MESSAGE_MAP()
		public:
			afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
			afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
			bool editing;
		};

		/// master machine window.
		class CMasterDlg : public CDialog
		{
		public:
			CMasterDlg(CChildView * pParent);
			BOOL Create();
			void PaintNumbers(float val, int x, int y);
			void PaintNumbersDC(CDC* dc,CDC* memDC,float val,int x,int y);
			LRESULT DrawSliderGraphics(NMHDR* pNMHDR);
			void PaintNames(char* name,int x,int y);
			void SetSliderValues();
			void UpdateUI(void);
			CBitmap m_numbers;
			CBitmap m_sliderknob;
			CBitmap m_back;
			CFont namesFont;
			Master* _pMachine;
			char macname[MAX_CONNECTIONS][32];
			afx_msg void OnCancel();
		// Dialog Data
			//{{AFX_DATA(CMasterDlg)
			enum { IDD = IDD_MASTERDLG };
			CStatic	m_masterpeak;
			CVolumeCtrl	m_slidermaster;
			CVolumeCtrl	m_sliderm9;
			CVolumeCtrl	m_sliderm8;
			CVolumeCtrl	m_sliderm7;
			CVolumeCtrl	m_sliderm6;
			CVolumeCtrl	m_sliderm5;
			CVolumeCtrl	m_sliderm4;
			CVolumeCtrl	m_sliderm3;
			CVolumeCtrl	m_sliderm2;
			CVolumeCtrl	m_sliderm12;
			CVolumeCtrl	m_sliderm11;
			CVolumeCtrl	m_sliderm10;
			CVolumeCtrl	m_sliderm1;
			CStatic	m_mixerview;
			CButton	m_autodec;
			//}}AFX_DATA
		// Overrides
			// ClassWizard generated virtual function overrides
			//{{AFX_VIRTUAL(CMasterDlg)
			public:
			virtual BOOL PreTranslateMessage(MSG* pMsg);
			protected:
			virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
			//}}AFX_VIRTUAL
		// Implementation
		protected:
			CChildView* m_pParent;
			// Generated message map functions
			//{{AFX_MSG(CMasterDlg)
			virtual BOOL OnInitDialog();
			afx_msg void OnAutodec();
			afx_msg void OnCustomdrawSlidermaster(NMHDR* pNMHDR, LRESULT* pResult);
			afx_msg void OnCustomdrawSliderm1(NMHDR* pNMHDR, LRESULT* pResult);
			afx_msg void OnCustomdrawSliderm10(NMHDR* pNMHDR, LRESULT* pResult);
			afx_msg void OnCustomdrawSliderm11(NMHDR* pNMHDR, LRESULT* pResult);
			afx_msg void OnCustomdrawSliderm12(NMHDR* pNMHDR, LRESULT* pResult);
			afx_msg void OnCustomdrawSliderm2(NMHDR* pNMHDR, LRESULT* pResult);
			afx_msg void OnCustomdrawSliderm3(NMHDR* pNMHDR, LRESULT* pResult);
			afx_msg void OnCustomdrawSliderm4(NMHDR* pNMHDR, LRESULT* pResult);
			afx_msg void OnCustomdrawSliderm5(NMHDR* pNMHDR, LRESULT* pResult);
			afx_msg void OnCustomdrawSliderm6(NMHDR* pNMHDR, LRESULT* pResult);
			afx_msg void OnCustomdrawSliderm7(NMHDR* pNMHDR, LRESULT* pResult);
			afx_msg void OnCustomdrawSliderm8(NMHDR* pNMHDR, LRESULT* pResult);
			afx_msg void OnCustomdrawSliderm9(NMHDR* pNMHDR, LRESULT* pResult);
			afx_msg void OnPaint();
			//}}AFX_MSG
			DECLARE_MESSAGE_MAP()
		public:
			afx_msg void OnStnClickedMixerview();
		};

		//{{AFX_INSERT_LOCATION}}
		// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

	PSYCLE__MFC__NAMESPACE__END
PSYCLE__MFC__NAMESPACE__END
