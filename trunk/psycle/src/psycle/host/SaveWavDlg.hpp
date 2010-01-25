///\file
///\brief interface file for psycle::host::CSaveWavDlg.
#pragma once
#include "Psycle.hpp"
#include <mmreg.h>

#if PSYCLE__CONFIGURATION__USE_PSYCORE
#include <psycle/helpers/riff.hpp>
#include <psycle/helpers/dither.hpp>
#include <psycle/audiodrivers/wavefileout.h>
namespace psycle {
	namespace core {
		class SequenceLine;
		class SequenceEntry;
	}
}
#endif

namespace psycle {
	namespace host {

		class CChildView;
		class CSelection;

		/// save wave dialog window.
		class CSaveWavDlg : public CDialog
		{
		// Construction
		public:
			/// mfc compliant constructor
			CSaveWavDlg(CChildView* pChildView, CSelection* pBlockSel, CWnd* pParent = 0);

			void SaveEnd(void);
			void SaveToClipboard();
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
		public:
			static int channelmode;
			struct fullheader
			{
				std::uint32_t	head;
				std::uint32_t	size;
				std::uint32_t	head2;
				std::uint32_t	fmthead;
				std::uint32_t	fmtsize;
				WAVEFORMATEX	fmtcontent;
				std::uint32_t datahead;
				std::uint32_t datasize;
			} clipboardwavheader;

			std::vector<char*> clipboardmem;

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
			afx_msg void OnOutputsample();
			//}}AFX_MSG
			DECLARE_MESSAGE_MAP()						

		public:
			void SwitchToTmpPlay();
			void SwitchToNormalPlay();

			psycle::audiodrivers::WaveFileOut file_out_;			
			psycle::core::SequenceLine* seq_tmp_play_line_;
			psycle::core::SequenceLine* seq_main_play_line_; // mainline between normal play
			psycle::core::SequenceEntry* seq_end_entry_;

		};

		
	}   // namespace host
}   // namespace psycle
