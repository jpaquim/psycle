///\file
///\brief interface file for psycle::host::CMasterDlg.
#pragma once
#include <psycle/host/detail/project.hpp>
#include "Psycle.hpp"

namespace psycle {
namespace host {

		class Master;

		class CVolumeCtrl: public CSliderCtrl
		{
		public:
			CVolumeCtrl(int index) : index_(index) {}

			int index() const { return index_; }

		private:
			int index_;
		};

		class CMasterVu: public CProgressCtrl
		{
		public:
			CMasterVu();
			virtual ~CMasterVu();

			void LoadBitmap(UINT IDControl);
			CBitmap m_vu;
			CBitmap* m_pback;

			DECLARE_MESSAGE_MAP()
			afx_msg BOOL OnEraseBkgnd(CDC* pDC);
			afx_msg void OnPaint();

		};

		/// master machine window.
		class CMasterDlg : public CDialog
		{
		public:
			CMasterDlg(CWnd* m_wndView, Master& new_master, CMasterDlg** windowVar);
			virtual ~CMasterDlg();

			void UpdateUI(void);

			///\todo should be private
			char macname[MAX_CONNECTIONS][32];

protected:
			void PaintNumbersDC(CDC* dc,float val,int x,int y);
			LRESULT DrawSliderGraphics(NMHDR* pNMHDR);
			void PaintNames(char* name,int x,int y);
			void SetSliderValues();
			void OnChangeSliderMaster(int pos);
			void OnChangeSliderMacs(CVolumeCtrl* slider);
public:
			virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
			virtual BOOL OnInitDialog();
			virtual BOOL PreTranslateMessage(MSG* pMsg);
			virtual void OnCancel();
			virtual void PostNcDestroy();
protected:
			DECLARE_MESSAGE_MAP()
			afx_msg void OnClose();
			afx_msg void OnAutodec();
			afx_msg BOOL OnEraseBkgnd(CDC* pDC);
			afx_msg void OnCustomdrawSlidermaster(NMHDR* pNMHDR, LRESULT* pResult);
			afx_msg void OnCustomdrawSliderm(UINT idx, NMHDR* pNMHDR, LRESULT* pResult);
			afx_msg void OnPaint();
			afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);

			enum { IDD = IDD_MASTERDLG };
			CStatic	m_masterpeak;
			CVolumeCtrl	m_slidermaster;
			std::vector<CVolumeCtrl*> sliders_;
			CMasterVu m_vuCtrl;
			CButton	m_autodec;
			CBitmap m_sliderknob;
			CBitmap m_back;
			int m_nBmpWidth;
			int m_nBmpHeight;

			CFont namesFont;
			Master& machine;
			CMasterDlg** windowVar_;
			CWnd* mainView;
		};

	}   // namespace host
}   // namespace psycle
