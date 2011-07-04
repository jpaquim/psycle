///\file
///\brief interface file for psycle::host::CMasterDlg.
#pragma once

#include "Psycle.hpp"

namespace psycle {
namespace host {

		class Master;
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
			CMasterDlg(CChildView * pParent, Master* new_master);
			~CMasterDlg();

			void UpdateUI(void);
			void SetMachine(Master* newMac){ _pMachine = newMac; }

			///\todo should be private
			char macname[MAX_CONNECTIONS][32];

protected:
			void PaintNumbers(float val, int x, int y);
			void PaintNumbersDC(CDC* dc,CDC* memDC,float val,int x,int y);
			LRESULT DrawSliderGraphics(NMHDR* pNMHDR);
			void PaintNames(char* name,int x,int y);
			void SetSliderValues();
public:
			virtual BOOL PreTranslateMessage(MSG* pMsg);
			afx_msg void OnCancel();
protected:
			virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
			virtual BOOL OnInitDialog();
			afx_msg void OnAutodec();
			afx_msg void OnCustomdrawSlidermaster(NMHDR* pNMHDR, LRESULT* pResult);
			afx_msg void OnCustomdrawSliderm(NMHDR* pNMHDR, LRESULT* pResult);
			afx_msg void OnPaint();
			afx_msg void OnStnClickedMixerview();
			DECLARE_MESSAGE_MAP()

			enum { IDD = IDD_MASTERDLG };
			CStatic	m_masterpeak;
			CVolumeCtrl	m_slidermaster;
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
