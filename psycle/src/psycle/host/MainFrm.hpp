///\file
///\brief interface file for psycle::host::CMainFrame.
#pragma once
#include "Psycle.hpp"
#include "ProjectData.hpp"
#include "ChildView.hpp"
#include "SeqView.hpp"
#include "ExListBox.h"
#include "InstrumentEditor.hpp"
#include "InfoDlg.hpp"
#include "MidiMonitorDlg.hpp"
#include "WaveEdFrame.hpp"

#if PSYCLE__CONFIGURATION__USE_PSYCORE
using namespace psycle::core;
#endif

namespace psycle {
	namespace host {

		class CWaveEdFrame;
		class CGearRackDlg;

		enum { AUX_MIDI = 0, AUX_PARAMS, AUX_WAVES };

		/// main frame window.
		class CMainFrame : public CFrameWnd
		{
		friend class InputHandler;
		public:
			CMainFrame();
			virtual ~CMainFrame();

			ProjectData* projects() { return &projects_; }

			virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
			virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);

			void SetAppSongBpm(int x);
			void SetAppSongTpb(int x);
			void ClosePsycle();
			void CheckForAutosave();
			void WaveEditorBackUpdate();									
			void UpdateEnvInfo();
			void HidePerformanceDlg();
			void ShowPerformanceDlg();
			void HideMidiMonitorDlg();
			void ShowMidiMonitorDlg();
			void HideInstrumentEditor();
			void ShowInstrumentEditor();
			void StatusBarText(const std::string& txt);
			void UpdateComboIns(bool updatelist=true);
			void UpdateComboGen(bool updatelist=true);
			void PsybarsUpdate();
			void UpdateVumeters(float l, float r, COLORREF vu1,COLORREF vu2,COLORREF vu3,bool clip);
			BOOL StatusBarIdleText();
			void StatusBarIdle();
			void RedrawGearRackList();
			void EditQuantizeChange(int diff);
			void ShiftOctave(int x);
			void UpdateMasterValue(int newvalue);
			int GetNumFromCombo(CComboBox* cb);
			void ChangeIns(int i);
			void ChangeGen(int i);
			afx_msg void OnFollowSong();
			afx_msg void OnLoadwave();
			///\ todo should be private
			ProjectData	projects_;
			CDialogBar m_wndControl2;
			CChildView m_wndView;
			SequencerView m_wndSeq;
			CGearRackDlg*	pGearRackDialog;
			CInstrumentEditor	m_wndInst;
			CWaveEdFrame* wave_ed_;

