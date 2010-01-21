///\file
///\brief interface file for psycle::host::CMasterDlg.
#pragma once

#include "Psycle.hpp"

#if PSYCLE__CONFIGURATION__USE_PSYCORE
namespace psycle {
	namespace core {
		class Master;
	}
}
using namespace psycle::core;
#endif



namespace psycle {
	namespace host {

		class CChildView;
#if !PSYCLE__CONFIGURATION__USE_PSYCORE
		class Master;
#endif

		class CVolumeCtrl: public CSliderCtrl
		{
		public:
			CVolumeCtrl() : editing_(false) {}

			bool editing() const { return editing_; }

		private:
			afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
			afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
			bool editing_;
			DECLARE_MESSAGE_MAP()
		};

		/// master machine window.
		class CMasterDlg : public CDialog
		{
		public:
			CMasterDlg(Master* master, CChildView * pParent);
			~CMasterDlg();

			void UpdateUI(void);
			void CenterWindowOnPoint(int x, int y);

			///\todo should be private
			char macname[MAX_CONNECTIONS][32];

		private:
			void PaintNumbers(float val, int x, int y);
			void PaintNumbersDC(CDC* dc,CDC* memDC,float val,int x,int y);
			LRESULT DrawSliderGraphics(NMHDR* pNMHDR);
			void PaintNames(char* name,int x,int y);
			void SetSliderValues();			
			virtual BOOL PreTranslateMessage(MSG* pMsg);
			virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
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
			afx_msg void OnCancel();
			DECLARE_MESSAGE_MAP()
			afx_msg void OnStnClickedMixerview();

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
			CBitmap m_numbers;
			CBitmap m_sliderknob;
			CBitmap m_back;
			CFont namesFont;
			Master* _pMachine;
			CChildView* m_pParent;			
		};

	}   // namespace host
}   // namespace psycle
