///\file
///\brief interface file for psycle::host::CSaveWavDlg.
#pragma once
#include "Psycle.hpp"
#include <mmreg.h>
#include <psycle/helpers/riff.hpp>
#include <psycle/helpers/dither.hpp>
#include <psycle/audiodrivers/wavefileout.h>
#include <universalis/stdlib/thread.hpp>

namespace psycle {
	namespace core {
		class SequenceLine;
		class SequenceEntry;
	}
}

namespace psycle {
	namespace host {

		class CChildView;
		class CSelection;
		
		class CSaveWavDlg : public CDialog
		{
		public:
			CSaveWavDlg(CChildView* pChildView, CSelection* pBlockSel, CWnd* pParent = 0);

		private:
			void SaveEnd();
			void SaveToClipboard();			
			void SwitchToTmpPlay();
			void SwitchToNormalPlay();
			void SaveNormal();
			void SaveWires();
			void SaveGenerators();
			void BackUpWires();
			void SaveFile();
			void ActivateOutputs(bool on);
			virtual void DoDataExchange(CDataExchange* pDX);
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
			DECLARE_MESSAGE_MAP()						

			int kill_thread;
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
			CChildView* pChildView;
			CSelection* pBlockSel;
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
			bool muted_[MAX_BUSES];
			bool sel[MAX_SONG_POSITIONS];
			psycle::audiodrivers::WaveFileOut file_out_;			
			psycle::core::SequenceLine* seq_tmp_play_line_;
			psycle::core::SequenceLine* seq_main_play_line_; // mainline between normal play
			psycle::core::SequenceEntry* seq_end_entry_;		
			thread* thread_;			
		};

		
	}   // namespace host
}   // namespace psycle
