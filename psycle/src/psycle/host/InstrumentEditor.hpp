/* -*- mode:c++, indent-tabs-mode:t -*- */
///\file
///\brief interface file for psycle::host::CInstrumentEditor.
#pragma once
#include "resources/resources.hpp"
#include "mfc_namespace.hpp"

PSYCLE__MFC__NAMESPACE__BEGIN(psycle)
	PSYCLE__MFC__NAMESPACE__BEGIN(host)
		class Song;
		class CMainFrame;

		/// instrument window.
		class CInstrumentEditor : public CDialog
		{
		public:
			CInstrumentEditor(CWnd* pParent = 0);
			void UpdateNoteLabel();
			void Validate();
			
			void WaveUpdate();
			void UpdateCombo();
			Song* _pSong;
			bool cando;
			bool initializingDialog;
			CMainFrame* pParentMain;
		// Dialog Data
			//{{AFX_DATA(CInstrumentEditor)
			enum { IDD = IDD_INSTRUMENT };
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
			//}}AFX_DATA
		// Overrides
			// ClassWizard generated virtual function overrides
			//{{AFX_VIRTUAL(CInstrumentEditor)
			protected:
			virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
			//}}AFX_VIRTUAL
		// Implementation
		protected:
			// Generated message map functions
			//{{AFX_MSG(CInstrumentEditor)
			virtual BOOL OnInitDialog();
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
			//}}AFX_MSG
			DECLARE_MESSAGE_MAP()
		};

		//{{AFX_INSERT_LOCATION}}
		// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
	PSYCLE__MFC__NAMESPACE__END
PSYCLE__MFC__NAMESPACE__END
