///\file
///\brief interface file for psycle::host::CMainFrame.
#pragma once
#include "Psycle.hpp"
#include "ChildView.hpp"
#include "SequenceBar.hpp"
#include "SongBar.hpp"
#include "MachineBar.hpp"
#include "InfoDlg.hpp"
#include "InstrumentEditor.hpp"
#include "MidiMonitorDlg.hpp"

namespace psycle { namespace host {

		class Song;
		class CWaveEdFrame;
		class CGearRackDlg;

		enum
		{
			AUX_PARAMS=0,
			AUX_INSTRUMENT
		};

		/// main frame window.
		class CMainFrame : public CFrameWnd
		{
			friend class InputHandler;
		public:
			CMainFrame();
			virtual ~CMainFrame();
		protected: 
			DECLARE_DYNAMIC(CMainFrame)
		// Operations
		public:
		// Overrides
			virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
			virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
		// Implementation
		public:
			void CenterWindowOnPoint(CWnd* pWnd, POINT point);
			void ClosePsycle();
			void CheckForAutosave();
			void CloseMacGui(int mac,bool closewiredialogs=true);
			void CloseAllMacGuis();
			void ShowMachineGui(int tmac, CPoint point);
			void UpdateEnvInfo();
			void ShowPerformanceDlg();
			void ShowMidiMonitorDlg();
			void ShowInstrumentEditor();
			void StatusBarText(std::string txt);
			void PsybarsUpdate();
			BOOL StatusBarIdleText();
			void StatusBarIdle();
			void RedrawGearRackList();
			void WaveEditorBackUpdate();

		private:
			void SaveRecent();
			#if !defined NDEBUG
				virtual void AssertValid() const;
				virtual void Dump(CDumpContext& dc) const;
			#endif
		public:  // control bar embedded members
			void SetAppSongBpm(int x);
			void SetAppSongTpb(int x);
			void ShiftOctave(int x);
			void UpdateMasterValue(int newvalue);
			void UpdateVumeters(float l, float r, COLORREF vu1,COLORREF vu2,COLORREF vu3,bool clip);

			void ChangeIns(int i);
			void ChangeGen(int i);
			void UpdateComboIns(bool updatelist=true);
			void UpdateComboGen(bool updatelist=true);
			void EditQuantizeChange(int diff);

			bool ToggleFollowSong();
			void UpdatePlayOrder(bool mode);
			void UpdateSequencer(int bottom = -1);
			
			CChildView  m_wndView;
			CToolBar    m_wndToolBar;
			SongBar		m_songBar;
			MachineBar	m_machineBar;
			SequenceBar  m_seqBar;
			CStatusBar  m_wndStatusBar;
			std::string		szStatusIdle;
			
			CInfoDlg	m_wndInfo;
			CMidiMonitorDlg	m_midiMonitorDlg;
			CGearRackDlg* pGearRackDialog;
			CInstrumentEditor	m_wndInst;
			CWaveEdFrame*	m_pWndWed;

			// Attributes
		public:
			Song* _pSong;
			bool isguiopen[MAX_MACHINES];
			CFrameWnd	*m_pWndMac[MAX_MACHINES];



		protected:
			LRESULT OnSetMessageString (WPARAM wParam, LPARAM lParam);
			afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
			afx_msg void OnSetFocus(CWnd *pOldWnd);
			afx_msg void OnClose();
			afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
			afx_msg void OnDestroy();
			afx_msg void OnDropFiles(HDROP hDropInfo);
//SongBar start
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
			afx_msg void OnClipbut();
//SongBar end
//Machinebar start
			afx_msg void OnSelchangeCombostep();
			afx_msg void OnCloseupCombostep();
			afx_msg void OnSelchangeBarCombogen();
			afx_msg void OnCloseupBarCombogen();
			afx_msg void OnCloseupAuxselect();
			afx_msg void OnSelchangeAuxselect();
			afx_msg void OnSelchangeBarComboins();
			afx_msg void OnCloseupBarComboins();
			afx_msg void OnBDecgen();
			afx_msg void OnBIncgen();
			afx_msg void OnGearRack();
			afx_msg void OnBDecwav();
			afx_msg void OnBIncwav();
		public:
			afx_msg void OnLoadwave();
		protected:
			afx_msg void OnSavewave();
			afx_msg void OnEditwave();
			afx_msg void OnWavebut();
//Machinebar end
//sequencebar start
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
//sequencebar end
//Statusbar start
			afx_msg void OnUpdateIndicatorSeqPos(CCmdUI *pCmdUI);
			afx_msg void OnUpdateIndicatorPattern(CCmdUI *pCmdUI);
			afx_msg void OnUpdateIndicatorLine(CCmdUI *pCmdUI);
			afx_msg void OnUpdateIndicatorTime(CCmdUI *pCmdUI);
			afx_msg void OnUpdateIndicatorEdit(CCmdUI *pCmdUI);
			afx_msg void OnUpdateIndicatorFollow(CCmdUI *pCmdUI);
			afx_msg void OnUpdateIndicatorNoteoff(CCmdUI *pCmdUI);
			afx_msg void OnUpdateIndicatorTweaks(CCmdUI *pCmdUI);
			afx_msg void OnUpdateIndicatorOctave(CCmdUI *pCmdUI);
//Statusbar end
//Menu start
			afx_msg void OnViewSongbar();
			afx_msg void OnViewMachinebar();
			afx_msg void OnViewSequencerbar();
			afx_msg void OnUpdateViewSongbar(CCmdUI* pCmdUI);
			afx_msg void OnUpdateViewSequencerbar(CCmdUI* pCmdUI);
			afx_msg void OnUpdateViewMachinebar(CCmdUI* pCmdUI);
			afx_msg void OnPsyhelp();
//Menu end
			DECLARE_MESSAGE_MAP()
};

}}
