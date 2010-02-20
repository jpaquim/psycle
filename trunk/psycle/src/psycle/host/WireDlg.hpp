///\file
///\brief interface file for psycle::host::CWireDlg.
#pragma once
#include "Psycle.hpp"

namespace psycle {
	namespace core {
		class Machine;
	}

	namespace host {
		using namespace core;

		class CChildView;
		class WireGui;

		const int MAX_SCOPE_BANDS = 128;
		const int SCOPE_BUF_SIZE = 4096;
		const int SCOPE_SPEC_SAMPLES = 1024;

		/// wire monitor window.
		class CWireDlg : public CDialog
		{
		public:
			CWireDlg(CChildView* pParent, WireGui* wire_gui);

		protected:
			virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support			
			afx_msg void OnCustomdrawSlider1(NMHDR* pNMHDR, LRESULT* pResult);
			afx_msg void OnButton1();
			afx_msg void OnTimer(UINT nIDEvent);
			afx_msg void OnCustomdrawSlider(NMHDR* pNMHDR, LRESULT* pResult);
			afx_msg void OnCustomdrawSlider2(NMHDR* pNMHDR, LRESULT* pResult);
			afx_msg void OnMode();
			afx_msg void OnHold();
			afx_msg void OnVolumeDb();
			afx_msg void OnVolumePer();
			afx_msg void OnCancel();
			DECLARE_MESSAGE_MAP()

		private:
			virtual BOOL PreTranslateMessage(MSG* pMsg);
			inline int GetY(float f);
			void SetMode();
			void InitSpectrum();
			void Init();

			WireGui* wire_gui_;
			int wireIndex;
			int isrcMac;
			UINT this_index;
			bool Inval;
			Machine* _pSrcMachine;
			Machine* _pDstMachine;
			int _dstWireIndex;
			float invol;
			float mult;
			int scope_mode;
			int scope_peak_rate;
			int scope_osc_freq;
			int scope_osc_rate;
			int scope_spec_bands;
			int scope_spec_rate;
			int scope_spec_mode;
			int scope_phase_rate;
			float peakL,peakR;
			float peak2L,peak2R;
			int peakLifeL,peakLifeR;
			float o_mvc, o_mvpc, o_mvl, o_mvdl, o_mvpl, o_mvdpl, o_mvr, o_mvdr, o_mvpr, o_mvdpr;
			float *pSamplesL;
			float *pSamplesR;
			float *inl;
			float *inr;
			// Dialog Data
			enum { IDD = IDD_WIREDIALOG };
			CSliderCtrl	m_slider;
			CSliderCtrl	m_slider2;
			CStatic	m_volabel_per;
			CStatic	m_volabel_db;
			CButton m_mode;
			CSliderCtrl	m_volslider;						
			CChildView* m_pParent;
			CBitmap* bufBM;
			CBitmap* clearBM;
			CPen linepenL;
			CPen linepenR;
			CPen linepenbL;
			CPen linepenbR;
			CRect rc;
			CFont font;
			BOOL hold;
			BOOL clip;
			int pos;
			int bar_heightsl[MAX_SCOPE_BANDS];
			int bar_heightsr[MAX_SCOPE_BANDS];
			float sth[SCOPE_SPEC_SAMPLES][MAX_SCOPE_BANDS];
			float cth[SCOPE_SPEC_SAMPLES][MAX_SCOPE_BANDS];
		};

	}   // namespace
}   // namespace
