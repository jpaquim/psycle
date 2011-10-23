///\file
///\brief interface file for psycle::host::CEnvDialog.
#pragma once
#include <psycle/host/detail/project.hpp>
#include "Psycle.hpp"

namespace psycle { namespace host {
		class Song;

		/// enveloppe window.
		class CEnvDialog : public CDialog
		{
		public:
			CEnvDialog(CWnd* pParent = 0);
			Song* _pSong;
			void DrawADSR(int AX,int BX,int CX,int DX);
			void DrawADSRFil(int AX,int BX,int CX,int DX);
			enum { IDD = IDD_ENVDIALOG };
			CStatic	m_envelope_label;
			CSliderCtrl	m_envelope_slider;
			CComboBox	m_filtercombo;
			CStatic	m_q_label;
			CStatic	m_cutoff_label;
			CSliderCtrl	m_q_slider;
			CSliderCtrl	m_cutoff_slider;
			CStatic	m_f_s_label;
			CStatic	m_f_r_label;
			CStatic	m_f_d_label;
			CStatic	m_f_a_label;
			CSliderCtrl	m_f_release_slider;
			CSliderCtrl	m_f_sustain_slider;
			CSliderCtrl	m_f_decay_slider;
			CSliderCtrl	m_f_attack_slider;
			CStatic	m_a_a_label;
			CStatic	m_filframe;
			CStatic	m_a_s_label;
			CStatic	m_a_r_label;
			CStatic	m_a_d_label;
			CStatic	m_ampframe;
			CSliderCtrl	m_a_release_slider;
			CSliderCtrl	m_a_sustain_slider;
			CSliderCtrl	m_a_decay_slider;
			CSliderCtrl	m_a_attack_slider;
		protected:
			virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
			virtual BOOL OnInitDialog();
		protected:
			DECLARE_MESSAGE_MAP()
			afx_msg void OnDrawAmpAttackSlider(NMHDR* pNMHDR, LRESULT* pResult);
			afx_msg void OnDrawAmpDecaySlider(NMHDR* pNMHDR, LRESULT* pResult);
			afx_msg void OnDrawAmpSustainSlider(NMHDR* pNMHDR, LRESULT* pResult);
			afx_msg void OnDrawAmpReleaseSlider(NMHDR* pNMHDR, LRESULT* pResult);
			afx_msg void OnCustomdrawFSlider1(NMHDR* pNMHDR, LRESULT* pResult);
			afx_msg void OnCustomdrawFSlider2(NMHDR* pNMHDR, LRESULT* pResult);
			afx_msg void OnCustomdrawFSlider3(NMHDR* pNMHDR, LRESULT* pResult);
			afx_msg void OnCustomdrawFSlider4(NMHDR* pNMHDR, LRESULT* pResult);
			afx_msg void OnCustomdrawSliderCutoff(NMHDR* pNMHDR, LRESULT* pResult);
			afx_msg void OnCustomdrawSliderQ(NMHDR* pNMHDR, LRESULT* pResult);
			afx_msg void OnSelchangeCombo1();
			afx_msg void OnCustomdrawEnvelope(NMHDR* pNMHDR, LRESULT* pResult);
		};

	}   // namespace
}   // namespace
