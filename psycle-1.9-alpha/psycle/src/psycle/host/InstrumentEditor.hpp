///\file
///\brief interface file for psycle::host::CInstrumentEditor.
#pragma once
#include <psycle/host/detail/project.hpp>
#include "Psycle.hpp"

namespace psycle { namespace host {

		class CMainFrame;

		/// instrument window.
		class CInstrumentEditor : public CDialog
		{
		public:
			CInstrumentEditor(class ProjectData* projects, CMainFrame* parent);

			void WaveUpdate();

		private:
			void Init();
			void UpdateNoteLabel();	
			void UpdateCombo();
			inline Song* song();				
			virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support			
			afx_msg void OnLoopoff();
			afx_msg void OnLoopforward();
			afx_msg void OnCustomdrawSlider1(NMHDR* pNMHDR, LRESULT* pResult);
			afx_msg void OnChangeLockInst();
			afx_msg void OnChangeInstname();
			afx_msg void OnSelchangeNnaCombo();
			afx_msg void OnPrevInstrument();
			afx_msg void OnNextInstrument();
			afx_msg void OnEnvButton();
			afx_msg void OnCustomdrawPanslider(NMHDR* pNMHDR, LRESULT* pResult);
			afx_msg void OnLockinst();
			afx_msg void OnRpan();
			afx_msg void OnRcut();
			afx_msg void OnRres();
			afx_msg void OnLoopCheck();
			afx_msg void OnChangeLoopedit();
			afx_msg void OnCustomdrawSlider2(NMHDR* pNMHDR, LRESULT* pResult);
			afx_msg void OnKillInstrument();
			afx_msg void OnInsDecoctave();
			afx_msg void OnInsDecnote();
			afx_msg void OnInsIncnote();
			afx_msg void OnInsIncoctave();
			DECLARE_MESSAGE_MAP()

			enum { IDD = IDD_INSTRUMENT };
			ProjectData* projects_;
			bool cando;
			bool initializingDialog;
			CMainFrame* pParentMain;
			CStatic	m_notelabel;
			CStatic	m_panlabel;
			CStatic	m_finelabel;
			CSliderCtrl	m_finetune;
			CEdit	m_loopedit;
			CButton	m_loopcheck;
			CButton	m_rres_check;
			CSliderCtrl	m_panslider;
			CButton m_lockinst;
			CButton	m_rpan_check;
			CButton	m_rcut_check;
			CComboBox	m_nna_combo;
			CEdit m_lockinstnumber;
			CEdit	m_instname;
			CStatic	m_volabel;
			CSliderCtrl	m_volumebar;
			CStatic	m_wlen;
			CStatic	m_loopstart;
			CStatic	m_loopend;
			CStatic	m_looptype;
			CStatic	m_instlabel;
		};

	}   // namespace
}   // namespace
