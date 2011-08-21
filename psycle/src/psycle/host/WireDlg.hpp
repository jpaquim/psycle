///\file
///\brief interface file for psycle::host::CWireDlg.
#pragma once
#include <psycle/host/detail/project.hpp>
#include "Psycle.hpp"

namespace psycle {
	namespace host {

		class Machine;

		const int MAX_SCOPE_BANDS = 128;
		const int SCOPE_BUF_SIZE = 4096;
		const int MAX_SCOPE_SPEC_SAMPLES = SCOPE_BUF_SIZE;

		/// wire monitor window.
		class CWireDlg : public CDialog
		{
		public:
			CWireDlg(CWnd* mainView, CWireDlg** windowVar, int wireDlgIdx,
				Machine& srcMac, int srcWireIdx, Machine& dstMac, int dstWireIdx);
			virtual ~CWireDlg();
		
		protected:
			virtual BOOL PreTranslateMessage(MSG* pMsg);
			virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
			virtual BOOL OnInitDialog();
			virtual	void OnCancel();
			virtual void PostNcDestroy();

		protected:
			DECLARE_MESSAGE_MAP()
			afx_msg void OnClose();
			afx_msg void OnTimer(UINT_PTR nIDEvent);
			afx_msg void OnDelete();
			afx_msg void OnMode();
			afx_msg void OnHold();
			afx_msg void OnVolumeDb();
			afx_msg void OnVolumePer();
			afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
			afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);

			void InitSpectrum();
			void SetMode();
			void OnChangeSliderMode(UINT nPos);
			void OnChangeSliderRate(UINT nPos);
			void OnChangeSliderVol(UINT nPos);
			void UpdateVolPerDb();
			inline int GetY(float f);

		public:
			Machine& srcMachine;
			Machine& dstMachine;
		protected:
			CWireDlg** windowVar;
			CWnd* mainView;
			UINT wireDlgIdx;
			int srcWireIdx;
			int dstWireIdx;
		// Dialog Data
			enum { IDD = IDD_WIREDIALOG };
			CSliderCtrl	m_volslider;
			CSliderCtrl	m_sliderMode;
			CSliderCtrl	m_sliderRate;
			CStatic	m_volabel_per;
			CStatic	m_volabel_db;
			CButton m_mode;

			CBitmap* bufBM;
			CBitmap* clearBM;
			CPen linepenL;
			CPen linepenR;
			CPen linepenbL;
			CPen linepenbR;
			CRect rc;
			CFont font;

		protected:
			float invol;
			float mult;
			float *pSamplesL;
			float *pSamplesR;

			int scope_mode;
			int scope_peak_rate;
			int scope_osc_freq;
			int SCOPE_SPEC_SAMPLES;
			int scope_osc_rate;
			int scope_spec_bands;
			int scope_spec_rate;
			int scope_spec_mode;
			int scope_phase_rate;

			//memories for oscillator.
			BOOL hold;
			BOOL clip;
			int pos;
			//memories for vu-meter
			float peakL,peakR;
			float peak2L,peak2R;
			int peakLifeL,peakLifeR;
			//Memories for phase
			float o_mvc, o_mvpc, o_mvl, o_mvdl, o_mvpl, o_mvdpl, o_mvr, o_mvdr, o_mvpr, o_mvdpr;
			//Memories and precalculated values for spectrum
			int bar_heightsl[MAX_SCOPE_BANDS];
			int bar_heightsr[MAX_SCOPE_BANDS];
			float sth[MAX_SCOPE_SPEC_SAMPLES][MAX_SCOPE_BANDS];
			float cth[MAX_SCOPE_SPEC_SAMPLES][MAX_SCOPE_BANDS];
		};

	}   // namespace
}   // namespace
