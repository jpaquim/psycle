#pragma once

#include "Psycle.hpp"
#include "ExListBox.h"

namespace psycle { namespace host {
	class CMainFrame;
	class CChildView;
	class Song;

	class SequenceBar : public CDialogBar
	{
		DECLARE_DYNAMIC(SequenceBar)

	public:
		SequenceBar();   // standard constructor
		virtual ~SequenceBar();
		
		void InitializeValues(CMainFrame* frame, CChildView* view, Song* song);

	protected:
		virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	public:
		DECLARE_MESSAGE_MAP()
		afx_msg LONG OnInitDialog ( UINT, LONG );
	public:
		afx_msg void OnSelchangeSeqlist();
		afx_msg void OnDblclkSeqlist();
		afx_msg void OnBnClickedIncshort();
		afx_msg void OnBnClickedDecshort();
		afx_msg void OnBnClickedInclong();
		afx_msg void OnBnClickedDeclong();
		afx_msg void OnBnClickedSeqnew();
		afx_msg void OnBnClickedSeqduplicate();
		afx_msg void OnBnClickedSeqins();
		afx_msg void OnBnClickedSeqdelete();
		afx_msg void OnBnClickedSeqcut();
		afx_msg void OnBnClickedSeqcopy();
		afx_msg void OnBnClickedSeqpaste();
		afx_msg void OnBnClickedSeqclr();
		afx_msg void OnBnClickedSeqsrt();
		afx_msg void OnBnClickedDeclen();
		afx_msg void OnBnClickedInclen();
		afx_msg void OnBnClickedFollow();
		afx_msg void OnBnClickedRecordNoteoff();
		afx_msg void OnBnClickedRecordTweaks();
		afx_msg void OnBnClickedShowpattername();
		afx_msg void OnBnClickedMultichannelAudition();
		afx_msg void OnBnClickedNotestoeffects();
		afx_msg void OnBnClickedMovecursorpaste();

		void UpdatePlayOrder(bool mode);
		void UpdateSequencer(int bottom = -1);
	protected:
		CStatic m_duration;
		CStatic m_seqLen;
		CExListBox m_sequence;
	public:
		CButton m_follow;
	protected:
		CButton m_noteoffs;
		CButton m_tweaks;
		CButton m_patNames;
		CButton m_multiChannel;
		CButton m_notesToEffects;
		CButton m_moveWhenPaste;

		CBitmap bplus;
		CBitmap bminus;
		CBitmap bplusplus;
		CBitmap bminusminus;
		CBitmap bless;
		CBitmap bmore;

		CMainFrame* m_pParentMain;
		CChildView*  m_pWndView;
		Song*		m_pSong;

		int seqcopybuffer[MAX_SONG_POSITIONS];
		int seqcopybufferlength;

	};
}}
