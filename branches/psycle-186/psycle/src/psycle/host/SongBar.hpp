
#pragma once

#include "Psycle.hpp"

namespace psycle { namespace host {
	class CChildView;
	class CMainFrame;
	class Song;

	class SongBar : public CDialogBar
	{
	DECLARE_DYNAMIC(SongBar)

	public:
		SongBar(void);
		virtual ~SongBar(void);

		void InitializeValues(CMainFrame* frame, CChildView* view, Song* song);
		void SetAppSongBpm(int x);
		void SetAppSongTpb(int x);
		void ShiftOctave(int x);
		void UpdateMasterValue(int newvalue);
		void UpdateVumeters(float l, float r, COLORREF vu1,COLORREF vu2,COLORREF vu3,bool clip);
	protected:
		virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

		DECLARE_MESSAGE_MAP()
		afx_msg LONG OnInitDialog ( UINT, LONG );
	public:
		afx_msg void OnSelchangeTrackcombo();
		afx_msg void OnCloseupTrackcombo();
		afx_msg void OnBpmAddOne();
		afx_msg void OnBpmAddTen();
		afx_msg void OnBpmDecOne();
		afx_msg void OnBpmDecTen();
		afx_msg void OnDecTPB();
		afx_msg void OnIncTPB();
		afx_msg void OnCloseupCombooctave();
		afx_msg void OnSelchangeCombooctave();
		afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
		afx_msg void OnClipbut();
		afx_msg BOOL OnToolTipNotify( UINT unId, NMHDR *pstNMHDR, LRESULT *pstResult );

		CComboBox       m_trackcombo;
		CComboBox       m_octavecombo;
		CSliderCtrl		m_masterslider;
	protected:
		CStatic			m_bpmlabel;
		CStatic			m_tpblabel;

		CBitmap blessless;
		CBitmap bless;
		CBitmap bmore;
		CBitmap bmoremore;

		CMainFrame* m_pParentMain;
		CChildView*  m_pWndView;
		Song*		m_pSong;

		int vuprevL;
		int vuprevR;
	};
}}
