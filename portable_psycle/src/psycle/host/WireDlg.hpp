///\file
///\brief interface file for psycle::host::CWireDlg.
#pragma once
#include "Machine.hpp"
#include "constants.hpp"
namespace psycle
{
	namespace host
	{
		class CChildView;

		#define MAX_SCOPE_BANDS 64
		#define SCOPE_BUF_SIZE 4096
		#define SCOPE_SPEC_SAMPLES	256

		class Song;

		/// wire monitor window.
		class CWireDlg : public CDialog
		{
		public:
			CWireDlg(CChildView* pParent);
			BOOL Create();
			afx_msg void OnCancel();
			UINT this_index;
			int wireIndex;
			int isrcMac;
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
			int scope_phase_rate;

			float peakL,peakR;
			float peak2L,peak2R;
			int peakLifeL,peakLifeR;

			float o_mvc, o_mvpc, o_mvl, o_mvdl, o_mvpl, o_mvdpl, o_mvr, o_mvdr, o_mvpr, o_mvdpr;

			float pSamplesL[SCOPE_BUF_SIZE];
			float pSamplesR[SCOPE_BUF_SIZE];
		// Dialog Data
			//{{AFX_DATA(CWireDlg)
			enum { IDD = IDD_WIREDIALOG };
			CSliderCtrl	m_slider;
			CSliderCtrl	m_slider2;
			CStatic	m_volabel_per;
			CStatic	m_volabel_db;
			CButton m_mode;
			CSliderCtrl	m_volslider;
			//}}AFX_DATA
		// Overrides
			// ClassWizard generated virtual function overrides
			//{{AFX_VIRTUAL(CWireDlg)
			public:
			virtual BOOL PreTranslateMessage(MSG* pMsg);
			protected:
			virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
			//}}AFX_VIRTUAL
		// Implementation
		protected:
			inline int GetY(float f);
			void SetMode();
			CChildView* m_pParent;
			CBitmap* bufBM;
			CBitmap* clearBM;
			CPen linepenL;
			CPen linepenR;
			CPen linepenbL;
			CPen linepenbR;
			CRect rc;
			CFont font;
			CFont* oldFont;
			BOOL hold;
			BOOL clip;
			int pos;
			int bar_heightsl[MAX_SCOPE_BANDS];
			int bar_heightsr[MAX_SCOPE_BANDS];
			// Generated message map functions
			//{{AFX_MSG(CWireDlg)
			virtual BOOL OnInitDialog();
			afx_msg void OnCustomdrawSlider1(NMHDR* pNMHDR, LRESULT* pResult);
			afx_msg void OnButton1();
			afx_msg void OnTimer(UINT nIDEvent);
			afx_msg void OnCustomdrawSlider(NMHDR* pNMHDR, LRESULT* pResult);
			afx_msg void OnCustomdrawSlider2(NMHDR* pNMHDR, LRESULT* pResult);
			afx_msg void OnMode();
			afx_msg void OnHold();
			afx_msg void OnVolumeDb();
			afx_msg void OnVolumePer();
			//}}AFX_MSG
			DECLARE_MESSAGE_MAP()
		};

		//{{AFX_INSERT_LOCATION}}
		// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
	}
}
