///\file
///\brief interface file for psycle::host::CMasterDlg.
#pragma once
#include <psycle/host/detail/project.hpp>
#include "Psycle.hpp"

namespace psycle {
namespace core {
	class Master;
}
namespace host {

		class CChildView;

		class CVolumeCtrl: public CSliderCtrl
		{
		public:
			CVolumeCtrl() : editing_(false), index_(-1) {}
			CVolumeCtrl(int index) : editing_(false), index_(index) {}

			bool editing() const { return editing_; }
			int index() const { return index_; }

		private:
			afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
			afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
			bool editing_;
			int index_;
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
			afx_msg void OnCustomdrawSliderm(NMHDR* pNMHDR, LRESULT* pResult);
			afx_msg void OnPaint();
			afx_msg void OnCancel();
			DECLARE_MESSAGE_MAP()
			afx_msg void OnStnClickedMixerview();

			enum { IDD = IDD_MASTERDLG };
			CStatic	m_masterpeak;
			CVolumeCtrl m_slidermaster;
			std::vector<CVolumeCtrl*> sliders_;
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
