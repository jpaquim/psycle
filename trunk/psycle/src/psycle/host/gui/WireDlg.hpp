///\file
///\brief interface file for psycle::host::CWireDlg.
#pragma once
#include <psycle/host/engine/Machine.hpp>
#include <psycle/host/engine/constants.hpp>
#include "afxwin.h"
UNIVERSALIS__COMPILER__NAMESPACE__BEGIN(psycle)
	UNIVERSALIS__COMPILER__NAMESPACE__BEGIN(host)
		class CChildView;

		#define SCOPE_BUF_SIZE 4096
		#define SCOPE_SPEC_SAMPLES	256
		#define MAX_SCOPE_BANDS 128

		class Song;

		/// wire monitor window.
		class CWireDlg : public CDialog
		{
		public:
			CWireDlg(CChildView* pParent);
			BOOL Create();

			virtual BOOL OnInitDialog();
			virtual void OnCancel();

			virtual BOOL PreTranslateMessage(MSG* pMsg);
		protected:
			void SetMode();
			inline int GetY(float f);
			void DrawPoints();
			RECT GetNodeRect(int node, bool bDirection);
			void GraphClicked (bool bSelType, CPoint point);

			virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

			afx_msg void OnTimer(UINT nIDEvent);
			afx_msg void OnPaint();
			afx_msg void OnSizing(UINT fwSide, LPRECT pRect);
			afx_msg void OnCustomdrawSlsize(NMHDR* pNMHDR, LRESULT* pResult);
			afx_msg void OnCustomdrawSlspeed(NMHDR* pNMHDR, LRESULT* pResult);
			afx_msg void OnMode();
			afx_msg void OnHold();
			afx_msg void OnDelete();
			afx_msg void OnCustomdrawSlvolume(NMHDR* pNMHDR, LRESULT* pResult);
			afx_msg void OnVolumeDb();
			afx_msg void OnVolumePer();
			afx_msg void OnBnClickedViewScope();
			afx_msg void OnBnClickedViewConnections();
			afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
			afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
			DECLARE_MESSAGE_MAP()

		public:
		// Dialog Data
			enum { IDD = IDD_WIREDIALOG };
			CSliderCtrl	m_slspeed;
			CSliderCtrl	m_slsize;
			CStatic m_lblspeed;
			CStatic m_lblsize;
			CButton m_mode;
			CButton	m_btnper;
			CButton	m_btndb;
			CSliderCtrl	m_slvolume;
		protected:
			CChildView* m_pParent;
			int numIns, numOuts;

		public:
			float pSamplesL[SCOPE_BUF_SIZE];
			float pSamplesR[SCOPE_BUF_SIZE];

			UINT this_index;
			int wireIndex;
			int isrcMac;
			Machine* _pSrcMachine;
			Machine* _pDstMachine;
			bool Inval;
			int _dstWireIndex;
			float invol;

		protected:
			CBitmap* bufBM;
			CBitmap* clearBM;
			CPen linepenL;
			CPen linepenR;
			CPen linepenbL;
			CPen linepenbR;
			CRect rc;
			RECT conn_grapharea;
			bool bcurrentview;
			CFont font;
			CFont* oldFont;

			BOOL hold;
			int scope_mode;
			int scope_offset;
			int scope_peak_rate;
			int scope_osc_freq;
			int scope_osc_rate;
			int scope_spec_bands;
			int scope_spec_rate;
			int scope_phase_rate;

			float mult;
			BOOL clip;
			float peakL,peakR;
			float peak2L,peak2R;
			int peakLifeL,peakLifeR;
			int bar_heightsl[MAX_SCOPE_BANDS];
			int bar_heightsr[MAX_SCOPE_BANDS];

			float o_mvc, o_mvpc, o_mvl, o_mvdl, o_mvpl, o_mvdpl, o_mvr, o_mvdr, o_mvpr, o_mvdpr;


		public:
			int conn_ins_spacing;
			int conn_outs_spacing;
			bool bSelectionType;
			bool bLastSelectionType;
			bool bOneSelected;
			int conn_sel_in, conn_sel_out;

			std::vector <bool> ins_node_used;
			std::vector <bool> outs_node_used;

			std::vector <std::string> ins_node_names;
			std::vector <std::string> outs_node_names;

			int connections[64][64]; //hopefully this is enough, shouldn't be more than 32 pairs of channels

};

		//{{AFX_INSERT_LOCATION}}
		// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
	UNIVERSALIS__COMPILER__NAMESPACE__END
UNIVERSALIS__COMPILER__NAMESPACE__END
