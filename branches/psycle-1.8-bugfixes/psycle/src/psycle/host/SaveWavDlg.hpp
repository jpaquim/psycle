///\file
///\brief interface file for psycle::host::CSaveWavDlg.
#pragma once
#include "constants.hpp"
#include "resources/resources.hpp"
#include "mfc_namespace.hpp"
PSYCLE__MFC__NAMESPACE__BEGIN(psycle)
	PSYCLE__MFC__NAMESPACE__BEGIN(host)

		class CChildView;
		class CSelection;

		/// save wave dialog window.
		class CSaveWavDlg : public CDialog
		{
		// Construction
		public:
			/// mfc compliant constructor
			CSaveWavDlg(CChildView* pChildView, CSelection* pBlockSel, CWnd* pParent = 0);

			void SaveTick(void);
			void SaveEnd(void);
			int kill_thread;
			int threadopen;
		// Dialog Data
			//{{AFX_DATA(CSaveWavDlg)
			enum { IDD = IDD_SAVEWAVDLG };
			CButton	m_browse;
			CButton	m_cancel;
			CButton	m_savewave;
			CButton	m_savewires;
			CButton	m_savegens;
			CButton	m_savetracks;
			CButton m_dither;
			CEdit	m_rangestart;
			CEdit	m_rangeend;
			CEdit	m_linestart;
			CEdit	m_lineend;
			CProgressCtrl	m_progress;
			CEdit	m_patnumber;
			CEdit	m_patnumber2;
			CEdit	m_filename;
			CStatic m_text;
			int		m_recmode;
			int		m_outputtype;
			CComboBox	m_rate;
			CComboBox	m_bits;
			CComboBox	m_channelmode;
			CComboBox	m_pdf;
			CComboBox	m_noiseshaping;
			//}}AFX_DATA
		// Overrides
			// ClassWizard generated virtual function overrides
			//{{AFX_VIRTUAL(CSaveWavDlg)
			protected:
			virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
			//}}AFX_VIRTUAL
		// Implementation
		protected:
			HANDLE thread_handle;

			CChildView* pChildView;
			CSelection* pBlockSel;
			int lastpostick;
			int lastlinetick;
			int tickcont;

			static int rate;
			static int bits;
			static int channelmode;
			static int ditherpdf;
			static int noiseshape;

			struct pdf
			{
				enum pdfs
				{
					triangular=0,
					rectangular,
					gaussian
				};
			};

			int current;

			std::string rootname;

			static BOOL savetracks;
			static BOOL savewires;
			static BOOL savegens;

			bool _Muted[MAX_BUSES];

			bool autostop;
			bool playblock;
			bool loopsong;
			bool sel[MAX_SONG_POSITIONS];
			bool saving;
			bool dither;

			void SaveWav(std::string file, int bits, int rate, int channelmode);
			
			// Generated message map functions
			//{{AFX_MSG(CSaveWavDlg)
			virtual BOOL OnInitDialog();
			afx_msg void OnFilebrowse();
			afx_msg void OnSelAllSong();
			afx_msg void OnSelRange();
			afx_msg void OnSelPattern();
			afx_msg void OnSavewave();
			virtual void OnCancel();
			afx_msg void OnSelchangeComboBits();
			afx_msg void OnSelchangeComboChannels();
			afx_msg void OnSelchangeComboRate();
			afx_msg void OnSelchangeComboPdf();
			afx_msg void OnSelchangeComboNoiseShaping();
			afx_msg void OnSavetracksseparated();
			afx_msg void OnSavewiresseparated();
			afx_msg void OnSavegensseparated();
			afx_msg void OnToggleDither();
			afx_msg void OnRecblock();
			afx_msg void OnOutputfile();
			afx_msg void OnOutputclipboard();
			//}}AFX_MSG
			DECLARE_MESSAGE_MAP()					
};

		//{{AFX_INSERT_LOCATION}}
		// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

	PSYCLE__MFC__NAMESPACE__END
PSYCLE__MFC__NAMESPACE__END
