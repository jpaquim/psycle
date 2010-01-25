///\file
///\brief interface file for psycle::host::CChildView.
#pragma once
#include "Psycle.hpp"

namespace psycle {
	namespace core {
		class Song;
	}

	namespace audiodrivers {
		class AudioDriver;
	}
}

using namespace psycle::core;
using namespace psycle::audiodrivers;

namespace psycle {
	namespace host {

		class CGearTracker;
		class XMSamplerUI;
		class CWaveInMacDlg;
		class MachineView;
		class PatternView;
	
		#define MAX_DRAW_MESSAGES 32

		struct draw_modes
		{
			enum draw_mode
			{
				all, ///< Repaints everything (means, slow). Used when switching views, or when a
				///< whole update is needed (For example, when changing pattern Properties, or TPB)

				all_machines, ///< Used to refresh all the machines, without refreshing the background/wires

				machine, ///< Used to refresh the image of one machine (mac num in "updatePar")

				pattern, ///< Use this when switching Patterns (changing from one to another)

				data, ///< Data has Changed. Which data to update is indicated with DrawLineStart/End
				///< and DrawTrackStart/End
				///< Use it when editing and copy/pasting

				horizontal_scroll, ///< Refresh called by the scrollbars or by mouse scrolling (when selecting).
				///< New values in ntOff and nlOff variables ( new_track_offset and new_line_offset);

				vertical_scroll, ///< Refresh called by the scrollbars or by mouse scrolling (when selecting).
				///< New values in ntOff and nlOff variables ( new_track_offset and new_line_offset);

				//resize, ///< Indicates the Refresh is called from the "OnSize()" event.

				playback, ///< Indicates it needs a refresh caused by Playback (update playback cursor)

				playback_change, ///< Indicates that while playing, a pattern switch is needed.

				cursor, ///< Indicates a movement of the cursor. update the values to "editcur" directly
				///< and call this function.
				///< this is arbitrary message as cursor is checked

				selection, ///< The selection has changed. use "blockSel" to indicate the values.

				track_header, ///< Track header refresh (mute/solo, Record updating)

				//pattern_header, ///< Octave, Pattern name, Edit Mode on/off

				none ///< Do not use this one directly. It is used to detect refresh calls from the OS.

				// If you add any new method, please, add the proper code to "PreparePatternRefresh()" and to
				// "DrawPatternEditor()".
				// Note: Modes are sorted by priority. (although it is not really used)

				// !!!BIG ADVISE!!! : The execution of Repaint(mode) does not imply an instant refresh of
				//						the Screen, and what's worse, you might end calling Repaint(anothermode)
				//						previous of the first refresh. In PreparePatternRefresh() there's code
				//						to avoid problems when two modes do completely different things. On
				//						other cases, it still ends to wrong content being shown.
			};
		};

		struct view_modes
		{
			enum view_mode
			{
				machine,
				pattern,
				sequence
			};
		};
		
		/// child view window
		class CChildView : public CWnd
		{
		public:
			CChildView(CMainFrame* main_frame, class ProjectData* projects);
			virtual ~CChildView();

			void InitTimer();
			void EnableSound();
			void Repaint(draw_modes::draw_mode drawMode = draw_modes::all);					

			void MidiPatternNote(int outnote , int velocity);	// called by the MIDI input to insert pattern notes
			void MidiPatternCommand(int command, int value); // called by midi to insert pattern commands
			void MidiPatternTweak(int command, int value); // called by midi to insert pattern commands
			void MidiPatternTweakSlide(int command, int value); // called by midi to insert pattern commands
			void MidiPatternMidiCommand(int command, int value); // called by midi to insert midi pattern commands
			void MidiPatternInstrument(int value); // called by midi to insert pattern commands
			void MousePatternTweak(int machine, int command, int value);
			void MousePatternTweakSlide(int machine, int command, int value);													
				
			void SetTitleBarText();
			void patTrackMute();
			void patTrackSolo();
			void patTrackRecord();
			void KeyDown( UINT nChar, UINT nRepCnt, UINT nFlags );
			void KeyUp( UINT nChar, UINT nRepCnt, UINT nFlags );								

			MachineView* machine_view() { return machine_view_; }
			PatternView* pattern_view() { return pattern_view_; }

			// addModules,creates mac and pattern view so far.
			// TODO for multisong case.
			void AddModules(class Project* project);

			
		public:
			//RECENT!!!//
			HMENU hRecentMenu;

			//CBitmap machinedial; //the machine dial bitmap

			class CMainFrame* main_frame_;
			ProjectData* projects_;
			//bool multiPattern;			
			CGearTracker * SamplerMachineDialog;
			XMSamplerUI* XMSamplerMachineDialog;
			CWaveInMacDlg* WaveInMachineDialog;						

			int updateMode;
			int updatePar;			// view_modes::pattern: Display update mode. view_modes::machine: Machine number to update.
			int viewMode;
			int CH;
			int CW;
			bool _outputActive;	// This variable indicates if the output (audio or midi) is active or not.
								// Its function is to prevent audio (and midi) operations while it is not
								// initialized, or while song is being modified (New(),Load()..).
								// 			

			int textLeftEdge;

		// Overrides
		protected:
			virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

		//////////////////////////////////////////////////////////////////////
		// Private operations

		private:
			//Recent Files!!!!//
			void CallOpenRecent(int pos);
			//Recent Files!!!!//
			void DrawAllMachineVumeters(CDC *devc);																						
				
