#pragma once
#include "constants.h"
///\file
///\brief interface file for psycle::host::CSaveWavDlg.
namespace psycle
{
	namespace host
	{
		/// save wave dialog window.
		class CSaveWavDlg : public CDialog
		{
		// Construction
		public:
			/// mfc compliant constructor
			CSaveWavDlg(CWnd* pParent = 0);

			void SaveTick(void);
			void SaveEnd(void);
			int kill_thread;
			int threadopen;
		// Dialog Data
			//{{AFX_DATA(CSaveWavDlg)
			enum { IDD = IDD_SAVEWAVDLG };
			CButton	m_cancel;
			CButton	m_savewave;
			CButton	m_savewires;
			CButton	m_savegens;
			CButton	m_savetracks;
			CEdit	m_rangestart;
			CEdit	m_rangeend;
			CProgressCtrl	m_progress;
			CEdit	m_patnumber;
			CEdit	m_filename;
			CStatic m_text;
			int		m_recmode;
			CComboBox	m_rate;
			CComboBox	m_bits;
			CComboBox	m_channelmode;
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

			int lastpostick;
			int lastlinetick;
			int tickcont;

			static int rate;
			static int bits;
			static int channelmode;

			int current;

			std::string rootname;

			static BOOL savetracks;
			static BOOL savewires;
			static BOOL savegens;

			bool _Muted[MAX_BUSES];

			bool autostop;
			bool playblock;
			bool sel[MAX_SONG_POSITIONS];
			bool saving;

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
			afx_msg void OnSavetracksseparated();
			afx_msg void OnSavewiresseparated();
			afx_msg void OnSavegensseparated();
			//}}AFX_MSG
			DECLARE_MESSAGE_MAP()
		};

		//{{AFX_INSERT_LOCATION}}
		// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
	}
}