		private:
			void SetUpStartProject();
			void SaveRecent();
			#if !defined NDEBUG
				virtual void AssertValid() const;
				virtual void Dump(CDumpContext& dc) const;
			#endif
			LRESULT OnSetMessageString (WPARAM wParam, LPARAM lParam);
			afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
			afx_msg void OnSetFocus(CWnd *pOldWnd);
			afx_msg void OnBarButton1();
			afx_msg void OnBpmAddTen();
			afx_msg void OnBpmDecOne();
			afx_msg void OnBpmDecTen();
			afx_msg void OnSelchangeSscombo2();
			afx_msg void OnCloseupSscombo2();
			afx_msg void OnSelchangeBarCombogen();
			afx_msg void OnCloseupBarCombogen();
			afx_msg void OnSelchangeBarComboins();
			afx_msg void OnCloseupBarComboins();
			afx_msg void OnClipbut();
			afx_msg void OnSelchangeTrackcombo();
			afx_msg void OnCloseupTrackcombo();
			afx_msg void OnPsyhelp();
			afx_msg void OnSavewave();
			afx_msg void OnEditwave();
			afx_msg void OnGearRack();
			afx_msg void OnWavebut();
			afx_msg void OnDestroy();
			afx_msg void OnBDecgen();
			afx_msg void OnBIncgen();
			afx_msg void OnBDecwav();
			afx_msg void OnBIncwav();
			afx_msg void OnClose();
			afx_msg void OnSelchangeSeqlist();
			afx_msg void OnDblclkSeqlist();
			afx_msg	void OnInclen();
			afx_msg void OnDeclen();
			afx_msg void OnIncshort();
			afx_msg void OnDecshort();
			afx_msg void OnSeqins();
			afx_msg void OnSeqnew();
			afx_msg void OnSeqcut();
			afx_msg void OnSeqcopy();
			afx_msg void OnSeqpaste();
			afx_msg void OnSeqdelete();
			afx_msg void OnSeqduplicate();
			afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
			afx_msg void OnDecTPB();
			afx_msg void OnIncTPB();					
			afx_msg void OnSeqclr();
			afx_msg void OnSeqsort();
			afx_msg void OnMultichannelAudition();
			afx_msg void OnRecordNoteoff();
			afx_msg void OnRecordTweaks();
			afx_msg void OnCloseupAuxselect();
			afx_msg void OnSelchangeAuxselect();
			afx_msg void OnDeclong();
			afx_msg void OnInclong();
			afx_msg void OnSeqShowpattername();
			afx_msg void OnUpdateIndicatorSeqPos(CCmdUI *pCmdUI);
			afx_msg void OnUpdateIndicatorPattern(CCmdUI *pCmdUI);
			afx_msg void OnUpdateIndicatorLine(CCmdUI *pCmdUI);
			afx_msg void OnUpdateIndicatorTime(CCmdUI *pCmdUI);
			afx_msg void OnUpdateIndicatorEdit(CCmdUI *pCmdUI);
			afx_msg void OnUpdateIndicatorFollow(CCmdUI *pCmdUI);
			afx_msg void OnUpdateIndicatorNoteoff(CCmdUI *pCmdUI);
			afx_msg void OnUpdateIndicatorTweaks(CCmdUI *pCmdUI);
			afx_msg void OnUpdateIndicatorOctave(CCmdUI *pCmdUI);
			afx_msg void OnCloseupCombooctave();
			afx_msg void OnSelchangeCombooctave();
			afx_msg void OnDropFiles(HDROP hDropInfo);
			afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
			afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
			afx_msg void OnViewSongbar();
			afx_msg void OnViewMachinebar();
			afx_msg void OnViewSequencerbar();
			afx_msg void OnUpdateViewSongbar(CCmdUI* pCmdUI);
			afx_msg void OnUpdateViewSequencerbar(CCmdUI* pCmdUI);
			afx_msg void OnUpdateViewMachinebar(CCmdUI* pCmdUI);
			afx_msg void OnEditBlockCopy();
			afx_msg void OnEditBlockCut();
			afx_msg void OnEditBlockDelete();
			afx_msg void OnEditBlockMixpaste();
			afx_msg void OnEditBlockPaste();
			afx_msg void OnNotestoeffects();
			afx_msg void OnMoveCursorPaste();
			afx_msg void OnCustomdrawMasterslider(NMHDR* pNMHDR, LRESULT* pResult);
			DECLARE_MESSAGE_MAP()

			CExListBox		m_seqListbox;						
			CInfoDlg		m_wndInfo;
			CMidiMonitorDlg	m_midiMonitorDlg; // MIDI_21st		
			int vuprevL;
			int vuprevR;
			bool macComboInitialized;
			CStatusBar  m_wndStatusBar;
			CReBar      m_wndReBar;
			CToolBar    m_wndToolBar;
			CDialogBar	m_wndControl;
			std::string	szStatusIdle;
			CBitmap blessless;
			CBitmap bless;
			CBitmap bmore;
			CBitmap bmoremore;
			CBitmap bplus;
			CBitmap bminus;
			CBitmap bplusplus;
			CBitmap bminusminus;
			CBitmap blittleleft;
			CBitmap blittleright;
		};

	}   // namespace host
}   // namespace psycle
