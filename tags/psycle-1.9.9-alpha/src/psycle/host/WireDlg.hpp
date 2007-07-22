///\file
///\brief interface file for psycle::host::CWireDlg.
#pragma once
#include <psycle/engine/constants.hpp>
#include <afxwin.h>
UNIVERSALIS__COMPILER__NAMESPACE__BEGIN(psycle)
	UNIVERSALIS__COMPILER__NAMESPACE__BEGIN(host)

		class CChildView;

		#define SCOPE_BUF_SIZE 4096
		#define SCOPE_SPEC_SAMPLES	256
		#define MAX_SCOPE_BANDS 128
		#define LOG2	0.301029995663981f

		class Machine;
		class CMultiScopeCtrl : public CStatic
		{
		public:
			enum
			{
				mode_vu=0,
				mode_scope,
				mode_spectrum,
				mode_phase,
				num_osci_modes,
				mode_viewcons,
				num_modes
			} scope_modes;

			CMultiScopeCtrl();
			virtual ~CMultiScopeCtrl();

			void Initialize();
			void InitSpectrum();
			void PrepareView();
			inline int GetY(float f,float _invol);
			RECT GetNodeRect(int node, bool bDirection);
			virtual void DrawItem( LPDRAWITEMSTRUCT lpDrawItemStruct );
			void PaintVu(CDC* bufDC);
			void PaintOsci(CDC* bufDC);
			void PaintSpectrum(CDC* bufDC);
			void PaintPhase(CDC* bufDC);
			void PaintConnections(CDC* bufDC);

		protected:
			CBitmap* bufBM;
			CBitmap* clearBM;
			CPen linepenL;
			CPen linepenR;
			CPen linepenbL;
			CPen linepenbR;
			CRect rcscope;
			CRect rccons;

			CFont font;

			float rangecorrection;
			float vucorrection;
			float scopecorrection;
			float spectrumcorrection;
			float cth[SCOPE_BUF_SIZE][MAX_SCOPE_BANDS];
			float sth[SCOPE_BUF_SIZE][MAX_SCOPE_BANDS];
			float heightcompensation[MAX_SCOPE_BANDS];

			int bar_heightsl[MAX_SCOPE_BANDS];
			int bar_heightsr[MAX_SCOPE_BANDS];

		public:
			float *pSamplesL;
			float *pSamplesR;
			Machine* _pSrcMachine;
			Machine* _pDstMachine;


			bool hold;
			int scope_mode;
			int scope_offset;
			int scope_peak_rate;
			int scope_osc_freq;
			int scope_osc_rate;
			int scope_spec_bands;
			int scope_spec_rate;
			int scope_phase_rate;

			bool clip;
			float involL;
			float involR;
			float peakL,peakR;
			float peak2L,peak2R;
			int peakLifeL,peakLifeR;
			float o_mvc, o_mvpc, o_mvl, o_mvdl, o_mvpl, o_mvdpl, o_mvr, o_mvdr, o_mvpr, o_mvdpr;

			int numIns, numOuts;

			int conn_ins_spacing;
			int conn_outs_spacing;
			bool bSelectionType;
			bool bLastSelectionType;
			bool bOneSelected;
			int conn_sel_in, conn_sel_out;

			std::vector <bool> ins_node_used;
			std::vector <bool> outs_node_used;
			int connections[64][64]; //hopefully this is enough, shouldn't be more than 32 pairs of channels


		};



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
			void GraphClicked (bool bSelType, CPoint point);

			virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

			afx_msg void OnTimer(UINT nIDEvent);
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
			CMultiScopeCtrl m_multiscope;

		protected:
			CChildView* m_pParent;

		public:
			float pSamplesL[SCOPE_BUF_SIZE];
			float pSamplesR[SCOPE_BUF_SIZE];

			UINT this_index;
			int wireIndex;
			int isrcMac;
			float invol;
			Machine* _pSrcMachine;
			Machine* _pDstMachine;
			int _dstWireIndex;

		protected:

		};

	UNIVERSALIS__COMPILER__NAMESPACE__END
UNIVERSALIS__COMPILER__NAMESPACE__END