			MachineView* machine_view_;
			PatternView* pattern_view_;
			// GDI Stuff		
			CBitmap* bmpDC;
			int FLATSIZES[256];
			psycle::audiodrivers::AudioDriver* output_driver_;
			int last_pos_;			

		public:			
			afx_msg void OnPaint();
			afx_msg void OnLButtonDown( UINT nFlags, CPoint point );
			afx_msg void OnRButtonDown( UINT nFlags, CPoint point );
			afx_msg void OnRButtonUp( UINT nFlags, CPoint point );
			afx_msg void OnLButtonUp( UINT nFlags, CPoint point );
			afx_msg void OnMouseMove( UINT nFlags, CPoint point );
			afx_msg void OnLButtonDblClk( UINT nFlags, CPoint point );
			afx_msg void OnHelpPsycleenviromentinfo();
			afx_msg void OnMidiMonitorDlg();
			afx_msg void OnDestroy();
			afx_msg void OnAppExit();
			afx_msg void OnMachineview();
			afx_msg void OnPatternView();
			afx_msg void OnKeyDown( UINT nChar, UINT nRepCnt, UINT nFlags );
			afx_msg void OnKeyUp( UINT nChar, UINT nRepCnt, UINT nFlags );
			afx_msg void OnBarplay();
			afx_msg void OnBarplayFromStart();
			afx_msg void OnBarrec();
			afx_msg void OnBarstop();
			afx_msg void OnRecordWav();
			afx_msg void OnTimer( UINT nIDEvent );
			afx_msg void OnUpdateRecordWav(CCmdUI* pCmdUI);
			afx_msg void OnFileNew();
			afx_msg BOOL OnExport(UINT id);
			afx_msg BOOL OnFileSave(UINT id);
			afx_msg BOOL OnFileSaveAs(UINT id);
			afx_msg void OnFileLoadsong();
			afx_msg void OnFileRevert();
			afx_msg void OnHelpSaludos();
			afx_msg void OnUpdatePatternView(CCmdUI* pCmdUI);
			afx_msg void OnUpdateMachineview(CCmdUI* pCmdUI);
			afx_msg void OnUpdateBarplay(CCmdUI* pCmdUI);
			afx_msg void OnUpdateBarplayFromStart(CCmdUI* pCmdUI);
			afx_msg void OnUpdateBarrec(CCmdUI* pCmdUI);
			afx_msg void OnFileSongproperties();
			afx_msg void OnViewInstrumenteditor();
			afx_msg void OnNewmachine();
			afx_msg void OnButtonplayseqblock();
			afx_msg void OnUpdateButtonplayseqblock(CCmdUI* pCmdUI);
			afx_msg void OnPopCut();
			afx_msg void OnUpdateCutCopy(CCmdUI* pCmdUI);
			afx_msg void OnPopCopy();
			afx_msg void OnPopPaste();
			afx_msg void OnUpdatePaste(CCmdUI* pCmdUI);
			afx_msg void OnPopMixpaste();
			afx_msg void OnPopDelete();
			afx_msg void OnPopInterpolate();
			afx_msg void OnPopChangegenerator();
			afx_msg void OnPopChangeinstrument();
			afx_msg void OnPopTranspose1();
			afx_msg void OnPopTranspose12();
			afx_msg void OnPopTranspose_1();
			afx_msg void OnPopTranspose_12();
			afx_msg void OnAutostop();
			afx_msg void OnUpdateAutostop(CCmdUI* pCmdUI);
			afx_msg void OnPopTransformpattern();
			afx_msg void OnPopPattenproperties();
			afx_msg void OnPopBlockSwingfill();
			afx_msg void OnPopTrackSwingfill();
			afx_msg void OnSize(UINT nType, int cx, int cy);
			afx_msg void OnConfigurationSettings();
			afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
			afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
			afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
			afx_msg void OnFileImportModulefile();
			afx_msg void OnFileRecent_01();
			afx_msg void OnFileRecent_02();
			afx_msg void OnFileRecent_03();
			afx_msg void OnFileRecent_04();
			afx_msg void OnEditUndo();
			afx_msg void OnEditRedo();
			afx_msg void OnUpdateUndo(CCmdUI* pCmdUI);
			afx_msg void OnUpdateRedo(CCmdUI* pCmdUI);
			afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
			afx_msg void OnMButtonDown( UINT nFlags, CPoint point );
			afx_msg void OnUpdatePatternCutCopy(CCmdUI* pCmdUI);
			afx_msg void OnUpdatePatternPaste(CCmdUI* pCmdUI);
			afx_msg void OnFileSaveaudio();
			afx_msg void OnHelpKeybtxt();
			afx_msg void OnHelpReadme();
			afx_msg void OnHelpTweaking();
			afx_msg void OnHelpWhatsnew();
			afx_msg void OnConfigurationLoopplayback();
			afx_msg void OnUpdateConfigurationLoopplayback(CCmdUI* pCmdUI);
			afx_msg void OnShowPatternSeq();
			afx_msg void OnUpdatePatternSeq(CCmdUI* pCmdUI);
			afx_msg void OnPopBlockswitch();
			afx_msg void OnUpdatePopBlockswitch(CCmdUI *pCmdUI);
			afx_msg void OnPopInterpolateCurve();
			afx_msg void OnPatCopy();
			afx_msg void OnPatPaste();
			afx_msg void OnPatMixPaste();
			afx_msg void OnPatCut();
			afx_msg void OnPatDelete();

			DECLARE_MESSAGE_MAP()
};
			
	}   // namespace
}   // namespace
