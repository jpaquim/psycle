///\file
///\brief interface file for psycle::host::CInstrumentEditor.
#pragma once
#include <psycle/host/detail/project.hpp>
#include "Psycle.hpp"
#include "EnvelopeEditor.hpp"

namespace psycle { namespace host {

		/// instrument window for classic sampler.
		class CInstrumentEditor : public CPropertyPage
		{
		public:
			DECLARE_DYNAMIC(CInstrumentEditor)

		public:
			CInstrumentEditor();
			virtual ~CInstrumentEditor();

			enum { IDD = IDD_INST_SAMPLER_INST };

			void WaveUpdate();

		protected:
			virtual BOOL PreTranslateMessage(MSG* pMsg);
			virtual void DoDataExchange(CDataExchange* pDX);    // Compatibilidad con DDX o DDV
			virtual BOOL OnInitDialog();

		protected:
			DECLARE_MESSAGE_MAP()
			afx_msg BOOL OnSetActive(void);

			afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
			afx_msg void OnCustomdrawSliderm(UINT idx, NMHDR* pNMHDR, LRESULT* pResult);
			afx_msg void OnLoopoff();
			afx_msg void OnLoopforward();
			afx_msg void OnChangeLockInst();
			afx_msg void OnEnChangeSamplerate();
			afx_msg void OnSelchangeNnaCombo();
			afx_msg void OnPrevTenInstrument();
			afx_msg void OnPrevInstrument();
			afx_msg void OnNextInstrument();
			afx_msg void OnNextTenInstrument();
			afx_msg void OnLockinst();
			afx_msg void OnSelchangeFilterType();
			afx_msg void OnRpan();
			afx_msg void OnRcut();
			afx_msg void OnRres();
			afx_msg void OnLoopCheck();
			afx_msg void OnChangeLoopedit();
			afx_msg void OnKillInstrument();
			afx_msg void OnInsDecoctave();
			afx_msg void OnInsDecnote();
			afx_msg void OnInsIncnote();
			afx_msg void OnInsIncoctave();
			afx_msg LRESULT OnEnvelopeChanged(WPARAM wParam, LPARAM lParam);


		protected:
			void RefreshEnvelopes();
			void UpdateNoteLabel();
			void UpdateComboNNA();

			void SliderVolume(CSliderCtrl& the_slider);
			void SliderFinetune(CSliderCtrl& the_slider);
			void SliderPan(CSliderCtrl& the_slider);
			void SliderFilterCut(CSliderCtrl& the_slider);
			void SliderFilterRes(CSliderCtrl& the_slider);
			void SliderAmpRel(CSliderCtrl& the_slider);
			void SliderAmpSus(CSliderCtrl& the_slider);
			void SliderAmpDec(CSliderCtrl& the_slider);
			void SliderAmpAtt(CSliderCtrl& the_slider);
			void SliderFilterMod(CSliderCtrl& the_slider);
			void SliderFilterRel(CSliderCtrl& the_slider);
			void SliderFilterSus(CSliderCtrl& the_slider);
			void SliderFilterDec(CSliderCtrl& the_slider);
			void SliderFilterAtt(CSliderCtrl& the_slider);

			bool initializingDialog;

			CStatic	m_instlabel;
			CButton m_lockinst;
			CEdit m_lockinstnumber;
			CComboBox m_nna_combo;
			CButton	m_loopcheck;
			CEdit	m_loopedit;
			CButton	m_rpan_check;
			CButton	m_rcut_check;
			CButton	m_rres_check;
			CComboBox	m_filtercombo;
			CSliderCtrl	m_q_slider;
			CSliderCtrl	m_cutoff_slider;
			CSliderCtrl	m_volumebar;
			CSliderCtrl	m_finetune;
			CSliderCtrl	m_panslider;
			CStatic	m_notelabel;
			CStatic	m_looptype;
			CStatic	m_loopstart;
			CStatic	m_loopend;
			CStatic	m_wlen;

			CSliderCtrl	m_a_attack_slider;
			CSliderCtrl	m_a_decay_slider;
			CSliderCtrl	m_a_sustain_slider;
			CSliderCtrl	m_a_release_slider;
			CSliderCtrl	m_f_attack_slider;
			CSliderCtrl	m_f_decay_slider;
			CSliderCtrl	m_f_sustain_slider;
			CSliderCtrl	m_f_release_slider;
			CSliderCtrl	m_f_amount_slider;
			CEnvelopeEditor m_ampframe;
			CEnvelopeEditor m_filframe;

			//These two are just for the CEnvelopeEditor.
			XMInstrument::Envelope m_ampEnv;
			XMInstrument::Envelope m_filEnv;
		};

	}   // namespace
}   // namespace
